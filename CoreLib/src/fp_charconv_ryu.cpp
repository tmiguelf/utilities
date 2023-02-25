// Copyright 2018 Ulf Adams
//
// The contents of this file may be used under the terms of the Apache License,
// Version 2.0.
//
//    (See accompanying file LICENSE-Apache or copy at
//     http://www.apache.org/licenses/LICENSE-2.0)
//
// Alternatively, the contents of this file may be used under the terms of
// the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE-Boost or copy at
//     https://www.boost.org/LICENSE_1_0.txt)
//
// Unless required by applicable law or agreed to in writing, this software
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.
//
// Modified by: Tiago Freire

#include <CoreLib/fp_charconv.hpp>

#include "ryu/common.hpp"
#include "ryu/f2s_intrinsics.hpp"
#include "ryu/d2s_intrinsics.hpp"

#include "fp_traits.hpp"


namespace core
{

	namespace
	{
		template<typename T>
		struct fp_utils;

		template<>
		struct fp_utils<float>: public fp_utils_pre<float>
		{
			static inline constexpr uint8_t sig_digits(const uint32_t mantissa)
			{
				if(mantissa <        10_ui32) return 1;
				if(mantissa <       100_ui32) return 2;
				if(mantissa <      1000_ui32) return 3;
				if(mantissa <     10000_ui32) return 4;
				if(mantissa <    100000_ui32) return 5;
				if(mantissa <   1000000_ui32) return 6;
				if(mantissa <  10000000_ui32) return 7;
				if(mantissa < 100000000_ui32) return 8;
				return 9;
			}
		};

		template<>
		struct fp_utils<double>: public fp_utils_pre<double>
		{
			static inline constexpr uint8_t sig_digits(const uint64_t mantissa)
			{
				if(mantissa <                10_ui64) return 1;
				if(mantissa <               100_ui64) return 2;
				if(mantissa <              1000_ui64) return 3;
				if(mantissa <             10000_ui64) return 4;
				if(mantissa <            100000_ui64) return 5;
				if(mantissa <           1000000_ui64) return 6;
				if(mantissa <          10000000_ui64) return 7;
				if(mantissa <         100000000_ui64) return 8;
				if(mantissa <        1000000000_ui64) return 9;
				if(mantissa <       10000000000_ui64) return 10;
				if(mantissa <      100000000000_ui64) return 11;
				if(mantissa <     1000000000000_ui64) return 12;
				if(mantissa <    10000000000000_ui64) return 13;
				if(mantissa <   100000000000000_ui64) return 14;
				if(mantissa <  1000000000000000_ui64) return 15;
				if(mantissa < 10000000000000000_ui64) return 16;
				return 17;
			}
		};

	} //namespace


	template<>
	fp_base_classify to_chars_shortest_classify<float>(float value, fp_to_chars_shortest_context<float>& context)
	{
		using fp_type = float;
		using fp_utils_t = fp_utils<fp_type>;
		using uint_t = fp_utils_t::uint_t;
		using exp_st = fp_utils_t::exp_st;
		//using exp_ut = fp_utils_t::exp_ut;

		const uint_t exponent_bits = fp_utils_t::get_exponent_bits(value);
		const uint_t mantissa_bits = fp_utils_t::get_mantissa(value);
		const bool sign_bit        = fp_utils_t::get_sign(value);

		if(exponent_bits == fp_utils_t::exponent_mask)
		{ //nan or inf
			if(mantissa_bits)
			{ //nan
				return fp_base_classify{.classification=fp_classify::nan};
			} //else inf
			return fp_base_classify{.classification=fp_classify::inf, .is_negative = sign_bit};
		} // else number

		exp_st exponent;
		uint_t mantissa = mantissa_bits;

		if(exponent_bits)
		{	//normal
			exponent = static_cast<exp_st>(exponent_bits >> fp_utils_t::exponent_offset) - fp_utils_t::exponent_fix_bias;
			mantissa |= fp_utils_t::mantissa_implicit_bit;
		}
		else
		{	//denormal
			if(mantissa_bits == 0)
			{ //Zero
				return fp_base_classify{.classification = fp_classify::zero, .is_negative = sign_bit};
			}
			exponent = 1 - fp_utils_t::exponent_fix_bias;
		}

		// We subtract 2 so that the bounds computation has 2 additional bits.
		const exp_st   e2 = exponent - 2;
		const uint32_t m2 = mantissa;

		const bool acceptBounds = (m2 & 1) == 0; //even

		// Step 2: Determine the interval of valid decimal representations.
		const uint32_t mv = 4 * m2;
		const uint32_t mp = 4 * m2 + 2;

		// Implicit bool -> int conversion. True is 1, false is 0.
		const uint32_t mmShift = mantissa_bits != 0 || exponent_bits <= 1;
		const uint32_t mm = mv - 1 - mmShift;


		// Step 3: Convert to a decimal power base using 64-bit arithmetic.
		uint32_t vr, vp, vm;
		int16_t  e10;
		bool     vmIsTrailingZeros = false;
		bool     vrIsTrailingZeros = false;
		uint8_t  lastRemovedDigit  = 0;
		const int16_t ne2 = -e2;

		if(e2 >= 0)
		{
			const uint16_t q = log10Pow2(e2);
			const uint16_t k = FLOAT_POW5_INV_BITCOUNT + pow5bits(q) - 1;
			const uint8_t  i = static_cast<uint8_t>(ne2 + static_cast<int16_t>(q + k));

			e10 = static_cast<int16_t>(q);

			vr = mulPow5InvDivPow2(mv, q, i);
			vp = mulPow5InvDivPow2(mp, q, i);
			vm = mulPow5InvDivPow2(mm, q, i);

			if(q != 0 && (vp - 1) / 10 <= vm / 10)
			{
				// We need to know one removed digit even if we are not going to loop below. We could use
				// q = X - 1 above, except that would require 33 bits for the result, and we've found that
				// 32-bit arithmetic is faster even on 64-bit machines.
				const uint16_t l = static_cast<uint16_t>(FLOAT_POW5_INV_BITCOUNT + pow5bits(q - 1) - 1);
				lastRemovedDigit = static_cast<uint8_t>(mulPow5InvDivPow2(mv, q - 1, static_cast<uint8_t>(ne2 + q - 1 + l)) % 10);
			}
			if(q <= 9)
			{
				// The largest power of 5 that fits in 24 bits is 5^10, but q <= 9 seems to be safe as well.
				// Only one of mp, mv, and mm can be a multiple of 5, if any.
				if(mv % 5 == 0)
				{
					vrIsTrailingZeros = multipleOfPowerOf5_32(mv, q);
				}
				else if(acceptBounds)
				{
					vmIsTrailingZeros = multipleOfPowerOf5_32(mm, q);
				}
				else
				{
					vp -= multipleOfPowerOf5_32(mp, q);
				}
			}
		}
		else
		{
			const uint16_t q = log10Pow5(ne2);
			const uint16_t i = static_cast<uint16_t>(ne2 - q);
			const int16_t  k = static_cast<int16_t>(pow5bits(i) - FLOAT_POW5_BITCOUNT);
			uint8_t        j = static_cast<uint8_t>(static_cast<int16_t>(q) - k);

			e10 = static_cast<int16_t>(q) + e2;

			vr = mulPow5divPow2(mv, i, j);
			vp = mulPow5divPow2(mp, i, j);
			vm = mulPow5divPow2(mm, i, j);

			if(q != 0 && (vp - 1) / 10 <= vm / 10)
			{
				j				 = static_cast<uint8_t>(q - 1 - (pow5bits(i + 1) - FLOAT_POW5_BITCOUNT));
				lastRemovedDigit = static_cast<uint8_t>(mulPow5divPow2(mv, (uint32_t) (i + 1), j) % 10);
			}
			if(q <= 1)
			{
				// {vr,vp,vm} is trailing zeros if {mv,mp,mm} has at least q trailing 0 bits.
				// mv = 4 * m2, so it always has at least two trailing 0 bits.
				vrIsTrailingZeros = true;
				if(acceptBounds)
				{
					// mm = mv - 1 - mmShift, so it has 1 trailing 0 bit iff mmShift == 1.
					vmIsTrailingZeros = (mmShift == 1);
				}
				else
				{
					// mp = mv + 2, so it always has at least one trailing 0 bit.
					--vp;
				}
			}
			else if(q < 31)
			{ // TODO(ulfjack): Use a tighter bound here.
				vrIsTrailingZeros = multipleOfPowerOf2_32(mv, q - 1);
			}
		}

		// Step 4: Find the shortest decimal representation in the interval of valid representations.
		uint32_t output;
		if(vmIsTrailingZeros || vrIsTrailingZeros)
		{
			uint32_t vp_10 = vp / 10;
			uint32_t vm_10 = vm / 10;
			while(vp_10 > vm_10)
			{
				vmIsTrailingZeros &= vm % 10 == 0;
				vrIsTrailingZeros &= lastRemovedDigit == 0;
				lastRemovedDigit = static_cast<uint8_t>(vr % 10);

				vp = vp_10;
				vm = vm_10;

				vp_10 /= 10;
				vm_10 /= 10;
				vr /= 10;
				++e10;
			}


			// General case, which happens rarely (~4.0%).
			//while(vp / 10 > vm / 10)
			//{
			//	vmIsTrailingZeros &= vm % 10 == 0;
			//	vrIsTrailingZeros &= lastRemovedDigit == 0;
			//	lastRemovedDigit = static_cast<uint8_t>(vr % 10);
			//	vr /= 10;
			//	vp /= 10;
			//	vm /= 10;
			//	++e10;
			//}

			if(vmIsTrailingZeros)
			{
				while(vm % 10 == 0)
				{
					vm /= 10;
					vrIsTrailingZeros &= lastRemovedDigit == 0;
					lastRemovedDigit = static_cast<uint8_t>(vr % 10);
					vr /= 10;
					vp /= 10;
					++e10;
				}
			}

			if(vrIsTrailingZeros && lastRemovedDigit == 5 && vr % 2 == 0)
			{
				// Round even if the exact number is .....50..0.
				lastRemovedDigit = 4;
			}
			// We need to take vr + 1 if vr is outside bounds or we need to round up.
			output = vr + ((vr == vm && (!acceptBounds || !vmIsTrailingZeros)) || lastRemovedDigit >= 5);
		}
		else
		{
			uint32_t vp_10 = vp / 10;
			uint32_t vm_10 = vm / 10;
			while(vp_10 > vm_10)
			{
				lastRemovedDigit = static_cast<uint8_t>(vr % 10);
				vr /= 10;
				vp = vp_10;
				vm = vm_10;
				vp_10 /= 10;
				vm_10 /= 10;
				++e10;
			}

			// Specialized for the common case (~96.0%). Percentages below are relative to this.
			// Loop iterations below (approximately):
			// 0: 13.6%, 1: 70.7%, 2: 14.1%, 3: 1.39%, 4: 0.14%, 5+: 0.01%
			//while(vp / 10 > vm / 10)
			//{
			//	lastRemovedDigit = static_cast<uint8_t>(vr % 10);
			//	vr /= 10;
			//	vp /= 10;
			//	vm /= 10;
			//	++e10;
			//}

			// We need to take vr + 1 if vr is outside bounds or we need to round up.
			output = vr + (vr == vm || lastRemovedDigit >= 5);
		}

		context.exponent = static_cast<int16_t>(e10);
		context.mantissa = output;
		context.sig_digits = fp_utils_t::sig_digits(context.mantissa);

		return fp_base_classify{.classification=fp_classify::finite, .is_negative = sign_bit};
	}


	template<>
	fp_base_classify to_chars_shortest_classify<double>(double value, fp_to_chars_shortest_context<double>& context)
	{
		using fp_type = double;
		using fp_utils_t = fp_utils<fp_type>;
		using uint_t = fp_utils_t::uint_t;
		using exp_st = fp_utils_t::exp_st;
		//using exp_ut = fp_utils_t::exp_ut;

		const uint_t exponent_bits = fp_utils_t::get_exponent_bits(value);
		const uint_t mantissa_bits = fp_utils_t::get_mantissa(value);
		const bool sign_bit        = fp_utils_t::get_sign(value);

		if(exponent_bits == fp_utils_t::exponent_mask)
		{ //nan or inf
			if(mantissa_bits)
			{ //nan
				return fp_base_classify{.classification=fp_classify::nan};
			} //else inf
			return fp_base_classify{.classification=fp_classify::inf, .is_negative = sign_bit};
		} // else number

		exp_st exponent;
		uint_t mantissa = mantissa_bits;

		if(exponent_bits)
		{	//normal
			exponent = static_cast<exp_st>(exponent_bits >> fp_utils_t::exponent_offset) - fp_utils_t::exponent_fix_bias;
			mantissa |= fp_utils_t::mantissa_implicit_bit;
		}
		else
		{	//denormal
			if(mantissa_bits == 0)
			{ //Zero
				return fp_base_classify{.classification = fp_classify::zero, .is_negative = sign_bit};
			}
			exponent = 1 - fp_utils_t::exponent_fix_bias;
		}

		// We subtract 2 so that the bounds computation has 2 additional bits.
		const exp_st  e2 = exponent - 2;
		const uint64_t m2 = mantissa;

		const bool acceptBounds = (m2 & 1) == 0; //even

		// Step 2: Determine the interval of valid decimal representations.
		const uint64_t mv = 4 * m2;
		// Implicit bool -> int conversion. True is 1, false is 0.
		const uint8_t mmShift = mantissa_bits != 0 || exponent_bits <= 1;
		// We would compute mp and mm like this:
		// uint64_t mp = 4 * m2 + 2;
		// uint64_t mm = mv - 1 - mmShift;

		// Step 3: Convert to a decimal power base using 128-bit arithmetic.
		uint64_t vr, vp, vm;
		int16_t e10;
		bool vmIsTrailingZeros = false;
		bool vrIsTrailingZeros = false;
		const int16_t ne2 = -e2;

		if(e2 >= 0)
		{
			// I tried special-casing q == 0, but there was no effect on performance.
			// This expression is slightly faster than max(0, log10Pow2(e2) - 1).
			const uint16_t q = log10Pow2(e2) - (e2 > 3);
			const uint16_t k = DOUBLE_POW5_INV_BITCOUNT + pow5bits(q) - 1;
			const uint8_t  i = static_cast<uint8_t>(ne2 + static_cast<int16_t>(q + k));

			e10 = static_cast<int16_t>(q);

#if defined(RYU_OPTIMIZE_SIZE)
			uint64_t pow5[2];
			double_computeInvPow5(q, pow5);
			vr = mulShiftAll64(m2, pow5, i, &vp, &vm, mmShift);
#else
			vr = mulShiftAll64(m2, DOUBLE_POW5_INV_SPLIT[q], i, &vp, &vm, mmShift);
#endif

			if(q <= 21)
			{
				// This should use q <= 22, but I think 21 is also safe. Smaller values
				// may still be safe, but it's more difficult to reason about them.
				// Only one of mp, mv, and mm can be a multiple of 5, if any.
				const uint32_t mvMod5 = static_cast<uint32_t>(mv % 5);
				if(mvMod5 == 0)
				{
					vrIsTrailingZeros = multipleOfPowerOf5(mv, q);
				}
				else if(acceptBounds)
				{
					// Same as min(e2 + (~mm & 1), pow5Factor(mm)) >= q
					// <=> e2 + (~mm & 1) >= q && pow5Factor(mm) >= q
					// <=> true && pow5Factor(mm) >= q, since e2 >= q.
					vmIsTrailingZeros = multipleOfPowerOf5(mv - 1 - mmShift, q);
				}
				else
				{
					// Same as min(e2 + 1, pow5Factor(mp)) >= q.
					vp -= multipleOfPowerOf5(mv + 2, q);
				}
			}
		}
		else
		{
			// This expression is slightly faster than max(0, log10Pow5(-e2) - 1).
			const uint16_t q = log10Pow5(static_cast<uint16_t>(ne2)) - (ne2 > 1);
			const uint16_t i = static_cast<uint16_t>(ne2 - q);
			const int16_t  k = pow5bits(i) - DOUBLE_POW5_BITCOUNT;
			const uint8_t  j = static_cast<uint8_t>(static_cast<int16_t>(q) - k);

			e10 = static_cast<int16_t>(q) + e2;

#if defined(RYU_OPTIMIZE_SIZE)
			uint64_t pow5[2];
			double_computePow5(i, pow5);
			vr = mulShiftAll64(m2, pow5, j, &vp, &vm, mmShift);
#else
			vr = mulShiftAll64(m2, DOUBLE_POW5_SPLIT[i], j, &vp, &vm, mmShift);
#endif

			if(q <= 1)
			{
				// {vr,vp,vm} is trailing zeros if {mv,mp,mm} has at least q trailing 0 bits.
				// mv = 4 * m2, so it always has at least two trailing 0 bits.
				vrIsTrailingZeros = true;
				if(acceptBounds)
				{
					// mm = mv - 1 - mmShift, so it has 1 trailing 0 bit iff mmShift == 1.
					vmIsTrailingZeros = mmShift == 1;
				}
				else
				{
					// mp = mv + 2, so it always has at least one trailing 0 bit.
					--vp;
				}
			}
			else if(q < 63)
			{ // TODO(ulfjack): Use a tighter bound here.
			  // We want to know if the full product has at least q trailing zeros.
			  // We need to compute min(p2(mv), p5(mv) - e2) >= q
			  // <=> p2(mv) >= q && p5(mv) - e2 >= q
			  // <=> p2(mv) >= q (because -e2 >= q)
				vrIsTrailingZeros = multipleOfPowerOf2(mv, static_cast<uint8_t>(q));
			}
		}

		// Step 4: Find the shortest decimal representation in the interval of valid representations.
		uint8_t	 lastRemovedDigit = 0;
		uint64_t output;
		// On average, we remove ~2 digits.
		if(vmIsTrailingZeros || vrIsTrailingZeros)
		{
			// General case, which happens rarely (~0.7%).
			for(;;)
			{
				const uint64_t vpDiv10 = vp / 10;
				const uint64_t vmDiv10 = vm / 10;
				if(vpDiv10 <= vmDiv10)
				{
					break;
				}
				const uint32_t vmMod10 = static_cast<uint32_t>(vm % 10);
				const uint64_t vrDiv10 = vr / 10;
				const uint32_t vrMod10 = static_cast<uint32_t>(vr % 10);
				vmIsTrailingZeros &= vmMod10 == 0;
				vrIsTrailingZeros &= lastRemovedDigit == 0;
				lastRemovedDigit = static_cast<uint8_t>(vrMod10);
				vr				 = vrDiv10;
				vp				 = vpDiv10;
				vm				 = vmDiv10;
				++e10;
			}

			if(vmIsTrailingZeros)
			{
				for(;;)
				{
					const uint64_t vmDiv10 = vm / 10;
					const uint32_t vmMod10 = static_cast<uint32_t>(vm % 10);
					if(vmMod10 != 0)
					{
						break;
					}
					const uint64_t vpDiv10 = vp / 10;
					const uint64_t vrDiv10 = vr / 10;
					const uint32_t vrMod10 = static_cast<uint32_t>(vr % 10);
					vrIsTrailingZeros &= lastRemovedDigit == 0;
					lastRemovedDigit = static_cast<uint8_t>(vrMod10);
					vr				 = vrDiv10;
					vp				 = vpDiv10;
					vm				 = vmDiv10;
					++e10;
				}
			}

			if(vrIsTrailingZeros && lastRemovedDigit == 5 && vr % 2 == 0)
			{
				// Round even if the exact number is .....50..0.
				lastRemovedDigit = 4;
			}
			// We need to take vr + 1 if vr is outside bounds or we need to round up.
			output = vr + ((vr == vm && (!acceptBounds || !vmIsTrailingZeros)) || lastRemovedDigit >= 5);
		}
		else
		{
			// Specialized for the common case (~99.3%). Percentages below are relative to this.
			bool		   roundUp	= false;
			const uint64_t vpDiv100 = vp / 100;
			const uint64_t vmDiv100 = vm / 100;
			if(vpDiv100 > vmDiv100)
			{ // Optimization: remove two digits at a time (~86.2%).
				const uint64_t vrDiv100 = vr / 100;
				roundUp					= vr % 100 >= 50;
				vr						= vrDiv100;
				vp						= vpDiv100;
				vm						= vmDiv100;
				e10 += 2;
			}
			// Loop iterations below (approximately), without optimization above:
			// 0: 0.03%, 1: 13.8%, 2: 70.6%, 3: 14.0%, 4: 1.40%, 5: 0.14%, 6+: 0.02%
			// Loop iterations below (approximately), with optimization above:
			// 0: 70.6%, 1: 27.8%, 2: 1.40%, 3: 0.14%, 4+: 0.02%
			for(;;)
			{
				const uint64_t vpDiv10 = vp / 10;
				const uint64_t vmDiv10 = vm / 10;
				if(vpDiv10 <= vmDiv10)
				{
					break;
				}
				const uint64_t vrDiv10 = vr / 10;
				roundUp				   = vr % 10 >= 5;
				vr					   = vrDiv10;
				vp					   = vpDiv10;
				vm					   = vmDiv10;
				++e10;
			}

			// We need to take vr + 1 if vr is outside bounds or we need to round up.
			output = vr + (vr == vm || roundUp);
		}

		context.exponent = static_cast<int16_t>(e10);
		context.mantissa = output;
		context.sig_digits = fp_utils_t::sig_digits(context.mantissa);

		return fp_base_classify{.classification=fp_classify::finite, .is_negative = sign_bit};
	}


}

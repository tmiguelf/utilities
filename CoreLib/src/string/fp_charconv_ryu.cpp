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

#include <CoreLib/string/core_fp_charconv.hpp>

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
		struct fp_utils<float32_t>: public fp_utils_pre<float32_t>
		{
			[[nodiscard]] static inline constexpr uint8_t sig_digits(uint32_t const mantissa)
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
		struct fp_utils<float64_t>: public fp_utils_pre<float64_t>
		{
			[[nodiscard]] static inline constexpr uint8_t sig_digits(uint64_t const mantissa)
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
	fp_base_classify to_chars_shortest_classify<float32_t>(float32_t const value, fp_to_chars_shortest_context<float32_t>& context)
	{
		using fp_type = float32_t;
		using fp_utils_t = fp_utils<fp_type>;
		using uint_t = fp_utils_t::uint_t;
		using exp_st = fp_utils_t::exp_st;
		//using exp_ut = fp_utils_t::exp_ut;

		uint_t const exponent_bits = fp_utils_t::get_exponent_bits(value);
		uint_t const mantissa_bits = fp_utils_t::get_mantissa(value);
		bool   const sign_bit      = fp_utils_t::get_sign(value);

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
		exp_st const   e2 = exponent - 2;
		uint32_t const m2 = mantissa;

		bool const acceptBounds = (m2 & 1) == 0; //even

		// Step 2: Determine the interval of valid decimal representations.
		uint32_t const m_md = 4 * m2;
		uint32_t const m_hi = m_md + 2;

		// Implicit bool -> int conversion. True is 1, false is 0.
		uint32_t const mmShift = mantissa_bits != 0 || exponent_bits <= 1;
		uint32_t const m_lo = m_md - 1 - mmShift;


		// Step 3: Convert to a decimal power base using 64-bit arithmetic.
		uint32_t v_md, v_hi, v_lo;
		int16_t  e10;
		bool     vloIsTrailingZeros = false;
		bool     vmdIsTrailingZeros = false;
		uint8_t  lastRemovedDigit  = 0;
		int16_t const ne2 = -e2;

		if(e2 >= 0)
		{
			uint16_t const q = log10Pow2(e2);
			uint16_t const k = FLOAT_POW5_INV_BITCOUNT + pow5bits(q) - 1;
			uint8_t  const i = static_cast<uint8_t>(ne2 + static_cast<int16_t>(q + k));

			e10 = static_cast<int16_t>(q);

			v_md = mulPow5InvDivPow2(m_md, q, i);
			v_hi = mulPow5InvDivPow2(m_hi, q, i);
			v_lo = mulPow5InvDivPow2(m_lo, q, i);

			if(q != 0 && (v_hi - 1) / 10 <= v_lo / 10)
			{
				// We need to know one removed digit even if we are not going to loop below. We could use
				// q = X - 1 above, except that would require 33 bits for the result, and we've found that
				// 32-bit arithmetic is faster even on 64-bit machines.
				uint16_t const l = static_cast<uint16_t>(FLOAT_POW5_INV_BITCOUNT + pow5bits(q - 1) - 1);
				lastRemovedDigit = static_cast<uint8_t>(mulPow5InvDivPow2(m_md, q - 1, static_cast<uint8_t>(ne2 + q - 1 + l)) % 10);
			}
			if(q <= 9)
			{
				// The largest power of 5 that fits in 24 bits is 5^10, but q <= 9 seems to be safe as well.
				// Only one of m_hi, m_md, and m_lo can be a multiple of 5, if any.
				if(m_md % 5 == 0)
				{
					vmdIsTrailingZeros = multipleOfPowerOf5_32(m_md, q);
				}
				else if(acceptBounds)
				{
					vloIsTrailingZeros = multipleOfPowerOf5_32(m_lo, q);
				}
				else
				{
					v_hi -= multipleOfPowerOf5_32(m_hi, q);
				}
			}
		}
		else
		{
			uint16_t const q = log10Pow5(ne2);
			uint16_t const i = static_cast<uint16_t>(ne2 - q);
			int16_t  const k = static_cast<int16_t>(pow5bits(i) - FLOAT_POW5_BITCOUNT);
			uint8_t        j = static_cast<uint8_t>(static_cast<int16_t>(q) - k);

			e10 = static_cast<int16_t>(q) + e2;

			v_md = mulPow5divPow2(m_md, i, j);
			v_hi = mulPow5divPow2(m_hi, i, j);
			v_lo = mulPow5divPow2(m_lo, i, j);

			if(q != 0 && (v_hi - 1) / 10 <= v_lo / 10)
			{
				j				 = static_cast<uint8_t>(q - 1 - (pow5bits(i + 1) - FLOAT_POW5_BITCOUNT));
				lastRemovedDigit = static_cast<uint8_t>(mulPow5divPow2(m_md, i + 1, j) % 10);
			}
			if(q <= 1)
			{
				// {v_md,v_hi,v_lo} is trailing zeros if {m_md,m_hi,m_lo} has at least q trailing 0 bits.
				// m_md = 4 * m2, so it always has at least two trailing 0 bits.
				vmdIsTrailingZeros = true;
				if(acceptBounds)
				{
					// m_lo = m_md - 1 - mmShift, so it has 1 trailing 0 bit iff mmShift == 1.
					vloIsTrailingZeros = (mmShift == 1);
				}
				else
				{
					// m_hi = m_md + 2, so it always has at least one trailing 0 bit.
					--v_hi;
				}
			}
			else if(q < 31)
			{ // TODO(ulfjack): Use a tighter bound here.
				vmdIsTrailingZeros = multipleOfPowerOf2_32(m_md, q - 1);
			}
		}

		// Step 4: Find the shortest decimal representation in the interval of valid representations.
		uint32_t output;

		if(vloIsTrailingZeros || vmdIsTrailingZeros)
		{
			// General case, which happens rarely (~4.0%).
			uint32_t vp_10 = v_hi / 10;
			uint32_t vm_10 = v_lo / 10;
			while(vp_10 > vm_10)
			{
				vloIsTrailingZeros &= v_lo % 10 == 0;
				v_lo = vm_10;
				vm_10 /= 10;

				vmdIsTrailingZeros &= lastRemovedDigit == 0;
				lastRemovedDigit = static_cast<uint8_t>(v_md % 10);
				v_md /= 10;
				vp_10 /= 10;

				++e10;
			}

			//while(v_hi / 10 > v_lo / 10)
			//{
			//	vloIsTrailingZeros &= v_lo % 10 == 0;
			//	vmdIsTrailingZeros &= lastRemovedDigit == 0;
			//	lastRemovedDigit = static_cast<uint8_t>(v_md % 10);
			//	v_md /= 10;
			//	v_hi /= 10;
			//	v_lo /= 10;
			//	++e10;
			//}

			if(vloIsTrailingZeros)
			{
				while(v_lo % 10 == 0)
				{
					v_lo /= 10;
					vmdIsTrailingZeros &= lastRemovedDigit == 0;
					lastRemovedDigit = static_cast<uint8_t>(v_md % 10);
					v_md /= 10;
					++e10;
				}
			}

			if(vmdIsTrailingZeros && lastRemovedDigit == 5 && (v_md & 1) == 0)
			{
				// Round even if the exact number is .....50..0.
				lastRemovedDigit = 4;
			}
			// We need to take v_md + 1 if v_md is outside bounds or we need to round up.
			output = v_md + ((v_md == v_lo && (!acceptBounds || !vloIsTrailingZeros)) || lastRemovedDigit >= 5);
		}
		else
		{

			// Specialized for the common case (~96.0%). Percentages below are relative to this.
			// Loop iterations below (approximately):
			// 0: 13.6%, 1: 70.7%, 2: 14.1%, 3: 1.39%, 4: 0.14%, 5+: 0.01%
			uint32_t vp_10 = v_hi / 10;
			uint32_t vm_10 = v_lo / 10;
			while(vp_10 > vm_10)
			{
				lastRemovedDigit = static_cast<uint8_t>(v_md % 10);
				v_md /= 10;
				v_lo = vm_10;
				vm_10 /= 10;
				vp_10 /= 10;
				++e10;
			}

			//while(v_hi / 10 > v_lo / 10)
			//{
			//	lastRemovedDigit = static_cast<uint8_t>(v_md % 10);
			//	v_md /= 10;
			//	v_hi /= 10;
			//	v_lo /= 10;
			//	++e10;
			//}

			// We need to take v_md + 1 if v_md is outside bounds or we need to round up.
			output = v_md + (v_md == v_lo || lastRemovedDigit >= 5);
		}

		context.exponent = static_cast<int16_t>(e10);
		context.mantissa = output;
		context.sig_digits = fp_utils_t::sig_digits(context.mantissa);

		return fp_base_classify{.classification=fp_classify::finite, .is_negative = sign_bit};
	}


	template<>
	fp_base_classify to_chars_shortest_classify<float64_t>(float64_t const value, fp_to_chars_shortest_context<float64_t>& context)
	{
		using fp_type = float64_t;
		using fp_utils_t = fp_utils<fp_type>;
		using uint_t = fp_utils_t::uint_t;
		using exp_st = fp_utils_t::exp_st;
		//using exp_ut = fp_utils_t::exp_ut;

		uint_t const exponent_bits = fp_utils_t::get_exponent_bits(value);
		uint_t const mantissa_bits = fp_utils_t::get_mantissa(value);
		bool const   sign_bit      = fp_utils_t::get_sign(value);

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
		exp_st   const e2 = exponent - 2;
		uint64_t const m2 = mantissa;

		bool const acceptBounds = (m2 & 1) == 0; //even

		// Step 2: Determine the interval of valid decimal representations.
		uint64_t const m_md = 4 * m2;
		// Implicit bool -> int conversion. True is 1, false is 0.
		uint8_t const mmShift = mantissa_bits != 0 || exponent_bits <= 1;
		// We would compute m_hi and m_lo like this:
		// uint64_t m_hi = 4 * m2 + 2;
		// uint64_t m_lo = m_md - 1 - mmShift;

		// Step 3: Convert to a decimal power base using 128-bit arithmetic.
		uint64_t v_md, v_hi, v_lo;
		exp_st e10;
		bool vloIsTrailingZeros = false;
		bool vmdIsTrailingZeros = false;
		int16_t const ne2 = -e2;

		if(e2 >= 0)
		{
			// I tried special-casing q == 0, but there was no effect on performance.
			// This expression is slightly faster than max(0, log10Pow2(e2) - 1).
			uint16_t const q = static_cast<uint16_t>(log10Pow2(static_cast<uint16_t>(e2)) - (e2 > 3));
			uint16_t const k = DOUBLE_POW5_INV_BITCOUNT + pow5bits(q) - 1;
			uint8_t  const i = static_cast<uint8_t>(ne2 + static_cast<int16_t>(q + k));

			e10 = static_cast<int16_t>(q);
			v_md = mulShiftAll64(m2, DOUBLE_POW5_INV_SPLIT[q], i, v_hi, v_lo, mmShift);

			if(q <= 21)
			{
				// This should use q <= 22, but I think 21 is also safe. Smaller values
				// may still be safe, but it's more difficult to reason about them.
				// Only one of m_hi, m_md, and m_lo can be a multiple of 5, if any.
				uint32_t const mvMod5 = static_cast<uint32_t>(m_md % 5);
				if(mvMod5 == 0)
				{
					vmdIsTrailingZeros = multipleOfPowerOf5(m_md, q);
				}
				else if(acceptBounds)
				{
					// Same as min(e2 + (~m_lo & 1), pow5Factor(m_lo)) >= q
					// <=> e2 + (~m_lo & 1) >= q && pow5Factor(m_lo) >= q
					// <=> true && pow5Factor(m_lo) >= q, since e2 >= q.
					vloIsTrailingZeros = multipleOfPowerOf5(m_md - 1 - mmShift, q);
				}
				else
				{
					// Same as min(e2 + 1, pow5Factor(m_hi)) >= q.
					v_hi -= multipleOfPowerOf5(m_md + 2, q);
				}
			}
		}
		else
		{
			// This expression is slightly faster than max(0, log10Pow5(-e2) - 1).
			uint16_t const q = static_cast<uint16_t>(log10Pow5(static_cast<uint16_t>(ne2)) - (ne2 > 1));
			uint16_t const i = static_cast<uint16_t>(ne2 - q);
			int16_t  const k = static_cast<int16_t>(pow5bits(i) - DOUBLE_POW5_BITCOUNT);
			uint8_t  const j = static_cast<uint8_t>(static_cast<int16_t>(q) - k);

			e10 = static_cast<int16_t>(q) + e2;
			v_md = mulShiftAll64(m2, DOUBLE_POW5_SPLIT[i], j, v_hi, v_lo, mmShift);

			if(q <= 1)
			{
				// {v_md,v_hi,v_lo} is trailing zeros if {m_md,m_hi,m_lo} has at least q trailing 0 bits.
				// m_md = 4 * m2, so it always has at least two trailing 0 bits.
				vmdIsTrailingZeros = true;
				if(acceptBounds)
				{
					// m_lo = m_md - 1 - mmShift, so it has 1 trailing 0 bit iff mmShift == 1.
					vloIsTrailingZeros = mmShift == 1;
				}
				else
				{
					// m_hi = m_md + 2, so it always has at least one trailing 0 bit.
					--v_hi;
				}
			}
			else if(q < 63)
			{ // TODO(ulfjack): Use a tighter bound here.
			  // We want to know if the full product has at least q trailing zeros.
			  // We need to compute min(p2(m_md), p5(m_md) - e2) >= q
			  // <=> p2(m_md) >= q && p5(m_md) - e2 >= q
			  // <=> p2(m_md) >= q (because -e2 >= q)
				vmdIsTrailingZeros = multipleOfPowerOf2(m_md, static_cast<uint8_t>(q));
			}
		}

		// Step 4: Find the shortest decimal representation in the interval of valid representations.
		uint8_t	 lastRemovedDigit = 0;
		uint64_t output;
		// On average, we remove ~2 digits.
		if(vloIsTrailingZeros || vmdIsTrailingZeros)
		{
			// General case, which happens rarely (~0.7%).
			for(;;)
			{
				uint64_t const vpDiv10 = v_hi / 10;
				uint64_t const vmDiv10 = v_lo / 10;
				if(vpDiv10 <= vmDiv10)
				{
					break;
				}
				uint32_t const vmMod10 = static_cast<uint32_t>(v_lo % 10);
				uint64_t const vrDiv10 = v_md / 10;
				uint32_t const vrMod10 = static_cast<uint32_t>(v_md % 10);
				vloIsTrailingZeros &= vmMod10 == 0;
				vmdIsTrailingZeros &= lastRemovedDigit == 0;
				lastRemovedDigit = static_cast<uint8_t>(vrMod10);
				v_md				 = vrDiv10;
				v_hi				 = vpDiv10;
				v_lo				 = vmDiv10;
				++e10;
			}

			if(vloIsTrailingZeros)
			{
				for(;;)
				{
					uint64_t const vmDiv10 = v_lo / 10;
					uint32_t const vmMod10 = static_cast<uint32_t>(v_lo % 10);
					if(vmMod10 != 0)
					{
						break;
					}
					uint64_t const vpDiv10 = v_hi / 10;
					uint64_t const vrDiv10 = v_md / 10;
					uint32_t const vrMod10 = static_cast<uint32_t>(v_md % 10);
					vmdIsTrailingZeros &= lastRemovedDigit == 0;
					lastRemovedDigit = static_cast<uint8_t>(vrMod10);
					v_md			 = vrDiv10;
					v_hi			 = vpDiv10;
					v_lo			 = vmDiv10;
					++e10;
				}
			}

			if(vmdIsTrailingZeros && lastRemovedDigit == 5 && (v_md & 1) == 0)
			{
				// Round even if the exact number is .....50..0.
				lastRemovedDigit = 4;
			}
			// We need to take v_md + 1 if v_md is outside bounds or we need to round up.
			output = v_md + ((v_md == v_lo && (!acceptBounds || !vloIsTrailingZeros)) || lastRemovedDigit >= 5);
		}
		else
		{
			// Specialized for the common case (~99.3%). Percentages below are relative to this.
			bool		   roundUp	= false;
			uint64_t const vpDiv100 = v_hi / 100;
			uint64_t const vmDiv100 = v_lo / 100;
			if(vpDiv100 > vmDiv100)
			{ // Optimization: remove two digits at a time (~86.2%).
				uint64_t const vrDiv100 = v_md / 100;
				roundUp					= v_md % 100 >= 50;
				v_md					= vrDiv100;
				v_hi					= vpDiv100;
				v_lo					= vmDiv100;
				e10 += 2;
			}
			// Loop iterations below (approximately), without optimization above:
			// 0: 0.03%, 1: 13.8%, 2: 70.6%, 3: 14.0%, 4: 1.40%, 5: 0.14%, 6+: 0.02%
			// Loop iterations below (approximately), with optimization above:
			// 0: 70.6%, 1: 27.8%, 2: 1.40%, 3: 0.14%, 4+: 0.02%
			for(;;)
			{
				uint64_t const vpDiv10 = v_hi / 10;
				uint64_t const vmDiv10 = v_lo / 10;
				if(vpDiv10 <= vmDiv10)
				{
					break;
				}
				uint64_t const vrDiv10 = v_md / 10;
				roundUp				   = v_md % 10 >= 5;
				v_md				   = vrDiv10;
				v_hi				   = vpDiv10;
				v_lo				   = vmDiv10;
				++e10;
			}

			// We need to take v_md + 1 if v_md is outside bounds or we need to round up.
			output = v_md + (v_md == v_lo || roundUp);
		}

		context.exponent = static_cast<int16_t>(e10);
		context.mantissa = output;
		context.sig_digits = fp_utils_t::sig_digits(context.mantissa);

		return fp_base_classify{.classification=fp_classify::finite, .is_negative = sign_bit};
	}


}

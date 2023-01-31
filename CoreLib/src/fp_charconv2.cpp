//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///
///	\copyright
///		Copyright (c) Tiago Miguel Oliveira Freire
///
///		Permission is hereby granted, free of charge, to any person obtaining a copy
///		of this software and associated documentation files (the "Software"), to deal
///		in the Software without restriction, including without limitation the rights
///		to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
///		copies of the Software, and to permit persons to whom the Software is
///		furnished to do so, subject to the following conditions:
///
///		The above copyright notice and this permission notice shall be included in all
///		copies or substantial portions of the Software.
///
///		THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///		IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///		FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///		AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
///		LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///		OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
///		SOFTWARE.
//======== ======== ======== ======== ======== ======== ======== ========

#include <CoreLib/fp_charconv.hpp>

#include <CoreLib/Core_Type.hpp>
#include <algorithm>

#include "ryu/common.hpp"
#include "ryu/f2s_intrinsics.hpp"
#include "ryu/d2s_intrinsics.hpp"

//#define USE_ORDER_REDUCE 1

namespace core
{
	using ::core::literals::operator "" _ui64;
	using ::core::literals::operator "" _ui32;
	using ::core::literals::operator "" _ui16;
	using ::core::literals::operator "" _i16;
	using ::core::literals::operator "" _ui8;


	struct fp_common_props
	{
		using exp_st = int16_t;
		using exp_ut = uint16_t;
	};

	template<typename T>
	struct fp_props_pre;

	template<>
	struct fp_props_pre<float>: public fp_type_traits<float>, public fp_common_props
	{
		using fp_type = float;
		using uint_t = uint32_t;

		static constexpr uint_t exponent_mask	= 0x7F800000_ui32;
		static constexpr uint_t exponent_offset	= 23_ui32;
		static constexpr uint_t mantissa_mask	= 0x007FFFFF_ui32;
		static constexpr uint_t sign_mask		= 0x80000000_ui32;

		static constexpr exp_st exponent_bias	= 0x7F_i16;
		static constexpr uint_t mantissa_bits	= 23_ui32;

		static constexpr exp_st exponent_fix_bias	= exponent_bias + mantissa_bits;
		static constexpr uint_t mantissa_implicit_bit = 1_ui32 << mantissa_bits;

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

		static inline constexpr exp_ut exp_digits_size(exp_st exp)
		{
			if(exp == 0) return 0;
			if(exp < 0)
			{
				exp = -exp;
			}
			return (exp < 10) ? 1 : 2;
		}
	};

	template<>
	struct fp_props_pre<double>: public fp_type_traits<double>, public fp_common_props
	{
		using fp_type = double;
		using uint_t = uint64_t;

		static constexpr uint_t exponent_mask	= 0x7FF0000000000000_ui64;
		static constexpr uint_t exponent_offset	= 52_ui64;
		static constexpr uint_t mantissa_mask	= 0x000FFFFFFFFFFFFF_ui64;
		static constexpr uint_t sign_mask		= 0x8000000000000000_ui64;

		static constexpr exp_st exponent_bias	= 0x3FF_i16;
		static constexpr uint_t mantissa_bits	= 52_ui64;

		static constexpr exp_st exponent_fix_bias	= exponent_bias + mantissa_bits;
		static constexpr uint_t mantissa_implicit_bit = 1_ui64 << mantissa_bits;

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

		static inline constexpr exp_ut exp_digits_size(exp_st exp)
		{
			if(exp == 0) return 0;
			if(exp < 0)
			{
				exp = -exp;
			}
			if(exp < 10) return 1;
			if(exp < 100) return 2;
			return 3;
		}
	};


	template<typename fp_type>
	struct fp_props: public fp_props_pre<fp_type>
	{
		using fp_props_p = fp_props_pre<fp_type>;
		using uint_t = typename fp_props_p::uint_t;
		using exp_st = typename fp_props_p::exp_st;
		using exp_ut = typename fp_props_p::exp_ut;


		static inline uint_t get_mantissa(fp_type input)
		{
			return reinterpret_cast<const uint_t&>(input) & fp_props_p::mantissa_mask;
		}

		static inline uint_t get_exponent_bits(fp_type input)
		{
			return (reinterpret_cast<const uint_t&>(input) & fp_props_p::exponent_mask);
		}

		static inline bool get_sign(const fp_type input)
		{
			return reinterpret_cast<const uint_t&>(input) & fp_props_p::sign_mask;
		}

		static inline constexpr fp_to_chars_sci_size sci_size(const uint8_t sig_digits, const exp_st ryu_exp)
		{
			const exp_st sci_exp = static_cast<exp_st>(sig_digits + ryu_exp - 1);
			return fp_to_chars_sci_size
			{
				.mantissa_decimal_size = static_cast<uint16_t>(sig_digits -1),
				.exponent_size = fp_props_p::exp_digits_size(sci_exp),
				.is_exp_negative = sci_exp < 0
			};
		}

		static inline constexpr fp_to_chars_fix_size fix_size(const uint8_t sig_digits, const exp_st ryu_exp)
		{
			if(ryu_exp >= 0)
			{
				return fp_to_chars_fix_size
				{
					.unit_size = static_cast<uint16_t>(sig_digits + ryu_exp),
					.decimal_size = 0
				};
			}

			exp_st unit_digits = static_cast<exp_st>(sig_digits) + ryu_exp;

			return fp_to_chars_fix_size
			{
				.unit_size = (unit_digits > 0) ? static_cast<uint16_t>(unit_digits) : 0_ui16,
				.decimal_size = static_cast<uint16_t>(-ryu_exp)
			};
		}
	};






	template<>
	fp_base_classify to_chars_shortest_classify<float>(float value, fp_to_chars_shortest_context<float>& context)
	{
		using fp_type = float;
		using fp_props_t = fp_props<fp_type>;
		using uint_t = fp_props_t::uint_t;
		using exp_st = fp_props_t::exp_st;
		using exp_ut = fp_props_t::exp_ut;

		const uint_t exponent_bits = fp_props_t::get_exponent_bits(value);
		const uint_t mantissa_bits = fp_props_t::get_mantissa(value);
		const bool sign_bit        = fp_props_t::get_sign(value);

		if(exponent_bits == fp_props_t::exponent_mask)
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
			exponent = static_cast<exp_st>(exponent_bits >> fp_props_t::exponent_offset) - fp_props_t::exponent_fix_bias;
			mantissa |= fp_props_t::mantissa_implicit_bit;
		}
		else
		{	//denormal
			if(mantissa == 0)
			{ //Zero
				return fp_base_classify{.classification = fp_classify::zero, .is_negative = sign_bit};
			}
			exponent = 1 - fp_props_t::exponent_fix_bias;
		}

#if USE_ORDER_REDUCE
		{
			exp_st offset = static_cast<exp_st>(std::countr_zero(mantissa));
			mantissa >>= offset;
			exponent += offset;
		}
#endif

		// We subtract 2 so that the bounds computation has 2 additional bits.
		int32_t  e2 = exponent - 2;
		uint32_t m2 = mantissa;

		const bool even			= (m2 & 1) == 0;
		const bool acceptBounds = even;

		// Step 2: Determine the interval of valid decimal representations.
		const uint32_t mv = 4 * m2;
		const uint32_t mp = 4 * m2 + 2;
		// Implicit bool -> int conversion. True is 1, false is 0.
		const uint32_t mmShift = mantissa_bits != 0 || exponent_bits <= 1;
		const uint32_t mm	   = 4 * m2 - 1 - mmShift;


		// Step 3: Convert to a decimal power base using 64-bit arithmetic.
		uint32_t vr, vp, vm;
		int32_t	 e10;
		bool	 vmIsTrailingZeros = false;
		bool	 vrIsTrailingZeros = false;
		uint8_t	 lastRemovedDigit  = 0;


		if(e2 >= 0)
		{
			const uint32_t q = log10Pow2(e2);
			e10				 = (int32_t) q;
			const int32_t k	 = FLOAT_POW5_INV_BITCOUNT + pow5bits(q) - 1;
			const int32_t i	 = -e2 + (int32_t) q + k;
			vr				 = mulPow5InvDivPow2(mv, q, i);
			vp				 = mulPow5InvDivPow2(mp, q, i);
			vm				 = mulPow5InvDivPow2(mm, q, i);

			if(q != 0 && (vp - 1) / 10 <= vm / 10)
			{
				// We need to know one removed digit even if we are not going to loop below. We could use
				// q = X - 1 above, except that would require 33 bits for the result, and we've found that
				// 32-bit arithmetic is faster even on 64-bit machines.
				const int32_t l	 = FLOAT_POW5_INV_BITCOUNT + pow5bits(q - 1) - 1;
				lastRemovedDigit = (uint8_t) (mulPow5InvDivPow2(mv, q - 1, -e2 + q - 1 + l) % 10);
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
			const uint32_t q = log10Pow5(-e2);
			e10				 = (int32_t) q + e2;
			const int32_t i	 = -e2 - (int32_t) q;
			const int32_t k	 = pow5bits(i) - FLOAT_POW5_BITCOUNT;
			int32_t		  j	 = (int32_t) q - k;
			vr				 = mulPow5divPow2(mv, (uint32_t) i, j);
			vp				 = mulPow5divPow2(mp, (uint32_t) i, j);
			vm				 = mulPow5divPow2(mm, (uint32_t) i, j);

			if(q != 0 && (vp - 1) / 10 <= vm / 10)
			{
				j				 = (int32_t) q - 1 - (pow5bits(i + 1) - FLOAT_POW5_BITCOUNT);
				lastRemovedDigit = (uint8_t) (mulPow5divPow2(mv, (uint32_t) (i + 1), j) % 10);
			}
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
			else if(q < 31)
			{ // TODO(ulfjack): Use a tighter bound here.
				vrIsTrailingZeros = multipleOfPowerOf2_32(mv, q - 1);
			}
		}

		// Step 4: Find the shortest decimal representation in the interval of valid representations.
		int32_t	 removed = 0;
		uint32_t output;
		if(vmIsTrailingZeros || vrIsTrailingZeros)
		{
			// General case, which happens rarely (~4.0%).
			while(vp / 10 > vm / 10)
			{
				vmIsTrailingZeros &= vm % 10 == 0;
				vrIsTrailingZeros &= lastRemovedDigit == 0;
				lastRemovedDigit = (uint8_t) (vr % 10);
				vr /= 10;
				vp /= 10;
				vm /= 10;
				++removed;
			}

			if(vmIsTrailingZeros)
			{
				while(vm % 10 == 0)
				{
					vrIsTrailingZeros &= lastRemovedDigit == 0;
					lastRemovedDigit = (uint8_t) (vr % 10);
					vr /= 10;
					vp /= 10;
					vm /= 10;
					++removed;
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
			// Specialized for the common case (~96.0%). Percentages below are relative to this.
			// Loop iterations below (approximately):
			// 0: 13.6%, 1: 70.7%, 2: 14.1%, 3: 1.39%, 4: 0.14%, 5+: 0.01%
			while(vp / 10 > vm / 10)
			{
				lastRemovedDigit = (uint8_t) (vr % 10);
				vr /= 10;
				vp /= 10;
				vm /= 10;
				++removed;
			}

			// We need to take vr + 1 if vr is outside bounds or we need to round up.
			output = vr + (vr == vm || lastRemovedDigit >= 5);
		}
		const int32_t exp = e10 + removed;

		context.exponent = static_cast<int16_t>(exp);
		context.mantissa = output;
		context.sig_digits = fp_props_t::sig_digits(context.mantissa);

		return fp_base_classify{.classification=fp_classify::finite, .is_negative = sign_bit};
	}




	template<>
	fp_base_classify to_chars_shortest_classify<double>(double value, fp_to_chars_shortest_context<double>& context)
	{
		using fp_type = double;
		using fp_props_t = fp_props<fp_type>;
		using uint_t = fp_props_t::uint_t;
		using exp_st = fp_props_t::exp_st;
		using exp_ut = fp_props_t::exp_ut;

		const uint_t exponent_bits = fp_props_t::get_exponent_bits(value);
		const uint_t mantissa_bits = fp_props_t::get_mantissa(value);
		const bool sign_bit        = fp_props_t::get_sign(value);

		if(exponent_bits == fp_props_t::exponent_mask)
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
			exponent = static_cast<exp_st>(exponent_bits >> fp_props_t::exponent_offset) - fp_props_t::exponent_fix_bias;
			mantissa |= fp_props_t::mantissa_implicit_bit;
		}
		else
		{	//denormal
			if(mantissa == 0)
			{ //Zero
				return fp_base_classify{.classification = fp_classify::zero, .is_negative = sign_bit};
			}
			exponent = 1 - fp_props_t::exponent_fix_bias;
		}

#if USE_ORDER_REDUCE
		{
			exp_st offset = static_cast<exp_st>(std::countr_zero(mantissa));
			mantissa >>= offset;
			exponent += offset;
		}
#endif

		// We subtract 2 so that the bounds computation has 2 additional bits.
		int32_t  e2 = exponent - 2;
		uint64_t m2 = mantissa;

		const bool even			= (m2 & 1) == 0;
		const bool acceptBounds = even;

		// Step 2: Determine the interval of valid decimal representations.
		const uint64_t mv = 4 * m2;
		// Implicit bool -> int conversion. True is 1, false is 0.
		const uint32_t mmShift = mantissa_bits != 0 || mantissa_bits <= 1;
		// We would compute mp and mm like this:
		// uint64_t mp = 4 * m2 + 2;
		// uint64_t mm = mv - 1 - mmShift;

		// Step 3: Convert to a decimal power base using 128-bit arithmetic.
		uint64_t vr, vp, vm;
		int32_t	 e10;
		bool	 vmIsTrailingZeros = false;
		bool	 vrIsTrailingZeros = false;
		if(e2 >= 0)
		{
			// I tried special-casing q == 0, but there was no effect on performance.
			// This expression is slightly faster than max(0, log10Pow2(e2) - 1).
			const uint32_t q = log10Pow2(e2) - (e2 > 3);
			e10				 = (int32_t) q;
			const int32_t k	 = DOUBLE_POW5_INV_BITCOUNT + pow5bits(q) - 1;
			const int32_t i	 = -e2 + (int32_t) q + k;

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
				const uint32_t mvMod5 = static_cast<uint32_t>(mv) - 5 * static_cast<uint32_t>(mv / 5);
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
			const uint32_t q = log10Pow5(static_cast<uint32_t>(-e2)) - (-e2 > 1);
			e10				 = (int32_t) q + e2;
			const int32_t i	 = -e2 - (int32_t) q;
			const int32_t k	 = pow5bits(i) - DOUBLE_POW5_BITCOUNT;
			const int32_t j	 = (int32_t) q - k;

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
				vrIsTrailingZeros = multipleOfPowerOf2(mv, q);
			}
		}

		// Step 4: Find the shortest decimal representation in the interval of valid representations.
		int32_t	 removed		  = 0;
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
				const uint32_t vmMod10 = ((uint32_t) vm) - 10 * ((uint32_t) vmDiv10);
				const uint64_t vrDiv10 = vr / 10;
				const uint32_t vrMod10 = ((uint32_t) vr) - 10 * ((uint32_t) vrDiv10);
				vmIsTrailingZeros &= vmMod10 == 0;
				vrIsTrailingZeros &= lastRemovedDigit == 0;
				lastRemovedDigit = (uint8_t) vrMod10;
				vr				 = vrDiv10;
				vp				 = vpDiv10;
				vm				 = vmDiv10;
				++removed;
			}

			if(vmIsTrailingZeros)
			{
				for(;;)
				{
					const uint64_t vmDiv10 = vm / 10;
					const uint32_t vmMod10 = ((uint32_t) vm) - 10 * ((uint32_t) vmDiv10);
					if(vmMod10 != 0)
					{
						break;
					}
					const uint64_t vpDiv10 = vp / 10;
					const uint64_t vrDiv10 = vr / 10;
					const uint32_t vrMod10 = ((uint32_t) vr) - 10 * ((uint32_t) vrDiv10);
					vrIsTrailingZeros &= lastRemovedDigit == 0;
					lastRemovedDigit = (uint8_t) vrMod10;
					vr				 = vrDiv10;
					vp				 = vpDiv10;
					vm				 = vmDiv10;
					++removed;
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
				const uint32_t vrMod100 = ((uint32_t) vr) - 100 * ((uint32_t) vrDiv100);
				roundUp					= vrMod100 >= 50;
				vr						= vrDiv100;
				vp						= vpDiv100;
				vm						= vmDiv100;
				removed += 2;
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
				const uint32_t vrMod10 = ((uint32_t) vr) - 10 * ((uint32_t) vrDiv10);
				roundUp				   = vrMod10 >= 5;
				vr					   = vrDiv10;
				vp					   = vpDiv10;
				vm					   = vmDiv10;
				++removed;
			}

			// We need to take vr + 1 if vr is outside bounds or we need to round up.
			output = vr + (vr == vm || roundUp);
		}
		const int32_t exp = e10 + removed;

		context.exponent = static_cast<int16_t>(exp);
		context.mantissa = output;
		context.sig_digits = fp_props_t::sig_digits(context.mantissa);

		return fp_base_classify{.classification=fp_classify::finite, .is_negative = sign_bit};
	}





	template<>
	fp_to_chars_sci_size to_chars_shortest_sci_size<float>(fp_to_chars_shortest_context<float> context)
	{
		using fp_type = float;
		using fp_props_t = fp_props<fp_type>;
		return fp_props_t::sci_size(context.sig_digits, context.exponent);
	}

	template<>
	fp_to_chars_sci_size to_chars_shortest_sci_size<double>(fp_to_chars_shortest_context<double> context)
	{
		using fp_type = double;
		using fp_props_t = fp_props<fp_type>;
		return fp_props_t::sci_size(context.sig_digits, context.exponent);
	}


	template<>
	fp_to_chars_fix_size to_chars_shortest_fix_size<float>(fp_to_chars_shortest_context<float> context)
	{
		using fp_type = float;
		using fp_props_t = fp_props<fp_type>;
		return fp_props_t::fix_size(context.sig_digits, context.exponent);
	}

	template<>
	fp_to_chars_fix_size to_chars_shortest_fix_size<double>(fp_to_chars_shortest_context<double> context)
	{
		using fp_type = double;
		using fp_props_t = fp_props<fp_type>;
		return fp_props_t::fix_size(context.sig_digits, context.exponent);
	}


	template<typename fp_t, typename char_t>
	void to_chars_shortest_sci_unsafe(fp_to_chars_shortest_context<fp_t> context, char_t* unit_char, char_t* decimal_chars)
	{
		using fp_props_t = fp_props<fp_t>;
		using uint_t = fp_props_t::uint_t;

		uint_t mantissa = context.mantissa;
		uint8_t dec_digits = context.sig_digits;
		char_t* pivot = decimal_chars + dec_digits - 1;
		while(--dec_digits);
		{
			*(--pivot) = static_cast<char_t>('0' + mantissa % 10);
			mantissa /= 10;
		}
		*unit_char = static_cast<char_t>('0' + mantissa);
	}

	template<typename fp_t, typename char_t>
	void to_chars_shortest_sci_exp_unsafe(fp_to_chars_shortest_context<fp_t> context, char_t* exp_chars)
	{
		using fp_props_p = fp_props_pre<fp_t>;
		using exp_st = typename fp_props_p::exp_st;
		using exp_ut = typename fp_props_p::exp_ut;

		const exp_st sci_exp = static_cast<exp_st>(context.sig_digits + context.exponent - 1);
		exp_ut exp = static_cast<exp_ut>((sci_exp < 0) ? -sci_exp: sci_exp);

		exp_ut digits_size = fp_props_p::exp_digits_size(exp);

		exp_chars += digits_size;
		while(exp)
		{
			*(--exp_chars) = static_cast<char_t>('0' + exp % 10);
			exp /= 10;
		}
	}

	template<typename fp_t, typename char_t>
	void to_chars_shortest_fix_unsafe(fp_to_chars_shortest_context<fp_t> context, char_t* unit_chars, char_t* decimal_chars)
	{
		using fp_props_p = fp_props_pre<fp_t>;
		using exp_st = typename fp_props_p::exp_st;
		using exp_ut = typename fp_props_p::exp_ut;
		using uint_t = typename fp_props_p::uint_t;

		exp_st ryu_exp = context.exponent;
		uint_t mantissa = context.mantissa;

		if(ryu_exp >= 0)
		{
			unit_chars += ryu_exp + context.sig_digits;
			do
			{
				*(--unit_chars) = static_cast<char_t>('0' + mantissa % 10);
				mantissa /= 10;
			}
			while(mantissa);
			while(ryu_exp--)
			{
				*(--unit_chars) = char_t{'0'};
			}
		}
		else
		{
			exp_st unit_digits = static_cast<exp_st>(context.sig_digits) + ryu_exp;
			if(unit_digits > 0)
			{
				exp_ut decimal_digits = static_cast<exp_ut>(-ryu_exp);
				decimal_chars += decimal_digits;
				unit_chars += unit_digits;

				do
				{
					*(--decimal_chars) = static_cast<char_t>('0' + mantissa % 10);
					mantissa /= 10;
				}
				while(--decimal_digits);

				do
				{
					*(--unit_chars) = static_cast<char_t>('0' + mantissa % 10);
					mantissa /= 10;
				}
				while(mantissa);
			}
			else
			{
				decimal_chars += ryu_exp;
				do
				{
					*(--decimal_chars) = static_cast<char_t>('0' + mantissa % 10);
					mantissa /= 10;
				}
				while(mantissa);

				while(unit_digits++)
				{
					*(--decimal_chars) = char_t{'0'};
				}
			}
		}
	}


	template void to_chars_shortest_sci_unsafe<float , char8_t >(fp_to_chars_shortest_context<float >, char8_t *, char8_t *);
	template void to_chars_shortest_sci_unsafe<float , char16_t>(fp_to_chars_shortest_context<float >, char16_t*, char16_t*);
	template void to_chars_shortest_sci_unsafe<float , char32_t>(fp_to_chars_shortest_context<float >, char32_t*, char32_t*);
	template void to_chars_shortest_sci_unsafe<double, char8_t >(fp_to_chars_shortest_context<double>, char8_t *, char8_t *);
	template void to_chars_shortest_sci_unsafe<double, char16_t>(fp_to_chars_shortest_context<double>, char16_t*, char16_t*);
	template void to_chars_shortest_sci_unsafe<double, char32_t>(fp_to_chars_shortest_context<double>, char32_t*, char32_t*);

	template void to_chars_shortest_sci_exp_unsafe<float , char8_t >(fp_to_chars_shortest_context<float >, char8_t *);
	template void to_chars_shortest_sci_exp_unsafe<float , char16_t>(fp_to_chars_shortest_context<float >, char16_t*);
	template void to_chars_shortest_sci_exp_unsafe<float , char32_t>(fp_to_chars_shortest_context<float >, char32_t*);
	template void to_chars_shortest_sci_exp_unsafe<double, char8_t >(fp_to_chars_shortest_context<double>, char8_t *);
	template void to_chars_shortest_sci_exp_unsafe<double, char16_t>(fp_to_chars_shortest_context<double>, char16_t*);
	template void to_chars_shortest_sci_exp_unsafe<double, char32_t>(fp_to_chars_shortest_context<double>, char32_t*);

	template void to_chars_shortest_fix_unsafe<float , char8_t >(fp_to_chars_shortest_context<float >, char8_t *, char8_t *);
	template void to_chars_shortest_fix_unsafe<float , char16_t>(fp_to_chars_shortest_context<float >, char16_t*, char16_t*);
	template void to_chars_shortest_fix_unsafe<float , char32_t>(fp_to_chars_shortest_context<float >, char32_t*, char32_t*);
	template void to_chars_shortest_fix_unsafe<double, char8_t >(fp_to_chars_shortest_context<double>, char8_t *, char8_t *);
	template void to_chars_shortest_fix_unsafe<double, char16_t>(fp_to_chars_shortest_context<double>, char16_t*, char16_t*);
	template void to_chars_shortest_fix_unsafe<double, char32_t>(fp_to_chars_shortest_context<double>, char32_t*, char32_t*);
}

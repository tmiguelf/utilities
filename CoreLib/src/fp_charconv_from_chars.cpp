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

#include <algorithm>


#include <CoreLib/string/core_string_numeric.hpp>

#include "ryu/common.hpp"
#include "ryu/f2s_intrinsics.hpp"
#include "ryu/d2s_intrinsics.hpp"

#include "fp_traits.hpp"

namespace core
{
	template<typename char_t>
	static inline constexpr bool is_all_num(const std::basic_string_view<char_t> p_string)
	{
		for(char_t character : p_string)
		{
			if(!is_digit(character)) return false;
		}
		return true;
	}

	static inline uint8_t floor_log2(const uint32_t value)
	{
		return static_cast<uint8_t>(31 - std::countl_zero(value));
	}

	static inline uint8_t floor_log2(const uint64_t value)
	{
		return static_cast<uint8_t>(63 - std::countl_zero(value));
	}

	static void from_chars_b10_to_b2(bool sign_bit, typename fp_utils_pre<float>::uint_t m10, typename fp_utils_pre<float>::exp_st e10, float& result)
	{
		using fp_t = float;
		using fp_utils_t = fp_utils_pre<fp_t>;
		using uint_t = fp_utils_t::uint_t;
		using exp_st = fp_utils_t::exp_st;

		constexpr uint8_t mantisssa_bits = fp_utils_t::mantissa_bits;

		// Convert to binary float m2 * 2^e2, while retaining information about whether the conversion
		// was exact (trailingZeros).
		exp_st e2;
		uint_t m2;
		bool trailingZeros;
		if(e10 >= 0)
		{
			// The length of m * 10^e in bits is:
			//   log2(m10 * 10^e10) = log2(m10) + e10 log2(10) = log2(m10) + e10 + e10 * log2(5)
			//
			// We want to compute the mantisssa_bits + 1 top-most bits (+1 for the implicit leading
			// one in IEEE format). We therefore choose a binary output exponent of
			//   log2(m10 * 10^e10) - (mantisssa_bits + 1).
			//
			// We use floor(log2(5^e10)) so that we get at least this many bits; better to
			// have an additional bit than to not have enough bits.
			e2 = static_cast<exp_st>(floor_log2(m10) + e10 + log2pow5(e10) - (mantisssa_bits + 1));

			// We now compute [m10 * 10^e10 / 2^e2] = [m10 * 5^e10 / 2^(e2-e10)].
			// To that end, we use the FLOAT_POW5_SPLIT table.
			const uint8_t j = static_cast<uint8_t>(e2 - e10 - ceil_log2pow5(e10) + FLOAT_POW5_BITCOUNT);
			m2 = mulPow5divPow2(m10, static_cast<uint16_t>(e10), j);

			// We also compute if the result is exact, i.e.,
			//   [m10 * 10^e10 / 2^e2] == m10 * 10^e10 / 2^e2.
			// This can only be the case if 2^e2 divides m10 * 10^e10, which in turn requires that the
			// largest power of 2 that divides m10 + e10 is greater than e2. If e2 is less than e10, then
			// the result must be exact. Otherwise we use the existing multipleOfPowerOf2 function.
			trailingZeros = e2 < e10 || (e2 - e10 < 32 && multipleOfPowerOf2_32(m10, e2 - e10));
		}
		else
		{
			e2 = static_cast<exp_st>(floor_log2(m10) + e10 - ceil_log2pow5(-e10) - (mantisssa_bits + 1));

			// We now compute [m10 * 10^e10 / 2^e2] = [m10 / (5^(-e10) 2^(e2-e10))].
			const uint8_t j = static_cast<uint8_t>(e2 - e10 + ceil_log2pow5(-e10) - 1 + FLOAT_POW5_INV_BITCOUNT);
			m2	  = mulPow5InvDivPow2(m10, static_cast<uint16_t>(-e10), j);

			// We also compute if the result is exact, i.e.,
			//   [m10 / (5^(-e10) 2^(e2-e10))] == m10 / (5^(-e10) 2^(e2-e10))
			//
			// If e2-e10 >= 0, we need to check whether (5^(-e10) 2^(e2-e10)) divides m10, which is the
			// case iff pow5(m10) >= -e10 AND pow2(m10) >= e2-e10.
			//
			// If e2-e10 < 0, we have actually computed [m10 * 2^(e10 e2) / 5^(-e10)] above,
			// and we need to check whether 5^(-e10) divides (m10 * 2^(e10-e2)), which is the case iff
			// pow5(m10 * 2^(e10-e2)) = pow5(m10) >= -e10.
			trailingZeros = (e2 < e10 || (e2 - e10 < 32 && multipleOfPowerOf2_32(m10, e2 - e10))) && multipleOfPowerOf5_32(m10, -e10);
		}


		exp_st e2_base = e2 + fp_utils_t::exponent_bias + floor_log2(m2);

		if(e2_base > 0xFE)
		{
			// Final IEEE exponent is larger than the maximum representable; return +/-Infinity.
			const uint_t t_resut = sign_bit ? (fp_utils_t::sign_mask | fp_utils_t::exponent_mask) : fp_utils_t::exponent_mask;
			result = reinterpret_cast<const fp_t&>(t_resut);
			return;
		}
		if(e2_base < 0)
		{
			e2_base = 0;
		}
		// Compute the final IEEE exponent.
		uint_t ieee_e2 = static_cast<uint_t>(e2_base);


		// We need to figure out how much we need to shift m2. The tricky part is that we need to take
		// the final IEEE exponent into account, so we need to reverse the bias and also special-case
		// the value 0.
		uint32_t shift = (ieee_e2 == 0 ? 1 : ieee_e2) - e2 - fp_utils_t::exponent_bias - mantisssa_bits;

		// We need to round up if the exact value is more than 0.5 above the value we computed. That's
		// equivalent to checking if the last removed bit was 1 and either the value was not just
		// trailing zeros or the result would otherwise be odd.
		//
		// We need to update trailingZeros given that we have the exact output exponent ieee_e2 now.
		trailingZeros &= ((m2 & ((1u << (shift - 1)) - 1)) == 0);
		bool lastRemovedBit = ((m2 >> (shift - 1)) & 1) ? true : false;
		bool roundUp = lastRemovedBit && (!trailingZeros || ((m2 >> shift) & 1));


		uint32_t ieee_m2 = (m2 >> shift) + roundUp;
		assert(ieee_m2 <= (1u << (mantisssa_bits + 1)));
		ieee_m2 &= (1u << mantisssa_bits) - 1;
		if(ieee_m2 == 0 && roundUp)
		{
			// Rounding up may overflow the mantissa.
			// In this case we move a trailing zero of the mantissa into the exponent.
			// Due to how the IEEE represents +/-Infinity, we don't need to check for overflow here.
			ieee_e2++;
		}

		uint_t t_resut = ieee_m2 | static_cast<uint_t>(ieee_e2) << fp_utils_t::exponent_offset;
		if(sign_bit) t_resut |= fp_utils_t::sign_mask;
		result		  = reinterpret_cast<const fp_t&>(t_resut);
	}

	static void from_chars_b10_to_b2(bool sign_bit, typename fp_utils_pre<double>::uint_t m10, typename fp_utils_pre<double>::exp_st e10, double& result)
	{
		using fp_t = double;
		using fp_utils_t = fp_utils_pre<fp_t>;
		using uint_t = fp_utils_t::uint_t;
		using exp_st = fp_utils_t::exp_st;

		constexpr uint8_t mantisssa_bits = fp_utils_t::mantissa_bits;

		// Convert to binary float m2 * 2^e2, while retaining information about whether the conversion
		// was exact (trailingZeros).
		exp_st e2;
		uint_t m2;
		bool trailingZeros;
		if(e10 >= 0)
		{
			// The length of m * 10^e in bits is:
			//   log2(m10 * 10^e10) = log2(m10) + e10 log2(10) = log2(m10) + e10 + e10 * log2(5)
			//
			// We want to compute the mantisssa_bits + 1 top-most bits (+1 for the implicit leading
			// one in IEEE format). We therefore choose a binary output exponent of
			//   log2(m10 * 10^e10) - (mantisssa_bits + 1).
			//
			// We use floor(log2(5^e10)) so that we get at least this many bits; better to
			// have an additional bit than to not have enough bits.
			e2 = static_cast<exp_st>(floor_log2(m10) + e10 + log2pow5(e10) - (mantisssa_bits + 1));

			// We now compute [m10 * 10^e10 / 2^e2] = [m10 * 5^e10 / 2^(e2-e10)].
			// To that end, we use the DOUBLE_POW5_SPLIT table.
			const uint8_t j = static_cast<uint8_t>(e2 - e10 - ceil_log2pow5(e10) + DOUBLE_POW5_BITCOUNT);

			assert(e10 < DOUBLE_POW5_TABLE_SIZE);
			m2 = mulShift64(m10, DOUBLE_POW5_SPLIT[e10], j);

			// We also compute if the result is exact, i.e.,
			//   [m10 * 10^e10 / 2^e2] == m10 * 10^e10 / 2^e2.
			// This can only be the case if 2^e2 divides m10 * 10^e10, which in turn requires that the
			// largest power of 2 that divides m10 + e10 is greater than e2. If e2 is less than e10, then
			// the result must be exact. Otherwise we use the existing multipleOfPowerOf2 function.
			trailingZeros = e2 < e10 || (e2 - e10 < 64 && multipleOfPowerOf2(m10, static_cast<uint8_t>(e2 - e10)));
		}
		else
		{
			e2 = static_cast<exp_st>(floor_log2(m10) + e10 - ceil_log2pow5(-e10) - (mantisssa_bits + 1));
			const uint8_t j = static_cast<uint8_t>(e2 - e10 + ceil_log2pow5(-e10) - 1 + DOUBLE_POW5_INV_BITCOUNT);

			assert(-e10 < DOUBLE_POW5_INV_TABLE_SIZE);
			m2 = mulShift64(m10, DOUBLE_POW5_INV_SPLIT[-e10], j);

			trailingZeros = multipleOfPowerOf5(m10, -e10);
		}

		exp_st e2_base = e2 + fp_utils_t::exponent_bias + floor_log2(m2);


		if(e2_base > 0x7FE)
		{
			// Final IEEE exponent is larger than the maximum representable; return +/-Infinity.
			const uint_t t_resut = sign_bit ? (fp_utils_t::sign_mask | fp_utils_t::exponent_mask) : fp_utils_t::exponent_mask;
			result = reinterpret_cast<const fp_t&>(t_resut);
			return;
		}
		if(e2_base < 0)
		{
			e2_base = 0;
		}
		// Compute the final IEEE exponent.
		uint_t ieee_e2 = static_cast<uint_t>(e2_base);

		// We need to figure out how much we need to shift m2. The tricky part is that we need to take
		// the final IEEE exponent into account, so we need to reverse the bias and also special-case
		// the value 0.
		uint32_t shift = static_cast<uint32_t>((ieee_e2 == 0 ? 1 : ieee_e2) - e2 - fp_utils_t::exponent_bias - mantisssa_bits);

		// We need to round up if the exact value is more than 0.5 above the value we computed. That's
		// equivalent to checking if the last removed bit was 1 and either the value was not just
		// trailing zeros or the result would otherwise be odd.
		//
		// We need to update trailingZeros given that we have the exact output exponent ieee_e2 now.
		trailingZeros &= (m2 & ((1ull << (shift - 1)) - 1)) == 0;
		bool lastRemovedBit = ((m2 >> (shift - 1)) & 1) ? true : false;
		bool roundUp = lastRemovedBit && (!trailingZeros || ((m2 >> shift) & 1));

		uint64_t ieee_m2 = (m2 >> shift) + roundUp;
		assert(ieee_m2 <= (1ull << (mantisssa_bits + 1)));
		ieee_m2 &= (1ull << mantisssa_bits) - 1;
		if(ieee_m2 == 0 && roundUp)
		{
			// Due to how the IEEE represents +/-Infinity, we don't need to check for overflow here.
			ieee_e2++;
		}

		uint_t t_resut = ieee_m2 | static_cast<uint_t>(ieee_e2) << fp_utils_t::exponent_offset;
		if(sign_bit) t_resut |= fp_utils_t::sign_mask;
		result		  = reinterpret_cast<const fp_t&>(t_resut);
	}



	template<typename fp_t, typename char_t>
	bool from_chars(bool sign_bit, std::basic_string_view<char_t> units, std::basic_string_view<char_t> decimal, bool exp_negative, std::basic_string_view<char_t> exponent, fp_t& result)
	{
		using fp_utils_t = fp_utils_pre<fp_t>;
		using uint_t = fp_utils_t::uint_t;
		using exp_st = fp_utils_t::exp_st;

		constexpr uint8_t max_sig_digits_10 = fp_utils_t::max_shortest_digits_10; //+1;

		if(!is_all_num(units) || !is_all_num(decimal) || !is_all_num(exponent))
		{
			return false;
		}

		uint_t m10 = 0;
		uint8_t sig_digits = 0;

		intptr_t decimal_offset = 0;

		{
			const char_t* pivot = units.data();
			const char_t* const end = pivot + units.size();

			for(;pivot < end && *pivot == '0'; ++pivot);

			for(;pivot < end; ++pivot)
			{
				m10 = m10 * 10 + (*pivot - '0');

				if(++sig_digits == max_sig_digits_10)
				{
					decimal_offset = static_cast<intptr_t>(end - pivot - 1);
					goto $exp_parse;
				}
			}
		}

		{
			const char_t* pivot = decimal.data();
			const char_t*const  end = pivot + decimal.size();

			if(sig_digits == 0)
			{
				for(;pivot < end && *pivot == '0'; ++pivot);
			}

			for(;pivot < end; ++pivot)
			{
				m10 = m10 * 10 + (*pivot - '0');

				if(++sig_digits == max_sig_digits_10)
				{
					++pivot;
					break;
				}
			}

			decimal_offset = static_cast<intptr_t>(decimal.data() - pivot);
		}

		if(sig_digits == 0)
		{
			const uint_t t_resut = sign_bit ? fp_utils_t::sign_mask : uint_t{0};
			result = reinterpret_cast<const fp_t&>(t_resut);
			return true;
		}

	$exp_parse:
		exp_st e10;
		{
			intptr_t e_temp = 0;

			const char_t* pivot = exponent.data();
			const char_t* const end = pivot + exponent.size();

			for(;pivot < end && *pivot == '0'; ++pivot);

			for(;pivot < end; ++pivot)
			{
				e_temp = e_temp * 10 + (*pivot - '0');
			}

			if(exp_negative)
			{
				e_temp = -e_temp;
			}

			e_temp += decimal_offset;

			const intptr_t adjusted_e10 = e_temp + sig_digits;
			if(adjusted_e10 > fp_utils_t::max_scientific_exponent_10 + 1)
			{
				const uint_t t_resut = sign_bit ? (fp_utils_t::sign_mask | fp_utils_t::exponent_mask) : fp_utils_t::exponent_mask;
				result = reinterpret_cast<const fp_t&>(t_resut);
				return true;
			}
			if(adjusted_e10 < fp_utils_t::min_scientific_exponent_10)
			{
				const uint_t t_resut = sign_bit ? (fp_utils_t::sign_mask | fp_utils_t::exponent_mask) : fp_utils_t::exponent_mask;
				result = reinterpret_cast<const fp_t&>(t_resut);
				return true;
			}
			e10 = static_cast<exp_st>(e_temp);
		}

		from_chars_b10_to_b2(sign_bit, m10, e10, result);
		return true;
	}

	template bool from_chars<float , char8_t >(bool, std::basic_string_view<char8_t >, std::basic_string_view<char8_t >, bool, std::basic_string_view<char8_t >, float &);
	template bool from_chars<float , char16_t>(bool, std::basic_string_view<char16_t>, std::basic_string_view<char16_t>, bool, std::basic_string_view<char16_t>, float &);
	template bool from_chars<float , char32_t>(bool, std::basic_string_view<char32_t>, std::basic_string_view<char32_t>, bool, std::basic_string_view<char32_t>, float &);
	template bool from_chars<double, char8_t >(bool, std::basic_string_view<char8_t >, std::basic_string_view<char8_t >, bool, std::basic_string_view<char8_t >, double&);
	template bool from_chars<double, char16_t>(bool, std::basic_string_view<char16_t>, std::basic_string_view<char16_t>, bool, std::basic_string_view<char16_t>, double&);
	template bool from_chars<double, char32_t>(bool, std::basic_string_view<char32_t>, std::basic_string_view<char32_t>, bool, std::basic_string_view<char32_t>, double&);
}

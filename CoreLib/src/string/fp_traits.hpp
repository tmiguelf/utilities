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

#include <CoreLib/string/core_fp_charconv.hpp>
#include <CoreLib/Core_Type.hpp>

#include <bit>

namespace core
{
	using ::core::literals::operator "" _ui64;
	using ::core::literals::operator "" _ui32;
	using ::core::literals::operator "" _i16;
	using ::core::literals::operator "" _ui8;


	struct fp_common_traits
	{
		using exp_st = int16_t;
		using exp_ut = uint16_t;
	};

	template<typename T>
	struct fp_traits;


	template<>
	struct fp_traits<float32_t>: public fp_type_traits<float32_t>, public fp_common_traits
	{
		using fp_type = float32_t;
		using uint_t = uint32_t;

		static constexpr uint_t  sign_mask		= 0x80000000_ui32;
		static constexpr uint_t  exponent_mask	= 0x7F800000_ui32;
		static constexpr uint_t  mantissa_mask	= 0x007FFFFF_ui32;

		static constexpr exp_st  exponent_bias		= 0x7F_i16;
		static constexpr uint8_t mantissa_bits		= 23_ui8;
		static constexpr uint8_t exponent_offset	= mantissa_bits;

		static constexpr exp_st exponent_fix_bias	= exponent_bias + mantissa_bits;
		static constexpr uint_t mantissa_implicit_bit = 1_ui32 << mantissa_bits;
	};

	template<>
	struct fp_traits<float64_t>: public fp_type_traits<float64_t>, public fp_common_traits
	{
		using fp_type = float64_t;
		using uint_t = uint64_t;

		static constexpr uint_t sign_mask		= 0x8000000000000000_ui64;
		static constexpr uint_t exponent_mask	= 0x7FF0000000000000_ui64;
		static constexpr uint_t mantissa_mask	= 0x000FFFFFFFFFFFFF_ui64;

		static constexpr exp_st  exponent_bias		= 0x3FF_i16;
		static constexpr uint8_t mantissa_bits		= 52_ui8;
		static constexpr uint8_t exponent_offset	= mantissa_bits;

		static constexpr exp_st exponent_fix_bias	= exponent_bias + mantissa_bits;
		static constexpr uint_t mantissa_implicit_bit = 1_ui64 << mantissa_bits;
	};

	template<_p::charconv_fp_c fp_type>
	struct fp_utils_pre: public fp_traits<fp_type>
	{
		using fp_traits_t = fp_traits<fp_type>;
		using uint_t = fp_traits_t::uint_t;

		[[nodiscard]] static inline constexpr uint_t get_mantissa(const fp_type input)
		{
			return std::bit_cast<const uint_t>(input) & fp_traits_t::mantissa_mask;
		}

		[[nodiscard]] static inline constexpr uint_t get_exponent_bits(const fp_type input)
		{
			return (std::bit_cast<const uint_t>(input) & fp_traits_t::exponent_mask);
		}

		[[nodiscard]] static inline constexpr bool get_sign(const fp_type input)
		{
			return std::bit_cast<const uint_t>(input) & fp_traits_t::sign_mask;
		}
	};
}

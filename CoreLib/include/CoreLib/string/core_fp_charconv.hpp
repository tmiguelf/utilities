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

#pragma once

#include <cstdint>
#include <string_view>

#include "numeric_common.hpp"

namespace core
{

	template <_p::is_charconv_fp_supported_c T>
	struct fp_type_traits;

	template <>
	struct fp_type_traits<float>
	{
		static constexpr int16_t max_scientific_exponent_10 = 38;
		static constexpr int16_t min_scientific_exponent_10 = -45;

		static constexpr uint16_t max_scientific_decimal_digits_10 = 111;
		static constexpr uint16_t max_scientific_precision_10 = max_scientific_decimal_digits_10;

		static constexpr uint16_t max_scientific_exponent_digits_10 = 2;

		static constexpr uint16_t max_fixed_decimal_digits_10 = 149;
		static constexpr uint16_t max_fixed_unit_digits_10 = 39;

		static constexpr int16_t max_fixed_precision_10 = 149;
		static constexpr int16_t min_fixed_precision_10 = -38;

		static constexpr uint16_t max_shortest_digits_10 = 9;
	};

	template <>
	struct fp_type_traits<double>
	{
		static constexpr int16_t max_scientific_exponent_10 = 308;
		static constexpr int16_t min_scientific_exponent_10 = -324;

		static constexpr uint16_t max_scientific_decimal_digits_10 = 766;
		static constexpr uint16_t max_scientific_precision_10 = 766;

		static constexpr uint16_t max_scientific_exponent_digits_10 = 3;

		static constexpr uint16_t max_fixed_decimal_digits_10 = 1074;
		static constexpr uint16_t max_fixed_unit_digits_10 = 325;

		static constexpr int16_t max_fixed_precision_10 = 1074;
		static constexpr int16_t min_fixed_precision_10 = -324;

		static constexpr uint16_t max_shortest_digits_10 = 17;
	};

	enum class fp_classify: uint8_t
	{
		zero    = 0,
		finite  ,
		inf     ,
		nan     ,
	};

	struct fp_base_classify
	{
		fp_classify classification;
		bool is_negative;
	};

	struct fp_to_chars_fix_size
	{
		uint16_t unit_size;
		uint16_t decimal_size;
	};

	struct fp_to_chars_sci_size
	{
		uint16_t mantissa_decimal_size;
		uint16_t exponent_size;
		bool is_exp_negative;
	};


	template <class T>
	struct fp_to_chars_shortest_context;

	template <>
	struct fp_to_chars_shortest_context<float>
	{
		uint32_t mantissa;
		int16_t exponent;
		uint8_t sig_digits;
	};

	template <>
	struct fp_to_chars_shortest_context<double>
	{
		uint64_t mantissa;
		int16_t exponent;
		uint8_t sig_digits;
	};

	template<_p::is_charconv_fp_supported_c fp_t>
	fp_base_classify to_chars_shortest_classify(fp_t value, fp_to_chars_shortest_context<fp_t>& context);

	template<_p::is_charconv_fp_supported_c fp_t>
	[[nodiscard]] fp_to_chars_sci_size to_chars_shortest_sci_size(fp_to_chars_shortest_context<fp_t> context);

	template<_p::is_charconv_fp_supported_c fp_t>
	[[nodiscard]] fp_to_chars_fix_size to_chars_shortest_fix_size(fp_to_chars_shortest_context<fp_t> context);

	template<_p::is_charconv_fp_supported_c fp_t, _p::is_internal_charconv_c char_t>
	void to_chars_shortest_sci_unsafe(fp_to_chars_shortest_context<fp_t> context, char_t* unit_char, char_t* decimal_chars);

	template<_p::is_charconv_fp_supported_c fp_t, _p::is_internal_charconv_c char_t>
	void to_chars_shortest_sci_exp_unsafe(fp_to_chars_shortest_context<fp_t> context, char_t* exp_chars);

	template<_p::is_charconv_fp_supported_c fp_t, _p::is_internal_charconv_c char_t>
	void to_chars_shortest_fix_unsafe(fp_to_chars_shortest_context<fp_t> context, char_t* unit_chars, char_t* decimal_chars);

	template<_p::is_charconv_fp_supported_c fp_t, _p::is_internal_charconv_c char_t>
	bool from_chars(bool sign_bit, std::basic_string_view<char_t> units, std::basic_string_view<char_t> decimal, bool exp_negative, std::basic_string_view<char_t> exponent, fp_t& result);

} //namespace core

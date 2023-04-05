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
#include <charconv>
#include <array>
#include <cstdint>
#include <type_traits>
#include <string_view>

#include "core_fp_charconv.hpp"
#include "numeric_common.hpp"

namespace core
{
	enum class fp_round: uint8_t
	{
		nearest,
		to_zero,
		away_zero,
		to_inf,
		to_neg_inf,
		standard = nearest,
	};

	struct fp_to_chars_fix_result: public fp_base_classify
	{
		fp_to_chars_fix_size size;
	};

	struct fp_to_chars_sci_result: public fp_base_classify
	{
		fp_to_chars_sci_size size;
	};


	template <_p::charconv_fp_c T>
	struct fp_to_chars_round_context;

	template <>
	struct fp_to_chars_round_context<float32_t>
	{
		static constexpr uint8_t bignum_width = 6;
		using bignum_t = std::array<uint64_t, bignum_width>;
	};

	template <>
	struct fp_to_chars_round_context<float64_t>
	{
		static constexpr uint8_t bignum_width = 41;
		using bignum_t = std::array<uint64_t, bignum_width>;
	};


	template <_p::charconv_fp_c T>
	struct fp_to_chars_sci_context
	{
		fp_to_chars_round_context<T>::bignum_t digits;
		int16_t exponent;
	};

	template <_p::charconv_fp_c T>
	struct fp_to_chars_fix_context
	{
		fp_to_chars_round_context<T>::bignum_t digits;
		int16_t decimal_offset;
	};


	template<_p::charconv_fp_c fp_t>
	fp_to_chars_sci_result to_chars_sci_size(fp_t value, fp_to_chars_sci_context<fp_t>& context, uint16_t significant_digits, fp_round rounding_mode);

	template<_p::charconv_fp_c fp_t>
	fp_to_chars_fix_result to_chars_fix_size(fp_t value, fp_to_chars_fix_context<fp_t>& context, int16_t precision, fp_round rounding_mode);

	template<_p::charconv_fp_c fp_t, _p::charconv_char_c char_t>
	void to_chars_sci_mantissa_unsafe(const fp_to_chars_sci_context<fp_t>& context, char_t* unit_char, char_t* decimal_chars);
	
	template<_p::charconv_fp_c fp_t, _p::charconv_char_c char_t>
	void to_chars_sci_exp_unsafe(const fp_to_chars_sci_context<fp_t>& context, char_t* exp_chars);

	template<_p::charconv_fp_c fp_t, _p::charconv_char_c char_t>
	void to_chars_fix_unsafe(const fp_to_chars_fix_context<fp_t>& context, char_t* unit_chars, char_t* decimal_chars);

} //namespace core

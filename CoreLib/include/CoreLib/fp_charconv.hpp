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
#include <cstdint>
#include <type_traits>

namespace core
{
	template <class T>
	struct fp_type_traits;

	template <>
	struct fp_type_traits<float>
	{
		static constexpr int16_t max_scientific_exponent = 38;
		static constexpr int16_t min_scientific_exponent = -45;

		static constexpr uint16_t max_scientific_decimal_digits = 111;
		static constexpr uint16_t max_scientific_precision = max_scientific_decimal_digits;

		static constexpr uint16_t max_scientific_exponent_digits = 2;

		static constexpr uint16_t max_fixed_decimal_digits = 149;
		static constexpr uint16_t max_fixed_unit_digits = 39;

		static constexpr int16_t max_fixed_precision = 149;
		static constexpr int16_t min_fixed_precision = -38;



		static constexpr uint8_t bignum_width = 6;
		using bignum_t = std::array<uint64_t, bignum_width>;
		using exp_t = int16_t;


		static constexpr uint16_t max_shortest_digits = 9;
	};

	template <>
	struct fp_type_traits<double>
	{
		static constexpr int16_t max_scientific_exponent = 324;
		static constexpr int16_t min_scientific_exponent = -324;
		 
		static constexpr uint16_t max_scientific_decimal_digits = 766;
		static constexpr uint16_t max_scientific_precision = 766;

		static constexpr uint16_t max_scientific_exponent_digits = 3;

		static constexpr uint16_t max_fixed_decimal_digits = 1074;
		static constexpr uint16_t max_fixed_unit_digits = 325;
		static constexpr int16_t max_fixed_precision = 1074;
		static constexpr int16_t min_fixed_precision = -324;

		static constexpr uint8_t bignum_width = 41;
		using bignum_t = std::array<uint64_t, bignum_width>;
		using exp_t = int16_t;

		static constexpr uint16_t max_shortest_digits = 17;
	};



	enum class fp_classify: uint8_t
	{
		zero    = 0,
		finite  ,
		inf     ,
		nan     ,
	};

	enum class fp_round: uint8_t
	{
		nearest,
		to_zero,
		away_zero,
		to_inf,
		to_neg_inf,
		standard = nearest,
	};




#if 0
	//======== Type support ========
	namespace _p
	{
		template <typename T>
		concept is_internal_charconv_c =
			std::is_same_v<T, char8_t>  ||
			std::is_same_v<T, char16_t> ||
			std::is_same_v<T, char32_t>;

		template <typename T>
		concept is_supported_charconv_c =
			is_internal_charconv_c<T> ||
			std::is_same_v<T, char> ||
			std::is_same_v<T, wchar_t>;
	} //namespace _p
#endif



	struct fp_base_classify
	{
		fp_classify classification;
		bool is_negative;
	};








	struct fp_to_chars_fix_data
	{
		uint16_t unit_size;
		uint16_t decimal_size;
	};

	struct fp_to_chars_sci_data
	{
		uint16_t mantissa_decimal_size;
		uint16_t exponent_size;
		bool is_exp_negative;
	};

	struct fp_to_chars_fix_result: public fp_base_classify
	{
		fp_to_chars_fix_data size;
	};

	struct fp_to_chars_sci_result: public fp_base_classify
	{
		fp_to_chars_sci_data size;
	};

	struct fp_to_chars_shortest_result: public fp_base_classify
	{
		union
		{
			fp_to_chars_fix_data fix;
			fp_to_chars_sci_data sci;
		} size;
		bool is_fix;
	};


	template <class T>
	struct fp_to_chars_sci_context
	{
		fp_type_traits<T>::bignum_t digits;
		fp_type_traits<T>::exp_t exponent;
	};

	template <class T>
	struct fp_to_chars_fix_context
	{
		fp_type_traits<T>::bignum_t digits;
		int16_t decimal_offset;
	};

	fp_to_chars_sci_result to_chars_sci_size(float value, fp_to_chars_sci_context<float>& context, uint16_t significant_digits, fp_round rounding_mode);
	fp_to_chars_fix_result to_chars_fix_size(float value, fp_to_chars_fix_context<float>& context, int16_t precision, fp_round rounding_mode);

	fp_to_chars_sci_result to_chars_sci_shortest_size(float value);
	fp_to_chars_fix_result to_chars_fix_shortest_size(float value);
	fp_to_chars_shortest_result to_chars_shortest_size(float value, uint8_t exp_penalty, bool exp_p_sign_penalty, bool fix_0_unit_penalty);



	void to_chars_sci_mantissa_unsafe(const fp_to_chars_sci_context<float>& context, char8_t* unit_char, char8_t* decimal_chars);
	void to_chars_sci_exp_unsafe(const fp_to_chars_sci_context<float>& context, char8_t* exp_chars);

	void to_chars_fix_unsafe(const fp_to_chars_fix_context<float>& context, char8_t* unit_chars, char8_t* decimal_chars);



	fp_to_chars_sci_result to_chars_sci_size(double value, fp_to_chars_sci_context<double>& context, uint16_t significant_digits, fp_round rounding_mode);
	void to_chars_sci_mantissa_unsafe(const fp_to_chars_sci_context<double>& context, char8_t* unit_char, char8_t* decimal_chars);
	void to_chars_sci_exp_unsafe(const fp_to_chars_sci_context<double>& context, char8_t* exp_chars);


} //namespace core

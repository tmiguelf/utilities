//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///		Numeric string conversion utilities.
///
///	\copyright
///		Copyright (c) 2020 Tiago Miguel Oliveira Freire
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

#include <span>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <concepts>
#include <optional>

#include <CoreLib/Core_Alternate.hpp>

/// \n
namespace core
{
	//======== Type support ========
	namespace _p
	{
		template <typename T>
		concept is_supported_charconv_c =
			std::is_same_v<T, char8_t>  ||
			std::is_same_v<T, char16_t> ||
			std::is_same_v<T, char32_t> ||
			std::is_same_v<T, char> ||
			std::is_same_v<T, wchar_t>;
	} //namespace _p


	template <typename>
	struct char_conv_dec_supported: public std::false_type {};
	template <> struct char_conv_dec_supported<uint8_t>		: public std::true_type {};
	template <> struct char_conv_dec_supported<int8_t>		: public std::true_type {};
	template <> struct char_conv_dec_supported<uint16_t>	: public std::true_type {};
	template <> struct char_conv_dec_supported<int16_t>		: public std::true_type {};
	template <> struct char_conv_dec_supported<uint32_t>	: public std::true_type {};
	template <> struct char_conv_dec_supported<int32_t>		: public std::true_type {};
	template <> struct char_conv_dec_supported<uint64_t>	: public std::true_type {};
	template <> struct char_conv_dec_supported<int64_t>		: public std::true_type {};
	template <> struct char_conv_dec_supported<float>		: public std::true_type {};
	template <> struct char_conv_dec_supported<double>		: public std::true_type {};
	template <> struct char_conv_dec_supported<long double>	: public std::true_type {};
	
	template <typename T>
	concept char_conv_dec_supported_c = char_conv_dec_supported<T>::value;
	

	template <typename>
	struct char_conv_hex_supported: public std::false_type {};
	template <> struct char_conv_hex_supported<uint8_t>		: public std::true_type {};
	template <> struct char_conv_hex_supported<uint16_t>	: public std::true_type {};
	template <> struct char_conv_hex_supported<uint32_t>	: public std::true_type {};
	template <> struct char_conv_hex_supported<uint64_t>	: public std::true_type {};
	
	template <typename T>
	concept char_conv_hex_supported_c = char_conv_hex_supported<T>::value;



	//======== Type properties ========
	template<typename T>
	struct to_char_dec_max_digits;

	template<typename T> requires char_conv_dec_supported_c<T> && std::floating_point<T>
	struct to_char_dec_max_digits<T>
	{
		static constexpr uintptr_t maxExpDigits()
		{
			uintptr_t res = 1;
			for(uintptr_t it = std::numeric_limits<T>::max_exponent10; it /= 10; ++res) {}
			return res;
		}
		static constexpr uintptr_t value = std::numeric_limits<T>::max_digits10 + maxExpDigits() + 4; //4 extra -.E-
	};

	template<typename T> requires char_conv_dec_supported_c<T> && std::signed_integral<T>
	struct to_char_dec_max_digits<T>
	{
		static constexpr uintptr_t maxDigits()
		{
			uintptr_t res = 1;
			for(int64_t it = std::numeric_limits<T>::max(); it /= 10; ++res) {}
			return res;
		}

		static constexpr uintptr_t value = maxDigits() + 1;
	};

	template<typename T> requires char_conv_dec_supported_c<T> && std::unsigned_integral<T>
	struct to_char_dec_max_digits<T>
	{
		static constexpr uintptr_t maxDigits()
		{
			uintptr_t res = 1;
			for(uint64_t it = std::numeric_limits<T>::max(); it /= 10; ++res) {}
			return res;
		}

		static constexpr uintptr_t value = maxDigits();
	};

	template<typename T>
	constexpr uintptr_t to_char_dec_max_digits_v = to_char_dec_max_digits<T>::value;


	template<char_conv_hex_supported_c T>
	struct to_char_hex_max_digits
	{
		static constexpr uintptr_t value = sizeof(T) * 2;
	};

	template<char_conv_hex_supported_c T>
	constexpr uintptr_t to_char_hex_max_digits_v = to_char_hex_max_digits<T>::value;

	/*
	namespace _p
	{
		template <char_conv_dec_supported_c num_T>
		[[nodiscard]] uintptr_t to_chars_dec_estimate(num_T p_val);

		template <_p::is_supported_charconv_c char_T, char_conv_dec_supported_c num_T>
		[[nodiscard]] void to_chars_dec_unsafe(num_T p_val, char_T* p_out);


		template <char_conv_dec_supported_c num_T>
		[[nodiscard]] uintptr_t to_chars_hex_estimate(num_T p_val);

		template <_p::is_supported_charconv_c char_T, char_conv_dec_supported_c num_T>
		[[nodiscard]] void to_chars_hex_unsafe(num_T p_val, char_T* p_out);


		template <char_conv_dec_supported_c num_T>
		[[nodiscard]] inline constexpr uintptr_t to_chars_hex_fix_estimate(num_T) { return to_char_hex_max_digits_v<num_T>; }

		template <_p::is_supported_charconv_c char_T, char_conv_dec_supported_c num_T>
		[[nodiscard]] void to_chars_hex_fix_unsafe(num_T p_val, char_T* p_out);
	} //namespace _p
	*/



	template <_p::is_supported_charconv_c T>
	[[nodiscard]] inline constexpr bool is_digit(const T p_char) { return (p_char >= '0' && p_char <= '9'); }

	template <_p::is_supported_charconv_c T>
	[[nodiscard]] inline constexpr bool is_xdigit(const T p_char) { return is_digit(p_char) || (p_char >= 'A' && p_char <= 'F') || (p_char >= 'a' && p_char <= 'f'); }


	template <_p::is_supported_charconv_c T>
	[[nodiscard]] bool is_uint(std::basic_string_view<T> p_str);

	template <_p::is_supported_charconv_c T>
	[[nodiscard]] bool is_int(std::basic_string_view<T> p_str);

	template <_p::is_supported_charconv_c T>
	[[nodiscard]] bool is_hex(std::basic_string_view<T> p_str);




	//from_chars_result
	//illegal_byte_sequence
	//value_too_large
	//no_buffer_space
	//invalid_argument
	//
	/// \brief 
	///		Auxiliary structure to return an optional result from a potentially failing conversion function
	template <char_conv_dec_supported_c T>
	using from_chars_result = alternate<T, std::errc, std::errc{}, std::errc::invalid_argument>;



	/*template <char_conv_dec_supported_c T>
	using from_chars_result = std::optional<T>;*/

	template<char_conv_dec_supported_c num_T, _p::is_supported_charconv_c char_T>
	[[nodiscard]] from_chars_result<num_T> from_chars(std::basic_string_view<char_T> p_str);

	template<char_conv_hex_supported_c num_T, _p::is_supported_charconv_c char_T>
	[[nodiscard]] from_chars_result<num_T> from_hex_chars(std::basic_string_view<char_T> p_str);




	template <_p::is_supported_charconv_c char_T, char_conv_dec_supported_c num_T>
	[[nodiscard]]  uintptr_t to_chars(num_T p_val, std::span<char_T, to_char_dec_max_digits_v<num_T>> p_str);

	template <_p::is_supported_charconv_c char_T, char_conv_dec_supported_c num_T>
	[[nodiscard]] std::basic_string<char_T> to_chars(num_T p_val);

	template <_p::is_supported_charconv_c char_T, char_conv_hex_supported_c num_T>
	[[nodiscard]] uintptr_t to_hex_chars(num_T p_val, std::span<char_T, to_char_hex_max_digits_v<num_T>> p_str);

	template <_p::is_supported_charconv_c char_T, char_conv_hex_supported_c num_T>
	[[nodiscard]] std::basic_string<char_T> to_hex_chars(num_T p_val);

	template <_p::is_supported_charconv_c char_T, char_conv_hex_supported_c num_T>
	void to_hex_chars_fix(num_T p_val, std::span<char_T, to_char_hex_max_digits_v<num_T>> p_str);

	template <_p::is_supported_charconv_c char_T, char_conv_hex_supported_c num_T>
	[[nodiscard]] std::basic_string<char_T> to_hex_chars_fix(num_T p_val);

}	//namespace core

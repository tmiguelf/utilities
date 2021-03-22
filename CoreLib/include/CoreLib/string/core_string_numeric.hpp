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
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <concepts>

#include <CoreLib/Core_Alternate.hpp>

/// \n
namespace core
{
	template <typename>
	struct from_chars_supported: public std::false_type {};

	template <> struct from_chars_supported<uint8_t>	: public std::true_type {};
	template <> struct from_chars_supported<int8_t>		: public std::true_type {};
	template <> struct from_chars_supported<uint16_t>	: public std::true_type {};
	template <> struct from_chars_supported<int16_t>	: public std::true_type {};
	template <> struct from_chars_supported<uint32_t>	: public std::true_type {};
	template <> struct from_chars_supported<int32_t>	: public std::true_type {};
	template <> struct from_chars_supported<uint64_t>	: public std::true_type {};
	template <> struct from_chars_supported<int64_t>	: public std::true_type {};

// :(
// I need floating point charconv but only MSVC seems to support it
#if defined(_MSC_BUILD)
	template <> struct from_chars_supported<float>			: public std::true_type {};
	template <> struct from_chars_supported<double>			: public std::true_type {};
	template <> struct from_chars_supported<long double>	: public std::true_type {};
#endif


	template <typename T>
	concept from_chars_supported_c = from_chars_supported<T>::value;

	template <typename>
	struct from_hex_chars_supported: public std::false_type {};

	template <> struct from_hex_chars_supported<uint8_t>	{ static constexpr bool value = true; };
	template <> struct from_hex_chars_supported<uint16_t>	{ static constexpr bool value = true; };
	template <> struct from_hex_chars_supported<uint32_t>	{ static constexpr bool value = true; };
	template <> struct from_hex_chars_supported<uint64_t>	{ static constexpr bool value = true; };

	template <typename T>
	concept from_hex_chars_supported_c = from_hex_chars_supported<T>::value;

	namespace core_p
	{
		template <typename T>
		concept is_supported_char_c = std::is_same_v<T, char8_t> || std::is_same_v<T, char32_t>;
	} //namespace core_p


	//from_chars_result
	//illegal_byte_sequence
	//value_too_large
	//no_buffer_space
	//invalid_argument
	//
	/// \brief 
	///		Auxiliary structure to return an optional result from a potentially failing conversion function
	template <from_chars_supported_c T>
	using from_chars_result = alternate<T, std::errc, std::errc{}, std::errc::invalid_argument>;

	//======== ======== ======== From String ======== ======== ========

	template <typename T = char32_t>
	[[nodiscard]] inline bool is_digit	(T p_char) { return (p_char >= '0' && p_char <= '9'); }

	template <typename T = char32_t>
	[[nodiscard]] inline bool is_xdigit	(T p_char) { return is_digit(p_char) || (p_char >= 'A' && p_char <= 'F') || (p_char >= 'a' && p_char <= 'f'); }

	[[nodiscard]] bool is_uint	(std::basic_string_view<char8_t>	p_str);
	[[nodiscard]] bool is_uint	(std::basic_string_view<char32_t>	p_str);
	[[nodiscard]] bool is_int	(std::basic_string_view<char8_t>	p_str);
	[[nodiscard]] bool is_int	(std::basic_string_view<char32_t>	p_str);
	[[nodiscard]] bool is_hex	(std::basic_string_view<char8_t>	p_str);
	[[nodiscard]] bool is_hex	(std::basic_string_view<char32_t>	p_str);

	//bool is_number	(std::basic_string_view<char8_t>	p_str);
	//bool is_number	(std::basic_string_view<char32_t>	p_str);

	template<from_chars_supported_c T>
	[[nodiscard]] from_chars_result<T> from_chars(std::basic_string_view<char8_t> p_str);

	template<from_chars_supported_c T>
	[[nodiscard]] from_chars_result<T> from_chars(std::basic_string_view<char32_t> p_str);

	template<from_hex_chars_supported_c T>
	[[nodiscard]] from_chars_result<T> from_hex_chars(std::basic_string_view<char8_t> p_str);

	template<from_hex_chars_supported_c T>
	[[nodiscard]] from_chars_result<T> from_hex_chars(std::basic_string_view<char32_t> p_str);

	//======== ======== ======== To String ======== ======== ========

	template<typename T>
	struct to_chars_max_digits;

	template<typename T> requires from_chars_supported_c<T> && std::floating_point<T>
	struct to_chars_max_digits<T>
	{
		static constexpr uintptr_t maxExpDigits()
		{
			uintptr_t res = 1;
			for(uintptr_t it = std::numeric_limits<T>::max_exponent10; it /= 10; ++res) {}
			return res;
		}
		static constexpr uintptr_t value = std::numeric_limits<T>::max_digits10 + maxExpDigits() + 4; //4 extra -.E-
	};
	
	template<typename T> requires from_chars_supported_c<T> && std::signed_integral<T>
	struct to_chars_max_digits<T>
	{
		static constexpr uintptr_t maxDigits()
		{
			uintptr_t res = 1;
			for(int64_t it = std::numeric_limits<T>::max(); it /= 10; ++res) {}
			return res;
		}

		static constexpr uintptr_t value = maxDigits() + 1;
	};

	template<typename T> requires from_chars_supported_c<T> && std::unsigned_integral<T>
	struct to_chars_max_digits<T>
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
	constexpr uintptr_t to_chars_max_digits_v = to_chars_max_digits<T>::value;


	template<from_hex_chars_supported_c T>
	struct to_hex_chars_max_digits
	{
		static constexpr uintptr_t value = sizeof(T) * 2;
	};

	template<typename T>
	constexpr uintptr_t to_hex_chars_max_digits_v = to_hex_chars_max_digits<T>::value;


	template <core_p::is_supported_char_c char_T, from_chars_supported_c num_T>
	[[nodiscard]]  uintptr_t to_chars(num_T p_val, std::span<char_T, to_chars_max_digits_v<num_T>> p_str);

	template <core_p::is_supported_char_c char_T, from_chars_supported_c num_T>
	[[nodiscard]] std::basic_string<char_T> to_chars(num_T p_val);

	template <core_p::is_supported_char_c  char_T, from_hex_chars_supported_c num_T>
	[[nodiscard]] uintptr_t to_hex_chars(num_T p_val, std::span<char_T, to_hex_chars_max_digits_v<num_T>> p_str);

	template <core_p::is_supported_char_c  char_T, from_hex_chars_supported_c num_T>
	[[nodiscard]] std::basic_string<char_T> to_hex_chars(num_T p_val);


	template <core_p::is_supported_char_c  char_T, from_hex_chars_supported_c num_T>
	void to_hex_chars_fix(num_T p_val, std::span<char_T, to_hex_chars_max_digits_v<num_T>> p_str);

	template <core_p::is_supported_char_c  char_T, from_hex_chars_supported_c num_T>
	[[nodiscard]] std::basic_string<char_T> to_hex_chars_fix(num_T p_val);

}	//namespace core

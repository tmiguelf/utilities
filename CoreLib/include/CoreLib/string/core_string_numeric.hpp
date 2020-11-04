//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///		Numeric string conversion utilities.
///
///	\author Tiago Freire
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

/// \n
namespace core
{
	template <typename>
	struct from_chars_supported { static constexpr bool value = false; };

	template <> struct from_chars_supported<uint8_t>		{ static constexpr bool value = true; };
	template <> struct from_chars_supported<int8_t>			{ static constexpr bool value = true; };
	template <> struct from_chars_supported<uint16_t>		{ static constexpr bool value = true; };
	template <> struct from_chars_supported<int16_t>		{ static constexpr bool value = true; };
	template <> struct from_chars_supported<uint32_t>		{ static constexpr bool value = true; };
	template <> struct from_chars_supported<int32_t>		{ static constexpr bool value = true; };
	template <> struct from_chars_supported<uint64_t>		{ static constexpr bool value = true; };
	template <> struct from_chars_supported<int64_t>		{ static constexpr bool value = true; };

// :(
// I need floating point charconv but only MSVC seems to support it
#if defined(_MSC_BUILD)
	template <> struct from_chars_supported<float>			{ static constexpr bool value = true; };
	template <> struct from_chars_supported<double>			{ static constexpr bool value = true; };
	template <> struct from_chars_supported<long double>	{ static constexpr bool value = true; };
#endif


	template <typename T>
	constexpr bool from_chars_supported_v = from_chars_supported<T>::value;

	template <typename>
	struct from_hex_chars_supported { static constexpr bool value = false; };

	template <> struct from_hex_chars_supported<uint8_t>	{ static constexpr bool value = true; };
	template <> struct from_hex_chars_supported<uint16_t>	{ static constexpr bool value = true; };
	template <> struct from_hex_chars_supported<uint32_t>	{ static constexpr bool value = true; };
	template <> struct from_hex_chars_supported<uint64_t>	{ static constexpr bool value = true; };

	template <typename T>
	constexpr bool from_hex_chars_supported_v = from_hex_chars_supported<T>::value;

	namespace core_p
	{
		template <typename T>
		constexpr bool is_supported_char_v = std::is_same_v<T, char8_t> || std::is_same_v<T, char32_t>;
	} //namespace core_p


	//from_chars_result
	//illegal_byte_sequence
	//value_too_large
	//no_buffer_space
	//invalid_argument
	//
	/// \brief 
	///		Auxiliary structure to return an optional result from a potentially failing conversion function
	template <typename T, typename = std::enable_if_t<from_chars_supported_v<T>, void>>
	class from_chars_result
	{
	public:
		inline constexpr from_chars_result()					: m_errorCode{std::errc::invalid_argument} {}
		inline constexpr from_chars_result(std::errc p_code)	: m_errorCode{p_code} {}
		inline constexpr from_chars_result(T p_val)				: m_value{p_val} {}

		inline constexpr from_chars_result(const from_chars_result&) = default;
		inline constexpr from_chars_result& operator = (const from_chars_result&) = default;

		[[nodiscard]] inline constexpr bool			has_value	()			const { return m_errorCode == std::errc{}; }
		[[nodiscard]] inline constexpr T			value		()			const { return m_value; }
		[[nodiscard]] inline constexpr T			value_or	(T p_alt)	const { return has_value() ? m_value : p_alt; }
		[[nodiscard]] inline constexpr std::errc	error_code	()			const { return m_errorCode; }

	private:
		T m_value{};
		std::errc m_errorCode{};
	};

	//======== ======== ======== From String ======== ======== ========

	template <typename T = char32_t>
	[[nodiscard]] inline bool isDigit		(T p_char) { return (p_char >= '0' && p_char <= '9'); }

	template <typename T = char32_t>
	[[nodiscard]] inline bool isXDigit	(T p_char) { return isDigit(p_char) || (p_char >= 'A' && p_char <= 'F') || (p_char >= 'a' && p_char <= 'f'); }

	[[nodiscard]] bool is_uint	(std::basic_string_view<char8_t>	p_str);
	[[nodiscard]] bool is_uint	(std::basic_string_view<char32_t>	p_str);
	[[nodiscard]] bool is_int	(std::basic_string_view<char8_t>	p_str);
	[[nodiscard]] bool is_int	(std::basic_string_view<char32_t>	p_str);
	[[nodiscard]] bool is_hex	(std::basic_string_view<char8_t>	p_str);
	[[nodiscard]] bool is_hex	(std::basic_string_view<char32_t>	p_str);

	//bool is_number	(std::basic_string_view<char8_t>	p_str);
	//bool is_number	(std::basic_string_view<char32_t>	p_str);

	template<typename T, typename = std::enable_if_t<from_chars_supported_v<T>, void>>
	[[nodiscard]] from_chars_result<T> from_chars(std::basic_string_view<char8_t> p_str);

	template<typename T, typename = std::enable_if_t<from_chars_supported_v<T>, void>>
	[[nodiscard]] from_chars_result<T> from_chars(std::basic_string_view<char32_t> p_str);

	template<typename T, typename = std::enable_if_t<from_hex_chars_supported_v<T>, void>>
	[[nodiscard]] from_chars_result<T> from_hex_chars(std::basic_string_view<char8_t> p_str);

	template<typename T, typename = std::enable_if_t<from_hex_chars_supported_v<T>, void>>
	[[nodiscard]] from_chars_result<T> from_hex_chars(std::basic_string_view<char32_t> p_str);

	//======== ======== ======== To String ======== ======== ========

	template<typename, typename = void>
	struct to_chars_max_digits;


	template<typename T>
	struct to_chars_max_digits<T, std::enable_if_t<from_chars_supported_v<T> && std::is_floating_point_v<T>, void>>
	{
		static constexpr uintptr_t maxExpDigits()
		{
			uintptr_t res = 1;
			for(uintptr_t it = std::numeric_limits<T>::max_exponent10; it /= 10; ++res) {}
			return res;
		}
		static constexpr uintptr_t value = std::numeric_limits<T>::max_digits10 + maxExpDigits() + 4; //4 extra -.E-
	};
	
	template<typename T>
	struct to_chars_max_digits<T, std::enable_if_t<from_chars_supported_v<T> && !std::is_floating_point_v<T> && std::is_signed_v<T>, void>>
	{
		static constexpr uintptr_t maxDigits()
		{
			uintptr_t res = 1;
			for(int64_t it = std::numeric_limits<T>::max(); it /= 10; ++res) {}
			return res;
		}

		static constexpr uintptr_t value = maxDigits() + 1;
	};

	template<typename T>
	struct to_chars_max_digits<T, std::enable_if_t<from_chars_supported_v<T> && std::is_unsigned_v<T>, void>>
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


	template<typename T, typename = std::enable_if_t<from_hex_chars_supported_v<T>, void>>
	struct to_hex_chars_max_digits
	{
		static constexpr uintptr_t value = sizeof(T) * 2;
	};

	template<typename T>
	constexpr uintptr_t to_hex_chars_max_digits_v = to_hex_chars_max_digits<T>::value;


	template <typename char_T, typename num_T,
		typename = std::enable_if_t<core_p::is_supported_char_v<char_T> && from_chars_supported_v<num_T>, void>>
	uintptr_t to_chars(num_T p_val, std::span<char_T, to_chars_max_digits_v<num_T>> p_str);

	template <typename char_T, typename num_T,
		typename = std::enable_if_t<core_p::is_supported_char_v<char_T> && from_chars_supported_v<num_T>, void>>
	[[nodiscard]] std::basic_string<char_T> to_chars(num_T p_val);

	template <typename char_T, typename num_T,
		typename = std::enable_if_t<core_p::is_supported_char_v<char_T> && from_chars_supported_v<num_T>, void>>
	uintptr_t to_hex_chars(num_T p_val, std::span<char_T, to_hex_chars_max_digits_v<num_T>> p_str);

	template <typename char_T, typename num_T,
		typename = std::enable_if_t<core_p::is_supported_char_v<char_T> && from_hex_chars_supported_v<num_T>, void>>
	[[nodiscard]] std::basic_string<char_T> to_hex_chars(num_T p_val);


	template <typename char_T, typename num_T,
		typename = std::enable_if_t<core_p::is_supported_char_v<char_T> && from_chars_supported_v<num_T>, void>>
	void to_hex_chars_fix(num_T p_val, std::span<char_T, to_hex_chars_max_digits_v<num_T>> p_str);

	template <typename char_T, typename num_T,
		typename = std::enable_if_t<core_p::is_supported_char_v<char_T> && from_hex_chars_supported_v<num_T>, void>>
	[[nodiscard]] std::basic_string<char_T> to_hex_chars_fix(num_T p_val);

}	//namespace core

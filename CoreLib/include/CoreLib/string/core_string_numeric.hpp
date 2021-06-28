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

#include <array>
#include <concepts>
#include <limits>
#include <optional>
#include <span>
#include <string_view>
#include <system_error>
#include <type_traits>


#include <CoreLib/Core_Alternate.hpp>
#include "core_wchar_alias.hpp"

/// \n
namespace core
{
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

#if defined(_MSC_BUILD)
	template <> struct char_conv_dec_supported<float>		: public std::true_type {};
	template <> struct char_conv_dec_supported<double>		: public std::true_type {};
	template <> struct char_conv_dec_supported<long double>	: public std::true_type {};
#endif

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
	struct to_chars_dec_max_digits;

	template<typename T> requires char_conv_dec_supported_c<T> && std::floating_point<T>
	struct to_chars_dec_max_digits<T>
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
	struct to_chars_dec_max_digits<T>
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
	struct to_chars_dec_max_digits<T>
	{
		static constexpr uintptr_t maxDigits()
		{
			uintptr_t res = 1;
			for(uint64_t it = std::numeric_limits<T>::max(); it /= 10; ++res) {}
			return res;
		}

		static constexpr uintptr_t value = maxDigits();
	};

	template<char_conv_dec_supported_c T>
	constexpr uintptr_t to_chars_dec_max_digits_v = to_chars_dec_max_digits<T>::value;


	template<char_conv_hex_supported_c T>
	struct to_chars_hex_max_digits
	{
		static constexpr uintptr_t value = sizeof(T) * 2;
	};

	template<char_conv_hex_supported_c T>
	constexpr uintptr_t to_chars_hex_max_digits_v = to_chars_hex_max_digits<T>::value;


	//======== Functions ========

	
	namespace _p
	{
		template <char_conv_dec_supported_c num_T>
		[[nodiscard]] uintptr_t to_chars_estimate(num_T p_val);

		template <_p::is_internal_charconv_c char_T, char_conv_dec_supported_c num_T>
		void to_chars_unsafe(num_T p_val, char_T* p_out);

		template <char_conv_hex_supported_c num_T>
		[[nodiscard]] uintptr_t to_chars_hex_estimate(num_T p_val);

		template <_p::is_internal_charconv_c char_T, char_conv_hex_supported_c num_T>
		void to_chars_hex_unsafe(num_T p_val, char_T* p_out);

		template <char_conv_hex_supported_c num_T>
		[[nodiscard]] inline constexpr uintptr_t to_chars_hex_fix_estimate(num_T) { return to_chars_hex_max_digits_v<num_T>; }

	//	template <_p::is_internal_charconv_c char_T, char_conv_hex_supported_c num_T>
	//	void to_chars_hex_fix_unsafe(num_T p_val, char_T* p_out);
	} //namespace _p

	//======== Low level ========

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

	template <_p::is_supported_charconv_c T>
	[[nodiscard]] inline constexpr bool is_digit(const T p_char) { return (p_char >= '0' && p_char <= '9'); }

	template <_p::is_supported_charconv_c T>
	[[nodiscard]] inline constexpr bool is_xdigit(const T p_char) { return is_digit(p_char) || (p_char >= 'A' && p_char <= 'F') || (p_char >= 'a' && p_char <= 'f'); }

	namespace _p
	{
		template <_p::is_internal_charconv_c T>
		[[nodiscard]] bool is_uint(std::basic_string_view<T> p_str);

		template <_p::is_internal_charconv_c T>
		[[nodiscard]] bool is_int(std::basic_string_view<T> p_str);

		template <_p::is_internal_charconv_c T>
		[[nodiscard]] bool is_hex(std::basic_string_view<T> p_str);

		template<char_conv_dec_supported_c num_T, _p::is_internal_charconv_c char_T>
		[[nodiscard]] from_chars_result<num_T> from_chars(std::basic_string_view<char_T> p_str);

		template<char_conv_hex_supported_c num_T, _p::is_internal_charconv_c char_T>
		[[nodiscard]] from_chars_result<num_T> from_chars_hex(std::basic_string_view<char_T> p_str);

		template <_p::is_internal_charconv_c char_T, char_conv_dec_supported_c num_T>
		[[nodiscard]] uintptr_t to_chars(num_T p_val, std::span<char_T, to_chars_dec_max_digits_v<num_T>> p_str);

		template <_p::is_internal_charconv_c char_T, char_conv_hex_supported_c num_T>
		[[nodiscard]] uintptr_t to_chars_hex(num_T p_val, std::span<char_T, to_chars_hex_max_digits_v<num_T>> p_str);

		template <_p::is_internal_charconv_c char_T, char_conv_hex_supported_c num_T>
		void to_chars_hex_fix(num_T p_val, std::span<char_T, to_chars_hex_max_digits_v<num_T>> p_str);
	}


	//======== High level ========

	[[nodiscard]] inline bool is_uint(std::basic_string_view<char8_t > p_str) { return _p::is_uint(p_str); }
	[[nodiscard]] inline bool is_uint(std::basic_string_view<char16_t> p_str) { return _p::is_uint(p_str); }
	[[nodiscard]] inline bool is_uint(std::basic_string_view<char32_t> p_str) { return _p::is_uint(p_str); }

	[[nodiscard]] inline bool is_uint(std::basic_string_view<char    > p_str)
	{
		return _p::is_uint(std::u8string_view{reinterpret_cast<const char8_t*>(p_str.data()), p_str.size()});
	}
	[[nodiscard]] inline bool is_uint(std::basic_string_view<wchar_t > p_str)
	{
		return _p::is_uint(std::basic_string_view<wchar_alias>{reinterpret_cast<const wchar_alias*>(p_str.data()), p_str.size()});
	}

	[[nodiscard]] inline bool is_int(std::basic_string_view<char8_t > p_str) { return _p::is_int(p_str); }
	[[nodiscard]] inline bool is_int(std::basic_string_view<char16_t> p_str) { return _p::is_int(p_str); }
	[[nodiscard]] inline bool is_int(std::basic_string_view<char32_t> p_str) { return _p::is_int(p_str); }

	[[nodiscard]] inline bool is_int(std::basic_string_view<char    > p_str)
	{
		return _p::is_int(std::u8string_view{reinterpret_cast<const char8_t*>(p_str.data()), p_str.size()});
	}

	[[nodiscard]] inline bool is_int(std::basic_string_view<wchar_t > p_str)
	{
		return _p::is_int(std::basic_string_view<wchar_alias>{reinterpret_cast<const wchar_alias*>(p_str.data()), p_str.size()});
	}

	[[nodiscard]] inline bool is_hex(std::basic_string_view<char8_t > p_str) { return _p::is_hex(p_str); }
	[[nodiscard]] inline bool is_hex(std::basic_string_view<char16_t> p_str) { return _p::is_hex(p_str); }
	[[nodiscard]] inline bool is_hex(std::basic_string_view<char32_t> p_str) { return _p::is_hex(p_str); }

	[[nodiscard]] inline bool is_hex(std::basic_string_view<char    > p_str)
	{
		return _p::is_hex(std::u8string_view{reinterpret_cast<const char8_t*>(p_str.data()), p_str.size()});
	}

	[[nodiscard]] inline bool is_hex(std::basic_string_view<wchar_t > p_str)
	{
		return _p::is_hex(std::basic_string_view<wchar_alias>{reinterpret_cast<const wchar_alias*>(p_str.data()), p_str.size()});
	}


	template<char_conv_dec_supported_c num_T>
	[[nodiscard]] inline from_chars_result<num_T> from_chars(std::basic_string_view<char8_t > p_str) { return _p::from_chars<num_T>(p_str); }

	template<char_conv_dec_supported_c num_T>
	[[nodiscard]] inline from_chars_result<num_T> from_chars(std::basic_string_view<char16_t> p_str) { return _p::from_chars<num_T>(p_str); }

	template<char_conv_dec_supported_c num_T>
	[[nodiscard]] inline from_chars_result<num_T> from_chars(std::basic_string_view<char32_t> p_str) { return _p::from_chars<num_T>(p_str); }

	template<char_conv_dec_supported_c num_T>
	[[nodiscard]] inline from_chars_result<num_T> from_chars(std::basic_string_view<char    > p_str)
	{
		return _p::from_chars<num_T>(std::u8string_view{reinterpret_cast<const char8_t*>(p_str.data()), p_str.size()});
	}

	template<char_conv_dec_supported_c num_T>
	[[nodiscard]] inline from_chars_result<num_T> from_chars(std::basic_string_view<wchar_t > p_str)
	{
		return _p::from_chars<num_T>(std::basic_string_view<wchar_alias>{reinterpret_cast<const wchar_alias*>(p_str.data()), p_str.size()});
	}


	template<char_conv_hex_supported_c num_T>
	[[nodiscard]] from_chars_result<num_T> from_chars_hex(std::basic_string_view<char8_t > p_str) { return _p::from_chars_hex<num_T>(p_str); }

	template<char_conv_hex_supported_c num_T>
	[[nodiscard]] from_chars_result<num_T> from_chars_hex(std::basic_string_view<char16_t> p_str) { return _p::from_chars_hex<num_T>(p_str); }

	template<char_conv_hex_supported_c num_T>
	[[nodiscard]] from_chars_result<num_T> from_chars_hex(std::basic_string_view<char32_t> p_str) { return _p::from_chars_hex<num_T>(p_str); }

	template<char_conv_hex_supported_c num_T>
	[[nodiscard]] from_chars_result<num_T> from_chars_hex(std::basic_string_view<char    > p_str)
	{
		return _p::from_chars_hex<num_T>(std::u8string_view{reinterpret_cast<const char8_t*>(p_str.data()), p_str.size()});
	}

	template<char_conv_hex_supported_c num_T>
	[[nodiscard]] from_chars_result<num_T> from_chars_hex(std::basic_string_view<wchar_t > p_str)
	{
		return _p::from_chars_hex<num_T>(std::basic_string_view<wchar_alias>{reinterpret_cast<const wchar_alias*>(p_str.data()), p_str.size()});
	}

	template <char_conv_dec_supported_c num_T>
	[[nodiscard]] inline uintptr_t to_chars(num_T p_val, std::span<char8_t , to_chars_dec_max_digits_v<num_T>> p_str) { return _p::to_chars(p_val, p_str); }

	template <char_conv_dec_supported_c num_T>
	[[nodiscard]] inline uintptr_t to_chars(num_T p_val, std::span<char16_t, to_chars_dec_max_digits_v<num_T>> p_str) { return _p::to_chars(p_val, p_str); }

	template <char_conv_dec_supported_c num_T>
	[[nodiscard]] inline uintptr_t to_chars(num_T p_val, std::span<char32_t, to_chars_dec_max_digits_v<num_T>> p_str) { return _p::to_chars(p_val, p_str); }

	template <char_conv_dec_supported_c num_T>
	[[nodiscard]] inline uintptr_t to_chars(num_T p_val, std::span<char    , to_chars_dec_max_digits_v<num_T>> p_str)
	{
		constexpr uintptr_t size = to_chars_dec_max_digits_v<num_T>;
		return _p::to_chars(p_val, std::span<char8_t, size>{reinterpret_cast<char8_t*>(p_str.data()), size});
	}

	template <char_conv_dec_supported_c num_T>
	[[nodiscard]] inline uintptr_t to_chars(num_T p_val, std::span<wchar_t , to_chars_dec_max_digits_v<num_T>> p_str)
	{
		constexpr uintptr_t size = to_chars_dec_max_digits_v<num_T>;
		return _p::to_chars(p_val, std::span<wchar_alias, size>{reinterpret_cast<wchar_alias*>(p_str.data()), size});
	}


	template <char_conv_hex_supported_c num_T>
	[[nodiscard]] inline uintptr_t to_chars_hex(num_T p_val, std::span<char8_t , to_chars_hex_max_digits_v<num_T>> p_str) { return _p::to_chars_hex(p_val, p_str); }

	template <char_conv_hex_supported_c num_T>
	[[nodiscard]] inline uintptr_t to_chars_hex(num_T p_val, std::span<char16_t, to_chars_hex_max_digits_v<num_T>> p_str) { return _p::to_chars_hex(p_val, p_str); }

	template <char_conv_hex_supported_c num_T>
	[[nodiscard]] inline uintptr_t to_chars_hex(num_T p_val, std::span<char32_t, to_chars_hex_max_digits_v<num_T>> p_str) { return _p::to_chars_hex(p_val, p_str); }

	template <char_conv_hex_supported_c num_T>
	[[nodiscard]] inline uintptr_t to_chars_hex(num_T p_val, std::span<char, to_chars_hex_max_digits_v<num_T>> p_str)
	{
		constexpr uintptr_t size = to_chars_hex_max_digits_v<num_T>;
		return _p::to_chars_hex(p_val, std::span<char8_t, size>{reinterpret_cast<char8_t*>(p_str.data()), size});
	}

	template <char_conv_hex_supported_c num_T>
	[[nodiscard]] inline uintptr_t to_chars_hex(num_T p_val, std::span<wchar_t, to_chars_hex_max_digits_v<num_T>> p_str)
	{
		constexpr uintptr_t size = to_chars_hex_max_digits_v<num_T>;
		return _p::to_chars_hex(p_val, std::span<wchar_alias, size>{reinterpret_cast<wchar_alias*>(p_str.data()), size});
	}


	template <char_conv_hex_supported_c num_T>
	inline void to_chars_hex_fix(num_T p_val, std::span<char8_t , to_chars_hex_max_digits_v<num_T>> p_str) { _p::to_chars_hex_fix(p_val, p_str); }

	template <char_conv_hex_supported_c num_T>
	inline void to_chars_hex_fix(num_T p_val, std::span<char16_t, to_chars_hex_max_digits_v<num_T>> p_str) { _p::to_chars_hex_fix(p_val, p_str); }

	template <char_conv_hex_supported_c num_T>
	inline void to_chars_hex_fix(num_T p_val, std::span<char32_t, to_chars_hex_max_digits_v<num_T>> p_str) { _p::to_chars_hex_fix(p_val, p_str); }

	template <char_conv_hex_supported_c num_T>
	inline void to_chars_hex_fix(num_T p_val, std::span<char    , to_chars_hex_max_digits_v<num_T>> p_str)
	{
		constexpr uintptr_t size = to_chars_dec_max_digits_v<num_T>;
		_p::to_chars_hex_fix(p_val, std::span<char8_t, size>{reinterpret_cast<char8_t*>(p_str.data()), size});
	}

	template <char_conv_hex_supported_c num_T>
	inline void to_chars_hex_fix(num_T p_val, std::span<wchar_t , to_chars_hex_max_digits_v<num_T>> p_str)
	{
		constexpr uintptr_t size = to_chars_dec_max_digits_v<num_T>;
		_p::to_chars_hex_fix(p_val, std::span<wchar_alias, size>{reinterpret_cast<wchar_alias*>(p_str.data()), size});
	}


	template <_p::is_supported_charconv_c char_T, char_conv_dec_supported_c num_T>
	[[nodiscard]] inline std::basic_string<char_T> to_chars(num_T p_val)
	{
		constexpr uintptr_t size = to_chars_dec_max_digits_v<num_T>;
		std::array<char_T, size> buff;
		return {buff.data(), _p::to_chars(p_val, std::span<char_T, size>{buff})};
	}

	template <_p::is_supported_charconv_c char_T, char_conv_hex_supported_c num_T>
	[[nodiscard]] inline std::basic_string<char_T> to_chars_hex(num_T p_val)
	{
		constexpr uintptr_t size = to_chars_hex_max_digits_v<num_T>;
		std::array<char_T, size> buff;
		return {buff.data(), _p::to_chars_hex(p_val, std::span<char_T, size>{buff})};
	}

	template <_p::is_supported_charconv_c char_T, char_conv_hex_supported_c num_T>
	[[nodiscard]] inline std::basic_string<char_T> to_chars_hex_fix(num_T p_val)
	{
		constexpr uintptr_t size = to_chars_hex_max_digits_v<num_T>;
		std::basic_string<char_T> buff;
		buff.resize(size);
		_p::to_chars_hex_fix(p_val, std::span<char_T, size>{buff.data(), size});
		return buff;
	}

}	//namespace core

//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///		Numeric string conversion utilities.
///
///	\copyright
///		Copyright (c) Tiago Miguel Oliveira Freire
///
///		Permission is hereby granted, free of charge, to any person obtaining a copy
///		of this software and associated documentation files (the "Software"),
///		to copy, modify, publish, and/or distribute copies of the Software,
///		and to permit persons to whom the Software is furnished to do so,
///		subject to the following conditions:
///
///		The copyright notice and this permission notice shall be included in all
///		copies or substantial portions of the Software.
///		The copyrighted work, or derived works, shall not be used to train
///		Artificial Intelligence models of any sort; or otherwise be used in a
///		transformative way that could obfuscate the source of the copyright.
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
#include <type_traits>
#include <cstdint>

#include "core_wchar_alias.hpp"
#include "numeric_common.hpp"

/// \n
namespace core
{
	template <typename T>
	concept char_conv_dec_supported_c = _p::charconv_int_c<T> || _p::charconv_fp_c<T>;

	template <typename T>
	concept char_conv_hex_supported_c = _p::charconv_uint_c<T>;


	template <typename T>
	concept char_conv_bin_supported_c = _p::charconv_uint_c<T>;

	//======== Type properties ========
	template<typename T>
	struct to_chars_dec_max_size;

	template<typename T> requires char_conv_dec_supported_c<T> && std::floating_point<T>
	struct to_chars_dec_max_size<T>
	{
		static constexpr uintptr_t max_exp_digits()
		{
			uintptr_t res = 1;
			for(uintptr_t it = std::numeric_limits<T>::max_exponent10; it /= 10; ++res) {}
			return res;
		}
		static constexpr uintptr_t value = std::numeric_limits<T>::max_digits10 + max_exp_digits() + 4; //4 extra -.E-
	};

	template<typename T> requires char_conv_dec_supported_c<T> && std::signed_integral<T>
	struct to_chars_dec_max_size<T>
	{
		static constexpr uintptr_t max_size()
		{
			uintptr_t res = 1;
			for(T it = std::numeric_limits<T>::max(); it /= 10; ++res) {}
			return res;
		}

		static constexpr uintptr_t value = max_size() + 1;
	};

	template<typename T> requires char_conv_dec_supported_c<T> && std::unsigned_integral<T>
	struct to_chars_dec_max_size<T>
	{
		static constexpr uintptr_t max_size()
		{
			uintptr_t res = 1;
			for(T it = std::numeric_limits<T>::max(); it /= 10; ++res) {}
			return res;
		}

		static constexpr uintptr_t value = max_size();
	};

	template<char_conv_dec_supported_c T>
	constexpr uintptr_t to_chars_dec_max_size_v = to_chars_dec_max_size<T>::value;


	template<char_conv_hex_supported_c T>
	struct to_chars_hex_max_size
	{
		static constexpr uintptr_t value = sizeof(T) * 2;
	};

	template<char_conv_hex_supported_c T>
	constexpr uintptr_t to_chars_hex_max_size_v = to_chars_hex_max_size<T>::value;


	template<char_conv_bin_supported_c T>
	struct to_chars_bin_max_size
	{
		static constexpr uintptr_t value = sizeof(T) * 8;
	};

	template<char_conv_bin_supported_c T>
	constexpr uintptr_t to_chars_bin_max_size_v = to_chars_bin_max_size<T>::value;


	//======== Functions ========

	

	template <char_conv_dec_supported_c num_T>
	[[nodiscard]] uintptr_t to_chars_size(num_T p_val);

	template <_p::charconv_char_c char_T, char_conv_dec_supported_c num_T>
	char_T* to_chars_unsafe(num_T p_val, char_T* p_out);

	template <char_conv_hex_supported_c num_T>
	[[nodiscard]] uintptr_t to_chars_hex_size(num_T p_val);

	template <_p::charconv_char_c char_T, char_conv_hex_supported_c num_T>
	char_T* to_chars_hex_unsafe(num_T p_val, char_T* p_out);

	template <char_conv_hex_supported_c num_T>
	[[nodiscard]] inline constexpr uintptr_t to_chars_hex_fix_size(num_T const) { return to_chars_hex_max_size_v<num_T>; }

	template <_p::charconv_char_c char_T, char_conv_hex_supported_c num_T>
	void to_chars_hex_fix_unsafe(num_T p_val, char_T* p_out);

	template <char_conv_bin_supported_c num_T>
	[[nodiscard]] uintptr_t to_chars_bin_size(num_T p_val);

	template <_p::charconv_char_c char_T, char_conv_bin_supported_c num_T>
	char_T* to_chars_bin_unsafe(num_T p_val, char_T* p_out);

	template <char_conv_bin_supported_c num_T>
	[[nodiscard]] inline constexpr uintptr_t to_chars_bin_fix_size(num_T const) { return to_chars_bin_max_size_v<num_T>; }

	template <_p::charconv_char_c char_T, char_conv_bin_supported_c num_T>
	void to_chars_bin_fix_unsafe(num_T p_val, char_T* p_out);



	//======== Low level ========

	template <_p::charconv_char_extended_c T>
	[[nodiscard]] inline constexpr bool is_digit(T const p_char) { return (p_char >= '0' && p_char <= '9'); }

	template <_p::charconv_char_extended_c T>
	[[nodiscard]] inline constexpr bool is_xdigit(T const p_char) { return is_digit(p_char) || (p_char >= 'A' && p_char <= 'F') || (p_char >= 'a' && p_char <= 'f'); }

	template <_p::charconv_char_extended_c T>
	[[nodiscard]] inline constexpr bool is_bdigit(T const p_char) { return (p_char == '0' || p_char == '1'); }

	namespace _p
	{
		template <_p::charconv_char_c T>
		[[nodiscard]] bool is_uint(std::basic_string_view<T> p_str);

		template <_p::charconv_char_c T>
		[[nodiscard]] bool is_int(std::basic_string_view<T> p_str);

		template <_p::charconv_char_c T>
		[[nodiscard]] bool is_hex(std::basic_string_view<T> p_str);

		template <_p::charconv_char_c T>
		[[nodiscard]] bool is_bin(std::basic_string_view<T> p_str);

		template<char_conv_dec_supported_c num_T, _p::charconv_char_c char_T>
		[[nodiscard]] from_chars_result<num_T> from_chars(std::basic_string_view<char_T> p_str);

		template<char_conv_hex_supported_c num_T, _p::charconv_char_c char_T>
		[[nodiscard]] from_chars_result<num_T> from_chars_hex(std::basic_string_view<char_T> p_str);

		template<char_conv_bin_supported_c num_T, _p::charconv_char_c char_T>
		[[nodiscard]] from_chars_result<num_T> from_chars_bin(std::basic_string_view<char_T> p_str);

		template <_p::charconv_char_c char_T, char_conv_dec_supported_c num_T>
		[[nodiscard]] uintptr_t to_chars(num_T p_val, std::span<char_T, to_chars_dec_max_size_v<num_T>> p_str);

		template <_p::charconv_char_c char_T, char_conv_hex_supported_c num_T>
		[[nodiscard]] uintptr_t to_chars_hex(num_T p_val, std::span<char_T, to_chars_hex_max_size_v<num_T>> p_str);

		template <_p::charconv_char_c char_T, char_conv_hex_supported_c num_T>
		void to_chars_hex_fix(num_T p_val, std::span<char_T, to_chars_hex_max_size_v<num_T>> p_str);

		template <_p::charconv_char_c char_T, char_conv_bin_supported_c num_T>
		[[nodiscard]] uintptr_t to_chars_bin(num_T p_val, std::span<char_T, to_chars_bin_max_size_v<num_T>> p_str);

		template <_p::charconv_char_c char_T, char_conv_bin_supported_c num_T>
		void to_chars_bin_fix(num_T p_val, std::span<char_T, to_chars_bin_max_size_v<num_T>> p_str);
	}


	//======== High level ========
	[[nodiscard]] inline bool is_uint(std::basic_string_view<char8_t > const p_str) { return _p::is_uint(p_str); }
	[[nodiscard]] inline bool is_uint(std::basic_string_view<char16_t> const p_str) { return _p::is_uint(p_str); }
	[[nodiscard]] inline bool is_uint(std::basic_string_view<char32_t> const p_str) { return _p::is_uint(p_str); }
	[[nodiscard]] inline bool is_uint(std::basic_string_view<char    > const p_str)
	{
		return _p::is_uint<char8_t>(std::u8string_view{reinterpret_cast<char8_t const*>(p_str.data()), p_str.size()});
	}
	[[nodiscard]] inline bool is_uint(std::basic_string_view<wchar_t> const p_str)
	{
		return _p::is_uint<wchar_alias>(std::basic_string_view<wchar_alias>{reinterpret_cast<wchar_alias const*>(p_str.data()), p_str.size()});
	}

	[[nodiscard]] inline bool is_int(std::basic_string_view<char8_t > const p_str) { return _p::is_int(p_str); }
	[[nodiscard]] inline bool is_int(std::basic_string_view<char16_t> const p_str) { return _p::is_int(p_str); }
	[[nodiscard]] inline bool is_int(std::basic_string_view<char32_t> const p_str) { return _p::is_int(p_str); }

	[[nodiscard]] inline bool is_int(std::basic_string_view<char    > const p_str)
	{
		return _p::is_int(std::u8string_view{reinterpret_cast<char8_t const*>(p_str.data()), p_str.size()});
	}

	[[nodiscard]] inline bool is_int(std::basic_string_view<wchar_t> const p_str)
	{
		return _p::is_int(std::basic_string_view<wchar_alias>{reinterpret_cast<wchar_alias const*>(p_str.data()), p_str.size()});
	}

	[[nodiscard]] inline bool is_hex(std::basic_string_view<char8_t > const p_str) { return _p::is_hex(p_str); }
	[[nodiscard]] inline bool is_hex(std::basic_string_view<char16_t> const p_str) { return _p::is_hex(p_str); }
	[[nodiscard]] inline bool is_hex(std::basic_string_view<char32_t> const p_str) { return _p::is_hex(p_str); }

	[[nodiscard]] inline bool is_hex(std::basic_string_view<char    > const p_str)
	{
		return _p::is_hex(std::u8string_view{reinterpret_cast<char8_t const*>(p_str.data()), p_str.size()});
	}

	[[nodiscard]] inline bool is_hex(std::basic_string_view<wchar_t> const p_str)
	{
		return _p::is_hex(std::basic_string_view<wchar_alias>{reinterpret_cast<wchar_alias const*>(p_str.data()), p_str.size()});
	}

	[[nodiscard]] inline bool is_bin(std::basic_string_view<char8_t > const p_str) { return _p::is_bin(p_str); }
	[[nodiscard]] inline bool is_bin(std::basic_string_view<char16_t> const p_str) { return _p::is_bin(p_str); }
	[[nodiscard]] inline bool is_bin(std::basic_string_view<char32_t> const p_str) { return _p::is_bin(p_str); }

	[[nodiscard]] inline bool is_bin(std::basic_string_view<char    > const p_str)
	{
		return _p::is_bin(std::u8string_view{ reinterpret_cast<char8_t const*>(p_str.data()), p_str.size() });
	}

	[[nodiscard]] inline bool is_bin(std::basic_string_view<wchar_t> const p_str)
	{
		return _p::is_bin(std::basic_string_view<wchar_alias>{reinterpret_cast<wchar_alias const*>(p_str.data()), p_str.size()});
	}


	template<char_conv_dec_supported_c num_T>
	[[nodiscard]] inline from_chars_result<num_T> from_chars(std::basic_string_view<char8_t > const p_str) { return _p::from_chars<num_T>(p_str); }

	template<char_conv_dec_supported_c num_T>
	[[nodiscard]] inline from_chars_result<num_T> from_chars(std::basic_string_view<char16_t> const p_str) { return _p::from_chars<num_T>(p_str); }

	template<char_conv_dec_supported_c num_T>
	[[nodiscard]] inline from_chars_result<num_T> from_chars(std::basic_string_view<char32_t> const p_str) { return _p::from_chars<num_T>(p_str); }

	template<char_conv_dec_supported_c num_T>
	[[nodiscard]] inline from_chars_result<num_T> from_chars(std::basic_string_view<char    > const p_str)
	{
		return _p::from_chars<num_T>(std::u8string_view{reinterpret_cast<char8_t const*>(p_str.data()), p_str.size()});
	}

	template<char_conv_dec_supported_c num_T>
	[[nodiscard]] inline from_chars_result<num_T> from_chars(std::basic_string_view<wchar_t > const p_str)
	{
		return _p::from_chars<num_T>(std::basic_string_view<wchar_alias>{reinterpret_cast<wchar_alias const*>(p_str.data()), p_str.size()});
	}


	template<char_conv_hex_supported_c num_T>
	[[nodiscard]] inline from_chars_result<num_T> from_chars_hex(std::basic_string_view<char8_t > const p_str) { return _p::from_chars_hex<num_T>(p_str); }

	template<char_conv_hex_supported_c num_T>
	[[nodiscard]] inline from_chars_result<num_T> from_chars_hex(std::basic_string_view<char16_t> const p_str) { return _p::from_chars_hex<num_T>(p_str); }

	template<char_conv_hex_supported_c num_T>
	[[nodiscard]] inline from_chars_result<num_T> from_chars_hex(std::basic_string_view<char32_t> const p_str) { return _p::from_chars_hex<num_T>(p_str); }

	template<char_conv_hex_supported_c num_T>
	[[nodiscard]] inline from_chars_result<num_T> from_chars_hex(std::basic_string_view<char    > const p_str)
	{
		return _p::from_chars_hex<num_T>(std::u8string_view{reinterpret_cast<char8_t const*>(p_str.data()), p_str.size()});
	}

	template<char_conv_hex_supported_c num_T>
	[[nodiscard]] inline from_chars_result<num_T> from_chars_hex(std::basic_string_view<wchar_t > const p_str)
	{
		return _p::from_chars_hex<num_T>(std::basic_string_view<wchar_alias>{reinterpret_cast<wchar_alias const*>(p_str.data()), p_str.size()});
	}


	template<char_conv_bin_supported_c num_T>
	[[nodiscard]] inline from_chars_result<num_T> from_chars_bin(std::basic_string_view<char8_t > const p_str) { return _p::from_chars_bin<num_T>(p_str); }

	template<char_conv_bin_supported_c num_T>
	[[nodiscard]] inline from_chars_result<num_T> from_chars_bin(std::basic_string_view<char16_t> const p_str) { return _p::from_chars_bin<num_T>(p_str); }

	template<char_conv_bin_supported_c num_T>
	[[nodiscard]] inline from_chars_result<num_T> from_chars_bin(std::basic_string_view<char32_t> const p_str) { return _p::from_chars_bin<num_T>(p_str); }

	template<char_conv_bin_supported_c num_T>
	[[nodiscard]] inline from_chars_result<num_T> from_chars_bin(std::basic_string_view<char    > const p_str)
	{
		return _p::from_chars_bin<num_T>(std::u8string_view{ reinterpret_cast<char8_t const*>(p_str.data()), p_str.size() });
	}

	template<char_conv_bin_supported_c num_T>
	[[nodiscard]] inline from_chars_result<num_T> from_chars_bin(std::basic_string_view<wchar_t > const p_str)
	{
		return _p::from_chars_bin<num_T>(std::basic_string_view<wchar_alias>{reinterpret_cast<wchar_alias const*>(p_str.data()), p_str.size()});
	}


	template <char_conv_dec_supported_c num_T>
	[[nodiscard]] inline uintptr_t to_chars(num_T const p_val, std::span<char8_t , to_chars_dec_max_size_v<num_T>> const p_str) { return _p::to_chars(p_val, p_str); }

	template <char_conv_dec_supported_c num_T>
	[[nodiscard]] inline uintptr_t to_chars(num_T const p_val, std::span<char16_t, to_chars_dec_max_size_v<num_T>> const p_str) { return _p::to_chars(p_val, p_str); }

	template <char_conv_dec_supported_c num_T>
	[[nodiscard]] inline uintptr_t to_chars(num_T const p_val, std::span<char32_t, to_chars_dec_max_size_v<num_T>> const p_str) { return _p::to_chars(p_val, p_str); }

	template <char_conv_dec_supported_c num_T>
	[[nodiscard]] inline uintptr_t to_chars(num_T const p_val, std::span<char    , to_chars_dec_max_size_v<num_T>> const p_str)
	{
		constexpr uintptr_t size = to_chars_dec_max_size_v<num_T>;
		return _p::to_chars(p_val, std::span<char8_t, size>{reinterpret_cast<char8_t*>(p_str.data()), size});
	}

	template <char_conv_dec_supported_c num_T>
	[[nodiscard]] inline uintptr_t to_chars(num_T const p_val, std::span<wchar_t , to_chars_dec_max_size_v<num_T>> const p_str)
	{
		constexpr uintptr_t size = to_chars_dec_max_size_v<num_T>;
		return _p::to_chars(p_val, std::span<wchar_alias, size>{reinterpret_cast<wchar_alias*>(p_str.data()), size});
	}


	template <char_conv_hex_supported_c num_T>
	[[nodiscard]] inline uintptr_t to_chars_hex(num_T const p_val, std::span<char8_t , to_chars_hex_max_size_v<num_T>> const p_str) { return _p::to_chars_hex(p_val, p_str); }

	template <char_conv_hex_supported_c num_T>
	[[nodiscard]] inline uintptr_t to_chars_hex(num_T const p_val, std::span<char16_t, to_chars_hex_max_size_v<num_T>> const p_str) { return _p::to_chars_hex(p_val, p_str); }

	template <char_conv_hex_supported_c num_T>
	[[nodiscard]] inline uintptr_t to_chars_hex(num_T const p_val, std::span<char32_t, to_chars_hex_max_size_v<num_T>> const p_str) { return _p::to_chars_hex(p_val, p_str); }

	template <char_conv_hex_supported_c num_T>
	[[nodiscard]] inline uintptr_t to_chars_hex(num_T const p_val, std::span<char, to_chars_hex_max_size_v<num_T>> const p_str)
	{
		constexpr uintptr_t size = to_chars_hex_max_size_v<num_T>;
		return _p::to_chars_hex(p_val, std::span<char8_t, size>{reinterpret_cast<char8_t*>(p_str.data()), size});
	}

	template <char_conv_hex_supported_c num_T>
	[[nodiscard]] inline uintptr_t to_chars_hex(num_T const p_val, std::span<wchar_t, to_chars_hex_max_size_v<num_T>> const p_str)
	{
		constexpr uintptr_t size = to_chars_hex_max_size_v<num_T>;
		return _p::to_chars_hex(p_val, std::span<wchar_alias, size>{reinterpret_cast<wchar_alias*>(p_str.data()), size});
	}


	template <char_conv_hex_supported_c num_T>
	inline void to_chars_hex_fix(num_T const p_val, std::span<char8_t , to_chars_hex_max_size_v<num_T>> const p_str) { _p::to_chars_hex_fix(p_val, p_str); }

	template <char_conv_hex_supported_c num_T>
	inline void to_chars_hex_fix(num_T const p_val, std::span<char16_t, to_chars_hex_max_size_v<num_T>> const p_str) { _p::to_chars_hex_fix(p_val, p_str); }

	template <char_conv_hex_supported_c num_T>
	inline void to_chars_hex_fix(num_T const p_val, std::span<char32_t, to_chars_hex_max_size_v<num_T>> const p_str) { _p::to_chars_hex_fix(p_val, p_str); }

	template <char_conv_hex_supported_c num_T>
	inline void to_chars_hex_fix(num_T const p_val, std::span<char    , to_chars_hex_max_size_v<num_T>> const p_str)
	{
		constexpr uintptr_t size = to_chars_hex_max_size_v<num_T>;
		_p::to_chars_hex_fix(p_val, std::span<char8_t, size>{reinterpret_cast<char8_t*>(p_str.data()), size});
	}

	template <char_conv_hex_supported_c num_T>
	inline void to_chars_hex_fix(num_T const p_val, std::span<wchar_t , to_chars_hex_max_size_v<num_T>> const p_str)
	{
		constexpr uintptr_t size = to_chars_hex_max_size_v<num_T>;
		_p::to_chars_hex_fix(p_val, std::span<wchar_alias, size>{reinterpret_cast<wchar_alias*>(p_str.data()), size});
	}


	template <char_conv_bin_supported_c num_T>
	[[nodiscard]] inline uintptr_t to_chars_bin(num_T const p_val, std::span<char8_t , to_chars_bin_max_size_v<num_T>> const p_str) { return _p::to_chars_bin(p_val, p_str); }

	template <char_conv_bin_supported_c num_T>
	[[nodiscard]] inline uintptr_t to_chars_bin(num_T const p_val, std::span<char16_t, to_chars_bin_max_size_v<num_T>> const p_str) { return _p::to_chars_bin(p_val, p_str); }

	template <char_conv_bin_supported_c num_T>
	[[nodiscard]] inline uintptr_t to_chars_bin(num_T const p_val, std::span<char32_t, to_chars_bin_max_size_v<num_T>> const p_str) { return _p::to_chars_bin(p_val, p_str); }

	template <char_conv_bin_supported_c num_T>
	[[nodiscard]] inline uintptr_t to_chars_bin(num_T const p_val, std::span<char, to_chars_bin_max_size_v<num_T>> const p_str)
	{
		constexpr uintptr_t size = to_chars_bin_max_size_v<num_T>;
		return _p::to_chars_bin(p_val, std::span<char8_t, size>{reinterpret_cast<char8_t*>(p_str.data()), size});
	}

	template <char_conv_bin_supported_c num_T>
	[[nodiscard]] inline uintptr_t to_chars_bin(num_T const p_val, std::span<wchar_t, to_chars_bin_max_size_v<num_T>> const p_str)
	{
		constexpr uintptr_t size = to_chars_bin_max_size_v<num_T>;
		return _p::to_chars_bin(p_val, std::span<wchar_alias, size>{reinterpret_cast<wchar_alias*>(p_str.data()), size});
	}


	template <char_conv_bin_supported_c num_T>
	inline void to_chars_bin_fix(num_T const p_val, std::span<char8_t , to_chars_bin_max_size_v<num_T>> const p_str) { _p::to_chars_bin_fix(p_val, p_str); }

	template <char_conv_bin_supported_c num_T>
	inline void to_chars_bin_fix(num_T const p_val, std::span<char16_t, to_chars_bin_max_size_v<num_T>> const p_str) { _p::to_chars_bin_fix(p_val, p_str); }

	template <char_conv_bin_supported_c num_T>
	inline void to_chars_bin_fix(num_T const p_val, std::span<char32_t, to_chars_bin_max_size_v<num_T>> const p_str) { _p::to_chars_bin_fix(p_val, p_str); }

	template <char_conv_bin_supported_c num_T>
	inline void to_chars_bin_fix(num_T const p_val, std::span<char    , to_chars_bin_max_size_v<num_T>> const p_str)
	{
		constexpr uintptr_t size = to_chars_bin_max_size_v<num_T>;
		_p::to_chars_bin_fix(p_val, std::span<char8_t, size>{reinterpret_cast<char8_t*>(p_str.data()), size});
	}

	template <char_conv_bin_supported_c num_T>
	inline void to_chars_bin_fix(num_T const p_val, std::span<wchar_t , to_chars_bin_max_size_v<num_T>> const p_str)
	{
		constexpr uintptr_t size = to_chars_bin_max_size_v<num_T>;
		_p::to_chars_bin_fix(p_val, std::span<wchar_alias, size>{reinterpret_cast<wchar_alias*>(p_str.data()), size});
	}

}	//namespace core

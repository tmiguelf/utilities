//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///		
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

#include <system_error>
#include <type_traits>
#include <CoreLib/Core_Alternate.hpp>

#if 0
#include <stdfloat>
#else
using float32_t = float;
using float64_t = double;
#endif

/// \n
namespace core
{
	//======== Type support ========
	namespace _p
	{
		template <typename T>
		concept charconv_char_c =
			std::same_as<T, char8_t>  ||
			std::same_as<T, char16_t> ||
			std::same_as<T, char32_t>;

		template <typename T>
		concept charconv_char_extension_c =
			std::same_as<T, char> ||
			std::same_as<T, wchar_t>;

		template <typename T>
		concept charconv_char_extended_c =
			charconv_char_c<T> ||
			charconv_char_extension_c<T>;


		template <typename T>
		concept charconv_uint_c =
			std::same_as<T, uint8_t>  ||
			std::same_as<T, uint16_t> ||
			std::same_as<T, uint32_t> ||
			std::same_as<T, uint64_t>;

		template <typename T>
		concept charconv_sint_c =
			std::same_as<T, int8_t>  ||
			std::same_as<T, int16_t> ||
			std::same_as<T, int32_t> ||
			std::same_as<T, int64_t>;

		template <typename T>
		concept charconv_int_c =
			charconv_uint_c<T> ||
			charconv_sint_c<T>;


		template<typename T>
		concept charconv_fp_c = 
			std::same_as<T, float32_t> ||
			std::same_as<T, float64_t>;

	} //namespace _p

	//from_chars_result
	//illegal_byte_sequence
	//value_too_large
	//no_buffer_space
	//invalid_argument
	//
	/// \brief 
	///		Auxiliary structure to return an optional result from a potentially failing conversion function
	template <typename T>
	using from_chars_result = alternate<T, std::errc, std::errc{}, std::errc::invalid_argument>;

} //namespace core
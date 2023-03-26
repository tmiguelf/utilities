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

#include <type_traits>

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

		template<typename T>
		concept is_charconv_fp_supported_c = 
			std::is_same_v<T, float>
			|| std::is_same_v<T, double>;
	} //namespace _p
} //namespace core
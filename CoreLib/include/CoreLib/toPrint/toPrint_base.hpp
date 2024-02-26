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
///
///	\todo	Provide a comprehensive and consistent set of error codes, to give
///			extra information regarding he nature of the failure
//======== ======== ======== ======== ======== ======== ======== ========

#pragma once

#include <CoreLib/core_type.hpp>

#include "toPrint_support.hpp"

namespace core
{

class toPrint_base {};

template<typename>
class toPrint;
template<typename T> toPrint(T) -> toPrint<std::remove_cvref_t<T>>;

namespace _p
{
	//template<c_toPrint_char, typename, typename = void>
	//struct toPrint_has_size : public std::false_type{};
	//template<c_toPrint_char Char_t, typename Type> requires std::is_same_v<uintptr_t, decltype(std::declval<const Type>().size(std::declval<Char_t>()))>
	//struct toPrint_has_size<Char_t, Type, void>: public std::true_type{};
	//
	//template<c_toPrint_char, typename, typename = void>
	//struct toPrint_has_get : public std::false_type{};
	//template<c_toPrint_char Char_t, typename Type> requires std::is_same_v<void, decltype(std::declval<const Type>().get_print(std::declval<Char_t*>()))>
	//struct toPrint_has_get<Char_t, Type, void>: public std::true_type{};

	template<typename T>
	constexpr bool is_toPrint_v = is_derived_v<T, ::core::toPrint_base>;

	//template<c_toPrint_char Char_t, typename T>
	//constexpr bool is_valid_toPrint_v = is_toPrint_v<T> && toPrint_has_size<Char_t, T>::value && toPrint_has_get<Char_t, T>::value;

	//template<typename T>
	//concept c_toPrint = is_valid_toPrint_v<T>;
} //namespace _p

} //namespace core

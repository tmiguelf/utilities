//======== ======== ======== ======== ======== ======== ======== ========
///	\file
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
	//template<c_toPrint_char Char_t, typename Type> requires std::is_same_v<uintptr_t, decltype(std::declval<Type const>().size(std::declval<Char_t>()))>
	//struct toPrint_has_size<Char_t, Type, void>: public std::true_type{};
	//
	//template<c_toPrint_char, typename, typename = void>
	//struct toPrint_has_get : public std::false_type{};
	//template<c_toPrint_char Char_t, typename Type> requires std::is_same_v<void, decltype(std::declval<Type const>().get_print(std::declval<Char_t*>()))>
	//struct toPrint_has_get<Char_t, Type, void>: public std::true_type{};

	template<typename T>
	constexpr bool is_toPrint_v = is_derived_v<T, ::core::toPrint_base>;

	//template<c_toPrint_char Char_t, typename T>
	//constexpr bool is_valid_toPrint_v = is_toPrint_v<T> && toPrint_has_size<Char_t, T>::value && toPrint_has_get<Char_t, T>::value;

	//template<typename T>
	//concept c_toPrint = is_valid_toPrint_v<T>;
} //namespace _p

} //namespace core

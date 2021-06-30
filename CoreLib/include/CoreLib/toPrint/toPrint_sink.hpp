//======== ======== ======== ======== ======== ======== ======== ========
///	\file
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
///
///	\todo	Provide a comprehensive and consistent set of error codes, to give
///			extra information regarding he nature of the failure
//======== ======== ======== ======== ======== ======== ======== ========

#pragma once

#include <string_view>

#include <CoreLib/Core_Type.hpp>

namespace core
{

class toPrint_sink_base {};

template<typename, typename = void>
struct toPrint_has_write : public std::false_type{};
template<typename Type> requires std::is_same_v<void, decltype(std::declval<Type>().write(std::declval<std::u8string_view>()))>
struct toPrint_has_write<Type, void>: public std::true_type{};

template<typename T>
constexpr bool is_toPrint_sink_v = is_derived_v<T, ::core::_p::toPrint_sink_base> && toPrint_has_write<T>::value;

template<typename T>
concept c_toPrint_sink = is_toPrint_sink_v<T>;

template<typename>
class toPrint_sink;
template<typename T> toPrint_sink(T) -> toPrint_sink<std::remove_cvref_t<T>>;

} //namespace core

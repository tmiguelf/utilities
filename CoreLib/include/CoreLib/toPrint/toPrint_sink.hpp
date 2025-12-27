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

#include <string_view>
#include <type_traits>

#include <CoreLib/core_type.hpp>

#include "toPrint_support.hpp"

namespace core
{

class sink_toPrint_base {};

template<typename>
class sink_toPrint;
template<typename T> sink_toPrint(T) -> sink_toPrint<std::remove_cvref_t<T>>;

struct sink_toPrint_properties_t
{
	bool const has_own_buffer;
};

namespace _p
{
	template<c_toPrint_char, typename>
	struct toPrint_has_write : public std::false_type{};
	
	template<c_toPrint_char Char_t, typename Type> requires requires(Type x) { x.write(std::declval<std::basic_string_view<Char_t>>()); }
	struct toPrint_has_write<Char_t, Type>: public std::true_type{};


	template<c_toPrint_char, typename>
	struct toPrint_has_own_buffer : public std::false_type{};

	template<c_toPrint_char Char_t, typename Type> requires
	(
		( std::declval<Type>().sink_toPrint_properties.has_own_buffer == true ) and
		std::is_same_v<Char_t*, decltype(std::declval<Type>().buffer_acquire(uintptr_t{}))> and
		requires(Type x) { x.buffer_released(std::declval<Char_t*>(), uintptr_t{}); }
	)
	struct toPrint_has_own_buffer<Char_t, Type>: public std::true_type{};


	template<typename T>
	constexpr bool is_sink_toPrint_v = is_derived_v<T, ::core::sink_toPrint_base>;

	template<c_toPrint_char Char_t, typename T>
	constexpr bool is_valid_sink_toPrint_v = is_sink_toPrint_v<T> and ( toPrint_has_write<Char_t, T>::value or toPrint_has_own_buffer<Char_t, T>::value);

} //namespace _p

} //namespace core

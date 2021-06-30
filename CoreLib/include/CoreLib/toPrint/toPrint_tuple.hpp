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

#include <tuple>
#include <type_traits>

#include <string>
#include <string_view>

#include <CoreLib/Core_Type.hpp>

#include "toPrint_base.hpp"

namespace core::_p
{

template <c_tuple Tuple>
struct tuple_toPrint
{
private:
	static constexpr uintptr_t tuple_size = std::tuple_size_v<Tuple>;

	template<uintptr_t Pos>
	struct transform
	{
		using evaluated_t = std::remove_cvref_t<decltype(std::get<Pos>(std::declval<Tuple>()))>;
		using current_t = std::conditional_t<is_toPrint_v<evaluated_t>, const evaluated_t&, toPrint<evaluated_t>>;
		using type = decltype(std::tuple_cat(std::declval<std::tuple<current_t>>(), std::declval<transform<Pos + 1>::type>()));
	};

	template<>
	struct transform<tuple_size>
	{
		using type = std::tuple<>;
	};

public:
	using type = transform<0>::type;
};


template <c_tuple Tuple>
struct is_all_toPrint
{
private:
	static constexpr uintptr_t tuple_size = std::tuple_size_v<Tuple>;

	template<uintptr_t Pos>
	struct count
	{
		using evaluated_t = std::remove_cvref_t<decltype(std::get<Pos>(std::declval<Tuple>()))>;
		static constexpr bool value = is_valid_toPrint_v<evaluated_t> && count<Pos + 1>::value;
	};

	template<>
	struct count<tuple_size>: public std::true_type{};

public:
	static constexpr bool value = count<0>::value;
};


template<typename> struct is_tuple_toPrint: public std::false_type{};
template<c_tuple T> struct is_tuple_toPrint<T>: public is_all_toPrint<T>{};

template<typename T>
concept c_tuple_toPrint = is_tuple_toPrint<T>::value;

template <c_tuple Tuple>
struct tuple_toPrint_or_string_view { using type = tuple_toPrint<Tuple>::type; };

template <>
struct tuple_toPrint_or_string_view<std::tuple<std::u8string_view>> { using type = std::u8string_view; };

template <>
struct tuple_toPrint_or_string_view<std::tuple<std::u8string>> { using type = std::u8string_view; };

template <>
struct tuple_toPrint_or_string_view<std::tuple<std::string_view>> { using type = std::string_view; };

template <>
struct tuple_toPrint_or_string_view<std::tuple<std::string>> { using type = std::string_view; };

} //namespace core::_p

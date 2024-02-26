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
//======== ======== ======== ======== ======== ======== ======== ========

#pragma once

#include <cstdint>
#include <type_traits>

namespace core
{


	template<typename ...>
	struct pack {};


	template<typename>
	struct is_pack: std::false_type {};

	template<typename ...T>
	struct is_pack<pack<T...>> : public std::true_type{};


	template<typename T>
	constexpr bool is_pack_v = is_pack<T>::value;


	template<typename T>
	concept c_pack = is_pack<T>::value;


	template<c_pack ...T>
	struct pack_cat {};

	template<>
	struct pack_cat<> { using type = pack<>; };

	template<typename ...T1>
	struct pack_cat<pack<T1...>> { using type = pack<T1...>; };

	template<typename ...T1, typename ...T2, c_pack ...T3>
	struct pack_cat<pack<T1...>, pack<T2...>, T3...> { using type = pack_cat<pack<T1..., T2...>, T3...>::type; };


	template<c_pack ...T>
	using pack_cat_t = pack_cat<T...>::type;



	template<c_pack>
	struct pack_count {};

	template<>
	struct pack_count<pack<>> { static constexpr uintptr_t value = uintptr_t{0}; };

	template<typename T1, typename ...T2>
	struct pack_count<pack<T1, T2...>> { static constexpr uintptr_t value = uintptr_t{1} + pack_count<pack<T2...>>::value; };


	template<c_pack T>
	constexpr uintptr_t pack_count_v = pack_count<T>::value;



	template<uintptr_t, c_pack ...>
	struct pack_get {};

	template<typename T1, typename ...T2>
	struct pack_get<0, pack<T1, T2...>> { using type = T1; };

	template<uintptr_t Index, typename T1, typename ...T2>
	struct pack_get<Index, pack<T1, T2...>> { using type = pack_get<Index -1,  pack<T2...>>::type; };

	template<uintptr_t Index, c_pack ...T>
	using pack_get_t = pack_get<Index, T...>::type;

} //namespace core

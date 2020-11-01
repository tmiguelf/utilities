//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///		Provides standardized size integers
///
///	\author Tiago Freire
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

#include <cstdint>
#include <type_traits>

namespace core
{

template <typename T_out, typename T>
typename std::enable_if_t<
	std::is_trivially_move_constructible_v<T_out> &&
	std::is_trivially_destructible_v<T_out> &&
	std::is_trivially_move_constructible_v<T> &&
	std::is_trivially_destructible_v<T> &&
	sizeof(T) == sizeof(T_out) &&
	alignof(T) == alignof(T)
	, T_out&&>
	rvalue_reinterpret_cast(T&& p_in)
{
	return reinterpret_cast<T_out&&>(p_in);
}
}

#define CORE_MAKE_ENUM_FLAG(TYPE) \
inline TYPE		operator |	(TYPE p_1, TYPE p_2){ return static_cast<TYPE>(static_cast<std::underlying_type_t<TYPE>>(p_1) | static_cast<std::underlying_type_t<TYPE>>(p_2)); } \
inline TYPE		operator &	(TYPE p_1, TYPE p_2){ return static_cast<TYPE>(static_cast<std::underlying_type_t<TYPE>>(p_1) & static_cast<std::underlying_type_t<TYPE>>(p_2)); } \
inline TYPE		operator ^	(TYPE p_1, TYPE p_2){ return static_cast<TYPE>(static_cast<std::underlying_type_t<TYPE>>(p_1) ^ static_cast<std::underlying_type_t<TYPE>>(p_2)); } \
inline TYPE&	operator |=	(TYPE& p_1, TYPE p_2){ return p_1 = static_cast<TYPE>(static_cast<std::underlying_type_t<TYPE>>(p_1) | static_cast<std::underlying_type_t<TYPE>>(p_2)); } \
inline TYPE&	operator &=	(TYPE& p_1, TYPE p_2){ return p_1 = static_cast<TYPE>(static_cast<std::underlying_type_t<TYPE>>(p_1) & static_cast<std::underlying_type_t<TYPE>>(p_2)); } \
inline TYPE&	operator ^=	(TYPE& p_1, TYPE p_2){ return p_1 = static_cast<TYPE>(static_cast<std::underlying_type_t<TYPE>>(p_1) ^ static_cast<std::underlying_type_t<TYPE>>(p_2)); } \
inline TYPE		operator ~	(TYPE p_1)			{ return static_cast<TYPE>(~static_cast<std::underlying_type_t<TYPE>>(p_1)); }

#define CORE_MAKE_ENUM_ORDERABLE(TYPE) \
inline bool		operator <	(TYPE p_1, TYPE p_2){ return static_cast<std::underlying_type_t<TYPE>>(p_1) < static_cast<std::underlying_type_t<TYPE>>(p_2); } \
inline bool		operator >	(TYPE p_1, TYPE p_2){ return static_cast<std::underlying_type_t<TYPE>>(p_1) > static_cast<std::underlying_type_t<TYPE>>(p_2); } \
inline bool		operator <=	(TYPE p_1, TYPE p_2){ return static_cast<std::underlying_type_t<TYPE>>(p_1) <= static_cast<std::underlying_type_t<TYPE>>(p_2); } \
inline bool		operator >=	(TYPE p_1, TYPE p_2){ return static_cast<std::underlying_type_t<TYPE>>(p_1) >= static_cast<std::underlying_type_t<TYPE>>(p_2); } \
inline bool		operator ==	(TYPE p_1, TYPE p_2){ return static_cast<std::underlying_type_t<TYPE>>(p_1) == static_cast<std::underlying_type_t<TYPE>>(p_2); } \
inline bool		operator !=	(TYPE p_1, TYPE p_2){ return static_cast<std::underlying_type_t<TYPE>>(p_1) != static_cast<std::underlying_type_t<TYPE>>(p_2); }

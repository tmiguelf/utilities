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
template <typename T_out, typename T> requires
	std::is_trivially_move_constructible_v<T_out> &&
	std::is_trivially_destructible_v<T_out> &&
	std::is_trivially_move_constructible_v<T> &&
	std::is_trivially_destructible_v<T> &&
	(sizeof(T) == sizeof(T_out)) &&
	(alignof(T) == alignof(T))
[[nodiscard]] T_out&& rvalue_reinterpret_cast(T&& p_in)
{
	return reinterpret_cast<T_out&&>(p_in);
}


namespace literals
{
	constexpr uint8_t	operator "" _ui8	(unsigned long long int p_var)	{ return static_cast<uint8_t>	(p_var); }
	constexpr uint16_t	operator "" _ui16	(unsigned long long int p_var)	{ return static_cast<uint16_t>	(p_var); }
	constexpr uint32_t	operator "" _ui32	(unsigned long long int p_var)	{ return static_cast<uint32_t>	(p_var); }
	constexpr uint64_t	operator "" _ui64	(unsigned long long int p_var)	{ return static_cast<uint64_t>	(p_var); }
	constexpr int8_t	operator "" _i8		(unsigned long long int p_var)	{ return static_cast<int8_t>	(p_var); }
	constexpr int16_t	operator "" _i16	(unsigned long long int p_var)	{ return static_cast<int16_t>	(p_var); }
	constexpr int32_t	operator "" _i32	(unsigned long long int p_var)	{ return static_cast<int32_t>	(p_var); }
	constexpr int64_t	operator "" _i64	(unsigned long long int p_var)	{ return static_cast<int64_t>	(p_var); }
	constexpr float		operator "" _fp32	(unsigned long long int p_var)	{ return static_cast<float>		(p_var); }
	constexpr float		operator "" _fp32	(long double p_var)				{ return static_cast<float>		(p_var); }
	constexpr double	operator "" _fp64	(unsigned long long int p_var)	{ return static_cast<double>	(p_var); }
	constexpr double	operator "" _fp64	(long double p_var)				{ return static_cast<double>	(p_var); }
	constexpr uintptr_t	operator "" _uip	(unsigned long long int p_var)	{ return static_cast<uintptr_t>	(p_var); }
	constexpr intptr_t	operator "" _ip		(unsigned long long int p_var)	{ return static_cast<intptr_t>	(p_var); }
} //namespace literals

} //namespace core

#define CORE_MAKE_ENUM_FLAG(TYPE) \
inline constexpr TYPE	operator |	(TYPE p_1, TYPE p_2){ return static_cast<TYPE>(static_cast<std::underlying_type_t<TYPE>>(p_1) | static_cast<std::underlying_type_t<TYPE>>(p_2)); } \
inline constexpr TYPE	operator &	(TYPE p_1, TYPE p_2){ return static_cast<TYPE>(static_cast<std::underlying_type_t<TYPE>>(p_1) & static_cast<std::underlying_type_t<TYPE>>(p_2)); } \
inline constexpr TYPE	operator ^	(TYPE p_1, TYPE p_2){ return static_cast<TYPE>(static_cast<std::underlying_type_t<TYPE>>(p_1) ^ static_cast<std::underlying_type_t<TYPE>>(p_2)); } \
inline constexpr TYPE&	operator |=	(TYPE& p_1, TYPE p_2){ return p_1 = static_cast<TYPE>(static_cast<std::underlying_type_t<TYPE>>(p_1) | static_cast<std::underlying_type_t<TYPE>>(p_2)); } \
inline constexpr TYPE&	operator &=	(TYPE& p_1, TYPE p_2){ return p_1 = static_cast<TYPE>(static_cast<std::underlying_type_t<TYPE>>(p_1) & static_cast<std::underlying_type_t<TYPE>>(p_2)); } \
inline constexpr TYPE&	operator ^=	(TYPE& p_1, TYPE p_2){ return p_1 = static_cast<TYPE>(static_cast<std::underlying_type_t<TYPE>>(p_1) ^ static_cast<std::underlying_type_t<TYPE>>(p_2)); } \
inline constexpr TYPE	operator ~	(TYPE p_1)			{ return static_cast<TYPE>(~static_cast<std::underlying_type_t<TYPE>>(p_1)); }

#define CORE_MAKE_ENUM_ORDERABLE(TYPE) \
inline constexpr bool	operator <	(TYPE p_1, TYPE p_2){ return static_cast<std::underlying_type_t<TYPE>>(p_1) < static_cast<std::underlying_type_t<TYPE>>(p_2); } \
inline constexpr bool	operator >	(TYPE p_1, TYPE p_2){ return static_cast<std::underlying_type_t<TYPE>>(p_1) > static_cast<std::underlying_type_t<TYPE>>(p_2); } \
inline constexpr bool	operator <=	(TYPE p_1, TYPE p_2){ return static_cast<std::underlying_type_t<TYPE>>(p_1) <= static_cast<std::underlying_type_t<TYPE>>(p_2); } \
inline constexpr bool	operator >=	(TYPE p_1, TYPE p_2){ return static_cast<std::underlying_type_t<TYPE>>(p_1) >= static_cast<std::underlying_type_t<TYPE>>(p_2); } \
inline constexpr bool	operator ==	(TYPE p_1, TYPE p_2){ return static_cast<std::underlying_type_t<TYPE>>(p_1) == static_cast<std::underlying_type_t<TYPE>>(p_2); } \
inline constexpr bool	operator !=	(TYPE p_1, TYPE p_2){ return static_cast<std::underlying_type_t<TYPE>>(p_1) != static_cast<std::underlying_type_t<TYPE>>(p_2); }

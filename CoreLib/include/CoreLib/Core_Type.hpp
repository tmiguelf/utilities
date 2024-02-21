//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///		Provides standardized size integers
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
#include <tuple>

namespace core
{

template<typename> struct is_tuple : public std::false_type {};
template<typename ...Type> struct is_tuple<std::tuple<Type...>> : public std::true_type{};

template<typename T>
concept c_tuple = is_tuple<T>::value;

template<typename Derived, typename Base>
constexpr bool is_derived_v = std::is_base_of_v<Base, Derived> && std::is_convertible_v<const volatile Derived*, const volatile Base*>;

template<typename T>
struct always_true: public std::true_type {};

namespace literals
{
	constexpr uint8_t   operator "" _ui8 (unsigned long long int const p_var) { return static_cast<uint8_t  >(p_var); }
	constexpr uint16_t  operator "" _ui16(unsigned long long int const p_var) { return static_cast<uint16_t >(p_var); }
	constexpr uint32_t  operator "" _ui32(unsigned long long int const p_var) { return static_cast<uint32_t >(p_var); }
	constexpr uint64_t  operator "" _ui64(unsigned long long int const p_var) { return static_cast<uint64_t >(p_var); }
	constexpr int8_t    operator "" _i8  (unsigned long long int const p_var) { return static_cast<int8_t   >(p_var); }
	constexpr int16_t   operator "" _i16 (unsigned long long int const p_var) { return static_cast<int16_t  >(p_var); }
	constexpr int32_t   operator "" _i32 (unsigned long long int const p_var) { return static_cast<int32_t  >(p_var); }
	constexpr int64_t   operator "" _i64 (unsigned long long int const p_var) { return static_cast<int64_t  >(p_var); }
	constexpr float     operator "" _fp32(unsigned long long int const p_var) { return static_cast<float    >(p_var); }
	constexpr float     operator "" _fp32(         long double   const p_var) { return static_cast<float    >(p_var); }
	constexpr double    operator "" _fp64(unsigned long long int const p_var) { return static_cast<double   >(p_var); }
	constexpr double    operator "" _fp64(         long double   const p_var) { return static_cast<double   >(p_var); }
	constexpr uintptr_t operator "" _uip (unsigned long long int const p_var) { return static_cast<uintptr_t>(p_var); }
	constexpr intptr_t  operator "" _ip  (unsigned long long int const p_var) { return static_cast<intptr_t >(p_var); }
} //namespace literals

} //namespace core

#define CORE_MAKE_ENUM_FLAG(TYPE) \
inline constexpr TYPE  operator |  (TYPE const p_1, TYPE const p_2){ using u_t = std::underlying_type_t<TYPE>; return static_cast<TYPE>(static_cast<u_t>(p_1) | static_cast<u_t>(p_2)); } \
inline constexpr TYPE  operator &  (TYPE const p_1, TYPE const p_2){ using u_t = std::underlying_type_t<TYPE>; return static_cast<TYPE>(static_cast<u_t>(p_1) & static_cast<u_t>(p_2)); } \
inline constexpr TYPE  operator ^  (TYPE const p_1, TYPE const p_2){ using u_t = std::underlying_type_t<TYPE>; return static_cast<TYPE>(static_cast<u_t>(p_1) ^ static_cast<u_t>(p_2)); } \
inline constexpr TYPE& operator |= (TYPE&      p_1, TYPE const p_2){ using u_t = std::underlying_type_t<TYPE>; return p_1 = static_cast<TYPE>(static_cast<u_t>(p_1) | static_cast<u_t>(p_2)); } \
inline constexpr TYPE& operator &= (TYPE&      p_1, TYPE const p_2){ using u_t = std::underlying_type_t<TYPE>; return p_1 = static_cast<TYPE>(static_cast<u_t>(p_1) & static_cast<u_t>(p_2)); } \
inline constexpr TYPE& operator ^= (TYPE&      p_1, TYPE const p_2){ using u_t = std::underlying_type_t<TYPE>; return p_1 = static_cast<TYPE>(static_cast<u_t>(p_1) ^ static_cast<u_t>(p_2)); } \
inline constexpr TYPE  operator ~  (TYPE const p_1)                { using u_t = std::underlying_type_t<TYPE>; return static_cast<TYPE>(~static_cast<u_t>(p_1)); }

#define CORE_MAKE_ENUM_ORDERABLE(TYPE) \
inline constexpr bool operator <  (TYPE const p_1, TYPE const p_2){ using u_t = std::underlying_type_t<TYPE>; return static_cast<u_t>(p_1) <  static_cast<u_t>(p_2); } \
inline constexpr bool operator >  (TYPE const p_1, TYPE const p_2){ using u_t = std::underlying_type_t<TYPE>; return static_cast<u_t>(p_1) >  static_cast<u_t>(p_2); } \
inline constexpr bool operator <= (TYPE const p_1, TYPE const p_2){ using u_t = std::underlying_type_t<TYPE>; return static_cast<u_t>(p_1) <= static_cast<u_t>(p_2); } \
inline constexpr bool operator >= (TYPE const p_1, TYPE const p_2){ using u_t = std::underlying_type_t<TYPE>; return static_cast<u_t>(p_1) >= static_cast<u_t>(p_2); } \
inline constexpr bool operator == (TYPE const p_1, TYPE const p_2){ using u_t = std::underlying_type_t<TYPE>; return static_cast<u_t>(p_1) == static_cast<u_t>(p_2); } \
inline constexpr bool operator != (TYPE const p_1, TYPE const p_2){ using u_t = std::underlying_type_t<TYPE>; return static_cast<u_t>(p_1) != static_cast<u_t>(p_2); }

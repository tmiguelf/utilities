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

#include <cstdint>
#include <string_view>
#include <string>
#include <array>
#include <concepts>
#include <type_traits>

#include "toPrint_base.hpp"

#include <CoreLib/string/core_string_numeric.hpp>
#include <CoreLib/string/core_string_encoding.hpp>
#include <CoreLib/string/core_wchar_alias.hpp>

namespace core
{
namespace _p
{
	template <typename T>
	constexpr bool toPrint_is_one_of_chars_v =
		std::is_same_v<T, char8_t > ||
		std::is_same_v<T, char16_t> ||
		std::is_same_v<T, char32_t> ||
		std::is_same_v<T, char    > ||
		std::is_same_v<T, wchar_t >;
	
	template <typename T>
	constexpr bool toPrint_explicitly_disalowed_v =
		(std::is_array_v<T> && toPrint_is_one_of_chars_v<std::remove_cvref_t<std::remove_all_extents_t<T>>>) ||
		(std::is_pointer_v<T> && toPrint_is_one_of_chars_v<std::remove_cvref_t<std::remove_pointer_t<T>>>);
} //namespace _p

template<typename T> requires  _p::toPrint_explicitly_disalowed_v<T>
class toPrint<T>: public toPrint_base
{
public:
	static_assert(!_p::toPrint_explicitly_disalowed_v<T>, "Raw null terminated strings are explicitly disallowed. Please decide if it should be seen as a pointer or as a string_view");
};

//-------- Raw pointers --------

template<typename T> requires (std::is_pointer_v<T> && !_p::toPrint_explicitly_disalowed_v<T>)
class toPrint<T>: public toPrint_base
{
private:
	static constexpr uintptr_t aux_size = core::to_chars_hex_max_digits_v<uintptr_t>;
public:
	toPrint(const T p_data): m_data(reinterpret_cast<uintptr_t>(p_data)) {}
	static inline constexpr uintptr_t size(char8_t) { return aux_size + 2; }

	void getPrint(char8_t* p_out) const
	{
		*(p_out++) = u8'0';
		*(p_out++) = u8'x';
		core::to_chars_hex_fix(m_data,
			std::span<char8_t, aux_size>{p_out, aux_size});
	}

private:
	const uintptr_t m_data;
};


//======== ======== Strings ======== ========

//-------- Single char --------

template<>
class toPrint<char8_t>: public toPrint_base
{
public:
	constexpr toPrint(char8_t p_data): m_data(p_data) {}

	static inline constexpr uintptr_t size(char8_t) { return 1; }
	void getPrint(char8_t* p_out) const
	{
		*p_out = m_data;
	}

private:
	const char8_t m_data;
};


template<>
class toPrint<char16_t>: public toPrint_base
{
public:
	toPrint(char16_t p_data)
	{
		const uintptr_t tsize = core::encode_UTF8(p_data, std::span<char8_t, 4>{m_preCalc.data(), 4});
		if(tsize)
		{
			m_size = tsize;
		}
		else
		{
			m_preCalc[0] = '?';
			m_size = 1;
		}
	}

	inline constexpr uintptr_t size(char8_t) const { return m_size; }

	void getPrint(char8_t* p_out) const
	{
		memcpy(p_out, m_preCalc.data(), m_size);
	}

private:
	std::array<char8_t, 3> m_preCalc;
	uintptr_t m_size;
};


template<>
class toPrint<char32_t>: public toPrint_base
{
public:
	toPrint(char32_t p_data)
	{
		const uintptr_t tsize = core::encode_UTF8(p_data, m_preCalc);
		if(tsize)
		{
			m_size = tsize;
		}
		else
		{
			m_preCalc[0] = '?';
			m_size = 1;
		}
	}

	inline constexpr uintptr_t size(char8_t) const { return m_size; }

	void getPrint(char8_t* p_out) const
	{
		memcpy(p_out, m_preCalc.data(), m_size);
	}
private:
	std::array<char8_t, 4> m_preCalc;
	uintptr_t m_size;
};


template<>
class toPrint<char>: public toPrint<char8_t>
{
public:
	constexpr toPrint(char p_data): toPrint<char8_t>(static_cast<char8_t>(p_data))
	{}
};

template<>
class toPrint<wchar_t>: public toPrint<core::wchar_alias>
{
public:
	toPrint(wchar_t p_data): toPrint<core::wchar_alias>(static_cast<core::wchar_alias>(p_data))
	{}
};


//-------- String view --------

template<>
class toPrint<std::u8string_view>: public toPrint_base
{
public:
	toPrint(std::u8string_view p_data): m_data(p_data){}

	inline constexpr uintptr_t size(char8_t) const { return m_data.size(); }

	void getPrint(char8_t* p_out) const
	{
		memcpy(p_out, m_data.data(), m_data.size());
	}
private:
	const std::u8string_view m_data;
};


template<>
class toPrint<std::u16string_view>: public toPrint_base
{
public:
	toPrint(std::u16string_view p_data)
		: m_data(p_data)
		, m_size(core::_p::UTF16_to_UTF8_faulty_estimate(p_data, '?'))
	{}
	
	inline constexpr uintptr_t size(char8_t) const { return m_size; }

	void getPrint(char8_t* p_out) const
	{
		core::_p::UTF16_to_UTF8_faulty_unsafe(m_data, '?', p_out);
	}

private:
	const std::u16string_view m_data;
	const uintptr_t m_size;
};

template<>
class toPrint<std::u32string_view>: public toPrint_base
{
public:
	toPrint(std::u32string_view p_data)
		: m_data(p_data)
		, m_size(core::_p::UCS4_to_UTF8_faulty_estimate(p_data, '?'))

	{}

	inline constexpr uintptr_t size(char8_t) const { return m_size; }

	void getPrint(char8_t* p_out) const
	{
		core::_p::UCS4_to_UTF8_faulty_unsafe(m_data, '?', p_out);
	}

private:
	const std::u32string_view m_data;
	const uintptr_t m_size;
};


template<>
class toPrint<std::string_view>: public toPrint<std::u8string_view>
{
public:
	toPrint(std::string_view p_data)
		: toPrint<std::u8string_view>(std::u8string_view{reinterpret_cast<const char8_t*>(p_data.data()), p_data.size()})
	{
	}
};

template<>
class toPrint<std::wstring_view>: public toPrint<std::basic_string_view<core::wchar_alias>>
{
public:
	toPrint(std::wstring_view p_data)
		: toPrint<std::basic_string_view<core::wchar_alias>>(
			std::basic_string_view<core::wchar_alias>{reinterpret_cast<const core::wchar_alias*>(p_data.data()), p_data.size()}
		)
	{
	}
};

//-------- String --------

template<typename T>
class toPrint<std::basic_string<T>>: public toPrint<std::basic_string_view<T>>
{
public:
	toPrint(const std::basic_string<T>& p_data): toPrint<std::basic_string_view<T>>(p_data) {}
};



//======== ======== Numeric ======== ========
namespace _p
{
	template<size_t TSIZE, size_t TALIGNED, bool TSIGNED>
	struct toPrint_int_aliased_type;

	template<> struct toPrint_int_aliased_type<sizeof(uint8_t ), alignof(uint8_t ), false> { using type = uint8_t ; };
	template<> struct toPrint_int_aliased_type<sizeof(uint16_t), alignof(uint16_t), false> { using type = uint16_t; };
	template<> struct toPrint_int_aliased_type<sizeof(uint32_t), alignof(uint32_t), false> { using type = uint32_t; };
	template<> struct toPrint_int_aliased_type<sizeof(uint64_t), alignof(uint64_t), false> { using type = uint64_t; };
	template<> struct toPrint_int_aliased_type<sizeof(int8_t ), alignof(int8_t ), true> { using type = int8_t ; };
	template<> struct toPrint_int_aliased_type<sizeof(int16_t), alignof(int16_t), true> { using type = int16_t; };
	template<> struct toPrint_int_aliased_type<sizeof(int32_t), alignof(int32_t), true> { using type = int32_t; };
	template<> struct toPrint_int_aliased_type<sizeof(int64_t), alignof(int64_t), true> { using type = int64_t; };

	template<typename T>
	using toPrint_int_aliased_t = toPrint_int_aliased_type<sizeof(T), alignof(T), std::is_signed_v<T>>::type;
}
//-------- Decimal -------- 

template<typename Num_T> requires core::char_conv_dec_supported<Num_T>::value && std::is_floating_point_v<Num_T>
class toPrint<Num_T>: public toPrint_base
{
private:
	using array_t = std::array<char8_t, core::to_chars_dec_max_digits_v<Num_T>>;
public:
	toPrint(Num_T p_data)
	{
		m_size = core::to_chars(p_data, m_preCalc);
	}

	inline constexpr uintptr_t size(char8_t) const { return m_size; }

	void getPrint(char8_t* p_out) const
	{
		memcpy(p_out, m_preCalc.data(), m_size);
	}

private:
	array_t m_preCalc;
	uintptr_t m_size;
};

template<typename Num_T> requires (core::char_conv_dec_supported<Num_T>::value && !std::is_floating_point_v<Num_T>)
class toPrint<Num_T>: public toPrint_base
{
public:
	toPrint(Num_T p_data)
		: m_data{p_data}
		, m_size(core::_p::to_chars_estimate(p_data))
	{
	}

	inline constexpr uintptr_t size(char8_t) const { return m_size; }

	void getPrint(char8_t* p_out) const
	{
		core::_p::to_chars_unsafe(m_data, p_out);
	}

private:
	const Num_T m_data;
	const uintptr_t m_size;
};

template<typename Num_T> requires (std::integral<Num_T> && !core::char_conv_dec_supported<Num_T>::value)
class toPrint<Num_T>: public toPrint<_p::toPrint_int_aliased_t<Num_T>>
{
	using alias_t = _p::toPrint_int_aliased_t<Num_T>;
public:
	toPrint(Num_T p_data): toPrint<alias_t>(static_cast<alias_t>(p_data)) {}
};


//-------- Hexadecimal -------- 
template<typename>
class toPrint_hex;
template <typename T> toPrint_hex(T) -> toPrint_hex<std::remove_cvref_t<T>>;

template<core::char_conv_hex_supported_c Num_T>
class toPrint_hex<Num_T>: public toPrint_base
{
private:
	using array_t = std::array<char8_t, core::to_chars_hex_max_digits_v<Num_T>>;

public:
	toPrint_hex(Num_T p_data)
		: m_data(p_data)
		, m_size(core::_p::to_chars_hex_estimate(p_data))
	{
	}

	inline constexpr uintptr_t size(char8_t) const { return m_size; }

	void getPrint(char8_t* p_out) const
	{
		core::_p::to_chars_hex_unsafe(m_data, p_out);
	}

private:
	const Num_T m_data;
	const uintptr_t m_size;
};

template<typename Num_T> requires (std::integral<Num_T> && !core::char_conv_hex_supported<Num_T>::value)
class toPrint_hex<Num_T>: public toPrint_hex<_p::toPrint_int_aliased_t<Num_T>>
{
	using alias_t = _p::toPrint_int_aliased_t<Num_T>;
public:
	toPrint_hex(Num_T p_data): toPrint_hex<alias_t>(static_cast<alias_t>(p_data)) {}
};


//-------- Hexadecimal fixed size -------- 

template<typename>
class toPrint_hex_fix;
template <typename T> toPrint_hex_fix(T) -> toPrint_hex_fix<std::remove_cvref_t<T>>;

template<core::char_conv_hex_supported_c Num_T>
class toPrint_hex_fix<Num_T>: public toPrint_base
{
private:
	static constexpr uintptr_t array_size = core::to_chars_hex_max_digits_v<Num_T>;

public:
	constexpr toPrint_hex_fix(Num_T p_data): m_data{p_data} {}

	static inline constexpr uintptr_t size(char8_t) { return core::to_chars_hex_max_digits_v<Num_T>; }

	void getPrint(char8_t* p_out) const
	{
		core::_p::to_chars_hex_fix_unsafe(m_data, p_out);
	}

private:
	const Num_T m_data;
};

template<typename Num_T> requires (std::unsigned_integral<Num_T> && !core::char_conv_hex_supported<Num_T>::value)
class toPrint_hex_fix<Num_T>: public toPrint_hex_fix<_p::toPrint_int_aliased_t<Num_T>>
{
	using alias_t = _p::toPrint_int_aliased_t<Num_T>;
public:
	constexpr toPrint_hex_fix(Num_T p_data): toPrint_hex_fix<alias_t>(static_cast<alias_t>(p_data)) {}
};



} //namespace core

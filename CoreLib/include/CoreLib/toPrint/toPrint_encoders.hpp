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
#include <cstring>
#include <array>
#include <concepts>
#include <type_traits>

#include "toPrint_base.hpp"

#include <CoreLib/string/core_string_numeric.hpp>
#include <CoreLib/string/core_string_encoding.hpp>
#include <CoreLib/string/core_wchar_alias.hpp>

using namespace std::literals::string_view_literals;

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

template<typename T> requires _p::toPrint_explicitly_disalowed_v<T>
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

	template<_p::c_toPrint_char CharT>
	static inline constexpr uintptr_t size(const CharT&) { return aux_size + 2; }

	template<_p::c_toPrint_char CharT>
	void getPrint(CharT* p_out) const
	{
		*(p_out++) = '0';
		*(p_out++) = 'x';
		core::to_chars_hex_fix(m_data,
			std::span<CharT, aux_size>{p_out, aux_size});
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
	constexpr toPrint(char8_t const p_data): m_data(p_data) {}

	template<_p::c_toPrint_char CharT>
	static inline constexpr uintptr_t size(const CharT&) { return 1; }

	template<_p::c_toPrint_char CharT>
	void getPrint(CharT* const p_out) const
	{
		*p_out = static_cast<CharT>(m_data);
	}

private:
	const char8_t m_data;
};


template<>
class toPrint<char16_t>: public toPrint_base
{
public:
	constexpr toPrint(char16_t const p_data): m_data(p_data) {}

	inline constexpr uintptr_t size(const char8_t&) const
	{
		if(m_data < 0x0080) //Level 0
		{
			return 1;
		}
		if(m_data < 0x0800) //Level 1
		{
			return 2;
		}
		return 3;
	}

	inline constexpr uintptr_t size(const char16_t&) const { return 1; }
	inline constexpr uintptr_t size(const char32_t&) const { return 1; }

	void getPrint(char8_t * const p_out) const { encode_UTF8(m_data, std::span<char8_t, 4>{p_out, 4}); }
	void getPrint(char16_t* const p_out) const { *p_out = m_data; }
	void getPrint(char32_t* const p_out) const { *p_out = static_cast<char32_t>(m_data); }

private:
	const char16_t m_data;
};


template<>
class toPrint<char32_t>: public toPrint_base
{
public:
	constexpr toPrint(char32_t const p_data): m_data(p_data) {}

	inline constexpr uintptr_t size(const char8_t&) const
	{
		if(m_data < 0x00000080) //Level 0
		{
			return 1;
		}
		if(m_data < 0x00000800) //Level 1
		{
			return 2;
		}
		if(m_data < 0x00010000) //Level 2
		{
			return 3;
		}
		if(m_data < 0x00200000) //Level 3
		{
			return 4;
		}
		return 1;
	}

	inline constexpr uintptr_t size(const char16_t&) const
	{
		if(m_data > 0xFFFF && m_data < 0x110000)
		{
			return 2;
		}
		return 1;
	}
	inline constexpr uintptr_t size(const char32_t&) const { return 1; }

	void getPrint(char8_t* const p_out) const
	{
		if(!encode_UTF8(m_data, std::span<char8_t, 4>{p_out, 4}))
		{
			*p_out = u8'?';
		}
	}

	void getPrint(char16_t* const p_out) const
	{
		if(!encode_UTF16(m_data, std::span<char16_t, 2>{p_out, 2}))
		{
			*p_out = u'?';
		}
	}

	inline void getPrint(char32_t* const p_out) const { *p_out = m_data; }

private:
	const char32_t m_data;
};


template<>
class toPrint<char>: public toPrint<char8_t>
{
public:
	constexpr toPrint(char const p_data): toPrint<char8_t>(static_cast<char8_t const>(p_data))
	{}
};

template<>
class toPrint<wchar_t>: public toPrint<core::wchar_alias>
{
public:
	constexpr toPrint(wchar_t const p_data): toPrint<core::wchar_alias>(static_cast<core::wchar_alias const>(p_data))
	{}
};


//-------- String view --------

template<>
class toPrint<std::u8string_view>: public toPrint_base
{
public:
	constexpr toPrint(std::u8string_view const p_data): m_data(p_data){}

	inline constexpr uintptr_t size(const char8_t& ) const { return m_data.size(); }
	inline uintptr_t size(const char16_t&) const
	{
		return _p::UTF8_to_UTF16_faulty_estimate(m_data, '?');
	}

	inline uintptr_t size(const char32_t& ) const
	{
		return _p::UTF8_to_UCS4_faulty_estimate(m_data);
	}

	void getPrint(char8_t* const p_out) const
	{
		memcpy(p_out, m_data.data(), m_data.size());
	}

	void getPrint(char16_t* const p_out) const
	{
		_p::UTF8_to_UTF16_faulty_unsafe(m_data, '?', p_out);
	}

	void getPrint(char32_t* const p_out) const
	{
		_p::UTF8_to_UCS4_faulty_unsafe(m_data, '?', p_out);
	}
private:
	const std::u8string_view m_data;
};


template<>
class toPrint<std::u16string_view>: public toPrint_base
{
public:
	constexpr toPrint(std::u16string_view const p_data)
		: m_data(p_data)
	{}
	
	inline uintptr_t size(const char8_t&) const
	{
		return core::_p::UTF16_to_UTF8_faulty_estimate(m_data, '?');
	}

	inline constexpr uintptr_t size(const char16_t& ) const { return m_data.size(); }

	inline uintptr_t size(const char32_t&) const
	{
		return core::_p::UTF16_to_UCS4_faulty_estimate(m_data);
	}

	void getPrint(char8_t* const p_out) const
	{
		core::_p::UTF16_to_UTF8_faulty_unsafe(m_data, '?', p_out);
	}

	void getPrint(char16_t* const p_out) const
	{
		memcpy(p_out, m_data.data(), m_data.size() * sizeof(char16_t));
	}

	void getPrint(char32_t* const p_out) const
	{
		core::_p::UTF16_to_UCS4_faulty_unsafe(m_data, '?', p_out);
	}

private:
	const std::u16string_view m_data;
};

template<>
class toPrint<std::u32string_view>: public toPrint_base
{
public:
	constexpr toPrint(std::u32string_view const p_data)
		: m_data(p_data)

	{}

	inline uintptr_t size(const char8_t&) const
	{
		return core::_p::UCS4_to_UTF8_faulty_estimate(m_data, '?');
	}

	inline uintptr_t size(const char16_t&) const
	{
		return core::_p::UCS4_to_UTF16_faulty_estimate(m_data, '?');
	}

	inline constexpr uintptr_t size(const char32_t& ) const
	{
		return m_data.size();
	}

	void getPrint(char8_t* const p_out) const
	{
		core::_p::UCS4_to_UTF8_faulty_unsafe(m_data, '?', p_out);
	}

	void getPrint(char16_t* const p_out) const
	{
		core::_p::UCS4_to_UTF16_faulty_unsafe(m_data, '?', p_out);
	}

	void getPrint(char32_t* const p_out) const
	{
		memcpy(p_out, m_data.data(), m_data.size() * sizeof(char32_t));
	}

private:
	const std::u32string_view m_data;
};


template<>
class toPrint<std::string_view>: public toPrint<std::u8string_view>
{
public:
	toPrint(std::string_view const p_data)
		: toPrint<std::u8string_view>(std::u8string_view{reinterpret_cast<const char8_t*>(p_data.data()), p_data.size()})
	{
	}
};

template<>
class toPrint<std::wstring_view>: public toPrint<std::basic_string_view<core::wchar_alias>>
{
public:
	toPrint(std::wstring_view const p_data)
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
	template<size_t TSIZE, size_t TALIGN, bool TSIGNED>
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


	template<size_t TSIZE, size_t TALIGN>
	struct toPrint_uint_clobber_type;

	template<> struct toPrint_uint_clobber_type<sizeof(uint8_t ), alignof(uint8_t )> { using type = uint8_t ; };
	template<> struct toPrint_uint_clobber_type<sizeof(uint16_t), alignof(uint16_t)> { using type = uint16_t; };
	template<> struct toPrint_uint_clobber_type<sizeof(uint32_t), alignof(uint32_t)> { using type = uint32_t; };
	template<> struct toPrint_uint_clobber_type<sizeof(uint64_t), alignof(uint64_t)> { using type = uint64_t; };

	template<typename T>
	using toPrint_uint_clobber_t = toPrint_uint_clobber_type<sizeof(T), alignof(T)>::type;

}
//-------- Decimal -------- 

template<typename Num_T> requires core::char_conv_dec_supported<Num_T>::value && std::is_floating_point_v<Num_T>
class toPrint<Num_T>: public toPrint_base
{
private:
	using array_t = std::array<char8_t, core::to_chars_dec_max_digits_v<Num_T>>;
public:
	toPrint(Num_T const p_data)
		: m_size(core::to_chars(p_data, m_preCalc))

	{
	}

	template<_p::c_toPrint_char CharT>
	inline constexpr uintptr_t size(const CharT&) const { return m_size; }

	template<_p::c_toPrint_char CharT>
	void getPrint(CharT* p_out) const
	{
		const char8_t* pivot = m_preCalc.data();
		const char8_t* const last = pivot + m_size;
		while(pivot != last)
		{
			*(p_out++) = *(pivot++);
		}
	}

private:
	array_t m_preCalc;
	uintptr_t m_size;
};

template<typename Num_T> requires (core::char_conv_dec_supported<Num_T>::value && !std::is_floating_point_v<Num_T>)
class toPrint<Num_T>: public toPrint_base
{
public:
	constexpr toPrint(Num_T const p_data)
		: m_data{p_data}
	{
	}

	inline uintptr_t size() const
	{
		return core::_p::to_chars_estimate(m_data);
	}

	template<_p::c_toPrint_char CharT>
	inline uintptr_t size(const CharT&) const { return size(); }

	template<_p::c_toPrint_char CharT>
	inline void getPrint(CharT* const p_out) const
	{
		core::_p::to_chars_unsafe(m_data, p_out);
	}

private:
	const Num_T m_data;
};

template<typename Num_T> requires (std::integral<Num_T> && !core::char_conv_dec_supported<Num_T>::value)
class toPrint<Num_T>: public toPrint<_p::toPrint_int_aliased_t<Num_T>>
{
	using alias_t = _p::toPrint_int_aliased_t<Num_T>;
public:
	constexpr toPrint(Num_T const p_data): toPrint<alias_t>(static_cast<alias_t const>(p_data)) {}
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
	constexpr toPrint_hex(Num_T const p_data)
		: m_data(p_data)
	{
	}

	inline uintptr_t size() const { return core::_p::to_chars_hex_estimate(m_data); }

	template<_p::c_toPrint_char CharT>
	inline uintptr_t size(const CharT&) const { return size(); }

	template<_p::c_toPrint_char CharT>
	inline void getPrint(CharT* const p_out) const
	{
		core::_p::to_chars_hex_unsafe(m_data, p_out);
	}

private:
	const Num_T m_data;
};

template<typename Num_T> requires (std::integral<Num_T> && !core::char_conv_hex_supported<Num_T>::value)
class toPrint_hex<Num_T>: public toPrint_hex<_p::toPrint_uint_clobber_t<Num_T>>
{
	using alias_t = _p::toPrint_uint_clobber_t<Num_T>;
public:
	constexpr toPrint_hex(Num_T const p_data): toPrint_hex<alias_t>(static_cast<alias_t>(p_data)) {}
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
	constexpr toPrint_hex_fix(Num_T const p_data): m_data{p_data} {}

	template<_p::c_toPrint_char CharT>
	static inline constexpr uintptr_t size(const CharT&) { return array_size; }

	template<_p::c_toPrint_char CharT>
	inline void getPrint(CharT* const p_out) const
	{
		core::_p::to_chars_hex_fix_unsafe(m_data, p_out);
	}

private:
	const Num_T m_data;
};

template<typename Num_T> requires (std::integral<Num_T> && !core::char_conv_hex_supported<Num_T>::value)
class toPrint_hex_fix<Num_T>: public toPrint_hex_fix<_p::toPrint_uint_clobber_t<Num_T>>
{
	using alias_t = _p::toPrint_uint_clobber_t<Num_T>;
public:
	constexpr toPrint_hex_fix(Num_T const p_data): toPrint_hex_fix<alias_t>(static_cast<alias_t const>(p_data)) {}
};

} //namespace core

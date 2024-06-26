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

#include <cstdint>
#include <string_view>
#include <string>
#include <cstring>
#include <array>
#include <concepts>
#include <type_traits>

#ifndef _WIN32
#	include <charconv>
#	include <limits>
#endif


#include "toPrint_base.hpp"

#include <CoreLib/string/core_string_numeric.hpp>
#include <CoreLib/string/core_string_encoding.hpp>
#include <CoreLib/string/core_wchar_alias.hpp>
#include <CoreLib/string/core_fp_charconv.hpp>

using namespace std::literals::string_view_literals; // operator"" sv;

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
	static constexpr uintptr_t aux_size = core::to_chars_hex_max_size_v<uintptr_t>;
public:
	toPrint(T const p_data): m_data(reinterpret_cast<uintptr_t>(p_data)) {}

	template<_p::c_toPrint_char CharT>
	static inline constexpr uintptr_t size(CharT const&) { return aux_size + 2; }

	template<_p::c_toPrint_char CharT>
	CharT* get_print(CharT* p_out) const
	{
		*(p_out++) = '0';
		*(p_out++) = 'x';
		core::to_chars_hex_fix(m_data,
			std::span<CharT, aux_size>{p_out, aux_size});
		return p_out + core::to_chars_hex_max_size_v<uintptr_t>;
	}

private:
	uintptr_t const m_data;
};


//======== ======== Strings ======== ========

//-------- Single char --------

template<>
class toPrint<char8_t>: public toPrint_base
{
public:
	constexpr toPrint(char8_t const p_data): m_data(p_data) {}

	template<_p::c_toPrint_char CharT>
	static inline constexpr uintptr_t size(CharT const&) { return 1; }

	template<_p::c_toPrint_char CharT>
	CharT* get_print(CharT* const p_out) const
	{
		*p_out = static_cast<CharT>(m_data);
		return p_out + 1;
	}

private:
	char8_t const m_data;
};


template<>
class toPrint<char16_t>: public toPrint_base
{
public:
	constexpr toPrint(char16_t const p_data): m_data(p_data) {}

	inline constexpr uintptr_t size(char8_t const&) const
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

	inline constexpr uintptr_t size(char16_t const&) const { return 1; }
	inline constexpr uintptr_t size(char32_t const&) const { return 1; }

	char8_t * get_print(char8_t* const p_out) const
	{
		return p_out + encode_UTF8(m_data, std::span<char8_t , 4>{p_out, 4});
	}

	char16_t* get_print(char16_t* const p_out) const
	{
		*p_out = m_data;
		return p_out + 1;
	}

	char32_t* get_print(char32_t* const p_out) const
	{
		*p_out = static_cast<char32_t>(m_data);
		return p_out + 1;
	}

private:
	char16_t const m_data;
};


template<>
class toPrint<char32_t>: public toPrint_base
{
public:
	constexpr toPrint(char32_t const p_data): m_data(p_data) {}

	inline constexpr uintptr_t size(char8_t const&) const
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

	inline constexpr uintptr_t size(char16_t const&) const
	{
		if(m_data > 0xFFFF && m_data < 0x110000)
		{
			return 2;
		}
		return 1;
	}
	inline constexpr uintptr_t size(char32_t const&) const { return 1; }

	char8_t* get_print(char8_t* const p_out) const
	{
		uint8_t const rsize = encode_UTF8(m_data, std::span<char8_t, 4>{p_out, 4});
		if(!rsize)
		{
			*p_out = u8'?';
			return p_out + 1;
		}
		return p_out + rsize;
	}

	char16_t* get_print(char16_t* const p_out) const
	{
		uint8_t const rsize = encode_UTF16(m_data, std::span<char16_t, 2>{p_out, 2});
		if(!rsize)
		{
			*p_out = u'?';
			return p_out + 1;
		}
		return p_out + rsize;
	}

	inline char32_t* get_print(char32_t* const p_out) const
	{
		*p_out = m_data;
		return p_out + 1;
	}

private:
	char32_t const m_data;
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

	inline constexpr uintptr_t size(char8_t const& ) const { return m_data.size(); }
	inline uintptr_t size(char16_t const&) const
	{
		return UTF8_to_UTF16_faulty_size(m_data, '?');
	}

	inline uintptr_t size(char32_t const& ) const
	{
		return UTF8_to_UCS4_faulty_size(m_data);
	}

	char8_t* get_print(char8_t* const p_out) const
	{
		memcpy(p_out, m_data.data(), m_data.size());
		return p_out + m_data.size();
	}

	char16_t* get_print(char16_t* const p_out) const
	{
		return UTF8_to_UTF16_faulty_unsafe(m_data, '?', p_out);
	}

	char32_t* get_print(char32_t* const p_out) const
	{
		return UTF8_to_UCS4_faulty_unsafe(m_data, '?', p_out);
	}
private:
	std::u8string_view const m_data;
};


template<>
class toPrint<std::u16string_view>: public toPrint_base
{
public:
	constexpr toPrint(std::u16string_view const p_data)
		: m_data(p_data)
	{}
	
	inline uintptr_t size(char8_t const&) const
	{
		return UTF16_to_UTF8_faulty_size(m_data, '?');
	}

	inline constexpr uintptr_t size(char16_t const& ) const { return m_data.size(); }

	inline uintptr_t size(char32_t const&) const
	{
		return UTF16_to_UCS4_faulty_size(m_data);
	}

	char8_t* get_print(char8_t* const p_out) const
	{
		return UTF16_to_UTF8_faulty_unsafe(m_data, '?', p_out);
	}

	char16_t* get_print(char16_t* const p_out) const
	{
		memcpy(p_out, m_data.data(), m_data.size() * sizeof(char16_t));
		return p_out + m_data.size();
	}

	char32_t* get_print(char32_t* const p_out) const
	{
		return UTF16_to_UCS4_faulty_unsafe(m_data, '?', p_out);
	}

private:
	std::u16string_view const m_data;
};

template<>
class toPrint<std::u32string_view>: public toPrint_base
{
public:
	constexpr toPrint(std::u32string_view const p_data)
		: m_data(p_data)

	{}

	inline uintptr_t size(char8_t const&) const
	{
		return UCS4_to_UTF8_faulty_size(m_data, '?');
	}

	inline uintptr_t size(char16_t const&) const
	{
		return UCS4_to_UTF16_faulty_size(m_data, '?');
	}

	inline constexpr uintptr_t size(char32_t const& ) const
	{
		return m_data.size();
	}

	char8_t* get_print(char8_t* const p_out) const
	{
		return UCS4_to_UTF8_faulty_unsafe(m_data, '?', p_out);
	}

	char16_t* get_print(char16_t* const p_out) const
	{
		return UCS4_to_UTF16_faulty_unsafe(m_data, '?', p_out);
	}

	char32_t* get_print(char32_t* const p_out) const
	{
		memcpy(p_out, m_data.data(), m_data.size() * sizeof(char32_t));
		return p_out + m_data.size();
	}

private:
	std::u32string_view const m_data;
};


template<>
class toPrint<std::string_view>: public toPrint<std::u8string_view>
{
public:
	toPrint(std::string_view const p_data)
		: toPrint<std::u8string_view>(std::u8string_view{reinterpret_cast<char8_t const*>(p_data.data()), p_data.size()})
	{
	}
};

template<>
class toPrint<std::wstring_view>: public toPrint<std::basic_string_view<core::wchar_alias>>
{
public:
	toPrint(std::wstring_view const p_data)
		: toPrint<std::basic_string_view<core::wchar_alias>>(
			std::basic_string_view<core::wchar_alias>{reinterpret_cast<core::wchar_alias const*>(p_data.data()), p_data.size()}
		)
	{
	}
};

//-------- String --------

template<typename T>
class toPrint<std::basic_string<T>>: public toPrint<std::basic_string_view<T>>
{
public:
	toPrint(std::basic_string<T> const& p_data): toPrint<std::basic_string_view<T>>(p_data) {}
};



//======== ======== Numeric ======== ========
namespace _p
{
	template<uintptr_t TSIZE, uintptr_t TALIGN, bool TSIGNED>
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


	template<uintptr_t TSIZE, uintptr_t TALIGN>
	struct toPrint_uint_clobber_type;

	template<> struct toPrint_uint_clobber_type<sizeof(uint8_t ), alignof(uint8_t )> { using type = uint8_t ; };
	template<> struct toPrint_uint_clobber_type<sizeof(uint16_t), alignof(uint16_t)> { using type = uint16_t; };
	template<> struct toPrint_uint_clobber_type<sizeof(uint32_t), alignof(uint32_t)> { using type = uint32_t; };
	template<> struct toPrint_uint_clobber_type<sizeof(uint64_t), alignof(uint64_t)> { using type = uint64_t; };

	template<typename T>
	using toPrint_uint_clobber_t = toPrint_uint_clobber_type<sizeof(T), alignof(T)>::type;

}
//-------- Decimal -------- 

template<core::_p::charconv_fp_c Num_T>
class toPrint<Num_T>: public toPrint_base
{
private:
	using array_t = std::array<char8_t, core::to_chars_dec_max_size_v<Num_T>>;
public:
	toPrint(Num_T const p_data)
		: m_size(core::to_chars(p_data, m_preCalc))

	{
	}

	template<_p::c_toPrint_char CharT>
	inline uintptr_t size(CharT const&) const { return m_size; }

	template<_p::c_toPrint_char CharT>
	CharT* get_print(CharT* p_out) const
	{
		const char8_t* pivot = m_preCalc.data();
		const char8_t* const last = pivot + m_size;
		while(pivot != last)
		{
			*(p_out++) = *(pivot++);
		}
		return p_out;
	}

private:
	array_t m_preCalc;
	uintptr_t const m_size;
};


template<core::_p::charconv_int_c Num_T>
class toPrint<Num_T>: public toPrint_base
{
public:
	constexpr toPrint(Num_T const p_data)
		: m_data{p_data}
	{
	}

	inline uintptr_t size() const
	{
		return core::to_chars_size(m_data);
	}

	template<_p::c_toPrint_char CharT>
	inline uintptr_t size(CharT const&) const { return size(); }

	template<_p::c_toPrint_char CharT>
	inline CharT* get_print(CharT* const p_out) const
	{
		return core::to_chars_unsafe(m_data, p_out);
	}

private:
	Num_T const m_data;
};

template<typename Num_T> requires (std::integral<Num_T> && !core::_p::charconv_int_c<Num_T>)
class toPrint<Num_T>: public toPrint<_p::toPrint_int_aliased_t<Num_T>>
{
	using alias_t = _p::toPrint_int_aliased_t<Num_T>;
public:
	constexpr toPrint(Num_T const p_data): toPrint<alias_t>(static_cast<alias_t const>(p_data)) {}
};


#ifdef _WIN32
template<typename Num_T> requires std::same_as<Num_T, long double>
class toPrint<Num_T>: public toPrint<float64_t>
{
public:
	toPrint(Num_T const p_data): toPrint<float64_t>(reinterpret_cast<float64_t const&>(p_data)){}
};

#else

template<typename Num_T> requires std::same_as<Num_T, long double>
class toPrint<Num_T>: public toPrint_base
{
private:
	static constexpr uintptr_t max_exp_digits()
	{
		uintptr_t res = 1;
		for(uintptr_t it = std::numeric_limits<Num_T>::max_exponent10; it /= 10; ++res) {}
		return res;
	}
	static constexpr uintptr_t max_size = std::numeric_limits<Num_T>::max_digits10 + max_exp_digits() + 4; //4 extra -.E-

	using array_t = std::array<char8_t, max_size>;

	static inline uintptr_t fp2dec(Num_T const p_val, std::span<char8_t, max_size> const p_out)
	{
		char* const start = reinterpret_cast<char*>(p_out.data());
		std::to_chars_result const res = std::to_chars(start, start + p_out.size(), p_val);

		if(res.ec == std::errc{})
		{
			return static_cast<uintptr_t>(res.ptr - start);
		}
		return 0;
	}


public:
	toPrint(Num_T const p_data)
		: m_size(fp2dec(p_data, m_preCalc))

	{
	}

	template<_p::c_toPrint_char CharT>
	inline constexpr uintptr_t size(CharT const&) const { return m_size; }

	template<_p::c_toPrint_char CharT>
	CharT* get_print(CharT* p_out) const
	{
		char8_t const* pivot = m_preCalc.data();
		char8_t const* const last = pivot + m_size;
		while(pivot != last)
		{
			*(p_out++) = *(pivot++);
		}
		return p_out;
	}

private:
	array_t m_preCalc;
	uintptr_t const m_size;
};
#endif


//-------- fp_fancy --------

namespace _p
{
	template<core::_p::charconv_fp_c fp_t>
	struct fp_fancy_props
	{
		static constexpr uintptr_t max_size =
			fp_type_traits<fp_t>::max_scientific_exponent_digits_10 +
			fp_type_traits<fp_t>::max_shortest_digits_10 + 6; //6 extra -.x10-
	};

	template<core::_p::charconv_fp_c fp_t>
	uintptr_t to_chars_fp_fancy(fp_t p_val, std::span<char16_t, fp_fancy_props<fp_t>::max_size> p_buff);
}


template<typename>
class toPrint_fp_fancy;
template <typename T> toPrint_fp_fancy(T) -> toPrint_fp_fancy<std::remove_cvref_t<T>>;

template<core::_p::charconv_fp_c Num_T>
class toPrint_fp_fancy<Num_T>: public toPrint_base
{
private:
	using array_t = std::array<char16_t, _p::fp_fancy_props<Num_T>::max_size>;
public:
	toPrint_fp_fancy(Num_T const p_data)
		: m_size(_p::to_chars_fp_fancy(p_data, m_preCalc))
	{
	}

	template<_p::c_toPrint_char CharT>
	inline uintptr_t size(CharT const&) const { return m_size; }
	
	template<_p::c_toPrint_char CharT>
	CharT* get_print(CharT* p_out) const
	{
		char8_t const* pivot = m_preCalc.data();
		char8_t const* const last = pivot + m_size;
		while(pivot != last)
		{
			*(p_out++) = *(pivot++);
		}
		return p_out;
	}

	inline uintptr_t size(char8_t const&) const
	{
		return UTF16_to_UTF8_faulty_size(
			std::u16string_view{m_preCalc.data(), m_size}, static_cast<char32_t>(-1));
	}

	char8_t* get_print(char8_t* const p_out) const
	{
		return UTF16_to_UTF8_faulty_unsafe(
			std::u16string_view{m_preCalc.data(), m_size}, static_cast<char32_t>(-1), p_out);
	}

private:
	array_t m_preCalc;
	uintptr_t const m_size;
};


//-------- Hexadecimal --------
template<typename>
class toPrint_hex;
template <typename T> toPrint_hex(T) -> toPrint_hex<std::remove_cvref_t<T>>;

template<core::char_conv_hex_supported_c Num_T>
class toPrint_hex<Num_T>: public toPrint_base
{
public:
	constexpr toPrint_hex(Num_T const p_data)
		: m_data(p_data)
	{
	}

	inline uintptr_t size() const { return core::to_chars_hex_size(m_data); }

	template<_p::c_toPrint_char CharT>
	inline uintptr_t size(CharT const&) const { return size(); }

	template<_p::c_toPrint_char CharT>
	inline CharT* get_print(CharT* const p_out) const
	{
		return core::to_chars_hex_unsafe(m_data, p_out);
	}

private:
	Num_T const m_data;
};

template<typename Num_T> requires (std::integral<Num_T> && !core::char_conv_hex_supported_c<Num_T>)
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
	static constexpr uintptr_t array_size = core::to_chars_hex_max_size_v<Num_T>;

public:
	constexpr toPrint_hex_fix(Num_T const p_data): m_data{p_data} {}

	template<_p::c_toPrint_char CharT>
	static inline constexpr uintptr_t size(CharT const&) { return array_size; }

	template<_p::c_toPrint_char CharT>
	inline CharT* get_print(CharT* const p_out) const
	{
		core::to_chars_hex_fix_unsafe(m_data, p_out);
		return p_out + core::to_chars_hex_max_size_v<Num_T>;
	}

private:
	Num_T const m_data;
};

template<typename Num_T> requires (std::integral<Num_T> && !core::char_conv_hex_supported_c<Num_T>)
class toPrint_hex_fix<Num_T>: public toPrint_hex_fix<_p::toPrint_uint_clobber_t<Num_T>>
{
	using alias_t = _p::toPrint_uint_clobber_t<Num_T>;
public:
	constexpr toPrint_hex_fix(Num_T const p_data): toPrint_hex_fix<alias_t>(static_cast<alias_t const>(p_data)) {}
};


//-------- Binary -------- 
template<typename>
class toPrint_bin;
template <typename T> toPrint_bin(T) -> toPrint_bin<std::remove_cvref_t<T>>;

template<core::char_conv_bin_supported_c Num_T>
class toPrint_bin<Num_T>: public toPrint_base
{
private:
	using array_t = std::array<char8_t, core::to_chars_bin_max_size_v<Num_T>>;

public:
	constexpr toPrint_bin(Num_T const p_data)
		: m_data(p_data)
	{
	}

	inline uintptr_t size() const { return core::to_chars_bin_size(m_data); }

	template<_p::c_toPrint_char CharT>
	inline uintptr_t size(CharT const&) const { return size(); }

	template<_p::c_toPrint_char CharT>
	inline CharT* get_print(CharT* const p_out) const
	{
		return core::to_chars_bin_unsafe(m_data, p_out);
	}

private:
	Num_T const m_data;
};

template<typename Num_T> requires (std::integral<Num_T> && !core::char_conv_bin_supported_c<Num_T>)
class toPrint_bin<Num_T>: public toPrint_bin<_p::toPrint_uint_clobber_t<Num_T>>
{
	using alias_t = _p::toPrint_uint_clobber_t<Num_T>;
public:
	constexpr toPrint_bin(Num_T const p_data): toPrint_bin<alias_t>(static_cast<alias_t const>(p_data)) {}
};

//-------- Hexadecimal fixed size -------- 

template<typename>
class toPrint_bin_fix;
template <typename T> toPrint_bin_fix(T) -> toPrint_bin_fix<std::remove_cvref_t<T>>;

template<core::char_conv_bin_supported_c Num_T>
class toPrint_bin_fix<Num_T>: public toPrint_base
{
private:
	static constexpr uintptr_t array_size = core::to_chars_bin_max_size_v<Num_T>;

public:
	constexpr toPrint_bin_fix(Num_T const p_data): m_data{p_data} {}

	template<_p::c_toPrint_char CharT>
	static inline constexpr uintptr_t size(CharT const&) { return array_size; }

	template<_p::c_toPrint_char CharT>
	inline CharT* get_print(CharT* const p_out) const
	{
		core::to_chars_bin_fix_unsafe(m_data, p_out);
		return p_out + core::to_chars_bin_max_size_v<Num_T>;
	}

private:
	Num_T const m_data;
};

template<typename Num_T> requires (std::integral<Num_T> && !core::char_conv_bin_supported_c<Num_T>)
class toPrint_bin_fix<Num_T>: public toPrint_bin_fix<_p::toPrint_uint_clobber_t<Num_T>>
{
	using alias_t = _p::toPrint_uint_clobber_t<Num_T>;
public:
	constexpr toPrint_bin_fix(Num_T const p_data): toPrint_bin_fix<alias_t>(static_cast<alias_t const>(p_data)) {}
};


} //namespace core

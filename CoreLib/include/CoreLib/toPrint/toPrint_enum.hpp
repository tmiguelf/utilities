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

#include <utility>
#include <type_traits>

#include "toPrint_encoders.hpp"

namespace core
{
	template<typename T>
	struct toPrint_enum_string_view_table;

	template<typename T>
	class toPrint_enum_Unicode : public toPrint_base
	{
	public:
		using string_table = toPrint_enum_string_view_table<T>;
		using enum_t = T;
		using char_t = typename decltype(string_table::enum_name)::value_type;
		using uint_t = _p::toPrint_uint_clobber_t<std::underlying_type_t<enum_t>>;

		static_assert(std::is_enum_v<T>);
		static_assert(_p::toPrint_supported_char<char_t>::value);
		static_assert(std::is_same_v<decltype(string_table::to_string(std::declval<enum_t>())), std::basic_string_view<char_t>>,
			"enum_name and return type of to_string, must have the same underlying basic_string_view value_type");

	public:
		toPrint_enum_Unicode(enum_t const p_val)
			: m_decoded(string_table::to_string(p_val))
			, m_val(static_cast<uint_t>(p_val))
		{
		}

		template<_p::c_toPrint_char CharT>
		inline uintptr_t size(const CharT& temp) const
		{
			if(m_decoded.empty())
			{
				return toPrint{string_table::enum_name}.size(temp) + 4 + _p::to_chars_hex_estimate(m_val);
			}
			else
			{
				return toPrint{string_table::enum_name}.size(temp) + 2 + toPrint{m_decoded}.size(temp);
			}
		}

		template<_p::c_toPrint_char CharT>
		inline CharT* get_print(CharT* p_out) const
		{
			{
				p_out = toPrint{string_table::enum_name}.get_print(p_out);
			}
			if(m_decoded.empty())
			{
				*(p_out++) = '(';
				*(p_out++) = '0';
				*(p_out++) = 'x';
				p_out += to_chars_hex(m_val, std::span<CharT, to_chars_hex_max_size_v<uint_t>>{p_out, to_chars_hex_max_size_v<uint_t>});
				*(p_out++) = ')';
			}
			else
			{
				*(p_out++) = ':';
				*(p_out++) = ':';
				p_out = toPrint{m_decoded}.get_print(p_out);
			}
			return p_out;
		}

	private:
		std::basic_string_view<char_t> m_decoded;
		uint_t m_val;
	};

	template<typename T>
	class toPrint_enum_ASCII : public toPrint_base
	{
	public:
		using string_table = toPrint_enum_string_view_table<T>;
		using enum_t = T;
		using char_t = typename decltype(string_table::enum_name)::value_type;
		using uint_t = _p::toPrint_uint_clobber_t<std::underlying_type_t<enum_t>>;

		static_assert(std::is_enum_v<T>);
		static_assert(_p::toPrint_supported_char<char_t>::value);
		static_assert(std::is_same_v<decltype(string_table::to_string(std::declval<enum_t>())), std::basic_string_view<char_t>>,
			"enum_name and return type of to_string, must have the same underlying basic_string_view value_type");

	public:
		toPrint_enum_ASCII(enum_t const p_val)
			: m_decoded(string_table::to_string(p_val))
			, m_val(static_cast<uint_t>(p_val))
		{
		}

		inline uintptr_t size() const
		{
			if(m_decoded.empty())
			{
				return string_table::enum_name.size() + 4 + _p::to_chars_hex_estimate(m_val);
			}
			else
			{
				return string_table::enum_name.size() + 2 + m_decoded.size();
			}
		}

		template<_p::c_toPrint_char CharT>
		inline uintptr_t size(const CharT&) const
		{
			return size();
		}

		template<_p::c_toPrint_char CharT>
		inline CharT* get_print(CharT* p_out) const
		{
			if constexpr(std::is_same_v<char_t, CharT>)
			{
				memcpy(p_out, string_table::enum_name.data(), string_table::enum_name.size() * sizeof(char_t));
				p_out += string_table::enum_name.size();
			}
			else
			{
				for(const char_t cp : string_table::enum_name)
				{
					*(p_out++) = cp;
				}
			}

			if(m_decoded.empty())
			{
				*(p_out++) = '(';
				*(p_out++) = '0';
				*(p_out++) = 'x';
				p_out += to_chars_hex(m_val, std::span<CharT, to_chars_hex_max_size_v<uint_t>>{p_out, to_chars_hex_max_size_v<uint_t>});
				*(p_out++) = ')';
			}
			else
			{
				*(p_out++) = ':';
				*(p_out++) = ':';
				if constexpr(std::is_same_v<char_t, CharT>)
				{
					memcpy(p_out, m_decoded.data(), m_decoded.size() * sizeof(char_t));
					p_out += m_decoded.size();
				}
				else
				{
					for(const char_t cp : m_decoded)
					{
						*(p_out++) = cp;
					}
				}
			}
			return p_out;
		}

	private:
		std::basic_string_view<char_t> m_decoded;
		uint_t m_val;
	};
}


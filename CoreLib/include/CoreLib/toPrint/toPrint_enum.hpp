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
///
///	\todo	Provide a comprehensive and consistent set of error codes, to give
///			extra information regarding he nature of the failure
//======== ======== ======== ======== ======== ======== ======== ========

#pragma once

#include <utility>
#include <type_traits>

#include "toPrint_encoders.hpp"
#include <CoreLib/string/core_fp_charconv.hpp>

namespace core
{
	template<typename T>
	struct toPrint_enum_string_view_table;

	template<typename T>
	class toPrint_enum : public toPrint_base
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
		toPrint_enum(enum_t const p_val)
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
		inline void get_print(CharT* p_out) const
		{
			{
				const toPrint temp = toPrint{string_table::enum_name};
				temp.get_print(p_out);
				p_out += temp.size(CharT{});
			}
			if(m_decoded.empty())
			{
				*(p_out++) = '(';
				*(p_out++) = '0';
				*(p_out++) = 'x';
				_p::to_chars_hex_unsafe(m_val, p_out);
				p_out += _p::to_chars_hex_estimate(m_val);
				*(p_out) = ')';
			}
			else
			{
				*(p_out++) = ':';
				*(p_out++) = ':';
				toPrint{m_decoded}.get_print(p_out);
			}
		}

	private:
		std::basic_string_view<char_t> m_decoded;
		uint_t m_val;
	};
}


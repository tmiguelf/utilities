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

#include <ostream>
#include <vector>
#include <type_traits>

#include "toPrint_sink.hpp"
#include "toPrint_base.hpp"

#include <CoreLib/core_alloca.hpp>
#include <CoreLib/string/core_wchar_alias.hpp>

#include <CoreLib/core_extra_compiler.hpp>

namespace core
{

	template<typename ostream_t> requires std::is_base_of_v<std::basic_ostream<char>, ostream_t>
	class sink_toPrint<ostream_t>: public sink_toPrint_base
	{
	public:
		sink_toPrint(ostream_t& p_stream): m_stream(p_stream){}

		void write(std::u8string_view const p_message)
		{
			m_stream.write(reinterpret_cast<char const*>(p_message.data()), p_message.size());
		}

	private:
		ostream_t& m_stream;
	};

	namespace _p
	{
		template<_p::c_toPrint_char C, typename T> requires is_toPrint_v<T>
		void push_ostream_toPrint(std::basic_ostream<C>& p_sink, T const& p_data, C* const p_buff, uintptr_t const p_size)
		{
			p_data.get_print(p_buff);
			p_sink.write(p_buff, p_size);
		}

		template<_p::c_toPrint_char C, typename T> requires is_toPrint_v<T>
		NO_INLINE void handle_ostream_toPrint(std::basic_ostream<C>& p_stream, T const& p_data)
		{
			uintptr_t /*const*/ size = p_data.size(C{0});
			if(size)
			{
				constexpr uintptr_t alloca_treshold = 0x10000 / sizeof(C);
				if(size > alloca_treshold)
				{
					std::vector<C> buff;
					buff.resize(size);
					push_ostream_toPrint(p_stream, p_data, buff.data(), size);
				}
				else
				{
					C* buff = reinterpret_cast<C*>(core_alloca(size * sizeof(C)));
					push_ostream_toPrint(p_stream, p_data, buff, size);
				}
			}
		}

		template<typename T> requires is_toPrint_v<T>
		void push_ostream_toPrint_alias(std::basic_ostream<char>& p_sink, T const& p_data, char8_t* const p_buff, uintptr_t const p_size)
		{
			p_data.get_print(p_buff);
			p_sink.write(reinterpret_cast<char const*>(p_buff), p_size);
		}

		template<typename T> requires is_toPrint_v<T>
		NO_INLINE void handle_ostream_toPrint_alias(std::basic_ostream<char>& p_stream, T const& p_data)
		{
			uintptr_t /*const*/ size = p_data.size(char8_t{0});
			if(size)
			{
				constexpr uintptr_t alloca_treshold = 0x10000;
				if(size > alloca_treshold)
				{
					std::vector<char8_t> buff;
					buff.resize(size);
					push_ostream_toPrint_alias(p_stream, p_data, buff.data(), size);
				}
				else
				{
					char8_t* buff = reinterpret_cast<char8_t*>(core_alloca(size));
					push_ostream_toPrint_alias(p_stream, p_data, buff, size);
				}
			}
		}

		template<typename T> requires is_toPrint_v<T>
		void push_ostream_toPrint_alias(std::basic_ostream<wchar_t>& p_sink, T const& p_data, wchar_alias* const p_buff, uintptr_t const p_size)
		{
			p_data.get_print(p_buff);
			p_sink.write(reinterpret_cast<wchar_t const*>(p_buff), p_size);
		}

		template<typename T> requires is_toPrint_v<T>
		NO_INLINE void handle_ostream_toPrint_alias(std::basic_ostream<wchar_t>& p_stream, T const& p_data)
		{
			uintptr_t const size = p_data.size(wchar_alias{0});
			if(size)
			{
				constexpr uintptr_t alloca_treshold = 0x10000 / sizeof(wchar_alias);
				if(size > alloca_treshold)
				{
					std::vector<wchar_alias> buff;
					buff.resize(size);
					push_ostream_toPrint_alias(p_stream, p_data, buff.data(), size);
				}
				else
				{
					wchar_alias* buff = reinterpret_cast<wchar_alias*>(core_alloca(size * sizeof(wchar_alias)));
					push_ostream_toPrint_alias(p_stream, p_data, buff, size);
				}
			}
		}
	} //namespace _p

} //namespace core


template<typename ostream_t, typename T> requires
(
	std::is_base_of_v<std::basic_ostream<typename ostream_t::char_type>, ostream_t>
	&& core::_p::is_toPrint_v<T>
	&& core::_p::c_toPrint_char<typename ostream_t::char_type>
	)
	ostream_t& operator << (ostream_t& p_stream, T const& p_data)
{
	core::_p::handle_ostream_toPrint(p_stream, p_data);
	return p_stream;
}

template<typename ostream_t, typename T> requires
(
	std::is_base_of_v<std::basic_ostream<typename ostream_t::char_type>, ostream_t>
	&& core::_p::is_toPrint_v<T>
	&& core::_p::c_toPrint_char<typename ostream_t::char_type>
	)
	ostream_t&& operator << (ostream_t&& p_stream, T const& p_data)
{
	core::_p::handle_ostream_toPrint(p_stream, p_data);
	return std::move(p_stream);
}

template<typename ostream_t, typename T> requires
(
	std::is_base_of_v<std::basic_ostream<typename ostream_t::char_type>, ostream_t>
	&& core::_p::is_toPrint_v<T>
	&& !core::_p::c_toPrint_char<typename ostream_t::char_type>
	)
	ostream_t& operator << (ostream_t& p_stream, T const& p_data)
{
	core::_p::handle_ostream_toPrint_alias(p_stream, p_data);
	return p_stream;
}

template<typename ostream_t, typename T> requires
(
	std::is_base_of_v<std::basic_ostream<typename ostream_t::char_type>, ostream_t>
	&& core::_p::is_toPrint_v<T>
	&& !core::_p::c_toPrint_char<typename ostream_t::char_type>
	)
	ostream_t&& operator << (ostream_t&& p_stream, T const& p_data)
{
	core::_p::handle_ostream_toPrint_alias(p_stream, p_data);
	return std::move(p_stream);
}

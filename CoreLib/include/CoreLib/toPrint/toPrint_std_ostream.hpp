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

#include <ostream>
#include <vector>

#include "toPrint_sink.hpp"
#include "toPrint_base.hpp"

#include <CoreLib/Core_Alloca.hpp>
#include <CoreLib/string/core_wchar_alias.hpp>

#include <CoreLib/Core_extra_compiler.hpp>

namespace core
{

template<>
class sink_toPrint<std::ostream>: public sink_toPrint_base
{
public:
	sink_toPrint(std::ostream& p_stream): m_stream(p_stream){}

	void write(std::u8string_view const p_message)
	{
		m_stream.write(reinterpret_cast<const char*>(p_message.data()), p_message.size());
	}

private:
	std::ostream& m_stream;
};

namespace _p
{
	template<typename C, typename T> requires is_toPrint_v<T>
	void push_ostream_toPrint(std::basic_ostream<C>& p_sink, const T& p_data, C* const p_buff, uintptr_t const p_size)
	{
		p_data.get_print(p_buff);
		p_sink.write(p_buff, p_size);
	}

	template<typename C, typename T> requires is_toPrint_v<T>
	NO_INLINE void handle_ostream_toPrint(std::basic_ostream<C>& p_stream, const T& p_data)
	{
		const uintptr_t size = p_data.size(C{0});
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
	void push_ostream_toPrint_alias(std::basic_ostream<char>& p_sink, const T& p_data, char8_t* const p_buff, uintptr_t const p_size)
	{
		p_data.get_print(p_buff);
		p_sink.write(reinterpret_cast<const char*>(p_buff), p_size);
	}

	template<typename T> requires is_toPrint_v<T>
	NO_INLINE void handle_ostream_toPrint_alias(std::basic_ostream<char>& p_stream, const T& p_data)
	{
		/*const*/ uintptr_t size = p_data.size(char8_t{0});
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
	void push_ostream_toPrint_alias(std::basic_ostream<wchar_t>& p_sink, const T& p_data, wchar_alias* const p_buff, uintptr_t const p_size)
	{
		p_data.get_print(p_buff);
		p_sink.write(reinterpret_cast<const wchar_t*>(p_buff), p_size);
	}

	template<typename T> requires is_toPrint_v<T>
	NO_INLINE void handle_ostream_toPrint_alias(std::basic_ostream<wchar_t>& p_stream, const T& p_data)
	{
		/*const*/ uintptr_t size = p_data.size(wchar_alias{0});
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


template<typename T> requires core::_p::is_toPrint_v<T>
std::basic_ostream<char8_t>& operator << (std::basic_ostream<char8_t>& p_stream, const T& p_data)
{
	core::_p::handle_ostream_toPrint(p_stream, p_data);
	return p_stream;
}

template<typename T> requires core::_p::is_toPrint_v<T>
std::basic_ostream<char16_t>& operator << (std::basic_ostream<char16_t>& p_stream, const T& p_data)
{
	core::_p::handle_ostream_toPrint(p_stream, p_data);
	return p_stream;
}

template<typename T> requires core::_p::is_toPrint_v<T>
std::basic_ostream<char32_t>& operator << (std::basic_ostream<char32_t>& p_stream, const T& p_data)
{
	core::_p::handle_ostream_toPrint(p_stream, p_data);
	return p_stream;
}

template<typename T> requires core::_p::is_toPrint_v<T>
std::basic_ostream<char>& operator << (std::basic_ostream<char>& p_stream, const T& p_data)
{
	core::_p::handle_ostream_toPrint_alias(p_stream, p_data);
	return p_stream;
}

template<typename T> requires core::_p::is_toPrint_v<T>
std::basic_ostream<wchar_t>& operator << (std::basic_ostream<wchar_t>& p_stream, const T& p_data)
{
	core::_p::handle_ostream_toPrint_alias(p_stream, p_data);
	return p_stream;
}

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

#include <CoreLib/core_debugger.hpp>

#ifdef _WIN32

#include <vector>

#include <Windows.h>

#include <CoreLib/core_alloca.hpp>
#include <CoreLib/core_extra_compiler.hpp>
#include <CoreLib/string/core_string_encoding.hpp>



namespace core
{

static void aux_debug_output(std::u8string_view const p_out, std::span<char16_t> const p_buffer)
{
	core::_p::UTF8_to_UTF16_faulty_unsafe(p_out, '?', p_buffer.data());
	p_buffer[p_buffer.size() - 1] = 0;
	OutputDebugStringW(reinterpret_cast<const wchar_t*>(p_buffer.data()));
}

static void aux_debug_output(std::u16string_view const p_out, std::span<char16_t> const p_buffer)
{
	memcpy(p_buffer.data(), p_out.data(), p_out.size() * sizeof(char16_t));
	p_buffer[p_buffer.size() - 1] = 0;
	OutputDebugStringW(reinterpret_cast<const wchar_t*>(p_buffer.data()));
}

static void aux_debug_output(std::u32string_view const p_out, std::span<char16_t> const p_buffer)
{
	core::_p::UCS4_to_UTF16_faulty_unsafe(p_out, '?', p_buffer.data());
	p_buffer[p_buffer.size() - 1] = 0;
	OutputDebugStringW(reinterpret_cast<const wchar_t*>(p_buffer.data()));
}


static constexpr uintptr_t alloca_treshold = 0x8000;

void debugger_out::write(std::string_view const p_out)
{
	debugger_out::write(std::u8string_view{reinterpret_cast<const char8_t*>(p_out.data()), p_out.size()});
}

void debugger_out::write(std::wstring_view const p_out)
{
	debugger_out::write(std::u16string_view{reinterpret_cast<const char16_t*>(p_out.data()), p_out.size()});
}

NO_INLINE void debugger_out::write(std::u8string_view const p_out)
{
	if(IsDebuggerPresent())
	{
		uintptr_t const buff_size = core::_p::UTF8_to_UTF16_faulty_estimate(p_out, '?') + 1;
		if(buff_size > alloca_treshold)
		{
			std::vector<char16_t> buff;
			buff.resize(buff_size);
			aux_debug_output(p_out, buff);
		}
		else
		{
			char16_t* const buff = reinterpret_cast<char16_t* const>(core_alloca(buff_size * sizeof(char16_t)));
			aux_debug_output(p_out, std::span<char16_t>(buff, buff_size));
		}
	}
}

NO_INLINE void debugger_out::write(std::u16string_view const p_out)
{
	if(IsDebuggerPresent())
	{
		uintptr_t buff_size = p_out.size() + 1;
		if(buff_size > alloca_treshold)
		{
			std::vector<char16_t> buff;
			buff.resize(buff_size);
			aux_debug_output(p_out, buff);
		}
		else
		{
			char16_t* const buff = reinterpret_cast<char16_t* const>(core_alloca(buff_size * sizeof(char16_t)));
			aux_debug_output(p_out, std::span<char16_t>(buff, buff_size));
		}
	}
}

NO_INLINE void debugger_out::write(std::u32string_view const p_out)
{
	if(IsDebuggerPresent())
	{
		uintptr_t const buff_size = core::_p::UCS4_to_UTF16_faulty_estimate(p_out, '?') + 1;
		if(buff_size > alloca_treshold)
		{
			std::vector<char16_t> buff;
			buff.resize(buff_size);
			aux_debug_output(p_out, buff);
		}
		else
		{
			char16_t* const buff = reinterpret_cast<char16_t* const>(core_alloca(buff_size * sizeof(char16_t)));
			aux_debug_output(p_out, std::span<char16_t>(buff, buff_size));
		}
	}
}

} //namespace core

#endif

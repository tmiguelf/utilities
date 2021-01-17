//======== ======== ======== ======== ======== ======== ======== ========
///	\file
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

#include "CoreLib/Core_Console.hpp"
#include "CoreLib/string/core_string_encoding.hpp"

#include <array>
#include <span>

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__unix__)
#include <cstdio>
#include <unistd.h>
#endif

namespace core
{
#if defined(_WIN32)
void console_out::write(std::string_view p_out) const
{
	write(std::u8string_view{reinterpret_cast<const char8_t*>(p_out.data()), p_out.size()});
}

void console_out::write(std::wstring_view p_out) const
{
	DWORD trash;
	WriteConsoleW(m_handle, p_out.data(), static_cast<DWORD>(p_out.size()), &trash, nullptr);
}

void console_out::write(std::u8string_view p_out) const
{
	write(core::UTF8_to_UTF16_faulty(p_out, '?'));
}

void console_out::write(std::u16string_view p_out) const
{
	DWORD trash;
	WriteConsoleW(m_handle, p_out.data(), static_cast<DWORD>(p_out.size()), &trash, nullptr);
}

void console_out::write(std::u32string_view p_out) const
{
	write(core::UCS4_to_UTF16_faulty(p_out, '?'));
}

void console_out::put(char p_out) const
{
	put(static_cast<char16_t>(p_out));
}

void console_out::put(wchar_t p_out) const
{
	put(static_cast<char16_t>(p_out));
}

void console_out::put(char8_t p_out) const
{
	put(static_cast<char16_t>(p_out));
}

void console_out::put(char16_t p_out) const
{
	DWORD trash;
	WriteConsoleW(m_handle, &p_out, 1, &trash, nullptr);
}

void console_out::put(char32_t p_out) const
{
	std::array<char16_t, 2> buff;
	uint8_t size = encode_UTF16(p_out, buff);
	if(size)
	{
		write(std::u16string_view{buff.data(), size});
	}
	else
	{
		put(u'?');
	}
}

const console_out cout{GetStdHandle(STD_OUTPUT_HANDLE)};
const console_out cerr{GetStdHandle(STD_ERROR_HANDLE )};

#elif defined(__unix__)

void console_out::write(std::string_view p_out) const
{
	::write(m_handle, p_out.data(), p_out.size());
}

void console_out::write(std::wstring_view p_out) const
{
	if constexpr (sizeof(wchar_t) == sizeof(char16_t))
	{
		write(std::u16string_view{reinterpret_cast<const char16_t*>(p_out.data()), p_out.size()});
	}
	else if constexpr (sizeof(wchar_t) == sizeof(char32_t))
	{
		write(std::u32string_view{reinterpret_cast<const char32_t*>(p_out.data()), p_out.size()});
	}

	static_assert(sizeof(wchar_t) == sizeof(char16_t) || sizeof(wchar_t) == sizeof(char32_t), "Unsuported wchar_t size");
}

void console_out::write(std::u8string_view p_out) const
{
	::write(m_handle, p_out.data(), p_out.size());
}

void console_out::write(std::u16string_view p_out) const
{
	write(UTF16_to_UTF8_faulty(p_out, '?'));
}

void console_out::write(std::u32string_view p_out) const
{
	write(UCS4_to_UTF8(p_out));
}

void console_out::put(char p_out) const
{
	::write(m_handle, &p_out, 1);
}

void console_out::put(wchar_t p_out) const
{
	if constexpr (sizeof(wchar_t) == sizeof(char16_t))
	{
		put(static_cast<char16_t>(p_out));
	}
	else if constexpr (sizeof(wchar_t) == sizeof(char32_t))
	{
		put(static_cast<char32_t>(p_out));
	}
	
	static_assert(sizeof(wchar_t) == sizeof(char16_t) || sizeof(wchar_t) == sizeof(char32_t), "Unsuported wchar_t size");
}

void console_out::put(char8_t p_out) const
{
	::write(m_handle, &p_out, 1);
}

void console_out::put(char16_t p_out) const
{
	std::array<char8_t, 7> buff;
	uint8_t size = encode_UTF8(static_cast<char32_t>(p_out), std::span<char8_t, 7>{buff});

	if(size)
	{
		::write(m_handle, buff.data(), size);
	}
	else
	{
		put('?');
	}
}

void console_out::put(char32_t p_out) const
{
	std::array<char8_t, 7> buff;
	uint8_t size = encode_UTF8(p_out, std::span<char8_t, 7>{buff});

	if(size)
	{
		::write(m_handle, buff.data(), size);
	}
	else
	{
		put('?');
	}
}


const console_out cout{1};
const console_out cerr{2};

#endif

}

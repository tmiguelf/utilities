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
//======== ======== ======== ======== ======== ======== ======== ========

#include <CoreLib/Core_Console.hpp>

#include <cstdint>
#include <array>
#include <vector>
#include <span>

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__unix__)
#include <cstdio>
#include <unistd.h>
#endif

#include <CoreLib/string/core_string_encoding.hpp>
#include <CoreLib/Core_Alloca.hpp>
#include <CoreLib/string/core_wchar_alias.hpp>


namespace core
{
#if defined(_WIN32)

static constexpr uintptr_t alloca_treshold = 0x8000;

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
	const uintptr_t buff_size = core::_p::UTF8_to_UTF16_faulty_estimate(p_out, '?');

	if(buff_size > alloca_treshold)
	{
		std::vector<char16_t> buff;
		buff.resize(buff_size);
		core::_p::UTF8_to_UTF16_faulty_unsafe(p_out, '?', buff.data());
		write(std::u16string_view{buff.data(), buff_size});
	}
	else
	{
		char16_t* buff = reinterpret_cast<char16_t*>(core_alloca(buff_size * sizeof(char16_t)));
		core::_p::UTF8_to_UTF16_faulty_unsafe(p_out, '?', buff);
		write(std::u16string_view{buff, buff_size});
	}
}

void console_out::write(std::u16string_view p_out) const
{
	DWORD trash;
	WriteConsoleW(m_handle, p_out.data(), static_cast<DWORD>(p_out.size()), &trash, nullptr);
}

void console_out::write(std::u32string_view p_out) const
{
	const uintptr_t buff_size = core::_p::UCS4_to_UTF16_faulty_estimate(p_out, '?');

	if(buff_size > alloca_treshold)
	{
		std::vector<char16_t> buff;
		buff.resize(buff_size);
		core::_p::UCS4_to_UTF16_faulty_unsafe(p_out, '?', buff.data());
		write(std::u16string_view{buff.data(), buff_size});
	}
	else
	{
		char16_t* buff = reinterpret_cast<char16_t*>(core_alloca(buff_size * sizeof(char16_t)));
		core::_p::UCS4_to_UTF16_faulty_unsafe(p_out, '?', buff);
		write(std::u16string_view{buff, buff_size});
	}
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

static constexpr uintptr_t alloca_treshold = 0x10000;

void console_out::write(std::string_view p_out) const
{
	[[maybe_unused]] ssize_t ret = ::write(m_handle, p_out.data(), p_out.size());
}

void console_out::write(std::wstring_view p_out) const
{
	write(std::basic_string_view<wchar_alias>{reinterpret_cast<const wchar_alias*>(p_out.data()), p_out.size()});
}

void console_out::write(std::u8string_view p_out) const
{
	[[maybe_unused]] ssize_t ret = ::write(m_handle, p_out.data(), p_out.size());
}

void console_out::write(std::u16string_view p_out) const
{
	const uintptr_t buff_size = core::_p::UTF16_to_UTF8_faulty_estimate(p_out, '?');

	if(buff_size > alloca_treshold)
	{
		std::vector<char8_t> buff;
		buff.resize(buff_size);
		core::_p::UTF16_to_UTF8_faulty_unsafe(p_out, '?', buff.data());
		write(std::u8string_view{buff.data(), buff_size});
	}
	else
	{
		char8_t* buff = reinterpret_cast<char8_t*>(core_alloca(buff_size));
		core::_p::UTF16_to_UTF8_faulty_unsafe(p_out, '?', buff);
		write(std::u8string_view{buff, buff_size});
	}
}

void console_out::write(std::u32string_view p_out) const
{
	const uintptr_t buff_size = core::_p::UCS4_to_UTF8_faulty_estimate(p_out, '?');

	if(buff_size > alloca_treshold)
	{
		std::vector<char8_t> buff;
		buff.resize(buff_size);
		core::_p::UCS4_to_UTF8_faulty_unsafe(p_out, '?', buff.data());
		write(std::u8string_view{buff.data(), buff_size});
	}
	else
	{
		char8_t* buff = reinterpret_cast<char8_t*>(core_alloca(buff_size));
		core::_p::UCS4_to_UTF8_faulty_unsafe(p_out, '?', buff);
		write(std::u8string_view{buff, buff_size});
	}
}

void console_out::put(char p_out) const
{
	[[maybe_unused]] ssize_t ret = ::write(m_handle, &p_out, 1);
}

void console_out::put(wchar_t p_out) const
{
	put(static_cast<wchar_alias>(p_out));
}

void console_out::put(char8_t p_out) const
{
	[[maybe_unused]] ssize_t ret = ::write(m_handle, &p_out, 1);
}

void console_out::put(char16_t p_out) const
{
	std::array<char8_t, 7> buff;
	uint8_t size = encode_UTF8(static_cast<char32_t>(p_out), std::span<char8_t, 7>{buff});

	if(size)
	{
		write(std::u8string_view{buff.data(), size});
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
		write(std::u8string_view{buff.data(), size});
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

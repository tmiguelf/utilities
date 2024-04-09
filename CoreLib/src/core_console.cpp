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
//======== ======== ======== ======== ======== ======== ======== ========

#include <CoreLib/core_console.hpp>

#include <CoreLib/core_extra_compiler.hpp>

#include <cstdint>
#include <array>
#include <vector>
#include <span>
#include <limits>

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
	static constexpr uintptr_t alloca_treshold = 0x10000;

	void console_out::write(std::string_view const p_out) const
	{
		write(std::u8string_view{reinterpret_cast<const char8_t*>(p_out.data()), p_out.size()});
	}

	void console_out::write(std::wstring_view const p_out) const
	{
		write(std::basic_string_view<wchar_alias>{reinterpret_cast<const wchar_alias*>(p_out.data()), p_out.size()});
	}

	NO_INLINE void console_out::write(std::u16string_view const p_out) const
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
			char8_t* const buff = reinterpret_cast<char8_t* const>(core_alloca(buff_size));
			core::_p::UTF16_to_UTF8_faulty_unsafe(p_out, '?', buff);
			write(std::u8string_view{buff, buff_size});
		}
	}

	NO_INLINE void console_out::write(std::u32string_view const p_out) const
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
			char8_t* const buff = reinterpret_cast<char8_t* const>(core_alloca(buff_size));
			core::_p::UCS4_to_UTF8_faulty_unsafe(p_out, '?', buff);
			write(std::u8string_view{buff, buff_size});
		}
	}

	void console_out::put(const char p_out) const
	{
		put(static_cast<char8_t>(p_out));
	}

	void console_out::put(const wchar_t p_out) const
	{
		put(static_cast<wchar_alias>(p_out));
	}

	void console_out::put(const char16_t p_out) const
	{
		std::array<char8_t, 4> buff;
		const uint8_t size = encode_UTF8(static_cast<char32_t>(p_out), buff);
		if(size)
		{
			write(std::u8string_view{buff.data(), size});
		}
		else
		{
			put(u8'?');
		}
	}

	void console_out::put(const char32_t p_out) const
	{
		std::array<char8_t, 4> buff;
		const uint8_t size = encode_UTF8(p_out, buff);
		if(size)
		{
			write(std::u8string_view{buff.data(), size});
		}
		else
		{
			put(u8'?');
		}
	}

} //namespace core




namespace core
{
#if defined(_WIN32)

NO_INLINE void console_out::write(std::u8string_view const p_out) const
{
	if(p_out.size() > std::numeric_limits<DWORD>::max())
	{
		return;
	}
	DWORD remaining = static_cast<DWORD>(p_out.size());
	DWORD writen = 0;
	const char8_t* pivot = p_out.data();
	while(true)
	{
		BOOL const result = WriteFile(m_handle, pivot, remaining, &writen, nullptr);
		if(!result || !writen)
		{
			break;
		}
		if(writen < remaining)
		{
			pivot += writen;
			remaining -= writen;
		}
		else
		{
			break;
		}
	};
}

void console_out::put(const char8_t p_out) const
{
	DWORD trash;
	WriteFile(m_handle, &p_out, 1, &trash, nullptr);
}

const console_out cout{GetStdHandle(STD_OUTPUT_HANDLE)};
const console_out cerr{GetStdHandle(STD_ERROR_HANDLE )};

#elif defined(__unix__)

void console_out::write(std::u8string_view const p_out) const
{
	if(p_out.size() > std::numeric_limits<ssize_t>::max())
	{
		return;
	}
	size_t remaining = static_cast<size_t>(p_out.size());
	const char8_t* pivot = p_out.data();
	while(true)
	{
		ssize_t const writen = ::write(m_handle, pivot, remaining);
		if(writen < 1)
		{
			break;
		}
		if(static_cast<size_t>(writen) < remaining)
		{
			pivot += writen;
			remaining -= writen;
		}
		else
		{
			break;
		}
	}
}

void console_out::put(const char8_t p_out) const
{
	[[maybe_unused]] ssize_t ret = ::write(m_handle, &p_out, 1);
}

const console_out cout{1};
const console_out cerr{2};

#endif

}

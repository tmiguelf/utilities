//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///		String suitable for OS operations
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

#include <CoreLib/string/core_os_string.hpp>

#include <span>

namespace core
{
#ifdef _WIN32
	static uintptr_t to_os_natural_size(std::u32string_view const p_string)
	{
		uintptr_t count = 0;

		for(const char32_t tchar : p_string)
		{
			if(tchar > 0xFFFF)
			{
				if(tchar > 0x10FFFF)
				{
					return 0;
				}
				count += 2;
			}
			else
			{
				++count;
			}
		}
		return count;
	}

	static uintptr_t from_os_natural_size(std::u16string_view const p_string)
	{
		uintptr_t count = 0;
		const char16_t* pos = p_string.data();
		const char16_t* const end = pos + p_string.size();

		for(; pos < end; ++pos, ++count)
		{
			const char16_t tchar = *pos;
			if((tchar & 0xFC00) == 0xD800)
			{
				const char16_t* pos_next = pos + 1;
				if(pos_next < end && ((*pos_next) & 0xFC00) == 0xDC00)
				{
					++pos;
				}
			}
		}
		return count;
	}

	static uint8_t encode_UTF16(char32_t p_char, std::span<char16_t, 2> const p_output)
	{
		if(p_char > 0x10FFFF)
		{
			return 0;
		}
		if(p_char > 0xFFFF)
		{
			p_char -= 0x010000;
			p_output[0] = static_cast<char16_t>(((p_char & 0xFFC00) >> 10) | 0xD800);
			p_output[1] = static_cast<char16_t>((p_char & 0x003FF) | 0xDC00);
			return 2;
		}
		p_output[0] = static_cast<char16_t>(p_char);
		return 1;
	}

	std::basic_string<os_char> to_os_natural_convert(std::u32string_view const p_string)
	{
		const uintptr_t reqSize = to_os_natural_size(p_string);
		if(reqSize == 0) return {};

		std::wstring buff;
		buff.resize(reqSize);

		char16_t* pivot = reinterpret_cast<char16_t*>(buff.data());

		for(const char32_t tchar : p_string)
		{
			pivot += encode_UTF16(tchar, std::span<char16_t, 2>{pivot, 2});
		}

		return buff;
	}

	std::u32string from_os_natural_convert(std::basic_string_view<os_char> const p_string)
	{
		std::u16string_view const t_string{reinterpret_cast<const char16_t*>(p_string.data()), p_string.size()};
		const uintptr_t reqSize = from_os_natural_size(t_string);
		if(reqSize == 0) return {};

		std::u32string buff;
		buff.resize(reqSize);
		char32_t* pivot = buff.data();

		const char16_t* pos = t_string.data();
		const char16_t* const end = pos + t_string.size();

		for(; pos < end; ++pos)
		{
			const char16_t tchar = *pos;
			if((tchar & 0xFC00) == 0xD800)
			{
				const char16_t* pos_next = pos + 1;
				if(pos_next < end && ((*pos_next) & 0xFC00) == 0xDC00)
				{
					*(pivot++) = static_cast<char32_t>((((tchar & 0x03FF) << 10) | (*pos_next & 0x03FF)) + 0x10000);
					++pos;
					continue;
				}
			}
			*(pivot++) = static_cast<char32_t>(tchar);
		}
		return buff;
	}
#endif
} //namespace core

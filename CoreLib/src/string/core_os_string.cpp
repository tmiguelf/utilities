//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///		String suitable for OS operations
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
///		
///	\todo	Provide a comprehensive and consistent set of error codes, to give
///			extra information regarding he nature of the failure
//======== ======== ======== ======== ======== ======== ======== ========

#include "CoreLib/string/core_os_string.hpp"

namespace core
{

#ifdef _WIN32

static uintptr_t requiredConversionSize(std::u32string_view p_string)
{
	uintptr_t count = 0;

	for(char32_t tchar : p_string)
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

static std::basic_string<os_char> to_os_natural_convert(std::u32string_view p_string)
{
	uintptr_t reqSize = requiredConversionSize(p_string);
	if(reqSize == 0) return {};

	std::basic_string<os_char> buff;
	buff.resize(reqSize);

	os_char* pivot = buff.data();

	for(char32_t tchar : p_string)
	{
		pivot += encode_UTF16(tchar, std::span<char16_t, 2>{reinterpret_cast<char16_t*>(pivot), 2});
	}

	return buff;
}

static char32_t extract_code_point(const char16_t*& p_input, const char16_t* const p_end)
{
	const char16_t* pivot = p_input;
	if(*pivot > 0xD7FF && *pivot < 0xE000)
	{
		if((p_end - pivot < 1) ||
			(*pivot & 0xFC00) != 0xD800 ||
			(pivot[1] & 0xFC00) != 0xDC00)
		{
			return *p_input;
		}

		++p_input;
		return (((char32_t{*pivot} & 0x03FF) << 10) | (char32_t{pivot[1]} & 0x03FF)) + 0x10000;
	}
	return *p_input;
}

static std::u32string from_os_natural_convert(std::basic_string_view<os_char> p_string)
{
	std::u32string buff;
	buff.reserve(p_string.size());

	const char16_t* pos = reinterpret_cast<const char16_t*>(p_string.data());
	const char16_t* const end = pos + p_string.size();
	for(; pos < end; ++pos)
	{
		buff.push_back(extract_code_point(pos, end));
	}
	return buff;
}

os_string::os_string(std::u32string_view p_string): basic_string<os_char>(to_os_natural_convert(p_string)) {}

os_string& os_string::operator = (std::u32string_view p_string)
{
	this_string_t::operator= (to_os_natural_convert(p_string));
	return *this;
}

std::u32string os_string::to_convertible() const
{
	return from_os_natural_convert(*this);
}

std::u32string os_string_view::to_convertible() const
{
	return from_os_natural_convert(*this);
}

#endif

} //namespace core

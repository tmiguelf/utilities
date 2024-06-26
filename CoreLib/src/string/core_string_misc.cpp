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

#include <CoreLib/string/core_string_misc.hpp>

namespace core
{

//======== ======== Private ======== ========

template <typename T = char32_t>
static inline bool isUpper		(T const p_char) { return (p_char >= 'A' && p_char <= 'Z'); }

template <typename T = char32_t>
static inline bool isLower		(T const p_char) { return (p_char >= 'a' && p_char <= 'z'); }

void toLowerCase(std::span<char8_t> const p_str)
{
	char8_t* pos = p_str.data();
	char8_t const* const end = pos + p_str.size();
	for(; pos < end; ++pos)
	{
		if(isUpper(*pos)) *pos += ('a' - 'A');
	}
}

void toUpperCase(std::span<char8_t> const p_str)
{
	char8_t* pos = p_str.data();
	char8_t const* const end = pos + p_str.size();
	for(; pos < end; ++pos)
	{
		if(isLower(*pos)) *pos -= ('a' - 'A');
	}
}

std::u8string toLowerCaseX(std::u8string_view const p_str)
{
	std::u8string output(p_str);
	toLowerCase(std::span<char8_t>{output});
	return output;
}

std::u8string toUpperCaseX(std::u8string_view const p_str)
{
	std::u8string output(p_str);
	toUpperCase(std::span<char8_t>{output});
	return output;
}

bool compareNoCase(char8_t const* p_str1, char8_t const* p_str2, uintptr_t const p_size)
{
	char8_t const* const end = p_str1 + p_size;
	for(; p_str1 < end ; ++p_str1, ++p_str2)
	{
		if(*p_str1 != *p_str2)
		{
			if(isUpper(*p_str1))
			{
				if((*p_str1 + ('a' - 'A')) != *p_str2) return false;
			}
			else if(isLower(*p_str1))
			{
				if((*p_str1 - ('a' - 'A')) != *p_str2) return false;
			}
			else return false;
		}
	}
	return true;
}

bool string_star_match(std::u8string_view const p_line, std::u8string_view const p_star)
{
	uintptr_t pivotStar	= 0;
	uintptr_t pivotLine	= 0;
	uintptr_t pos		= 0;

	std::u8string_view subString;

	pos = p_star.find(u8'*', pivotStar);
	if(pos != std::u8string::npos)
	{
		if(pos !=0 )
		{
			subString = p_star.substr(0, pos);
			if(p_line.substr(0, pos).compare(subString) != 0)
			{
				return false;
			}
			pivotLine += subString.size();
		}
		pivotStar += pos + 1;
	}
	else
	{
		return (p_line.compare(p_star) == 0);
	}
	while(pivotStar < p_star.size())
	{
		pos = p_star.find(u8'*', pivotStar);
		if(pos != std::u8string::npos)
		{
			if(pos != 0)
			{
				subString = p_star.substr	(pivotStar, pos - pivotStar);
				pivotLine = p_line.find	(subString, pivotLine);
				if(pivotLine == std::u8string::npos)
				{
					return false;
				}
				pivotLine += subString.size();
			}
			pivotStar = pos + 1;
		}
		else
		{
			subString = p_star.substr	(pivotStar, pos - pivotStar);
			pivotLine = p_line.find	(subString, pivotLine);
			return !(pivotLine == std::u8string::npos || pivotLine != p_line.size() - p_star.substr(pivotStar, std::u8string::npos).size());
		}
	}
	return true;
}


} //namespace core

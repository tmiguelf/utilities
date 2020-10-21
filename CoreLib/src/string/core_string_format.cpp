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

#include <CoreLib/string/core_string_format.hpp>

#ifdef _WIN32
#include <intrin.h>
#endif

namespace core
{

//======== ======== Private ======== ========
static bool __Extract_UTF8_code_point(const char8_t*& p_input, const char8_t* const p_end, char32_t& p_code)
{
	if((*p_input & 0x80) == 0) //level 0
	{
		p_code = *p_input;
		return true;
	}

	if((*p_input & 0xC0) == 0x80) return false;

	const char8_t* codeStart = p_input;

	if((*p_input & 0xE0) == 0xC0) //level 1
	{
		if( p_end - p_input <= 1	||
			(*++p_input & 0xC0) != 0x80) return false;

		p_code = *codeStart & 0x1F;
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		return true;
	}

	if((*p_input & 0xF0) == 0xE0) //level 2
	{
		if(	p_end - p_input <= 2		||
			(*++p_input & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	) return false;

		p_code = *codeStart & 0x0F;
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);

		return true;
	}

	if((*p_input & 0xF8) == 0xF0) //level 3
	{
		if(	p_end - p_input <= 3		||
			(*++p_input & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	) return false;

		p_code = *codeStart & 0x07;
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);

		return true;
	}

	if((*p_input & 0xFC) == 0xF8) //level 4
	{
		if(	p_end - p_input <= 4		||
			(*++p_input & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	) return false;

		p_code = *codeStart & 0x03;
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		return true;
	}

	if((*p_input & 0xFE) == 0xFC) //level 5
	{
		if(	p_end - p_input <= 5		||
			(*++p_input & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	)	return false;

		p_code = *codeStart & 0x01;
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		return true;
	}

	if((*p_input & 0xFF) == 0xFE) //level 6
	{
		if(	p_end - p_input <= 6		||
			(*++p_input & 0x3F)  > 0x03	||
			(*p_input   & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	)	return false;

		p_code = (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);

		return true;
	}
	return false;
}

static bool __Extract_UTF16_code_point(const char16_t*& p_input, const char16_t* const p_end, char32_t& p_code)
{
	if(*p_input > 0xD7FF && *p_input < 0xE000)
	{
		if(	(p_input + 1 >= p_end)				||
			(*p_input & 0xFC00)		!= 0xD800	||
			(p_input[1] & 0xFC00)	!= 0xDC0)	return false;

		p_code = (((*p_input & 0x03FF) << 10) | (p_input[1] & 0x03FF)) + 0x10000;
		++p_input;
	}
	else
	{
		p_code = *p_input;
	}
	return true;
}

uint8_t __Encode_UTF8(char32_t p_char, std::span<char8_t, 7> p_output)
{
	if(p_char < 0x00000080) //Level 0
	{
		p_output[0] = static_cast<char8_t>(p_char);
		return 1;
	}

	if(p_char < 0x00000800) //Level 1
	{
		p_output[0] = static_cast<char8_t>( (p_char >>  6			) | 0xC0);
		p_output[1] = static_cast<char8_t>(( p_char		& 0x3F	) | 0x80);
		return 2;
	}

	if(p_char < 0x00010000) //Level 2
	{
		p_output[0] = static_cast<char8_t>( (p_char >> 12			) | 0xE0);
		p_output[1] = static_cast<char8_t>(((p_char >>  6)	& 0x3F	) | 0x80);
		p_output[2] = static_cast<char8_t>(( p_char			& 0x3F	) | 0x80);
		return 3;
	}

	if(p_char < 0x00200000) //Level 3
	{
		p_output[0] = static_cast<char8_t>( (p_char >> 18			) | 0xF0);
		p_output[1] = static_cast<char8_t>(((p_char >> 12)	& 0x3F	) | 0x80);
		p_output[2] = static_cast<char8_t>(((p_char >>  6)	& 0x3F	) | 0x80);
		p_output[3] = static_cast<char8_t>(( p_char			& 0x3F	) | 0x80);
		return 4;
	}

	if(p_char < 0x04000000) //Level 4
	{
		p_output[0] = static_cast<char8_t>( (p_char >> 24			) | 0xF8);
		p_output[1] = static_cast<char8_t>(((p_char >> 18)	& 0x3F	) | 0x80);
		p_output[2] = static_cast<char8_t>(((p_char >> 12)	& 0x3F	) | 0x80);
		p_output[3] = static_cast<char8_t>(((p_char >>  6)	& 0x3F	) | 0x80);
		p_output[4] = static_cast<char8_t>(( p_char			& 0x3F	) | 0x80);
		return 5;
	}

	if(p_char < 0x80000000) //Level 5
	{
		p_output[0] = static_cast<char8_t>(( p_char >> 30			) | 0xFC);
		p_output[1] = static_cast<char8_t>(((p_char >> 24)	& 0x3F	) | 0x80);
		p_output[2] = static_cast<char8_t>(((p_char >> 18)	& 0x3F	) | 0x80);
		p_output[3] = static_cast<char8_t>(((p_char >> 12)	& 0x3F	) | 0x80);
		p_output[4] = static_cast<char8_t>(((p_char >>  6)	& 0x3F	) | 0x80);
		p_output[5] = static_cast<char8_t>(( p_char			& 0x3F	) | 0x80);
		return 6;
	}

	//Level 6
	p_output[0] = char8_t			{0xFE						};
	p_output[1] = static_cast<char8_t>(((p_char >> 30)	& 0x3F	) | 0x80);
	p_output[2] = static_cast<char8_t>(((p_char >> 24)	& 0x3F	) | 0x80);
	p_output[3] = static_cast<char8_t>(((p_char >> 18)	& 0x3F	) | 0x80);
	p_output[4] = static_cast<char8_t>(((p_char >> 12)	& 0x3F	) | 0x80);
	p_output[5] = static_cast<char8_t>(((p_char >>  6)	& 0x3F	) | 0x80);
	p_output[6] = static_cast<char8_t>(( p_char			& 0x3F	) | 0x80);
	return 7;
}

uint8_t __Encode_UTF16(char32_t p_char, std::span<char16_t, 2> p_output)
{
	if((p_char > 0xD7FF && p_char < 0xE000) || p_char > 0x10FFFF)
	{
		return 0;
	}
	if(p_char > 0xFFFF)
	{
		p_char -= 0x010000;
		p_output[0] = static_cast<char16_t>(((p_char & 0xFFC00) >> 10)	| 0xD800);
		p_output[1] = static_cast<char16_t>( (p_char & 0x003FF)			| 0xDC00);
		return 2;
	}
	p_output[0] = static_cast<char16_t>(p_char);
	return 1;
}


//======== ======== Public ======== ========

bool UTF8_UNICODE_Compliant(std::u8string_view p_input)
{
	const char8_t* pos = p_input.data();
	const char8_t* const end = pos + p_input.size();
	for(; pos < end; ++pos)
	{
		if(*pos & 0x80)
		{
			if((*pos & 0xC0) == 0x80)
			{ //invalid encoding
				return false;
			}
			else if((*pos & 0xE0) == 0xC0) //level 1
			{
				if(pos + 1 >= end)					return false;	//validate size
				if((pos[1] & 0xC0) != 0x80)			return false;	//validate encoding
				if((*pos & 0x1F) < 0x02)			return false;	//validate range
				++pos;
			}
			else if((*pos & 0xF0) == 0xE0) //level 2
			{
				if(pos + 2 >= end)					return false;	//validate size
				if(	((pos[1] & 0xC0) != 0x80)	||
					((pos[2] & 0xC0) != 0x80)	)	return false;	//validate encoding

																		//validate range
				if(((*pos & 0x0F) == 0) &&
					((pos[1] & 0x3F) < 0x20))		return false;

				char16_t temp = ((*pos & 0x0F) << 12) | ((pos[1] & 0x3F) << 6) | (pos[2] & 0x3F);
				if(!UNICODE_Compliant(temp))		return false;

				pos += 2;
			}
			else if((*pos & 0xF8) == 0xF0) //level 3
			{
				if(pos + 3 >= end)					return false;	//validate size
				if(	((pos[1] & 0xC0) != 0x80)	||
					((pos[2] & 0xC0) != 0x80)	||
					((pos[3] & 0xC0) != 0x80)	)	return false;	//validate encoding

																		//validate range
				if(((*pos & 0x07) == 0) &&
					((pos[1] & 0x3F) < 0x10))		return false;

				char32_t temp = ((*pos & 0x07) << 18) | ((pos[1] & 0x3F) << 12) | ((pos[2] & 0x3F) << 6) | (pos[3] & 0x3F);
				if(!UNICODE_Compliant(temp))		return false;
				pos += 3;
			}
			return false;
		}
		//else level 0 always ok
	}
	return true;
}

bool UTF16_UNICODE_Compliant(std::u16string_view p_input)
{
	const char16_t* pos = p_input.data();
	const char16_t* const end = pos + p_input.size();
	for(; pos < end; ++pos)
	{
		if((*pos > 0xD7FF && *pos < 0xE000))
		{
			if(pos + 1 >= end)			return false;	//validate size
			if(	(*pos & 0xFC) != 0xD8 ||
				(pos[1] & 0xFC) != 0xDC )	return false; 	//validate encoding
			++pos;
		}
	}
	return true;
}

bool UCS2_UNICODE_Compliant(std::u16string_view p_input)
{
	for(const char16_t tchar: p_input)
	{
		if(!UNICODE_Compliant(tchar)) return false;
	}
	return true;
}

bool UCS4_UNICODE_Compliant(std::u32string_view p_input)
{
	for(const char32_t tchar: p_input)
	{
		if(!UNICODE_Compliant(tchar)) return false;
	}
	return true;
}

bool UTF8_valid(std::u8string_view p_input)
{
	//intrinsics acelerated algorithm
	const char8_t* pos = p_input.data();
	const char8_t* const end = pos + p_input.size();
	for(; pos < end; ++pos)
	{
		if(*pos & 0x80)
		{
			uint32_t index = 0;
#ifdef _WIN32
			static_assert(sizeof(unsigned long) == sizeof(uint32_t));
			if(!_BitScanReverse(reinterpret_cast<unsigned long*>(&index), ~*pos & 0xFF)) return false;
#else
			uint32_t test = ~*pos & 0xFF;
			if(!test) return false;
			index = 31 - __builtin_clz(test);
#endif

			if(end - pos <= (5 - index)) return false;
			switch(index)
			{
				case 0:
					if((*(++pos) & 0xC0) != 0x80 || ((*pos & 0x3F)  > 0x03)) return false;
				case 1:	if((*(++pos) & 0xC0) != 0x80) return false;
				case 2:	if((*(++pos) & 0xC0) != 0x80) return false;
				case 3:	if((*(++pos) & 0xC0) != 0x80) return false;
				case 4:	if((*(++pos) & 0xC0) != 0x80) return false;
				case 5:	if((*(++pos) & 0xC0) != 0x80) return false;
					break;
				default:
					return false;
			}
		}
	}
	return true;

	//old more logical algorithm
#if 0
	const char8_t* pos = p_input.data();
	const char8_t* const end = pos + p_input.size();
	for(; pos < end; ++pos)
	{
		if(*pos & 0x80)
		{
			if((*pos & 0xC0) == 0x80)
			{
				return false;
			}
			if((*pos & 0xE0) == 0xC0) //level 1
			{
				if(	1 >= end - pos ||
					(*++pos & 0xC0) != 0x80 )return false;
				++pos;
				continue;
			}
			if((*pos & 0xF0) == 0xE0) //level 2
			{
				if(	2 >= end - pos			||
					(*++pos & 0xC0) != 0x80	||
					(*++pos & 0xC0) != 0x80	) return false;
				pos += 2;
				continue;
			}
			if((*pos & 0xF8) == 0xF0) //level 3
			{
				if(	3 >= end - pos			||
					(*++pos & 0xC0) != 0x80	||
					(*++pos & 0xC0) != 0x80	||
					(*++pos & 0xC0) != 0x80	) return false;
				pos += 3;
				continue;
			}
			if((*pos & 0xFC) == 0xF8) //level 4
			{
				if(	4 >= end - pos			||
					(*++pos & 0xC0) != 0x80	||
					(*++pos & 0xC0) != 0x80	||
					(*++pos & 0xC0) != 0x80	||
					(*++pos & 0xC0) != 0x80	) return false;
				pos += 4;
				continue;
			}
			if((*pos & 0xFE) == 0xFC) //level 5
			{
				if(	5 >= end - pos			||
					(*++pos & 0xC0) != 0x80	||
					(*++pos & 0xC0) != 0x80	||
					(*++pos & 0xC0) != 0x80	||
					(*++pos & 0xC0) != 0x80	||
					(*++pos & 0xC0) != 0x80	) return false;
				continue;
			}
			if((*pos & 0xFF) == 0xFE) //level 6
			{
				if(	6 >= end - pos			||
					(*++pos & 0x3F)  > 0x03	||
					(  *pos & 0xC0) != 0x80	||
					(*++pos & 0xC0) != 0x80	||
					(*++pos & 0xC0) != 0x80	||
					(*++pos & 0xC0) != 0x80	||
					(*++pos & 0xC0) != 0x80	||
					(*++pos & 0xC0) != 0x80	) return false;
				continue;
			}
			return false;
		}
	}
	return true;
#endif
}

bool ASCII_Compliant(std::u8string_view p_input)
{
	for(const char8_t tchar: p_input)
	{
		if(tchar > 0x7F) return false;
	}
	return true;
}

bool ASCII_Compliant(std::u32string_view p_input)
{
	for(const char32_t tchar: p_input)
	{
		if(tchar > 0x7F) return false;
	}
	return true;
}

encodeResult<char8_t> UTF8_to_ANSI(std::u8string_view p_input)
{
	char32_t t_char;
	std::u8string output;
	output.reserve(p_input.size());

	const char8_t* pos = p_input.data();
	const char8_t* const end = pos + p_input.size();

	for(; pos < end; ++pos)
	{
		if(!__Extract_UTF8_code_point(pos, end, t_char))
		{
			return {};
		}
		if(t_char > 0xFF) return {};
		output.push_back(static_cast<const char8_t>(t_char));
	}
	return output;
}

encodeResult<char8_t> UTF16_to_ANSI(std::u16string_view p_input)
{
	char32_t t_char;
	std::u8string output;
	output.reserve(p_input.size());

	const char16_t* pos = p_input.data();
	const char16_t* const end = pos + p_input.size();

	for(; pos < end; ++pos)
	{
		if(!__Extract_UTF16_code_point(pos, end, t_char))
		{
			return {};
		}
		if(t_char > 0xFF) return {};
		output.push_back(static_cast<const char8_t>(t_char));
	}
	return output;
}

encodeResult<char8_t> UCS2_to_ANSI(std::u16string_view p_input)
{
	for(const char16_t t_char: p_input)
	{
		if(t_char > 0xFF) return {};
	}
	
	std::u8string output;
	output.resize(p_input.size());

	size_t pos = 0;
	for(const char16_t t_char: p_input)
	{
		output[pos++] = static_cast<const char8_t>(t_char);
	}
	return output;
}

encodeResult<char8_t> UCS4_to_ANSI(std::u32string_view p_input)
{
	for(const char32_t t_char: p_input)
	{
		if(t_char > 0xFF) return {};
	}
	
	std::u8string output;
	output.resize(p_input.size());

	size_t pos = 0;
	for(const char32_t t_char: p_input)
	{
		output[pos++] = static_cast<const char8_t>(t_char);
	}

	return output;
}

std::u8string ANSI_to_UTF8(std::u8string_view p_input)
{
	char8_t buffer[7];
	std::u8string output;

	const char8_t* pos = p_input.data();
	const char8_t* const end = pos + p_input.size();
	for(; pos < end; ++pos)
	{
		output.append(buffer, __Encode_UTF8(*pos, buffer));
	}
	return output;
}

encodeResult<char8_t> UTF16_to_UTF8(std::u16string_view p_input)
{
	char32_t t_char;
	char8_t buffer[7];
	std::u8string output;

	const char16_t* pos = p_input.data();
	const char16_t* const end = pos + p_input.size();
	for(; pos < end; ++pos)
	{
		if(!__Extract_UTF16_code_point(pos, end, t_char))
		{
			return {};
		}
		output.append(buffer, __Encode_UTF8(t_char, buffer));
	}
	return output;
}

std::u8string UCS2_to_UTF8(std::u16string_view p_input)
{
	char8_t buffer[7];
	std::u8string output;

	const char16_t* pos = p_input.data();
	const char16_t* const end = pos + p_input.size();
	for(; pos < end; ++pos)
	{
		output.append(buffer, __Encode_UTF8(*pos, buffer));
	}
	return output;
}

std::u8string UCS4_to_UTF8(std::u32string_view p_input)
{
	char8_t buffer[7];
	std::u8string output;

	const char32_t* pos = p_input.data();
	const char32_t* const end = pos + p_input.size();
	for(; pos < end; ++pos)
	{
		output.append(buffer, __Encode_UTF8(*pos, buffer));
	}
	return output;
}

std::u16string ANSI_to_UTF16(std::u8string_view p_input)
{
	std::u16string output;
	output.resize(p_input.size());
	size_t pos = 0;
	for(char8_t tchar: p_input)
	{
		output[pos++] = static_cast<char16_t>(tchar);
	}
	return output;
}

encodeResult<char16_t> UTF8_to_UTF16(std::u8string_view p_input)
{
	char32_t t_char;
	char16_t buff[2];
	uint8_t ret;

	std::u16string output;

	const char8_t* pos = p_input.data();
	const char8_t* const end = pos + p_input.size();
	for(; pos < end; ++pos)
	{
		if(!__Extract_UTF8_code_point(pos, end, t_char))
		{
			return {};
		}
		ret = __Encode_UTF16(t_char, buff);
		if(ret == 0) return {};
		output.append(buff, ret);
	}
	return output;
}

encodeResult<char16_t> UCS2_to_UTF16(std::u16string_view p_input)
{
	char16_t buff[2];
	uint8_t ret;
	std::u16string output;

	for(char16_t tchar: p_input)
	{
		ret = __Encode_UTF16(tchar, buff);
		if(ret == 0) return {};
		output.append(buff, ret);
	}
	return output;
}

encodeResult<char16_t> UCS4_to_UTF16(std::u32string_view p_input)
{
	char16_t buff[2];
	uint8_t ret;
	std::u16string output;

	for(char32_t tchar: p_input)
	{
		ret = __Encode_UTF16(tchar, buff);
		if(ret == 0) return {};
		output.append(buff, ret);
	}
	return output;
}

std::u16string ANSI_to_UCS2(std::u8string_view p_input)
{
	std::u16string output;
	output.resize(p_input.size());

	size_t pos = 0;
	for(char8_t tchar: p_input)
	{
		output[pos++] = static_cast<char16_t>(tchar);
	}
	return output;
}

encodeResult<char16_t> UTF8_to_UCS2(std::u8string_view p_input)
{
	char32_t t_char;
	std::u16string output;
	output.reserve(p_input.size());

	const char8_t* pos = p_input.data();
	const char8_t* const end = pos + p_input.size();

	for(; pos < end; ++pos)
	{
		if(!__Extract_UTF8_code_point(pos, end, t_char))
		{
			return {};
		}
		if(t_char > 0xFFFF) return {};
		output.push_back(static_cast<const char16_t>(t_char));
	}
	return output;
}

encodeResult<char16_t> UTF16_to_UCS2(std::u16string_view p_input)
{
	char32_t t_char;
	std::u16string output;
	output.reserve(p_input.size());

	const char16_t* pos = p_input.data();
	const char16_t* const end = pos + p_input.size();

	for(; pos < end; ++pos)
	{
		if(!__Extract_UTF16_code_point(pos, end, t_char))
		{
			return {};
		}
		if(t_char > 0xFFFF) return {};
		output.push_back(static_cast<char16_t>(t_char));
	}
	return output;
}

encodeResult<char16_t> UCS4_to_UCS2(std::u32string_view p_input)
{
	for(const char32_t t_char: p_input)
	{
		if(t_char > 0xFFFF) return {};
	}
	std::u16string output;
	output.resize(p_input.size());

	size_t pos = 0;
	for(const char32_t t_char: p_input)
	{
		output[pos++] = static_cast<const char16_t>(t_char);
	}
	return output;
}

std::u32string ANSI_to_UCS4(std::u8string_view p_input)
{
	std::u32string output;
	output.resize(p_input.size());
	size_t pos = 0;
	for(char8_t tchar: p_input)
	{
		output[pos++] = static_cast<char32_t>(tchar);
	}
	return output;
}

encodeResult<char32_t> UTF8_to_UCS4(std::u8string_view p_input)
{
	char32_t t_char;
	std::u32string output;

	const char8_t* pos = p_input.data();
	const char8_t* const end = pos + p_input.size();
	for(; pos < end; ++pos)
	{
		if(!__Extract_UTF8_code_point(pos, end, t_char))
		{
			return {};
		}
		output.push_back(t_char);
	}
	return output;
}

encodeResult<char32_t> UTF16_to_UCS4(std::u16string_view p_input)
{
	char32_t t_char;
	std::u32string output;

	const char16_t* pos = p_input.data();
	const char16_t* const end = pos + p_input.size();
	for(; pos < end; ++pos)
	{
		if(!__Extract_UTF16_code_point(pos, end, t_char))
		{
			return {};
		}
		output.push_back(t_char);
	}
	return output;
}

std::u32string UCS2_to_UCS4(std::u16string_view p_input)
{
	std::u32string output;
	output.resize(p_input.size());
	size_t pos = 0;
	for(char16_t tchar: p_input)
	{
		output[pos++] = static_cast<char32_t>(tchar);
	}
	return output;
}




std::u8string UTF8_to_ANSI_faulty(std::u8string_view p_input, char8_t p_placeHolder)
{
	char32_t t_char;

	std::u8string output;
	output.reserve(p_input.size());

	const char8_t* pos = p_input.data();
	const char8_t* const end = pos + p_input.size();

	for(; pos < end; ++pos)
	{
		if(__Extract_UTF8_code_point(pos, end, t_char))
		{
			output.push_back(static_cast<const char8_t>(t_char));
		}
		else output.push_back(p_placeHolder);
	}
	return output;
}

std::u8string UTF16_to_ANSI_faulty(std::u16string_view p_input, char8_t p_placeHolder)
{
	char32_t t_char;

	std::u8string output;
	output.reserve(p_input.size());

	const char16_t* pos = p_input.data();
	const char16_t* const end = pos + p_input.size();

	for(; p_input < end; ++pos)
	{
		if(__Extract_UTF16_code_point(pos, end, t_char))
		{
			output.push_back(static_cast<const char8_t>(t_char));
		}
		else output.push_back(p_placeHolder);
	}
	return output;
}

std::u8string UCS2_to_ANSI_faulty(std::u16string_view p_input, char8_t p_placeHolder)
{
	std::u8string output;
	output.resize(p_input.size());

	size_t pos = 0;
	for(const char16_t t_char: p_input)
	{
		if(t_char < 256) output[pos++] = static_cast<const char8_t>(t_char);
		else output[pos++] = p_placeHolder;
	}
	return output;
}

std::u8string UCS4_to_ANSI_faulty(std::u32string_view p_input, char8_t p_placeHolder)
{
	std::u8string output;
	output.resize(p_input.size());

	size_t pos = 0;
	for(const char32_t t_char: p_input)
	{
		if(t_char < 256) output[pos++] = static_cast<const char8_t>(t_char);
		else output[pos++] = p_placeHolder;
	}

	return output;
}


std::u32string UTF8_to_UCS4_faulty(std::u8string_view p_input, char32_t p_placeHolder)
{
	char32_t t_char;
	std::u32string output;

	const char8_t* pos = p_input.data();
	const char8_t* const end = pos + p_input.size();
	for(; pos < end; ++pos)
	{
		if(__Extract_UTF8_code_point(pos, end, t_char))
		{
			output.push_back(t_char);
		}
		else output.push_back(p_placeHolder);
	}
	return output;
}

std::u32string UTF16_to_UCS4_faulty(std::u16string_view p_input, char32_t p_placeHolder)
{
	char32_t t_char;
	std::u32string output;

	const char16_t* pos = p_input.data();
	const char16_t* const end = pos + p_input.size();
	for(; pos < end; ++pos)
	{
		if(__Extract_UTF16_code_point(pos, end, t_char))
		{
			output.push_back(t_char);
		}
		else output.push_back(p_placeHolder);
	}
	return output;
}


} //namespace core

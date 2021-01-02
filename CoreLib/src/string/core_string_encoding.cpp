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

#include <CoreLib/string/core_string_encoding.hpp>

#include <bit>

namespace core
{

//======== ======== Private ======== ========
static bool __Extract_UTF8_code_point(const char8_t*& p_input, const char8_t* const p_end, char32_t& p_code)
{
	const char8_t* testp = p_input;
	if((*testp & 0x80) == 0) //level 0
	{
		p_code = *testp;
		return true;
	}

	if((*testp & 0xC0) == 0x80) return false;

	const char8_t* codeStart = testp;

	if((*testp & 0xE0) == 0xC0) //level 1
	{
		if( p_end - testp <= 1	||
			(*++testp & 0xC0) != 0x80) return false;

		++p_input;

		p_code = *codeStart & 0x1F;
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		return true;
	}

	if((*testp & 0xF0) == 0xE0) //level 2
	{
		if(	p_end - testp <= 2		||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	) return false;

		p_input += 2;

		p_code = *codeStart & 0x0F;
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);

		return true;
	}

	if((*testp & 0xF8) == 0xF0) //level 3
	{
		if(	p_end - testp <= 3		||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	) return false;

		p_input += 3;

		p_code = *codeStart & 0x07;
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);

		return true;
	}

	if((*testp & 0xFC) == 0xF8) //level 4
	{
		if(	p_end - testp <= 4		||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	) return false;

		p_input += 4;

		p_code = *codeStart & 0x03;
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		return true;
	}

	if((*testp & 0xFE) == 0xFC) //level 5
	{
		if(	p_end - testp <= 5		||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)	return false;

		p_input += 5;

		p_code = *codeStart & 0x01;
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		return true;
	}

	if((*testp & 0xFF) == 0xFE) //level 6
	{
		if(	p_end - testp <= 6		||
			(*++testp & 0x3F)  > 0x03	||
			(*testp   & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)	return false;

		p_input += 6;

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

static bool __Extract_UTF8_code_point_failforward(const char8_t*& p_input, const char8_t* const p_end, char32_t& p_code)
{
	const char8_t* testp = p_input;
	if((*testp & 0x80) == 0) //level 0
	{
		p_code = *testp;
		return true;
	}

	if((*testp & 0xC0) == 0x80) return false;

	const char8_t* codeStart = testp;

	if((*testp & 0xE0) == 0xC0) //level 1
	{
		if( p_end - testp <= 1) return false;
		if((*++testp & 0xC0) != 0x80)
		{
			p_input = testp -1;
			return false;
		}

		++p_input;

		p_code = *codeStart & 0x1F;
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		return true;
	}

	if((*testp & 0xF0) == 0xE0) //level 2
	{
		if(	p_end - testp <= 2) return false;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
			return false;
		}

		p_input += 2;

		p_code = *codeStart & 0x0F;
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);

		return true;
	}

	if((*testp & 0xF8) == 0xF0) //level 3
	{
		if(	p_end - testp <= 3) return false;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
			return false;
		}

		p_input += 3;

		p_code = *codeStart & 0x07;
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);

		return true;
	}

	if((*testp & 0xFC) == 0xF8) //level 4
	{
		if(	p_end - testp <= 4) return false;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
			return false;
		}

		p_input += 4;

		p_code = *codeStart & 0x03;
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		return true;
	}

	if((*testp & 0xFE) == 0xFC) //level 5
	{
		if(	p_end - testp <= 5) return false;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
			return false;
		}

		p_input += 5;

		p_code = *codeStart & 0x01;
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		p_code = p_code << 6 | (*++codeStart & 0x3F);
		return true;
	}

	if((*testp & 0xFF) == 0xFE) //level 6
	{
		if(	p_end - testp <= 6) return false;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
			return false;
		}

		p_input += 6;

		if((*++codeStart & 0x3F) > 0x03) return false;
		p_code = (*codeStart & 0x3F);
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
	const char16_t* pivot = p_input;
	if(*pivot > 0xD7FF && *pivot < 0xE000)
	{
		if(	(p_end - pivot < 1 )				||
			(*pivot & 0xFC00)		!= 0xD800	||
			(pivot[1] & 0xFC00)	!= 0xDC00)	return false;

		p_code = (((*pivot & 0x03FF) << 10) | (pivot[1] & 0x03FF)) + 0x10000;
		++p_input;
	}
	else
	{
		p_code = *pivot;
	}
	return true;
}

uint8_t encode_UTF8(char32_t p_char, std::span<char8_t, 7> p_output)
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

uint8_t encode_UTF16(char32_t p_char, std::span<char16_t, 2> p_output)
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
				if(	end - pos < 2			||	//validate size
					(*pos & 0x1F) < 0x02	||	//validate range
					(*++pos & 0xC0) != 0x80) return false;	//validate encoding
			}
			else if((*pos & 0xF0) == 0xE0) //level 2
			{
				const char8_t* decode = pos;

				if(end - pos < 3			||	//validate size
					(((*pos++ & 0x0F) == 0) && ((*pos & 0x3F) < 0x20)) || //validate range
					(*pos & 0xC0) != 0x80	||	//validate encoding
					(*++pos & 0xC0) != 0x80) return false;

				char16_t temp = (*decode & 0x0F);
				temp = static_cast<char16_t>(temp << 6 | (*++decode & 0x3F));
				temp = static_cast<char16_t>(temp << 6 | (*++decode & 0x3F));

				if(temp > 0xD7FF && temp < 0xE000) return false;
			}
			else if((*pos & 0xF8) == 0xF0) //level 3
			{
				const char8_t* decode = pos;
				if(end - pos < 4			||	//validate size
					(((*pos++ & 0x07) == 0) && ((*pos & 0x3F) < 0x10)) ||	//validate range
					(*pos & 0xC0) != 0x80 ||	//validate encoding
					(*++pos & 0xC0) != 0x80 ||
					(*++pos & 0xC0) != 0x80) return false;

				char32_t temp = (*decode & 0x07);
				temp = temp << 6 | (*++decode & 0x3F);
				temp = temp << 6 | (*++decode & 0x3F);
				temp = temp << 6 | (*++decode & 0x3F);
				if(!UNICODE_Compliant(temp)) return false;
			}
			else return false;
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
		if(*pos > 0xD7FF && *pos < 0xE000)
		{
			if(	end - pos < 2 ||
				(*pos   & 0xFC00) != 0xD800 ||
				(*++pos & 0xFC00) != 0xDC00 ) return false; 	//validate encoding
		}
	}
	return true;
}

bool UCS2_UNICODE_Compliant(std::u16string_view p_input)
{
	for(const char16_t tchar: p_input)
	{
		if(tchar > 0xD7FF && tchar < 0xE000) return false;
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
			uint32_t index = static_cast<uint32_t>(std::countl_one<uint8_t>(*pos));

			if(static_cast<uintptr_t>(end - pos) < index) return false;
			switch(index)
			{
				case 7: if((*(++pos) & 0xC0) != 0x80 || ((*pos & 0x3F)  > 0x03)) return false;
				case 6: if((*(++pos) & 0xC0) != 0x80) return false;
				case 5: if((*(++pos) & 0xC0) != 0x80) return false;
				case 4: if((*(++pos) & 0xC0) != 0x80) return false;
				case 3: if((*(++pos) & 0xC0) != 0x80) return false;
				case 2: if((*(++pos) & 0xC0) != 0x80) return false;
					break;
				default:
					return false;
			}
		}
	}
	return true;
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
	//Because ANSI code points can not be larger than 0xFF
	//if input has code points larger than 0xFF then it's already an error
	//Otherwise for all code points <0xFF UTF16 == UCS2
	return UCS2_to_ANSI(p_input);
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
		output.append(buffer, encode_UTF8(*pos, buffer));
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
		output.append(buffer, encode_UTF8(t_char, buffer));
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
		output.append(buffer, encode_UTF8(*pos, buffer));
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
		output.append(buffer, encode_UTF8(*pos, buffer));
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
		ret = encode_UTF16(t_char, buff);
		if(ret == 0) return {};
		output.append(buff, ret);
	}
	return output;
}

encodeResult<char16_t> UCS2_to_UTF16(std::u16string_view p_input)
{
	for(char16_t tchar: p_input)
	{
		if(tchar > 0xD7FF && tchar < 0xE000) return {};
	}
	return {std::u16string{p_input}};
}

encodeResult<char16_t> UCS4_to_UTF16(std::u32string_view p_input)
{
	char16_t buff[2];
	uint8_t ret;
	std::u16string output;

	for(char32_t tchar: p_input)
	{
		ret = encode_UTF16(tchar, buff);
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
	for(char16_t tchar: p_input)
	{
		if(tchar > 0xD7FF && tchar < 0xE000) return {};
	}
	return {std::u16string{p_input}};
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
		if(__Extract_UTF8_code_point_failforward(pos, end, t_char) && t_char < 256)
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

	for(; pos < end; ++pos)
	{
		if(__Extract_UTF16_code_point(pos, end, t_char) && t_char < 256)
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

std::u8string UTF16_to_UTF8_faulty(std::u16string_view p_input, char32_t p_placeHolder)
{
	char8_t placeHolder[7];
	uintptr_t placeHolderSize = encode_UTF8(p_placeHolder, placeHolder);

	char32_t t_char;
	char8_t buffer[7];
	std::u8string output;

	const char16_t* pos = p_input.data();
	const char16_t* const end = pos + p_input.size();
	for(; pos < end; ++pos)
	{
		if(__Extract_UTF16_code_point(pos, end, t_char))
		{
			output.append(buffer, encode_UTF8(t_char, buffer));
		}
		else
		{
			output.append(placeHolder, placeHolderSize);
		}
		
	}
	return output;
}

std::u16string UTF8_to_UTF16_faulty(std::u8string_view p_input, char32_t p_placeHolder)
{
	char16_t placeHolder[2];
	uintptr_t placeHolderSize = encode_UTF16(p_placeHolder, placeHolder);

	char32_t t_char;
	char16_t buff[2];
	uint8_t ret;

	std::u16string output;

	const char8_t* pos = p_input.data();
	const char8_t* const end = pos + p_input.size();
	for(; pos < end; ++pos)
	{
		if(__Extract_UTF8_code_point_failforward(pos, end, t_char))
		{
			ret = encode_UTF16(t_char, buff);
			if(ret)
			{
				output.append(buff, ret);
			}
			else output.append(placeHolder, placeHolderSize);
		}
		else output.append(placeHolder, placeHolderSize);
	}
	return output;
}

std::u16string UCS2_to_UTF16_faulty(std::u16string_view p_input, char32_t p_placeHolder)
{
	char16_t placeHolder[2];
	uintptr_t placeHolderSize = encode_UTF16(p_placeHolder, placeHolder);

	std::u16string output;

	for(char16_t tchar: p_input)
	{
		if(tchar > 0xD7FF && tchar < 0xE000)
		{
			output.append(placeHolder, placeHolderSize);
		}
		else
		{
			output.push_back(tchar);
		}
	}
	return output;
}

std::u16string UCS4_to_UTF16_faulty(std::u32string_view p_input, char32_t p_placeHolder)
{
	char16_t placeHolder[2];
	uintptr_t placeHolderSize = encode_UTF16(p_placeHolder, placeHolder);
	char16_t buff[2];
	uint8_t ret;
	std::u16string output;

	for(char32_t tchar: p_input)
	{
		ret = encode_UTF16(tchar, buff);
		if(ret)
		{
			output.append(buff, ret);
		}
		else output.append(placeHolder, placeHolderSize);
	}
	return output;
}

std::u16string UTF8_to_UCS2_faulty(std::u8string_view p_input, char16_t p_placeHolder)
{
	char32_t t_char;
	std::u16string output;
	output.reserve(p_input.size());

	const char8_t* pos = p_input.data();
	const char8_t* const end = pos + p_input.size();

	for(; pos < end; ++pos)
	{
		if(__Extract_UTF8_code_point_failforward(pos, end, t_char))
		{
			if(t_char > 0xFFFF)
			{
				output.push_back(p_placeHolder);
			}
			else output.push_back(static_cast<const char16_t>(t_char));
		}
		else output.push_back(p_placeHolder);
	}
	return output;
}

std::u16string UTF16_to_UCS2_faulty(std::u16string_view p_input, char16_t p_placeHolder)
{
	char32_t t_char;
	std::u16string output;

	const char16_t* pos = p_input.data();
	const char16_t* const end = pos + p_input.size();
	for(; pos < end; ++pos)
	{
		if(__Extract_UTF16_code_point(pos, end, t_char) && t_char < 0x010000)
		{
			output.push_back(static_cast<char16_t>(t_char));
		}
		else output.push_back(p_placeHolder);
	}
	return output;
}

std::u16string UCS4_to_UCS2_faulty(std::u32string_view p_input, char16_t p_placeHolder)
{
	std::u16string output;
	output.resize(p_input.size());

	size_t pos = 0;
	for(const char32_t t_char: p_input)
	{
		output[pos++] = (t_char < 0x010000) ? static_cast<const char16_t>(t_char) : p_placeHolder;
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
		if(__Extract_UTF8_code_point_failforward(pos, end, t_char))
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

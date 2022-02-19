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
//======== ======== ======== ======== ======== ======== ======== ========

#include <CoreLib/string/core_string_encoding.hpp>

#include <array>
#include <bit>
#include <cstring>

namespace core
{

static inline bool __fmove_UTF8_ANSI(const char8_t*& p_input, const char8_t*& p_end)
{
	const char8_t testp = *p_input;
	return !(testp & 0x80) ||
		((testp == 0xC2 || testp == 0xC3) &&	//validate encoding 1, check range and unique representation simultaneously
		(p_end - p_input) > 1	&&				//validate size
		(*(++p_input) & 0xC0) == 0x80);			//validate encoding 2;
}

static inline char8_t __convert_UTF8_ANSI_unsafe(const char8_t*& p_input)
{
	const char8_t testp = *p_input;
	if(testp & 0x80)
	{
		return static_cast<char8_t>((testp << 6) | (*++p_input & 0x3F));
	}
	return testp;
}

static uintptr_t __estimate_UTF8_UTF16(const char8_t*& p_input, const char8_t* const p_end)
{
	const char8_t testp = *p_input;

	if((testp & 0x80) == 0) //level 0
	{
		return 1;
	}

	if((testp & 0xE0) == 0xC0) //level 1
	{
		if(	p_end - p_input < 2	|| //validate size
			testp < 0xC2		|| //validate range
			(*++p_input & 0xC0) != 0x80) //validate encoding
		{
			return 0;
		}
		return 1;
	}

	const char8_t* tlocal = p_input;

	if((testp & 0xF0) == 0xE0) //level 2
	{
		if(p_end - tlocal < 3) //validate size
		{
			return 0;
		}
		const char8_t t1 = testp & 0x0F;
		const char8_t t2 = *++tlocal;
		const char8_t t2m = t2 & 0x20;
		if(
			(!t1 && !t2m)		|| //validate range
			(t1 == 0x0D && t2m)	|| //surrogates
			(t2 & 0xC0) != 0x80	|| //validate encoding
			(*++tlocal & 0xC0) != 0x80
			)
		{
			return 0;
		}

		p_input += 2;
		return 1;
	}

	if((testp & 0xF8) == 0xF0) //level 3
	{
		const char8_t t1 = testp & 0x07;
		if((p_end - tlocal < 4) || t1 > 4) //validate size, and partial upper_range
		{
			return 0;
		}

		const char8_t t2 = *++tlocal;
		if(
			(t1 == 4 && (t2 & 0x30))	|| //validate upper_range
			(!t1 && (t2 < 0x90))		|| //validate sub-range
			(t2 & 0xC0) != 0x80			|| //validate encoding
			(*++tlocal & 0xC0) != 0x80	||
			(*++tlocal & 0xC0) != 0x80)
		{
			return 0;
		}
		p_input += 3;
		return 2;
	}

	return 0;
}

static void __convert_UTF8_UTF16_unsafe(const char8_t*& p_input, char16_t*& p_out)
{
	const char8_t testp = *p_input;

	if((testp & 0x80) == 0) //level 0
	{
		*p_out = static_cast<char16_t>(testp);
	}
	else if((testp & 0xE0) == 0xC0) //level 1
	{
		*p_out =
			static_cast<char16_t>((testp & 0x1F) << 6) |
			static_cast<char16_t>(*++p_input & 0x3F);
	}
	else
	{
		const char8_t* codeStart = p_input;
		if((testp & 0xF0) == 0xE0) //level 2
		{
			p_input += 2;
			const char16_t tcode =
				static_cast<char16_t>(testp << 12) |
				static_cast<char16_t>(static_cast<char16_t>(*++codeStart & 0x3F) << 6);
			*p_out = tcode | static_cast<char16_t>(*++codeStart & 0x3F);
		}
		else //level 3
		{
			p_input += 3;
			char16_t tcode = static_cast<char16_t>(
				(static_cast<char16_t>((testp & 0x07) << 8) |
				static_cast<char16_t>(static_cast<char16_t>(*++codeStart & 0x3F) << 2)) - char16_t{0x40});

			*p_out = 0xD800 | tcode | static_cast<char16_t>(static_cast<char16_t>(*++codeStart & 0x30) >> 4);

			tcode = 0xDC00 | static_cast<char16_t>(static_cast<char16_t>(*codeStart & 0x0F) << 6);
			*++p_out = tcode | static_cast<char16_t>(*++codeStart & 0x3F);
		}
	}
}

static bool __fmove_UTF8_UCS2(const char8_t*& p_input, const char8_t* const p_end)
{
	const char8_t testp = *p_input;
	if((testp & 0x80) == 0) //level 0
	{
		return true;
	}

	if((testp & 0xE0) == 0xC0) //level 1
	{
		if( p_end - p_input < 2	|| //validate size
			testp < 0xC2		|| //validate range
			(*++p_input & 0xC0) != 0x80) //validate encoding
		{
			return false;
		}
		return true;
	}

	if((testp & 0xF0) == 0xE0) //level 2
	{
		const char8_t* tlocal = p_input;
		if(p_end - tlocal < 3			||	//validate size
			(!(*(tlocal++) & 0x0F) && !(*tlocal & 0x20)) || //validate range
			(*tlocal & 0xC0) != 0x80	||	//validate encoding
			(*++tlocal & 0xC0) != 0x80)
		{
			return false;
		}
		p_input += 2;
		return true;
	}
	return false;
}

static char16_t __convert_UTF8_UCS2_unsafe(const char8_t*& p_input)
{
	const char8_t testp = *p_input;

	if((testp & 0x80) == 0) //level 0
	{
		return static_cast<char16_t>(testp);
	}

	if((testp & 0xE0) == 0xC0) //level 1
	{
		return 
			static_cast<char16_t>((testp & 0x1F) << 6) |
			static_cast<char16_t>(*(++p_input) & 0x3F);
	}

	//level 2
	const char8_t* codeStart = p_input;
	p_input += 2;

	const char16_t tcode =
		static_cast<char16_t>((testp & 0x0F) << 12) |
		static_cast<char16_t>(static_cast<char16_t>(*++codeStart & 0x3F) << 6);
	return tcode | static_cast<char16_t>(*++codeStart & 0x3F);
}

static bool __fmove_UTF8_UCS4(const char8_t*& p_input, const char8_t* const p_end)
{
	const char8_t testp = *p_input;
	if(testp & 0x80) //level 0
	{
		if((testp & 0xE0) == 0xC0) //level 1
		{
			if( p_end - p_input < 2		||	//validate size
				testp < 0xC2			||	//validate range
				(*++p_input & 0xC0) != 0x80) //validate encoding
			{
				return false;
			}
		}
		else
		{
			const char8_t* tlocal = p_input;
			if((testp & 0xF0) == 0xE0) //level 2
			{
				if(p_end - tlocal < 3			||	//validate size
					(!(*(tlocal++) & 0x0F) && !(*tlocal & 0x20)) || //validate range
					(*tlocal & 0xC0) != 0x80	||	//validate encoding
					(*++tlocal & 0xC0) != 0x80)
				{
					return false;
				}
				p_input += 2;
			}
			else if((testp & 0xF8) == 0xF0) //level 3
			{
				const char8_t t1 = testp & 0x07;
				if((p_end - tlocal < 4) || t1 > 4) //validate size, and partial upper_range
				{
					return false;
				}

				const char8_t t2 = *++tlocal;
				if(
					(t1 == 4 && (t2 & 0x30))	|| //validate upper_range
					(!t1 && (t2 < 0x90))		|| //validate sub-range
					(t2 & 0xC0) != 0x80			|| //validate encoding
					(*++tlocal & 0xC0) != 0x80	||
					(*++tlocal & 0xC0) != 0x80)
				{
					return false;
				}
				p_input += 3;
			}
			else
			{
				return false;
			}
		}
	}
	return true;
}

static char32_t __convert_UTF8_UCS4(const char8_t*& p_input)
{
	const char8_t testp = *p_input;

	if((testp & 0x80) == 0) //level 0
	{
		return static_cast<char32_t>(testp);
	}

	if((testp & 0xE0) == 0xC0) //level 1
	{
		return 
			static_cast<char32_t>((testp & 0x1F) << 6) |
			static_cast<char32_t>(*(++p_input) & 0x3F);
	}

	const char8_t* codeStart = p_input;

	if((testp & 0xF0) == 0xE0) //level 2
	{
		p_input += 2;
		const char32_t tcode =
			static_cast<char32_t>(testp & 0x0F) << 12 |
			static_cast<char32_t>(*++codeStart & 0x3F) << 6;
		return tcode | static_cast<char32_t>(*++codeStart & 0x3F);
	}

	p_input += 3;
	char32_t tcode =
		static_cast<char32_t>(testp & 0x07) << 18 |
		static_cast<char32_t>(*++codeStart & 0x3F) << 12;
	tcode |= static_cast<char32_t>(*++codeStart & 0x3F) << 6;
	return tcode | static_cast<char32_t>(*++codeStart & 0x3F);
}

static void __fmove_UTF8_failForward(const char8_t*& p_input, const char8_t* const p_end)
{
	const char8_t* testp = p_input;
	if((*testp & 0x80) == 0) //level 0
	{
		return;
	}
	if((*testp & 0xE0) == 0xC0) //level 1
	{
		if( p_end - testp < 2) return;
		if((*++testp & 0xC0) != 0x80)
		{
			return;
		}
		++p_input;
		return;
	}
	if((*testp & 0xF0) == 0xE0) //level 2
	{
		if(	p_end - testp < 3) goto unwind_as_much_as_possible;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
		}
		else p_input += 2;
		return;
	}
	if((*testp & 0xF8) == 0xF0) //level 3
	{
		if(	p_end - testp < 4) goto unwind_as_much_as_possible;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
		}
		else p_input += 3;
		return;
	}
	if((*testp & 0xFC) == 0xF8) //level 4
	{
		if(	p_end - testp < 5) goto unwind_as_much_as_possible;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
		}
		else p_input += 4;
		return;
	}
	if((*testp & 0xFE) == 0xFC) //level 5
	{
		if(	p_end - testp < 6) goto unwind_as_much_as_possible;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
		}
		else p_input += 5;
		return;
	}
	if(*testp == 0xFE) //level 6
	{
		if(	p_end - testp < 7) goto unwind_as_much_as_possible;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
		}
		else p_input += 6;
		return;
	}

unwind_as_much_as_possible:
	while(++testp != p_end && (*testp & 0xC0) == 0x80);
	p_input = testp -1;
}

static char8_t __convert_UTF8_ANSI_failforward(const char8_t*& p_input, const char8_t* const p_end, const char8_t p_placeholder)
{
	const char8_t* testp = p_input;
	if(!(*testp & 0x80)) //level 0
	{
		return *testp;
	}

	const char8_t* codeStart = testp;

	if((*testp & 0xE0) == 0xC0) //level 1
	{
		if(p_end - testp < 2) return p_placeholder;
		if((*++testp & 0xC0) != 0x80)
		{
			return p_placeholder;
		}

		++p_input;

		if((*codeStart & 0x1E) != 0x02) return p_placeholder;

		const char8_t rcode = *codeStart << 6;
		return rcode | static_cast<char8_t>(*++codeStart & 0x3F);
	}

	if((*testp & 0xF0) == 0xE0) //level 2
	{
		if(p_end - testp < 3) goto unwind_as_much_as_possible;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
		}
		else p_input += 2;
		return p_placeholder;
	}
	else if((*testp & 0xF8) == 0xF0) //level 3
	{
		if(	p_end - testp < 4) goto unwind_as_much_as_possible;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
		}
		else p_input += 3;
		return p_placeholder;
	}
	else if((*testp & 0xFC) == 0xF8) //level 4
	{
		if(	p_end - testp < 5) goto unwind_as_much_as_possible;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
		}
		else p_input += 4;
		return p_placeholder;
	}
	else if((*testp & 0xFE) == 0xFC) //level 5
	{
		if(	p_end - testp < 6) goto unwind_as_much_as_possible;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
		}
		else p_input += 5;
		return p_placeholder;
	}
	else if(*testp == 0xFE) //level 6
	{
		if(	p_end - testp < 7) goto unwind_as_much_as_possible;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
		}
		else p_input += 6;
		return p_placeholder;
	}

unwind_as_much_as_possible:
	while(++testp != p_end && (*testp & 0xC0) == 0x80);
	p_input = testp -1;
	return p_placeholder;
}

static char16_t __convert_UTF8_UCS2_failforward(const char8_t*& p_input, const char8_t* const p_end, const char16_t p_placeholder)
{
	const char8_t* testp = p_input;
	if(!(*testp & 0x80)) //level 0
	{
		return static_cast<char16_t>(*testp);
	}

	const char8_t* codeStart = testp;

	if((*testp & 0xE0) == 0xC0) //level 1
	{
		if( p_end - testp < 2) return p_placeholder;
		if((*++testp & 0xC0) != 0x80)
		{
			return p_placeholder;
		}

		++p_input;

		if(*codeStart < 0xC2) return p_placeholder;

		const char16_t rcode = (*codeStart & 0x3F) << 6;
		return rcode | static_cast<char16_t>(*++codeStart & 0x3F);
	}

	if((*testp & 0xF0) == 0xE0) //level 2
	{
		if(	p_end - testp < 3) goto unwind_as_much_as_possible;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
			return p_placeholder;
		}
		p_input += 2;

		testp = codeStart;
		if(!(*(codeStart++) & 0x0F) && !(*codeStart & 0x20))
		{
			return p_placeholder;
		}

		char16_t tcode = static_cast<char16_t>(static_cast<char16_t>(*testp & 0x0F) << 12);
		tcode |= static_cast<char16_t>(static_cast<char16_t>(*++testp & 0x3F) << 6);
		return tcode | static_cast<char16_t>(*++testp & 0x3F);
	}
	if((*testp & 0xF8) == 0xF0) //level 3
	{
		if(	p_end - testp < 4) goto unwind_as_much_as_possible;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
		}
		else p_input += 3;
		return p_placeholder;
	}
	if((*testp & 0xFC) == 0xF8) //level 4
	{
		if(	p_end - testp < 5) goto unwind_as_much_as_possible;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
		}
		else p_input += 4;
		return p_placeholder;
	}
	if((*testp & 0xFE) == 0xFC) //level 5
	{
		if(	p_end - testp < 6) goto unwind_as_much_as_possible;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
		}
		else p_input += 5;
		return p_placeholder;
	}
	if(*testp == 0xFE) //level 6
	{
		if(	p_end - testp < 7) goto unwind_as_much_as_possible;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
		}
		else p_input += 6;
		return p_placeholder;
	}

unwind_as_much_as_possible:
	while(++testp != p_end && (*testp & 0xC0) == 0x80);
	p_input = testp -1;
	return p_placeholder;
}

static char32_t __convert_UTF8_UCS4_failforward(const char8_t*& p_input, const char8_t* const p_end, const char32_t p_placeholder)
{
	const char8_t* testp = p_input;
	if(!(*testp & 0x80)) //level 0
	{
		return static_cast<char16_t>(*testp);
	}

	const char8_t* codeStart = testp;

	if((*testp & 0xE0) == 0xC0) //level 1
	{
		if( p_end - testp < 2) return p_placeholder;
		if((*++testp & 0xC0) != 0x80)
		{
			return p_placeholder;
		}
		++p_input;

		if(*codeStart < 0xC2) return p_placeholder;

		const char16_t rcode = static_cast<char16_t>(static_cast<char16_t>(*codeStart & 0x3F) << 6);
		return rcode | static_cast<char16_t>(*++codeStart & 0x3F);
	}

	if((*testp & 0xF0) == 0xE0) //level 2
	{
		if(	p_end - testp < 3) goto unwind_as_much_as_possible;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
			return p_placeholder;
		}
		p_input += 2;

		testp = codeStart;
		if(!(*(codeStart++) & 0x0F) && !(*codeStart & 0x20))
		{
			return p_placeholder;
		}

		char16_t tcode = static_cast<char16_t>(static_cast<char16_t>(*testp & 0x0F) << 12);
		tcode |= static_cast<char16_t>(static_cast<char16_t>(*++testp & 0x3F) << 6);
		return tcode | static_cast<char16_t>(*++testp & 0x3F);
	}
	if((*testp & 0xF8) == 0xF0) //level 3
	{
		if(	p_end - testp < 4) goto unwind_as_much_as_possible;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
			return p_placeholder;
		}
		p_input += 3;

		const char8_t t1 = *codeStart & 0x07;
		if(t1 > 4) //validate size, and partial upper_range
		{
			return p_placeholder;
		}
		const char8_t t2 = *++codeStart;
		if(	(t1 == 4 && (t2 & 0x30))	|| //validate upper_range
			(!t1 && (t2 < 0x90))) //validate sub-range
		{
			return p_placeholder;
		}

		char32_t tcode = t1 << 18;
		tcode |= static_cast<char32_t>(t2 & 0x3F) << 12;
		tcode |= static_cast<char32_t>(*++codeStart & 0x3F) << 6;
		return tcode | static_cast<char32_t>(*++codeStart & 0x3F);
	}

	if((*testp & 0xFC) == 0xF8) //level 4
	{
		if(	p_end - testp < 5) goto unwind_as_much_as_possible;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
		}
		else p_input += 4;
		return p_placeholder;
	}
	if((*testp & 0xFE) == 0xFC) //level 5
	{
		if(	p_end - testp < 6) goto unwind_as_much_as_possible;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
		}
		else p_input += 5;
		return p_placeholder;
	}
	if(*testp == 0xFE) //level 6
	{
		if(	p_end - testp < 7) goto unwind_as_much_as_possible;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
		}
		else p_input += 6;

		return p_placeholder;
	}

unwind_as_much_as_possible:
	while(++testp != p_end && (*testp & 0xC0) == 0x80);
	p_input = testp -1;
	return p_placeholder;
}

static uintptr_t __estimate_UTF8_UTF16_failforward(const char8_t*& p_input, const char8_t* const p_end)
{
	const char8_t* testp = p_input;
	if(!(*testp & 0x80)) //level 0
	{
		return 1;
	}
	const char8_t* codeStart = testp;
	if((*testp & 0xE0) == 0xC0) //level 1
	{
		if(p_end - testp < 2) return 0;
		if((*++testp & 0xC0) != 0x80)
		{
			return 0;
		}
		++p_input;
		if(*codeStart < 0xC2) return 0;
		return 1;
	}

	if((*testp & 0xF0) == 0xE0) //level 2
	{
		if(	p_end - testp < 3) goto unwind_as_much_as_possible;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
			return 0;
		}
		p_input += 2;

		const char8_t tsub1 = *codeStart & 0x0F;
		const char8_t tsub2 = *++codeStart & 0x20;

		if(	(!tsub1 && !tsub2) || //validate range
			((tsub1 == 0x0D) && tsub2)) //surrogates
		{
			return 0;
		}

		return 1;
	}
	if((*testp & 0xF8) == 0xF0) //level 3
	{
		if(	p_end - testp < 4) goto unwind_as_much_as_possible;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
			return 0;
		}
		p_input += 3;

		const char8_t tsub1 = *codeStart & 0x07;
		const char8_t tsub2 = *++codeStart;

		if(	(!tsub1 && (tsub2 < 0x90)) ||
			(tsub1 > 4) ||
			((tsub1 == 4) && (tsub2 & 0x30)))
		{
			return 0;
		}

		return 2;
	}
	if((*testp & 0xFC) == 0xF8) //level 4
	{
		if(	p_end - testp < 5) goto unwind_as_much_as_possible;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
			return 0;
		}
		p_input += 4;
		return 0;
	}
	if((*testp & 0xFE) == 0xFC) //level 5
	{
		if(	p_end - testp < 6) goto unwind_as_much_as_possible;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
			return 0;
		}
		p_input += 5;
		return 0;
	}
	if(*testp == 0xFE) //level 6
	{
		if(	p_end - testp < 7) goto unwind_as_much_as_possible;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
			return 0;
		}
		p_input += 6;
		return 0;
	}

unwind_as_much_as_possible:
	while(++testp != p_end && (*testp & 0xC0) == 0x80);
	p_input = testp -1;
	return 0;
}

static uintptr_t __convert_UTF8_UTF16_failforward(const char8_t*& p_input, const char8_t* const p_end, char16_t* p_output)
{
	const char8_t* testp = p_input;
	if(!(*testp & 0x80)) //level 0
	{
		*p_output = static_cast<char16_t>(*testp);
		return 1;
	}
	const char8_t* codeStart = testp;
	if((*testp & 0xE0) == 0xC0) //level 1
	{
		if(p_end - testp < 2) return 0;
		if((*++testp & 0xC0) != 0x80)
		{
			return 0;
		}
		++p_input;
		const char8_t tsub1 = *codeStart;
		if(tsub1 < 0xC2) return 0;

		*p_output =
			static_cast<char16_t>(static_cast<char16_t>(tsub1 & 0x1F) << 6) |
			static_cast<char16_t>(*testp & 0x3F);

		return 1;
	}

	if((*testp & 0xF0) == 0xE0) //level 2
	{
		if(	p_end - testp < 3) goto unwind_as_much_as_possible;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
			return 0;
		}
		p_input += 2;

		const char8_t tsub1 = *codeStart & 0x0F;
		const char8_t tsub2 = *++codeStart & 0x20;

		if(	((tsub1 == 0) && !(tsub2)) || //validate range
			((tsub1 == 0x0D) && tsub2)) //surrogates
		{
			return 0;
		}

		const char16_t tcode =
			static_cast<char16_t>(tsub1 << 12) |
			static_cast<char16_t>(static_cast<char16_t>(*codeStart & 0x3F) << 6);
		*p_output = tcode | static_cast<char16_t>(*++codeStart & 0x3F);
		return 1;
	}
	if((*testp & 0xF8) == 0xF0) //level 3
	{
		if(	p_end - testp < 4) goto unwind_as_much_as_possible;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
			return 0;
		}
		p_input += 3;

		const char8_t tsub1 = *codeStart & 0x07;
		const char8_t tsub2 = *++codeStart;

		if(	((!tsub1) && (tsub2 < 0x90)) ||
			(tsub1 > 4) ||
			((tsub1 == 4) && (tsub2 & 0x30)))
		{
			return 0;
		}

		p_input += 3;
		char16_t tcode = static_cast<char16_t>(
			(static_cast<char16_t>(tsub1 << 8) |
			static_cast<char16_t>(static_cast<char16_t>(tsub2 & 0x3F) << 2)) - char16_t{0x40});

		*p_output = 0xD800 | tcode | static_cast<char16_t>(static_cast<char16_t>(*++codeStart & 0x30) >> 4);

		tcode = 0xDC00 | static_cast<char16_t>(static_cast<char16_t>(*codeStart & 0x0F) << 6);
		*++p_output = tcode | static_cast<char16_t>(*++codeStart & 0x3F);

		return 2;
	}

	if((*testp & 0xFC) == 0xF8) //level 4
	{
		if(	p_end - testp < 5) goto unwind_as_much_as_possible;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
		}
		else p_input += 4;
		return 0;
	}
	if((*testp & 0xFE) == 0xFC) //level 5
	{
		if(	p_end - testp < 6) goto unwind_as_much_as_possible;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
		}
		else p_input += 5;
		return 0;
	}
	if(*testp == 0xFE) //level 6
	{
		if(	p_end - testp < 7) goto unwind_as_much_as_possible;
		if(	(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	||
			(*++testp & 0xC0) != 0x80	)
		{
			p_input = testp -1;
		}
		else p_input += 6;
		return 0;
	}

unwind_as_much_as_possible:
	while(++testp != p_end && (*testp & 0xC0) == 0x80);
	p_input = testp -1;
	return 0;
}

uintptr_t __inline_encode_UTF8(const char32_t p_char, char8_t* p_output)
{
	if(p_char < 0x00000080) //Level 0
	{
		*p_output = static_cast<char8_t>(p_char);
		return 1;
	}

	if(p_char < 0x00000800) //Level 1
	{
		*p_output   = static_cast<char8_t>( (p_char >>  6		) | 0xC0);
		*++p_output = static_cast<char8_t>(( p_char		& 0x3F	) | 0x80);
		return 2;
	}

	if(p_char < 0x00010000) //Level 2
	{
		*p_output   = static_cast<char8_t>( (p_char >> 12			) | 0xE0);
		*++p_output = static_cast<char8_t>(((p_char >>  6)	& 0x3F	) | 0x80);
		*++p_output = static_cast<char8_t>(( p_char			& 0x3F	) | 0x80);
		return 3;
	}

	if(p_char < 0x00110000) //Level 3
	{
		*p_output   = static_cast<char8_t>( (p_char >> 18			) | 0xF0);
		*++p_output = static_cast<char8_t>(((p_char >> 12)	& 0x3F	) | 0x80);
		*++p_output = static_cast<char8_t>(((p_char >>  6)	& 0x3F	) | 0x80);
		*++p_output = static_cast<char8_t>(( p_char			& 0x3F	) | 0x80);
		return 4;
	}

	return 0;
}

static uintptr_t __estimate_UTF16_UTF8(const char16_t*& p_input, const char16_t* const p_end)
{
	const char16_t testp = *p_input;

	if(testp < 0x0080) //Level 0
	{
		return 1;
	}
	if(testp < 0x0800) //Level 1
	{
		return 2;
	}
	if(testp > 0xD7FF && testp < 0xE000)
	{
		if(	(p_end - p_input < 2)             ||
			(testp & 0xFC00)        != 0xD800 ||
			(*(++p_input) & 0xFC00) != 0xDC00) return 0;
		return 4;
	}
	return 3;
}

static void __convert_UTF16_UTF8(const char16_t*& p_input, char8_t*& p_out)
{
	const char16_t testp = *p_input;
	if(testp < 0x0080) //Level 0
	{
		*p_out = static_cast<char8_t>(testp);
	}
	else if(testp < 0x0800) //Level 1
	{
		*p_out   = static_cast<char8_t>(testp >> 6)     | char8_t{0xC0};
		*++p_out = (static_cast<char8_t>(testp) & 0x3F) | char8_t{0x80};
	}
	else if(testp > 0xD7FF && testp < 0xE000)
	{
		const char32_t refrag = (((testp & 0x03FF) << 10) | (*(++p_input) & 0x03FF)) + 0x10000;
		*p_out   =  static_cast<char8_t>(refrag >> 18)                  | char8_t{0xF0};
		*++p_out = (static_cast<char8_t>(refrag >> 12) & char8_t{0x3F}) | char8_t{0x80};
		*++p_out = (static_cast<char8_t>(refrag >>  6) & char8_t{0x3F}) | char8_t{0x80};
		*++p_out = (static_cast<char8_t>(refrag      ) & char8_t{0x3F}) | char8_t{0x80};
	}
	else
	{
		*p_out    =  static_cast<char8_t>(testp >> 12)                  | char8_t{0xE0};
		*++p_out  = (static_cast<char8_t>(testp >>  6) & char8_t{0x3F}) | char8_t{0x80};
		*++p_out  = (static_cast<char8_t>(testp      ) & char8_t{0x3F}) | char8_t{0x80};
	}
}

inline static char32_t __convert_UTF16_UCS4(const char16_t*& p_input)
{
	const char16_t testp = *p_input;
	if(testp > 0xD7FF && testp < 0xE000)
	{
		return (((testp & 0x03FF) << 10) | (*++p_input & 0x03FF)) + 0x10000;
	}
	return static_cast<char32_t>(testp);
}


static void __fmove_UTF16_failForward(const char16_t*& p_input, const char16_t* const p_end)
{
	const char16_t tchar = *p_input;
	if(tchar > 0xD7FF && tchar < 0xE000)
	{
		const char16_t* pivot = p_input;
		if(	(p_end - pivot < 2 )			||
			(tchar & 0xFC00)	!= 0xD800	||
			(*++pivot & 0xFC00)	!= 0xDC00)	return;
		++p_input;
	}
}

static char8_t __convert_UTF16_ANSI_failforward(const char16_t*& p_input, const char16_t* const p_end, const char8_t p_placeholder)
{
	const char16_t testp = *p_input;
	if(testp < 0x0100)
	{
		return static_cast<char8_t>(testp);
	}

	if(testp > 0xD7FF && testp < 0xE000)
	{
		const char16_t* pivot = p_input;
		if(	(p_end - pivot < 2)				||
			(testp & 0xFC00)	!= 0xD800	||
			(*++pivot & 0xFC00)	!= 0xDC00)	return p_placeholder;
		++p_input;
	}

	return p_placeholder;
}

static char16_t __convert_UTF16_UCS2_failforward(const char16_t*& p_input, const char16_t* const p_end, const char16_t p_placeholder)
{
	const char16_t testp = *p_input;

	if(testp > 0xD7FF && testp < 0xE000)
	{
		const char16_t* pivot = p_input;
		if(	(p_end - pivot < 2)				||
			(testp & 0xFC00)	!= 0xD800	||
			(*++pivot & 0xFC00)	!= 0xDC00)	return p_placeholder;
		++p_input;
		return p_placeholder;
	}

	return testp;
}

static char32_t __convert_UTF16_UCS4_failforward(const char16_t*& p_input, const char16_t* const p_end, const char32_t p_placeholder)
{
	const char16_t testp = *p_input;

	if(testp > 0xD7FF && testp < 0xE000)
	{
		const char16_t* pivot = p_input;
		if(	(p_end - pivot < 2)				||
			(testp & 0xFC00)	!= 0xD800	||
			(*++pivot & 0xFC00)	!= 0xDC00)	return p_placeholder;
		++p_input;

		return (((testp & 0x03FF) << 10) | (*pivot & 0x03FF)) + 0x10000;
	}

	return static_cast<char32_t>(testp);
}


static uintptr_t __estimate_UTF16_UTF8_failforward(const char16_t*& p_input, const char16_t* const p_end)
{
	const char16_t testp = *p_input;

	if(testp < 0x0080) //Level 0
	{
		return 1;
	}
	if(testp < 0x0800) //Level 1
	{
		return 2;
	}
	if(testp > 0xD7FF && testp < 0xE000)
	{
		if(	(p_end - p_input < 2)             ||
			(testp & 0xFC00)        != 0xD800 ||
			(*(p_input + 1) & 0xFC00) != 0xDC00) return 0;
		++p_input;
		return 4;
	}
	return 3;
}

static uintptr_t __convert_UTF16_UTF8_failforward(const char16_t*& p_input, const char16_t* const p_end, char8_t* p_out)
{
	const char16_t testp = *p_input;
	if(testp < 0x0080) //Level 0
	{
		*p_out = static_cast<char8_t>(testp);
		return 1;
	}

	if(testp < 0x0800) //Level 1
	{
		*p_out   = static_cast<char8_t>(testp >> 6)     | char8_t{0xC0};
		*++p_out = (static_cast<char8_t>(testp) & 0x3F) | char8_t{0x80};
		return 2;
	}

	if(testp > 0xD7FF && testp < 0xE000)
	{
		const char16_t* pivot = p_input;
		if((p_end - pivot < 2) ||
			(testp & 0xFC00) != 0xD800 ||
			(*++pivot & 0xFC00) != 0xDC00)
		{
			return 0;
		}

		char32_t refrag = (((testp & 0x03FF) << 10) | (*(++p_input) & 0x03FF)) + 0x10000;

		*p_out   =  static_cast<char8_t>(refrag >> 18)                  | char8_t{0xF0};
		*++p_out = (static_cast<char8_t>(refrag >> 12) & char8_t{0x3F}) | char8_t{0x80};
		*++p_out = (static_cast<char8_t>(refrag >>  6) & char8_t{0x3F}) | char8_t{0x80};
		*++p_out = (static_cast<char8_t>(refrag      ) & char8_t{0x3F}) | char8_t{0x80};
		return 4;
	}

	*p_out   = static_cast<char8_t>(testp >> 12)                 | char8_t{0xE0};
	*++p_out = (static_cast<char8_t>(testp >>  6) & char8_t{0x3F}) | char8_t{0x80};
	*++p_out = (static_cast<char8_t>(testp      ) & char8_t{0x3F}) | char8_t{0x80};

	return 3;
}

uintptr_t __inline_encode_UTF16(const char32_t p_char, char16_t* p_output)
{
	if(p_char < 0xD800)
	{
		*p_output = static_cast<char16_t>(p_char);
		return 1;
	}
	if(p_char < 0xDE00)
	{
		return 0;
	}
	if(p_char < 0x010000)
	{
		*p_output = static_cast<char16_t>(p_char);
		return 1;
	}
	if(p_char < 0x110000)
	{
		*p_output   = 0xD800 | static_cast<char16_t>((p_char - 0x010000) >> 10);
		*++p_output = 0xDC00 | static_cast<char16_t>(p_char & 0x03FF);
		return 2;
	}
	return 0;
}

static uintptr_t __estimate_UCS4_UTF8(const char32_t p_input)
{
	if(p_input < 0x00000080) //Level 0
	{
		return 1;
	}
	if(p_input < 0x00000800) //Level 1
	{
		return 2;
	}
	if(p_input < 0x00010000) //Level 2
	{
		return 3;
	}
	if(p_input < 0x00110000) //Level 3
	{
		return 4;
	}

	return 0;
}

static uintptr_t __convert_UCS4_UTF8(const char32_t p_input, char8_t* p_out)
{
	if(p_input < 0x00000080) //Level 0
	{
		*p_out = static_cast<char8_t>(p_input);
		return 1;
	}
	if(p_input < 0x00000800) //Level 1
	{
		*(p_out++) = static_cast<char8_t>(p_input >> 6  ) | char8_t{0xC0};
		*p_out     = static_cast<char8_t>(p_input & 0x3F) | char8_t{0x80};
		return 2;
	}
	if(p_input < 0x00010000) //Level 2
	{
		*(p_out++) = static_cast<char8_t>( p_input >> 12        ) | char8_t{0xE0};
		*(p_out++) = static_cast<char8_t>((p_input >>  6) & 0x3F) | char8_t{0x80};
		*p_out     = static_cast<char8_t>( p_input        & 0x3F) | char8_t{0x80};
		return 3;
	}

	*(p_out++) = static_cast<char8_t>( p_input >> 18        ) | char8_t{0xF0};
	*(p_out++) = static_cast<char8_t>((p_input >> 12) & 0x3F) | char8_t{0x80};
	*(p_out++) = static_cast<char8_t>((p_input >>  6) & 0x3F) | char8_t{0x80};
	*p_out     = static_cast<char8_t>( p_input        & 0x3F) | char8_t{0x80};
	return 4;
} 

static uintptr_t __convert_UCS4_UTF8_checked(const char32_t p_input, char8_t* p_out)
{
	if(p_input < 0x00000080) //Level 0
	{
		*p_out = static_cast<char8_t>(p_input);
		return 1;
	}
	if(p_input < 0x00000800) //Level 1
	{
		*(p_out++) = static_cast<char8_t>(p_input >> 6  ) | char8_t{0xC0};
		*p_out     = static_cast<char8_t>(p_input & 0x3F) | char8_t{0x80};
		return 2;
	}
	if(p_input < 0x00010000) //Level 2
	{
		*(p_out++) = static_cast<char8_t>( p_input >> 12        ) | char8_t{0xE0};
		*(p_out++) = static_cast<char8_t>((p_input >>  6) & 0x3F) | char8_t{0x80};
		*p_out     = static_cast<char8_t>( p_input        & 0x3F) | char8_t{0x80};
		return 3;
	}
	if(p_input < 0x00110000) //Level 3
	{
		*(p_out++) = static_cast<char8_t>( p_input >> 18        ) | char8_t{0xF0};
		*(p_out++) = static_cast<char8_t>((p_input >> 12) & 0x3F) | char8_t{0x80};
		*(p_out++) = static_cast<char8_t>((p_input >>  6) & 0x3F) | char8_t{0x80};
		*p_out     = static_cast<char8_t>( p_input        & 0x3F) | char8_t{0x80};
		return 4;
	}
	return 0;
} 

uintptr_t __estimate_UCS4_UTF16(const char32_t p_char)
{
	if(p_char < 0xD800)
	{
		return 1;
	}
	if(p_char < 0xDE00)
	{
		return 0;
	}
	if(p_char < 0x010000)
	{
		return 1;
	}
	if(p_char < 0x110000)
	{
		return 2;
	}
	return 0;
}

namespace _p
{

[[nodiscard]] std::optional<uintptr_t> UTF8_to_ANSI_estimate(std::u8string_view const p_input)
{
	uintptr_t count = 0;

	const char8_t* pos = p_input.data();
	const char8_t* const end = pos + p_input.size();

	for(; pos < end; ++pos, ++count)
	{
		const char8_t testp = *pos;
		if(testp & 0x80)
		{
			if	(
					(testp != 0xC2 && testp != 0xC3) ||	//validate encoding 1, check range and unique representation simultaneously
					(end - pos) < 2	||					//validate size
					(*(++pos) & 0xC0) != 0x80			// validate encoding 2
				)
			{
				return {};
			}
		}
	}

	return count;
}

void UTF8_to_ANSI_unsafe(std::u8string_view const p_input, char8_t* p_output)
{
	const char8_t* pos = p_input.data();
	const char8_t* const end = pos + p_input.size();

	for(; pos < end; ++pos, ++p_output)
	{
		*(p_output) = __convert_UTF8_ANSI_unsafe(pos);
	}
}


[[nodiscard]] std::optional<uintptr_t> UTF16_to_ANSI_estimate(std::u16string_view const p_input)
{
	return UCS2_to_ANSI_estimate(p_input);
}

void UTF16_to_ANSI_unsafe(std::u16string_view const p_input, char8_t* const p_output)
{
	UCS2_to_ANSI_unsafe(p_input, p_output);
}


[[nodiscard]] std::optional<uintptr_t> UCS2_to_ANSI_estimate(std::u16string_view const p_input)
{
	for(const char16_t tchar : p_input)
	{
		if(tchar > 0xFF) return {};
	}
	return p_input.size();
}

void UCS2_to_ANSI_unsafe(std::u16string_view const p_input, char8_t* p_output)
{
	for(const char16_t tchar : p_input)
	{
		*(p_output++) = static_cast<char8_t>(tchar);
	}
}


[[nodiscard]] std::optional<uintptr_t> UCS4_to_ANSI_estimate(std::u32string_view const p_input)
{
	for(const char32_t tchar : p_input)
	{
		if(tchar > 0xFF) return {};
	}
	return p_input.size();
}

void UCS4_to_ANSI_unsafe(std::u32string_view const p_input, char8_t* p_output)
{
	for(const char32_t tchar : p_input)
	{
		*(p_output++) = static_cast<char8_t>(tchar);
	}
}

[[nodiscard]] uintptr_t ANSI_to_UTF8_estimate(std::u8string_view const p_input)
{
	uintptr_t res = 0;
	for(const char8_t tchar : p_input)
	{
		if(tchar > 0x7F)
		{
			res += 2;
		}
		else
		{
			++res;
		}
	}
	return res;
}

void ANSI_to_UTF8_unsafe(std::u8string_view const p_input, char8_t* p_output)
{
	for(const char8_t tchar : p_input)
	{
		if(tchar > 0x7F)
		{
			*(p_output++) = 0xC0 | (tchar >> 6);
			*(p_output++) = 0x80 | (tchar & 0x3F);
		}
		else
		{
			*(p_output++) = tchar;
		}
	}
}


[[nodiscard]] std::optional<uintptr_t> UTF16_to_UTF8_estimate(std::u16string_view const p_input)
{
	uintptr_t count = 0;
	const char16_t* pos = p_input.data();
	const char16_t* const end = pos + p_input.size();

	for(; pos < end; ++pos)
	{
		const uintptr_t res = __estimate_UTF16_UTF8(pos, end);
		if(res == 0)
		{
			return {};
		}
		count += res;
	}

	return count;
}

void UTF16_to_UTF8_unsafe(std::u16string_view const p_input, char8_t* p_output)
{
	const char16_t* pos = p_input.data();
	const char16_t* const end = pos + p_input.size();
	for(; pos < end; ++pos, ++p_output)
	{
		__convert_UTF16_UTF8(pos, p_output);
	}
}


[[nodiscard]] uintptr_t UCS2_to_UTF8_estimate(std::u16string_view const p_input)
{
	uintptr_t count = 0;
	for(const char16_t tchar : p_input)
	{
		if(tchar < 0x0080)
		{
			++count;
		}
		else if(tchar < 0x0800)
		{
			count += 2;
		}
		else
		{
			count += 3;
		}
	}
	return count;
}

void UCS2_to_UTF8_unsafe(std::u16string_view const p_input, char8_t* p_output)
{
	for(const char16_t tchar : p_input)
	{
		if(tchar < 0x0080)
		{
			*(p_output++) = static_cast<char8_t>(tchar);
		}
		else if(tchar < 0x0800)
		{
			*(p_output++) =  static_cast<char8_t>(tchar >> 6)    | char8_t{0xC0};
			*(p_output++) = (static_cast<char8_t>(tchar) & 0x3F) | char8_t{0x80};
		}
		else
		{
			*(p_output++) = static_cast<char8_t>(tchar >> 12)                 | char8_t{0xE0};
			*(p_output++) = (static_cast<char8_t>(tchar >>  6) & char8_t{0x3F}) | char8_t{0x80};
			*(p_output++) = (static_cast<char8_t>(tchar      ) & char8_t{0x3F}) | char8_t{0x80};
		}
	}
}

[[nodiscard]] std::optional<uintptr_t> UCS4_to_UTF8_estimate(std::u32string_view const p_input)
{
	uintptr_t count = 0;
	for(const char32_t tchar : p_input)
	{
		const uintptr_t res = __estimate_UCS4_UTF8(tchar);
		if(!res)
		{
			return {};
		}
		else
		{
			count += res;
		}
	}
	return count;
}

void UCS4_to_UTF8_unsafe(std::u32string_view const p_input, char8_t* p_output)
{
	for(const char32_t tchar : p_input)
	{
		p_output += __convert_UCS4_UTF8(tchar, p_output);
	}
}

void ANSI_to_UTF16_unsafe(std::u8string_view const p_input, char16_t* p_output)
{
	for(const char8_t tchar : p_input)
	{
		*(p_output++) = static_cast<char16_t>(tchar);
	}
}


[[nodiscard]] std::optional<uintptr_t> UTF8_to_UTF16_estimate(std::u8string_view const p_input)
{
	uintptr_t count = 0;
	const char8_t* pos = p_input.data();
	const char8_t* const end = pos + p_input.size();

	for(; pos < end; ++pos)
	{
		const uintptr_t res = __estimate_UTF8_UTF16(pos, end);
		if(res == 0)
		{
			return {};
		}
		count += res;
	}

	return count;
}

void UTF8_to_UTF16_unsafe(std::u8string_view const p_input, char16_t* p_output)
{
	const char8_t* pos = p_input.data();
	const char8_t* const end = pos + p_input.size();

	for(; pos < end; ++pos, ++p_output)
	{
		__convert_UTF8_UTF16_unsafe(pos, p_output);
	}
}


[[nodiscard]] std::optional<uintptr_t> UCS2_to_UTF16_estimate(std::u16string_view const p_input)
{
	for(const char16_t tchar : p_input)
	{
		if(tchar > 0xD7FF && tchar < 0xDE00) return {};
	}
	return p_input.size();
}

void UCS2_to_UTF16_unsafe(std::u16string_view const p_input, char16_t* const p_output)
{
	memcpy(p_output, p_input.data(), p_input.size());
}


[[nodiscard]] std::optional<uintptr_t> UCS4_to_UTF16_estimate(std::u32string_view const p_input)
{
	uintptr_t count = 0;
	for(const char32_t tchar : p_input)
	{
		if(tchar > 0xFFFF)
		{
			if(tchar > 0x10FFFF) return {};
			count += 2;
		}
		else
		{
			if(tchar > 0xD7FF && tchar < 0xDE00) return {};
			++count;
		}
	}
	return count;
}

void UCS4_to_UTF16_unsafe(std::u32string_view const p_input, char16_t* p_output)
{
	for(const char32_t tchar : p_input)
	{
		if(tchar > 0xFFFF)
		{
			*(p_output++) = 0xD800 | static_cast<char16_t>((tchar - 0x010000) >> 10);
			*(p_output++) = 0xDC00 | static_cast<char16_t>(tchar & 0x03FF);
		}
		else
		{
			*(p_output++) = static_cast<char16_t>(tchar);
		}
	}
}


void ANSI_to_UCS2_unsafe(std::u8string_view const p_input, char16_t* p_output)
{
	for(const char8_t tchar : p_input)
	{
		*(p_output++) = static_cast<char16_t>(tchar);
	}
}


[[nodiscard]] std::optional<uintptr_t> UTF8_to_UCS2_estimate(std::u8string_view const p_input)
{
	uintptr_t count = 0;
	const char8_t* pos = p_input.data();
	const char8_t* const end = pos + p_input.size();
	for(; pos < end; ++pos, ++count)
	{
		if(!__fmove_UTF8_UCS2(pos, end))
		{
			return {};
		}
	}
	return count;
}

void UTF8_to_UCS2_unsafe(std::u8string_view const p_input, char16_t* p_output)
{
	const char8_t* pos = p_input.data();
	const char8_t* const end = pos + p_input.size();
	for(; pos < end; ++pos, ++p_output)
	{
		(*p_output) = __convert_UTF8_UCS2_unsafe(pos);
	}
}


[[nodiscard]] std::optional<uintptr_t> UTF16_to_UCS2_estimate(std::u16string_view const p_input)
{
	for(const char16_t tchar : p_input)
	{
		if(tchar > 0xD7FF && tchar < 0xDE00)
		{
			return {};
		}
	}
	return p_input.size();
}

void UTF16_to_UCS2_unsafe(std::u16string_view const p_input, char16_t* const p_output)
{
	memcpy(p_output, p_input.data(), p_input.size());
}


[[nodiscard]] std::optional<uintptr_t> UCS4_to_UCS2_estimate(std::u32string_view const p_input)
{
	for(const char32_t tchar : p_input)
	{
		if(tchar > 0xFFFF)
		{
			return {};
		}
	}
	return p_input.size();
}

void UCS4_to_UCS2_unsafe(std::u32string_view const p_input, char16_t* p_output)
{
	for(const char32_t tchar : p_input)
	{
		*(p_output++) = static_cast<char16_t>(tchar);
	}
}


[[nodiscard]] uintptr_t ANSI_to_UCS4_estimate(std::u8string_view const p_input)
{
	return p_input.size();
}

void ANSI_to_UCS4_unsafe(std::u8string_view const p_input, char32_t* p_output)
{
	for(const char8_t tchar : p_input)
	{
		*(p_output++) = static_cast<char32_t>(tchar);
	}
}


[[nodiscard]] std::optional<uintptr_t> UTF8_to_UCS4_estimate(std::u8string_view const p_input)
{
	uintptr_t count = 0;
	const char8_t* pos = p_input.data();
	const char8_t* const end = pos + p_input.size();
	for(; pos < end; ++pos, ++count)
	{
		if(!__fmove_UTF8_UCS4(pos, end))
		{
			return {};
		}
	}
	return count;
}

void UTF8_to_UCS4_unsafe(std::u8string_view const p_input, char32_t* p_output)
{
	const char8_t* pos = p_input.data();
	const char8_t* const end = pos + p_input.size();
	for(; pos < end; ++pos, ++p_output)
	{
		*p_output = __convert_UTF8_UCS4(pos);
	}
}


[[nodiscard]] std::optional<uintptr_t> UTF16_to_UCS4_estimate(std::u16string_view const p_input)
{
	uintptr_t count = 0;

	const char16_t* pos = p_input.data();
	const char16_t* const end = pos + p_input.size();
	for(; pos < end; ++pos, ++count)
	{
		const char16_t testp = *pos;
		if(testp > 0xD7FF && testp < 0xE000)
		{
			if(	(end - pos < 2 )              ||
				(testp & 0xFC00)    != 0xD800 ||
				(*(++pos) & 0xFC00) != 0xDC00) return {};
		}
	}
	return count;
}

void UTF16_to_UCS4_unsafe(std::u16string_view const p_input, char32_t* p_output)
{
	const char16_t* pos = p_input.data();
	const char16_t* const end = pos + p_input.size();
	for(; pos < end; ++pos, ++p_output)
	{
		*p_output = __convert_UTF16_UCS4(pos);
	}
}

void UCS2_to_UCS4_unsafe(std::u16string_view const p_input, char32_t* p_output)
{
	for(const char16_t tchar : p_input)
	{
		*(p_output++) = static_cast<char32_t>(tchar);
	}
}


[[nodiscard]] static uintptr_t UTF8_to_fix_faulty_estimate(std::u8string_view const p_input)
{
	uintptr_t count = 0;
	const char8_t* pos = p_input.data();
	const char8_t* const end = pos + p_input.size();
	for(; pos < end; ++pos, ++count)
	{
		__fmove_UTF8_failForward(pos, end);
	}

	return count;
}

[[nodiscard]] static uintptr_t UTF16_to_fix_faulty_estimate(std::u16string_view const p_input)
{
	uintptr_t count = 0;
	const char16_t* pos = p_input.data();
	const char16_t* const end = pos + p_input.size();
	for(; pos < end; ++pos, ++count)
	{
		__fmove_UTF16_failForward(pos, end);
	}
	return count;
}

[[nodiscard]] uintptr_t UTF8_to_ANSI_faulty_estimate(std::u8string_view const p_input)
{
	return UTF8_to_fix_faulty_estimate(p_input);
}

void UTF8_to_ANSI_faulty_unsafe(std::u8string_view const p_input, const  char8_t p_placeHolder, char8_t* p_output)
{
	const char8_t* pos = p_input.data();
	const char8_t* const end = pos + p_input.size();
	for(; pos < end; ++pos, ++p_output)
	{
		*p_output = __convert_UTF8_ANSI_failforward(pos, end, p_placeHolder);
	}
}


[[nodiscard]] uintptr_t UTF16_to_ANSI_faulty_estimate(std::u16string_view const p_input)
{
	return UTF16_to_fix_faulty_estimate(p_input);
}

void UTF16_to_ANSI_faulty_unsafe(std::u16string_view const p_input, const char8_t p_placeHolder, char8_t* p_output)
{
	const char16_t* pos = p_input.data();
	const char16_t* const end = pos + p_input.size();
	for(; pos < end; ++pos, ++p_output)
	{
		*p_output = __convert_UTF16_ANSI_failforward(pos, end, p_placeHolder);
	}
}


void UCS2_to_ANSI_faulty_unsafe(std::u16string_view const p_input, const char8_t p_placeHolder, char8_t* p_output)
{
	for(const char16_t tchar : p_input)
	{
		if(tchar < 0x0100)
		{
			*p_output = static_cast<char8_t>(tchar);
		}
		else
		{
			*p_output = p_placeHolder;
		}
		++p_output;
	}
}


void UCS4_to_ANSI_faulty_unsafe(std::u32string_view const p_input, const char8_t p_placeHolder, char8_t* p_output)
{
	for(const char32_t tchar : p_input)
	{
		if(tchar < 0x0100)
		{
			*p_output = static_cast<char8_t>(tchar);
		}
		else
		{
			*p_output = p_placeHolder;
		}
		++p_output;
	}
}


[[nodiscard]] uintptr_t UTF16_to_UTF8_faulty_estimate(std::u16string_view const p_input, const char32_t p_placeHolder)
{
	uintptr_t count = 0;
	const char16_t* pos = p_input.data();
	const char16_t* const end = pos + p_input.size();

	const uintptr_t ph_count = __estimate_UCS4_UTF8(p_placeHolder);

	for(; pos < end; ++pos)
	{
		const uintptr_t tcount = __estimate_UTF16_UTF8_failforward(pos, end);
		count += tcount ? tcount: ph_count;
	}

	return count;
}

void UTF16_to_UTF8_faulty_unsafe(std::u16string_view const p_input, const char32_t p_placeHolder, char8_t* p_output)
{
	std::array<char8_t, 4> placeHolder;
	const uintptr_t placeHolderSize = __inline_encode_UTF8(p_placeHolder, placeHolder.data());

	const char16_t* pos = p_input.data();
	const char16_t* const end = pos + p_input.size();

	for(; pos < end; ++pos)
	{
		uintptr_t res = __convert_UTF16_UTF8_failforward(pos, end, p_output);
		if(res)
		{
			p_output += res;
		}
		else
		{
			memcpy(p_output, placeHolder.data(), placeHolderSize);
			p_output += placeHolderSize;
		}
	}
}

[[nodiscard]] uintptr_t UCS4_to_UTF8_faulty_estimate(std::u32string_view const p_input, const char32_t p_placeHolder)
{
	uintptr_t count = 0;
	const uintptr_t ph_count = __estimate_UCS4_UTF8(p_placeHolder);
	for(const char32_t tchar : p_input)
	{
		const uintptr_t tcount = __estimate_UCS4_UTF8(tchar);
		count += tcount ? tcount: ph_count;
	}

	return count;
}

void UCS4_to_UTF8_faulty_unsafe(std::u32string_view const p_input, const char32_t p_placeHolder, char8_t* p_output)
{
	std::array<char8_t, 4> placeHolder;
	const uintptr_t placeHolderSize = __inline_encode_UTF8(p_placeHolder, placeHolder.data());

	for(const char32_t tchar : p_input)
	{
		const uintptr_t tcount = __convert_UCS4_UTF8_checked(tchar, p_output);
		if(tcount)
		{
			p_output += tcount;
		}
		else
		{
			memcpy(p_output, placeHolder.data(), placeHolderSize);
			p_output += placeHolderSize;
		}
	}
}

[[nodiscard]] uintptr_t UTF8_to_UTF16_faulty_estimate(std::u8string_view const p_input, const char32_t p_placeHolder)
{
	uintptr_t count = 0;
	const char8_t* pos = p_input.data();
	const char8_t* const end = pos + p_input.size();
	const uintptr_t ph_count = __estimate_UCS4_UTF16(p_placeHolder);

	for(; pos < end; ++pos)
	{
		const uintptr_t tcount = __estimate_UTF8_UTF16_failforward(pos, end);
		if(tcount)
		{
			count += tcount;
		}
		else
		{
			count += ph_count;
		}
	}

	return count;
}

void UTF8_to_UTF16_faulty_unsafe(std::u8string_view const p_input, const char32_t p_placeHolder, char16_t* p_output)
{
	std::array<char16_t, 2> placeHolder;
	const uintptr_t placeHolderCount = __inline_encode_UTF16(p_placeHolder, placeHolder.data());
	const uintptr_t placeHolderSize = placeHolderCount * sizeof(char16_t);

	const char8_t* pos = p_input.data();
	const char8_t* const end = pos + p_input.size();

	for(; pos < end; ++pos)
	{
		uintptr_t res = __convert_UTF8_UTF16_failforward(pos, end, p_output);
		if(res)
		{
			p_output += res;
		}
		else
		{
			memcpy(p_output, placeHolder.data(), placeHolderSize);
			p_output += placeHolderCount;
		}
	}
}


[[nodiscard]] uintptr_t UCS2_to_UTF16_faulty_estimate(std::u16string_view const p_input, const char32_t p_placeHolder)
{
	const uintptr_t ph_count = __estimate_UCS4_UTF16(p_placeHolder);
	uintptr_t count = 0;
	for(const char16_t tchar : p_input)
	{
		if((tchar & 0xF800) == 0xD800)
		{
			count += ph_count;
		}
		else
		{
			++count;
		}
	}
	return count;
}

void UCS2_to_UTF16_faulty_unsafe(std::u16string_view const p_input, const char32_t p_placeHolder, char16_t* p_output)
{
	std::array<char16_t, 2> placeHolder;
	const uintptr_t placeHolderCount =  __inline_encode_UTF16(p_placeHolder, placeHolder.data());
	const uintptr_t placeHolderSize  = placeHolderCount * sizeof(char16_t);

	for(const char16_t tchar : p_input)
	{
		if(tchar > 0xD7FF && tchar < 0xDE00)
		{
			memcpy(p_output, placeHolder.data(), placeHolderSize);
			p_output += placeHolderCount;
		}
		else
		{
			*(p_output++) = tchar;
		}
	}
}

[[nodiscard]] uintptr_t UCS4_to_UTF16_faulty_estimate(std::u32string_view const p_input, const char32_t p_placeHolder)
{
	const uintptr_t ph_count = __estimate_UCS4_UTF16(p_placeHolder);
	uintptr_t count = 0;
	for(const char32_t tchar : p_input)
	{
		uintptr_t res = __estimate_UCS4_UTF16(tchar);
		if(res)
		{
			count += res;
		}
		else
		{
			count += ph_count;
		}
	}
	return count;
	
}

void UCS4_to_UTF16_faulty_unsafe(std::u32string_view const p_input, const char32_t p_placeHolder, char16_t* p_output)
{
	std::array<char16_t, 2> placeHolder;
	const uintptr_t placeHolderCount =  __inline_encode_UTF16(p_placeHolder, placeHolder.data());
	const uintptr_t placeHolderSize = placeHolderCount * sizeof(char16_t);


	for(const char32_t tchar : p_input)
	{
		uintptr_t res = __inline_encode_UTF16(tchar, p_output);
		if(res)
		{
			p_output += res;
		}
		else
		{
			memcpy(p_output, placeHolder.data(), placeHolderSize);
			p_output += placeHolderCount;
		}
	}
}

[[nodiscard]] uintptr_t UTF8_to_UCS2_faulty_estimate(std::u8string_view const p_input)
{
	return UTF8_to_fix_faulty_estimate(p_input);
}

void UTF8_to_UCS2_faulty_unsafe(std::u8string_view const p_input, const char16_t p_placeHolder, char16_t* p_output)
{
	const char8_t* pos = p_input.data();
	const char8_t* const end = pos + p_input.size();
	for(; pos < end; ++pos, ++p_output)
	{
		*p_output = __convert_UTF8_UCS2_failforward(pos, end, p_placeHolder);
	}
}


[[nodiscard]] uintptr_t UTF16_to_UCS2_faulty_estimate(std::u16string_view const p_input)
{
	return UTF16_to_fix_faulty_estimate(p_input);
}

void UTF16_to_UCS2_faulty_unsafe(std::u16string_view const p_input, const char16_t p_placeHolder, char16_t* p_output)
{
	const char16_t* pos = p_input.data();
	const char16_t* const end = pos + p_input.size();
	for(; pos < end; ++pos, ++p_output)
	{
		*p_output = __convert_UTF16_UCS2_failforward(pos, end, p_placeHolder);
	}
}

void UCS4_to_UCS2_faulty_unsafe(std::u32string_view const p_input, const char16_t p_placeHolder, char16_t* p_output)
{
	for(char32_t tchar : p_input)
	{
		if(tchar < 0x010000)
		{
			*(p_output++) = static_cast<char16_t>(tchar);
		}
		else
		{
			*(p_output++) = p_placeHolder;
		}
	}
}


[[nodiscard]] uintptr_t UTF8_to_UCS4_faulty_estimate(std::u8string_view const p_input)
{
	return UTF8_to_fix_faulty_estimate(p_input);
}

void UTF8_to_UCS4_faulty_unsafe(std::u8string_view const p_input, const char32_t p_placeHolder, char32_t* p_output)
{
	const char8_t* pos = p_input.data();
	const char8_t* const end = pos + p_input.size();
	for(; pos < end; ++pos, ++p_output)
	{
		*p_output = __convert_UTF8_UCS4_failforward(pos, end, p_placeHolder);
	}
}


[[nodiscard]] uintptr_t UTF16_to_UCS4_faulty_estimate(std::u16string_view const p_input)
{
	return UTF16_to_fix_faulty_estimate(p_input);
}

void UTF16_to_UCS4_faulty_unsafe(std::u16string_view const p_input, const char32_t p_placeHolder, char32_t* p_output)
{
	const char16_t* pos = p_input.data();
	const char16_t* const end = pos + p_input.size();
	for(; pos < end; ++pos, ++p_output)
	{
		*p_output = __convert_UTF16_UCS4_failforward(pos, end, p_placeHolder);
	}
}

} //namespace _p


uint8_t encode_UTF8(char32_t const p_char, std::span<char8_t, 4> const p_output)
{
	if(p_char < 0x00000080) //Level 0
	{
		p_output[0] = static_cast<char8_t>(p_char);
		return 1;
	}

	if(p_char < 0x00000800) //Level 1
	{
		p_output[0] = static_cast<char8_t>( (p_char >>  6		) | 0xC0);
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

	return 0;
}

uint8_t encode_UTF16(char32_t p_char, std::span<char16_t, 2> const p_output)
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

bool UTF8_UNICODE_Compliant(std::u8string_view const p_input)
{
	const char8_t* pos = p_input.data();
	const char8_t* const end = pos + p_input.size();
	for(; pos < end; ++pos)
	{
		if(*pos & 0x80)
		{
			if((*pos & 0xE0) == 0xC0) //level 1
			{
				if(	end - pos < 2			||	//validate size
					(*pos & 0x1F) < 0x02	||	//validate range
					(*++pos & 0xC0) != 0x80) return false;	//validate encoding
			}
			else if((*pos & 0xF0) == 0xE0) //level 2
			{
				const char8_t* decode = pos;

				if(end - pos < 3			||	//validate size
					(!(*(pos++) & 0x0F) && !(*pos & 0x20)) || //validate range
					(*pos & 0xC0) != 0x80	||	//validate encoding
					(*++pos & 0xC0) != 0x80) return false;

				if(((*decode & 0x0F) == 0x0D) &&
					(*++decode & 0x20)) //surrogates
				{
					return false;
				}
			}
			else if((*pos & 0xF8) == 0xF0) //level 3
			{
				const char8_t* decode = pos;
				if(end - pos < 4			||	//validate size
					(((*pos++ & 0x07) == 0) && (*pos < 0x90)) ||	//validate sub-range
					(*pos & 0xC0) != 0x80 ||	//validate encoding
					(*++pos & 0xC0) != 0x80 ||
					(*++pos & 0xC0) != 0x80) return false;

				char32_t temp = (*decode & 0x07);

				if(temp > 4 ||
					(temp == 4 && (*++decode & 0x30))) return false; //validate upper-range
			}
			else return false;
		}
		//else level 0 always ok
	}
	return true;
}

bool UTF16_UNICODE_Compliant(std::u16string_view const p_input)
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

bool UCS2_UNICODE_Compliant(std::u16string_view const p_input)
{
	for(const char16_t tchar: p_input)
	{
		if(tchar > 0xD7FF && tchar < 0xE000) return false;
	}
	return true;
}

bool UCS4_UNICODE_Compliant(std::u32string_view const p_input)
{
	for(const char32_t tchar: p_input)
	{
		if(!UNICODE_Compliant(tchar)) return false;
	}
	return true;
}


bool ASCII_Compliant(std::u8string_view const p_input)
{
	for(const char8_t tchar: p_input)
	{
		if(tchar > 0x7F) return false;
	}
	return true;
}

bool ASCII_Compliant(std::u32string_view const p_input)
{
	for(const char32_t tchar: p_input)
	{
		if(tchar > 0x7F) return false;
	}
	return true;
}

std::optional<std::u8string> UTF8_to_ANSI(std::u8string_view const p_input)
{
	const std::optional<uintptr_t> res = _p::UTF8_to_ANSI_estimate(p_input);
	if(!res.has_value()) return {};
	std::u8string output;
	output.resize(res.value());
	_p::UTF8_to_ANSI_unsafe(p_input, output.data());
	return output;
}

std::optional<std::u8string> UTF16_to_ANSI(std::u16string_view const p_input)
{
	//Because ANSI code points can not be larger than 0xFF
	//if input has code points larger than 0xFF then it's already an error
	//Otherwise for all code points <0xFF UTF16 == UCS2
	return UCS2_to_ANSI(p_input);
}

std::optional<std::u8string> UCS2_to_ANSI(std::u16string_view const p_input)
{
	const std::optional<uintptr_t> res = _p::UCS2_to_ANSI_estimate(p_input);
	if(!res.has_value()) return {};
	std::u8string output;
	output.resize(res.value());
	_p::UCS2_to_ANSI_unsafe(p_input, output.data());
	return output;
}

std::optional<std::u8string> UCS4_to_ANSI(std::u32string_view const p_input)
{
	const std::optional<uintptr_t> res = _p::UCS4_to_ANSI_estimate(p_input);
	if(!res.has_value()) return {};
	std::u8string output;
	output.resize(res.value());
	_p::UCS4_to_ANSI_unsafe(p_input, output.data());
	return output;
}

std::u8string ANSI_to_UTF8(std::u8string_view const p_input)
{
	std::u8string output;
	output.resize(_p::ANSI_to_UTF8_estimate(p_input));
	_p::ANSI_to_UTF8_unsafe(p_input, output.data());
	return output;
}

std::optional<std::u8string> UTF16_to_UTF8(std::u16string_view const p_input)
{
	const std::optional<uintptr_t> res = _p::UTF16_to_UTF8_estimate(p_input);
	if(!res.has_value()) return {};
	std::u8string output;
	output.resize(res.value());
	_p::UTF16_to_UTF8_unsafe(p_input, output.data());
	return output;
}

std::u8string UCS2_to_UTF8(std::u16string_view const p_input)
{
	std::u8string output;
	output.resize(_p::UCS2_to_UTF8_estimate(p_input));
	_p::UCS2_to_UTF8_unsafe(p_input, output.data());
	return output;
}

std::optional<std::u8string> UCS4_to_UTF8(std::u32string_view const p_input)
{
	const std::optional<uintptr_t> res = _p::UCS4_to_UTF8_estimate(p_input);
	if(!res.has_value()) return {};

	std::u8string output;
	output.resize(res.value());
	_p::UCS4_to_UTF8_unsafe(p_input, output.data());
	return output;
}

std::u16string ANSI_to_UTF16(std::u8string_view const p_input)
{
	std::u16string output;
	output.resize(_p::ANSI_to_UTF16_estimate(p_input));
	_p::ANSI_to_UTF16_unsafe(p_input, output.data());
	return output;
}

std::optional<std::u16string> UTF8_to_UTF16(std::u8string_view const p_input)
{
	const std::optional<uintptr_t> res = _p::UTF8_to_UTF16_estimate(p_input);
	if(!res.has_value()) return {};
	std::u16string output;
	output.resize(res.value());
	_p::UTF8_to_UTF16_unsafe(p_input, output.data());
	return output;
}

std::optional<std::u16string> UCS2_to_UTF16(std::u16string_view const p_input)
{
	for(const char16_t tchar: p_input)
	{
		if(tchar > 0xD7FF && tchar < 0xE000) return {};
	}
	return {std::u16string{p_input}};
}

std::optional<std::u16string> UCS4_to_UTF16(std::u32string_view const p_input)
{
	const std::optional<uintptr_t> res = _p::UCS4_to_UTF16_estimate(p_input);
	if(!res.has_value()) return {};
	std::u16string output;
	output.resize(res.value());
	_p::UCS4_to_UTF16_unsafe(p_input, output.data());
	return output;
}

std::u16string ANSI_to_UCS2(std::u8string_view const p_input)
{
	std::u16string output;
	output.resize(_p::ANSI_to_UCS2_estimate(p_input));
	_p::ANSI_to_UCS2_unsafe(p_input, output.data());
	return output;
}

std::optional<std::u16string> UTF8_to_UCS2(std::u8string_view const p_input)
{
	const std::optional<uintptr_t> res = _p::UTF8_to_UCS2_estimate(p_input);
	if(!res.has_value()) return {};
	std::u16string output;
	output.resize(res.value());
	_p::UTF8_to_UCS2_unsafe(p_input, output.data());
	return output;
}

std::optional<std::u16string> UTF16_to_UCS2(std::u16string_view const p_input)
{
	for(const char16_t tchar: p_input)
	{
		if(tchar > 0xD7FF && tchar < 0xE000) return {};
	}
	return {std::u16string{p_input}};
}

std::optional<std::u16string> UCS4_to_UCS2(std::u32string_view const p_input)
{
	const std::optional<uintptr_t> res = _p::UCS4_to_UCS2_estimate(p_input);
	if(!res.has_value()) return {};
	std::u16string output;
	output.resize(res.value());
	_p::UCS4_to_UCS2_unsafe(p_input, output.data());
	return output;
}

std::u32string ANSI_to_UCS4(std::u8string_view const p_input)
{
	std::u32string output;
	output.resize(_p::ANSI_to_UCS4_estimate(p_input));
	_p::ANSI_to_UCS4_unsafe(p_input, output.data());
	return output;
}

std::optional<std::u32string> UTF8_to_UCS4(std::u8string_view const p_input)
{
	const std::optional<uintptr_t> res = _p::UTF8_to_UCS4_estimate(p_input);
	if(!res.has_value()) return {};
	std::u32string output;
	output.resize(res.value());
	_p::UTF8_to_UCS4_unsafe(p_input, output.data());
	return output;
}

std::optional<std::u32string> UTF16_to_UCS4(std::u16string_view const p_input)
{
	const std::optional<uintptr_t> res = _p::UTF16_to_UCS4_estimate(p_input);
	if(!res.has_value()) return {};
	std::u32string output;
	output.resize(res.value());
	_p::UTF16_to_UCS4_unsafe(p_input, output.data());
	return output;
}

std::u32string UCS2_to_UCS4(std::u16string_view const p_input)
{
	std::u32string output;
	output.resize(_p::UCS2_to_UCS4_estimate(p_input));
	_p::UCS2_to_UCS4_unsafe(p_input, output.data());
	return output;
}

std::u8string UTF8_to_ANSI_faulty(std::u8string_view const p_input, const char8_t p_placeHolder)
{
	std::u8string output;
	output.resize(_p::UTF8_to_ANSI_faulty_estimate(p_input));
	_p::UTF8_to_ANSI_faulty_unsafe(p_input, p_placeHolder, output.data());
	return output;
}

std::u8string UTF16_to_ANSI_faulty(std::u16string_view const p_input, const char8_t p_placeHolder)
{
	std::u8string output;
	output.resize(_p::UTF16_to_ANSI_faulty_estimate(p_input));
	_p::UTF16_to_ANSI_faulty_unsafe(p_input, p_placeHolder, output.data());
	return output;
}

std::u8string UCS2_to_ANSI_faulty(std::u16string_view const p_input, const char8_t p_placeHolder)
{
	std::u8string output;
	output.resize(_p::UCS2_to_ANSI_faulty_estimate(p_input));
	_p::UCS2_to_ANSI_faulty_unsafe(p_input, p_placeHolder, output.data());
	return output;
}

std::u8string UCS4_to_ANSI_faulty(std::u32string_view const p_input, const char8_t p_placeHolder)
{
	std::u8string output;
	output.resize(_p::UCS4_to_ANSI_faulty_estimate(p_input));
	_p::UCS4_to_ANSI_faulty_unsafe(p_input, p_placeHolder, output.data());
	return output;
}

std::u8string UTF16_to_UTF8_faulty(std::u16string_view const p_input, const char32_t p_placeHolder)
{
	std::u8string output;
	output.resize(_p::UTF16_to_UTF8_faulty_estimate(p_input, p_placeHolder));
	_p::UTF16_to_UTF8_faulty_unsafe(p_input, p_placeHolder, output.data());
	return output;
}


[[nodiscard]] std::u8string UCS4_to_UTF8_faulty(std::u32string_view const p_input, const char32_t p_placeHolder)
{
	std::u8string output;
	output.resize(_p::UCS4_to_UTF8_faulty_estimate(p_input, p_placeHolder));
	_p::UCS4_to_UTF8_faulty_unsafe(p_input, p_placeHolder, output.data());
	return output;
}


std::u16string UTF8_to_UTF16_faulty(std::u8string_view const p_input, const char32_t p_placeHolder)
{
	std::u16string output;
	output.resize(_p::UTF8_to_UTF16_faulty_estimate(p_input, p_placeHolder));
	_p::UTF8_to_UTF16_faulty_unsafe(p_input, p_placeHolder, output.data());
	return output;
}

std::u16string UCS2_to_UTF16_faulty(std::u16string_view const p_input, const char32_t p_placeHolder)
{
	std::u16string output;
	output.resize(_p::UCS2_to_UTF16_faulty_estimate(p_input, p_placeHolder));
	_p::UCS2_to_UTF16_faulty_unsafe(p_input, p_placeHolder, output.data());
	return output;
}

std::u16string UCS4_to_UTF16_faulty(std::u32string_view const p_input, const char32_t p_placeHolder)
{
	std::u16string output;
	output.resize(_p::UCS4_to_UTF16_faulty_estimate(p_input, p_placeHolder));
	_p::UCS4_to_UTF16_faulty_unsafe(p_input, p_placeHolder, output.data());
	return output;
}

std::u16string UTF8_to_UCS2_faulty(std::u8string_view const p_input, const char16_t p_placeHolder)
{
	std::u16string output;
	output.resize(_p::UTF8_to_UCS2_faulty_estimate(p_input));
	_p::UTF8_to_UCS2_faulty_unsafe(p_input, p_placeHolder, output.data());
	return output;
}

std::u16string UTF16_to_UCS2_faulty(std::u16string_view const p_input, const char16_t p_placeHolder)
{
	std::u16string output;
	output.resize(_p::UTF16_to_UCS2_faulty_estimate(p_input));
	_p::UTF16_to_UCS2_faulty_unsafe(p_input, p_placeHolder, output.data());
	return output;
}

std::u16string UCS4_to_UCS2_faulty(std::u32string_view const p_input, const char16_t p_placeHolder)
{
	std::u16string output;
	output.resize(_p::UCS4_to_UCS2_faulty_estimate(p_input));
	_p::UCS4_to_UCS2_faulty_unsafe(p_input, p_placeHolder, output.data());
	return output;
}

std::u32string UTF8_to_UCS4_faulty(std::u8string_view const p_input, const char32_t p_placeHolder)
{
	std::u32string output;
	output.resize(_p::UTF8_to_UCS4_faulty_estimate(p_input));
	_p::UTF8_to_UCS4_faulty_unsafe(p_input, p_placeHolder, output.data());
	return output;
}

std::u32string UTF16_to_UCS4_faulty(std::u16string_view const p_input, const char32_t p_placeHolder)
{
	std::u32string output;
	output.resize(_p::UTF16_to_UCS4_faulty_estimate(p_input));
	_p::UTF16_to_UCS4_faulty_unsafe(p_input, p_placeHolder, output.data());
	return output;
}


} //namespace core

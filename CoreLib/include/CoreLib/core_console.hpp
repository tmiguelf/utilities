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

#pragma once
#include <string_view>
#include "toPrint/toPrint_sink.hpp"

namespace core
{

class console_out: public sink_toPrint_base
{
public:
#if defined(_WIN32)
	inline console_out(void* const p_handle): m_handle(p_handle){}
#elif defined(__unix__)
	inline console_out(int const p_handle): m_handle(p_handle){}
#endif

	void write(std::string_view    p_out) const;
	void write(std::wstring_view   p_out) const;
	void write(std::u8string_view  p_out) const;
	void write(std::u16string_view p_out) const;
	void write(std::u32string_view p_out) const;

	void put(char     p_out) const;
	void put(wchar_t  p_out) const;
	void put(char8_t  p_out) const;
	void put(char16_t p_out) const;
	void put(char32_t p_out) const;

private:
#if defined(_WIN32)
	void* const m_handle;
#elif defined(__unix__)
	int const m_handle;
#endif
};

extern console_out const cout;
extern console_out const cerr;

} //namespace core

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

#pragma once

#include <string>
#include <string_view>
#include <initializer_list>

#include "core_string_encoding.hpp"
#include "core_string_tostream.hpp"

namespace core
{

#ifdef _WIN32
using os_char = wchar_t;
#else
using os_char = char;
#endif // _WIN32

/*namespace _p
{
#ifdef _WIN32
using os_fix_char_r = char16_t;
#else
using os_fix_char_r = char8_t;
#endif // _WIN32

}
*/

//using os_string = std::basic_string<os_char>;

class os_string: public std::basic_string<os_char>
{
private:
	using this_string_t = std::basic_string<os_char>;


public:
	using std::basic_string<os_char>::basic_string;
	using std::basic_string<os_char>::operator =;

	os_string(std::basic_string<os_char>&& p_other): basic_string<os_char>(p_other) {}
	os_string(const std::basic_string<os_char>& p_other): basic_string<os_char>(p_other) {}

	os_string(const os_string&) = default;
	os_string(os_string&&) = default;

	os_string(std::u32string_view p_string)
		:
#ifdef _WIN32
		basic_string(std::move(reinterpret_cast<this_string_t&>(UCS4_to_UCS2(p_string).value())))
#else
		basic_string(std::move(reinterpret_cast<this_string_t&>(UCS4_to_ANSI(p_string).value())))
#endif
	{}

	os_string& operator = (const os_string&) = default;
	os_string& operator = (os_string&&) = default;



#ifdef _WIN32
	inline void from_UTF		(std::u32string_view p_string) { this_string_t::operator = (std::move(reinterpret_cast<this_string_t&>(UCS4_to_UTF16(p_string).value()))); }
	inline void from_codePoints	(std::u32string_view p_string) { this_string_t::operator = (std::move(reinterpret_cast<this_string_t&>(UCS4_to_UCS2 (p_string).value()))); }

	[[nodiscard]] inline std::u8string  toUTF8 (char32_t p_placeholder) const { return UTF16_to_UTF8_faulty(reinterpret_cast<const std::u16string&>(*this), p_placeholder); }
	[[nodiscard]] inline std::u32string toUTF32(char32_t p_placeholder) const { return UTF16_to_UCS4_faulty(reinterpret_cast<const std::u16string&>(*this), p_placeholder); }
	[[nodiscard]] inline std::u32string toCodePoints() const { return UCS2_to_UCS4(reinterpret_cast<const std::u16string&>(*this)); }

	[[nodiscard]] static inline os_string make_from_UTF			(const std::u32string& p_string) { return std::move(reinterpret_cast<this_string_t&>(UCS4_to_UTF16(p_string).value())); }
	[[nodiscard]] static inline os_string make_from_codePoints	(const std::u32string& p_string) { return std::move(reinterpret_cast<this_string_t&>(UCS4_to_UCS2(p_string).value())); }
#else
	inline void from_UTF		(std::u32string_view p_string) { from_codePoints(p_string); }
	inline void from_codePoints	(std::u32string_view p_string) { this_string_t::operator = (std::move(reinterpret_cast<this_string_t&>(UCS4_to_ANSI(p_string).value()))); }
	[[nodiscard]] inline std::u8string  toUTF8 (char32_t) const { return ANSI_to_UTF8(reinterpret_cast<const std::u8string&>(*this)); }
	[[nodiscard]] inline std::u32string toUTF32(char32_t) const { return toCodePoints(); }
	[[nodiscard]] inline std::u32string toCodePoints() const { return ANSI_to_UCS4(reinterpret_cast<const std::u8string&>(*this)); }

	[[nodiscard]] static inline os_string make_from_UTF			(const std::u32string& p_string) { return make_from_codePoints(p_string); }
	[[nodiscard]] static inline os_string make_from_codePoints	(const std::u32string& p_string) { return std::move(reinterpret_cast<this_string_t&>(UCS4_to_ANSI(p_string).value())); }
#endif
};

class os_string_view: public std::basic_string_view<os_char>
{
private:
	using this_string_view_t = std::basic_string_view<os_char>;
public:
	using std::basic_string_view<os_char>::basic_string_view;

#ifdef _WIN32
	[[nodiscard]] inline std::u8string  toUTF8 (char32_t p_placeholder) const { return UTF16_to_UTF8_faulty(reinterpret_cast<const std::u16string&>(*this), p_placeholder); }
	[[nodiscard]] inline std::u32string toUTF32(char32_t p_placeholder) const { return UTF16_to_UCS4_faulty(reinterpret_cast<const std::u16string&>(*this), p_placeholder); }
	[[nodiscard]] inline std::u32string toCodePoints() const { return UCS2_to_UCS4(reinterpret_cast<const std::u16string&>(*this)); }
#else
	[[nodiscard]] inline std::u8string  toUTF8 (char32_t) const { return ANSI_to_UTF8(reinterpret_cast<const std::u8string&>(*this)); }
	[[nodiscard]] inline std::u32string toUTF32(char32_t) const { return toCodePoints(); }
	[[nodiscard]] inline std::u32string toCodePoints() const { return ANSI_to_UCS4(reinterpret_cast<const std::u8string&>(*this)); }
#endif

};

template<>
class toStream<os_string>
{
public:
	toStream(const os_string& p_data): m_data{p_data}{}
	inline void stream(std::ostream& p_stream) const
	{
#ifdef _WIN32
		const std::u8string& res =  core::UTF16_to_UTF8_faulty(reinterpret_cast<const std::u16string&>(m_data), '?');
		p_stream.write(reinterpret_cast<const char*>(res.data()), res.size());
#else
		p_stream.write(reinterpret_cast<const char*>(m_data.data()), m_data.size());
#endif
	}

private:
	const os_string& m_data;
};

template<>
class toStream<os_string_view>
{
public:
	toStream(os_string_view p_data): m_data{p_data}{}
	inline void stream(std::ostream& p_stream) const
	{
#ifdef _WIN32
		const std::u8string& res =  core::UTF16_to_UTF8_faulty(reinterpret_cast<const std::u16string_view&>(m_data), '?');
		p_stream.write(reinterpret_cast<const char*>(res.data()), res.size());
#else
		p_stream.write(reinterpret_cast<const char*>(m_data.data()), m_data.size());
#endif
	}

private:
	const os_string_view m_data;
};

}

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

#include <string>
#include <string_view>
#include <initializer_list>

#include "core_string_format.hpp"

namespace core
{

class os_string_view_win;

class os_string_win
{
public:
	os_string_win() = default;
	constexpr os_string_win(const os_string_win&) = default;
	constexpr os_string_win(os_string_win&&) = default;

	inline os_string_win(std::u16string&& p_string): m_string{std::move(p_string)} {}
	inline os_string_win(const std::u16string&& p_string): m_string{p_string} {}
	inline os_string_win(const char16_t* p_data, uintptr_t p_size): m_string{p_data, p_size} {}
	inline os_string_win(std::initializer_list<char16_t> p_list): m_string{p_list} {}

	inline os_string_win(const std::u8string& p_string): m_string{ANSI_to_UCS2(p_string)} {}
	inline os_string_win(const std::u32string& p_string): m_string{std::move(UCS4_to_UCS2(p_string).value())} {}

	inline void from_native(const std::u16string& p_string)	{ m_string = p_string; }
	inline void from_native(std::u16string&& p_string)		{ m_string = std::move(p_string); }

	inline void from_UTF(const std::u8string& p_string) { m_string = std::move(UTF8_to_UTF16(p_string).value()); }
	inline void from_UTF(const std::u32string& p_string) { m_string = std::move(UCS4_to_UTF16(p_string).value()); }

	inline void from_codePoints(const std::u8string& p_string) { m_string = ANSI_to_UCS2(p_string); }
	inline void from_codePoints(const std::u32string& p_string) { m_string = std::move(UCS4_to_UCS2(p_string).value()); }

	inline [[nodiscard]] std::u8string toUTF8 (char32_t p_placeholder) const { return UTF16_to_UTF8_faulty(m_string, p_placeholder); }
	inline [[nodiscard]] std::u32string toUTF32(char32_t p_placeholder) const { return UTF16_to_UCS4_faulty(m_string, p_placeholder); }

	constexpr [[nodiscard]] bool operator == (const os_string_win& p_other) { return m_string == p_other.m_string; }
	constexpr [[nodiscard]] bool operator != (const os_string_win& p_other) { return m_string != p_other.m_string; }

	os_string_win& operator = (const os_string_win&) = default;
	os_string_win& operator = (os_string_win&&) = default;

	inline os_string_win& operator += (const os_string_win& p_other) { m_string += p_other.m_string; return *this; }
	inline [[nodiscard]] os_string_win operator + (const os_string_win& p_other) { return {m_string + p_other.m_string}; }

	inline void swap(os_string_win& p_other) { m_string.swap(p_other.m_string); }
	inline void clear() { m_string.clear(); }

	constexpr [[nodiscard]] bool empty() const { return m_string.empty(); }

	inline				[[nodiscard]] char16_t*			data() { return m_string.data(); }
	inline constexpr	[[nodiscard]] const char16_t*	data() const { return m_string.data(); }
	inline constexpr	[[nodiscard]] uintptr_t			size() const { return m_string.size(); }

	inline				[[nodiscard]] std::u16string&		native	() { return m_string; }
	inline constexpr	[[nodiscard]] const std::u16string& native	() const { return m_string; }
	inline				[[nodiscard]] const char16_t*		c_str	() const { return m_string.c_str(); }

	static inline [[nodiscard]] os_string_win make_from_UTF			(const std::u8string& p_string)  { return std::move(UTF8_to_UTF16(p_string).value()); }
	static inline [[nodiscard]] os_string_win make_from_UTF			(const std::u32string& p_string) { return std::move(UCS4_to_UTF16(p_string).value()); }
	static inline [[nodiscard]] os_string_win make_from_codePoints	(const std::u8string& p_string)  { return ANSI_to_UCS2(p_string); }
	static inline [[nodiscard]] os_string_win make_from_codePoints	(const std::u32string& p_string) { return std::move(UCS4_to_UCS2(p_string).value()); }
private:
	std::u16string m_string;
};

class os_string_view_unix;

class os_string_unix
{
public:
	os_string_unix() = default;
	constexpr os_string_unix(const os_string_unix&) = default;
	constexpr os_string_unix(os_string_unix&&) = default;

	inline os_string_unix(std::u8string&& p_string): m_string{std::move(p_string)} {}
	inline os_string_unix(const std::u8string&& p_string): m_string{p_string} {}
	inline os_string_unix(const char8_t* p_data, uintptr_t p_size): m_string{p_data, p_size} {}
	inline os_string_unix(std::initializer_list<char8_t> p_list): m_string{p_list} {}

	inline os_string_unix(const std::u32string& p_string) : m_string{std::move(UCS4_to_ANSI(p_string).value())} {}

	inline void from_native(const std::u8string& p_string)	{ m_string = p_string; }
	inline void from_native(std::u8string&& p_string)		{ m_string = std::move(p_string); }

	inline void from_UTF(const std::u8string& p_string) { m_string = std::move(UTF8_to_ANSI(p_string).value()); }
	inline void from_UTF(const std::u32string& p_string) { m_string = std::move(UCS4_to_ANSI(p_string).value()); }

	inline void from_codePoints(const std::u8string& p_string) { m_string = p_string; }
	inline void from_codePoints(std::u8string&& p_string) { m_string = std::move(p_string); }

	inline void from_codePoints(const std::u32string& p_string) { m_string = std::move(UCS4_to_ANSI(p_string).value()); }

	[[nodiscard]] std::u8string toUTF8 (char32_t) const { return ANSI_to_UTF8(m_string); }
	[[nodiscard]] std::u32string toUTF32(char32_t) const { return ANSI_to_UCS4(m_string); }

	constexpr [[nodiscard]] bool operator == (const os_string_unix& p_other) { return m_string == p_other.m_string; }
	constexpr [[nodiscard]] bool operator != (const os_string_unix& p_other) { return m_string != p_other.m_string; }

	os_string_unix& operator = (const os_string_unix&) = default;
	os_string_unix& operator = (os_string_unix&&) = default;

	inline os_string_unix& operator += (const os_string_unix& p_other) { m_string += p_other.m_string; return *this; }
	inline [[nodiscard]] os_string_unix operator + (const os_string_unix& p_other) { return {m_string + p_other.m_string}; }

	inline void swap(os_string_unix& p_other) { m_string.swap(p_other.m_string); }
	inline void clear() { m_string.clear(); }

	constexpr [[nodiscard]] bool empty() const { return m_string.empty(); }

	inline				[[nodiscard]] char8_t*			data() { return m_string.data(); }
	inline constexpr	[[nodiscard]] const char8_t*	data() const { return m_string.data(); }
	inline constexpr	[[nodiscard]] uintptr_t			size() const { return m_string.size(); }

	inline				[[nodiscard]] std::u8string&		native	() { return m_string; }
	inline constexpr	[[nodiscard]] const std::u8string& native	() const { return m_string; }
	inline				[[nodiscard]] const char8_t*		c_str	() const { return m_string.c_str(); }

	static inline [[nodiscard]] os_string_win make_from_UTF			(const std::u8string& p_string)  { return std::move(UTF8_to_ANSI(p_string).value()); }
	static inline [[nodiscard]] os_string_win make_from_UTF			(const std::u32string& p_string) { return std::move(UCS4_to_ANSI(p_string).value()); }
	static inline [[nodiscard]] os_string_win make_from_codePoints	(const std::u8string& p_string)  { return p_string; }
	static inline [[nodiscard]] os_string_win make_from_codePoints	(std::u8string&& p_string)       { return p_string; }
	static inline [[nodiscard]] os_string_win make_from_codePoints	(const std::u32string& p_string) { return std::move(UCS4_to_ANSI(p_string).value()); }

private:
	std::u8string m_string;
};


#ifdef _WIN32
using os_string			= os_string_win;
#else
using os_string			= os_string_unix;
#endif

}

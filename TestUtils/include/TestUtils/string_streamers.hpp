//======== ======== ======== ======== ======== ======== ======== ========
///	\file
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

#include <concepts>
#include <cstdint>
#include <string>
#include <string_view>
#include <filesystem>
#include <CoreLib/string/core_string_encoding.hpp>
#include <CoreLib/string/core_string_numeric.hpp>
#include <CoreLib/string/core_wchar_alias.hpp>
#include <CoreLib/string/core_os_string.hpp>

#include "string_tostream.hpp"

namespace core
{

template<>
class toStream<std::u8string_view>
{
public:
	toStream(const std::u8string_view& p_data): m_data{p_data}{}
	inline void stream(std::ostream& p_stream) const { p_stream.write(reinterpret_cast<const char*>(m_data.data()), m_data.size()); }

private:
	const std::u8string_view& m_data;
};

template<>
class toStream<std::u16string_view>
{
public:
	toStream(const std::u16string_view& p_data): m_data{p_data}{}
	inline void stream(std::ostream& p_stream) const
	{
		const std::u8string& temp = core::UTF16_to_UTF8_faulty(m_data, '?');
		p_stream << toStream<std::u8string_view>{temp};
	}

private:
	const std::u16string_view& m_data;
};

template<>
class toStream<std::u32string_view>
{
public:
	toStream(const std::u32string_view& p_data): m_data{p_data}{}
	inline void stream(std::ostream& p_stream) const
	{
		const std::u8string& temp = core::UCS4_to_UTF8_faulty(m_data, '?');
		p_stream << toStream<std::u8string_view>{temp};
	}

private:
	const std::u32string_view& m_data;
};

template<>
class toStream<std::string_view>
{
public:
	toStream(const std::string_view& p_data): m_data{p_data}{}
	inline void stream(std::ostream& p_stream) const { p_stream.write(m_data.data(), m_data.size()); }

private:
	const std::string_view& m_data;
};

template<>
class toStream<std::wstring_view>: public toStream<std::basic_string_view<wchar_alias>>
{
public:
	toStream(const std::wstring_view& p_data):
		toStream<std::basic_string_view<wchar_alias>>(
			std::basic_string_view<wchar_alias>{reinterpret_cast<const wchar_alias*>(p_data.data()), p_data.size()}
		)
	{}
};

template<typename T>
class toStream<std::basic_string<T>>: public toStream<std::basic_string_view<T>>
{
public:
	toStream(const std::basic_string<T>& p_data): toStream<std::basic_string_view<T>>(std::basic_string_view<T>{p_data}){}
};


template<char_conv_dec_supported_c num_T>
void num2stream(std::ostream& p_stream, const num_T& p_data)
{
	constexpr uintptr_t buffSize = to_chars_dec_max_digits_v<num_T>;
	char8_t buff[buffSize];
	p_stream.write(reinterpret_cast<const char*>(buff), to_chars(p_data, buff));
}

template<char_conv_hex_supported_c num_T>
void num2stream_hex(std::ostream& p_stream, const num_T& p_data)
{
	constexpr uintptr_t buffSize = to_chars_hex_max_digits_v<num_T>;
	char8_t buff[buffSize];
	p_stream.write(reinterpret_cast<const char*>(buff), to_chars_hex(p_data, buff));
}

template<char_conv_hex_supported_c num_T>
void num2stream_hex_fix(std::ostream& p_stream, const num_T& p_data)
{
	constexpr uintptr_t buffSize = to_chars_hex_max_digits_v<num_T>;
	char8_t buff[buffSize];
	to_chars_hex_fix(p_data, buff);
	p_stream.write(reinterpret_cast<const char*>(buff), buffSize);
}

template<char_conv_dec_supported_c num_T>
class toStream<num_T>
{
public:
	constexpr toStream(num_T p_data): m_data{p_data}{}
	inline void stream(std::ostream& p_stream) const
	{
		num2stream(p_stream, m_data);
	}

private:
	const num_T m_data;
};


template<>
class toStream<void*>
{
public:
	toStream(const void* p_data): m_data{p_data}{}
	inline void stream(std::ostream& p_stream) const
	{
		constexpr std::u8string_view prefix = u8"0x";
		p_stream << toStream<std::u8string_view>{prefix} << toStream<uintptr_t, toStreamForwardMethod>{reinterpret_cast<uintptr_t>(m_data), num2stream_hex_fix<uintptr_t>};
	}

private:
	const void* m_data;
};

template<>
class toStream<std::filesystem::path>: public toStream<std::basic_string_view<os_char>>
{
public:
	toStream(const std::filesystem::path& p_data)
		: toStream<std::basic_string_view<os_char>>(std::basic_string_view<os_char>{p_data.native()})
	{}
};

} //namespace core

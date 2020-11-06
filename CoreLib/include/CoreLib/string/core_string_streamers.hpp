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
///		
///	\todo	Provide a comprehensive and consistent set of error codes, to give
///			extra information regarding he nature of the failure
//======== ======== ======== ======== ======== ======== ======== ========

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include "core_string_tostream.hpp"
#include "core_string_format.hpp"
#include "core_string_numeric.hpp"

namespace core
{

template<>
class toStream<std::u8string>
{
public:
	toStream(const std::u8string& p_data): m_data{p_data}{}
	inline void stream(std::ostream& p_stream) const { p_stream.write(reinterpret_cast<const char*>(m_data.data()), m_data.size()); }

private:
	const std::u8string& m_data;
};

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
class toStream<std::u16string>
{
public:
	toStream(const std::u16string& p_data): m_data{p_data}{}
	inline void stream(std::ostream& p_stream) const
	{
		p_stream << toStream<std::u8string>{core::UTF16_to_UTF8_faulty(m_data, '?')};
	}

private:
	const std::u16string& m_data;
};

template<>
class toStream<std::u16string_view>
{
public:
	toStream(const std::u16string_view& p_data): m_data{p_data}{}
	inline void stream(std::ostream& p_stream) const
	{
		p_stream << toStream<std::u8string>{core::UTF16_to_UTF8_faulty(m_data, '?')};
	}

private:
	const std::u16string_view& m_data;
};


template<typename num_T, std::enable_if_t<from_chars_supported_v<num_T>, int> = 0>
void num2stream(std::ostream& p_stream, const num_T& p_data)
{
	constexpr uintptr_t buffSize = to_chars_max_digits_v<num_T>;
	char8_t buff[buffSize];
	p_stream.write(reinterpret_cast<const char*>(buff), to_chars<char8_t>(p_data, buff));
}

template<typename num_T, std::enable_if_t<from_hex_chars_supported_v<num_T>, int> = 0>
void num2stream_hex(std::ostream& p_stream, const num_T& p_data)
{
	constexpr uintptr_t buffSize = to_hex_chars_max_digits_v<num_T>;
	char8_t buff[buffSize];
	p_stream.write(reinterpret_cast<const char*>(buff), to_hex_chars<char8_t>(p_data, buff));
}

template<typename num_T, std::enable_if_t<from_hex_chars_supported_v<num_T>, int> = 0>
void num2stream_hex_fix(std::ostream& p_stream, const num_T& p_data)
{
	constexpr uintptr_t buffSize = to_hex_chars_max_digits_v<num_T>;
	char8_t buff[buffSize];
	to_hex_chars_fix<char8_t>(p_data, buff);
	p_stream.write(reinterpret_cast<const char*>(buff), buffSize);
}

template<typename num_T>
class toStream<num_T, std::enable_if_t<from_chars_supported_v<num_T>, void>>
{
public:
	toStream(num_T p_data): m_data{p_data}{}
	inline void stream(std::ostream& p_stream) const
	{
		num2stream(p_stream, m_data);
	}

private:
	const num_T m_data;
};

#if !defined(_MSC_BUILD)
template<typename fp_T>
class toStream<fp_T, std::enable_if_t<std::is_floating_point_v<fp_T>, void>>
{
public:
	toStream(fp_T p_data): m_data{p_data}{}
	inline void stream(std::ostream& p_stream) const
	{
		p_stream << m_data;
	}

private:
	const fp_T m_data;
};
#endif

} //namespace core


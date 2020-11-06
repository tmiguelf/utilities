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

#include <ostream>
#include <type_traits>

namespace core
{

enum class toStreamForwardMethod:uint8_t {};

template<typename, typename = void>
class toStream;

//CTAD deduction guide
template <typename T> toStream(T) -> toStream<std::remove_cvref_t<T>>;
template <typename T> toStream(T, void(*)(std::ostream&, const std::remove_cvref_t<T>&)) -> toStream<std::remove_cvref_t<T>, toStreamForwardMethod>;

template<typename T>
class toStream<T, toStreamForwardMethod>
{
public:
	using method_t = void (*)(std::ostream&, const T&);

public:
	toStream(T p_data, method_t p_method): m_data{p_data}, m_method{p_method}{}
	inline void stream(std::ostream& p_stream) const
	{
		m_method(p_stream, m_data);
	}

private:
	const T m_data;
	method_t m_method;
};

} //namespace core

template<typename T1, typename T2>
static std::ostream& operator << (std::ostream& p_stream, const core::toStream<T1, T2>& p_data)
{
	p_data.stream(p_stream);
	return p_stream;
}


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

#pragma once

#include <type_traits>

namespace mathlib
{
	template<typename T>
	constexpr std::enable_if_t<std::is_floating_point_v<T>, T> pi			= T{3.14159265358979323846l};

	template<typename T>
	constexpr std::enable_if_t<std::is_floating_point_v<T>, T> pi_2			= T{1.57079632679489661923l};

	template<typename T>
	constexpr std::enable_if_t<std::is_floating_point_v<T>, T> pi_4			= T{0.785398163397448309616l};

	template<typename T>
	constexpr std::enable_if_t<std::is_floating_point_v<T>, T> tau			= T{6.283185307179586476925l};

	template<typename T>
	constexpr std::enable_if_t<std::is_floating_point_v<T>, T> squareRoot_2	= T{1.41421356237309504880l};

	template<typename T>
	constexpr std::enable_if_t<std::is_floating_point_v<T>, T> e			= T{2.71828182845904523536l};

}	//namespace mathLib
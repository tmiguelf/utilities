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
#include <array>

namespace mathlib
{

	/// \brief Algebraic vector
	//
	template <typename T, size_t T_size, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
	class Vector: public std::array<T, T_size>
	{
	public:
		using this_t = Vector;
		using container_t = std::array<T, T_size>;

	public:
		[[nodiscard]] inline static constexpr size_t size() { return T_size; }

		using container_t::operator [];

		inline this_t& operator = (const this_t& p_other) = default;


		[[nodiscard]] inline bool operator == (const this_t& p_other) const
		{
			return static_cast<const container_t&>(*this) == static_cast<const container_t&>(p_other);
		}

		[[nodiscard]] inline bool operator != (const this_t& p_other) const
		{
			return !operator == (p_other);
		}

		inline this_t& operator += (const this_t& p_other)
		{
			for(size_t i = 0; i < T_size; ++i)
			{
				(*this)[i] += p_other[i];
			}
			return *this;
		}

		inline this_t& operator -= (const this_t& p_other)
		{
			for(size_t i = 0; i < T_size; ++i)
			{
				(*this)[i] -= p_other[i];
			}
			return *this;
		}

		template <typename O_T, std::enable_if_t<std::is_arithmetic_v<O_T>, int> = 0>
		inline this_t& operator *= (O_T p_scalar)
		{
			for(T& obj: *this)
			{
				obj *= p_scalar;
			}
			return *this;
		}

		template <typename O_T, std::enable_if_t<std::is_arithmetic_v<O_T>, int> = 0>
		inline this_t& operator /= (O_T p_scalar)
		{
			for(T& obj: *this)
			{
				obj /= p_scalar;
			}
			return *this;
		}

		[[nodiscard]] inline constexpr this_t operator + (const this_t& p_other) const
		{
			return this_t {*this} += p_other;
		}

		[[nodiscard]] inline constexpr this_t operator - (const this_t& p_other) const
		{
			return this_t{*this} -= p_other;
		}

		[[nodiscard]] inline constexpr this_t operator - () const
		{
			this_t temp{*this};
			for(T& obj: temp)
			{
				obj = -obj;
			}
			return temp;
		}

		template <typename O_T, std::enable_if_t<std::is_arithmetic_v<O_T>, int> = 0>
		[[nodiscard]] inline constexpr this_t operator * (O_T p_scalar) const
		{
			return this_t{*this} *= p_scalar;
		}

		template <typename O_T, std::enable_if_t<std::is_arithmetic_v<O_T>, int> = 0>
		[[nodiscard]] inline constexpr this_t operator / (O_T p_scalar) const
		{
			return this_t{*this} /= p_scalar;
		}

		[[nodiscard]] inline constexpr T internalProduct (const this_t& p_other) const
		{
			T res = 0;
			for(size_t i = 0; i < T_size; ++i)
			{
				res += (*this)[i] * p_other[i];
			}
			return res;
		}

		[[nodiscard]] inline constexpr T operator * (const this_t& p_other) const
		{
			return internalProduct(p_other);
		}

		inline this_t& operator = (const container_t& p_other)
		{
			container_t::operator = (p_other);
			return *this;
		}

		inline this_t& operator = (container_t&& p_other)
		{
			container_t::operator = (p_other);
			return *this;
		}
	};

	///	\brief Calculates the cross product between 2 vectors
	///	\note
	///		Cross product is Only defined for R3
	//
	template <typename T>
	[[nodiscard]] constexpr Vector<T, 3> crossProduct(const Vector<T, 3>& p_1, const Vector<T, 3>& p_2)
	{
		//Sarrus's rule
		return Vector<T, 3>
			{
				p_1[1] * p_2[2] - p_1[2] * p_2[1],
				p_1[2] * p_2[0] - p_1[0] * p_2[2],
				p_1[0] * p_2[1] - p_1[1] * p_2[0],
			};
	}

}	//namespace mathLib


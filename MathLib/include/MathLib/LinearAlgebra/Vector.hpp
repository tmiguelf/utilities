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

#include <type_traits>
#include <array>
#include <cmath>
#include <optional>


#include <MathLib/_p/mathlib_type_help.hpp>

namespace mathlib
{

	/// \brief Algebraic vector
	//
	template <_p::is_non_const_arithmetic T, uintptr_t T_size> requires (T_size > 0)
	class Vector: public std::array<T, T_size>
	{
	public:
		using this_t = Vector;
		using container_t = std::array<T, T_size>;

	public:
		[[nodiscard]] inline static constexpr uintptr_t size() { return T_size; }

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
			for(uintptr_t i = 0; i < T_size; ++i)
			{
				(*this)[i] += p_other[i];
			}
			return *this;
		}

		inline this_t& operator -= (const this_t& p_other)
		{
			for(uintptr_t i = 0; i < T_size; ++i)
			{
				(*this)[i] -= p_other[i];
			}
			return *this;
		}

		template <_p::is_arithmetic O_T>
		inline this_t& operator *= (const O_T p_scalar)
		{
			for(T& obj: *this)
			{
				obj *= p_scalar;
			}
			return *this;
		}

		template <_p::is_arithmetic O_T>
		inline this_t& operator /= (const O_T p_scalar)
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

		template <_p::is_arithmetic O_T>
		[[nodiscard]] inline constexpr this_t operator * (const O_T p_scalar) const
		{
			return this_t{*this} *= p_scalar;
		}

		template <_p::is_arithmetic O_T>
		[[nodiscard]] inline constexpr this_t operator / (const O_T p_scalar) const
		{
			return this_t{*this} /= p_scalar;
		}

		[[nodiscard]] inline constexpr T internalProduct (const this_t& p_other) const
		{
			T res = 0;
			for(uintptr_t i = 0; i < T_size; ++i)
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

		[[nodiscard]] inline std::optional<Vector> normal() const
		{
			if constexpr (T_size == 1)
			{
				if(operator [](0) > 0)
				{
					return Vector{static_cast<T>(1)};
				}
				else if constexpr(std::is_signed_v<T>)
				{
					if(operator [](0) < 0)
					{
						return Vector{static_cast<T>(-1)};
					}
				}
			}
			else if constexpr (T_size < 4)
			{
				auto res = hypot(*this);
				if(res > 0)
				{
					return *this / res;
				}
			}
			else
			{
				auto res = std::sqrt(*this * *this);
				if(res > 0)
				{
					return *this / res;
				}
			}
			return {};
		}

		[[nodiscard]] inline Vector fast_normal() const
		{
			if constexpr (T_size == 1)
			{
				if(operator [](0) > 0)
				{
					return Vector{static_cast<T>(1)};
				}
				else if constexpr(std::is_signed_v<T>)
				{
					if(operator [](0) < 0)
					{
						return Vector{static_cast<T>(-1)};
					}
				}
			}
			else
			{
				auto res = std::sqrt(*this * *this);
				if(res > T{0})
				{
					return *this / res;
				}
			}
			Vector temp{};
			temp[0] = T{1};
			return temp;
		}
	};


	template <_p::is_arithmetic T1, _p::is_non_const_arithmetic T2, uintptr_t T_size>
	[[nodiscard]] Vector<T2, T_size> operator * (const T1 p_1, const Vector<T2, T_size>& p_2)
	{
		return p_2 * p_1;
	}


	template<typename T, uintptr_t T_size> requires (T_size > 1 && T_size < 4)
	[[nodiscard]] inline auto hypot(const Vector<T, T_size>& p_vect)
	{
		if constexpr(T_size == 2)
		{
			return std::hypot(p_vect[0], p_vect[1]);
		}
		else
		{
			return std::hypot(p_vect[0], p_vect[1], p_vect[2]);
		}
	}

	///	\brief Calculates the cross product between 2 vectors
	///	\note
	///		Cross product is Only defined for R3
	template <_p::is_non_const_arithmetic T>
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


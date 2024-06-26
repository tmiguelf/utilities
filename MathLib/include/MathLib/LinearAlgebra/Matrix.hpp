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
#include <utility>

#include "Vector.hpp"
#include <MathLib/_p/mathlib_type_help.hpp>

namespace mathlib
{
	/// \brief Matrix with a 2D storage space
	//
	template <_p::is_non_const_arithmetic T, uintptr_t T_size1, uintptr_t T_size2>
	class Matrix
	{
	public:
		using this_t = Matrix;

		using line_t = typename std::array<typename std::remove_cv<T>::type, T_size2>;
		using init_t = typename std::array<line_t, T_size1>;

		using container_t = T[T_size1][T_size2];

	private:
		container_t _M{};

	public:
		constexpr Matrix()				= default;
		constexpr Matrix(this_t const&)	= default;
	
		constexpr Matrix(init_t const& p_data)
		{
			set(p_data);
		}

		this_t& operator = (this_t const&)	= default;

		inline this_t& operator = (init_t const& p_data)
		{
			set(p_data);
			return *this;
		}

		[[nodiscard]] bool operator == (this_t const& p_other) const
		{
			return memcmp(this, &p_other, sizeof(this_t)) == 0;
		}

		[[nodiscard]] bool operator != (this_t const& p_other) const
		{
			return memcmp(this, &p_other, sizeof(this_t)) != 0;
		}

		void set(uintptr_t const p_pos1, uintptr_t const p_pos2, T const p_newvalue)
		{
			_M[p_pos1][p_pos2] = p_newvalue;
		}

		void set(init_t const& p_data)
		{
			for(uintptr_t i = 0; i < T_size1; ++i)
			{
				for(uintptr_t j = 0; j < T_size2; ++j)
				{
					_M[i][j] = p_data[i][j];
				}
			}
		}

		inline T* operator [] (uintptr_t const p_pos)
		{
			return _M[p_pos];
		}

		inline const T* operator [] (uintptr_t const p_pos) const
		{
			return _M[p_pos];
		}

		[[nodiscard]] inline T& get(uintptr_t const p_pos1, uintptr_t const p_pos2)
		{
			return _M[p_pos1][p_pos2];
		}

		[[nodiscard]] inline constexpr T get(uintptr_t const p_pos1, uintptr_t const p_pos2) const
		{
			return _M[p_pos1][p_pos2];
		}

		static constexpr std::pair<uintptr_t, uintptr_t> size() { return std::pair<uintptr_t, uintptr_t>(T_size1, T_size2); }
		static constexpr uintptr_t size1() { return T_size1; }
		static constexpr uintptr_t size2() { return T_size2; }

		this_t& operator += (this_t const& p_other)
		{
			for(uintptr_t i = 0; i < T_size1; ++i)
			{
				for(uintptr_t j = 0; j < T_size2; ++j)
				{
					_M[i][j] += p_other._M[i][j];
				}
			}
			return *this;
		}

		this_t& operator -= (this_t const& p_other)
		{
			for(uintptr_t i = 0; i < T_size1; ++i)
			{
				for(uintptr_t j = 0; j < T_size2; ++j)
				{
					_M[i][j] -= p_other._M[i][j];
				}
			}
			return *this;
		}

		template <_p::is_arithmetic O_T>
		constexpr this_t& operator *= (O_T const p_scalar)
		{
			for(uintptr_t i = 0; i < T_size1; ++i)
			{
				for(uintptr_t j = 0; j < T_size2; ++j)
				{
					_M[i][j] *= p_scalar;
				}
			}
			return *this;
		}

		template <_p::is_arithmetic O_T>
		constexpr this_t& operator /= (O_T const p_scalar)
		{
			for(uintptr_t i = 0; i < T_size1; ++i)
			{
				for(uintptr_t j = 0; j < T_size2; ++j)
				{
					_M[i][j] /= p_scalar;
				}
			}
			return *this;
		}

		[[nodiscard]] inline constexpr this_t operator + (this_t const& p_other) const
		{
			return this_t {*this} += p_other;
		}

		[[nodiscard]] inline constexpr this_t operator - (this_t const& p_other) const
		{
			return this_t{*this} -= p_other;
		}

		template <_p::is_arithmetic O_T>
		[[nodiscard]] inline constexpr this_t operator * (O_T const p_scalar) const
		{
			return this_t{*this} *= p_scalar;
		}

		template <_p::is_arithmetic O_T>
		[[nodiscard]] inline constexpr this_t operator / (O_T const p_scalar) const
		{
			return this_t{*this} /= p_scalar;
		}

		[[nodiscard]] constexpr Vector<T, T_size1> operator * (Vector<T, T_size2> const& p_vect) const
		{
			Vector<T, T_size1> p_res;

			for(uintptr_t i = 0; i < T_size1; ++i)
			{
				T temp = 0;
				for(uintptr_t j = 0; j < T_size2; ++j)
				{
					temp += _M[i][j] * p_vect[j];
				}

				p_res[i] = temp;
			}

			return p_res;
		}

		template <uintptr_t OT_size1, uintptr_t OT_size2> requires (OT_size1 == T_size2)
		[[nodiscard]] constexpr Matrix<T, T_size1, OT_size2> operator * (Matrix<T, OT_size1, OT_size2> const& p_mat) const
		{
			static_assert(OT_size1 == T_size2, "Matrix must have compatible dimmensions");
			Matrix<T, T_size1, OT_size2> p_res;

			for(uintptr_t i = 0; i < OT_size2; ++i)
			{
				for(uintptr_t j = 0; j < T_size1; ++j)
				{
					T temp = 0;
					for(uintptr_t k = 0; k < T_size2; ++k)
					{
						temp += _M[j][k] * p_mat[k][i];
					}
					p_res.set(j, i, temp);
				}
			}

			return p_res;
		}

		[[nodiscard]] constexpr Matrix<T, T_size2, T_size1> transpose() const
		{
			Matrix<T, T_size2, T_size1> t_out;
			for(uintptr_t i = 0; i < T_size1; ++i)
			{
				for(uintptr_t j = 0; j < T_size2; ++j)
				{
					t_out.set(j, i, _M[i][j]);
				}
			}
			return t_out;
		}

		[[nodiscard]] constexpr this_t operator - () const
		{
			this_t t_out;
			for(uintptr_t i = 0; i < T_size1; ++i)
			{
				for(uintptr_t j = 0; j < T_size2; ++j)
				{
					t_out.set(i, j, -_M[i][j]);
				}
			}
			return t_out;
		}

		//TODO:
		//	Determinant - requires a conditional enable, the ability to create a sub matrix and recursion, but we don't need it for now
	};


	template <_p::is_arithmetic T1, _p::is_non_const_arithmetic T2, uintptr_t T_size1, uintptr_t T_size2>
	[[nodiscard]]  Matrix<T2, T_size1, T_size2> operator * (T1 const p_1, Matrix<T2, T_size1, T_size2> const& p_2)
	{
		return p_2 * p_1;
	}

	///	\brief calculates the trace of the matrix
	///	\note
	///		Trace is only defined for a square matrix
	template <typename T, uintptr_t T_size>
	[[nodiscard]] constexpr T trace(Matrix<T, T_size, T_size> const& p_mat)
	{
		T res = 0;
		for(uintptr_t i = 0; i < T_size; ++i)
		{
			res += p_mat[i][i];
		}
		return res;
	}
}



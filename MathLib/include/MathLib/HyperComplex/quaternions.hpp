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
///
/// \notes
///		ii = jj = kk = ijk = -1
///		ij = k				x| 1  i  j  k
///		jk = i				-+-----------
///		ki = j				1| 1  i  j  k
///		ji = -k				i| i -1  k -j
///		kj = -i				j| j -k -1  i
///		ik = -j				k| k  j -i -1
//======== ======== ======== ======== ======== ======== ======== ========

#pragma once

#include <type_traits>
#include <array>
#include <cmath>
#include <optional>

#include <MathLib/_p/mathlib_type_help.hpp>

namespace mathlib
{

template <_p::is_non_const_floating_point T>
class Quaternion
{
public:
//======== constexpr ========
//-------- constructors --------
	constexpr Quaternion() = default;
	constexpr Quaternion(Quaternion const&) = default;
	constexpr Quaternion& operator = (Quaternion const&) = default;
	constexpr Quaternion(T p_real, T p_i, T p_j, T p_k) : m_data{p_real, p_i, p_j, p_k} {}

//-------- accessors --------
	inline constexpr T r() const { return m_data[0]; };
	inline constexpr T i() const { return m_data[1]; };
	inline constexpr T j() const { return m_data[2]; };
	inline constexpr T k() const { return m_data[3]; };

//-------- operators --------
	constexpr bool operator == (Quaternion const& p_other) const
	{
		return 
			m_data[0] == p_other.m_data[0] &&
			m_data[1] == p_other.m_data[1] &&
			m_data[2] == p_other.m_data[2] &&
			m_data[3] == p_other.m_data[3];
	}

	inline constexpr bool operator != (Quaternion const& p_other) const { return !operator == (p_other); }

	constexpr Quaternion operator + (Quaternion const& p_other) const
	{
		return Quaternion
		{
			m_data[0] + p_other.m_data[0],
			m_data[1] + p_other.m_data[1],
			m_data[2] + p_other.m_data[2],
			m_data[3] + p_other.m_data[3],
		};
	}

	constexpr Quaternion operator - (Quaternion const& p_other) const
	{
		return Quaternion
		{
			m_data[0] - p_other.m_data[0],
			m_data[1] - p_other.m_data[1],
			m_data[2] - p_other.m_data[2],
			m_data[3] - p_other.m_data[3],
		};
	}

	constexpr Quaternion operator - () const
	{
		return Quaternion
		{
			-m_data[0],
			-m_data[1],
			-m_data[2],
			-m_data[3],
		};
	}

	constexpr Quaternion operator * (Quaternion const& p_other) const
	{
		return Quaternion
			{
				r() * p_other.r() - i() * p_other.i() - j() * p_other.j() - k() * p_other.k(),
				r() * p_other.i() + i() * p_other.r() + j() * p_other.k() - k() * p_other.j(),
				r() * p_other.j() - i() * p_other.k() + j() * p_other.r() + k() * p_other.i(),
				r() * p_other.k() + i() * p_other.j() - j() * p_other.i() + k() * p_other.r(),
			};

		//the following would give extra precision, but it is also slower and uneven
		//need to figure out how to implement partial sums more effectively or make an optional precision
		//return Quaternion
		//	{
		//		std::fma(-k(), p_other.k(), std::fma(-j(), p_other.j(), std::fma(-i(), p_other.i(), r() * p_other.r()))),
		//		std::fma(-k(), p_other.j(), std::fma( j(), p_other.k(), std::fma( i(), p_other.r(), r() * p_other.i()))),
		//		std::fma( k(), p_other.i(), std::fma( j(), p_other.r(), std::fma(-i(), p_other.k(), r() * p_other.j()))),
		//		std::fma( k(), p_other.r(), std::fma(-j(), p_other.i(), std::fma( i(), p_other.j(), r() * p_other.k())))
		//	};
	}

	constexpr Quaternion operator * (T const p_scalar) const
	{
		return Quaternion
		{
			m_data[0] * p_scalar,
			m_data[1] * p_scalar,
			m_data[2] * p_scalar,
			m_data[3] * p_scalar,
		};
	}

	constexpr Quaternion operator / (T const p_scalar) const
	{
		return Quaternion
		{
			m_data[0] / p_scalar,
			m_data[1] / p_scalar,
			m_data[2] / p_scalar,
			m_data[3] / p_scalar,
		};
	}

//-------- transformations --------
	constexpr Quaternion conjugate() const
	{
		return Quaternion{m_data[0], -m_data[1], -m_data[2], -m_data[3]};
	}

	constexpr std::optional<Quaternion> inverse() const
	{
		//note
		//	q^-1 = q*/|q|^2
		//	q* - conjugate
		//	|q| - magnitude/norm
		T normS = norm_squared();
		if(normS == T{0.0})
		{
			return {};
		}
		return conjugate() / normS;
	}

	// \brief the norm squared
	constexpr T norm_squared() const
	{
		return
			m_data[0] * m_data[0] + 
			m_data[1] * m_data[1] +
			m_data[2] * m_data[2] +
			m_data[3] * m_data[3];
	}

//-------- other --------
	constexpr bool isZero() const
	{
		return
			m_data[0] == T{0.0} && 
			m_data[1] == T{0.0} && 
			m_data[2] == T{0.0} && 
			m_data[3] == T{0.0};
	}

//======== runtime ========
//-------- operators --------
	Quaternion& operator += (Quaternion const& p_other)
	{
		m_data[0] += p_other.m_data[0];
		m_data[1] += p_other.m_data[1];
		m_data[2] += p_other.m_data[2];
		m_data[3] += p_other.m_data[3];
		return *this;
	}

	Quaternion& operator -= (Quaternion const& p_other)
	{
		m_data[0] -= p_other.m_data[0];
		m_data[1] -= p_other.m_data[1];
		m_data[2] -= p_other.m_data[2];
		m_data[3] -= p_other.m_data[3];
		return *this;
	}

	Quaternion& operator *= (Quaternion const& p_other)
	{
		return operator = (operator * (p_other));
	}

	Quaternion& operator *= (T const p_scalar)
	{
		m_data[0] *= p_scalar;
		m_data[1] *= p_scalar;
		m_data[2] *= p_scalar;
		m_data[3] *= p_scalar;
		return *this;
	}

	Quaternion& operator /= (T const p_scalar)
	{
		m_data[0] /= p_scalar;
		m_data[1] /= p_scalar;
		m_data[2] /= p_scalar;
		m_data[3] /= p_scalar;
		return *this;
	}

//-------- transformations --------
	inline T norm() const { return std::sqrt(norm_squared()); }

	std::optional<Quaternion> renormalized() const
	{
		T const normV = norm();
		if(T{0} == normV)
		{
			return {};
		}
		return operator / (normV);
	}

//-------- setters --------
	void set(T const p_r, T const p_i, T const p_j, T const p_k)
	{
		m_data[0] = p_r;
		m_data[1] = p_i;
		m_data[2] = p_j;
		m_data[3] = p_k;
	}

	inline void setR(T const p_val) { m_data[0] = p_val; };
	inline void setI(T const p_val) { m_data[1] = p_val; };
	inline void setJ(T const p_val) { m_data[2] = p_val; };
	inline void setK(T const p_val) { m_data[3] = p_val; };

//-------- other --------
	bool isFinite() const
	{
		return
			std::isfinite(m_data[0]) &&
			std::isfinite(m_data[1]) &&
			std::isfinite(m_data[2]) &&
			std::isfinite(m_data[3]);
	}

private:
	std::array<T, 4> m_data {0, 0, 0, 0};
};


template <_p::is_arithmetic T1, _p::is_non_const_arithmetic T2>
[[nodiscard]] Quaternion<T2> operator * (T1 const p_1, Quaternion<T2> const& p_2)
{
	return p_2 * p_1;
}



} //namespace mathlib

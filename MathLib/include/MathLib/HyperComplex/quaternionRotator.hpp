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
#include <cmath>

#include "quaternions.hpp"
#include "MathLib/LinearAlgebra/Vector.hpp"
#include "MathLib/LinearAlgebra/Matrix.hpp"

namespace mathlib
{

template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
class QuaternionRotator
{
public:
	constexpr QuaternionRotator(const QuaternionRotator&) = default;

	constexpr QuaternionRotator()
		: m_identity{1.0, 0.0, 0.0, 0.0}
	{
	}

	constexpr QuaternionRotator(const Quaternion<T>& p_quat)
		: m_identity{p_quat.renormalized().value_or(Quaternion<T>{1.0, 0.0, 0.0, 0.0})}
	{
	}

	QuaternionRotator(const Vector<T, 3> p_vector)
	{
		T vx = p_vector[0];
		T vy = p_vector[1];
		T vz = p_vector[2];
		T norm = std::hypot(vx, vy, vz);

		if(norm > 0.0)
		{
			T S = sin(norm / T{2.0});
			T C = cos(norm / T{2.0});

			m_identity = Quaternion<T>{C, (vx / norm) * S, (vy / norm) * S, (vz / norm) * S};
		}
		else
		{
			m_identity = Quaternion<T>{1.0, 0.0, 0.0, 0.0};
		}
	}

	QuaternionRotator(const Vector<T, 3> p_vector, T p_rotation)
	{
		T vx = p_vector[0];
		T vy = p_vector[1];
		T vz = p_vector[2];
		T norm = std::hypot(vx, vy, vz);

		if(norm >= 0.0)
		{
			T S = std::sin(p_rotation / T{2.0});
			T C = std::cos(p_rotation / T{2.0});

			m_identity = Quaternion<T>{C, (vx / norm) * S, (vy / norm) * S, (vz / norm) * S};
		}
		else
		{
			m_identity = Quaternion<T>{1.0, 0.0, 0.0, 0.0};
		}
	}

	constexpr bool operator == (const QuaternionRotator& p_other) const 
	{
		return m_identity == p_other.m_identity;
	}

	QuaternionRotator& operator = (const QuaternionRotator&) = default;
	
	inline QuaternionRotator& operator *= (const QuaternionRotator& p_other)
	{
		return operator = (operator * (p_other));
	}

	constexpr QuaternionRotator operator * (const QuaternionRotator& p_other) const
	{
		return {m_identity * p_other.m_identity};
	}

	constexpr Vector<T, 3> rotate(const Vector<T, 3> p_vector) const
	{
		//technically the last multiplication should be the inverse of the rotation identity
		//but because the identity quaternion is unitary, the inverse and the conjugate are the same
		//conjugate is much easier to compute
		Quaternion<T> res = m_identity * Quaternion<T>{0.0, p_vector[0], p_vector[1], p_vector[2]} * m_identity.conjugate();
		return Vector<T, 3>{res.i(), res.j(), res.k()};
	}

	Quaternion<T> identity() const { return m_identity; }
	Quaternion<T> inverse () const { return m_identity.conjugate(); }

	Vector<T, 3> vector() const
	{
		T r = m_identity.r();
		T i = m_identity.i();
		T j = m_identity.j();
		T k = m_identity.k();

		T angle = std::acos(r) * T{2.0};
		T norm  = std::sqrt(1 - r * r);

		if(norm == T{0.0})
		{
			return {T{0.0}, T{0.0}, T{0.0}};
		}

		return Vector<T, 3>{i / norm, j / norm, k / norm} * angle;
	}

	Matrix<T, 3, 3> matrix() const
	{
		const T r = m_identity.r();
		const T i = m_identity.i();
		const T j = m_identity.j();
		const T k = m_identity.k();

		const T pr2 = r * r;
		const T pi2 = i * i;
		const T pj2 = j * j;
		const T pk2 = k * k;

		const T pri = r * i;
		const T prj = r * j;
		const T prk = r * k;
		const T pij = i * j;
		const T pik = i * k;
		const T pjk = j * k;

		using line_t = typename Matrix<T, 3, 3>::line_t;
		using init_t = typename Matrix<T, 3, 3>::init_t;

		return init_t
			{
				line_t{(pr2 + pi2) - (pj2 + pk2), T{2.0} * (pij - prk)		, T{2.0} * (pik + prj)		},
				line_t{T{2.0} * (pij + prk)		, (pr2 + pj2) - (pi2 + pk2)	, T{2.0} * (pjk - pri)		},
				line_t{T{2.0} * (pik - prj)		, T{2.0} * (pjk + pri)		, (pr2 + pk2) - (pi2 + pj2)	}
			};
	}

private:
	Quaternion<T> m_identity;
};

}

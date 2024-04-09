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
#include <cmath>

#include "quaternions.hpp"
#include <MathLib/LinearAlgebra/3D.hpp>
#include <MathLib/_p/mathlib_type_help.hpp>
#include <MathLib/constants.hpp>

namespace mathlib
{

template <_p::is_non_const_floating_point T>
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

	QuaternionRotator(const Vector3<T> p_axis_angle)
	{
		T vx = p_axis_angle[0];
		T vy = p_axis_angle[1];
		T vz = p_axis_angle[2];
		T norm = std::hypot(vx, vy, vz);

		if(norm > 0.0)
		{
			T S = std::sin(norm / T{2.0});
			T C = std::cos(norm / T{2.0});

			m_identity = Quaternion<T>{C, (vx / norm) * S, (vy / norm) * S, (vz / norm) * S};
		}
		else
		{
			m_identity = Quaternion<T>{1.0, 0.0, 0.0, 0.0};
		}
	}

	QuaternionRotator(const Vector3<T> p_vector, const T p_rotation)
	{
		const T vx = p_vector[0];
		const T vy = p_vector[1];
		const T vz = p_vector[2];
		const T norm = std::hypot(vx, vy, vz);

		if(norm >= 0.0)
		{
			const T S = std::sin(p_rotation / T{2.0});
			const T C = std::cos(p_rotation / T{2.0});

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

	constexpr Vector3<T> rotate(const Vector3<T> p_vector) const
	{
		//technically the last multiplication should be the inverse of the rotation identity
		//but because the identity quaternion is unitary, the inverse and the conjugate are the same
		//conjugate is much easier to compute
		Quaternion<T> res = m_identity * Quaternion<T>{0.0, p_vector[0], p_vector[1], p_vector[2]} * m_identity.conjugate();
		return Vector3<T>{res.i(), res.j(), res.k()};
	}

	Quaternion<T> identity() const { return m_identity; }
	Quaternion<T> inverse () const { return m_identity.conjugate(); }

	Vector3<T> axis_angle() const
	{
		const T r = m_identity.r();
		const T i = m_identity.i();
		const T j = m_identity.j();
		const T k = m_identity.k();

		const T norm  = std::sqrt(1 - r * r);

		if(norm == T{0.0})
		{
			return {T{0.0}, T{0.0}, T{0.0}};
		}

		T angle = std::acos(r) * T{2.0};
		if(angle > pi<T>)
		{
			angle -= tau<T>;
		}
		else if(angle <= -pi<T>)
		{
			angle += tau<T>;
		}

		return Vector3<T>{i, j, k} * angle / norm;
	}

	Matrix3<T> matrix() const
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

		using line_t = typename Matrix3<T>::line_t;
		using init_t = typename Matrix3<T>::init_t;

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

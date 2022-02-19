//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///		3D optimized versions of Vector and Matrix
///
///	\copyright
///		Copyright (c) Tiago Miguel Oliveira Freire
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
#include <utility>

#include <MathLib/_p/mathlib_type_help.hpp>

namespace mathlib
{

/// \brief Algebraic vector
template <_p::is_non_const_arithmetic T>
class Vector3: public std::array<T, 3>
{
public:
	using this_t = Vector3;
	using container_t = std::array<T, 3>;

public:
	[[nodiscard]] inline static constexpr uintptr_t size() { return 3; }

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
		(*this)[0] += p_other[0];
		(*this)[1] += p_other[1];
		(*this)[2] += p_other[2];
		return *this;
	}

	inline this_t& operator -= (const this_t& p_other)
	{
		(*this)[0] -= p_other[0];
		(*this)[1] -= p_other[1];
		(*this)[2] -= p_other[2];
		return *this;
	}

	template <_p::is_arithmetic O_T>
	inline this_t& operator *= (const O_T p_scalar)
	{
		(*this)[0] *= p_scalar;
		(*this)[1] *= p_scalar;
		(*this)[2] *= p_scalar;
		return *this;
	}

	template <_p::is_arithmetic O_T>
	inline this_t& operator /= (const O_T p_scalar)
	{
		(*this)[0] /= p_scalar;
		(*this)[1] /= p_scalar;
		(*this)[2] /= p_scalar;
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
		temp[0] = -temp[0];
		temp[1] = -temp[1];
		temp[2] = -temp[2];
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
		return (*this)[0] * p_other[0] + (*this)[1] * p_other[1] + (*this)[2] * p_other[2];
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

template <_p::is_arithmetic T1, _p::is_non_const_arithmetic T2>
[[nodiscard]] Vector3<T2> operator * (const T1 p_1, const Vector3<T2>& p_2)
{
	return p_2 * p_1;
}


template <_p::is_non_const_arithmetic T>
class Matrix3
{
public:
	using this_t = Matrix3;

	using line_t = typename std::array<typename std::remove_cv<T>::type, 3>;
	using init_t = typename std::array<line_t, 3>;

	using container_t = T[3][3];

private:
	container_t _M{};

public:
	constexpr Matrix3()				= default;
	constexpr Matrix3(const this_t&)	= default;

	constexpr Matrix3(const init_t& p_data)
	{
		set(p_data);
	}

	this_t& operator = (const this_t&)	= default;

	inline this_t& operator = (const init_t& p_data)
	{
		set(p_data);
		return *this;
	}

	[[nodiscard]] bool operator == (const this_t& p_other) const
	{
		return memcmp(this, &p_other, sizeof(this_t)) == 0;
	}

	[[nodiscard]] bool operator != (const this_t& p_other) const
	{
		return memcmp(this, &p_other, sizeof(this_t)) != 0;
	}

	void set(const uintptr_t p_pos1, const uintptr_t p_pos2, const T p_newvalue)
	{
		_M[p_pos1][p_pos2] = p_newvalue;
	}

	void set(const init_t& p_data)
	{
		_M[0][0] = p_data[0][0];
		_M[0][1] = p_data[0][1];
		_M[0][2] = p_data[0][2];

		_M[1][0] = p_data[1][0];
		_M[1][1] = p_data[1][1];
		_M[1][2] = p_data[1][2];

		_M[2][0] = p_data[2][0];
		_M[2][1] = p_data[2][1];
		_M[2][2] = p_data[2][2];
	}

	inline T* operator [] (const uintptr_t p_pos)
	{
		return _M[p_pos];
	}

	inline const T* operator [] (const uintptr_t p_pos) const
	{
		return _M[p_pos];
	}

	[[nodiscard]] inline T& get(const uintptr_t p_pos1, const uintptr_t p_pos2)
	{
		return _M[p_pos1][p_pos2];
	}

	[[nodiscard]] inline constexpr T get(const uintptr_t p_pos1, const uintptr_t p_pos2) const
	{
		return _M[p_pos1][p_pos2];
	}

	static constexpr std::pair<uintptr_t, uintptr_t> size() { return std::pair<uintptr_t, uintptr_t>(3, 3); }
	static constexpr uintptr_t size1() { return 3; }
	static constexpr uintptr_t size2() { return 3; }

	this_t& operator += (const this_t& p_other)
	{
		_M[0][0] += p_other._M[0][0];
		_M[0][1] += p_other._M[0][1];
		_M[0][2] += p_other._M[0][2];
		_M[1][0] += p_other._M[1][0];
		_M[1][1] += p_other._M[1][1];
		_M[1][2] += p_other._M[1][2];
		_M[2][0] += p_other._M[2][0];
		_M[2][1] += p_other._M[2][1];
		_M[2][2] += p_other._M[2][2];
		return *this;
	}

	this_t& operator -= (const this_t& p_other)
	{
		_M[0][0] -= p_other._M[0][0];
		_M[0][1] -= p_other._M[0][1];
		_M[0][2] -= p_other._M[0][2];
		_M[1][0] -= p_other._M[1][0];
		_M[1][1] -= p_other._M[1][1];
		_M[1][2] -= p_other._M[1][2];
		_M[2][0] -= p_other._M[2][0];
		_M[2][1] -= p_other._M[2][1];
		_M[2][2] -= p_other._M[2][2];
		return *this;
	}

	template <_p::is_arithmetic O_T>
	constexpr this_t& operator *= (const O_T p_scalar)
	{
		_M[0][0] *= p_scalar;
		_M[0][1] *= p_scalar;
		_M[0][2] *= p_scalar;
		_M[1][0] *= p_scalar;
		_M[1][1] *= p_scalar;
		_M[1][2] *= p_scalar;
		_M[2][0] *= p_scalar;
		_M[2][1] *= p_scalar;
		_M[2][2] *= p_scalar;

		return *this;
	}

	template <_p::is_arithmetic O_T>
	constexpr this_t& operator /= (const O_T p_scalar)
	{
		_M[0][0] /= p_scalar;
		_M[0][1] /= p_scalar;
		_M[0][2] /= p_scalar;
		_M[1][0] /= p_scalar;
		_M[1][1] /= p_scalar;
		_M[1][2] /= p_scalar;
		_M[2][0] /= p_scalar;
		_M[2][1] /= p_scalar;
		_M[2][2] /= p_scalar;
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

	[[nodiscard]] constexpr Vector3<T> operator * (const Vector3<T>& p_vect) const
	{
		Vector3<T> p_res;

		p_res[0] = _M[0][0] * p_vect[0] + _M[0][1] * p_vect[1] + _M[0][2] * p_vect[2];
		p_res[1] = _M[1][0] * p_vect[0] + _M[1][1] * p_vect[1] + _M[1][2] * p_vect[2];
		p_res[2] = _M[2][0] * p_vect[0] + _M[2][1] * p_vect[1] + _M[2][2] * p_vect[2];
		return p_res;
	}

	[[nodiscard]] constexpr Matrix3<T> operator * (const Matrix3<T>& p_mat) const
	{
		Matrix3<T> p_res;
		p_res.set(0, 0,  _M[0][0] * p_mat[0][0] +  _M[0][1] * p_mat[1][0] +  _M[0][2] * p_mat[2][0]);
		p_res.set(1, 0,  _M[1][0] * p_mat[0][0] +  _M[1][1] * p_mat[1][0] +  _M[1][2] * p_mat[2][0]);
		p_res.set(2, 0,  _M[2][0] * p_mat[0][0] +  _M[2][1] * p_mat[1][0] +  _M[2][2] * p_mat[2][0]);
		p_res.set(0, 1,  _M[0][0] * p_mat[0][1] +  _M[0][1] * p_mat[1][1] +  _M[0][2] * p_mat[2][1]);
		p_res.set(1, 1,  _M[1][0] * p_mat[0][1] +  _M[1][1] * p_mat[1][1] +  _M[1][2] * p_mat[2][1]);
		p_res.set(2, 1,  _M[2][0] * p_mat[0][1] +  _M[2][1] * p_mat[1][1] +  _M[2][2] * p_mat[2][1]);
		p_res.set(0, 2,  _M[0][0] * p_mat[0][2] +  _M[0][1] * p_mat[1][2] +  _M[0][2] * p_mat[2][2]);
		p_res.set(1, 2,  _M[1][0] * p_mat[0][2] +  _M[1][1] * p_mat[1][2] +  _M[1][2] * p_mat[2][2]);
		p_res.set(2, 2,  _M[2][0] * p_mat[0][2] +  _M[2][1] * p_mat[1][2] +  _M[2][2] * p_mat[2][2]);
		return p_res;
	}

	[[nodiscard]] constexpr Matrix3<T> transpose() const
	{
		Matrix3<T> t_out;
		t_out.set(0, 0, _M[0][0]);
		t_out.set(1, 0, _M[0][1]);
		t_out.set(2, 0, _M[0][2]);
		t_out.set(0, 1, _M[1][0]);
		t_out.set(1, 1, _M[1][1]);
		t_out.set(2, 1, _M[1][2]);
		t_out.set(0, 2, _M[2][0]);
		t_out.set(1, 2, _M[2][1]);
		t_out.set(2, 2, _M[2][2]);
		return t_out;
	}

	[[nodiscard]] constexpr this_t operator - () const
	{
		this_t t_out;
		t_out[0][0] = -_M[0][0];
		t_out[0][1] = -_M[0][1];
		t_out[0][2] = -_M[0][2];
		t_out[1][0] = -_M[1][0];
		t_out[1][1] = -_M[1][1];
		t_out[1][2] = -_M[1][2];
		t_out[2][0] = -_M[2][0];
		t_out[2][1] = -_M[2][1];
		t_out[2][2] = -_M[2][2];
		return t_out;
	}

	//TODO:
	//	Determinant - requires a conditional enable, the ability to create a sub matrix and recursion, but we don't need it for now
};

template <_p::is_arithmetic T1, _p::is_non_const_arithmetic T2>
[[nodiscard]] Matrix3<T2> operator * (const T1 p_1, const Matrix3<T2>& p_2)
{
	return p_2 * p_1;
}

} //namespace mathlib

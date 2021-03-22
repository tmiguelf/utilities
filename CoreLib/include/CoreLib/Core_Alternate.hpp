//======== ======== ======== ======== ======== ======== ======== ========
///	\file
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
#include <utility>

namespace core
{

template <typename T_primary, typename T_alternate, T_alternate Good_v, T_alternate Default_v>
class alternate
{
	static_assert(!std::is_same_v<std::remove_cv_t<T_primary>, std::remove_cv_t<T_alternate>>, "Types must be distinct");
	//static_assert(std::is_integral_v<T_alternate> || std::is_enum_v<T_alternate>, "Alternate Type must be an error core");
	static_assert(Good_v != Default_v, "Good_v and Default_v must differ");
public:
	inline constexpr alternate()						: m_errorCode{Default_v} {}
	inline constexpr alternate(T_alternate p_code)		: m_errorCode{p_code} {}
	inline constexpr alternate(const T_primary& p_val)	: m_value{p_val}, m_errorCode{Good_v} {}
	inline constexpr alternate(T_primary&& p_val)		: m_value{std::move(p_val)}, m_errorCode{Good_v} {}

	inline constexpr	alternate(const alternate&) = default;
	inline				alternate(alternate&&) = default;
	inline constexpr	alternate& operator = (const alternate&) = default;
	inline				alternate& operator = (alternate&&) = default;

	[[nodiscard]] inline constexpr bool				has_value	() const { return m_errorCode == Good_v; }
	[[nodiscard]] inline constexpr const T_primary&	value		() const { return m_value; }
	[[nodiscard]] inline T_primary&					value		() { return m_value; }
	[[nodiscard]] inline constexpr T_primary		value_or	(T_primary p_alt) const { if(has_value()) return m_value; return p_alt; }
	[[nodiscard]] inline constexpr T_alternate		error_code	() const { return m_errorCode; }

private:
	T_primary   m_value{};
	T_alternate m_errorCode{Default_v};
};

} //namespace core

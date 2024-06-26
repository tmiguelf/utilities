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
	inline constexpr alternate(T_alternate const p_code): m_errorCode{p_code} {}
	inline constexpr alternate(T_primary const& p_val)	: m_value{p_val}, m_errorCode{Good_v} {}
	inline constexpr alternate(T_primary&& p_val)		: m_value{std::move(p_val)}, m_errorCode{Good_v} {}

	inline constexpr	alternate(alternate const&) = default;
	inline				alternate(alternate&&) = default;
	inline constexpr	alternate& operator = (alternate const&) = default;
	inline				alternate& operator = (alternate&&) = default;

	[[nodiscard]] inline constexpr bool				has_value	() const { return m_errorCode == Good_v; }
	[[nodiscard]] inline constexpr T_primary const&	value		() const { return m_value; }
	[[nodiscard]] inline T_primary&					value		() { return m_value; }
	[[nodiscard]] inline constexpr T_primary		value_or	(T_primary const p_alt) const { if(has_value()) return m_value; return p_alt; }
	[[nodiscard]] inline constexpr T_alternate		error_code	() const { return m_errorCode; }

private:
	T_primary   m_value{};
	T_alternate m_errorCode{Default_v};
};

} //namespace core

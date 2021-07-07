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
///
///	\todo	Provide a comprehensive and consistent set of error codes, to give
///			extra information regarding he nature of the failure
//======== ======== ======== ======== ======== ======== ======== ========

#pragma once

#include "toPrint_base.hpp"
#include <array>

#include <CoreLib/net/core_net_address.hpp>
#include <CoreLib/string/core_string_numeric.hpp>
namespace core
{

template<>
class toPrint<core::IPv4_address>: public toPrint_base
{
private:
	static constexpr uintptr_t max_ip_size = 15;
	using array_t = std::array<char8_t, max_ip_size>;
public:
	toPrint(const core::IPv4_address& p_data)
	{
		m_size = p_data.to_string(m_preCalc);
	}

	inline constexpr uintptr_t size(const char8_t&) const { return m_size; }

	void getPrint(char8_t* p_out) const
	{
		memcpy(p_out, m_preCalc.data(), m_size);
	}

private:
	array_t m_preCalc;
	uintptr_t m_size;
};

template<>
class toPrint<core::IPv6_address>: public toPrint_base
{
private:
	static constexpr uintptr_t max_ip_size = 39;
	using array_t = std::array<char8_t, max_ip_size>;
public:
	toPrint(const core::IPv6_address& p_data)
	{
		m_size = p_data.to_string(m_preCalc);
	}

	inline constexpr uintptr_t size(const char8_t&) const { return m_size; }

	void getPrint(char8_t* p_out) const
	{
		memcpy(p_out, m_preCalc.data(), m_size);
	}

private:
	array_t m_preCalc;
	uintptr_t m_size;
};

template<>
class toPrint<core::IP_address>: public toPrint_base
{
private:
	static constexpr uintptr_t max_ip_size = 39;
	using array_t = std::array<char8_t, max_ip_size>;
public:
	toPrint(const core::IP_address& p_data)
	{
		m_size = p_data.to_string(m_preCalc);
	}

	inline constexpr uintptr_t size(const char8_t&) const { return m_size; }

	void getPrint(char8_t* p_out) const
	{
		memcpy(p_out, m_preCalc.data(), m_size);
	}

private:
	array_t m_preCalc;
	uintptr_t m_size;
};


template<typename T>
class toPrint_net;
template <typename T> toPrint_net(T, uint16_t) -> toPrint_net<std::remove_cvref_t<T>>;

template<>
class toPrint_net<core::IPv4_address>: public toPrint_base
{
private:
	static constexpr uintptr_t max_ip_size = 15;
	static constexpr uintptr_t max_port_size = core::to_chars_dec_max_digits_v<uint16_t>;
	using array_t = std::array<char8_t, max_ip_size + max_port_size + 1>;
public:
	toPrint_net(const core::IPv4_address& p_ip, uint16_t p_port)
	{
		const uintptr_t size1 = p_ip.to_string(std::span<char8_t, max_ip_size>{m_preCalc.data(), max_ip_size});
		m_preCalc[size1] = u8'@';
		m_size = size1 + 1 + core::to_chars(p_port, std::span<char8_t, max_port_size>{m_preCalc.data() + size1 + 1, max_port_size});
	}

	inline constexpr uintptr_t size(const char8_t&) const { return m_size; }

	void getPrint(char8_t* p_out) const
	{
		memcpy(p_out, m_preCalc.data(), m_size);
	}

private:
	array_t m_preCalc;
	uintptr_t m_size;
};

template<>
class toPrint_net<core::IPv6_address>: public toPrint_base
{
private:
	static constexpr uintptr_t max_ip_size = 39;
	static constexpr uintptr_t max_port_size = core::to_chars_dec_max_digits_v<uint16_t>;
	using array_t = std::array<char8_t, max_ip_size + max_port_size + 1>;
public:
	toPrint_net(const core::IPv6_address& p_ip, uint16_t p_port)
	{
		const uintptr_t size1 = p_ip.to_string(std::span<char8_t, max_ip_size>{m_preCalc.data(), max_ip_size});
		m_preCalc[size1] = u8'@';
		m_size = size1 + 1 + core::to_chars(p_port, std::span<char8_t, max_port_size>{m_preCalc.data() + size1 + 1, max_port_size});
	}

	inline constexpr uintptr_t size(const char8_t&) const { return m_size; }

	void getPrint(char8_t* p_out) const
	{
		memcpy(p_out, m_preCalc.data(), m_size);
	}

private:
	array_t m_preCalc;
	uintptr_t m_size;
};

template<>
class toPrint_net<core::IP_address>: public toPrint_base
{
private:
	static constexpr uintptr_t max_ip_size = 39;
	static constexpr uintptr_t max_port_size = core::to_chars_dec_max_digits_v<uint16_t>;
	using array_t = std::array<char8_t, max_ip_size + max_port_size + 1>;
public:
	toPrint_net(const core::IP_address& p_ip, uint16_t p_port)
	{
		const uintptr_t size1 = p_ip.to_string(std::span<char8_t, max_ip_size>{m_preCalc.data(), max_ip_size});
		m_preCalc[size1] = u8'@';
		m_size = size1 + 1 + core::to_chars(p_port, std::span<char8_t, max_port_size>{m_preCalc.data() + size1 + 1, max_port_size});
	}

	inline constexpr uintptr_t size(const char8_t&) const { return m_size; }

	void getPrint(char8_t* p_out) const
	{
		memcpy(p_out, m_preCalc.data(), m_size);
	}

private:
	array_t m_preCalc;
	uintptr_t m_size;
};

} //namespace core

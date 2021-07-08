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
	toPrint(const core::IPv4_address& p_data): m_data(p_data) {}

	inline uintptr_t size() const { return _p::to_chars_IPv4_estimate(m_data.byteField); }
	inline uintptr_t size(const char8_t &) const { return size(); }
	inline uintptr_t size(const char16_t&) const { return size(); }
	inline uintptr_t size(const char32_t&) const { return size(); }

	void getPrint(char8_t * p_out) const { _p::to_chars_IPv4_unsafe(m_data.byteField, p_out); }
	void getPrint(char16_t* p_out) const { _p::to_chars_IPv4_unsafe(m_data.byteField, p_out); }
	void getPrint(char32_t* p_out) const { _p::to_chars_IPv4_unsafe(m_data.byteField, p_out); }

private:
	const core::IPv4_address m_data;
};

template<>
class toPrint<core::IPv6_address>: public toPrint_base
{
private:
	static constexpr uintptr_t max_ip_size = 39;
	using array_t = std::array<char8_t, max_ip_size>;
public:
	toPrint(const core::IPv6_address& p_data): m_data(p_data) {}

	inline uintptr_t size() const { return _p::to_chars_IPv6_estimate(m_data.doubletField); }
	inline uintptr_t size(const char8_t &) const { return size(); }
	inline uintptr_t size(const char16_t&) const { return size(); }
	inline uintptr_t size(const char32_t&) const { return size(); }

	void getPrint(char8_t * p_out) const { _p::to_chars_IPv6_unsafe(m_data.doubletField, p_out); }
	void getPrint(char16_t* p_out) const { _p::to_chars_IPv6_unsafe(m_data.doubletField, p_out); }
	void getPrint(char32_t* p_out) const { _p::to_chars_IPv6_unsafe(m_data.doubletField, p_out); }

private:
	const core::IPv6_address m_data;
};

template<>
class toPrint<core::IP_address>: public toPrint_base
{
private:
	static constexpr uintptr_t max_ip_size = 39;
	using array_t = std::array<char8_t, max_ip_size>;
public:
	toPrint(const core::IP_address& p_data): m_data(p_data) {}

	uintptr_t size() const
	{
		if(m_data.m_ipv == core::IP_address::IPv::IPv_4)
		{
			return _p::to_chars_IPv4_estimate(m_data.v4.byteField);
		}
		else if(m_data.m_ipv == core::IP_address::IPv::IPv_6)
		{
			return _p::to_chars_IPv6_estimate(m_data.v6.doubletField);
		}
		return 0;
	}

	inline uintptr_t size(const char8_t &) const { return size(); }
	inline uintptr_t size(const char16_t&) const { return size(); }
	inline uintptr_t size(const char32_t&) const { return size(); }

	void getPrint(char8_t* p_out) const
	{
		if(m_data.m_ipv == core::IP_address::IPv::IPv_4)
		{
			_p::to_chars_IPv4_unsafe(m_data.v4.byteField, p_out);
		}
		else if(m_data.m_ipv == core::IP_address::IPv::IPv_6)
		{
			_p::to_chars_IPv6_unsafe(m_data.v6.doubletField, p_out);
		}
	}

	void getPrint(char16_t* p_out) const
	{
		if(m_data.m_ipv == core::IP_address::IPv::IPv_4)
		{
			_p::to_chars_IPv4_unsafe(m_data.v4.byteField, p_out);
		}
		else if(m_data.m_ipv == core::IP_address::IPv::IPv_6)
		{
			_p::to_chars_IPv6_unsafe(m_data.v6.doubletField, p_out);
		}
	}

	void getPrint(char32_t* p_out) const
	{
		if(m_data.m_ipv == core::IP_address::IPv::IPv_4)
		{
			_p::to_chars_IPv4_unsafe(m_data.v4.byteField, p_out);
		}
		else if(m_data.m_ipv == core::IP_address::IPv::IPv_6)
		{
			_p::to_chars_IPv6_unsafe(m_data.v6.doubletField, p_out);
		}
	}

private:
	const core::IP_address& m_data;
};


template<typename T>
class toPrint_net;
template <typename T> toPrint_net(T, uint16_t) -> toPrint_net<std::remove_cvref_t<T>>;

template<>
class toPrint_net<core::IPv4_address>: public toPrint_base
{
public:
	toPrint_net(const core::IPv4_address& p_ip, uint16_t p_port)
		: m_ip  (p_ip)
		, m_port(p_port)
	{}

	inline uintptr_t size() const
	{
		return
			_p::to_chars_IPv4_estimate(m_ip.byteField) +
			_p::to_chars_estimate(m_port) + 1;
	}

	inline uintptr_t size(const char8_t &) const { return size(); }
	inline uintptr_t size(const char16_t&) const { return size(); }
	inline uintptr_t size(const char32_t&) const { return size(); }

	void getPrint(char8_t* p_out) const
	{
		p_out += _p::to_chars_IPv4(m_ip.byteField, std::span<char8_t, 15>{p_out, 15});
		*(p_out++) = u8'@';
		_p::to_chars_unsafe(m_port, p_out);
	}

	void getPrint(char16_t* p_out) const
	{
		p_out += _p::to_chars_IPv4(m_ip.byteField, std::span<char16_t, 15>{p_out, 15});
		*(p_out++) = u'@';
		_p::to_chars_unsafe(m_port, p_out);
	}

	void getPrint(char32_t* p_out) const
	{
		p_out += _p::to_chars_IPv4(m_ip.byteField, std::span<char32_t, 15>{p_out, 15});
		*(p_out++) = U'@';
		_p::to_chars_unsafe(m_port, p_out);
	}

private:
	const core::IPv4_address m_ip;
	const uint16_t m_port;
};

template<>
class toPrint_net<core::IPv6_address>: public toPrint_base
{
public:
	toPrint_net(const core::IPv6_address& p_ip, uint16_t p_port)
		: m_ip  (p_ip)
		, m_port(p_port)
	{}

	inline uintptr_t size() const
	{
		return
			_p::to_chars_IPv6_estimate(m_ip.doubletField) +
			_p::to_chars_estimate(m_port) + 1;
	}

	inline uintptr_t size(const char8_t &) const { return size(); }
	inline uintptr_t size(const char16_t&) const { return size(); }
	inline uintptr_t size(const char32_t&) const { return size(); }


	void getPrint(char8_t* p_out) const
	{
		p_out += _p::to_chars_IPv6(m_ip.doubletField, std::span<char8_t, 39>{p_out, 39});
		*(p_out++) = u8'@';
		_p::to_chars_unsafe(m_port, p_out);
	}

	void getPrint(char16_t* p_out) const
	{
		p_out += _p::to_chars_IPv6(m_ip.doubletField, std::span<char16_t, 39>{p_out, 39});
		*(p_out++) = u'@';
		_p::to_chars_unsafe(m_port, p_out);
	}

	void getPrint(char32_t* p_out) const
	{
		p_out += _p::to_chars_IPv6(m_ip.doubletField, std::span<char32_t, 39>{p_out, 39});
		*(p_out++) = U'@';
		_p::to_chars_unsafe(m_port, p_out);
	}

private:
	const core::IPv6_address m_ip;
	const uint16_t m_port;
};

template<>
class toPrint_net<core::IP_address>: public toPrint_base
{
private:
	uintptr_t ip_size() const
	{
		if(m_ip.m_ipv == core::IP_address::IPv::IPv_4)
		{
			return _p::to_chars_IPv4_estimate(m_ip.v4.byteField);
		}
		else if(m_ip.m_ipv == core::IP_address::IPv::IPv_6)
		{
			return _p::to_chars_IPv6_estimate(m_ip.v6.doubletField);
		}
		return 0;
	}

public:
	toPrint_net(const core::IP_address& p_ip, uint16_t p_port)
		: m_ip  (p_ip)
		, m_port(p_port)
	{}

	inline uintptr_t size() const
	{
		return ip_size() + _p::to_chars_estimate(m_port) + 1;
	}

	inline uintptr_t size(const char8_t &) const { return size(); }
	inline uintptr_t size(const char16_t&) const { return size(); }
	inline uintptr_t size(const char32_t&) const { return size(); }

	void getPrint(char8_t* p_out) const
	{
		if(m_ip.m_ipv == core::IP_address::IPv::IPv_4)
		{
			p_out += _p::to_chars_IPv4(m_ip.v4.byteField, std::span<char8_t, 15>{p_out, 15});
		}
		else if(m_ip.m_ipv == core::IP_address::IPv::IPv_6)
		{
			p_out += _p::to_chars_IPv6(m_ip.v6.doubletField, std::span<char8_t, 39>{p_out, 39});
		}
		*(p_out++) = u8'@';
		_p::to_chars_unsafe(m_port, p_out);
	}

	void getPrint(char16_t* p_out) const
	{
		if(m_ip.m_ipv == core::IP_address::IPv::IPv_4)
		{
			p_out += _p::to_chars_IPv4(m_ip.v4.byteField, std::span<char16_t, 15>{p_out, 15});
		}
		else if(m_ip.m_ipv == core::IP_address::IPv::IPv_6)
		{
			p_out += _p::to_chars_IPv6(m_ip.v6.doubletField, std::span<char16_t, 39>{p_out, 39});
		}
		*(p_out++) = u'@';
		_p::to_chars_unsafe(m_port, p_out);
	}

	void getPrint(char32_t* p_out) const
	{
		if(m_ip.m_ipv == core::IP_address::IPv::IPv_4)
		{
			p_out += _p::to_chars_IPv4(m_ip.v4.byteField, std::span<char32_t, 15>{p_out, 15});
		}
		else if(m_ip.m_ipv == core::IP_address::IPv::IPv_6)
		{
			p_out += _p::to_chars_IPv6(m_ip.v6.doubletField, std::span<char32_t, 39>{p_out, 39});
		}
		*(p_out++) = U'@';
		_p::to_chars_unsafe(m_port, p_out);
	}

private:
	const core::IP_address& m_ip;
	const uint16_t m_port;
};

} //namespace core

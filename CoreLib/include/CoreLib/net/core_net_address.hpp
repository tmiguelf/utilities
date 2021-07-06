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

#include <cstdint>
#include <string>
#include <string_view>
#include <span>
#include <utility>

/// \n
namespace core
{
/// \brief provides a wrapper for a IP v4 and v6 addresses
struct IP_address
{
	union
	{
		union
		{
			uint32_t	ui32Type;
			uint8_t		byteField[4];
		} v4;	//!< Data of the object if IPv4 is used

		union
		{
			uint64_t	ui64Type	[2];
			uint16_t	wordField	[8];
			uint8_t		byteField	[16];
		} v6;	//!< Data of the object if IPv6 is used
	};

	/// \brief Indetifies the IP versionof this object
	enum class IPv: uint8_t
	{
		None	= 0,	//!< IP version is not set
		IPv_4	= 4,	//!< IP version 4
		IPv_6	= 6		//!< IP version 6
	} m_ipv = IPv::None;

	IP_address();
	IP_address(IPv p_version, std::span<const uint8_t> p_init);
	IP_address(std::u8string_view p_address);

	IP_address(const IP_address& p_other)	= default;
	IP_address(IP_address&& p_other)		= default;

	///	\brief Tries to initialize an IPv4 address from string
	///	\param[in] p_address - String in dot-decimal
	///	\return true if string contains valid IPv4 in dot-decimal
	bool from_string_v4(std::u8string_view p_address);

	///	\brief Tries to initialize an IPv6 address from string
	///	\param[in] p_address - String in RFC 5952
	///	\return true if string contains valid IPv6 in RFC5952
	bool from_string_v6(std::u8string_view p_address);

	///	\brief Tries to initialize the address from string
	///	\param[in] p_address - String in dot-decimal or RFC 5952
	///	\return true if string contains valid IPv4 in dot-decimal or IPv6 in RFC5952
	bool from_string(std::u8string_view p_address);

	///	\brief Outputs current IP into a string
	///	\param[out] p_address - Non-null-terminated string in dot-decimal for IPv4 address, in RFC5952 for IPv6, or empty if neither. Buffer size must be at least 39 Bytes
	///	\return Number of output bytes used
	uintptr_t to_string(std::span<char8_t, 39> p_output) const;

	///	\brief Inline version of IPv6_netAddr::to_string(std::span<char8_t, 39>)
	std::u8string to_string() const;

	///	\brief Sets address to IPv4 any address (0.0.0.0)
	void set_any_v4();

	///	\brief Sets address to IPv6 any address (::0)
	void set_any_v6();

	///	\brief Sets address to IPv4 loopback address (127.0.0.1)
	void set_loopback_v4();

	///	\brief Sets address to IPv6 loopback address (::1)
	void set_loopback_v6();

	///	\brief Swaps this IP with another
	void swap(IP_address& p_other);

	///	\brief  Checks if IP is set to 0
	bool is_null() const;

	///	\brief  Checks if  object is set to either IPv4 or IPv6 address
	bool is_valid() const;

	IP_address&	operator =	(const IP_address&) = default;

	IP_address&	operator |=	(const IP_address& p_other);
	IP_address&	operator &=	(const IP_address& p_other);
	IP_address&	operator ^=	(const IP_address& p_other);
	IP_address	operator |	(const IP_address& p_other) const;
	IP_address	operator &	(const IP_address& p_other) const;
	IP_address	operator ^	(const IP_address& p_other) const;
	IP_address	operator ~	() const;
	bool		operator ==	(const IP_address& p_other) const;
	bool		operator !=	(const IP_address& p_other) const;
	bool		operator <	(const IP_address& p_other) const;

	///	\brief The IP version of this address
	/// \return \ref core::IP_netAddr::IPv
	IPv version() const;

	///	\brief Resets the object to no IP
	void clear();
};


/// \brief provides a wrapper for a IPv4 address
struct IPv4_address
{
	union
	{
		uint32_t	ui32Type;
		uint8_t		byteField[4];
	};

	IPv4_address();
	IPv4_address(uint32_t						p_init);
	IPv4_address(std::span<const uint8_t, 4>	p_init);
	IPv4_address(const IPv4_address&			p_other);
	IPv4_address(std::u8string_view				p_address);

	///	\brief Tries to initialize address from string
	///	\param[in] p_address - String in dot-decimal notation
	///	\return true if string contains valid IP in dot-decimal notation
	bool from_string(std::u8string_view	p_address);

	///	\brief Outputs current IP into a string
	///	\param[out] p_address - Non-null-terminated string in dot-decimal notation. Buffer size must be at least 15 Bytes
	///	\return Number of output bytes used
	uintptr_t to_string(std::span<char8_t, 15> p_output) const;

	///	\brief Inline version of IPv4_netAddr::to_string(std::span<char8_t, 15>)
	std::u8string to_string() const;

	///	\brief Sets IP to "Any address" (i.e. 0.0.0.0)
	void set_any();

	///	\brief Swaps this IP with another
	void swap(IPv4_address& p_other);

	///	\brief  Checks if IP is set to 0.0.0.0
	bool is_null() const;

	IPv4_address&	operator =	(const IPv4_address& p_other);
	IPv4_address&	operator |=	(const IPv4_address& p_other);
	IPv4_address&	operator &=	(const IPv4_address& p_other);
	IPv4_address&	operator ^=	(const IPv4_address& p_other);
	IPv4_address	operator |	(const IPv4_address& p_other) const;
	IPv4_address	operator &	(const IPv4_address& p_other) const;
	IPv4_address	operator ^	(const IPv4_address& p_other) const;
	IPv4_address	operator ~	() const;
	bool			operator ==	(const IPv4_address& p_other) const;
	bool			operator !=	(const IPv4_address& p_other) const;
	bool			operator <	(const IPv4_address& p_other) const;
};


/// \brief provides a wrapper for a IPv6 address
struct IPv6_address
{
	union
	{
		uint64_t	ui64Type	[2];
		uint16_t	wordField	[8];
		uint8_t		byteField	[16];
	};

	IPv6_address();
	IPv6_address(std::span<const uint8_t, 16>	p_init);
	IPv6_address(const IPv6_address&	p_other);
	IPv6_address(std::u8string_view		p_address);

	///	\brief Tries to initialize the address from string
	///	\param[in] p_address - String in RFC 5952 standard
	///	\return true if string contains valid IP in RFC5952 standard
	bool from_string(std::u8string_view	p_address);

	///	\brief Outputs current IP into a string
	///	\param[out] p_address - Non-null-terminated string in RFC5952 standard notation. Buffer size must be at least 39 Bytes
	///	\return Number of output bytes used
	uintptr_t to_string(std::span<char8_t, 39> p_output) const;

	///	\brief Inline version of IPv6_netAddr::to_string(std::span<char8_t, 39>)
	std::u8string to_string() const;

	///	\brief Sets IP to "Any address" (i.e. ::0)
	void set_any();

	///	\brief Swaps this IP with another
	void swap(IPv6_address& p_other);

	///	\brief Checks if IP is set to ::0
	bool is_null() const;

	IPv6_address&	operator =	(const IPv6_address& p_other);
	IPv6_address&	operator |=	(const IPv6_address& p_other);
	IPv6_address&	operator &=	(const IPv6_address& p_other);
	IPv6_address&	operator ^=	(const IPv6_address& p_other);
	IPv6_address	operator |	(const IPv6_address& p_other) const;
	IPv6_address	operator &	(const IPv6_address& p_other) const;
	IPv6_address	operator ^	(const IPv6_address& p_other) const;
	IPv6_address	operator ~	() const;
	bool			operator ==	(const IPv6_address& p_other) const;
	bool			operator !=	(const IPv6_address& p_other) const;
	bool			operator <	(const IPv6_address& p_other) const;
};






//======== ======== ======== inline optimization ======== ======== ========


//======== ======== IP_address ======== ========
inline IP_address::IPv IP_address::version() const { return m_ipv; }
inline void IP_address::clear() { m_ipv = IPv::None; }
inline void IP_address::swap(IP_address& p_other) { std::swap(*this, p_other); }




//======== ======== IPv4_address ======== ========
inline IPv4_address::IPv4_address()								: ui32Type(0) {}
inline IPv4_address::IPv4_address(uint32_t p_init)				: ui32Type(p_init){}
inline IPv4_address::IPv4_address(std::span<const uint8_t, 4> p_init) { memcpy(byteField, p_init.data(), 4); }
inline IPv4_address::IPv4_address(const IPv4_address& p_other)	: ui32Type(p_other.ui32Type) { }


inline void IPv4_address::set_any() { ui32Type = 0; }
inline void IPv4_address::swap(IPv4_address& p_other) { std::swap(ui32Type, p_other.ui32Type); }
inline bool IPv4_address::is_null() const {return ui32Type == 0;}

inline IPv4_address& IPv4_address::operator  = (const IPv4_address& p_other) { ui32Type = p_other.ui32Type; return *this; }
inline IPv4_address& IPv4_address::operator |= (const IPv4_address& p_other) { ui32Type |= p_other.ui32Type; return *this; }
inline IPv4_address& IPv4_address::operator &= (const IPv4_address& p_other) { ui32Type &= p_other.ui32Type; return *this; }
inline IPv4_address& IPv4_address::operator ^= (const IPv4_address& p_other) { ui32Type ^= p_other.ui32Type; return *this; }
inline IPv4_address  IPv4_address::operator |  (const IPv4_address& p_other) const { return IPv4_address(ui32Type | p_other.ui32Type); }
inline IPv4_address  IPv4_address::operator &  (const IPv4_address& p_other) const { return IPv4_address(ui32Type & p_other.ui32Type); }
inline IPv4_address  IPv4_address::operator ^  (const IPv4_address& p_other) const { return IPv4_address(ui32Type ^ p_other.ui32Type); }
inline IPv4_address  IPv4_address::operator ~  () const { return IPv4_address(~ui32Type); }

inline bool IPv4_address::operator == (const IPv4_address& p_other) const { return ui32Type == p_other.ui32Type; }
inline bool IPv4_address::operator != (const IPv4_address& p_other) const { return ui32Type != p_other.ui32Type; }
inline bool IPv4_address::operator <  (const IPv4_address& p_other) const { return ui32Type < p_other.ui32Type; }

//======== ======== IPv6_address ======== ========
inline bool IPv6_address::is_null() const {return ui64Type[0] == 0 && ui64Type[1] == 0;}











} //namespace core

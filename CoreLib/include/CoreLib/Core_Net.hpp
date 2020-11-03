//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///		Provides network interfaces
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
///
///	\todo	Provide a comprehensive and consistent set of error codes, to give
///			extra information regarding he nature of the failure
///
///	\todo Socket RAW
///	\todo Multi-cast support
///	\todo Native socket options
///	\todo Auto memory management assist
///	\todo Split of IPv4 and IPv6 protocols
//======== ======== ======== ======== ======== ======== ======== ========

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <span>
#include <cstring>
#include <utility>

#include <CoreLib/Core_Type.hpp>

/// \n
namespace core
{

	/// \brief Network error codes
	//
	enum class NET_Error: uint8_t
	{
		NoErr					= 0x00,	//!< The operation completed as intended
		Already_Used			= 0x01,	//!< The operation would require the creation of a new socket, but the object already contains one in use. Please close that socket first before re-using the object.
		Invalid_Option			= 0x02,	//!< One ore more parameters are invalid for this operation
		Invalid_IP				= 0x03,	//!< The specified IP is invalid for this operation
		Invalid_Socket			= 0x04,	//!< The socket is invalid or does not exist
		Sock_Option				= 0x05,	//!< Failed to set a scoket option
		Sock_Bind				= 0x06,	//!< Failed to bind the socket to the requested address
		Sock_Listen				= 0x07,	//!< A problem ocured while setting the socket to listen mode
		Sock_Bad_Close			= 0x08,	//!< A problem ocured while closing a socket
		
		Buffer_Full				= 0x0F,	//!< For external use

		Incompatible_Protocol	= 0x11,	//!< You are using an IPv agnostic socket but you are trying to perform a version specific operation that differs from the version the socket has been initialized for

		TCP_GracefullClose		= 0xF0,	//!< Indicates that the TCP peer has executed a gracefull close
		Fail					= 0xFC,	//!< The attempted operations failed
		Unknown					= 0xFD,	//!< A problem of unknown nature ocured
		Connection				= 0xFE,	//!< A connection problem was detected
		WouldBlock				= 0xFF	//!< You are trying to perform an operation on a non-blocking socket that could not be completed at this time without blocking
	};

#ifdef _WIN32
	typedef	uintptr_t	SocketHandle_t;
#else
	typedef	int			SocketHandle_t;
#endif

	/// \brief provides a wrapper for a IPv4 address
	//
	struct IPv4_netAddr
	{
		union
		{
			uint32_t	ui32Type;
			uint8_t		byteField[4];
		};

		IPv4_netAddr();
		IPv4_netAddr(uint32_t						p_init);
		IPv4_netAddr(std::span<const uint8_t, 4>	p_init);
		IPv4_netAddr(const IPv4_netAddr&			p_other);
		IPv4_netAddr(std::u8string_view				p_address);

		///	\brief Tries to initialize address from string
		///	\param[in] p_address - String in dot-decimal notation
		///	\return true if string contains valid IP in dot-decimal notation
		//
		bool fromString		(std::u8string_view	p_address);

		///	\brief Outputs current IP into a string
		///	\param[out] p_address - Non-null-terminated string in dot-decimal notation. Buffer size must be at least 15 Bytes
		///	\return Number of output bytes used
		//
		uintptr_t toString	(std::span<char8_t, 15> p_output) const;

		///	\brief Inline version of IPv4_netAddr::toString(const std::string&)
		//
		std::u8string toString() const;

		///	\brief Sets IP to "Any address" (i.e. 0.0.0.0)
		//
		void setAny			();

		///	\brief Swaps this IP with another
		//
		void swap			(IPv4_netAddr& p_other);

		///	\brief  Checks if IP is set to 0.0.0.0
		//
		bool is_null		() const;

		IPv4_netAddr&	operator =	(const IPv4_netAddr& p_other);
		IPv4_netAddr&	operator |=	(const IPv4_netAddr& p_other);
		IPv4_netAddr&	operator &=	(const IPv4_netAddr& p_other);
		IPv4_netAddr&	operator ^=	(const IPv4_netAddr& p_other);
		IPv4_netAddr	operator |	(const IPv4_netAddr& p_other) const;
		IPv4_netAddr	operator &	(const IPv4_netAddr& p_other) const;
		IPv4_netAddr	operator ^	(const IPv4_netAddr& p_other) const;
		IPv4_netAddr	operator ~	() const;
		bool			operator ==	(const IPv4_netAddr& p_other) const;
		bool			operator !=	(const IPv4_netAddr& p_other) const;
		bool			operator <	(const IPv4_netAddr& p_other) const;
	};

	/// \brief provides a wrapper for a IPv6 address
	//
	struct IPv6_netAddr
	{
		union
		{
			uint64_t	ui64Type	[2];
			uint16_t	wordField	[8];
			uint8_t		byteField	[16];
		};

		IPv6_netAddr();
		IPv6_netAddr(std::span<const uint8_t, 16>	p_init);
		IPv6_netAddr(const IPv6_netAddr&	p_other);
		IPv6_netAddr(std::u8string_view		p_address);

		///	\brief Tries to initialize the address from string
		///	\param[in] p_address - String in RFC 5952 standard
		///	\return true if string contains valid IP in RFC5952 standard
		//
		bool fromString	(std::u8string_view	p_address);


		///	\brief Outputs current IP into a string
		///	\param[out] p_address - Non-null-terminated string in RFC5952 standard notation. Buffer size must be at least 39 Bytes
		///	\return Number of output bytes used
		//
		uintptr_t toString	(std::span<char8_t, 39> p_output) const;

		///	\brief Inline version of IPv6_netAddr::toString(const std::string&)
		//
		std::u8string toString	() const;

		///	\brief Sets IP to "Any address" (i.e. ::0)
		//
		void setAny		();

		///	\brief Swaps this IP with another
		//
		void swap(IPv6_netAddr& p_other);

		///	\brief Checks if IP is set to ::0
		//
		bool is_null() const;

		IPv6_netAddr&	operator =	(const IPv6_netAddr& p_other);
		IPv6_netAddr&	operator |=	(const IPv6_netAddr& p_other);
		IPv6_netAddr&	operator &=	(const IPv6_netAddr& p_other);
		IPv6_netAddr&	operator ^=	(const IPv6_netAddr& p_other);
		IPv6_netAddr	operator |	(const IPv6_netAddr& p_other) const;
		IPv6_netAddr	operator &	(const IPv6_netAddr& p_other) const;
		IPv6_netAddr	operator ^	(const IPv6_netAddr& p_other) const;
		IPv6_netAddr	operator ~	() const;
		bool			operator ==	(const IPv6_netAddr& p_other) const;
		bool			operator !=	(const IPv6_netAddr& p_other) const;
		bool			operator <	(const IPv6_netAddr& p_other) const;
	};


	/// \brief provides a wrapper for a IP v4 and v6 addresses
	//
	struct IP_netAddr
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
		//
		enum class IPv: uint8_t
		{
			None	= 0,	//!< IP version is not set
			IPv_4	= 4,	//!< IP version 4
			IPv_6	= 6		//!< IP version 6
		} m_ipv;

		IP_netAddr();
		IP_netAddr(IPv p_version, std::span<const uint8_t> p_init);
		IP_netAddr(std::u8string_view p_address);

		IP_netAddr(const IP_netAddr& p_other)	= default;
		IP_netAddr(IP_netAddr&& p_other)		= default;

		///	\brief Tries to initialize an IPv4 address from string
		///	\param[in] p_address - String in dot-decimal
		///	\return true if string contains valid IPv4 in dot-decimal
		//
		bool fromStringV4	(std::u8string_view p_address);

		///	\brief Tries to initialize an IPv6 address from string
		///	\param[in] p_address - String in RFC 5952
		///	\return true if string contains valid IPv6 in RFC5952
		//
		bool fromStringV6	(std::u8string_view p_address);

		///	\brief Tries to initialize the address from string
		///	\param[in] p_address - String in dot-decimal or RFC 5952
		///	\return true if string contains valid IPv4 in dot-decimal or IPv6 in RFC5952
		//
		bool fromString		(std::u8string_view p_address);

		///	\brief Outputs current IP into a string
		///	\param[out] p_address - Non-null-terminated string in dot-decimal for IPv4 address, in RFC5952 for IPv6, or empty if neither. Buffer size must be at least 39 Bytes
		///	\return Number of output bytes used
		//
		uintptr_t	toString	(std::span<char8_t, 39> p_output) const;

		///	\brief Inline version of IPv6_netAddr::toString(const std::string&)
		//
		std::u8string	toString	() const;

		///	\brief Sets address to IPv4 any address (0.0.0.0)
		//
		void setAny_V4		();

		///	\brief Sets address to IPv6 any address (::0)
		//
		void setAny_V6		();

		///	\brief Sets address to IPv4 loopback address (127.0.0.1)
		//
		void setLoopBack_V4	();

		///	\brief Sets address to IPv6 loopback address (::1)
		//
		void setLoopBack_V6	();

		///	\brief Swaps this IP with another
		//
		void swap(IP_netAddr& p_other);

		///	\brief  Checks if IP is set to 0
		//
		bool is_null	() const;

		///	\brief  Checks if  object is set to either IPv4 or IPv6 address
		//
		bool is_valid	() const;

		IP_netAddr&	operator =	(const IP_netAddr&) = default;

		IP_netAddr&	operator |=	(const IP_netAddr& p_other);
		IP_netAddr&	operator &=	(const IP_netAddr& p_other);
		IP_netAddr&	operator ^=	(const IP_netAddr& p_other);
		IP_netAddr	operator |	(const IP_netAddr& p_other) const;
		IP_netAddr	operator &	(const IP_netAddr& p_other) const;
		IP_netAddr	operator ^	(const IP_netAddr& p_other) const;
		IP_netAddr	operator ~	() const;
		bool		operator ==	(const IP_netAddr& p_other) const;
		bool		operator !=	(const IP_netAddr& p_other) const;
		bool		operator <	(const IP_netAddr& p_other) const;

		///	\brief The IP version of this address
		/// \return \ref core::IP_netAddr::IPv
		//
		IPv version() const;

		///	\brief Resets the object to no IP
		//
		void clear();
	};

	class NetTCP_S;
	class NetTCP_S_V4;
	class NetTCP_S_V6;

/// \n
namespace core_p
{
	///	\brief Private class to implement generic socket functionality
	//
	class Net_Socket
	{
		friend class NetTCP_S;
	public:
		
		/// \brief identifies local socket communication endpoints
		//
		enum class Endpoint: uint8_t
		{
			Receive	= 0x01,
			Send	= 0x02,
			Both	= Receive | Send
		};

	protected:
		SocketHandle_t	m_sock;

		Net_Socket();
		~Net_Socket();

		///	\brief Checks if the socket is in use
		///	\return true if socket is in use
		//
		bool isOpen() const;

		///	\brief Closes a previously open socket
		///	\return \ref core::NET_Error specifically it return \ref core::NET_Error::NET_NoErr if the socket is successfully released.
		///	\remarks
		///		It closes the socket if one was previously open.
		///		This function may fail or block depending on the blocking and linger properties set for the socket.
		///		For more details please see:
		///			https://docs.microsoft.com/en-us/windows/desktop/api/winsock/nf-winsock-closesocket
		///			https://linux.die.net/man/7/socket
		//
		NET_Error CloseSocket	();

		///	\brief Sets the blocking mode of the socket. Sockets are by default blocking.
		///	\param[in] p_blocking - If true sets the socket to blocking, if false sets the socket to non-blocking
		///	\return \ref core::NET_Error
		//
		NET_Error SetBlocking		(const bool p_blocking);

		///	\brief Marks the socket address to allow re-use. Sockets by default do not allow re-use.
		///	\param[in] p_blocking - If true sets the socket address to permitted for re-use, if false revokes this permission.
		///	\return \ref core::NET_Error
		///	\remark There is never an absolute guarantee that the address is not re-used.
		//
		NET_Error SetReuseAddress	(const bool p_reuse);

		///	\brief Sets the linger structure for the socket
		///	\return \ref core::NET_Error
		///	\remarks
		///		For more details pelase see:
		///			https://docs.microsoft.com/en-us/windows/desktop/api/winsock/nf-winsock-closesocket
		///			https://linux.die.net/man/7/socket
		//
		NET_Error SetLinger			(const bool p_linger, const uint16_t p_timeout);

		///	\brief Used to wait until data arrives on the reading end of the socket
		///	\param[in] p_microseconds - Time in microseconds to wait for data to arrive at the socket before abandoning the operation
		///	\return \ref core::NET_Error, more specifically \ref core::NET_Error::NET_NoErr on success,
		///		and \ref core::NET_Error::NET_WouldBlock if the operation was abandoned. Or other for any other errors.
		///
		///	\remarks
		///			If input time is larger than std::numeric_limits<long>::max() x 1000000,
		///			then the call will block indefinitely on blocking sockets.
		//
		NET_Error Poll(const uint64_t p_microseconds);

		///	\brief Closes the communication on a specific endpoint of the socket
		///	\param[in] p_direction - core::core_p::Net_Socket::Endpoint_t, Endpoint to close the communication on.
		///	\return \ref core::NET_Error
		//
		NET_Error shutdown(const Endpoint p_direction);

		///	\brief Swaps this socket with another
		//
		void swap(Net_Socket& p_other);

	private:
		Net_Socket(Net_Socket&& p_other)				= delete;
		Net_Socket(const Net_Socket&)					= delete;
		Net_Socket& operator = (const Net_Socket&)		= delete;
		Net_Socket& operator = (Net_Socket&& p_other)	= delete;
	};

	///	\brief Private class to implement generic UDP functionality
	//
	class NetUDP_p: protected Net_Socket
	{
	protected:
		NetUDP_p() = default;

	public:
		using Net_Socket::isOpen;
		using Net_Socket::SetBlocking;
		using Net_Socket::SetReuseAddress;
		using Net_Socket::SetLinger;
		using Net_Socket::Poll;
		using Net_Socket::shutdown;

		///	\brief Sets/Unsets the broadcast mode on the socket
		///	\param[in] p_broadcast - if true turns the broadcast mode on, if false turns it of
		///	\return \ref core::NET_Error
		//
		NET_Error SetBroadcasting	(const bool p_broadcast);

		///	\brief Reads data ready to be received on the socket
		///	\param[out] p_data - pointer to buffer that receives the data
		///	\param[in, out] p_size - on input it informs the size of the receiving buffer, on success the value is updated with the ammout of data read
		///					the absolute mmaximum amount of data that can be recieved on a single UDP packet is 65507 Bytes, however this size can be lower depending
		///					on the underlying implementation.
		///
		///	\return \ref core::NET_Error
		///
		///	\remarks
		///		If the buffer is not big enough to receive the incoming data, the data will be truncated.
		///		On blocking sockets, this call will block until data is ready to be read on the socket.
		///		On a non-blocking socket this can return \ref core::NET_Error::NET_WouldBlock if no data is ready to be read
		//
		NET_Error Receive			(uint8_t* p_data, uintptr_t& p_size);

		///	\brief Checks the size of the next data pack ready to be received on the socket
		///	\param[out] p_size - Returns the size of the packet
		///	\return \ref core::NET_Error
		///
		///	\remarks
		///		On blocking sockets, this call will block until data is ready to be read on the socket.
		///		On a non-blocking socket this can return \ref core::NET_Error::NET_WouldBlock if no data is ready to be read.
		//
		NET_Error PeekSize			(uintptr_t& p_size);
	};

	///	\brief Private class to implement generic TCP server functionality
	//
	class NetTCP_S_p: protected Net_Socket
	{
	protected:
		NetTCP_S_p() = default;

	public:
		using Net_Socket::isOpen;
		using Net_Socket::SetBlocking;
		using Net_Socket::SetLinger;
		using Net_Socket::Poll;
		using Net_Socket::shutdown;

		///	\brief Sets the socket into listening mode
		///	\param[in] p_max_connections - Number of connections allowed to be pending on the socket before starting to refuse them
		///	\return \ref core::NET_Error
		//
		NET_Error Listen(const int p_max_connections);
	};

	///	\brief Private class to implement generic TCP client functionality
	//
	class NetTCP_C_p: protected Net_Socket
	{
	protected:
		NetTCP_C_p() = default;

	public:
		using Net_Socket::isOpen;
		using Net_Socket::SetBlocking;
		using Net_Socket::SetLinger;
		using Net_Socket::Poll;
		using Net_Socket::shutdown;
		//using Net_Socket::SetReuseAddress;

		///	\brief Used to check when the connection is established on a non-blocking socket
		///	\return \ref core::NET_Error
		///
		///	\remarks
		///		A connection attempt on a non-blocking socket will not be able to complete immediately
		///		a thus this method is required in order to check when the connection is finally established (or otherwise failed).
		///		Calling this function on a blocking socket, or after there has been previously confirmed, produces undefined behaviour.
		///		You should not use this function to confirm if the connection is still valid after traffic has occured.
		//
		NET_Error NonBlock_Connect_state();

		///	\brief Sends data over the socket using a context algorithm to handle unsent data
		///	\param[in] p_buffer - p_buffer containing the data to send
		///	\param[in] p_size - amount of data in buffer
		///	\param[in,out] p_context - context tracking variable
		///	\return \ref core::NET_Error
		///
		///	\remarks
		///		On blocking sockets, this call will block until data is sent (which may be less than the amount requested).
		///		On a non-blocking socket this can return \ref core::NET_Error::NET_WouldBlock indicating
		///		that the request could not be serviced at the moment without blocking.
		///		When a data sent is requested on a TCP socket it is not guaranteed that the system
		///		is capable of processing the full amount of data, and the system can return having
		///		sent less data than what was actually requested. Thus it is necessary that users handles
		///		this conditions with specialized algorithms to make sure that the right data is sent.
		///		With a context type algorithm the user should make the first call with the variable
		///		\ref p_context with the value 0. If the function succeeds but the entire buffer could
		///		not be sent in this call, then the \ref p_context is updated with the amount of data sent
		///		so far. If after returning \ref p_context is not 0, this is an indication that there was
		///		data left unsent on the buffer.
		///		The idea is that the user should pass the same buffer, with the same size, with this
		///		updated context in order to keep sending the data until it's complete.
		///		Once all data is sent \ref p_context will be 0. If after returning, the return code is
		///		\ref core::NET_Error::NET_NoErr and \ref p_context is 0, this means that all data was
		///		captured, and that the user is free to try and send new data.
		///		For example, if a user wants to sent 500B but only 200B are able to proccessed at the moment,
		///		before the call \ref p_context starts at 0 and becomes 200 after the call. When the user calls
		///		the send function again with \ref p_context at 200, the method will skip the first 200B of the buffer
		///		and only tries to send the remaining 300B left unsent, if on this call all remaing 300B are sent, then
		///		\ref p_context becomes 0 again.
		///		If an error is returned \ref p_context is left unchanged, so if this is 0 before calling,
		///		and remains 0 after the call but an error is returned, this means that no data was sent.
		//
		NET_Error Send_context	(const uint8_t* p_buffer, const uintptr_t p_size, uintptr_t& p_context);
		
		///	\brief Sends data over the socket using a size tracking algorithm to handle unsent data
		///
		///	\param[in] p_buffer - p_buffer containing the data to send
		///	\param[in] p_size - amount of data in buffer
		///	\param[out] p_sent - amount of data actually sent
		///	\return \ref core::NET_Error
		///
		///	\remarks
		///		On blocking sockets, this call will block until data is sent (which may be less than the amount requested).
		///		On a non-blocking socket this can return \ref core::NET_Error::NET_WouldBlock indicating
		///		that the request could not be serviced at the moment without blocking.
		///		When a data sent is requested on a TCP socket it is not guaranteed that the system
		///		is capable of processing the full amount of data, and the system can return having
		///		sent less data than what was actually requested. Thus it is necessary that users handles
		///		this conditions with specialized algorithms to make sure that the right data is sent.
		///		With a size type algorithm the user is responsible for keeping track of the data that was already
		///		sent and adjust the buffer for the next send call.
		///		If an error is returned \ref p_sent is left unchanged.
		//
		NET_Error Send_size			(const uint8_t* p_buffer, const uintptr_t p_size, uintptr_t& p_sent);

		///	\brief Receives data pending on the socket using a context algorithm to handle unreceived data
		///	\param[in] p_buffer - p_buffer to receive the data
		///	\param[in] p_size - amount of data to receive
		///	\param[in,out] p_context - context tracking variable
		///	\return \ref core::NET_Error
		///
		///	\remarks
		///		On blocking sockets, this call will block until data is received (which may be less than the amount requested).
		///		On a non-blocking socket this can return \ref core::NET_Error::NET_WouldBlock indicating
		///		that the request could not be serviced at the moment without blocking.
		///		When a data receive is requested on a TCP socket it is not guaranteed that the system
		///		has the full amount pending on the socket, and can return having received less data than what was
		///		actually requested. Thus it is necessary that users handles this conditions with specialized algorithms
		///		to make sure that the right data is received.
		///		With a context type algorithm the user should make the first call with the variable
		///		\ref p_context with the value 0. If the function succeeds but the entire amount could
		///		not be received in this call, then the \ref p_context is updated with the amount data received
		///		so far. If after returning \ref p_context is not 0, this is an indication that less data
		///		was received than that which was requested.
		///		The idea is that the user should pass the same buffer, with the same size, with this
		///		updated context in order to keep feeling the buffer until it's complete.
		///		Once all data is received \ref p_context will be 0. If after returning, the return code is
		///		\ref core::NET_Error::NET_NoErr and \ref p_context is 0, this means that all requested data was
		///		received.
		///		This type of algorithm is ideal in situations where the amount of data to be received is known in advance
		///		by an agreed upon prootocol common on both ends.
		///		For example, if a user wants to receive 500B but only 200B are able to proccessed at the moment,
		///		before the call \ref p_context starts at 0 and becomes 200 after the call. When the user calls
		///		the receive function again with \ref p_context at 200, the method will continue writing on the buffer
		///		at position 200B and only tries to receive the remaining 300B left unreceived,
		///		if on this call all remaing 300B are received, then \ref p_context becomes 0 again.
		///		If an error is returned \ref p_context is left unchanged, so if this is 0 before calling,
		///		and remains 0 after the call but an error is returned, this means that no data was received.
		//
		NET_Error Receive_context	(uint8_t* p_buffer, const uintptr_t p_size, uintptr_t& p_context);

		///	\brief Receives data pending on the socket using a size tracking algorithm to handle unreceived data
		///	\param[in] p_buffer - p_buffer to receive the data
		///	\param[in] p_size - amount of data to receive
		///	\param[out] p_received - amount of data actually received
		///	\return \ref core::NET_Error
		///
		///	\remarks
		///		On blocking sockets, this call will block until data is received (which may be less than the amount requested).
		///		On a non-blocking socket this can return \ref core::NET_Error::NET_WouldBlock indicating
		///		that the request could not be serviced at the moment without blocking.
		///		When a data receive is requested on a TCP socket it is not guaranteed that the system
		///		has the full amount pending on the socket, and can return having received less data than what was
		///		actually requested. Thus it is necessary that users handles this conditions with specialized algorithms
		///		to make sure that the right data is received.
		///		This type of algorithm is ideal in situations where the amount of data to be received is unknown.
		///		With a size type algorithm the user is responsible for keeping track of the data that was already
		///		received and adjust the buffer for the next send call.
		///		If an error is returned \ref p_received is left unchanged.
		//
		NET_Error Receive_size		(uint8_t* p_buffer, const uintptr_t p_size, uintptr_t& p_received);

		///	\brief Turns on or off the Nagle's algorithm on the socket. By default the Nagle's algorithm is on.
		///	\param[in] p_useNagle - If true turns on the Nagle's algorithm, if false turns off the Nagle's algorithm
		///	\return \ref core::NET_Error
		///
		///	\remarks
		///		The Nagle's algorithm is a network optimization algorithm tha delays sending TCP data in case there is
		///		more data to send, allowing the system to concatenate messages togheter in the same packet thus reducing
		///		network traffic, but at the cost of a time delay.
		///		Please see: https://en.wikipedia.org/wiki/Nagle%27s_algorithm
		//
		NET_Error SetNagle	(const bool p_useNagle);

		///	\brief Turns on or off 'Keep Alive' packets on a tcp connection
		///	\param[in] p_keepAlive - If true truns On Keep Alive, if false turns it off
		///	\param[in] p_probePeriod - If p_keepAlive is true, sets the period (in seconds) at which keep alive probes are sent, otherwise has no effect
		///								Required p_probePeriod > 1
		///	\param[in] p_maxProbes - If p_keepAlive is true, sets the maximuum number of failed probes after which connection is considered droped, otherwise has no effect
		///								Required that p_maxProbes > 1 and p_maxProbes * p_probePeriod <= 9000, and recommended that p_maxProbes > 8
		///	\return \ref core::NET_Error
		///
		///	\remarks
		///		Underlying behaviour has some variability depending on operating system
		///		please refer to:
		///			https://docs.microsoft.com/en-us/windows/win32/winsock/so-keepalive
		///			https://linux.die.net/man/7/tcp
		///		for more information.
		//
		NET_Error SetKeepAlive(const bool p_keepAlive, const uint32_t p_probePeriod, const uint32_t p_maxProbes);
	};

} //namesapce core_p


	/// \brief provides a UDP IPv4 interface
	//
	class NetUDP_V4: public core_p::NetUDP_p
	{
	public:
		using Endpoint = core_p::Net_Socket::Endpoint;

	public:
		NetUDP_V4() = default;
		NetUDP_V4(NetUDP_V4&& p_other);

		using Net_Socket::CloseSocket;
		using NetUDP_p::PeekSize;
		using NetUDP_p::Receive;

		///	\brief Creates the socket
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		//
		NET_Error OpenSocket	(const bool p_blocking = true);

		///	\brief Binds a previously created socket with \ref OpenSocket to a specific network interface
		///	\param[in] p_IP - IP address of the interface to bind too. If 0.0.0.0 is used, then the socket is bound to any address
		///	\param[in] p_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\return \ref core::NET_Error
		//
		NET_Error Bind			(const IPv4_netAddr& p_IP, const uint16_t p_Port);

		///	\brief performs both a \ref OpenSocket and \ref Bind
		///	\param[in] p_IP - IP address of the interface to bind too. If 0.0.0.0 is used, then the socket is bound to any address
		///	\param[in] p_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		//
		NET_Error OpenAndBind	(const IPv4_netAddr& p_IP, const uint16_t p_Port, bool p_blocking = true);

		///	\brief Joins a multicast group
		///	\param[in] p_group - address of the multicast group
		///	\param[in] p_interface - address of the interface to use (in raw form).  If 0 is used, then the socket uses the default interface.
		///	\return \ref core::NET_Error
		///	\remarks Must be called after a socket has been created (\ref OpenSocket), but before it is bound (\ref Bind)
		//
		NET_Error JoinMulticastGroup(const IPv4_netAddr& p_group, const uint32_t p_interface);

		///	\brief leaves a multicast group
		///	\param[in] p_group - address of the multicast group
		///	\param[in] p_interface - address of the interface to use (in raw form). If 0 is used, then the socket uses the default interface.
		///	\return \ref core::NET_Error
		//
		NET_Error LeaveMulticastGroup(const IPv4_netAddr& p_group, const uint32_t p_interface);

		///	\brief Gets the address information of the interface. Usefull if an implicit binding is used.
		///	\param[in] p_IP - receives the IP address
		///	\param[in] p_Port - receives the port number
		///	\return \ref core::NET_Error
		//
		NET_Error GetAddress		(IPv4_netAddr& p_IP, uint16_t& p_Port);

		///	\brief Sends data to the network
		///	\param[in] p_data - buffer containing the data to send
		///	\param[in] p_size - size of p_data
		///	\param[in] p_IP - Destination IP address
		///	\param[in] p_Port - Destination port number
		///	\param[in] p_repeat - Number of times to repeat sending this message
		///	\return \ref core::NET_Error
		///	\remarks
		///		An implicit bind will be made on the socket if it hasn't be bound before.
		///		On blocking sockets, this call will block until the data has been sent.
		///		On a non-blocking socket this can return \ref core::NET_Error::NET_WouldBlock
		///		if data could not be sent at tthe time without blocking.
		//
		NET_Error Send		(const uint8_t* p_data, const uintptr_t p_size, const IPv4_netAddr& p_IP, const uint16_t p_Port, const uint8_t p_repeat = 0);
		
		///	\brief Collects the data pending on the socket, and retrieves address of the sender
		///	\param[out] p_data - pointer to buffer that receives the data
		///	\param[in, out] p_size - on input it informs the size of the receiving buffer, on success the value is updated with the ammout of data read
		///					the absolute mmaximum amount of data that can be recieved on a single UDP packet is 65507 Bytes, however this size can be lower depending
		///					on the underlying implementation.
		///	\param[out] p_other_IP - The sender's IP address as reported by the sender
		///	\param[out] p_other_port - The sender's port number as reported by the sender
		///	\return \ref core::NET_Error
		///
		///	\remarks
		///		On blocking sockets, this call will block until data is ready to be read on the socket.
		///		On a non-blocking socket this can return \ref core::NET_Error::NET_WouldBlock if no data is ready to be read.
		//
		NET_Error Receive	(uint8_t* p_data, uintptr_t& p_size, IPv4_netAddr& p_other_IP, uint16_t& p_other_port);

		///	\brief Checks the size of the next data pack ready to be received on the socket as well as the address of the sender
		///	\param[out] p_size - Returns the size of the packet
		///	\param[out] p_other_IP - The sender's IP address as reported by the sender
		///	\param[out] p_other_port - The sender's port number as reported by the sender
		///	\return \ref core::NET_Error
		///
		///	\remarks
		///		On blocking sockets, this call will block until data is ready to be read on the socket.
		///		On a non-blocking socket this can return \ref core::NET_Error::NET_WouldBlock if no data is ready to be read.
		//
		NET_Error PeekSize	(uintptr_t& p_size, IPv4_netAddr& p_other_IP, uint16_t& p_other_port);

		///	\brief Sends a magic packet commonly used for Wake On Lan
		///	\param[in] p_MacAddress - The MAC address of the interface to wake up
		///	\param[in] p_subNet - IP address of the subsystem responsibel for reaching the target device (typically a broadcast address is used)
		///	\param[in] p_port - Pointer to a variable containing the target port number where WOL is configured.
		///						Or nullptr to send the message to commonly used port numbers (i.e. send to both port 7 and port 9)
		///	\return \ref core::NET_Error
		///
		///	\remarks
		///		Has the same properties as \ref Send, except the data payload is managed internally by the method.
		//
		NET_Error WakeOnLan			(std::span<const uint8_t, 6> p_MacAddress, const IPv4_netAddr& p_subNet, const uint16_t* p_port = nullptr);

		///	\brief Sends a magic packet commonly used for Wake On Lan on systems with password
		///	\param[in] p_MacAddress - The MAC address of the interface to wake up
		///	\param[in] p_subNet - IP address of the subsystem responsibel for reaching the target device (typically a broadcast address is used)
		///	\param[in] p_port - Pointer to a variable containing the target port number where WOL is configured.
		///						Or nullptr to send the message to commonly used port numbers (i.e. send to both port 7 and port 9)
		///	\param[in] p_password - Buffer containing the password message to be appended at the end of the magic packet.
		///	\param[in] p_password_size - The size of the password buffer. If 0 the behaviour is identical to \ref WakeOnLan
		///	\return \ref core::NET_Error
		///
		///	\remarks
		///		Has the same properties as \ref Send, except the data payload is managed internally by the method.
		//
		NET_Error WakeOnLan_password(std::span<const uint8_t, 6> p_MacAddress, const IPv4_netAddr& p_subNet, const uint16_t* p_port = nullptr, const uint8_t* p_password = nullptr, const uint16_t p_password_size = 0);

		///	\brief Swaps this socket with another
		//
		void swap(NetUDP_V4& p_other);
	};

	/// \brief provides a UDP IPv6 interface
	//
	class NetUDP_V6: public core_p::NetUDP_p
	{
	public:
		using Endpoint = core_p::Net_Socket::Endpoint;

	public:
		NetUDP_V6() = default;
		NetUDP_V6(NetUDP_V6&& p_other);

		using Net_Socket::CloseSocket;
		using NetUDP_p::PeekSize;
		using NetUDP_p::Receive;

		///	\brief Creates the socket
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		//
		NET_Error OpenSocket	(const bool p_blocking = true);

		///	\brief Binds a previously created socket with \ref OpenSocket to a specific network interface
		///	\param[in] p_IP - IP address of the interface to bind too. If ::0 is used, then the socket is bound to any address
		///	\param[in] p_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\return \ref core::NET_Error
		//
		NET_Error Bind			(const IPv6_netAddr& p_IP, const uint16_t p_Port);

		///	\brief performs both a \ref OpenSocket and \ref Bind
		///	\param[in] p_IP - IP address of the interface to bind too. If ::0 is used, then the socket is bound to any address
		///	\param[in] p_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		//
		NET_Error OpenAndBind	(const IPv6_netAddr& p_IP, const uint16_t p_Port, const bool p_blocking = true);

		///	\brief Joins a multicast group
		///	\param[in] p_group - address of the multicast group
		///	\param[in] p_interface - Interface index to use. If 0 is used, then the socket picks up the default.
		///	\return \ref core::NET_Error
		///	\remarks Must be called after a socket has been created (\ref OpenSocket), but before it is bound (\ref Bind)
		//
		NET_Error JoinMulticastGroup(const IPv6_netAddr& p_group, const uint32_t p_interface);

		///	\brief Leavess a multicast group
		///	\param[in] p_group - address of the multicast group
		///	\param[in] p_interface - Interface index to use. If 0 is used, then the socket picks up the default.
		///	\return \ref core::NET_Error
		//
		NET_Error LeaveMulticastGroup(const IPv6_netAddr& p_group, const uint32_t p_interface);

		///	\brief Gets the address information of the interface. Usefull if an implicit binding is used.
		///	\param[in] p_IP - receives the IP address
		///	\param[in] p_Port - receives the port number
		///	\return \ref core::NET_Error
		//
		NET_Error GetAddress		(IPv6_netAddr&	p_IP, uint16_t& p_Port);

		///	\brief Sends data to the network
		///	\param[in] p_data - buffer containing the data to send
		///	\param[in] p_size - size of p_data
		///	\param[in] p_IP - Destination IP address
		///	\param[in] p_Port - Destination port number
		///	\param[in] p_repeat - Number of times to repeat sending this message
		///	\return \ref core::NET_Error
		///	\remarks
		///		An implicit bind will be made on the socket if it hasn't be bound before.
		///		On blocking sockets, this call will block until the data has been sent.
		///		On a non-blocking socket this can return \ref core::NET_Error::NET_WouldBlock
		///		if data could not be sent at tthe time without blocking.
		//
		NET_Error Send		(const uint8_t* p_data, const uintptr_t p_size, const IPv6_netAddr& p_IP, const uint16_t p_Port, const uint8_t p_repeat = 0);

		///	\brief Collects the data pending on the socket, and retrieves address of the sender
		///	\param[out] p_data - pointer to buffer that receives the data
		///	\param[in, out] p_size - on input it informs the size of the receiving buffer, on success the value is updated with the ammout of data read
		///					the absolute mmaximum amount of data that can be recieved on a single UDP packet is 65507 Bytes, however this size can be lower depending
		///					on the underlying implementation.
		///	\param[out] p_other_IP - The sender's IP address as reported by the sender
		///	\param[out] p_other_port - The sender's port number as reported by the sender
		///	\return \ref core::NET_Error
		///
		///	\remarks
		///		On blocking sockets, this call will block until data is ready to be read on the socket.
		///		On a non-blocking socket this can return \ref core::NET_Error::NET_WouldBlock if no data is ready to be read.
		//
		NET_Error Receive	(uint8_t* p_data, uintptr_t& p_size, IPv6_netAddr& p_other_IP, uint16_t& p_other_port);

		///	\brief Checks the size of the next data pack ready to be received on the socket as well as the address of the sender
		///	\param[out] p_size - Returns the size of the packet
		///	\param[out] p_other_IP - The sender's IP address as reported by the sender
		///	\param[out] p_other_port - The sender's port number as reported by the sender
		///	\return \ref core::NET_Error
		///
		///	\remarks
		///		On blocking sockets, this call will block until data is ready to be read on the socket.
		///		On a non-blocking socket this can return \ref core::NET_Error::NET_WouldBlock if no data is ready to be read.
		//
		NET_Error PeekSize	(uintptr_t& p_size, IPv6_netAddr& p_other_IP, uint16_t& p_other_port);

		///	\brief Sends a magic packet commonly used for Wake On Lan
		///	\param[in] p_MacAddress - The MAC address of the interface to wake up
		///	\param[in] p_subNet - IP address of the subsystem responsibel for reaching the target device (typically a broadcast address is used)
		///	\param[in] p_port - Pointer to a variable containing the target port number where WOL is configured.
		///						Or nullptr to send the message to commonly used port numbers (i.e. send to both port 7 and port 9)
		///	\return \ref core::NET_Error
		///
		///	\remarks
		///		Has the same properties as \ref Send, except the data payload is managed internally by the method.
		//
		NET_Error WakeOnLan(std::span<const uint8_t, 6> p_MacAddress, const IPv6_netAddr& p_subNet, const uint16_t* p_port = nullptr);
		
		///	\brief Sends a magic packet commonly used for Wake On Lan on systems with password
		///	\param[in] p_MacAddress - The MAC address of the interface to wake up
		///	\param[in] p_subNet - IP address of the subsystem responsibel for reaching the target device (typically a broadcast address is used)
		///	\param[in] p_port - Pointer to a variable containing the target port number where WOL is configured.
		///						Or nullptr to send the message to commonly used port numbers (i.e. send to both port 7 and port 9)
		///	\param[in] p_password - Buffer containing the password message to be appended at the end of the magic packet.
		///	\param[in] p_password_size - The size of the password buffer. If 0 the behaviour is identical to \ref WakeOnLan
		///	\return \ref core::NET_Error
		///
		///	\remarks
		///		Has the same properties as \ref Send, except the data payload is managed internally by the method.
		//
		NET_Error WakeOnLan_password(std::span<const uint8_t, 6> p_MacAddress, const IPv6_netAddr& p_subNet, const uint16_t* p_port = nullptr, const uint8_t* p_password = nullptr, const uint16_t p_password_size = 0);

		///	\brief Swaps this socket with another
		//
		void swap(NetUDP_V6& p_other);
	};

	class NetTCP_C_V4;
	class NetTCP_C_V6;

	/// \brief provides a TCP IPv4 server interface
	//
	class NetTCP_S_V4: public core_p::NetTCP_S_p
	{
	public:
		using Endpoint = core_p::Net_Socket::Endpoint;

	public:
		NetTCP_S_V4() = default;
		NetTCP_S_V4(NetTCP_S_V4&& p_other);

		using Net_Socket::CloseSocket;

		///	\brief Creates the socket
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		//
		NET_Error OpenSocket		(const bool p_blocking = true);

		///	\brief Binds a previously created socket with \ref OpenSocket to a specific network interface
		///	\param[in] p_IP - IP address of the interface to bind too. If 0.0.0.0 is used, then the socket is bound to any address
		///	\param[in] p_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\return \ref core::NET_Error
		//
		NET_Error Bind				(const IPv4_netAddr& p_IP, const uint16_t p_Port);

		///	\brief performs both a \ref OpenSocket and \ref Bind
		///	\param[in] p_IP - IP address of the interface to bind too. If 0.0.0.0 is used, then the socket is bound to any address
		///	\param[in] p_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		//
		NET_Error OpenAndBind	(const IPv4_netAddr& p_IP, const uint16_t p_Port, bool p_blocking = true);

		///	\brief performs a \ref OpenSocket, \ref Bind, and \ref listen in one method
		///	\param[in] p_IP - IP address of the interface to bind too. If 0.0.0.0 is used, then the socket is bound to any address
		///	\param[in] p_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\param[in] p_max_connections - Number of connections allowed to be pending on the socket before starting to refuse them
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		//
		NET_Error OpenBindAndListen	(const IPv4_netAddr& p_IP, const uint16_t p_Port, const int p_max_connections, const bool p_blocking = true);

		///	\brief Accepts a connection request pending on the socket.
		///	\param[out] p_Client - A client object that will manage the client communication. Object must be unused.
		///	\param[in] p_blocking - Determines if the new socket should be a blocking or non-blocking
		///	\return \ref core::NET_Error
		//
		NET_Error Accept		(NetTCP_C_V4& p_Client, const bool p_blocking = true);

		///	\brief Accepts a connection request pending on the socket.
		///	\param[out] p_Client - A client object that will manage the client communication. Object must be unused.
		///	\param[out] p_other_IP - The peer's IP address.
		///	\param[out] p_other_port - The peer's port number
		///	\param[in] p_blocking - Determines if the new socket should be a blocking or non-blocking
		///	\return \ref core::NET_Error
		//
		NET_Error Accept		(NetTCP_C_V4& p_Client, IPv4_netAddr& p_other_IP, uint16_t& p_other_port, const bool p_blocking = true);

		///	\brief Gets the address information of the interface. Usefull if \ref Bind is used without specifying the address.
		///	\param[in] p_IP - receives the IP address
		///	\param[in] p_Port - receives the port number
		///	\return \ref core::NET_Error
		//
		NET_Error GetAddress	(IPv4_netAddr& p_IP, uint16_t& p_Port);

		///	\brief Swaps this socket with another
		//
		void swap(NetTCP_S_V4& p_other);
	};

	/// \brief provides a TCP IPv6 server interface
	//
	class NetTCP_S_V6: public core_p::NetTCP_S_p
	{
	public:
		using Endpoint = core_p::Net_Socket::Endpoint;

	public:
		NetTCP_S_V6() = default;
		NetTCP_S_V6(NetTCP_S_V6&& p_other);

		using Net_Socket::CloseSocket;

		///	\brief Creates the socket
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		//
		NET_Error OpenSocket		(const bool p_blocking = true);

		///	\brief Binds a previously created socket with \ref OpenSocket to a specific network interface
		///	\param[in] p_IP - IP address of the interface to bind too. If ::0 is used, then the socket is bound to any address
		///	\param[in] p_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\return \ref core::NET_Error
		//
		NET_Error Bind				(const IPv6_netAddr& p_IP, const uint16_t p_Port);

		///	\brief performs both a \ref OpenSocket and \ref Bind
		///	\param[in] p_IP - IP address of the interface to bind too. If ::0 is used, then the socket is bound to any address
		///	\param[in] p_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		//
		NET_Error OpenAndBind	(const IPv6_netAddr& p_IP, const uint16_t p_Port, bool p_blocking = true);

		///	\brief performs a \ref OpenSocket, \ref Bind, and \ref listen in one method
		///	\param[in] p_IP - IP address of the interface to bind too. If ::0 is used, then the socket is bound to any address
		///	\param[in] p_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\param[in] p_max_connections - Number of connections allowed to be pending on the socket before starting to refuse them
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		//
		NET_Error OpenBindAndListen	(const IPv6_netAddr& p_IP, const uint16_t p_Port, const int p_max_connections, const bool p_blocking = true);

		///	\brief Accepts a connection request pending on the socket.
		///	\param[out] p_Client - A client object that will manage the client communication. Object must be unused.
		///	\param[in] p_blocking - Determines if the new socket should be a blocking or non-blocking
		///	\return \ref core::NET_Error
		///	\remarks
		///		On blocking sockets, this call will block until a clients is pending to connect.
		///		On a non-blocking socket this can return \ref core::NET_Error::NET_WouldBlock if no client is pending.
		//
		NET_Error Accept		(NetTCP_C_V6& p_Client, const bool p_blocking = true);

		///	\brief Accepts a connection request pending on the socket.
		///	\param[out] p_Client - A client object that will manage the client communication. Object must be unused.
		///	\param[out] p_other_IP - The peer's IP address.
		///	\param[out] p_other_port - The peer's port number
		///	\param[in] p_blocking - Determines if the new socket should be a blocking or non-blocking
		///	\return \ref core::NET_Error
		///	\remarks
		///		On blocking sockets, this call will block until a clients is pending to connect.
		///		On a non-blocking socket this can return \ref core::NET_Error::NET_WouldBlock if no client is pending.
		//
		NET_Error Accept		(NetTCP_C_V6& p_Client, IPv6_netAddr& p_other_IP, uint16_t& p_other_port, const bool p_blocking = true);

		///	\brief Gets the address information of the interface. Usefull if \ref Bind is used without specifying the address.
		///	\param[in] p_IP - receives the IP address
		///	\param[in] p_Port - receives the port number
		///	\return \ref core::NET_Error
		//
		NET_Error GetAddress	(IPv6_netAddr& p_IP, uint16_t& p_Port);

		///	\brief Swaps this socket with another
		//
		void swap(NetTCP_S_V6& p_other);
	};

	/// \brief provides a TCP IPv4 client interface
	//
	class NetTCP_C_V4: public core_p::NetTCP_C_p
	{
		friend class NetTCP_S_V4;

	public:
		using Endpoint = core_p::Net_Socket::Endpoint;

	public:
		NetTCP_C_V4() = default;
		NetTCP_C_V4(NetTCP_C_V4&& p_other);

		using Net_Socket::CloseSocket;

		///	\brief Creates the socket
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		//
		NET_Error OpenSocket		(const bool p_blocking = true);

		///	\brief Binds a previously created socket with \ref OpenSocket to a specific network interface
		///	\param[in] p_IP - IP address of the interface to bind too. If 0.0.0.0 is used, then the socket is bound to any address
		///	\param[in] p_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\return \ref core::NET_Error
		//
		NET_Error Bind				(const IPv4_netAddr& my_IP, const uint16_t my_Port);

		///	\brief Connects a previously created socket with \ref OpenSocket to a server.
		///	\param[in] dest_IP - IP address of the server to connect too.
		///	\param[in] dest_Port - Port number to the server to connect too.
		///	\return \ref core::NET_Error
		///	\remarks
		///		An implicit bind will be made on the socket if it hasn't be bound before.
		///		On blocking sockets, this call will block until the connection is established.
		///		On a non-blocking socket this can return \ref core::NET_Error::NET_WouldBlock if the connection
		///		could not be completed immediately. The user must then use \ref NonBlock_Connect_state to check
		///		when the connections is actually established.
		//
		NET_Error Connect			(const IPv4_netAddr& dest_IP, const uint16_t dest_Port);

		///	\brief performs both a \ref OpenSocket and \ref Bind
		///	\param[in] my_IP - IP address of the interface to bind too. If 0.0.0.0 is used, then the socket is bound to any address
		///	\param[in] my_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		//
		NET_Error OpenAndBind	(const IPv4_netAddr& my_IP, const uint16_t my_Port, bool p_blocking = true);

		///	\brief performs a \ref OpenSocket, \ref Bind, and \ref Connect in one method
		///	\param[in] my_IP - IP address of the interface to bind too. If 0.0.0.0 is used, then the socket is bound to any address
		///	\param[in] my_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\param[in] dest_IP - IP address of the server to connect too.
		///	\param[in] dest_Port - Port number to the server to connect too.
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		///	\remarks
		///		An implicit bind will be made on the socket if it hasn't be bound before.
		///		On blocking sockets, this call will block until the connection is established.
		///		On a non-blocking socket this can return \ref core::NET_Error::NET_WouldBlock if the connection
		///		could not be completed immediately. The user must then use \ref NonBlock_Connect_state to check
		///		when the connections is actually established.
		//
		NET_Error OpenBindAndConnect(const IPv4_netAddr& my_IP, const uint16_t my_Port, const IPv4_netAddr& dest_IP, const uint16_t dest_Port, const bool p_blocking = true);

		///	\brief Gets the address information of the interface. Usefull if \ref Bind is used without specifying the address.
		///	\param[in] p_IP - receives the IP address
		///	\param[in] p_Port - receives the port number
		///	\return \ref core::NET_Error
		//
		NET_Error GetAddress	(IPv4_netAddr& p_IP, uint16_t& p_Port);

		///	\brief Gets the peer's address information.
		///	\param[in] p_IP - receives the IP address
		///	\param[in] p_Port - receives the port number
		///	\return \ref core::NET_Error
		//
		NET_Error GetPeerAddress(IPv4_netAddr& p_IP, uint16_t& p_Port);

		///	\brief Swaps this socket with another
		//
		void swap(NetTCP_C_V4& p_other);
	};

	/// \brief provides a TCP IPv6 client interface
	//
	class NetTCP_C_V6: public core_p::NetTCP_C_p
	{
		friend class NetTCP_S_V6;

	public:
		using Endpoint = core_p::Net_Socket::Endpoint;

	public:
		NetTCP_C_V6() = default;
		NetTCP_C_V6(NetTCP_C_V6&& p_other);

		using Net_Socket::CloseSocket;

		///	\brief Creates the socket
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		//
		NET_Error OpenSocket		(const bool p_blocking = true);

		///	\brief Binds a previously created socket with \ref OpenSocket to a specific network interface
		///	\param[in] p_IP - IP address of the interface to bind too. If ::0 is used, then the socket is bound to any address
		///	\param[in] p_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\return \ref core::NET_Error
		//
		NET_Error Bind				(const IPv6_netAddr& my_IP, const uint16_t my_Port);

		///	\brief Connects a previously created socket with \ref OpenSocket to a server.
		///	\param[in] dest_IP - IP address of the server to connect too.
		///	\param[in] dest_Port - Port number to the server to connect too.
		///	\return \ref core::NET_Error
		///	\remarks
		///		An implicit bind will be made on the socket if it hasn't be bound before.
		///		On blocking sockets, this call will block until the connection is established.
		///		On a non-blocking socket this can return \ref core::NET_Error::NET_WouldBlock if the connection
		///		could not be completed immediately. The user must then use \ref NonBlock_Connect_state to check
		///		when the connections is actually established.
		//
		NET_Error Connect		(const IPv6_netAddr& dest_IP, const uint16_t dest_Port);

		///	\brief performs both a \ref OpenSocket and \ref Bind
		///	\param[in] my_IP - IP address of the interface to bind too. If ::0 is used, then the socket is bound to any address
		///	\param[in] my_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		//
		NET_Error OpenAndBind	(const IPv6_netAddr& my_IP, const uint16_t my_Port, bool p_blocking = true);

		///	\brief performs a \ref OpenSocket, \ref Bind, and \ref Connect in one method
		///	\param[in] my_IP - IP address of the interface to bind too. If ::0 is used, then the socket is bound to any address
		///	\param[in] my_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\param[in] dest_IP - IP address of the server to connect too.
		///	\param[in] dest_Port - Port number to the server to connect too.
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		///	\remarks
		///		An implicit bind will be made on the socket if it hasn't be bound before.
		///		On blocking sockets, this call will block until the connection is established.
		///		On a non-blocking socket this can return \ref core::NET_Error::NET_WouldBlock if the connection
		///		could not be completed immediately. The user must then use \ref NonBlock_Connect_state to check
		///		when the connections is actually established.
		//
		NET_Error OpenBindAndConnect(const IPv6_netAddr& my_IP, const uint16_t my_Port, const IPv6_netAddr& dest_IP, const uint16_t dest_Port, const bool p_blocking = true);

		///	\brief Gets the address information of the interface. Usefull if \ref Bind is used without specifying the address.
		///	\param[in] p_IP - receives the IP address
		///	\param[in] p_Port - receives the port number
		///	\return \ref core::NET_Error
		//
		NET_Error GetAddress	(IPv6_netAddr& p_IP, uint16_t& p_Port);

		///	\brief Gets the peer's address information.
		///	\param[in] p_IP - receives the IP address
		///	\param[in] p_Port - receives the port number
		///	\return \ref core::NET_Error
		//
		NET_Error GetPeerAddress(IPv6_netAddr& p_IP, uint16_t& p_Port);

		///	\brief Swaps this socket with another
		//
		void swap(NetTCP_C_V6& p_other);
	};


	//========= ======== ======== IPv Neutral ========= ======== ========

	/// \brief Provides an IPv agnostic UDP interface
	//
	class NetUDP: public core_p::NetUDP_p
	{
	public:
		using IPv			= IP_netAddr::IPv;
		using Endpoint	= core_p::Net_Socket::Endpoint;

	private:
		IPv m_IpV;

	public:
		NetUDP();
		NetUDP(NetUDP&& p_other);

		using NetUDP_p::PeekSize;
		using NetUDP_p::Receive;

		///	\brief Closes a previously open socket
		///	\return \ref core::NET_Error specifically it return \ref core::NET_Error::NET_NoErr if the socket is successfully released.
		///	\remarks
		///		It closes the socket if one was previously open.
		///		This function may fail or block depending on the blocking and linger properties set for the socket.
		///		For more details please see:
		///			https://docs.microsoft.com/en-us/windows/desktop/api/winsock/nf-winsock-closesocket
		///			https://linux.die.net/man/7/socket
		//
		NET_Error CloseSocket		();

		///	\brief Creates the socket
		///	\param[in] p_ipV - Internet Protocol version too use for this socket
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		///	\remarks System should not mix IPv4 and IPv6 protocols
		//
		NET_Error OpenSocket		(const IPv p_ipV, const bool p_blocking = true);

		///	\brief Binds a previously created socket with \ref OpenSocket to a specific network interface
		///	\param[in] p_IP - IP address of the interface to bind too. If 0.0.0.0 (on IPv4) or ::0 (on IPv6) is used, then the socket is bound to any address
		///	\param[in] p_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\return \ref core::NET_Error
		//
		NET_Error Bind				(const IP_netAddr& p_IP, const uint16_t p_Port);

		///	\brief performs both a \ref OpenSocket and \ref Bind
		///	\param[in] p_IP - IP address of the interface to bind too. If 0.0.0.0 (on IPv4) or ::0 (on IPv6) is used, then the socket is bound to any address
		///	\param[in] p_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		//
		NET_Error OpenAndBind		(const IP_netAddr& p_IP, const uint16_t p_Port, const bool p_blocking = true);

		///	\brief Joins a multicast group
		///	\param[in] p_group - address of the multicast group
		///	\param[in] p_interface - On IPv4 it represents the RAW address of the interface in integer form, on IPv6 it represents the interface index. If 0 is used, then the socket uses the default interface.
		///	\return \ref core::NET_Error
		///	\remarks Must be called after a socket has been created (\ref OpenSocket), but before it is bound (\ref Bind)
		//
		NET_Error JoinMulticastGroup(const IP_netAddr& p_group, const uint32_t p_interface);

		///	\brief Leaves a multicast group
		///	\param[in] p_group - address of the multicast group
		///	\param[in] p_interface - On IPv4 it represents the RAW address of the interface in integer form, on IPv6 it represents the interface index. If 0 is used, then the socket uses the default interface.
		///	\return \ref core::NET_Error
		//
		NET_Error LeaveMulticastGroup(const IP_netAddr& p_group, const uint32_t p_interface);

		///	\brief Gets the address information of the interface. Usefull if an implicit binding is used.
		///	\param[in] p_IP - receives the IP address
		///	\param[in] p_Port - receives the port number
		///	\return \ref core::NET_Error
		//
		NET_Error GetAddress		(IP_netAddr& p_IP, uint16_t& p_Port);

		///	\brief Sends data to the network
		///	\param[in] p_data - buffer containing the data to send
		///	\param[in] p_size - size of p_data
		///	\param[in] p_IP - Destination IP address
		///	\param[in] p_Port - Destination port number
		///	\param[in] p_repeat - Number of times to repeat sending this message
		///	\return \ref core::NET_Error
		///	\remarks
		///		An implicit bind will be made on the socket if it hasn't be bound before.
		///		On blocking sockets, this call will block until the data has been sent.
		///		On a non-blocking socket this can return \ref core::NET_Error::NET_WouldBlock
		///		if data could not be sent at tthe time without blocking.
		///		IPv4 sockets can not send to IPv6 networks and vice-versa.
		//
		NET_Error Send(const uint8_t* p_data, const uintptr_t p_size, const IP_netAddr& p_IP, const uint16_t p_Port, const uint8_t p_repeat = 0);

		///	\brief Collects the data pending on the socket, and retrieves address of the sender
		///	\param[out] p_data - pointer to buffer that receives the data
		///	\param[in, out] p_size - on input it informs the size of the receiving buffer, on success the value is updated with the ammout of data read
		///					the absolute mmaximum amount of data that can be recieved on a single UDP packet is 65507 Bytes, however this size can be lower depending
		///					on the underlying implementation.
		///	\param[out] p_other_IP - The sender's IP address as reported by the sender
		///	\param[out] p_other_port - The sender's port number as reported by the sender
		///	\return \ref core::NET_Error
		///
		///	\remarks
		///		On blocking sockets, this call will block until data is ready to be read on the socket.
		///		On a non-blocking socket this can return \ref core::NET_Error::NET_WouldBlock if no data is ready to be read.
		//
		NET_Error Receive	(uint8_t* p_data, uintptr_t& p_size, IP_netAddr& p_other_IP, uint16_t& p_other_port);

		///	\brief Checks the size of the next data pack ready to be received on the socket as well as the address of the sender
		///	\param[out] p_size - Returns the size of the packet
		///	\param[out] p_other_IP - The sender's IP address as reported by the sender
		///	\param[out] p_other_port - The sender's port number as reported by the sender
		///	\return \ref core::NET_Error
		///
		///	\remarks
		///		On blocking sockets, this call will block until data is ready to be read on the socket.
		///		On a non-blocking socket this can return \ref core::NET_Error::NET_WouldBlock if no data is ready to be read.
		//
		NET_Error PeekSize	(uintptr_t& p_size, IP_netAddr& p_other_IP, uint16_t& p_other_port);

		///	\brief Sends a magic packet commonly used for Wake On Lan
		///	\param[in] p_MacAddress - The MAC address of the interface to wake up
		///	\param[in] p_subNet - IP address of the subsystem responsibel for reaching the target device (typically a broadcast address is used)
		///	\param[in] p_port - Pointer to a variable containing the target port number where WOL is configured.
		///						Or nullptr to send the message to commonly used port numbers (i.e. send to both port 7 and port 9)
		///	\return \ref core::NET_Error
		///
		///	\remarks
		///		Has the same properties as \ref Send, except the data payload is managed internally by the method.
		//
		NET_Error WakeOnLan			(std::span<const uint8_t, 6> p_MacAddress, const IP_netAddr& p_subNet, const uint16_t* p_port = nullptr);

		///	\brief Sends a magic packet commonly used for Wake On Lan on systems with password
		///	\param[in] p_MacAddress - The MAC address of the interface to wake up
		///	\param[in] p_subNet - IP address of the subsystem responsibel for reaching the target device (typically a broadcast address is used)
		///	\param[in] p_port - Pointer to a variable containing the target port number where WOL is configured.
		///						Or nullptr to send the message to commonly used port numbers (i.e. send to both port 7 and port 9)
		///	\param[in] p_password - Buffer containing the password message to be appended at the end of the magic packet.
		///	\param[in] p_password_size - The size of the password buffer. If 0 the behaviour is identical to \ref WakeOnLan
		///	\return \ref core::NET_Error
		///
		///	\remarks
		///		Has the same properties as \ref Send, except the data payload is managed internally by the method.
		//
		NET_Error WakeOnLan_password(std::span<const uint8_t, 6> p_MacAddress, const IP_netAddr& p_subNet, const uint16_t* p_port = nullptr, const uint8_t* p_password = nullptr, const uint16_t p_password_size = 0);

		///	\brief Retrieves the Internet Protocol version currently used on this socket.
		///	\return \ref IPv
		///
		///	\remarks System should not mix IPv4 and IPv6 protocols
		//
		IPv IPversion() const;

		///	\brief Swaps this socket with another
		//
		void swap(NetUDP& p_other);
	};


	class NetTCP_C;

	/// \brief Provides an IPv agnostic TCP server interface
	//
	class NetTCP_S: public core_p::NetTCP_S_p
	{
	public:
		using IPv		= IP_netAddr::IPv;
		using Endpoint	= core_p::Net_Socket::Endpoint;

	private:
		IPv m_IpV;

	public:
		NetTCP_S();
		NetTCP_S(NetTCP_S&& p_other);

		///	\brief Closes a previously open socket
		///	\return \ref core::NET_Error specifically it return \ref core::NET_Error::NET_NoErr if the socket is successfully released.
		///	\remarks
		///		It closes the socket if one was previously open.
		///		This function may fail or block depending on the blocking and linger properties set for the socket.
		///		For more details please see:
		///			https://docs.microsoft.com/en-us/windows/desktop/api/winsock/nf-winsock-closesocket
		///			https://linux.die.net/man/7/socket
		//
		NET_Error CloseSocket		();

		///	\brief Creates the socket
		///	\param[in] p_ipV - Internet Protocol version too use for this socket
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		///	\remarks System should not mix IPv4 and IPv6 protocols
		//
		NET_Error OpenSocket		(const IPv p_ipV, const bool p_blocking = true);

		///	\brief Binds a previously created socket with \ref OpenSocket to a specific network interface
		///	\param[in] p_IP - IP address of the interface to bind too. If 0.0.0.0 (on IPv4) or ::0 (on IPv6) is used, then the socket is bound to any address
		///	\param[in] p_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\return \ref core::NET_Error
		//
		NET_Error Bind				(const IP_netAddr& p_IP, const uint16_t p_Port);

		///	\brief performs both a \ref OpenSocket and \ref Bind
		///	\param[in] p_IP - IP address of the interface to bind too. If 0.0.0.0 (on IPv4) or ::0 (on IPv6) is used, then the socket is bound to any address
		///	\param[in] p_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		//
		NET_Error OpenAndBind		(const IP_netAddr& p_IP, const uint16_t p_Port, const bool p_blocking = true);

		///	\brief performs a \ref OpenSocket, \ref Bind, and \ref listen in one method
		///	\param[in] p_IP - IP address of the interface to bind too. If 0.0.0.0 (on IPv4) or ::0 (on IPv6) is used, then the socket is bound to any address
		///	\param[in] p_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\param[in] p_max_connections - Number of connections allowed to be pending on the socket before starting to refuse them
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		//
		NET_Error OpenBindAndListen	(const IP_netAddr& p_IP, const uint16_t p_Port, const int p_max_connections, const bool p_blocking = true);

		///	\brief Accepts a connection request pending on the socket.
		///	\param[out] p_Client - A client object that will manage the client communication. Object must be unused.
		///	\param[in] p_blocking - Determines if the new socket should be a blocking or non-blocking
		///	\return \ref core::NET_Error
		//
		NET_Error Accept		(NetTCP_C& p_Client, const bool p_blocking = true);

		///	\brief Accepts a connection request pending on the socket.
		///	\param[out] p_Client - A client object that will manage the client communication. Object must be unused.
		///	\param[out] p_other_IP - The peer's IP address.
		///	\param[out] p_other_port - The peer's port number
		///	\param[in] p_blocking - Determines if the new socket should be a blocking or non-blocking
		///	\return \ref core::NET_Error
		//
		NET_Error Accept		(NetTCP_C& p_Client, IP_netAddr& p_other_IP, uint16_t& p_other_port, const bool p_blocking = true);

		///	\brief Gets the address information of the interface. Usefull if \ref Bind is used without specifying the address.
		///	\param[in] p_IP - receives the IP address
		///	\param[in] p_Port - receives the port number
		///	\return \ref core::NET_Error
		//
		NET_Error GetAddress	(IP_netAddr& p_IP, uint16_t& p_Port);

		///	\brief Retrieves the Internet Protocol version currently used on this socket.
		///	\return \ref IPv
		///
		///	\remarks System should not mix IPv4 and IPv6 protocols
		//
		IPv IPversion() const;

		///	\brief Swaps this socket with another
		//
		void swap(NetTCP_S& p_other);
	};

	/// \brief Provides an IPv agnostic TCP client interface
	//
	class NetTCP_C: public core_p::NetTCP_C_p
	{
		friend class NetTCP_S;

	public:
		using IPv			= IP_netAddr::IPv;
		using Endpoint	= core_p::Net_Socket::Endpoint;
	private:
		IPv m_IpV;

	public:
		NetTCP_C();
		NetTCP_C(NetTCP_C&& p_other);

		///	\brief Closes a previously open socket
		///	\return \ref core::NET_Error specifically it return \ref core::NET_Error::NET_NoErr if the socket is successfully released.
		///	\remarks
		///		It closes the socket if one was previously open.
		///		This function may fail or block depending on the blocking and linger properties set for the socket.
		///		For more details please see:
		///			https://docs.microsoft.com/en-us/windows/desktop/api/winsock/nf-winsock-closesocket
		///			https://linux.die.net/man/7/socket
		//
		NET_Error CloseSocket		();

		///	\brief Creates the socket
		///	\param[in] p_ipV - Internet Protocol version too use for this socket
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		///	\remarks System should not mix IPv4 and IPv6 protocols
		//
		NET_Error OpenSocket		(const IPv p_ipV, const bool p_blocking = true);

		///	\brief Binds a previously created socket with \ref OpenSocket to a specific network interface
		///	\param[in] p_IP - IP address of the interface to bind too. If 0.0.0.0 (on IPv4) or ::0 (on IPv6) is used, then the socket is bound to any address
		///	\param[in] p_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\return \ref core::NET_Error
		//
		NET_Error Bind				(const IP_netAddr& my_IP, const uint16_t my_Port);

		///	\brief Connects a previously created socket with \ref OpenSocket to a server.
		///	\param[in] dest_IP - IP address of the server to connect too.
		///	\param[in] dest_Port - Port number to the server to connect too.
		///	\return \ref core::NET_Error
		///	\remarks
		///		An implicit bind will be made on the socket if it hasn't be bound before.
		///		On blocking sockets, this call will block until the connection is established.
		///		On a non-blocking socket this can return \ref core::NET_Error::NET_WouldBlock if the connection
		///		could not be completed immediately. The user must then use \ref NonBlock_Connect_state to check
		///		when the connections is actually established.
		//
		NET_Error Connect			(const IP_netAddr& dest_IP, const uint16_t dest_Port);

		///	\brief performs both a \ref OpenSocket and \ref Bind
		///	\param[in] p_IP - IP address of the interface to bind too. If 0.0.0.0 (on IPv4) or ::0 (on IPv6) is used, then the socket is bound to any address
		///	\param[in] p_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		//
		NET_Error OpenAndBind		(const IP_netAddr& my_IP, const uint16_t my_Port, const bool p_blocking = true);

		///	\brief performs a \ref OpenSocket, \ref Bind, and \ref Connect in one method
		///	\param[in] my_IP - IP address of the interface to bind too.  If 0.0.0.0 (on IPv4) or ::0 (on IPv6) is used, then the socket is bound to any address
		///	\param[in] my_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\param[in] dest_IP - IP address of the server to connect too.
		///	\param[in] dest_Port - Port number to the server to connect too.
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		///	\remarks
		///		An implicit bind will be made on the socket if it hasn't be bound before.
		///		On blocking sockets, this call will block until the connection is established.
		///		On a non-blocking socket this can return \ref core::NET_Error::NET_WouldBlock if the connection
		///		could not be completed immediately. The user must then use \ref NonBlock_Connect_state to check
		///		when the connections is actually established.
		//
		NET_Error OpenBindAndConnect(const IP_netAddr& my_IP, const uint16_t my_Port, const IP_netAddr& dest_IP, const uint16_t dest_Port, const bool p_blocking = true);

		///	\brief Gets the address information of the interface. Usefull if \ref Bind is used without specifying the address.
		///	\param[in] p_IP - receives the IP address
		///	\param[in] p_Port - receives the port number
		///	\return \ref core::NET_Error
		//
		NET_Error GetAddress	(IP_netAddr& p_IP, uint16_t& p_Port);

		///	\brief Gets the peer's address information.
		///	\param[in] p_IP - receives the IP address
		///	\param[in] p_Port - receives the port number
		///	\return \ref core::NET_Error
		//
		NET_Error GetPeerAdrress(IP_netAddr& p_IP, uint16_t& p_Port);

		///	\brief Retrieves the Internet Protocol version currently used on this socket.
		///	\return \ref IPv
		///
		///	\remarks System should not mix IPv4 and IPv6 protocols
		//
		IPv IPversion() const;

		///	\brief Swaps this socket with another
		//
		void swap(NetTCP_C& p_other);
	};

#ifdef _WIN32
	///	\brief Initializes the network subsystem for windows. On linux this has no effect.
	///		see https://docs.microsoft.com/en-us/windows/desktop/api/winsock/nf-winsock-wsastartup
	///
	///	\return true if the subsystem was initialized successfully, false on error. On linux always returns true.
	/// \sa Net_End
	//
	bool Net_Init	();

	///	\brief Releases the network subsystem on windows. On linux it has no effect.
	///		see https://docs.microsoft.com/en-us/windows/desktop/api/winsock/nf-winsock-wsacleanup
	/// \sa Net_Init
	//
	void Net_End	();
#else

	///	\brief Initializes the network subsystem for windows. On linux this has no effect.
	///		see https://docs.microsoft.com/en-us/windows/desktop/api/winsock/nf-winsock-wsastartup
	///
	///	\return true if the subsystem was initialized successfully, false on error. On linux always returns true.
	/// \sa Net_End
	//
	inline constexpr bool Net_Init() { return true; }

	///	\brief Releases the network subsystem on windows. On linux it has no effect.
	///		see https://docs.microsoft.com/en-us/windows/desktop/api/winsock/nf-winsock-wsacleanup
	/// \sa Net_Init
	//
	inline constexpr void Net_End	() {}
#endif

	//======== ======== ======== inline optimization ======== ======== ========

	//======== ======== IPv4_netAddr ======== ========
	inline IPv4_netAddr::IPv4_netAddr()								: ui32Type(0) {}
	inline IPv4_netAddr::IPv4_netAddr(uint32_t p_init)				: ui32Type(p_init){}
	inline IPv4_netAddr::IPv4_netAddr(std::span<const uint8_t, 4> p_init) { memcpy(byteField, p_init.data(), 4); }
	inline IPv4_netAddr::IPv4_netAddr(const IPv4_netAddr& p_other)	: ui32Type(p_other.ui32Type) { }


	inline void IPv4_netAddr::setAny() { ui32Type = 0; }
	inline void IPv4_netAddr::swap(IPv4_netAddr& p_other) { std::swap(ui32Type, p_other.ui32Type); }
	inline bool IPv4_netAddr::is_null() const {return ui32Type == 0;}

	inline IPv4_netAddr& IPv4_netAddr::operator = (const IPv4_netAddr& p_other) { ui32Type = p_other.ui32Type; return *this; }
	inline IPv4_netAddr& IPv4_netAddr::operator |= (const IPv4_netAddr& p_other) { ui32Type |= p_other.ui32Type; return *this; }
	inline IPv4_netAddr& IPv4_netAddr::operator &= (const IPv4_netAddr& p_other) { ui32Type &= p_other.ui32Type; return *this; }
	inline IPv4_netAddr& IPv4_netAddr::operator ^= (const IPv4_netAddr& p_other) { ui32Type ^= p_other.ui32Type; return *this; }
	inline IPv4_netAddr IPv4_netAddr::operator | (const IPv4_netAddr& p_other) const { return IPv4_netAddr(ui32Type | p_other.ui32Type); }
	inline IPv4_netAddr IPv4_netAddr::operator & (const IPv4_netAddr& p_other) const { return IPv4_netAddr(ui32Type & p_other.ui32Type); }
	inline IPv4_netAddr IPv4_netAddr::operator ^ (const IPv4_netAddr& p_other) const { return IPv4_netAddr(ui32Type ^ p_other.ui32Type); }
	inline IPv4_netAddr IPv4_netAddr::operator ~ () const { return IPv4_netAddr(~ui32Type); }

	inline bool IPv4_netAddr::operator == (const IPv4_netAddr& p_other) const { return ui32Type == p_other.ui32Type; }
	inline bool IPv4_netAddr::operator != (const IPv4_netAddr& p_other) const { return ui32Type != p_other.ui32Type; }
	inline bool IPv4_netAddr::operator <  (const IPv4_netAddr& p_other) const { return ui32Type < p_other.ui32Type; }

	//======== ======== IPv6_netAddr ======== ========
	inline bool IPv6_netAddr::is_null() const {return ui64Type[0] == 0 && ui64Type[1] == 0;}

	//======== ======== IP_netAddr ======== ========
	inline IP_netAddr::IPv IP_netAddr::version() const { return m_ipv; }
	inline void IP_netAddr::clear() { m_ipv = IPv::None; }
	inline void IP_netAddr::swap(IP_netAddr& p_other) { std::swap(*this, p_other); }

	//======== ======== core_p::Net_Socket ======== ========
	inline void core_p::Net_Socket::swap(Net_Socket& p_other) { std::swap(m_sock, p_other.m_sock); }

	//======== ======== NetUDP_V4 ======== ========
	inline void NetUDP_V4::swap(NetUDP_V4& p_other) { Net_Socket::swap(p_other); }

	//======== ======== NetUDP_V6 ======== ========
	inline void NetUDP_V6::swap(NetUDP_V6& p_other) { Net_Socket::swap(p_other); }

	//======== ======== NetTCP_S_V4 ======== ========
	inline void NetTCP_S_V4::swap(NetTCP_S_V4& p_other) { Net_Socket::swap(p_other); }

	//======== ======== NetTCP_S_V6 ======== ========
	inline void NetTCP_S_V6::swap(NetTCP_S_V6& p_other) { Net_Socket::swap(p_other); }

	//======== ======== NetTCP_C_V4 ======== ========
	inline void NetTCP_C_V4::swap(NetTCP_C_V4& p_other) { Net_Socket::swap(p_other); }

	//======== ======== NetTCP_C_V6 ======== ========
	inline void NetTCP_C_V6::swap(NetTCP_C_V6& p_other) { Net_Socket::swap(p_other); }

	//======== ======== NetUDP ======== ========
	inline void NetUDP::swap(NetUDP& p_other) { Net_Socket::swap(p_other); std::swap(m_IpV, p_other.m_IpV); }
	inline NetUDP::IPv NetUDP::IPversion() const { return m_IpV; }

	//======== ======== NetTCP_S ======== ========
	inline void NetTCP_S::swap(NetTCP_S& p_other) { Net_Socket::swap(p_other); std::swap(m_IpV, p_other.m_IpV); }
	inline NetTCP_S::IPv NetTCP_S::IPversion() const { return m_IpV; }

	//======== ======== NetTCP_C ======== ========
	inline void NetTCP_C::swap(NetTCP_C& p_other) { Net_Socket::swap(p_other); std::swap(m_IpV, p_other.m_IpV); }
	inline NetTCP_C::IPv NetTCP_C::IPversion() const { return m_IpV; }

}	//namespace core

CORE_MAKE_ENUM_FLAG(::core::core_p::Net_Socket::Endpoint)

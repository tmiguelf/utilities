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

#include <CoreLib/Core_Type.hpp>

/// \n
namespace core
{
	/// \brief Network error codes
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

	namespace _p
	{
#ifdef _WIN32
		typedef	uintptr_t	SocketHandle_t;
#else
		typedef	int			SocketHandle_t;
#endif

		///	\brief Private class to implement generic socket functionality
		class Net_Socket
		{
		public:

			/// \brief identifies local socket communication endpoints
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
			bool is_open() const;

			///	\brief Closes a previously open socket
			///	\return \ref core::NET_Error specifically it return \ref core::NET_Error::NoErr if the socket is successfully released.
			///	\remarks
			///		It closes the socket if one was previously open.
			///		This function may fail or block depending on the blocking and linger properties set for the socket.
			///		For more details please see:
			///			https://docs.microsoft.com/en-us/windows/desktop/api/winsock/nf-winsock-closesocket
			///			https://linux.die.net/man/7/socket
			NET_Error close();

			///	\brief Sets the blocking mode of the socket. Sockets are by default blocking.
			///	\param[in] p_blocking - If true sets the socket to blocking, if false sets the socket to non-blocking
			///	\return \ref core::NET_Error
			NET_Error set_blocking(const bool p_blocking);

			///	\brief Marks the socket address to allow re-use. Sockets by default do not allow re-use.
			///	\param[in] p_blocking - If true sets the socket address to permitted for re-use, if false revokes this permission.
			///	\return \ref core::NET_Error
			///	\remark There is never an absolute guarantee that the address is not re-used.
			NET_Error set_reuse_address	(const bool p_reuse);

			///	\brief Sets the linger structure for the socket
			///	\return \ref core::NET_Error
			///	\remarks
			///		For more details pelase see:
			///			https://docs.microsoft.com/en-us/windows/desktop/api/winsock/nf-winsock-closesocket
			///			https://linux.die.net/man/7/socket
			NET_Error set_linger			(const bool p_linger, const uint16_t p_timeout);

			///	\brief Used to wait until data arrives on the reading end of the socket
			///	\param[in] p_microseconds - Time in microseconds to wait for data to arrive at the socket before abandoning the operation
			///	\return \ref core::NET_Error, more specifically \ref core::NET_Error::NoErr on success,
			///		and \ref core::NET_Error::WouldBlock if the operation was abandoned. Or other for any other errors.
			///
			///	\remarks
			///			If input time is larger than std::numeric_limits<long>::max() x 1000000,
			///			then the call will block indefinitely on blocking sockets.
			NET_Error poll(const uint64_t p_microseconds);

			///	\brief Closes the communication on a specific endpoint of the socket
			///	\param[in] p_direction - core::core_p::Net_Socket::Endpoint_t, Endpoint to close the communication on.
			///	\return \ref core::NET_Error
			NET_Error shutdown(const Endpoint p_direction);

			///	\brief Swaps this socket with another
			void swap(Net_Socket& p_other);

		private:
			Net_Socket(Net_Socket&& p_other)				= delete;
			Net_Socket(const Net_Socket&)					= delete;
			Net_Socket& operator = (const Net_Socket&)		= delete;
			Net_Socket& operator = (Net_Socket&& p_other)	= delete;
		};


		inline void Net_Socket::swap(Net_Socket& p_other) { std::swap(m_sock, p_other.m_sock); }

	} //namespace _p

} //namespace core

CORE_MAKE_ENUM_FLAG(::core::_p::Net_Socket::Endpoint)

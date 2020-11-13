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
///
///	\todo	Provide a comprehensive and consistent set of error codes, to give
///			extra information regarding he nature of the failure
//======== ======== ======== ======== ======== ======== ======== ========

#include <CoreLib/Core_Net.hpp>

#include <CoreLib/Core_Endian.hpp>
#include <CoreLib/Core_String.hpp>

#include <array>
#include <algorithm>
#include <deque>
#include <utility>

#ifdef _WIN32
#	include <Winsock2.h>
#	include <Ws2tcpip.h>
#	include <Iphlpapi.h>
#	include <Mswsock.h>
#	include <Mstcpip.h>
#else
#	include <string.h>
#	include <fcntl.h>
#	include <stdlib.h>
#	include <stdio.h>
#	include <netinet/tcp.h>
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <sys/ioctl.h>
#	include <arpa/inet.h>
#	include <errno.h>
#	include <unistd.h>
#	include <netinet/tcp.h>
#endif

#ifndef _WIN32
#	define INVALID_SOCKET	-1
#	define SOCKET_ERROR		-1
#endif

static constexpr uint16_t CORE_NET_MAX_DATA_LEN = 65507;

/// \n
namespace core
{

	//========	========	========	========	========
	//========			Bind and Join				========
	//========	========	========	========	========

	static inline NET_Error Core_BindIPv4(const SocketHandle_t p_sock, const uint32_t p_rawAddr, const uint16_t p_port)
	{
		sockaddr_in addr_info4;

		addr_info4.sin_family		= AF_INET;
		addr_info4.sin_addr.s_addr	= p_rawAddr;
		addr_info4.sin_port			= core::endian_host2big(p_port);

		if(bind(p_sock, reinterpret_cast<const sockaddr*>(&addr_info4), sizeof(sockaddr_in)) != 0)
		{
			return NET_Error::Sock_Bind;
		}

		return NET_Error::NoErr;
	}

	static inline NET_Error Core_BindIPv6(const SocketHandle_t p_sock, std::span<const uint8_t, 16> p_rawAddr, const uint16_t p_port)
	{
		sockaddr_in6 addr_info6{};
		memcpy(&addr_info6.sin6_addr, p_rawAddr.data(), 16);
		addr_info6.sin6_family	= AF_INET6;
		addr_info6.sin6_port	= core::endian_host2big(p_port);

		if(bind(p_sock, reinterpret_cast<const sockaddr*>(&addr_info6), sizeof(sockaddr_in6)) != 0)
		{
			return NET_Error::Sock_Bind;
		}

		return NET_Error::NoErr;
	}

	static inline NET_Error Core_JoinMulticastGroupIPv4(const SocketHandle_t p_sock, const uint32_t p_rawGroup, const uint32_t p_InterfaceAddr)
	{
		ip_mreq mreq;
		mreq.imr_multiaddr.s_addr = p_rawGroup;
		mreq.imr_interface.s_addr = p_InterfaceAddr;

		if(setsockopt(p_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<const char*>(&mreq), sizeof(ip_mreq)))
		{
			return NET_Error::Sock_Option;
		}
		return NET_Error::NoErr;
	}

	static inline NET_Error Core_JoinMulticastGroupIPv6(const SocketHandle_t p_sock, std::span<const uint8_t, 16> p_rawGroup, const uint32_t p_InterfaceNum)
	{
		ipv6_mreq mreq;
		memcpy(&mreq.ipv6mr_multiaddr, p_rawGroup.data(), 16);
		mreq.ipv6mr_interface = p_InterfaceNum;

		if(setsockopt(p_sock, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, reinterpret_cast<const char*>(&mreq), sizeof(ipv6_mreq)))
		{
			return NET_Error::Sock_Option;
		}
		return NET_Error::NoErr;
	}

	static inline NET_Error Core_LeaveMulticastGroupIPv4(const SocketHandle_t p_sock, const uint32_t p_rawGroup, const uint32_t p_InterfaceAddr)
	{
		ip_mreq mreq;
		mreq.imr_multiaddr.s_addr = p_rawGroup;
		mreq.imr_interface.s_addr = p_InterfaceAddr;

		if(setsockopt(p_sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, reinterpret_cast<const char*>(&mreq), sizeof(ip_mreq)))
		{
			return NET_Error::Sock_Option;
		}
		return NET_Error::NoErr;
	}

	static inline NET_Error Core_LeaveMulticastGroupIPv6(const SocketHandle_t p_sock, std::span<const uint8_t, 16> p_rawGroup, const uint32_t p_InterfaceNum)
	{
		ipv6_mreq mreq;
		memcpy(&mreq.ipv6mr_multiaddr, p_rawGroup.data(), 16);
		mreq.ipv6mr_interface = p_InterfaceNum;

		if(setsockopt(p_sock, IPPROTO_IPV6, IPV6_DROP_MEMBERSHIP, reinterpret_cast<const char*>(&mreq), sizeof(ipv6_mreq)))
		{
			return NET_Error::Sock_Option;
		}
		return NET_Error::NoErr;
	}

	//========	========	========	========	========
	//========			Common Check				========
	//========	========	========	========	========

	static inline NET_Error Core_Poll(const SocketHandle_t p_sock, const uint64_t p_microseconds)
	{
		fd_set fds;
		const uint64_t sec = p_microseconds / 1000000;
		if(sec > std::numeric_limits<decltype(timeval::tv_sec)>::max())
		{
			switch (select(static_cast<int>(p_sock + 1), &fds, nullptr, nullptr, nullptr))
			{
				case 0:
					return NET_Error::WouldBlock;
				case 1:
					return NET_Error::NoErr;
				default:
					break;
			}
		}
		else
		{
			struct timeval tv;
			tv.tv_sec	= static_cast<decltype(timeval::tv_sec)>(sec);
			tv.tv_usec	= p_microseconds % 1000000;
			FD_ZERO(&fds);
			FD_SET(p_sock, &fds);
			switch (select(static_cast<int>(p_sock + 1), &fds, nullptr, nullptr, &tv))
			{
				case 0:
					return NET_Error::WouldBlock;
				case 1:
					return NET_Error::NoErr;
				default:
					break;
			}
		}
		return NET_Error::Unknown;
	}

	static inline NET_Error Core_TCP_NonBlock_Connect_state(const SocketHandle_t p_sock)
	{
		fd_set fds;
		struct timeval tv;
		tv.tv_sec	= 0;
		tv.tv_usec	= 0;

		FD_ZERO(&fds);
		FD_SET(p_sock, &fds);

		if(select(static_cast<int>(p_sock + 1), nullptr, nullptr, &fds, &tv))
		{
			return NET_Error::Fail;
		}

		tv.tv_sec	= 0;
		tv.tv_usec	= 0;
		FD_ZERO(&fds);
		FD_SET(p_sock, &fds);

		switch(select(static_cast<int>(p_sock + 1), nullptr, &fds, nullptr, &tv))
		{
			case 0:
				return NET_Error::WouldBlock;
			case 1:
				return NET_Error::NoErr;
			default:
				break;
		}

		return NET_Error::Fail;
	}

#ifdef _WIN32
	
	typedef int CoreSockLen_t;


	//========	========	========	========	========
	//========	========	Error Check	========	========
	//========	========	========	========	========

	/// \brief Gets socket last error
	//
	static inline int SockLastError(const SocketHandle_t)
	{
		return WSAGetLastError();
	}

	/// \brief Used to check if last error on the socket was to a timeout on a non-blocking socket
	//
	static inline bool SockWouldBlock(const SocketHandle_t p_socket)
	{
		return (SockLastError(p_socket) == WSAEWOULDBLOCK);
	}

	/// \brief Used to check if last error on the socket was a buffer overflow
	//
	static inline bool SockBuffOverflow(const SocketHandle_t p_socket)
	{
		return (SockLastError(p_socket) == WSAEMSGSIZE);
	}

	static inline bool SockNonBlockingConnectCheck(const SocketHandle_t p_socket)
	{
		return SockLastError(p_socket) == WSAEWOULDBLOCK;
	}


	//========	========	========	========	========
	//========	========	Set options	========	========
	//========	========	========	========	========

	static inline NET_Error Core_setSockBlocking(const SocketHandle_t p_sock, const bool p_blocking)
	{
		u_long opt = p_blocking ? 0 : 1;
		return ioctlsocket(p_sock, FIONBIO, &opt) ? NET_Error::Sock_Option : NET_Error::NoErr;
	}

	static inline NET_Error Core_setSockLinger(const SocketHandle_t p_sock, const bool p_linger, const uint16_t p_timeOut)
	{
		linger opt;

		opt.l_onoff		= p_linger ? 1: 0;
		opt.l_linger	= p_timeOut;

		return setsockopt(p_sock, SOL_SOCKET, SO_LINGER, reinterpret_cast<const char*>(&opt), sizeof(opt)) ? NET_Error::Sock_Option : NET_Error::NoErr;
	}

	static inline NET_Error Core_setBrodCasting(const SocketHandle_t p_sock, const bool p_broadcast)
	{
		BOOL opt = p_broadcast ? TRUE : FALSE;
		return setsockopt(p_sock, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<const char*>(&opt), sizeof(opt)) ? NET_Error::Sock_Option : NET_Error::NoErr;
	}

	static inline NET_Error Core_setReuseAddress(const SocketHandle_t p_sock, const bool p_reuse)
	{
		BOOL opt = p_reuse ? TRUE : FALSE;
		return setsockopt(p_sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt)) ? NET_Error::Sock_Option : NET_Error::NoErr;
	}

	static inline NET_Error Core_setNagle(const SocketHandle_t p_sock, const bool p_nagle)
	{
		BOOL opt = p_nagle ? FALSE : TRUE;
		return setsockopt(p_sock, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&opt), sizeof(opt)) ? NET_Error::Sock_Option : NET_Error::NoErr;
	}

	static inline NET_Error Core_SetKeepAlive(const SocketHandle_t p_sock, const bool p_keepAlive, const uint32_t p_probePeriod, const uint32_t p_maxProbes)
	{
		if(p_keepAlive)
		{
			if((p_probePeriod == 0) ||
				(p_maxProbes < 1) ||
				(static_cast<uint64_t>(p_maxProbes) * static_cast<uint64_t>(p_probePeriod)) > 9000)
			{
				return NET_Error::Invalid_Option;
			}

			tcp_keepalive opt;
			opt.onoff				= true;
			opt.keepaliveinterval	= p_probePeriod * 1000;
			opt.keepalivetime		= p_maxProbes * p_probePeriod * 1000;

			int res = WSAIoctl(p_sock, SIO_KEEPALIVE_VALS, &opt, sizeof(opt), nullptr, 0, nullptr, nullptr, nullptr);

			switch(res)
			{
				case 0:
				case WSA_IO_PENDING:	//Should this be supported?
					return NET_Error::NoErr;
				default:
					break;
			}
			return NET_Error::Sock_Option;
		}

		DWORD opt = 0;
		return setsockopt(p_sock, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char*>(&opt), sizeof(opt)) ? NET_Error::Sock_Option : NET_Error::NoErr;
	}

	//========	========	========	========	========
	//========			Open and Close				========
	//========	========	========	========	========

	static inline int Core_CloseSock(const SocketHandle_t p_sock)
	{
		return closesocket(p_sock);
	}

	static inline NET_Error Core_createSocket(SocketHandle_t& p_sock, const int p_af, const int p_domain, const int p_protocol, const bool p_blocking)
	{
		//not sure why WSA_FLAG_OVERLAPPED was added originally but I don't think it is needed
		//I will leave it just in case
		//p_sock = WSASocketW(p_af, p_domain, p_protocol, nullptr, 0, WSA_FLAG_OVERLAPPED | WSA_FLAG_NO_HANDLE_INHERIT);
		p_sock = WSASocketW(p_af, p_domain, p_protocol, nullptr, 0, WSA_FLAG_NO_HANDLE_INHERIT);
		if(p_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;

		u_long b = p_blocking ? 0 : 1;
		if(ioctlsocket(p_sock, FIONBIO, &b) != 0)
		{
			Core_CloseSock(p_sock);
			p_sock = INVALID_SOCKET;
			return NET_Error::Sock_Option;
		}

		return NET_Error::NoErr;
	}

	static inline NET_Error Core_CreateUDPSocketIPv4(SocketHandle_t& p_sock, const bool p_blocking)
	{
		NET_Error err = Core_createSocket(p_sock, AF_INET, SOCK_DGRAM, IPPROTO_UDP, p_blocking);
		if(err != NET_Error::NoErr) return err;

		//disables recieving UDP connection reset messages as regular messages
		BOOL	bnewbehavior = FALSE;
		DWORD	tempGarb;
		if(WSAIoctl(p_sock, SIO_UDP_CONNRESET ,&bnewbehavior, sizeof(bnewbehavior), nullptr, 0, &tempGarb, nullptr, nullptr) != 0)
		{
			Core_CloseSock(p_sock);
			p_sock = INVALID_SOCKET;
			return NET_Error::Sock_Option;
		}
		return NET_Error::NoErr;
	}

	static inline NET_Error Core_CreateUDPSocketIPv6(SocketHandle_t& p_sock, const bool p_blocking)
	{
		NET_Error err = Core_createSocket(p_sock, AF_INET6, SOCK_DGRAM, IPPROTO_UDP, p_blocking);
		if(err != NET_Error::NoErr) return err;

		//disables recieving UDP connection reset messages as regular messages
		BOOL	bnewbehavior = FALSE;
		DWORD	tempGarb;
		if(WSAIoctl(p_sock, SIO_UDP_CONNRESET ,&bnewbehavior, sizeof(bnewbehavior), nullptr, 0, &tempGarb, nullptr, nullptr) != 0)
		{
			Core_CloseSock(p_sock);
			p_sock = INVALID_SOCKET;
			return NET_Error::Sock_Option;
		}
		return NET_Error::NoErr;
	}

	static inline NET_Error Core_Shutdown(const SocketHandle_t p_sock, const core_p::Net_Socket::Endpoint p_direction)
	{
		//this optimization is only aforder because documentation states that Read=0, Write=1 amd ReadWrite=2
		static_assert(core_p::Net_Socket::Endpoint::Receive	== core_p::Net_Socket::Endpoint{1});
		static_assert(core_p::Net_Socket::Endpoint::Send	== core_p::Net_Socket::Endpoint{2});
		static_assert(core_p::Net_Socket::Endpoint::Both	== core_p::Net_Socket::Endpoint{3});
		return shutdown(p_sock, static_cast<uint8_t>(p_direction) - 1) ? NET_Error::Fail : NET_Error::NoErr;
	}

#if 0
	static inline NET_Error Core_Shutdown(const SocketHandle_t p_sock, const core_p::Net_Socket::Endpoint p_direction)
	{
		switch(p_direction)
		{
			case core_p::Net_Socket::Endpoint_Receive:
				return shutdown(p_sock, SD_RECEIVE) ? NET_Error::Fail : NET_Error::NoErr;
			case core_p::Net_Socket::Endpoint_Send:
				return shutdown(p_sock, SD_SEND) ? NET_Error::Fail : NET_Error::NoErr;
			case core_p::Net_Socket::Endpoint_Both:
				return shutdown(p_sock, SD_BOTH) ? NET_Error::Fail : NET_Error::NoErr;
			default:
				break;
		}
		return NET_Error::Invalid_Option; 
	}
#endif

	//========	========	========	========	========
	//========	========	Peek		========	========
	//========	========	========	========	========


	static inline NET_Error Core_PeekSize(const SocketHandle_t p_sock, uintptr_t& p_size)
	{
		u_long	size = 0;
		char	c[1];
		int		check;

		check = recvfrom(p_sock, c, 0, MSG_PEEK, nullptr, nullptr);

		if(check == SOCKET_ERROR)
		{
			check = SockLastError(p_sock);

			if(check == WSAEWOULDBLOCK)	return NET_Error::WouldBlock;
			if(check != WSAEMSGSIZE)	return NET_Error::Connection;
		}

		ioctlsocket(p_sock, FIONREAD, &size);

		p_size = size;
		return NET_Error::NoErr;
	}

	static inline NET_Error Core_PeekSizeIPv4(const SocketHandle_t p_sock, uintptr_t& p_size, uint32_t& p_rawAddr, uint16_t& p_port)
	{
		int			addr_size;
		u_long		size = 0;
		char		c[1];
		int			check;
		sockaddr_in	addr_info4;

		addr_size	= sizeof(sockaddr_in);
		check		= recvfrom(p_sock, c, 0, MSG_PEEK, reinterpret_cast<sockaddr*>(&addr_info4), &addr_size);

		if(check == SOCKET_ERROR)
		{
			check = SockLastError(p_sock);
			if(check == WSAEWOULDBLOCK)	return NET_Error::WouldBlock;
			if(check != WSAEMSGSIZE)	return NET_Error::Connection;
		}

		ioctlsocket(p_sock, FIONREAD, &size);

		p_size		= size;
		p_rawAddr	= addr_info4.sin_addr.s_addr;
		p_port		= core::endian_big2host(addr_info4.sin_port);

		return NET_Error::NoErr;
	}

	static inline NET_Error Core_PeekSizeIPv6(const SocketHandle_t p_sock, uintptr_t& p_size, void* p_rawAddr, uint16_t& p_port)
	{
		int		addr_size;
		u_long	size = 0;
		char			c[1];
		int				check;
		sockaddr_in6	addr_info6;

		addr_size	= sizeof(sockaddr_in6);
		check		= recvfrom(p_sock, c, 0, MSG_PEEK, reinterpret_cast<sockaddr*>(&addr_info6), &addr_size);

		if(check == SOCKET_ERROR)
		{
			check = SockLastError(p_sock);
			if(check == WSAEWOULDBLOCK)	return NET_Error::WouldBlock;
			if(check != WSAEMSGSIZE)	return NET_Error::Connection;
		}

		ioctlsocket(p_sock, FIONREAD, &size);
		p_size = size;

		memcpy(p_rawAddr, &addr_info6.sin6_addr, 16);
		p_port = core::endian_big2host(addr_info6.sin6_port);

		return NET_Error::NoErr;
	}

#else //OS

	typedef socklen_t CoreSockLen_t;


	//========	========	========	========	========
	//========	========	Error Check	========	========
	//========	========	========	========	========

	/// \brief Gets socket last error
	//
	static inline int SockLastError(const SocketHandle_t p_socket)
	{
		int error;
		socklen_t len;
		len = 4;
		getsockopt(p_socket, SOL_SOCKET, SO_ERROR, reinterpret_cast<void*>(&error), &len);
		return error;
	}

	/// \brief Used to check if last error on the socket was to a timeout on a non-blocking socket
	//
	static bool SockWouldBlock(const SocketHandle_t p_socket)
	{
		int error;
		socklen_t len;
		len = 4;
		if(getsockopt(p_socket, SOL_SOCKET, SO_ERROR, reinterpret_cast<void*>(&error), &len) != 0) return false;
		return (error == EAGAIN || error == EWOULDBLOCK);
	}

	static inline bool SockNonBlockingConnectCheck(const SocketHandle_t p_socket)
	{
		int error;
		socklen_t len;
		len = 4;
		getsockopt(p_socket, SOL_SOCKET, SO_ERROR, reinterpret_cast<void*>(&error), &len);
		return error == EINPROGRESS;
	}


	//========	========	========	========	========
	//========	========	Set options	========	========
	//========	========	========	========	========

	static inline NET_Error Core_setSockBlocking(const SocketHandle_t p_sock, const bool blocking)
	{
		int b = blocking ? 0 : 1;
		return ioctl(p_sock, FIONBIO, &b) ? NET_Error::Sock_Option : NET_Error::NoErr;
	}

	static inline NET_Error Core_setSockLinger(const SocketHandle_t p_sock, const bool p_linger, const uint16_t p_timeOut)
	{
		linger opt;

		opt.l_onoff		= p_linger ? 1: 0;
		opt.l_linger	= p_timeOut;

		return setsockopt(p_sock, SOL_SOCKET, SO_LINGER, reinterpret_cast<const void*>(&opt), sizeof(opt)) ? NET_Error::Sock_Option : NET_Error::NoErr;
	}

	static inline NET_Error Core_setBrodCasting(const SocketHandle_t p_sock, const bool broadcast)
	{
		int h = broadcast ? 1 : 0;
		return setsockopt(p_sock, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<const void*>(&h), sizeof(h)) ? NET_Error::Sock_Option : NET_Error::NoErr;
	}

	static inline NET_Error Core_setReuseAddress(const SocketHandle_t p_sock, const bool p_reuse)
	{
		int opt = p_reuse ? 1 : 0;
		return setsockopt(p_sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const void*>(&opt), sizeof(opt)) ? NET_Error::Sock_Option : NET_Error::NoErr;
	}

	static inline NET_Error Core_setNagle(const SocketHandle_t p_sock, const bool p_nagle)
	{
		int opt = p_nagle ? 0 : 1;
		return setsockopt(p_sock, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const void*>(&opt), sizeof(opt)) ? NET_Error::Sock_Option : NET_Error::NoErr;
	}

	static inline NET_Error Core_SetKeepAlive(const SocketHandle_t p_sock, const bool p_keepAlive, const uint32_t p_probePeriod, const uint32_t p_maxProbes)
	{
		if(p_keepAlive)
		{
			if((p_probePeriod == 0) ||
				(p_maxProbes < 1) ||
				(static_cast<uint64_t>(p_maxProbes) * static_cast<uint64_t>(p_probePeriod)) > 9000)
			{
				return NET_Error::Invalid_Option;
			}

			int probes		= static_cast<int>(p_maxProbes);
			int period		= static_cast<int>(p_probePeriod);
			int firstDelay	= period;
			int keepAlive	= 1;

			if(	setsockopt(p_sock, IPPROTO_TCP, TCP_KEEPCNT,   &probes,     sizeof(probes)    ) ||
				setsockopt(p_sock, IPPROTO_TCP, TCP_KEEPIDLE,  &firstDelay, sizeof(firstDelay)) ||
				setsockopt(p_sock, IPPROTO_TCP, TCP_KEEPINTVL, &period,     sizeof(period)    ) ||
				setsockopt(p_sock, SOL_SOCKET,  SO_KEEPALIVE,  &keepAlive,  sizeof(keepAlive) ) )
			{
				return NET_Error::Sock_Option;
			}

			return NET_Error::NoErr;
		}

		int opt = 0;
		return setsockopt(p_sock, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)) ? NET_Error::Sock_Option : NET_Error::NoErr;

	}

	//========	========	========	========	========
	//========			Open and Close				========
	//========	========	========	========	========

	static inline int Core_CloseSock(const SocketHandle_t p_sock)
	{
		return close(p_sock);
	}

	static inline NET_Error Core_createSocket(SocketHandle_t& p_sock, const int p_af, const int p_domain, const int p_protocol, const bool p_blocking)
	{
		p_sock = socket(p_af, p_domain | (p_blocking ? SOCK_CLOEXEC : SOCK_CLOEXEC | SOCK_NONBLOCK), p_protocol);
		if(p_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return NET_Error::NoErr;
	}

	static inline NET_Error Core_CreateUDPSocketIPv4(SocketHandle_t& p_sock, const bool p_blocking)
	{
		return Core_createSocket(p_sock, AF_INET, SOCK_DGRAM, IPPROTO_UDP, p_blocking);
	}

	static inline NET_Error Core_CreateUDPSocketIPv6(SocketHandle_t& p_sock, const bool p_blocking)
	{
		return Core_createSocket(p_sock, AF_INET6, SOCK_DGRAM, IPPROTO_UDP, p_blocking);
	}

	static inline NET_Error Core_Shutdown(const SocketHandle_t p_sock, const core_p::Net_Socket::Endpoint p_direction)
	{
		//this optimization is only aforder because documentation states that Read=0, Write=1 amd ReadWrite=2
		static_assert(core_p::Net_Socket::Endpoint::Receive	== core_p::Net_Socket::Endpoint{1});
		static_assert(core_p::Net_Socket::Endpoint::Send	== core_p::Net_Socket::Endpoint{2});
		static_assert(core_p::Net_Socket::Endpoint::Both	== core_p::Net_Socket::Endpoint{3});
		return shutdown(p_sock, static_cast<uint8_t>(p_direction) - 1) ? NET_Error::Fail : NET_Error::NoErr;
	}

#if 0
	static inline NET_Error Core_Shutdown(const SocketHandle_t p_sock, const core_p::Net_Socket::Endpoint p_direction)
	{
		switch(p_direction)
		{
			case core_p::Net_Socket::Endpoint_Receive:
				return shutdown(p_sock, SHUT_RD) ? NET_Error::Fail : NET_Error::NoErr;
			case core_p::Net_Socket::Endpoint_Send:
				return shutdown(p_sock, SHUT_WR) ? NET_Error::Fail : NET_Error::NoErr;
			case core_p::Net_Socket::Endpoint_Both:
				return shutdown(p_sock, SHUT_RDWR) ? NET_Error::Fail : NET_Error::NoErr;
			default:
				break;
		}
		return NET_Error::Invalid_Option; 
	}
#endif

	//========	========	========	========	========
	//========	========	Peek		========	========
	//========	========	========	========	========


	static inline NET_Error Core_PeekSize(const SocketHandle_t p_sock, uintptr_t& p_size)
	{
		int		size = 0;
		char	c[1];
		ssize_t	check;

		check = recvfrom(p_sock, c, 0, MSG_PEEK, nullptr, nullptr);

		if(check == SOCKET_ERROR)
		{
			if(SockWouldBlock(p_sock))	return NET_Error::WouldBlock;
			return NET_Error::Connection;
		}

		ioctl(p_sock, FIONREAD, &size);
		p_size = static_cast<uint32_t>(size);
		return NET_Error::NoErr;
	}

	static inline NET_Error Core_PeekSizeIPv4(const SocketHandle_t p_sock, uintptr_t& p_size, uint32_t& p_rawAddr, uint16_t& p_port)
	{
		socklen_t	addr_size;
		int			size = 0;
		char		c[1];
		ssize_t		check;
		sockaddr_in	addr_info4;

		addr_size	= sizeof(sockaddr_in);
		check		= recvfrom(p_sock, c, 0, MSG_PEEK, reinterpret_cast<sockaddr*>(&addr_info4), &addr_size);

		if(check == SOCKET_ERROR)
		{
			if(SockWouldBlock(p_sock))	return NET_Error::WouldBlock;
			return NET_Error::Connection;
		}

		ioctl(p_sock, FIONREAD, &size);

		p_size		= size;
		p_rawAddr	= addr_info4.sin_addr.s_addr;
		p_port		= core::endian_big2host(addr_info4.sin_port);

		return NET_Error::NoErr;
	}

	static inline NET_Error Core_PeekSizeIPv6(const SocketHandle_t p_sock, uintptr_t& p_size, void* p_rawAddr, uint16_t& p_port)
	{
		socklen_t		addr_size;
		int				size = 0;
		char			c[1];
		ssize_t			check;
		sockaddr_in6	addr_info6;

		addr_size	= sizeof(sockaddr_in6);
		check		= recvfrom(p_sock, c, 0, MSG_PEEK, reinterpret_cast<sockaddr*>(&addr_info6), &addr_size);

		if(check == SOCKET_ERROR)
		{
			if(SockWouldBlock(p_sock))	return NET_Error::WouldBlock;
			return NET_Error::Connection;
		}

		ioctl(p_sock, FIONREAD, &size);

		p_size = size;
		memcpy(&p_rawAddr, &addr_info6.sin6_addr, 16);
		p_port = core::endian_big2host(addr_info6.sin6_port);

		return NET_Error::NoErr;
	}

#endif

	static inline NET_Error Core_CreateTCPSocketIPv4(SocketHandle_t& p_sock, const bool p_blocking)
	{
		return Core_createSocket(p_sock, AF_INET, SOCK_STREAM, IPPROTO_TCP, p_blocking);
	}

	static inline NET_Error Core_CreateTCPSocketIPv6(SocketHandle_t& p_sock, const bool p_blocking)
	{
		return Core_createSocket(p_sock, AF_INET6, SOCK_STREAM, IPPROTO_TCP, p_blocking);
	}

	//========	========	========	========	========
	//========	========	Get Address	========	========
	//========	========	========	========	========

	static inline NET_Error Core_GetAddressIPv4(const SocketHandle_t p_sock, uint32_t& p_rawAddr, uint16_t& p_port)
	{
		sockaddr_in		addr_info4;
		CoreSockLen_t	len;

		len = sizeof(addr_info4);
		if(getsockname(p_sock, reinterpret_cast<sockaddr*>(&addr_info4), &len) || len != sizeof(sockaddr_in))
		{
			return NET_Error::Fail;
		}

		p_rawAddr	= addr_info4.sin_addr.s_addr;
		p_port		= core::endian_host2big(addr_info4.sin_port);

		return NET_Error::NoErr;
	}

	static inline NET_Error Core_GetAddressIPv6(const SocketHandle_t p_sock, void* p_rawGroup, uint16_t& p_port)
	{
		sockaddr_in6	addr_info6;
		CoreSockLen_t	len;

		len = sizeof(addr_info6);
		if(getsockname(p_sock, reinterpret_cast<sockaddr*>(&addr_info6), &len) || len != sizeof(sockaddr_in6))
		{
			return NET_Error::Fail;
		}

		memcpy(p_rawGroup, &addr_info6.sin6_addr, 16);
		p_port = core::endian_host2big(addr_info6.sin6_port);

		return NET_Error::NoErr;
	}

	static inline NET_Error Core_GetPeerAddressIPv4(const SocketHandle_t p_sock, uint32_t& p_rawAddr, uint16_t& p_port)
	{
		sockaddr_in addr_info4;
		CoreSockLen_t len;

		len = sizeof(addr_info4);
		if(getpeername(p_sock, reinterpret_cast<sockaddr*>(&addr_info4), &len) || len != sizeof(sockaddr_in))
		{
			return NET_Error::Fail;
		}

		p_rawAddr	= addr_info4.sin_addr.s_addr;
		p_port		= core::endian_host2big(addr_info4.sin_port);

		return NET_Error::NoErr;
	}

	static inline NET_Error Core_GetPeerAddressIPv6(const SocketHandle_t p_sock, void* p_rawGroup, uint16_t& p_port)
	{
		sockaddr_in6 addr_info6;
		CoreSockLen_t len;

		len = sizeof(addr_info6);
		if(getpeername(p_sock, reinterpret_cast<sockaddr*>(&addr_info6), &len) || len != sizeof(sockaddr_in6))
		{
			return NET_Error::Fail;
		}

		memcpy(p_rawGroup, &addr_info6.sin6_addr, 16);
		p_port = core::endian_host2big(addr_info6.sin6_port);

		return NET_Error::NoErr;
	}

	//========	========	========	========	========
	//========			Connect and Accept			========
	//========	========	========	========	========

	static inline NET_Error Core_ConnectIPv4(const SocketHandle_t p_sock, const uint32_t p_rawAddr, const uint16_t p_port)
	{
		sockaddr_in o_addr_info4;
		o_addr_info4.sin_family			= AF_INET;
		o_addr_info4.sin_addr.s_addr	= p_rawAddr;
		o_addr_info4.sin_port			= core::endian_host2big(p_port);
		if(connect(p_sock, reinterpret_cast<const sockaddr*>(&o_addr_info4), sizeof(sockaddr_in)) != 0)
		{
			if(SockNonBlockingConnectCheck(p_sock)) return NET_Error::WouldBlock;
			return NET_Error::Connection;
		}
		return NET_Error::NoErr;
	}

	static inline NET_Error Core_ConnectIPv6(const SocketHandle_t p_sock, std::span<const uint8_t, 16> p_rawAddr, const uint16_t p_port)
	{
		sockaddr_in6 o_addr_info6{};
		memcpy(&o_addr_info6.sin6_addr, p_rawAddr.data(), 16);
		o_addr_info6.sin6_family	= AF_INET6;
		o_addr_info6.sin6_port		= core::endian_host2big(p_port);

		if(connect(p_sock, reinterpret_cast<const sockaddr*>(&o_addr_info6), sizeof(sockaddr_in6)) != 0)
		{
			if(SockNonBlockingConnectCheck(p_sock)) return NET_Error::WouldBlock;
			return NET_Error::Connection;
		}
		return NET_Error::NoErr;
	}

	static inline NET_Error Core_AcceptIPv4(const SocketHandle_t p_sock, SocketHandle_t& p_connection, const bool p_blocking, sockaddr_in& p_info)
	{
		SocketHandle_t sock;
		CoreSockLen_t addr_size = sizeof(sockaddr_in);

#ifdef _WIN32
		sock = accept(p_sock, reinterpret_cast<sockaddr*>(&p_info), &addr_size);
#else
		sock = accept4(p_sock, reinterpret_cast<sockaddr*>(&p_info), &addr_size,  p_blocking ? SOCK_CLOEXEC : SOCK_CLOEXEC | SOCK_NONBLOCK);
#endif

		if(sock == INVALID_SOCKET)
		{
			if(SockWouldBlock(p_sock)) return NET_Error::WouldBlock;
			return NET_Error::Connection;
		}

#ifdef _WIN32
		u_long b = p_blocking ? 0 : 1;
		if(ioctlsocket(sock, FIONBIO, &b) != 0)
		{
			Core_CloseSock(sock);
			return NET_Error::Sock_Option;
		}
#endif

		p_connection = sock;
		return NET_Error::NoErr;
	}

	static inline NET_Error Core_AcceptIPv6(const SocketHandle_t p_sock, SocketHandle_t& p_connection, const bool p_blocking, sockaddr_in6& p_info)
	{
		SocketHandle_t sock;
		CoreSockLen_t addr_size = sizeof(sockaddr_in6);

#ifdef _WIN32
		sock = accept(p_sock, reinterpret_cast<sockaddr*>(&p_info), &addr_size);
#else
		sock = accept4(p_sock, reinterpret_cast<sockaddr*>(&p_info), &addr_size, p_blocking ? SOCK_CLOEXEC : SOCK_CLOEXEC | SOCK_NONBLOCK);
#endif

		if(sock == INVALID_SOCKET)
		{
			if(SockWouldBlock(p_sock)) return NET_Error::WouldBlock;
			return NET_Error::Connection;
		}

#ifdef _WIN32
		u_long b = p_blocking ? 0 : 1;
		if(ioctlsocket(sock, FIONBIO, &b) != 0)
		{
			Core_CloseSock(sock);
			return NET_Error::Sock_Option;
		}
#endif

		p_connection = sock;
		return NET_Error::NoErr;
	}


	//========	========	========	========	========
	//========			Send and Receive			========
	//========	========	========	========	========

	static inline NET_Error Core_Send_size(const SocketHandle_t p_sock, const void* p_buffer, uintptr_t p_size, uintptr_t& p_sent)
	{
#ifdef _WIN32
		if(p_size > std::numeric_limits<int>::max()) p_size = std::numeric_limits<int>::max();
		int ret = send(p_sock, reinterpret_cast<const char*>(p_buffer), static_cast<int>(p_size), 0);
#else
		intptr_t ret = send(p_sock, reinterpret_cast<const char*>(p_buffer), p_size, 0);
#endif
		if(ret == SOCKET_ERROR)
		{
			if(SockWouldBlock(p_sock))
			{
				return NET_Error::WouldBlock;
			}
			return NET_Error::Connection;
		}
		p_sent = ret;
		return NET_Error::NoErr;
	}

	static inline NET_Error Core_Send_context(const SocketHandle_t p_sock, const void* p_buffer, const uintptr_t p_size, uintptr_t& p_context)
	{
		if(p_context >= p_size) return NET_Error::Invalid_Option;

		uintptr_t currentSize = p_size - p_context;

#ifdef _WIN32
		if(currentSize > std::numeric_limits<int>::max()) currentSize = std::numeric_limits<int>::max();
		int ret = send(p_sock, reinterpret_cast<const char*>(p_buffer) + p_context, static_cast<int>(currentSize), 0);
#else
		intptr_t ret = send(p_sock, reinterpret_cast<const char*>(p_buffer) + p_context, currentSize, 0);
#endif
		if(ret == SOCKET_ERROR)
		{
			if(SockWouldBlock(p_sock))
			{
				return NET_Error::WouldBlock;
			}
			return NET_Error::Connection;
		}
		if(ret + p_context == p_size)
		{
			p_context = 0;
		}
		else
		{
			p_context += ret;
		}
		return NET_Error::NoErr;
	}

	static inline NET_Error Core_Receive_size(const SocketHandle_t p_sock, void* p_buffer, uintptr_t p_size, uintptr_t& p_received)
	{
#ifdef _WIN32
		if(p_size > std::numeric_limits<int>::max()) p_size = std::numeric_limits<int>::max();
		int ret = recv(p_sock, reinterpret_cast<char*>(p_buffer), static_cast<int>(p_size), 0);
#else
		intptr_t ret = recv(p_sock, reinterpret_cast<char*>(p_buffer), p_size, 0);
#endif
		if(ret == SOCKET_ERROR)
		{
			if(SockWouldBlock(p_sock))
			{
				return NET_Error::WouldBlock;
			}
		}
		else if(ret > 0)
		{
			p_received = ret;
			return NET_Error::NoErr;
		}
		else if(ret == 0)
		{
			return NET_Error::TCP_GracefullClose;
		}
		return NET_Error::Connection;
	}

	static inline NET_Error Core_Receive_context(const SocketHandle_t p_sock, void* p_buffer, const uintptr_t p_size, uintptr_t& p_context)
	{
		if(p_context >= p_size) return NET_Error::Invalid_Option;

		uintptr_t currentSize = p_size - p_context;
#ifdef _WIN32
		if(currentSize > std::numeric_limits<int>::max()) currentSize = std::numeric_limits<int>::max();
		int ret = recv(p_sock, reinterpret_cast<char*>(p_buffer) + p_context, static_cast<int>(currentSize), 0);
#else
		intptr_t ret = recv(p_sock, reinterpret_cast<char*>(p_buffer) + p_context, currentSize, 0);
#endif
		if(ret == SOCKET_ERROR)
		{
			if(SockWouldBlock(p_sock))
			{
				return NET_Error::WouldBlock;
			}
		}
		else if(ret > 0)
		{
			if(ret + p_context == p_size)
			{
				p_context = 0;
			}
			else
			{
				p_context += ret;
			}
			return NET_Error::NoErr;
		}
		else if(ret == 0)
		{
			return NET_Error::TCP_GracefullClose;
		}

		return NET_Error::Connection;
	}

	static inline NET_Error Core_SendToIPv4(const SocketHandle_t p_sock, const void* p_data, uintptr_t p_size, const uint32_t p_rawAddr, const uint16_t p_port, const uint8_t p_repeat)
	{
		uint8_t		count = 0;
		sockaddr_in	addr_info4;
		bool		sentOnce = false;

#ifdef _WIN32
		if(p_size > std::numeric_limits<int>::max()) return NET_Error::Buffer_Full;
#endif

		addr_info4.sin_family		= AF_INET;
		addr_info4.sin_addr.s_addr	= p_rawAddr;
		addr_info4.sin_port			= core::endian_host2big(p_port);

		do
		{
#ifdef _WIN32
			if(sendto(p_sock, reinterpret_cast<const char*>(p_data), static_cast<int>(p_size), 0, reinterpret_cast<sockaddr*>(&addr_info4), sizeof(addr_info4)) == SOCKET_ERROR)
#else
			if(sendto(p_sock, reinterpret_cast<const char*>(p_data), p_size, 0, reinterpret_cast<sockaddr*>(&addr_info4), sizeof(addr_info4)) == SOCKET_ERROR)
#endif
			{
				if(SockWouldBlock(p_sock))
				{
					if(count != 0) break;
					return NET_Error::WouldBlock;
				}
			}
			else sentOnce = true;
			++count;
		}
		while(p_repeat > count);
		return sentOnce ? NET_Error::NoErr : NET_Error::Unknown;
	}

	static inline NET_Error Core_SendToIPv6(const SocketHandle_t p_sock, const void* p_data, uintptr_t p_size, std::span<const uint8_t, 16> p_rawAddr, const uint16_t p_port, const uint8_t p_repeat)
	{
		uint8_t			count = 0;
		sockaddr_in6	addr_info6{};
		bool			sentOnce = false;

#ifdef _WIN32
		if(p_size > std::numeric_limits<int>::max()) return NET_Error::Buffer_Full;
#endif

		memcpy(&addr_info6.sin6_addr, p_rawAddr.data(), 16);
		addr_info6.sin6_family	= AF_INET6;
		addr_info6.sin6_port	= endian_host2big(p_port);

		do
		{
#ifdef _WIN32
			if(sendto(p_sock, reinterpret_cast<const char*>(p_data), static_cast<int>(p_size), 0, reinterpret_cast<const sockaddr*>(&addr_info6), sizeof(addr_info6)) == SOCKET_ERROR)
#else
			if(sendto(p_sock, reinterpret_cast<const char*>(p_data), p_size, 0, reinterpret_cast<const sockaddr*>(&addr_info6), sizeof(addr_info6)) == SOCKET_ERROR)
#endif
			{
				if(SockWouldBlock(p_sock))
				{
					if(count != 0) break;
					return NET_Error::WouldBlock;
				}
			}
			else sentOnce = true;
			++count;
		}
		while(p_repeat > count);
		return sentOnce ? NET_Error::NoErr : NET_Error::Unknown;

	}

	static inline NET_Error Core_ReceiveFrom(const SocketHandle_t p_sock, void* p_data, uintptr_t& p_size)
	{
#ifdef _WIN32
		uintptr_t size = p_size;
		if(size > std::numeric_limits<int>::max()) size = std::numeric_limits<int>::max();
		int check = recvfrom(p_sock, reinterpret_cast<char*>(p_data), static_cast<int>(size), 0, nullptr, nullptr);
#else
		intptr_t check = recvfrom(p_sock, reinterpret_cast<char*>(p_data), p_size, 0, nullptr, nullptr);
#endif


		if(SockWouldBlock(p_sock)) return NET_Error::WouldBlock;

		if(check == SOCKET_ERROR)
		{
			return NET_Error::Connection;
		}
		p_size = check;
		return NET_Error::NoErr;
	}

	static inline NET_Error Core_ReceiveFromIPv4(const SocketHandle_t p_sock, void* p_data, uintptr_t& p_size, uint32_t& p_rawAddr, uint16_t& p_port)
	{
		sockaddr_in		addr_info4{};
		CoreSockLen_t	addr_size = sizeof(sockaddr_in);

#ifdef _WIN32
		uintptr_t size = p_size;
		if(size > std::numeric_limits<int32_t>::max()) size = std::numeric_limits<int32_t>::max();
		int check = recvfrom(p_sock, reinterpret_cast<char*>(p_data), static_cast<int>(size), 0, reinterpret_cast<sockaddr*>(&addr_info4), &addr_size);
#else
		intptr_t check = recvfrom(p_sock, reinterpret_cast<char*>(p_data), p_size, 0, reinterpret_cast<sockaddr*>(&addr_info4), &addr_size);
#endif
		if(SockWouldBlock(p_sock)) return NET_Error::WouldBlock;

		if(check == SOCKET_ERROR)
		{
			return NET_Error::Connection;
		}

		p_rawAddr	= addr_info4.sin_addr.s_addr;
		p_size		= check;
		p_port		= core::endian_big2host(addr_info4.sin_port);

		return NET_Error::NoErr;
	}

	static inline NET_Error Core_ReceiveFromIPv6(const SocketHandle_t p_sock, void* p_data, uintptr_t& p_size, void* p_rawAddr, uint16_t& p_port)
	{
		sockaddr_in6	addr_info6{};
		CoreSockLen_t	addr_size = sizeof(sockaddr_in6);

#ifdef _WIN32
		uintptr_t size = p_size;
		if(size > std::numeric_limits<int>::max()) size = std::numeric_limits<int>::max();
		int check = recvfrom(p_sock, reinterpret_cast<char*>(p_data), static_cast<int>(size), 0, reinterpret_cast<sockaddr*>(&addr_info6), &addr_size);
#else
		intptr_t check = recvfrom(p_sock, reinterpret_cast<char*>(p_data), p_size, 0, reinterpret_cast<sockaddr*>(&addr_info6), &addr_size);
#endif
		if(SockWouldBlock(p_sock)) return NET_Error::WouldBlock;

		if(check == SOCKET_ERROR)
		{
			return NET_Error::Connection;
		}

		p_size = check;

		memcpy(p_rawAddr, &addr_info6.sin6_addr, 16);
		p_port = core::endian_big2host(addr_info6.sin6_port);

		return NET_Error::NoErr;
	}


	//========	========	========	========	========
	//========	========	Wake on LAN	========	========
	//========	========	========	========	========

	static inline void Core_PrepareWOLPacket(std::span<uint8_t, 102> p_payload, std::span<const uint8_t, 6> p_MacAddress)
	{
		uint8_t* payload = p_payload.data();
		uint8_t* pivot = payload + 6;
		memset(payload, 0xFF, 6);
		memcpy(payload + (6 * 1), p_MacAddress.data(), 6);
		memcpy(payload + (6 * 2), pivot, 6);
		memcpy(payload + (6 * 3), pivot, 12);
		memcpy(payload + (6 * 5), pivot, 24);
		memcpy(payload + (6 * 9), pivot, 48);
	}

	static inline NET_Error Core_WakeOnLanIPv4(const SocketHandle_t p_sock, std::span<const uint8_t, 6> p_MacAddress, const uint32_t p_rawAddr, const uint16_t* p_port)
	{
		uint8_t				payload[102];
		Core_PrepareWOLPacket(payload, p_MacAddress);

		if(p_port)
		{
			return Core_SendToIPv4(p_sock, payload, 102, p_rawAddr, *p_port, 0);
		}

		NET_Error ret = Core_SendToIPv4(p_sock, payload, 102, p_rawAddr, 7, 0);
		if(ret == NET_Error::NoErr)
		{
			return Core_SendToIPv4(p_sock, payload, 102, p_rawAddr, 9, 0);
		}

		return ret;
	}

	static inline NET_Error Core_WakeOnLanIPv6(const SocketHandle_t p_sock, std::span<const uint8_t, 6> p_MacAddress, std::span<const uint8_t, 16> p_rawAddr, const uint16_t* p_port)
	{
		uint8_t				payload[102];
		Core_PrepareWOLPacket(payload, p_MacAddress);

		if(p_port)
		{
			return Core_SendToIPv6(p_sock, payload, 102, p_rawAddr, *p_port, 0);
		}

		NET_Error ret = Core_SendToIPv6(p_sock, payload, 102, p_rawAddr, 7, 0);
		if(ret == NET_Error::NoErr)
		{
			return Core_SendToIPv6(p_sock, payload, 102, p_rawAddr, 9, 0);
		}

		return ret;
	}

	static inline NET_Error Core_WakeOnLanIPv4(const SocketHandle_t p_sock, std::span<const uint8_t, 6> p_MacAddress, const uint32_t p_rawAddr, const uint16_t* p_port, const void* p_password, const uint16_t p_password_size)
	{
		uint16_t	payload_size = 102;
		uint8_t		payload[CORE_NET_MAX_DATA_LEN];

		if(p_password_size)
		{
			if(p_password == nullptr || (p_password_size > CORE_NET_MAX_DATA_LEN - 103))
			{
				return NET_Error::Invalid_Option;
			}
			payload_size += p_password_size;
		}

		NET_Error ret = NET_Error::NoErr;

		Core_PrepareWOLPacket(std::span<uint8_t, 102>{payload, 102}, p_MacAddress);

		if(p_password_size)
		{
			memcpy(payload + 102, p_password, p_password_size);
		}

		if(p_port)
		{
			ret = Core_SendToIPv4(p_sock, payload, payload_size, p_rawAddr, *p_port, 0);
		}
		else
		{
			ret = Core_SendToIPv4(p_sock, payload, payload_size, p_rawAddr, 7, 0);
			if(ret == NET_Error::NoErr)
			{
				ret = Core_SendToIPv4(p_sock, payload, payload_size, p_rawAddr, 9, 0);
			}
		}

		return ret;
	}

	static inline NET_Error Core_WakeOnLanIPv6(const SocketHandle_t p_sock, std::span<const uint8_t, 6> p_MacAddress, std::span<const uint8_t, 16> p_rawAddr, const uint16_t* p_port, const void* p_password, const uint16_t p_password_size)
	{
		uint16_t	payload_size = 102;
		uint8_t		payload[CORE_NET_MAX_DATA_LEN];

		if(p_password_size)
		{
			if(p_password == nullptr || (p_password_size > CORE_NET_MAX_DATA_LEN - 103))
			{
				return NET_Error::Invalid_Option;
			}
			payload_size += p_password_size;
		}

		NET_Error ret = NET_Error::NoErr;

		Core_PrepareWOLPacket(std::span<uint8_t, 102>{payload, 102}, p_MacAddress);

		if(p_password_size)
		{
			memcpy(payload + 102, p_password, p_password_size);
		}

		if(p_port)
		{
			ret = Core_SendToIPv6(p_sock, payload, payload_size, p_rawAddr, *p_port, 0);
		}
		else
		{
			ret = Core_SendToIPv6(p_sock, payload, payload_size, p_rawAddr, 7, 0);
			if(ret == NET_Error::NoErr)
			{
				ret = Core_SendToIPv6(p_sock, payload, payload_size, p_rawAddr, 9, 0);
			}
		}

		return ret;
	}


	//========	========	========	========	========
	//========		IP String Conversions			========
	//========	========	========	========	========

	static inline bool Core_fromStringIPv4(std::u8string_view p_address, void* p_raw)
	{
		size_t		i, pos1, pos2 = 0;
		uint8_t		t_byteField[4];
		size_t size = p_address.size();

		if(size < 7 || size > 15)
		{
			return false;
		}

		for(i = 0; i < 3; ++i)
		{
			pos1 = p_address.find('.', pos2);
			if(pos1 - pos2 > 3 || pos1 == pos2) return false;

			from_chars_result<uint8_t> auxRet = from_chars<uint8_t>(p_address.substr(pos2, pos1 - pos2));
			if(!auxRet.has_value()) return false;

			t_byteField[i] = static_cast<uint8_t>(auxRet.value());
			pos2 = pos1 + 1;
		}

		pos1 = p_address.size();
		if(pos1 - pos2 > 3 || pos1 == pos2) return false;

		from_chars_result<uint8_t> auxRet = from_chars<uint8_t>(p_address.substr(pos2, pos1 - pos2));
		if(!auxRet.has_value()) return false;

		t_byteField[3] = static_cast<uint8_t>(auxRet.value());

		memcpy(p_raw, t_byteField, 4);
		return true;
	}

	static inline bool Core_fromStringIPv6(std::u8string_view p_address, uint16_t* p_raw)
	{
		size_t pos1 = 0; // initialized because otherwise generates warning 4701
		size_t pos2, size;
		bool b_hasEliad = false;
		uint8_t count;

		std::deque<uint16_t> preEliad;
		std::deque<uint16_t> postEliad;
		uint16_t auxI;
		uint16_t t_wordField[8]{0};
		const char8_t* tdata = p_address.data();

		size = p_address.size();

		if(size < 2 || size > 39) return false;

		if(p_address[0] == ':')
		{
			if(p_address[1] != ':') return false;
			count	= 2;
			pos2	= 2;
			b_hasEliad = true;
		}
		else
		{
			count	= 0;
			pos2	= 0;
		}

		for(; count < 8; ++count)
		{
			pos1 = p_address.find(':', pos2);
			if(pos1 == std::u8string_view::npos)
			{
				if(size == pos2) break;
				if(size - pos2 > 4) return false;

				from_chars_result<uint16_t> auxRet = from_hex_chars<uint16_t>({tdata + pos2, size - pos2});
				if(!auxRet.has_value())
				{
					return false;
				}
				auxI = core::endian_host2big(auxRet.value());

				if(b_hasEliad)	postEliad.push_back(auxI);
				else			preEliad.push_back(auxI);
				break;
			}

			if(pos1 - pos2 > 4) return false;
			if(pos1 == pos2)
			{
				if(b_hasEliad) return false;
				b_hasEliad = true;
				pos2 = pos1 + 1;
				continue;
			}

			from_chars_result<uint16_t> auxRet = from_hex_chars<uint16_t>({tdata + pos2, pos1 - pos2});
			if(!auxRet.has_value())
			{
				return false;
			}

			auxI = core::endian_host2big(auxRet.value());

			if(b_hasEliad)	postEliad.push_back(auxI);
			else			preEliad.push_back(auxI);

			pos2 = pos1 + 1;
			if(pos2 >= size) return false;
		}

		if(pos1 != std::u8string_view::npos) return false;

		if(b_hasEliad)
		{

			count = 0;
			while(!preEliad.empty())
			{
				t_wordField[count] = preEliad.front();
				preEliad.pop_front();
				++count;
			}

			count = 7;
			while(!postEliad.empty())
			{
				t_wordField[count] = postEliad.back();
				postEliad.pop_back();
				--count;
			}
		}
		else
		{
			if(count < 8) return false;
			count = 0;
			while(!preEliad.empty())
			{
				t_wordField[count] = preEliad.front();
				preEliad.pop_front();
				++count;
			}
		}

		memcpy(p_raw, t_wordField, 16);
		return true;
	}


	static inline uintptr_t Core_ToStringIPv4(std::span<const uint8_t, 4> p_raw, std::span<char8_t, 15> p_output)
	{
		char8_t* pivot = p_output.data();
		pivot += core::to_chars(p_raw[0], std::span<char8_t, 3>{pivot, 3});
		*pivot = '.';
		++pivot;
		pivot += core::to_chars(p_raw[1], std::span<char8_t, 3>{pivot, 3});
		*pivot = '.';
		++pivot;
		pivot += core::to_chars(p_raw[2], std::span<char8_t, 3>{pivot, 3});
		*pivot = '.';
		++pivot;
		pivot += core::to_chars(p_raw[3], std::span<char8_t, 3>{pivot, 3});

		return static_cast<uintptr_t>(pivot - p_output.data());
	}

	static inline uintptr_t Core_ToStringIPv6(std::span<const uint16_t, 8> p_raw, std::span<char8_t, 39> p_output)
	{
		uint8_t cEliad = 0, posEliad = 0;	// initialized because otherwise generates warning 4701
		uint8_t sizeEliad, cSize;

		uint8_t it;

		sizeEliad = 0;
		cSize = 0;
		for(it = 0; it < 8; ++it)
		{
			if(p_raw[it] == 0)
			{
				if(cSize) ++cSize;
				else
				{
					cEliad = it;
					cSize = 1;
				}
			}
			else
			{
				if(cSize > sizeEliad)
				{
					sizeEliad = cSize;
					posEliad = cEliad;
				}
				cSize = 0;
			}
		}

		if(cSize > sizeEliad)
		{
			sizeEliad = cSize;
			posEliad = cEliad;
		}

		char8_t* pivot = p_output.data();

		if(sizeEliad > 1)
		{
			if(posEliad == 0)
			{
				*pivot = ':';
				++pivot;
			}
			else
			{
				for(it = 0; it < posEliad; ++it)
				{
					pivot += core::to_hex_chars(core::endian_big2host(p_raw[it]), std::span<char8_t, 4>{pivot, 4});
					*pivot = ':';
					++pivot;
				}
			}

			*pivot = ':';
			++pivot;
			it = posEliad + sizeEliad;
			if(it < 8)
			{
				for(; it < 7; ++it)
				{
					pivot += core::to_hex_chars(core::endian_big2host(p_raw[it]), std::span<char8_t, 4>{pivot, 4});
					*pivot = ':';
					++pivot;
				}
				pivot += core::to_hex_chars(core::endian_big2host(p_raw[7]), std::span<char8_t, 4>{pivot, 4});
			}
		}
		else
		{
			for(it = 0; it < 7; ++it)
			{
				pivot += core::to_hex_chars(core::endian_big2host(p_raw[it]), std::span<char8_t, 4>{pivot, 4});
				*pivot = ':';
				++pivot;
			}
			pivot += core::to_hex_chars(core::endian_big2host(p_raw[7]), std::span<char8_t, 4>{pivot, 4});
		}
		return static_cast<uintptr_t> (pivot - p_output.data());
	}


	//========	========	========	========	========
	//========	========	Net Service	========	========
	//========	========	========	========	========

#ifdef _WIN32
	bool Net_Init()
	{
		WSADATA wsaData;
		// initiates use of WS2_32.DLL by a process (only Win32 platform)
		return WSAStartup(MAKEWORD(2,2), &wsaData) == 0;
	}

	void Net_End()
	{
		WSACleanup();
	}
#endif


namespace core_p
{

	//========= ======== ======== Net_Socket ========= ======== ========
	Net_Socket::Net_Socket():
		m_sock(INVALID_SOCKET)
	{
	}

	Net_Socket::~Net_Socket()
	{
		if(m_sock != INVALID_SOCKET)
		{
			if(Core_CloseSock(m_sock))
			{
				Core_setSockLinger(m_sock, false, 0);
				Core_CloseSock(m_sock);
			}
		}
	}

	bool Net_Socket::isOpen() const
	{
		return m_sock != INVALID_SOCKET;
	}

	NET_Error Net_Socket::CloseSocket()
	{
		if(m_sock != INVALID_SOCKET)
		{
			if(Core_CloseSock(m_sock))
			{
				if(SockWouldBlock(m_sock))
				{
					return NET_Error::WouldBlock;
				}
				return NET_Error::Sock_Bad_Close;
			}
			m_sock = INVALID_SOCKET;
		}
		return NET_Error::NoErr;
	}

	NET_Error Net_Socket::Poll(const uint64_t p_microseconds)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_Poll(m_sock, p_microseconds);
	}

	NET_Error Net_Socket::shutdown(const Endpoint p_direction)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_Shutdown(m_sock, p_direction);
	}

	NET_Error Net_Socket::SetBlocking(const bool p_blocking)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_setSockBlocking(m_sock, p_blocking);
	}

	NET_Error Net_Socket::SetReuseAddress(const bool p_reuse)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_setReuseAddress(m_sock, p_reuse);
	}

	NET_Error Net_Socket::SetLinger(const bool p_linger, const uint16_t p_timeout)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_setSockLinger(m_sock, p_linger, p_timeout);
	}


	//========= ======== ======== NetUDP_p ========= ======== ========

	NET_Error NetUDP_p::SetBroadcasting(const bool p_broadcast)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_setBrodCasting(m_sock, p_broadcast);
	}

	NET_Error NetUDP_p::Receive(void* p_data, uintptr_t& p_size)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_ReceiveFrom(m_sock, p_data, p_size);
	}

	NET_Error NetUDP_p::PeekSize(uintptr_t& p_size)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_PeekSize(m_sock, p_size);
	}


	//========= ======== ======== NetTCP_S_p ========= ======== ========

	NET_Error NetTCP_S_p::Listen(const int p_max_connections)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;

		if(listen(m_sock, p_max_connections))
		{
			return NET_Error::Sock_Listen;
		}
		return NET_Error::NoErr;
	}


	//========= ======== ======== NetTCP_C_p ========= ======== ========

	NET_Error NetTCP_C_p::NonBlock_Connect_state()
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_TCP_NonBlock_Connect_state(m_sock);
	}

	NET_Error NetTCP_C_p::Send_context(const void* p_buffer, const uintptr_t p_size, uintptr_t& p_context)
	{
		return Core_Send_context(m_sock, p_buffer, p_size, p_context);
	}

	NET_Error NetTCP_C_p::Send_size(const void* p_buffer, const uintptr_t p_size, uintptr_t& p_sent)
	{
		return Core_Send_size(m_sock, p_buffer, p_size, p_sent);
	}

	NET_Error NetTCP_C_p::Receive_context(void* p_buffer, const uintptr_t p_size, uintptr_t& p_context)
	{
		return Core_Receive_context(m_sock, p_buffer, p_size, p_context);
	}

	NET_Error NetTCP_C_p::Receive_size(void* p_buffer, const uintptr_t p_size, uintptr_t& p_received)
	{
		return Core_Receive_size(m_sock, p_buffer, p_size, p_received);
	}

	NET_Error NetTCP_C_p::SetNagle(const bool p_useNagle)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_setNagle(m_sock, p_useNagle);
	}

	NET_Error NetTCP_C_p::SetKeepAlive(const bool p_keepAlive, const uint32_t p_probePeriod, const uint32_t p_maxProbes)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_SetKeepAlive(m_sock, p_keepAlive, p_probePeriod, p_maxProbes);
	}

} //namesapce core_p


	//========	========	========	========	========
	//========	========	UDP			========	========
	//========	========	========	========	========

	//========= ======== ======== NetUDP_V4 ========= ======== ========

	NetUDP_V4::NetUDP_V4(NetUDP_V4&& p_other): NetUDP_V4()
	{
		std::swap(m_sock, p_other.m_sock);
	}

	NET_Error NetUDP_V4::OpenSocket(const bool p_blocking)
	{
		if(m_sock != INVALID_SOCKET) return NET_Error::Already_Used;
		return Core_CreateUDPSocketIPv4(m_sock, p_blocking);
	}

	NET_Error NetUDP_V4::Bind(const IPv4_netAddr& p_IP, const uint16_t p_Port)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_BindIPv4(m_sock, p_IP.ui32Type, p_Port);
	}

	NET_Error NetUDP_V4::OpenAndBind(const IPv4_netAddr& p_IP, const uint16_t p_Port, const bool p_blocking)
	{
		if(m_sock != INVALID_SOCKET) return NET_Error::Already_Used;
		NET_Error ret;

		//open socket
		ret = Core_CreateUDPSocketIPv4(m_sock, p_blocking);
		if(ret != NET_Error::NoErr) return ret;

		//bind
		ret = Core_BindIPv4(m_sock, p_IP.ui32Type, p_Port);
		if(ret != NET_Error::NoErr)
		{
			Core_CloseSock(m_sock);
			m_sock = INVALID_SOCKET;
			return ret;
		}
		return NET_Error::NoErr;
	}

	NET_Error NetUDP_V4::JoinMulticastGroup(const IPv4_netAddr& p_group, const uint32_t p_interface)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_JoinMulticastGroupIPv4(m_sock, p_group.ui32Type, p_interface);
	}

	NET_Error NetUDP_V4::LeaveMulticastGroup(const IPv4_netAddr& p_group, const uint32_t p_interface)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_LeaveMulticastGroupIPv4(m_sock, p_group.ui32Type, p_interface);
	}

	NET_Error NetUDP_V4::GetAddress(IPv4_netAddr& p_IP, uint16_t& p_Port)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_GetAddressIPv4(m_sock, p_IP.ui32Type, p_Port);
	}

	NET_Error NetUDP_V4::Send(const void* p_data, const uintptr_t p_size, const IPv4_netAddr& p_IP, const uint16_t p_Port, const uint8_t p_repeat)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_SendToIPv4(m_sock, p_data, p_size, p_IP.ui32Type, p_Port, p_repeat);
	}

	NET_Error NetUDP_V4::Receive(void* p_data, uintptr_t& p_size, IPv4_netAddr& p_other_IP, uint16_t& p_other_port)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_ReceiveFromIPv4(m_sock, p_data, p_size, p_other_IP.ui32Type, p_other_port);
	}

	NET_Error NetUDP_V4::PeekSize(uintptr_t& p_size, IPv4_netAddr& p_other_IP, uint16_t& p_other_port)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_PeekSizeIPv4(m_sock, p_size, p_other_IP.ui32Type, p_other_port);
	}

	NET_Error NetUDP_V4::WakeOnLan(std::span<const uint8_t, 6> p_MacAddress, const IPv4_netAddr& p_subNet, const uint16_t* p_port)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_WakeOnLanIPv4(m_sock, p_MacAddress, p_subNet.ui32Type, p_port);
	}

	NET_Error NetUDP_V4::WakeOnLan_password(std::span<const uint8_t, 6> p_MacAddress, const IPv4_netAddr& p_subNet, const uint16_t* p_port, const void* p_password, const uint16_t p_password_size)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_WakeOnLanIPv4(m_sock, p_MacAddress, p_subNet.ui32Type, p_port, p_password, p_password_size);
	}


	//========= ======== ======== NetUDP_V6 ========= ======== ========
	
	NetUDP_V6::NetUDP_V6(NetUDP_V6&& p_other): NetUDP_V6()
	{
		std::swap(m_sock, p_other.m_sock);
	}

	NET_Error NetUDP_V6::OpenSocket(const bool p_blocking)
	{
		if(m_sock != INVALID_SOCKET) return NET_Error::Already_Used;
		return Core_CreateUDPSocketIPv6(m_sock, p_blocking);
	}

	NET_Error NetUDP_V6::Bind(const IPv6_netAddr& p_IP, const uint16_t p_Port)
	{
		return Core_BindIPv6(m_sock, p_IP.byteField, p_Port);
	}

	NET_Error NetUDP_V6::OpenAndBind(const IPv6_netAddr& p_IP, const uint16_t p_Port, bool p_blocking)
	{
		if(m_sock != INVALID_SOCKET) return NET_Error::Already_Used;

		NET_Error ret;

		//open socket
		ret = Core_CreateUDPSocketIPv6(m_sock, p_blocking);
		if(ret != NET_Error::NoErr) return ret;

		//bind
		ret = Core_BindIPv6(m_sock, p_IP.byteField, p_Port);
		if(ret != NET_Error::NoErr)
		{
			Core_CloseSock(m_sock);
			m_sock = INVALID_SOCKET;
			return ret;
		}
		return NET_Error::NoErr;
	}

	NET_Error NetUDP_V6::JoinMulticastGroup(const IPv6_netAddr& p_group, const uint32_t p_interface)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_JoinMulticastGroupIPv6(m_sock, p_group.byteField, p_interface);
	}

	NET_Error NetUDP_V6::LeaveMulticastGroup(const IPv6_netAddr& p_group, const uint32_t p_interface)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_LeaveMulticastGroupIPv6(m_sock, p_group.byteField, p_interface);
	}

	NET_Error NetUDP_V6::GetAddress(IPv6_netAddr& p_IP, uint16_t& p_Port)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_GetAddressIPv6(m_sock, p_IP.byteField, p_Port);
	}

	NET_Error NetUDP_V6::Send(const void* p_data, const uintptr_t p_size, const IPv6_netAddr& p_IP, const uint16_t p_Port, const uint8_t p_repeat)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_SendToIPv6(m_sock, p_data, p_size, p_IP.byteField, p_Port, p_repeat);
	}

	NET_Error NetUDP_V6::Receive(void* p_data, uintptr_t& p_size, IPv6_netAddr& p_other_IP, uint16_t& p_other_port)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_ReceiveFromIPv6(m_sock, p_data, p_size, p_other_IP.byteField, p_other_port);
	}

	NET_Error NetUDP_V6::PeekSize(uintptr_t& p_size, IPv6_netAddr& p_other_IP, uint16_t& p_other_port)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_PeekSizeIPv6(m_sock, p_size, p_other_IP.byteField, p_other_port);
	}

	NET_Error NetUDP_V6::WakeOnLan(std::span<const uint8_t, 6> p_MacAddress, const IPv6_netAddr& p_subNet, const uint16_t* p_port)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_WakeOnLanIPv6(m_sock, p_MacAddress, p_subNet.byteField, p_port);
	}

	NET_Error NetUDP_V6::WakeOnLan_password(std::span<const uint8_t, 6> p_MacAddress, const IPv6_netAddr& p_subNet, const uint16_t* p_port, const void* p_password, const uint16_t p_password_size)
	{
		return Core_WakeOnLanIPv6(m_sock, p_MacAddress, p_subNet.byteField, p_port, p_password, p_password_size);
	}


	//========	========	========	========	========
	//========	========	TCP S		========	========
	//========	========	========	========	========

	//========= ======== ======== NetTCP_S_V4 ========= ======== ========

	NetTCP_S_V4::NetTCP_S_V4(NetTCP_S_V4&& p_other): NetTCP_S_V4()
	{
		std::swap(m_sock, p_other.m_sock);
	}

	NET_Error NetTCP_S_V4::OpenSocket(const bool p_blocking)
	{
		if(m_sock != INVALID_SOCKET) return NET_Error::Already_Used;
		return Core_CreateTCPSocketIPv4(m_sock, p_blocking);
	}

	NET_Error NetTCP_S_V4::Bind(const IPv4_netAddr& p_IP, const uint16_t p_Port)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_BindIPv4(m_sock, p_IP.ui32Type, p_Port);
	}

	NET_Error NetTCP_S_V4::OpenAndBind(const IPv4_netAddr& p_IP, const uint16_t p_Port, bool p_blocking)
	{
		if(m_sock != INVALID_SOCKET) return NET_Error::Already_Used;

		//open
		NET_Error err = Core_CreateTCPSocketIPv4(m_sock, p_blocking);
		if(err != NET_Error::NoErr) return err;

		//bind
		err = Core_BindIPv4(m_sock, p_IP.ui32Type, p_Port);
		if(err != NET_Error::NoErr)
		{
			Core_CloseSock(m_sock);
			m_sock = INVALID_SOCKET;
			return err;
		}

		return NET_Error::NoErr;
	}

	NET_Error NetTCP_S_V4::OpenBindAndListen(const IPv4_netAddr& p_IP, const uint16_t p_Port, const int p_max_connections, const bool p_blocking)
	{
		if(m_sock != INVALID_SOCKET) return NET_Error::Already_Used;

		//open
		NET_Error err = Core_CreateTCPSocketIPv4(m_sock, p_blocking);
		if(err != NET_Error::NoErr) return err;

		//bind
		err = Core_BindIPv4(m_sock, p_IP.ui32Type, p_Port);
		if(err != NET_Error::NoErr)
		{
			Core_CloseSock(m_sock);
			m_sock = INVALID_SOCKET;
			return err;
		}

		//listen
		if(listen(m_sock, p_max_connections))
		{
			Core_CloseSock(m_sock);
			m_sock = INVALID_SOCKET;
			return NET_Error::Sock_Listen;
		}
		return NET_Error::NoErr;
	}

	NET_Error NetTCP_S_V4::Accept(NetTCP_C_V4& p_Client, bool p_blocking)
	{
		if(m_sock == INVALID_SOCKET)	return NET_Error::Invalid_Socket;
		if(p_Client.isOpen())			return NET_Error::Already_Used;

		sockaddr_in addr_info4;
		return Core_AcceptIPv4(m_sock, p_Client.m_sock, p_blocking, addr_info4);
	}

	NET_Error NetTCP_S_V4::Accept(NetTCP_C_V4& p_Client, IPv4_netAddr& p_other_IP, uint16_t& p_other_port, const bool p_blocking)
	{
		if(m_sock == INVALID_SOCKET)	return NET_Error::Invalid_Socket;
		if(p_Client.isOpen())			return NET_Error::Already_Used;

		sockaddr_in addr_info4;

		NET_Error ret = Core_AcceptIPv4(m_sock, p_Client.m_sock, p_blocking, addr_info4);
		if(ret != NET_Error::NoErr)
		{
			return ret;
		}

		p_other_IP.ui32Type		= addr_info4.sin_addr.s_addr;
		p_other_port			= core::endian_big2host(addr_info4.sin_port);

		return NET_Error::NoErr;
	}

	NET_Error NetTCP_S_V4::GetAddress(IPv4_netAddr& p_IP, uint16_t& p_Port)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_GetAddressIPv4(m_sock, p_IP.ui32Type, p_Port);
	}


	//========= ======== ======== NetTCP_S_V6 ========= ======== ========

	NetTCP_S_V6::NetTCP_S_V6(NetTCP_S_V6&& p_other): NetTCP_S_V6()
	{
		std::swap(m_sock, p_other.m_sock);
	}

	NET_Error NetTCP_S_V6::OpenSocket(const bool p_blocking)
	{
		if(m_sock != INVALID_SOCKET) return NET_Error::Already_Used;
		return Core_CreateTCPSocketIPv6(m_sock, p_blocking);
	}

	NET_Error NetTCP_S_V6::Bind(const IPv6_netAddr& p_IP, const uint16_t p_Port)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_BindIPv6(m_sock, p_IP.byteField, p_Port);
	}

	NET_Error NetTCP_S_V6::OpenAndBind(const IPv6_netAddr& p_IP, const uint16_t p_Port, const bool p_blocking)
	{
		if(m_sock != INVALID_SOCKET) return NET_Error::Already_Used;

		//open
		NET_Error err = Core_CreateTCPSocketIPv6(m_sock, p_blocking);
		if(err != NET_Error::NoErr) return err;

		//bind
		err = Core_BindIPv6(m_sock, p_IP.byteField, p_Port);
		if(err != NET_Error::NoErr)
		{
			Core_CloseSock(m_sock);
			m_sock = INVALID_SOCKET;
			return err;
		}

		return NET_Error::NoErr;
	}

	NET_Error NetTCP_S_V6::OpenBindAndListen(const IPv6_netAddr& p_IP, const uint16_t p_Port, const int p_max_connections, const bool p_blocking)
	{
		if(m_sock != INVALID_SOCKET) return NET_Error::Already_Used;

		//open
		NET_Error err = Core_CreateTCPSocketIPv6(m_sock, p_blocking);
		if(err != NET_Error::NoErr) return err;

		//bind
		err = Core_BindIPv6(m_sock, p_IP.byteField, p_Port);
		if(err != NET_Error::NoErr)
		{
			Core_CloseSock(m_sock);
			m_sock = INVALID_SOCKET;
			return err;
		}

		//listen
		if(listen(m_sock, p_max_connections))
		{
			Core_CloseSock(m_sock);
			m_sock = INVALID_SOCKET;
			return NET_Error::Sock_Listen;
		}
		return NET_Error::NoErr;
	}

	NET_Error NetTCP_S_V6::Accept(NetTCP_C_V6& p_Client, const bool p_blocking)
	{
		if(m_sock == INVALID_SOCKET)	return NET_Error::Invalid_Socket;
		if(p_Client.isOpen())			return NET_Error::Already_Used;

		sockaddr_in6 addr_info6;
		return Core_AcceptIPv6(m_sock, p_Client.m_sock, p_blocking, addr_info6);
	}

	NET_Error NetTCP_S_V6::Accept(NetTCP_C_V6& p_Client, IPv6_netAddr& p_other_IP, uint16_t& p_other_port, const bool p_blocking)
	{
		if(m_sock == INVALID_SOCKET)	return NET_Error::Invalid_Socket;
		if(p_Client.isOpen())			return NET_Error::Already_Used;

		sockaddr_in6	addr_info6;

		NET_Error ret = Core_AcceptIPv6(m_sock, p_Client.m_sock, p_blocking, addr_info6);
		if(ret != NET_Error::NoErr)
		{
			return ret;
		}

		memcpy(p_other_IP.byteField, &addr_info6.sin6_addr, 16);
		p_other_port = core::endian_big2host(addr_info6.sin6_port);

		return NET_Error::NoErr;
	}

	NET_Error NetTCP_S_V6::GetAddress(IPv6_netAddr& p_IP, uint16_t& p_Port)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_GetAddressIPv6(m_sock, p_IP.byteField, p_Port);
	}


	//========	========	 ========	========	========
	//========	========	TCP C		========	========
	//========	========	 ========	========	========


	//========= ======== ======== NetTCP_C_V4 ========= ======== ========

	NetTCP_C_V4::NetTCP_C_V4(NetTCP_C_V4&& p_other): NetTCP_C_V4()
	{
		std::swap(m_sock, p_other.m_sock);
	}

	NET_Error NetTCP_C_V4::OpenSocket(const bool p_blocking)
	{
		if(m_sock != INVALID_SOCKET)	return NET_Error::Invalid_Socket;
		return Core_CreateTCPSocketIPv4(m_sock, p_blocking);
	}

	NET_Error NetTCP_C_V4::Bind(const IPv4_netAddr& my_IP, const uint16_t my_Port)
	{
		if(m_sock == INVALID_SOCKET)	return NET_Error::Invalid_Socket;
		return Core_BindIPv4(m_sock, my_IP.ui32Type, my_Port);
	}

	NET_Error NetTCP_C_V4::Connect(const IPv4_netAddr& dest_IP, const uint16_t dest_Port)
	{
		if(m_sock == INVALID_SOCKET)	return NET_Error::Invalid_Socket;
		if(dest_IP.ui32Type == 0)		return NET_Error::Invalid_IP;
		return Core_ConnectIPv4(m_sock, dest_IP.ui32Type, dest_Port);
	}

	NET_Error NetTCP_C_V4::OpenAndBind(const IPv4_netAddr& my_IP, const uint16_t my_Port, const bool p_blocking)
	{
		if(m_sock != INVALID_SOCKET)	return NET_Error::Already_Used;

		//open
		NET_Error err = Core_CreateTCPSocketIPv4(m_sock, p_blocking);
		if(err != NET_Error::NoErr) return err;

		//bind
		err = Core_BindIPv4(m_sock, my_IP.ui32Type, my_Port);
		if(err != NET_Error::NoErr)
		{
			Core_CloseSock(m_sock);
			m_sock = INVALID_SOCKET;
			return err;
		}
		return NET_Error::NoErr;
	}

	NET_Error NetTCP_C_V4::OpenBindAndConnect(const IPv4_netAddr& my_IP, const uint16_t my_Port, const IPv4_netAddr& dest_IP, const uint16_t dest_Port, const bool p_blocking)
	{
		if(m_sock != INVALID_SOCKET)	return NET_Error::Already_Used;
		if(dest_IP.ui32Type == 0)		return NET_Error::Invalid_IP;

		//open
		NET_Error err = Core_CreateTCPSocketIPv4(m_sock, p_blocking);
		if(err != NET_Error::NoErr) return err;

		//bind
		err = Core_BindIPv4(m_sock, my_IP.ui32Type, my_Port);
		if(err != NET_Error::NoErr)
		{
			Core_CloseSock(m_sock);
			m_sock = INVALID_SOCKET;
			return err;
		}

		//connect
		err = Core_ConnectIPv4(m_sock, dest_IP.ui32Type, dest_Port);
		if(err != NET_Error::NoErr && err != NET_Error::WouldBlock)
		{
			Core_CloseSock(m_sock);
			m_sock = INVALID_SOCKET;
			return err;
		}
		return err;
	}

	NET_Error NetTCP_C_V4::GetAddress(IPv4_netAddr& p_IP, uint16_t& p_Port)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_GetAddressIPv4(m_sock, p_IP.ui32Type, p_Port);
	}

	NET_Error NetTCP_C_V4::GetPeerAddress(IPv4_netAddr& p_IP, uint16_t& p_Port)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_GetPeerAddressIPv4(m_sock, p_IP.ui32Type, p_Port);
	}


	//========= ======== ======== NetTCP_C_V6 ========= ======== ========

	NetTCP_C_V6::NetTCP_C_V6(NetTCP_C_V6&& p_other): NetTCP_C_V6()
	{
		std::swap(m_sock, p_other.m_sock);
	}

	NET_Error NetTCP_C_V6::OpenSocket(const bool p_blocking)
	{
		if(m_sock != INVALID_SOCKET)	return NET_Error::Invalid_Socket;
		return Core_CreateTCPSocketIPv6(m_sock, p_blocking);
	}

	NET_Error NetTCP_C_V6::Bind(const IPv6_netAddr& my_IP, const uint16_t my_Port)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_BindIPv6(m_sock, my_IP.byteField, my_Port);
	}

	NET_Error NetTCP_C_V6::Connect(const IPv6_netAddr& dest_IP, const uint16_t dest_Port)
	{
		if(m_sock == INVALID_SOCKET)	return NET_Error::Invalid_Socket;
		if(dest_IP.ui64Type[0] == 0 && dest_IP.ui64Type[1] == 0) return NET_Error::Invalid_IP;

		return Core_ConnectIPv6(m_sock, std::span<const uint8_t, 16>{dest_IP.byteField, 16}, dest_Port);
	}

	NET_Error NetTCP_C_V6::OpenAndBind(const IPv6_netAddr& my_IP, const uint16_t my_Port, const bool p_blocking)
	{
		if(m_sock != INVALID_SOCKET)	return NET_Error::Already_Used;

		//open
		NET_Error err = Core_CreateTCPSocketIPv6(m_sock, p_blocking);
		if(err != NET_Error::NoErr) return err;

		//bind
		err = Core_BindIPv6(m_sock, my_IP.byteField, my_Port);
		if(err != NET_Error::NoErr)
		{
			Core_CloseSock(m_sock);
			m_sock = INVALID_SOCKET;
			return err;
		}

		return NET_Error::NoErr;
	}

	NET_Error NetTCP_C_V6::OpenBindAndConnect(const IPv6_netAddr& my_IP, const uint16_t my_Port, const IPv6_netAddr& dest_IP, const uint16_t dest_Port, const bool p_blocking)
	{
		if(m_sock != INVALID_SOCKET)	return NET_Error::Already_Used;
		if(dest_IP.ui64Type[0] == 0 && dest_IP.ui64Type[1] == 0) return NET_Error::Invalid_IP;

		//open
		NET_Error err = Core_CreateTCPSocketIPv6(m_sock, p_blocking);
		if(err != NET_Error::NoErr) return err;

		//bind
		err = Core_BindIPv6(m_sock, my_IP.byteField, my_Port);
		if(err != NET_Error::NoErr)
		{
			Core_CloseSock(m_sock);
			m_sock = INVALID_SOCKET;
			return err;
		}

		//connect
		err = Core_ConnectIPv6(m_sock, dest_IP.byteField, dest_Port);
		if(err != NET_Error::NoErr && err != NET_Error::WouldBlock)
		{
			Core_CloseSock(m_sock);
			m_sock = INVALID_SOCKET;
			return err;
		}
		return err;
	}

	NET_Error NetTCP_C_V6::GetAddress(IPv6_netAddr& p_IP, uint16_t& p_Port)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		return Core_GetAddressIPv6(m_sock, p_IP.byteField, p_Port);
	}

	NET_Error NetTCP_C_V6::GetPeerAddress(IPv6_netAddr& p_IP, uint16_t& p_Port)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;

		return Core_GetPeerAddressIPv6(m_sock, p_IP.byteField, p_Port);
	}



	//========	========	========	========	========
	//========	========	IPv Neutral	========	========
	//========	========	========	========	========


	//========= ======== ======== NetUDP ========= ======== ========

	NetUDP::NetUDP()
		: NetUDP_p()
		, m_IpV(IPv::None)
	{
	}

	NetUDP::NetUDP(NetUDP&& p_other): NetUDP()
	{
		std::swap(m_sock,	p_other.m_sock);
		std::swap(m_IpV,	p_other.m_IpV);
	}

	NET_Error NetUDP::CloseSocket()
	{
		NET_Error ret = NetUDP_p::CloseSocket();
		if(ret != NET_Error::NoErr) return ret;
		m_IpV = IPv::None;
		return NET_Error::NoErr;
	}

	NET_Error NetUDP::OpenSocket(const IPv p_ipV, const bool p_blocking)
	{
		if(m_sock != INVALID_SOCKET) return NET_Error::Already_Used;

		if(p_ipV == IPv::IPv_4)
		{
			NET_Error ret = Core_CreateUDPSocketIPv4(m_sock, p_blocking);
			if(ret != NET_Error::NoErr) return ret;
			m_IpV = IPv::IPv_4;
			return NET_Error::NoErr;
		}
		else if(p_ipV == IPv::IPv_6)
		{
			NET_Error ret = Core_CreateUDPSocketIPv6(m_sock, p_blocking);
			if(ret != NET_Error::NoErr) return ret;
			m_IpV = IPv::IPv_6;
			return NET_Error::NoErr;
		}

		return NET_Error::Invalid_Option;
	}

	NET_Error NetUDP::Bind(const IP_netAddr& p_IP, const uint16_t p_Port)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		const IPv ver = p_IP.version();

		if(ver != m_IpV) return NET_Error::Incompatible_Protocol;

		if(ver == IPv::IPv_4)
		{
			return Core_BindIPv4(m_sock, p_IP.v4.ui32Type, p_Port);
		}
		//else if(ver == IPv::IPv_6);

		return Core_BindIPv6(m_sock, p_IP.v6.byteField, p_Port);

	}

	NET_Error NetUDP::OpenAndBind(const IP_netAddr& p_IP, const uint16_t p_Port, const bool p_blocking)
	{
		if(m_sock != INVALID_SOCKET) return NET_Error::Already_Used;
		const IPv ver = p_IP.version();

		if(ver == IPv::IPv_4)
		{
			NET_Error ret = Core_CreateUDPSocketIPv4(m_sock, p_blocking);
			if(ret != NET_Error::NoErr) return ret;

			ret = Core_BindIPv4(m_sock, p_IP.v4.ui32Type, p_Port);
			if(ret != NET_Error::NoErr)
			{
				Core_CloseSock(m_sock);
				m_sock = INVALID_SOCKET;
				return ret;
			}

			m_IpV = IPv::IPv_4;
			return NET_Error::NoErr;
		}
		else if(ver == IPv::IPv_6)
		{
			NET_Error ret = Core_CreateUDPSocketIPv6(m_sock, p_blocking);
			if(ret != NET_Error::NoErr) return ret;

			ret = Core_BindIPv6(m_sock, p_IP.v6.byteField, p_Port);
			if(ret != NET_Error::NoErr)
			{
				Core_CloseSock(m_sock);
				m_sock = INVALID_SOCKET;
				return ret;
			}

			m_IpV = IPv::IPv_6;
			return NET_Error::NoErr;
		}

		return NET_Error::Invalid_IP;
	}

	NET_Error NetUDP::JoinMulticastGroup(const IP_netAddr& p_group, const uint32_t p_interface)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		const IPv ver	= p_group		.version();

		if(ver != m_IpV) return NET_Error::Incompatible_Protocol;

		if(ver == IPv::IPv_4)
		{
			return Core_JoinMulticastGroupIPv4(m_sock, p_group.v4.ui32Type, p_interface);
		}
		//else if(ver == IPv::IPv_6);

		return Core_JoinMulticastGroupIPv6(m_sock, p_group.v6.byteField, p_interface);
	}

	NET_Error NetUDP::LeaveMulticastGroup(const IP_netAddr& p_group, const uint32_t p_interface)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		const IPv ver	= p_group		.version();

		if(ver != m_IpV) return NET_Error::Incompatible_Protocol;

		if(ver == IPv::IPv_4)
		{
			return Core_LeaveMulticastGroupIPv4(m_sock, p_group.v4.ui32Type, p_interface);
		}
		//else if(ver == IPv::IPv_6);

		return Core_LeaveMulticastGroupIPv6(m_sock, p_group.v6.byteField, p_interface);
	}

	NET_Error NetUDP::GetAddress(IP_netAddr& p_IP, uint16_t& p_Port)
	{
		if(m_IpV == IPv::IPv_4)
		{
			p_IP.m_ipv = IPv::IPv_4;
			return Core_GetAddressIPv4(m_sock, p_IP.v4.ui32Type, p_Port);
		}
		else if(m_IpV == IPv::IPv_6)
		{
			p_IP.m_ipv = IPv::IPv_6;
			return Core_GetAddressIPv6(m_sock, p_IP.v6.byteField, p_Port);
		}

		p_IP.clear();
		p_Port = 0;
		return NET_Error::NoErr;
	}

	NET_Error NetUDP::Send(const void* p_data, const uintptr_t p_size, const IP_netAddr& p_IP, const uint16_t p_Port, const uint8_t p_repeat)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		const IPv ver = p_IP.version();
		if(ver != m_IpV) return NET_Error::Incompatible_Protocol;

		if(ver == IPv::IPv_4)
		{
			return Core_SendToIPv4(m_sock, p_data, p_size, p_IP.v4.ui32Type, p_Port, p_repeat);
		}
		//else if(ver == IPv::IPv_6);

		return Core_SendToIPv6(m_sock, p_data, p_size, p_IP.v6.byteField, p_Port, p_repeat);
	}

	NET_Error NetUDP::Receive(void* p_data, uintptr_t& p_size, IP_netAddr& p_other_IP, uint16_t& p_other_port)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;

		if(m_IpV == IPv::IPv_4)
		{
			p_other_IP.m_ipv = IPv::IPv_4;
			return Core_ReceiveFromIPv4(m_sock, p_data, p_size, p_other_IP.v4.ui32Type, p_other_port);
		}
		//else if(m_IpV == IPv::IPv_6);

		p_other_IP.m_ipv = IPv::IPv_6;
		return Core_ReceiveFromIPv6(m_sock, p_data, p_size, p_other_IP.v6.byteField, p_other_port);
	}

	NET_Error NetUDP::PeekSize(uintptr_t& p_size, IP_netAddr& p_other_IP, uint16_t& p_other_port)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;

		if(m_IpV == IPv::IPv_4)
		{
			p_other_IP.m_ipv = IPv::IPv_4;
			return Core_PeekSizeIPv4(m_sock, p_size, p_other_IP.v4.ui32Type, p_other_port);
		}
		//else if(m_IpV == IPv::IPv_6);

		p_other_IP.m_ipv = IPv::IPv_6;
		return Core_PeekSizeIPv6(m_sock, p_size, p_other_IP.v6.byteField, p_other_port);
	}

	NET_Error NetUDP::WakeOnLan(std::span<const uint8_t, 6> p_MacAddress, const IP_netAddr& p_subNet, const uint16_t* p_port)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		const IPv ver = p_subNet.version();

		if(ver != m_IpV) return NET_Error::Incompatible_Protocol;

		if(ver == IPv::IPv_4)
		{
			return Core_WakeOnLanIPv4(m_sock, p_MacAddress, p_subNet.v4.ui32Type, p_port);
		}
		return Core_WakeOnLanIPv6(m_sock, p_MacAddress, p_subNet.v6.byteField, p_port);
	}

	NET_Error NetUDP::WakeOnLan_password(std::span<const uint8_t, 6> p_MacAddress, const IP_netAddr& p_subNet, const uint16_t* p_port, const void* p_password, const uint16_t p_password_size)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		const IPv ver = p_subNet.version();

		if(ver != m_IpV) return NET_Error::Incompatible_Protocol;

		if(ver == IPv::IPv_4)
		{
			return Core_WakeOnLanIPv4(m_sock, p_MacAddress, p_subNet.v4.ui32Type, p_port, p_password, p_password_size);
		}
		return Core_WakeOnLanIPv6(m_sock, p_MacAddress, p_subNet.v6.byteField, p_port, p_password, p_password_size);
	}


	//========= ======== ======== NetTCP_S ========= ======== ========

	NetTCP_S::NetTCP_S()
		: NetTCP_S_p()
		, m_IpV(IPv::None)
	{
	}

	NetTCP_S::NetTCP_S(NetTCP_S&& p_other): NetTCP_S()
	{
		std::swap(m_sock,	p_other.m_sock);
		std::swap(m_IpV,	p_other.m_IpV);
	}

	NET_Error NetTCP_S::CloseSocket()
	{
		NET_Error ret = NetTCP_S_p::CloseSocket();
		if(ret != NET_Error::NoErr) return ret;
		m_IpV = IPv::None;
		return NET_Error::NoErr;
	}

	NET_Error NetTCP_S::OpenSocket(const IPv p_ipV, const bool p_blocking)
	{
		if(m_sock != INVALID_SOCKET) return NET_Error::Already_Used;

		if(p_ipV == IPv::IPv_4)
		{
			NET_Error ret = Core_CreateTCPSocketIPv4(m_sock, p_blocking);
			if(ret != NET_Error::NoErr) return ret;
			m_IpV = IPv::IPv_4;
			return NET_Error::NoErr;
		}
		else if(p_ipV == IPv::IPv_6)
		{
			NET_Error ret = Core_CreateTCPSocketIPv6(m_sock, p_blocking);
			if(ret != NET_Error::NoErr) return ret;
			m_IpV = IPv::IPv_6;
			return NET_Error::NoErr;
		}

		return NET_Error::Invalid_Option;
	}

	NET_Error NetTCP_S::Bind(const IP_netAddr& p_IP, const uint16_t p_Port)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		const IPv ver = p_IP.version();

		if(ver != m_IpV) return NET_Error::Incompatible_Protocol;

		if(ver == IPv::IPv_4)
		{
			return Core_BindIPv4(m_sock, p_IP.v4.ui32Type, p_Port);
		}
		//else if(ver == IPv::IPv_6);

		return Core_BindIPv6(m_sock, p_IP.v6.byteField, p_Port);
	}

	NET_Error NetTCP_S::OpenAndBind(const IP_netAddr& p_IP, const uint16_t p_Port, const bool p_blocking)
	{
		if(m_sock != INVALID_SOCKET) return NET_Error::Already_Used;
		const IPv ver = p_IP.version();

		if(ver == IPv::IPv_4)
		{
			//open
			NET_Error err = Core_CreateTCPSocketIPv4(m_sock, p_blocking);
			if(err != NET_Error::NoErr) return err;

			//bind
			err = Core_BindIPv4(m_sock, p_IP.v4.ui32Type, p_Port);
			if(err != NET_Error::NoErr)
			{
				Core_CloseSock(m_sock);
				m_sock = INVALID_SOCKET;
				return err;
			}

			m_IpV = IPv::IPv_4;
			return NET_Error::NoErr;
		}
		else if(ver == IPv::IPv_6)
		{
			//open
			NET_Error err = Core_CreateTCPSocketIPv6(m_sock, p_blocking);
			if(err != NET_Error::NoErr) return err;

			//bind
			err = Core_BindIPv6(m_sock, p_IP.v6.byteField, p_Port);
			if(err != NET_Error::NoErr)
			{
				Core_CloseSock(m_sock);
				m_sock = INVALID_SOCKET;
				return err;
			}

			m_IpV = IPv::IPv_6;
			return NET_Error::NoErr;
		}

		return NET_Error::Invalid_IP;
	}

	NET_Error NetTCP_S::OpenBindAndListen(const IP_netAddr& p_IP, const uint16_t p_Port, const int p_max_connections, const bool p_blocking)
	{
		if(m_sock != INVALID_SOCKET) return NET_Error::Already_Used;
		const IPv ver = p_IP.version();

		if(ver == IPv::IPv_4)
		{
			//open
			NET_Error err = Core_CreateTCPSocketIPv4(m_sock, p_blocking);
			if(err != NET_Error::NoErr) return err;

			//bind
			err = Core_BindIPv4(m_sock, p_IP.v4.ui32Type, p_Port);
			if(err != NET_Error::NoErr)
			{
				Core_CloseSock(m_sock);
				m_sock = INVALID_SOCKET;
				return err;
			}

			//listen
			if(listen(m_sock, p_max_connections))
			{
				Core_CloseSock(m_sock);
				m_sock = INVALID_SOCKET;
				return NET_Error::Sock_Listen;
			}
			m_IpV = IPv::IPv_4;
			return NET_Error::NoErr;
		}
		else if(ver == IPv::IPv_6)
		{
			//open
			NET_Error err = Core_CreateTCPSocketIPv6(m_sock, p_blocking);
			if(err != NET_Error::NoErr) return err;

			//bind
			err = Core_BindIPv6(m_sock, p_IP.v6.byteField, p_Port);
			if(err != NET_Error::NoErr)
			{
				Core_CloseSock(m_sock);
				m_sock = INVALID_SOCKET;
				return err;
			}

			//listen
			if(listen(m_sock, p_max_connections))
			{
				Core_CloseSock(m_sock);
				m_sock = INVALID_SOCKET;
				return NET_Error::Sock_Listen;
			}

			m_IpV = IPv::IPv_6;
			return NET_Error::NoErr;
		}

		return NET_Error::Invalid_IP;
	}

	NET_Error NetTCP_S::Accept(NetTCP_C& p_Client, const bool p_blocking)
	{
		if(m_sock == INVALID_SOCKET)	return NET_Error::Invalid_Socket;
		if(p_Client.isOpen())			return NET_Error::Already_Used;

		if(m_IpV == IPv::IPv_4)
		{
			sockaddr_in addr_info4;
			NET_Error ret = Core_AcceptIPv4(m_sock, p_Client.m_sock, p_blocking, addr_info4);
			if(ret != NET_Error::NoErr)
			{
				return ret;
			}

			p_Client.m_IpV = IPv::IPv_4;
			return NET_Error::NoErr;
		}
		//else if(m_IpV == IPv::IPv_6);

		sockaddr_in6 addr_info6;
		NET_Error ret = Core_AcceptIPv6(m_sock, p_Client.m_sock, p_blocking, addr_info6);
		if(ret != NET_Error::NoErr)
		{
			return ret;
		}

		p_Client.m_IpV = IPv::IPv_6;
		return NET_Error::NoErr;
	}

	NET_Error NetTCP_S::Accept(NetTCP_C& p_Client, IP_netAddr& p_other_IP, uint16_t& p_other_port, const bool p_blocking)
	{
		if(m_sock == INVALID_SOCKET)	return NET_Error::Invalid_Socket;
		if(p_Client.isOpen())			return NET_Error::Already_Used;

		if(m_IpV == IPv::IPv_4)
		{
			sockaddr_in addr_info4;
			NET_Error ret = Core_AcceptIPv4(m_sock, p_Client.m_sock, p_blocking, addr_info4);
			if(ret != NET_Error::NoErr)
			{
				return ret;
			}

			p_Client.m_IpV		= IPv::IPv_4;
			p_other_IP.m_ipv	= IPv::IPv_4;
			p_other_IP.v4.ui32Type	= addr_info4.sin_addr.s_addr;
			p_other_port			= core::endian_big2host(addr_info4.sin_port);

			return NET_Error::NoErr;
		}
		//else if(m_IpV == IPv::IPv_6);

		sockaddr_in6 addr_info6;
		NET_Error ret = Core_AcceptIPv6(m_sock, p_Client.m_sock, p_blocking, addr_info6);
		if(ret != NET_Error::NoErr)
		{
			return ret;
		}

		p_Client.m_IpV		= IPv::IPv_6;
		p_other_IP.m_ipv	= IPv::IPv_6;
		memcpy(p_other_IP.v6.byteField, &addr_info6.sin6_addr, 16);
		p_other_port = core::endian_big2host(addr_info6.sin6_port);

		return NET_Error::NoErr;
	}

	NET_Error NetTCP_S::GetAddress(IP_netAddr& p_IP, uint16_t& p_Port)
	{
		if(m_IpV == IPv::IPv_4)
		{
			p_IP.m_ipv = IPv::IPv_4;
			return Core_GetAddressIPv4(m_sock, p_IP.v4.ui32Type, p_Port);
		}
		else if(m_IpV == IPv::IPv_6)
		{
			p_IP.m_ipv = IPv::IPv_6;
			return Core_GetAddressIPv6(m_sock, p_IP.v6.byteField, p_Port);
		}

		p_IP.clear();
		p_Port = 0;
		return NET_Error::NoErr;
	}


	//========= ======== ======== NetTCP_C ========= ======== ========

	NetTCP_C::NetTCP_C()
		: NetTCP_C_p()
		, m_IpV(IPv::None)
	{

	}

	NetTCP_C::NetTCP_C(NetTCP_C&& p_other): NetTCP_C()
	{
		std::swap(m_sock,	p_other.m_sock);
		std::swap(m_IpV,	p_other.m_IpV);
	}

	NET_Error NetTCP_C::CloseSocket()
	{
		NET_Error ret = NetTCP_C_p::CloseSocket();
		if(ret != NET_Error::NoErr) return ret;
		m_IpV = IPv::None;
		return NET_Error::NoErr;
	}


	NET_Error NetTCP_C::OpenSocket(const IPv p_ipV, const bool p_blocking)
	{
		if(m_sock != INVALID_SOCKET) return NET_Error::Already_Used;

		if(p_ipV == IPv::IPv_4)
		{
			NET_Error ret = Core_CreateTCPSocketIPv4(m_sock, p_blocking);
			if(ret != NET_Error::NoErr) return ret;
			m_IpV = IPv::IPv_4;
			return NET_Error::NoErr;
		}
		else if(p_ipV == IPv::IPv_6)
		{
			NET_Error ret = Core_CreateTCPSocketIPv6(m_sock, p_blocking);
			if(ret != NET_Error::NoErr) return ret;
			m_IpV = IPv::IPv_6;
			return NET_Error::NoErr;
		}

		return NET_Error::Invalid_Option;
	}

	NET_Error NetTCP_C::Bind(const IP_netAddr& my_IP, const uint16_t my_Port)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		
		const IPv ver = my_IP.version();
		if(ver != m_IpV) return NET_Error::Incompatible_Protocol;

		if(ver == IPv::IPv_4)
		{
			return Core_BindIPv4(m_sock, my_IP.v4.ui32Type, my_Port);
		}
		//else if(ver == IPv::IPv_6);

		return Core_BindIPv6(m_sock, my_IP.v6.byteField, my_Port);
	}

	NET_Error NetTCP_C::Connect(const IP_netAddr& dest_IP, const uint16_t dest_Port)
	{
		if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
		
		const IPv ver = dest_IP.version();
		if(ver != m_IpV) return NET_Error::Incompatible_Protocol;

		if(ver == IPv::IPv_4)
		{
			if(dest_IP.v4.ui32Type == 0) return NET_Error::Invalid_IP;
			return Core_ConnectIPv4(m_sock, dest_IP.v4.ui32Type, dest_Port);
		}
		//else if(ver == IPv::IPv_6);

		if(dest_IP.v6.ui64Type[0] == 0 && dest_IP.v6.ui64Type[1] == 0) return NET_Error::Invalid_IP;
		return Core_ConnectIPv6(m_sock, dest_IP.v6.byteField, dest_Port);
	}

	NET_Error NetTCP_C::OpenAndBind(const IP_netAddr& my_IP, const uint16_t my_Port, const bool p_blocking)
	{
		if(m_sock != INVALID_SOCKET) return NET_Error::Already_Used;
		const IPv ver	= my_IP		.version();

		if(ver == IPv::IPv_4)
		{
			//open
			NET_Error err = Core_CreateTCPSocketIPv4(m_sock, p_blocking);
			if(err != NET_Error::NoErr) return err;

			//bind
			err = Core_BindIPv4(m_sock, my_IP.v4.ui32Type, my_Port);
			if(err != NET_Error::NoErr)
			{
				Core_CloseSock(m_sock);
				m_sock = INVALID_SOCKET;
				return err;
			}

			m_IpV = IPv::IPv_4;
			return NET_Error::NoErr;
		}
		else if(ver == IPv::IPv_6)
		{
			//open
			NET_Error err = Core_CreateTCPSocketIPv6(m_sock, p_blocking);
			if(err != NET_Error::NoErr) return err;

			//bind
			err = Core_BindIPv6(m_sock, my_IP.v6.byteField, my_Port);
			if(err != NET_Error::NoErr)
			{
				Core_CloseSock(m_sock);
				m_sock = INVALID_SOCKET;
				return err;
			}

			m_IpV = IPv::IPv_6;
			return NET_Error::NoErr;
		}

		return NET_Error::Invalid_IP;
	}

	NET_Error NetTCP_C::OpenBindAndConnect(const IP_netAddr& my_IP, const uint16_t my_Port, const IP_netAddr& dest_IP, const uint16_t dest_Port, const bool p_blocking)
	{
		if(m_sock != INVALID_SOCKET) return NET_Error::Already_Used;
		const IPv ver	= my_IP		.version();
		const IPv ver2	= dest_IP	.version();

		if(ver != ver2) return NET_Error::Invalid_IP;

		if(ver == IPv::IPv_4)
		{
			//open
			NET_Error err = Core_CreateTCPSocketIPv4(m_sock, p_blocking);
			if(err != NET_Error::NoErr) return err;

			//bind
			err = Core_BindIPv4(m_sock, my_IP.v4.ui32Type, my_Port);
			if(err != NET_Error::NoErr)
			{
				Core_CloseSock(m_sock);
				m_sock = INVALID_SOCKET;
				return err;
			}

			//connect
			err = Core_ConnectIPv4(m_sock, dest_IP.v4.ui32Type, dest_Port);
			if(err != NET_Error::NoErr && err != NET_Error::WouldBlock)
			{
				Core_CloseSock(m_sock);
				m_sock = INVALID_SOCKET;
				return err;
			}

			m_IpV = IPv::IPv_4;
			return err;
		}
		else if(ver == IPv::IPv_6)
		{
			//open
			NET_Error err = Core_CreateTCPSocketIPv6(m_sock, p_blocking);
			if(err != NET_Error::NoErr) return err;

			//bind
			err = Core_BindIPv6(m_sock, my_IP.v6.byteField, my_Port);
			if(err != NET_Error::NoErr)
			{
				Core_CloseSock(m_sock);
				m_sock = INVALID_SOCKET;
				return err;
			}

			//connect
			err = Core_ConnectIPv6(m_sock, dest_IP.v6.byteField, dest_Port);
			if(err != NET_Error::NoErr && err != NET_Error::WouldBlock)
			{
				Core_CloseSock(m_sock);
				m_sock = INVALID_SOCKET;
				return err;
			}

			m_IpV = IPv::IPv_6;
			return err;
		}

		return NET_Error::Invalid_IP;
	}

	NET_Error NetTCP_C::GetAddress(IP_netAddr& p_IP, uint16_t& p_Port)
	{
		if(m_IpV == IPv::IPv_4)
		{
			p_IP.m_ipv = IPv::IPv_4;
			return Core_GetAddressIPv4(m_sock, p_IP.v4.ui32Type, p_Port);
		}
		else if(m_IpV == IPv::IPv_6)
		{
			p_IP.m_ipv = IPv::IPv_6;
			return Core_GetAddressIPv6(m_sock, p_IP.v6.byteField, p_Port);
		}

		p_IP.clear();
		p_Port = 0;
		return NET_Error::NoErr;
	}

	NET_Error NetTCP_C::GetPeerAdrress(IP_netAddr& p_IP, uint16_t& p_Port)
	{
		if(m_IpV == IPv::IPv_4)
		{
			p_IP.m_ipv = IPv::IPv_4;
			return Core_GetPeerAddressIPv4(m_sock, p_IP.v4.ui32Type, p_Port);
		}
		else if(m_IpV == IPv::IPv_6)
		{
			p_IP.m_ipv = IPv::IPv_6;
			return Core_GetPeerAddressIPv6(m_sock, p_IP.v6.byteField, p_Port);
		}

		p_IP.clear();
		p_Port = 0;
		return NET_Error::NoErr;
	}

	//========	========	========	========	========
	//========	========	OTHER		========	========
	//========	========	========	========	========

	//========= ======== ======== IPv4_netAddr ========= ======== ========

	IPv4_netAddr::IPv4_netAddr(std::u8string_view p_address)
	{
		if(!fromString(p_address)) ui32Type = 0;
	}

	bool IPv4_netAddr::fromString(std::u8string_view p_address)
	{
		return Core_fromStringIPv4(p_address, byteField);
	}

	uintptr_t IPv4_netAddr::toString(std::span<char8_t, 15> p_output) const
	{
		return Core_ToStringIPv4(byteField, p_output);
	}
	std::u8string IPv4_netAddr::toString() const
	{
		std::array<char8_t, 15> buff;
		return {buff.data(), toString(buff)};
	}

	//========= ======== ======== IPv6_netAddr ========= ======== ========

	IPv6_netAddr::IPv6_netAddr()
	{
		ui64Type[0]	= 0;
		ui64Type[1]	= 0;
	}

	IPv6_netAddr::IPv6_netAddr(std::span<const uint8_t, 16> p_init)
	{
		memcpy(byteField, p_init.data(), 16);
	}

	IPv6_netAddr::IPv6_netAddr(const IPv6_netAddr& p_other)
	{
		ui64Type[0]	= p_other.ui64Type[0];
		ui64Type[1]	= p_other.ui64Type[1];
	}

	IPv6_netAddr::IPv6_netAddr(std::u8string_view p_address)
	{
		if(!fromString(p_address))
		{
			ui64Type[0] = 0;
			ui64Type[1] = 0;
		}
	}

	bool IPv6_netAddr::fromString(std::u8string_view p_address)
	{
		return Core_fromStringIPv6(p_address, wordField);
	}

	uintptr_t IPv6_netAddr::toString(std::span<char8_t, 39> p_output) const
	{
		return Core_ToStringIPv6(wordField, p_output);
	}

	std::u8string IPv6_netAddr::toString() const
	{
		std::array<char8_t, 39> buff;
		return {buff.data(), toString(buff)};
	}

	void IPv6_netAddr::setAny()
	{
		ui64Type[0]	= 0;
		ui64Type[1]	= 0;
	}

	void IPv6_netAddr::swap(IPv6_netAddr& p_other)
	{
		std::swap(ui64Type[0], p_other.ui64Type[0]);
		std::swap(ui64Type[1], p_other.ui64Type[1]);
	}

	IPv6_netAddr& IPv6_netAddr::operator = (const IPv6_netAddr& p_other)
	{
		ui64Type[0] = p_other.ui64Type[0];
		ui64Type[1] = p_other.ui64Type[1];
		return *this;
	}

	IPv6_netAddr& IPv6_netAddr::operator |= (const IPv6_netAddr& p_other)
	{
		ui64Type[0] |= p_other.ui64Type[0];
		ui64Type[1] |= p_other.ui64Type[1];
		return *this;
	}

	IPv6_netAddr& IPv6_netAddr::operator &= (const IPv6_netAddr& p_other)
	{
		ui64Type[0] &= p_other.ui64Type[0];
		ui64Type[1] &= p_other.ui64Type[1];
		return *this;
	}

	IPv6_netAddr& IPv6_netAddr::operator ^= (const IPv6_netAddr& p_other)
	{
		ui64Type[0] ^= p_other.ui64Type[0];
		ui64Type[1] ^= p_other.ui64Type[1];
		return *this;
	}

	IPv6_netAddr IPv6_netAddr::operator | (const IPv6_netAddr& p_other) const
	{
		IPv6_netAddr out;
		out.ui64Type[0] = ui64Type[0] | p_other.ui64Type[0];
		out.ui64Type[1] = ui64Type[1] | p_other.ui64Type[1];
		return out;
	}

	IPv6_netAddr IPv6_netAddr::operator & (const IPv6_netAddr& p_other) const
	{
		IPv6_netAddr out;
		out.ui64Type[0] = ui64Type[0] & p_other.ui64Type[0];
		out.ui64Type[1] = ui64Type[1] & p_other.ui64Type[1];
		return out;
	}

	IPv6_netAddr IPv6_netAddr::operator ^ (const IPv6_netAddr& p_other) const
	{
		IPv6_netAddr out;
		out.ui64Type[0] = ui64Type[0] ^ p_other.ui64Type[0];
		out.ui64Type[1] = ui64Type[1] ^ p_other.ui64Type[1];
		return out;
	}

	IPv6_netAddr IPv6_netAddr::operator ~ () const
	{
		IPv6_netAddr out;
		out.ui64Type[0] = ~ui64Type[0];
		out.ui64Type[1] = ~ui64Type[1];
		return out;
	}

	bool IPv6_netAddr::operator == (const IPv6_netAddr& p_other) const
	{
		return memcmp(byteField, p_other.byteField, 16) == 0;
	}

	bool IPv6_netAddr::operator != (const IPv6_netAddr& p_other) const
	{
		return memcmp(byteField, p_other.byteField, 16) != 0;
	}

	bool IPv6_netAddr::operator < (const IPv6_netAddr& p_other) const
	{
		if(ui64Type[0] == p_other.ui64Type[0])
		{
			return ui64Type[1] < p_other.ui64Type[1];
		}
		return ui64Type[0] < p_other.ui64Type[0];
	}


	//========= ======== ======== IP_netAddr ========= ======== ========

	IP_netAddr::IP_netAddr():
		m_ipv(IPv::None)
	{
	}

	IP_netAddr::IP_netAddr(IPv p_version, std::span<const uint8_t> p_init)
	{
		if(p_version == IPv::IPv_4)
		{
			if(p_init.size() == 4)
			{
				memcpy(v4.byteField, p_init.data(), 4);
			}
			else
			{
				v4.ui32Type = 0;
			}
			m_ipv = IPv::IPv_4;
		}
		else if(p_version == IPv::IPv_6)
		{
			if(p_init.size() == 16)
			{
				memcpy(v6.byteField, p_init.data(), 16);
			}
			else
			{
				v6.ui64Type[0] = 0;
				v6.ui64Type[1] = 0;
			}
			m_ipv = IPv::IPv_6;
		}
		else
		{
			m_ipv = IPv::None;
		}
	}

	IP_netAddr::IP_netAddr(std::u8string_view p_address)
	{
		fromString(p_address);
	}

	bool IP_netAddr::fromStringV4(std::u8string_view p_address)
	{
		m_ipv = IPv::None;
		if(Core_fromStringIPv4(p_address, v4.byteField))
		{
			m_ipv = IPv::IPv_4;
			return true;
		}

		return false;
	}

	bool IP_netAddr::fromStringV6(std::u8string_view p_address)
	{
		m_ipv = IPv::None;
		if(Core_fromStringIPv6(p_address, v6.wordField))
		{
			m_ipv = IPv::IPv_6;
			return true;
		}

		return false;
	}

	bool IP_netAddr::fromString(std::u8string_view p_address)
	{
		if(fromStringV4(p_address)) return true;
		return fromStringV6(p_address);
	}

	uintptr_t IP_netAddr::toString(std::span<char8_t, 39> p_output) const
	{
		if(m_ipv == IPv::IPv_4)
		{
			return Core_ToStringIPv4(v4.byteField, p_output.subspan<0, 15>());
		}
		else if(m_ipv == IPv::IPv_6)
		{
			return Core_ToStringIPv6(v6.wordField, p_output);
		}
		return 0;
	}

	std::u8string IP_netAddr::toString() const
	{
		std::array<char8_t, 39> buff;
		return {buff.data(), toString(buff)};
	}

	void IP_netAddr::setAny_V4()
	{
		m_ipv		= IPv::IPv_4;
		v4.ui32Type	= 0;
	}

	void IP_netAddr::setAny_V6()
	{
		m_ipv			= IPv::IPv_6;
		v6.ui64Type[0]	= 0;
		v6.ui64Type[1]	= 0;
	}

	void IP_netAddr::setLoopBack_V4()
	{
		m_ipv			= IPv::IPv_4;
		v4.ui32Type		= 0;
		v4.byteField[0]	= 127;
		v4.byteField[3]	= 1;
	}

	void IP_netAddr::setLoopBack_V6()
	{
		m_ipv				= IPv::IPv_6;
		v6.ui64Type[0]		= 0;
		v6.ui64Type[1]		= 0;
		v6.byteField[15]	= 1;
	}

	bool IP_netAddr::is_null() const
	{
		if(m_ipv == IPv::IPv_4)
		{
			return v4.ui32Type == 0;
		}
		else if(m_ipv == IPv::IPv_6)
		{
			return v6.ui64Type[0] == 0 && v6.ui64Type[1] == 0;
		}
		return true;
	}

	bool IP_netAddr::is_valid() const
	{
		return (m_ipv == IPv::IPv_4) || (m_ipv == IPv::IPv_6);
	}

	IP_netAddr& IP_netAddr::operator |= (const IP_netAddr& p_other)
	{
		v6.ui64Type[0] |= p_other.v6.ui64Type[0];
		v6.ui64Type[1] |= p_other.v6.ui64Type[1];
		return *this;
	}

	IP_netAddr& IP_netAddr::operator &= (const IP_netAddr& p_other)
	{
		v6.ui64Type[0] &= p_other.v6.ui64Type[0];
		v6.ui64Type[1] &= p_other.v6.ui64Type[1];
		return *this;
	}

	IP_netAddr& IP_netAddr::operator ^= (const IP_netAddr& p_other)
	{
		v6.ui64Type[0] ^= p_other.v6.ui64Type[0];
		v6.ui64Type[1] ^= p_other.v6.ui64Type[1];
		return *this;
	}

	IP_netAddr IP_netAddr::operator | (const IP_netAddr& p_other) const
	{
		IP_netAddr ret(*this);
		return ret |= p_other;
	}

	IP_netAddr IP_netAddr::operator & (const IP_netAddr& p_other) const
	{
		IP_netAddr ret(*this);
		return ret &= p_other;
	}

	IP_netAddr IP_netAddr::operator ^ (const IP_netAddr& p_other) const
	{
		IP_netAddr ret(*this);
		return ret ^= p_other;
	}

	IP_netAddr IP_netAddr::operator ~ () const
	{
		IP_netAddr out;
		out.m_ipv = m_ipv;
		out.v6.ui64Type[0] = ~v6.ui64Type[0];
		out.v6.ui64Type[1] = ~v6.ui64Type[1];
		return out;
	}

	bool IP_netAddr::operator == (const IP_netAddr& p_other) const
	{
		if(m_ipv == p_other.m_ipv)
		{
			if(m_ipv == IPv::IPv_4)
			{
				return v4.ui32Type == p_other.v4.ui32Type;
			}
			return memcmp(v6.byteField, p_other.v6.byteField, 16) == 0;
		}
		return false;
	}

	bool IP_netAddr::operator != (const IP_netAddr& p_other) const
	{
		if(m_ipv == p_other.m_ipv)
		{
			if(m_ipv == IPv::IPv_4)
			{
				return v4.ui32Type != p_other.v4.ui32Type;
			}
			return memcmp(v6.byteField, p_other.v6.byteField, 16) != 0;
		}
		return true;
	}

	bool IP_netAddr::operator < (const IP_netAddr& p_other) const
	{
		if(m_ipv < p_other.m_ipv) return true;
		if(m_ipv == IPv::IPv_4)
		{
			return v4.ui32Type < p_other.v4.ui32Type;
		}
		if(v6.ui64Type[0] == p_other.v6.ui64Type[0])
		{
			return v6.ui64Type[1] < p_other.v6.ui64Type[1];
		}
		return v6.ui64Type[0] < p_other.v6.ui64Type[0];
	}

} //namesapce core

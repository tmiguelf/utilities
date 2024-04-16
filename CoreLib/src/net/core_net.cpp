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
///
///	\todo	Provide a comprehensive and consistent set of error codes, to give
///			extra information regarding he nature of the failure
//======== ======== ======== ======== ======== ======== ======== ========

#include <CoreLib/net/core_net_socket.hpp>
#include <CoreLib/net/core_net_UDP.hpp>
#include <CoreLib/net/core_net_TCP.hpp>

#include <CoreLib/Core_Endian.hpp>

#include <limits>

#ifdef _WIN32
#	include <Winsock2.h>
#	include <Ws2tcpip.h>
//#	include <Iphlpapi.h>
#	include <Mswsock.h>
#	include <Mstcpip.h>
#else
#	include <cstring>
//#	include <fcntl.h>
#	include <sys/ioctl.h>
//#	include <sys/socket.h>
//#	include <netinet/in.h>
#	include <netinet/tcp.h>
#	include <arpa/inet.h>
#	include <unistd.h>
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

static inline NET_Error Core_BindIPv4(_p::SocketHandle_t const p_sock, uint32_t const p_rawAddr, uint16_t const p_port)
{
	sockaddr_in addr_info4;

	addr_info4.sin_family		= AF_INET;
	addr_info4.sin_addr.s_addr	= p_rawAddr;
	addr_info4.sin_port			= core::endian_host2big(p_port);

	if(bind(p_sock, reinterpret_cast<sockaddr const*>(&addr_info4), sizeof(sockaddr_in)) != 0)
	{
		return NET_Error::Sock_Bind;
	}

	return NET_Error::NoErr;
}

static inline NET_Error Core_BindIPv6(_p::SocketHandle_t const p_sock, std::span<uint8_t const, 16> const p_rawAddr, uint16_t const p_port)
{
	sockaddr_in6 addr_info6{};
	memcpy(&addr_info6.sin6_addr, p_rawAddr.data(), 16);
	addr_info6.sin6_family	= AF_INET6;
	addr_info6.sin6_port	= core::endian_host2big(p_port);

	if(bind(p_sock, reinterpret_cast<sockaddr const*>(&addr_info6), sizeof(sockaddr_in6)) != 0)
	{
		return NET_Error::Sock_Bind;
	}

	return NET_Error::NoErr;
}

static inline NET_Error Core_JoinMulticastGroupIPv4(_p::SocketHandle_t const p_sock, uint32_t const p_rawGroup, uint32_t const p_InterfaceAddr)
{
	ip_mreq mreq;
	mreq.imr_multiaddr.s_addr = p_rawGroup;
	mreq.imr_interface.s_addr = p_InterfaceAddr;

	if(setsockopt(p_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<char const*>(&mreq), sizeof(ip_mreq)))
	{
		return NET_Error::Sock_Option;
	}
	return NET_Error::NoErr;
}

static inline NET_Error Core_JoinMulticastGroupIPv6(_p::SocketHandle_t const p_sock, std::span<uint8_t const, 16> const p_rawGroup, uint32_t const p_InterfaceNum)
{
	ipv6_mreq mreq;
	memcpy(&mreq.ipv6mr_multiaddr, p_rawGroup.data(), 16);
	mreq.ipv6mr_interface = p_InterfaceNum;

	if(setsockopt(p_sock, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, reinterpret_cast<char const*>(&mreq), sizeof(ipv6_mreq)))
	{
		return NET_Error::Sock_Option;
	}
	return NET_Error::NoErr;
}

static inline NET_Error Core_LeaveMulticastGroupIPv4(_p::SocketHandle_t const p_sock, uint32_t const p_rawGroup, uint32_t const p_InterfaceAddr)
{
	ip_mreq mreq;
	mreq.imr_multiaddr.s_addr = p_rawGroup;
	mreq.imr_interface.s_addr = p_InterfaceAddr;

	if(setsockopt(p_sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, reinterpret_cast<char const*>(&mreq), sizeof(ip_mreq)))
	{
		return NET_Error::Sock_Option;
	}
	return NET_Error::NoErr;
}

static inline NET_Error Core_LeaveMulticastGroupIPv6(_p::SocketHandle_t const p_sock, std::span<uint8_t const, 16> const p_rawGroup, uint32_t const p_InterfaceNum)
{
	ipv6_mreq mreq;
	memcpy(&mreq.ipv6mr_multiaddr, p_rawGroup.data(), 16);
	mreq.ipv6mr_interface = p_InterfaceNum;

	if(setsockopt(p_sock, IPPROTO_IPV6, IPV6_DROP_MEMBERSHIP, reinterpret_cast<char const*>(&mreq), sizeof(ipv6_mreq)))
	{
		return NET_Error::Sock_Option;
	}
	return NET_Error::NoErr;
}

//========	========	========	========	========
//========			Common Check				========
//========	========	========	========	========

static inline NET_Error Core_Poll(_p::SocketHandle_t const p_sock, uint64_t const p_microseconds)
{
	fd_set fds;
	uint64_t const sec = p_microseconds / 1000000;
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

static inline NET_Error Core_TCP_NonBlock_Connect_state(_p::SocketHandle_t const p_sock)
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
static inline int SockLastError(_p::SocketHandle_t const)
{
	return WSAGetLastError();
}

/// \brief Used to check if last error on the socket was to a timeout on a non-blocking socket
//
static inline bool SockWouldBlock(_p::SocketHandle_t const p_socket)
{
	return (SockLastError(p_socket) == WSAEWOULDBLOCK);
}

/// \brief Used to check if last error on the socket was a buffer overflow
//
static inline bool SockBuffOverflow(_p::SocketHandle_t const p_socket)
{
	return (SockLastError(p_socket) == WSAEMSGSIZE);
}

static inline bool SockNonBlockingConnectCheck(_p::SocketHandle_t const p_socket)
{
	return SockLastError(p_socket) == WSAEWOULDBLOCK;
}


//========	========	========	========	========
//========	========	Set options	========	========
//========	========	========	========	========

static inline NET_Error Core_setSockBlocking(_p::SocketHandle_t const p_sock, bool const p_blocking)
{
	u_long opt = p_blocking ? 0 : 1;
	return ioctlsocket(p_sock, FIONBIO, &opt) ? NET_Error::Sock_Option : NET_Error::NoErr;
}

static inline NET_Error Core_setSockLinger(_p::SocketHandle_t const p_sock, bool const p_linger, uint16_t const p_timeOut)
{
	linger opt;

	opt.l_onoff		= p_linger ? 1: 0;
	opt.l_linger	= p_timeOut;

	return setsockopt(p_sock, SOL_SOCKET, SO_LINGER, reinterpret_cast<char const*>(&opt), sizeof(opt)) ? NET_Error::Sock_Option : NET_Error::NoErr;
}

static inline NET_Error Core_setBrodCasting(_p::SocketHandle_t const p_sock, bool const p_broadcast)
{
	BOOL const opt = p_broadcast ? TRUE : FALSE;
	return setsockopt(p_sock, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char const*>(&opt), sizeof(opt)) ? NET_Error::Sock_Option : NET_Error::NoErr;
}

static inline NET_Error Core_setReuseAddress(_p::SocketHandle_t const p_sock, bool const p_reuse)
{
	BOOL const opt = p_reuse ? TRUE : FALSE;
	return setsockopt(p_sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char const*>(&opt), sizeof(opt)) ? NET_Error::Sock_Option : NET_Error::NoErr;
}

static inline NET_Error Core_setNagle(_p::SocketHandle_t const p_sock, bool const p_nagle)
{
	BOOL const opt = p_nagle ? FALSE : TRUE;
	return setsockopt(p_sock, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char const*>(&opt), sizeof(opt)) ? NET_Error::Sock_Option : NET_Error::NoErr;
}

static inline NET_Error Core_SetKeepAlive(_p::SocketHandle_t const p_sock, bool const p_keepAlive, uint32_t const p_probePeriod, uint32_t const p_maxProbes)
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

		int const res = WSAIoctl(p_sock, SIO_KEEPALIVE_VALS, &opt, sizeof(opt), nullptr, 0, nullptr, nullptr, nullptr);

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

	DWORD const opt = 0;
	return setsockopt(p_sock, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<char const*>(&opt), sizeof(opt)) ? NET_Error::Sock_Option : NET_Error::NoErr;
}

//========	========	========	========	========
//========			Open and Close				========
//========	========	========	========	========

static inline int Core_CloseSock(_p::SocketHandle_t const p_sock)
{
	return closesocket(p_sock);
}

static inline NET_Error Core_createSocket(_p::SocketHandle_t& p_sock, int const p_af, int const p_domain, int const p_protocol, bool const p_blocking)
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

static inline NET_Error Core_CreateUDPSocketIPv4(_p::SocketHandle_t& p_sock, bool const p_blocking)
{
	NET_Error const err = Core_createSocket(p_sock, AF_INET, SOCK_DGRAM, IPPROTO_UDP, p_blocking);
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

static inline NET_Error Core_CreateUDPSocketIPv6(_p::SocketHandle_t& p_sock, bool const p_blocking)
{
	NET_Error const err = Core_createSocket(p_sock, AF_INET6, SOCK_DGRAM, IPPROTO_UDP, p_blocking);
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

static inline NET_Error Core_Shutdown(_p::SocketHandle_t const p_sock, _p::Net_Socket::Endpoint const p_direction)
{
	//this optimization is only aforder because documentation states that Read=0, Write=1 amd ReadWrite=2
	static_assert(_p::Net_Socket::Endpoint::Receive	== _p::Net_Socket::Endpoint{1});
	static_assert(_p::Net_Socket::Endpoint::Send	== _p::Net_Socket::Endpoint{2});
	static_assert(_p::Net_Socket::Endpoint::Both	== _p::Net_Socket::Endpoint{3});
	return shutdown(p_sock, static_cast<uint8_t>(p_direction) - 1) ? NET_Error::Fail : NET_Error::NoErr;
}

#if 0
static inline NET_Error Core_Shutdown(_p::SocketHandle_t const p_sock, _p::Net_Socket::Endpoint const p_direction)
{
	switch(p_direction)
	{
		case _p::Net_Socket::Endpoint_Receive:
			return shutdown(p_sock, SD_RECEIVE) ? NET_Error::Fail : NET_Error::NoErr;
		case _p::Net_Socket::Endpoint_Send:
			return shutdown(p_sock, SD_SEND) ? NET_Error::Fail : NET_Error::NoErr;
		case _p::Net_Socket::Endpoint_Both:
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


static inline NET_Error Core_PeekSize(_p::SocketHandle_t const p_sock, uintptr_t& p_size)
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

static inline NET_Error Core_PeekSizeIPv4(_p::SocketHandle_t const p_sock, uintptr_t& p_size, uint32_t& p_rawAddr, uint16_t& p_port)
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

static inline NET_Error Core_PeekSizeIPv6(_p::SocketHandle_t const p_sock, uintptr_t& p_size, void* const p_rawAddr, uint16_t& p_port)
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
static inline int SockLastError(_p::SocketHandle_t const p_socket)
{
	int error;
	socklen_t len;
	len = 4;
	getsockopt(p_socket, SOL_SOCKET, SO_ERROR, reinterpret_cast<void*>(&error), &len);
	return error;
}

/// \brief Used to check if last error on the socket was to a timeout on a non-blocking socket
//
static bool SockWouldBlock(_p::SocketHandle_t const p_socket)
{
	int error;
	socklen_t len;
	len = 4;
	if(getsockopt(p_socket, SOL_SOCKET, SO_ERROR, reinterpret_cast<void*>(&error), &len) != 0) return false;
	return (error == EAGAIN || error == EWOULDBLOCK);
}

static inline bool SockNonBlockingConnectCheck(_p::SocketHandle_t const p_socket)
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

static inline NET_Error Core_setSockBlocking(_p::SocketHandle_t const p_sock, bool const blocking)
{
	int const b = blocking ? 0 : 1;
	return ioctl(p_sock, FIONBIO, &b) ? NET_Error::Sock_Option : NET_Error::NoErr;
}

static inline NET_Error Core_setSockLinger(_p::SocketHandle_t const p_sock, bool const p_linger, uint16_t const p_timeOut)
{
	linger const opt
	{
		.l_onoff  = p_linger ? 1 : 0,
		.l_linger = p_timeOut,
	};

	return setsockopt(p_sock, SOL_SOCKET, SO_LINGER, reinterpret_cast<void const*>(&opt), sizeof(opt)) ? NET_Error::Sock_Option : NET_Error::NoErr;
}

static inline NET_Error Core_setBrodCasting(_p::SocketHandle_t const p_sock, bool const broadcast)
{
	int const h = broadcast ? 1 : 0;
	return setsockopt(p_sock, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<void const*>(&h), sizeof(h)) ? NET_Error::Sock_Option : NET_Error::NoErr;
}

static inline NET_Error Core_setReuseAddress(_p::SocketHandle_t const p_sock, bool const p_reuse)
{
	int const opt = p_reuse ? 1 : 0;
	return setsockopt(p_sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<void const*>(&opt), sizeof(opt)) ? NET_Error::Sock_Option : NET_Error::NoErr;
}

static inline NET_Error Core_setNagle(_p::SocketHandle_t const p_sock, bool const p_nagle)
{
	int const opt = p_nagle ? 0 : 1;
	return setsockopt(p_sock, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<void const*>(&opt), sizeof(opt)) ? NET_Error::Sock_Option : NET_Error::NoErr;
}

static inline NET_Error Core_SetKeepAlive(_p::SocketHandle_t const p_sock, bool const p_keepAlive, uint32_t const p_probePeriod, uint32_t const p_maxProbes)
{
	if(p_keepAlive)
	{
		if((p_probePeriod == 0) ||
			(p_maxProbes < 1) ||
			(static_cast<uint64_t>(p_maxProbes) * static_cast<uint64_t>(p_probePeriod)) > 9000)
		{
			return NET_Error::Invalid_Option;
		}

		int const probes		= static_cast<int>(p_maxProbes);
		int const period		= static_cast<int>(p_probePeriod);
		int const firstDelay	= period;
		int const keepAlive		= 1;

		if(	setsockopt(p_sock, IPPROTO_TCP, TCP_KEEPCNT,   &probes,     sizeof(probes)    ) ||
			setsockopt(p_sock, IPPROTO_TCP, TCP_KEEPIDLE,  &firstDelay, sizeof(firstDelay)) ||
			setsockopt(p_sock, IPPROTO_TCP, TCP_KEEPINTVL, &period,     sizeof(period)    ) ||
			setsockopt(p_sock, SOL_SOCKET,  SO_KEEPALIVE,  &keepAlive,  sizeof(keepAlive) ) )
		{
			return NET_Error::Sock_Option;
		}

		return NET_Error::NoErr;
	}

	int const opt = 0;
	return setsockopt(p_sock, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)) ? NET_Error::Sock_Option : NET_Error::NoErr;

}

//========	========	========	========	========
//========			Open and Close				========
//========	========	========	========	========

static inline int Core_CloseSock(_p::SocketHandle_t const p_sock)
{
	return close(p_sock);
}

static inline NET_Error Core_createSocket(_p::SocketHandle_t& p_sock, int const p_af, int const p_domain, int const p_protocol, bool const p_blocking)
{
	p_sock = socket(p_af, p_domain | (p_blocking ? SOCK_CLOEXEC : SOCK_CLOEXEC | SOCK_NONBLOCK), p_protocol);
	if(p_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return NET_Error::NoErr;
}

static inline NET_Error Core_CreateUDPSocketIPv4(_p::SocketHandle_t& p_sock, bool const p_blocking)
{
	return Core_createSocket(p_sock, AF_INET, SOCK_DGRAM, IPPROTO_UDP, p_blocking);
}

static inline NET_Error Core_CreateUDPSocketIPv6(_p::SocketHandle_t& p_sock, bool const p_blocking)
{
	return Core_createSocket(p_sock, AF_INET6, SOCK_DGRAM, IPPROTO_UDP, p_blocking);
}

static inline NET_Error Core_Shutdown(_p::SocketHandle_t const p_sock, _p::Net_Socket::Endpoint const p_direction)
{
	//this optimization is only aforder because documentation states that Read=0, Write=1 amd ReadWrite=2
	static_assert(_p::Net_Socket::Endpoint::Receive	== _p::Net_Socket::Endpoint{1});
	static_assert(_p::Net_Socket::Endpoint::Send	== _p::Net_Socket::Endpoint{2});
	static_assert(_p::Net_Socket::Endpoint::Both	== _p::Net_Socket::Endpoint{3});
	return shutdown(p_sock, static_cast<uint8_t>(p_direction) - 1) ? NET_Error::Fail : NET_Error::NoErr;
}

#if 0
static inline NET_Error Core_Shutdown(_p::SocketHandle_t const p_sock, _p::Net_Socket::Endpoint const p_direction)
{
	switch(p_direction)
	{
		case _p::Net_Socket::Endpoint_Receive:
			return shutdown(p_sock, SHUT_RD) ? NET_Error::Fail : NET_Error::NoErr;
		case _p::Net_Socket::Endpoint_Send:
			return shutdown(p_sock, SHUT_WR) ? NET_Error::Fail : NET_Error::NoErr;
		case _p::Net_Socket::Endpoint_Both:
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


static inline NET_Error Core_PeekSize(_p::SocketHandle_t const p_sock, uintptr_t& p_size)
{
	int		size = 0;
	char	c[1];

	ssize_t	const check = recvfrom(p_sock, c, 0, MSG_PEEK, nullptr, nullptr);
	if(check == SOCKET_ERROR)
	{
		if(SockWouldBlock(p_sock))	return NET_Error::WouldBlock;
		return NET_Error::Connection;
	}

	ioctl(p_sock, FIONREAD, &size);
	p_size = static_cast<uint32_t>(size);
	return NET_Error::NoErr;
}

static inline NET_Error Core_PeekSizeIPv4(_p::SocketHandle_t const p_sock, uintptr_t& p_size, uint32_t& p_rawAddr, uint16_t& p_port)
{
	socklen_t	addr_size;
	int			size = 0;
	char		c[1];
	sockaddr_in	addr_info4;

	addr_size = sizeof(sockaddr_in);
	ssize_t const check = recvfrom(p_sock, c, 0, MSG_PEEK, reinterpret_cast<sockaddr*>(&addr_info4), &addr_size);

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

static inline NET_Error Core_PeekSizeIPv6(_p::SocketHandle_t const p_sock, uintptr_t& p_size, void* const p_rawAddr, uint16_t& p_port)
{
	socklen_t		addr_size;
	int				size = 0;
	char			c[1];
	sockaddr_in6	addr_info6;

	addr_size = sizeof(sockaddr_in6);
	ssize_t const check = recvfrom(p_sock, c, 0, MSG_PEEK, reinterpret_cast<sockaddr*>(&addr_info6), &addr_size);

	if(check == SOCKET_ERROR)
	{
		if(SockWouldBlock(p_sock))	return NET_Error::WouldBlock;
		return NET_Error::Connection;
	}

	ioctl(p_sock, FIONREAD, &size);

	p_size = size;
	memcpy(p_rawAddr, &addr_info6.sin6_addr, 16);
	p_port = core::endian_big2host(addr_info6.sin6_port);

	return NET_Error::NoErr;
}

#endif

static inline NET_Error Core_CreateTCPSocketIPv4(_p::SocketHandle_t& p_sock, bool const p_blocking)
{
	return Core_createSocket(p_sock, AF_INET, SOCK_STREAM, IPPROTO_TCP, p_blocking);
}

static inline NET_Error Core_CreateTCPSocketIPv6(_p::SocketHandle_t& p_sock, bool const p_blocking)
{
	return Core_createSocket(p_sock, AF_INET6, SOCK_STREAM, IPPROTO_TCP, p_blocking);
}

//========	========	========	========	========
//========	========	Get Address	========	========
//========	========	========	========	========

static inline NET_Error Core_GetAddressIPv4(_p::SocketHandle_t const p_sock, uint32_t& p_rawAddr, uint16_t& p_port)
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

static inline NET_Error Core_GetAddressIPv6(_p::SocketHandle_t const p_sock, void* const p_rawGroup, uint16_t& p_port)
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

static inline NET_Error Core_GetPeerAddressIPv4(_p::SocketHandle_t const p_sock, uint32_t& p_rawAddr, uint16_t& p_port)
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

static inline NET_Error Core_GetPeerAddressIPv6(_p::SocketHandle_t const p_sock, void* const p_rawGroup, uint16_t& p_port)
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

static inline NET_Error Core_ConnectIPv4(_p::SocketHandle_t const p_sock, uint32_t const p_rawAddr, uint16_t const p_port)
{
	sockaddr_in o_addr_info4;
	o_addr_info4.sin_family			= AF_INET;
	o_addr_info4.sin_addr.s_addr	= p_rawAddr;
	o_addr_info4.sin_port			= core::endian_host2big(p_port);
	if(connect(p_sock, reinterpret_cast<sockaddr const*>(&o_addr_info4), sizeof(sockaddr_in)) != 0)
	{
		if(SockNonBlockingConnectCheck(p_sock)) return NET_Error::WouldBlock;
		return NET_Error::Connection;
	}
	return NET_Error::NoErr;
}

static inline NET_Error Core_ConnectIPv6(_p::SocketHandle_t const p_sock, std::span<uint8_t const, 16> const p_rawAddr, uint16_t const p_port)
{
	sockaddr_in6 o_addr_info6{};
	memcpy(&o_addr_info6.sin6_addr, p_rawAddr.data(), 16);
	o_addr_info6.sin6_family	= AF_INET6;
	o_addr_info6.sin6_port		= core::endian_host2big(p_port);

	if(connect(p_sock, reinterpret_cast<sockaddr const*>(&o_addr_info6), sizeof(sockaddr_in6)) != 0)
	{
		if(SockNonBlockingConnectCheck(p_sock)) return NET_Error::WouldBlock;
		return NET_Error::Connection;
	}
	return NET_Error::NoErr;
}

static inline NET_Error Core_AcceptIPv4(_p::SocketHandle_t const p_sock, _p::SocketHandle_t& p_connection, bool const p_blocking, sockaddr_in& p_info)
{
	_p::SocketHandle_t sock;
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

static inline NET_Error Core_AcceptIPv6(_p::SocketHandle_t const p_sock, _p::SocketHandle_t& p_connection, bool const p_blocking, sockaddr_in6& p_info)
{
	_p::SocketHandle_t sock;
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

static inline NET_Error Core_Send_size(_p::SocketHandle_t const p_sock, void const* const p_buffer, uintptr_t p_size, uintptr_t& p_sent)
{
#ifdef _WIN32
	if(p_size > static_cast<uintptr_t>(std::numeric_limits<int>::max())) p_size = static_cast<uintptr_t>(std::numeric_limits<int>::max());
	int const ret = send(p_sock, reinterpret_cast<char const*>(p_buffer), static_cast<int>(p_size), 0);
#else
	intptr_t const ret = send(p_sock, reinterpret_cast<char const*>(p_buffer), p_size, 0);
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

static inline NET_Error Core_Send_context(_p::SocketHandle_t const p_sock, void const* p_buffer, uintptr_t const p_size, uintptr_t& p_context)
{
	if(p_context >= p_size) return NET_Error::Invalid_Option;

	uintptr_t currentSize = p_size - p_context;

#ifdef _WIN32
	if(currentSize > static_cast<uintptr_t>(std::numeric_limits<int>::max())) currentSize = static_cast<uintptr_t>(std::numeric_limits<int>::max());
	int const ret = send(p_sock, reinterpret_cast<char const*>(p_buffer) + p_context, static_cast<int>(currentSize), 0);
#else
	intptr_t const ret = send(p_sock, reinterpret_cast<char const*>(p_buffer) + p_context, currentSize, 0);
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

static inline NET_Error Core_Receive_size(_p::SocketHandle_t const p_sock, void* const p_buffer, uintptr_t p_size, uintptr_t& p_received)
{
#ifdef _WIN32
	if(p_size > static_cast<uintptr_t>(std::numeric_limits<int>::max())) p_size =static_cast<uintptr_t>( std::numeric_limits<int>::max());
	int const ret = recv(p_sock, reinterpret_cast<char* const>(p_buffer), static_cast<int>(p_size), 0);
#else
	intptr_t const ret = recv(p_sock, reinterpret_cast<char* const>(p_buffer), p_size, 0);
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

static inline NET_Error Core_Receive_context(_p::SocketHandle_t const p_sock, void* const p_buffer, uintptr_t const p_size, uintptr_t& p_context)
{
	if(p_context >= p_size) return NET_Error::Invalid_Option;

	uintptr_t currentSize = p_size - p_context;
#ifdef _WIN32
	if(currentSize > static_cast<uintptr_t>(std::numeric_limits<int>::max())) currentSize = static_cast<uintptr_t>(std::numeric_limits<int>::max());
	int const ret = recv(p_sock, reinterpret_cast<char*>(p_buffer) + p_context, static_cast<int>(currentSize), 0);
#else
	intptr_t const ret = recv(p_sock, reinterpret_cast<char*>(p_buffer) + p_context, currentSize, 0);
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

static inline NET_Error Core_SendToIPv4(_p::SocketHandle_t const p_sock, void const* p_data, uintptr_t const p_size, uint32_t const p_rawAddr, uint16_t const p_port, uint8_t const p_repeat)
{
	uint8_t		count = 0;
	sockaddr_in	addr_info4;
	bool		sentOnce = false;

#ifdef _WIN32
	if(p_size > static_cast<uintptr_t>(std::numeric_limits<int>::max())) return NET_Error::Buffer_Full;
#endif

	addr_info4.sin_family		= AF_INET;
	addr_info4.sin_addr.s_addr	= p_rawAddr;
	addr_info4.sin_port			= core::endian_host2big(p_port);

	do
	{
#ifdef _WIN32
		if(sendto(p_sock, reinterpret_cast<char const*>(p_data), static_cast<int>(p_size), 0, reinterpret_cast<sockaddr*>(&addr_info4), sizeof(addr_info4)) == SOCKET_ERROR)
#else
		if(sendto(p_sock, reinterpret_cast<char const*>(p_data), p_size, 0, reinterpret_cast<sockaddr const*>(&addr_info4), sizeof(addr_info4)) == SOCKET_ERROR)
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

static inline NET_Error Core_SendToIPv6(_p::SocketHandle_t const p_sock, void const* const p_data, uintptr_t p_size, std::span<uint8_t const, 16> p_rawAddr, uint16_t const p_port, uint8_t const p_repeat)
{
	uint8_t			count = 0;
	sockaddr_in6	addr_info6{};
	bool			sentOnce = false;

#ifdef _WIN32
	if(p_size > static_cast<uintptr_t>(std::numeric_limits<int>::max())) return NET_Error::Buffer_Full;
#endif

	memcpy(&addr_info6.sin6_addr, p_rawAddr.data(), 16);
	addr_info6.sin6_family	= AF_INET6;
	addr_info6.sin6_port	= endian_host2big(p_port);

	do
	{
#ifdef _WIN32
		if(sendto(p_sock, reinterpret_cast<char const*>(p_data), static_cast<int>(p_size), 0, reinterpret_cast<sockaddr const*>(&addr_info6), sizeof(addr_info6)) == SOCKET_ERROR)
#else
		if(sendto(p_sock, reinterpret_cast<char const*>(p_data), p_size, 0, reinterpret_cast<sockaddr const*>(&addr_info6), sizeof(addr_info6)) == SOCKET_ERROR)
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

static inline NET_Error Core_ReceiveFrom(_p::SocketHandle_t const p_sock, void* const p_data, uintptr_t& p_size)
{
#ifdef _WIN32
	uintptr_t size = p_size;
	if(size > static_cast<uintptr_t>(std::numeric_limits<int>::max())) size = static_cast<uintptr_t>(std::numeric_limits<int>::max());
	int const check = recvfrom(p_sock, reinterpret_cast<char*>(p_data), static_cast<int>(size), 0, nullptr, nullptr);
#else
	intptr_t const check = recvfrom(p_sock, reinterpret_cast<char*>(p_data), p_size, 0, nullptr, nullptr);
#endif


	if(SockWouldBlock(p_sock)) return NET_Error::WouldBlock;

	if(check == SOCKET_ERROR)
	{
		return NET_Error::Connection;
	}
	p_size = check;
	return NET_Error::NoErr;
}

static inline NET_Error Core_ReceiveFromIPv4(_p::SocketHandle_t const p_sock, void* const p_data, uintptr_t& p_size, uint32_t& p_rawAddr, uint16_t& p_port)
{
	sockaddr_in		addr_info4{};
	CoreSockLen_t	addr_size = sizeof(sockaddr_in);

#ifdef _WIN32
	uintptr_t size = p_size;
	if(size > static_cast<uintptr_t>(std::numeric_limits<int32_t>::max())) size = static_cast<uintptr_t>(std::numeric_limits<int32_t>::max());
	int const check = recvfrom(p_sock, reinterpret_cast<char*>(p_data), static_cast<int>(size), 0, reinterpret_cast<sockaddr*>(&addr_info4), &addr_size);
#else
	intptr_t const check = recvfrom(p_sock, reinterpret_cast<char*>(p_data), p_size, 0, reinterpret_cast<sockaddr*>(&addr_info4), &addr_size);
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

static inline NET_Error Core_ReceiveFromIPv6(_p::SocketHandle_t const p_sock, void* const p_data, uintptr_t& p_size, void* const p_rawAddr, uint16_t& p_port)
{
	sockaddr_in6	addr_info6{};
	CoreSockLen_t	addr_size = sizeof(sockaddr_in6);

#ifdef _WIN32
	uintptr_t size = p_size;
	if(size > static_cast<uintptr_t>(std::numeric_limits<int>::max())) size = static_cast<uintptr_t>(std::numeric_limits<int>::max());
	int const check = recvfrom(p_sock, reinterpret_cast<char*>(p_data), static_cast<int>(size), 0, reinterpret_cast<sockaddr*>(&addr_info6), &addr_size);
#else
	intptr_t const check = recvfrom(p_sock, reinterpret_cast<char*>(p_data), p_size, 0, reinterpret_cast<sockaddr*>(&addr_info6), &addr_size);
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

static inline void Core_PrepareWOLPacket(std::span<uint8_t, 102> const p_payload, std::span<uint8_t const, 6> const p_MacAddress)
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

static inline NET_Error Core_WakeOnLanIPv4(_p::SocketHandle_t const p_sock, std::span<uint8_t const, 6> const p_MacAddress, uint32_t const p_rawAddr, uint16_t const p_port)
{
	uint8_t payload[102];
	Core_PrepareWOLPacket(payload, p_MacAddress);

	return Core_SendToIPv4(p_sock, payload, 102, p_rawAddr, p_port, 0);
}

static inline NET_Error Core_WakeOnLanIPv6(_p::SocketHandle_t const p_sock, std::span<uint8_t const, 6> const p_MacAddress, std::span<uint8_t const, 16> const p_rawAddr, uint16_t const p_port)
{
	uint8_t payload[102];
	Core_PrepareWOLPacket(payload, p_MacAddress);
	return Core_SendToIPv6(p_sock, payload, 102, p_rawAddr, p_port, 0);
}


//WOL typically port 7 or 9
static inline NET_Error Core_WakeOnLanIPv4(_p::SocketHandle_t const p_sock, std::span<uint8_t const, 6> const p_MacAddress, uint32_t const p_rawAddr, uint16_t const p_port, void const* const p_password, uint16_t const p_password_size)
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

	Core_PrepareWOLPacket(std::span<uint8_t, 102>{payload, 102}, p_MacAddress);

	if(p_password_size)
	{
		memcpy(payload + 102, p_password, p_password_size);
	}

	return Core_SendToIPv4(p_sock, payload, payload_size, p_rawAddr, p_port, 0);
}

static inline NET_Error Core_WakeOnLanIPv6(_p::SocketHandle_t const p_sock, std::span<uint8_t const, 6> const p_MacAddress, std::span<uint8_t const, 16> const p_rawAddr, uint16_t const p_port, void const* const p_password, uint16_t const p_password_size)
{
	uint16_t payload_size = 102;
	uint8_t payload[CORE_NET_MAX_DATA_LEN];

	if(p_password_size)
	{
		if(p_password == nullptr || (p_password_size > CORE_NET_MAX_DATA_LEN - 103))
		{
			return NET_Error::Invalid_Option;
		}
		payload_size += p_password_size;
	}

	Core_PrepareWOLPacket(std::span<uint8_t, 102>{payload, 102}, p_MacAddress);

	if(p_password_size)
	{
		memcpy(payload + 102, p_password, p_password_size);
	}

	return Core_SendToIPv6(p_sock, payload, payload_size, p_rawAddr, p_port, 0);
}



namespace _p
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

bool Net_Socket::is_open() const
{
	return m_sock != INVALID_SOCKET;
}

NET_Error Net_Socket::close()
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

NET_Error Net_Socket::poll(uint64_t const p_microseconds)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_Poll(m_sock, p_microseconds);
}

NET_Error Net_Socket::shutdown(Endpoint const p_direction)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_Shutdown(m_sock, p_direction);
}

NET_Error Net_Socket::set_blocking(bool const p_blocking)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_setSockBlocking(m_sock, p_blocking);
}

NET_Error Net_Socket::set_reuse_address(bool const p_reuse)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_setReuseAddress(m_sock, p_reuse);
}

NET_Error Net_Socket::set_linger(bool const p_linger, uint16_t const p_timeout)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_setSockLinger(m_sock, p_linger, p_timeout);
}


//========= ======== ======== NetUDP_p ========= ======== ========

NET_Error NetUDP_p::set_broadcasting(bool const p_broadcast)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_setBrodCasting(m_sock, p_broadcast);
}

NET_Error NetUDP_p::receive(void* const p_data, uintptr_t& p_size)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_ReceiveFrom(m_sock, p_data, p_size);
}

NET_Error NetUDP_p::peek_size(uintptr_t& p_size)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_PeekSize(m_sock, p_size);
}


//========= ======== ======== NetTCP_S_p ========= ======== ========

NET_Error NetTCP_S_p::listen(int const p_max_connections)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;

	if(::listen(m_sock, p_max_connections))
	{
		return NET_Error::Sock_Listen;
	}
	return NET_Error::NoErr;
}


//========= ======== ======== NetTCP_C_p ========= ======== ========

NET_Error NetTCP_C_p::nonblock_connect_state()
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_TCP_NonBlock_Connect_state(m_sock);
}

NET_Error NetTCP_C_p::send_context(void const* const p_buffer, uintptr_t const p_size, uintptr_t& p_context)
{
	return Core_Send_context(m_sock, p_buffer, p_size, p_context);
}

NET_Error NetTCP_C_p::send_size(void const* const p_buffer, uintptr_t const p_size, uintptr_t& p_sent)
{
	return Core_Send_size(m_sock, p_buffer, p_size, p_sent);
}

NET_Error NetTCP_C_p::receive_context(void* const p_buffer, uintptr_t const p_size, uintptr_t& p_context)
{
	return Core_Receive_context(m_sock, p_buffer, p_size, p_context);
}

NET_Error NetTCP_C_p::receive_size(void* const p_buffer, uintptr_t const p_size, uintptr_t& p_received)
{
	return Core_Receive_size(m_sock, p_buffer, p_size, p_received);
}

NET_Error NetTCP_C_p::set_nagle(bool const p_useNagle)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_setNagle(m_sock, p_useNagle);
}

NET_Error NetTCP_C_p::set_keep_alive(bool const p_keepAlive, uint32_t const p_probePeriod, uint32_t const p_maxProbes)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_SetKeepAlive(m_sock, p_keepAlive, p_probePeriod, p_maxProbes);
}

} //namesapce _p


  //========	========	========	========	========
  //========	========	UDP			========	========
  //========	========	========	========	========

  //========= ======== ======== NetUDP_V4 ========= ======== ========

NetUDP_V4::NetUDP_V4(NetUDP_V4&& p_other): NetUDP_V4()
{
	std::swap(m_sock, p_other.m_sock);
}

NET_Error NetUDP_V4::open(bool const p_blocking)
{
	if(m_sock != INVALID_SOCKET) return NET_Error::Already_Used;
	return Core_CreateUDPSocketIPv4(m_sock, p_blocking);
}

NET_Error NetUDP_V4::bind(IPv4_address const& p_IP, uint16_t const p_Port)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_BindIPv4(m_sock, p_IP.ui32Type, p_Port);
}

NET_Error NetUDP_V4::open_bind(IPv4_address const& p_IP, uint16_t const p_Port, bool const p_blocking)
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

NET_Error NetUDP_V4::join_multicast_group(IPv4_address const& p_group, uint32_t const p_interface)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_JoinMulticastGroupIPv4(m_sock, p_group.ui32Type, p_interface);
}

NET_Error NetUDP_V4::leave_multicast_group(IPv4_address const& p_group, uint32_t const p_interface)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_LeaveMulticastGroupIPv4(m_sock, p_group.ui32Type, p_interface);
}

NET_Error NetUDP_V4::get_address(IPv4_address& p_IP, uint16_t& p_Port)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_GetAddressIPv4(m_sock, p_IP.ui32Type, p_Port);
}

NET_Error NetUDP_V4::send(void const* const p_data, uintptr_t const p_size, IPv4_address const& p_IP, uint16_t const p_Port, uint8_t const p_repeat)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_SendToIPv4(m_sock, p_data, p_size, p_IP.ui32Type, p_Port, p_repeat);
}

NET_Error NetUDP_V4::receive(void* const p_data, uintptr_t& p_size, IPv4_address& p_other_IP, uint16_t& p_other_port)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_ReceiveFromIPv4(m_sock, p_data, p_size, p_other_IP.ui32Type, p_other_port);
}

NET_Error NetUDP_V4::peek_size(uintptr_t& p_size, IPv4_address& p_other_IP, uint16_t& p_other_port)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_PeekSizeIPv4(m_sock, p_size, p_other_IP.ui32Type, p_other_port);
}

NET_Error NetUDP_V4::WakeOnLan(std::span<uint8_t const, 6> const p_MacAddress, IPv4_address const& p_subNet, uint16_t const p_port)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_WakeOnLanIPv4(m_sock, p_MacAddress, p_subNet.ui32Type, p_port);
}

NET_Error NetUDP_V4::WakeOnLan_password(std::span<uint8_t const, 6> const p_MacAddress, IPv4_address const& p_subNet, uint16_t const p_port, void const* const p_password, uint16_t const p_password_size)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_WakeOnLanIPv4(m_sock, p_MacAddress, p_subNet.ui32Type, p_port, p_password, p_password_size);
}


//========= ======== ======== NetUDP_V6 ========= ======== ========

NetUDP_V6::NetUDP_V6(NetUDP_V6&& p_other): NetUDP_V6()
{
	std::swap(m_sock, p_other.m_sock);
}

NET_Error NetUDP_V6::open(bool const p_blocking)
{
	if(m_sock != INVALID_SOCKET) return NET_Error::Already_Used;
	return Core_CreateUDPSocketIPv6(m_sock, p_blocking);
}

NET_Error NetUDP_V6::bind(IPv6_address const& p_IP, uint16_t const p_Port)
{
	return Core_BindIPv6(m_sock, p_IP.byteField, p_Port);
}

NET_Error NetUDP_V6::open_bind(IPv6_address const& p_IP, uint16_t const p_Port, bool const p_blocking)
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

NET_Error NetUDP_V6::join_multicast_group(IPv6_address const& p_group, uint32_t const p_interface)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_JoinMulticastGroupIPv6(m_sock, p_group.byteField, p_interface);
}

NET_Error NetUDP_V6::leave_multicast_group(IPv6_address const& p_group, uint32_t const p_interface)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_LeaveMulticastGroupIPv6(m_sock, p_group.byteField, p_interface);
}

NET_Error NetUDP_V6::get_address(IPv6_address& p_IP, uint16_t& p_Port)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_GetAddressIPv6(m_sock, p_IP.byteField, p_Port);
}

NET_Error NetUDP_V6::send(void const* const p_data, uintptr_t const p_size, IPv6_address const& p_IP, uint16_t const p_Port, uint8_t const p_repeat)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_SendToIPv6(m_sock, p_data, p_size, p_IP.byteField, p_Port, p_repeat);
}

NET_Error NetUDP_V6::receive(void* const p_data, uintptr_t& p_size, IPv6_address& p_other_IP, uint16_t& p_other_port)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_ReceiveFromIPv6(m_sock, p_data, p_size, p_other_IP.byteField, p_other_port);
}

NET_Error NetUDP_V6::peek_size(uintptr_t& p_size, IPv6_address& p_other_IP, uint16_t& p_other_port)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_PeekSizeIPv6(m_sock, p_size, p_other_IP.byteField, p_other_port);
}

NET_Error NetUDP_V6::WakeOnLan(std::span<uint8_t const, 6> const p_MacAddress, IPv6_address const& p_subNet, uint16_t const p_port)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_WakeOnLanIPv6(m_sock, p_MacAddress, p_subNet.byteField, p_port);
}

NET_Error NetUDP_V6::WakeOnLan_password(std::span<uint8_t const, 6> const p_MacAddress, IPv6_address const& p_subNet, uint16_t const p_port, void const* const p_password, uint16_t const p_password_size)
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

NET_Error NetTCP_S_V4::open(bool const p_blocking)
{
	if(m_sock != INVALID_SOCKET) return NET_Error::Already_Used;
	return Core_CreateTCPSocketIPv4(m_sock, p_blocking);
}

NET_Error NetTCP_S_V4::bind(IPv4_address const& p_IP, uint16_t const p_Port)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_BindIPv4(m_sock, p_IP.ui32Type, p_Port);
}

NET_Error NetTCP_S_V4::open_bind(IPv4_address const& p_IP, uint16_t const p_Port, bool const p_blocking)
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

NET_Error NetTCP_S_V4::open_bind_listen(IPv4_address const& p_IP, uint16_t const p_Port, int const p_max_connections, bool const p_blocking)
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
	if(::listen(m_sock, p_max_connections))
	{
		Core_CloseSock(m_sock);
		m_sock = INVALID_SOCKET;
		return NET_Error::Sock_Listen;
	}
	return NET_Error::NoErr;
}

NET_Error NetTCP_S_V4::accept(NetTCP_C_V4& p_Client, bool const p_blocking)
{
	if(m_sock == INVALID_SOCKET)	return NET_Error::Invalid_Socket;
	if(p_Client.is_open())			return NET_Error::Already_Used;

	sockaddr_in addr_info4;
	return Core_AcceptIPv4(m_sock, p_Client.m_sock, p_blocking, addr_info4);
}

NET_Error NetTCP_S_V4::accept(NetTCP_C_V4& p_Client, IPv4_address& p_other_IP, uint16_t& p_other_port, bool const p_blocking)
{
	if(m_sock == INVALID_SOCKET)	return NET_Error::Invalid_Socket;
	if(p_Client.is_open())			return NET_Error::Already_Used;

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

NET_Error NetTCP_S_V4::get_address(IPv4_address& p_IP, uint16_t& p_Port)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_GetAddressIPv4(m_sock, p_IP.ui32Type, p_Port);
}


//========= ======== ======== NetTCP_S_V6 ========= ======== ========

NetTCP_S_V6::NetTCP_S_V6(NetTCP_S_V6&& p_other): NetTCP_S_V6()
{
	std::swap(m_sock, p_other.m_sock);
}

NET_Error NetTCP_S_V6::open(bool const p_blocking)
{
	if(m_sock != INVALID_SOCKET) return NET_Error::Already_Used;
	return Core_CreateTCPSocketIPv6(m_sock, p_blocking);
}

NET_Error NetTCP_S_V6::bind(IPv6_address const& p_IP, uint16_t const p_Port)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_BindIPv6(m_sock, p_IP.byteField, p_Port);
}

NET_Error NetTCP_S_V6::open_bind(IPv6_address const& p_IP, uint16_t const p_Port, bool const p_blocking)
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

NET_Error NetTCP_S_V6::open_bind_listen(IPv6_address const& p_IP, uint16_t const p_Port, int const p_max_connections, bool const p_blocking)
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
	if(::listen(m_sock, p_max_connections))
	{
		Core_CloseSock(m_sock);
		m_sock = INVALID_SOCKET;
		return NET_Error::Sock_Listen;
	}
	return NET_Error::NoErr;
}

NET_Error NetTCP_S_V6::accept(NetTCP_C_V6& p_Client, bool const p_blocking)
{
	if(m_sock == INVALID_SOCKET)	return NET_Error::Invalid_Socket;
	if(p_Client.is_open())			return NET_Error::Already_Used;

	sockaddr_in6 addr_info6;
	return Core_AcceptIPv6(m_sock, p_Client.m_sock, p_blocking, addr_info6);
}

NET_Error NetTCP_S_V6::accept(NetTCP_C_V6& p_Client, IPv6_address& p_other_IP, uint16_t& p_other_port, bool const p_blocking)
{
	if(m_sock == INVALID_SOCKET)	return NET_Error::Invalid_Socket;
	if(p_Client.is_open())			return NET_Error::Already_Used;

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

NET_Error NetTCP_S_V6::get_address(IPv6_address& p_IP, uint16_t& p_Port)
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

NET_Error NetTCP_C_V4::open(bool const p_blocking)
{
	if(m_sock != INVALID_SOCKET)	return NET_Error::Invalid_Socket;
	return Core_CreateTCPSocketIPv4(m_sock, p_blocking);
}

NET_Error NetTCP_C_V4::bind(IPv4_address const& my_IP, uint16_t const my_Port)
{
	if(m_sock == INVALID_SOCKET)	return NET_Error::Invalid_Socket;
	return Core_BindIPv4(m_sock, my_IP.ui32Type, my_Port);
}

NET_Error NetTCP_C_V4::connect(IPv4_address const& dest_IP, uint16_t const dest_Port)
{
	if(m_sock == INVALID_SOCKET)	return NET_Error::Invalid_Socket;
	if(dest_IP.ui32Type == 0)		return NET_Error::Invalid_IP;
	return Core_ConnectIPv4(m_sock, dest_IP.ui32Type, dest_Port);
}

NET_Error NetTCP_C_V4::open_bind(IPv4_address const& my_IP, uint16_t const my_Port, bool const p_blocking)
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

NET_Error NetTCP_C_V4::open_bind_connect(IPv4_address const& my_IP, uint16_t const my_Port, IPv4_address const& dest_IP, uint16_t const dest_Port, bool const p_blocking)
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

NET_Error NetTCP_C_V4::get_address(IPv4_address& p_IP, uint16_t& p_Port)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_GetAddressIPv4(m_sock, p_IP.ui32Type, p_Port);
}

NET_Error NetTCP_C_V4::get_peer_address(IPv4_address& p_IP, uint16_t& p_Port)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_GetPeerAddressIPv4(m_sock, p_IP.ui32Type, p_Port);
}


//========= ======== ======== NetTCP_C_V6 ========= ======== ========

NetTCP_C_V6::NetTCP_C_V6(NetTCP_C_V6&& p_other): NetTCP_C_V6()
{
	std::swap(m_sock, p_other.m_sock);
}

NET_Error NetTCP_C_V6::open(bool const p_blocking)
{
	if(m_sock != INVALID_SOCKET)	return NET_Error::Invalid_Socket;
	return Core_CreateTCPSocketIPv6(m_sock, p_blocking);
}

NET_Error NetTCP_C_V6::bind(IPv6_address const& my_IP, uint16_t const my_Port)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_BindIPv6(m_sock, my_IP.byteField, my_Port);
}

NET_Error NetTCP_C_V6::connect(IPv6_address const& dest_IP, uint16_t const dest_Port)
{
	if(m_sock == INVALID_SOCKET)	return NET_Error::Invalid_Socket;
	if(dest_IP.ui64Type[0] == 0 && dest_IP.ui64Type[1] == 0) return NET_Error::Invalid_IP;

	return Core_ConnectIPv6(m_sock, std::span<uint8_t const, 16>{dest_IP.byteField, 16}, dest_Port);
}

NET_Error NetTCP_C_V6::open_bind(IPv6_address const& my_IP, uint16_t const my_Port, bool const p_blocking)
{
	if(m_sock != INVALID_SOCKET) return NET_Error::Already_Used;

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

NET_Error NetTCP_C_V6::open_bind_connect(IPv6_address const& my_IP, uint16_t const my_Port, IPv6_address const& dest_IP, uint16_t const dest_Port, bool const p_blocking)
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

NET_Error NetTCP_C_V6::get_address(IPv6_address& p_IP, uint16_t& p_Port)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	return Core_GetAddressIPv6(m_sock, p_IP.byteField, p_Port);
}

NET_Error NetTCP_C_V6::get_peer_address(IPv6_address& p_IP, uint16_t& p_Port)
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

NET_Error NetUDP::close()
{
	NET_Error const ret = NetUDP_p::close();
	if(ret != NET_Error::NoErr) return ret;
	m_IpV = IPv::None;
	return NET_Error::NoErr;
}

NET_Error NetUDP::open(IPv const p_ipV, bool const p_blocking)
{
	if(m_sock != INVALID_SOCKET) return NET_Error::Already_Used;

	if(p_ipV == IPv::IPv_4)
	{
		NET_Error const ret = Core_CreateUDPSocketIPv4(m_sock, p_blocking);
		if(ret != NET_Error::NoErr) return ret;
		m_IpV = IPv::IPv_4;
		return NET_Error::NoErr;
	}
	else if(p_ipV == IPv::IPv_6)
	{
		NET_Error const ret = Core_CreateUDPSocketIPv6(m_sock, p_blocking);
		if(ret != NET_Error::NoErr) return ret;
		m_IpV = IPv::IPv_6;
		return NET_Error::NoErr;
	}

	return NET_Error::Invalid_Option;
}

NET_Error NetUDP::bind(IP_address const& p_IP, uint16_t const p_Port)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	IPv const ver = p_IP.version();

	if(ver != m_IpV) return NET_Error::Incompatible_Protocol;

	if(ver == IPv::IPv_4)
	{
		return Core_BindIPv4(m_sock, p_IP.v4.ui32Type, p_Port);
	}
	//else if(ver == IPv::IPv_6);

	return Core_BindIPv6(m_sock, p_IP.v6.byteField, p_Port);

}

NET_Error NetUDP::open_bind(IP_address const& p_IP, uint16_t const p_Port, bool const p_blocking)
{
	if(m_sock != INVALID_SOCKET) return NET_Error::Already_Used;
	IPv const ver = p_IP.version();

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

NET_Error NetUDP::join_multicast_group(IP_address const& p_group, uint32_t const p_interface)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	IPv const ver = p_group.version();

	if(ver != m_IpV) return NET_Error::Incompatible_Protocol;

	if(ver == IPv::IPv_4)
	{
		return Core_JoinMulticastGroupIPv4(m_sock, p_group.v4.ui32Type, p_interface);
	}
	//else if(ver == IPv::IPv_6);

	return Core_JoinMulticastGroupIPv6(m_sock, p_group.v6.byteField, p_interface);
}

NET_Error NetUDP::leave_multicast_group(IP_address const& p_group, uint32_t const p_interface)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	IPv const ver = p_group.version();

	if(ver != m_IpV) return NET_Error::Incompatible_Protocol;

	if(ver == IPv::IPv_4)
	{
		return Core_LeaveMulticastGroupIPv4(m_sock, p_group.v4.ui32Type, p_interface);
	}
	//else if(ver == IPv::IPv_6);

	return Core_LeaveMulticastGroupIPv6(m_sock, p_group.v6.byteField, p_interface);
}

NET_Error NetUDP::get_address(IP_address& p_IP, uint16_t& p_Port)
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

NET_Error NetUDP::send(void const* const p_data, uintptr_t const p_size, IP_address const& p_IP, uint16_t const p_Port, uint8_t const p_repeat)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	IPv const ver = p_IP.version();
	if(ver != m_IpV) return NET_Error::Incompatible_Protocol;

	if(ver == IPv::IPv_4)
	{
		return Core_SendToIPv4(m_sock, p_data, p_size, p_IP.v4.ui32Type, p_Port, p_repeat);
	}
	//else if(ver == IPv::IPv_6);

	return Core_SendToIPv6(m_sock, p_data, p_size, p_IP.v6.byteField, p_Port, p_repeat);
}

NET_Error NetUDP::receive(void* const p_data, uintptr_t& p_size, IP_address& p_other_IP, uint16_t& p_other_port)
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

NET_Error NetUDP::peek_size(uintptr_t& p_size, IP_address& p_other_IP, uint16_t& p_other_port)
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

NET_Error NetUDP::WakeOnLan(std::span<uint8_t const, 6> const p_MacAddress, IP_address const& p_subNet, uint16_t const p_port)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	IPv const ver = p_subNet.version();

	if(ver != m_IpV) return NET_Error::Incompatible_Protocol;

	if(ver == IPv::IPv_4)
	{
		return Core_WakeOnLanIPv4(m_sock, p_MacAddress, p_subNet.v4.ui32Type, p_port);
	}
	return Core_WakeOnLanIPv6(m_sock, p_MacAddress, p_subNet.v6.byteField, p_port);
}

NET_Error NetUDP::WakeOnLan_password(std::span<uint8_t const, 6> const p_MacAddress, IP_address const& p_subNet, uint16_t const p_port, void const* const p_password, uint16_t const p_password_size)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	IPv const ver = p_subNet.version();

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

NET_Error NetTCP_S::close()
{
	NET_Error ret = NetTCP_S_p::close();
	if(ret != NET_Error::NoErr) return ret;
	m_IpV = IPv::None;
	return NET_Error::NoErr;
}

NET_Error NetTCP_S::open(IPv const p_ipV, bool const p_blocking)
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

NET_Error NetTCP_S::bind(IP_address const& p_IP, uint16_t const p_Port)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;
	IPv const ver = p_IP.version();

	if(ver != m_IpV) return NET_Error::Incompatible_Protocol;

	if(ver == IPv::IPv_4)
	{
		return Core_BindIPv4(m_sock, p_IP.v4.ui32Type, p_Port);
	}
	//else if(ver == IPv::IPv_6);

	return Core_BindIPv6(m_sock, p_IP.v6.byteField, p_Port);
}

NET_Error NetTCP_S::open_bind(IP_address const& p_IP, uint16_t const p_Port, bool const p_blocking)
{
	if(m_sock != INVALID_SOCKET) return NET_Error::Already_Used;
	IPv const ver = p_IP.version();

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

NET_Error NetTCP_S::open_bind_listen(IP_address const& p_IP, uint16_t const p_Port, int const p_max_connections, bool const p_blocking)
{
	if(m_sock != INVALID_SOCKET) return NET_Error::Already_Used;
	IPv const ver = p_IP.version();

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
		if(::listen(m_sock, p_max_connections))
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
		if(::listen(m_sock, p_max_connections))
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

NET_Error NetTCP_S::accept(NetTCP_C& p_Client, bool const p_blocking)
{
	if(m_sock == INVALID_SOCKET)	return NET_Error::Invalid_Socket;
	if(p_Client.is_open())			return NET_Error::Already_Used;

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

NET_Error NetTCP_S::accept(NetTCP_C& p_Client, IP_address& p_other_IP, uint16_t& p_other_port, bool const p_blocking)
{
	if(m_sock == INVALID_SOCKET)	return NET_Error::Invalid_Socket;
	if(p_Client.is_open())			return NET_Error::Already_Used;

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

NET_Error NetTCP_S::get_address(IP_address& p_IP, uint16_t& p_Port)
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

NET_Error NetTCP_C::close()
{
	NET_Error ret = NetTCP_C_p::close();
	if(ret != NET_Error::NoErr) return ret;
	m_IpV = IPv::None;
	return NET_Error::NoErr;
}


NET_Error NetTCP_C::open(IPv const p_ipV, bool const p_blocking)
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

NET_Error NetTCP_C::bind(IP_address const& my_IP, uint16_t const my_Port)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;

	IPv const ver = my_IP.version();
	if(ver != m_IpV) return NET_Error::Incompatible_Protocol;

	if(ver == IPv::IPv_4)
	{
		return Core_BindIPv4(m_sock, my_IP.v4.ui32Type, my_Port);
	}
	//else if(ver == IPv::IPv_6);

	return Core_BindIPv6(m_sock, my_IP.v6.byteField, my_Port);
}

NET_Error NetTCP_C::connect(IP_address const& dest_IP, uint16_t const dest_Port)
{
	if(m_sock == INVALID_SOCKET) return NET_Error::Invalid_Socket;

	IPv const ver = dest_IP.version();
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

NET_Error NetTCP_C::open_bind(IP_address const& my_IP, uint16_t const my_Port, bool const p_blocking)
{
	if(m_sock != INVALID_SOCKET) return NET_Error::Already_Used;
	IPv const ver = my_IP.version();

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

NET_Error NetTCP_C::open_bind_connect(IP_address const& my_IP, uint16_t const my_Port, IP_address const& dest_IP, uint16_t const dest_Port, bool const p_blocking)
{
	if(m_sock != INVALID_SOCKET) return NET_Error::Already_Used;
	IPv const ver	= my_IP		.version();
	IPv const ver2	= dest_IP	.version();

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

NET_Error NetTCP_C::get_address(IP_address& p_IP, uint16_t& p_Port)
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

NET_Error NetTCP_C::get_peer_address(IP_address& p_IP, uint16_t& p_Port)
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
} //namesapce core

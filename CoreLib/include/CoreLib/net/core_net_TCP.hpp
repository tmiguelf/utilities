//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///		Provides network interfaces
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
///
///	\todo Socket RAW
///	\todo Native socket options
///	\todo Auto memory management assist
//======== ======== ======== ======== ======== ======== ======== ========

#pragma once


#include "core_net_address.hpp"
#include "core_net_socket.hpp"


/// \n
namespace core
{
	/// \n
	namespace _p
	{
		///	\brief Private class to implement generic TCP server functionality
		class NetTCP_S_p: protected Net_Socket
		{
		protected:
			NetTCP_S_p() = default;

		public:
			using Net_Socket::is_open;
			using Net_Socket::set_blocking;
			using Net_Socket::set_linger;
			using Net_Socket::poll;
			using Net_Socket::shutdown;

			///	\brief Sets the socket into listening mode
			///	\param[in] p_max_connections - Number of connections allowed to be pending on the socket before starting to refuse them
			///	\return \ref core::NET_Error
			NET_Error listen(const int p_max_connections);
		};

		///	\brief Private class to implement generic TCP client functionality
		class NetTCP_C_p: protected Net_Socket
		{
		protected:
			NetTCP_C_p() = default;

		public:
			using Net_Socket::is_open;
			using Net_Socket::set_blocking;
			using Net_Socket::set_linger;
			using Net_Socket::poll;
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
			NET_Error nonblock_connect_state();

			///	\brief Sends data over the socket using a context algorithm to handle unsent data
			///	\param[in] p_buffer - p_buffer containing the data to send
			///	\param[in] p_size - amount of data in buffer
			///	\param[in,out] p_context - context tracking variable
			///	\return \ref core::NET_Error
			///
			///	\remarks
			///		On blocking sockets, this call will block until data is sent (which may be less than the amount requested).
			///		On a non-blocking socket this can return \ref core::NET_Error::WouldBlock indicating
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
			///		\ref core::NET_Error::NoErr and \ref p_context is 0, this means that all data was
			///		captured, and that the user is free to try and send new data.
			///		For example, if a user wants to sent 500B but only 200B are able to proccessed at the moment,
			///		before the call \ref p_context starts at 0 and becomes 200 after the call. When the user calls
			///		the send function again with \ref p_context at 200, the method will skip the first 200B of the buffer
			///		and only tries to send the remaining 300B left unsent, if on this call all remaing 300B are sent, then
			///		\ref p_context becomes 0 again.
			///		If an error is returned \ref p_context is left unchanged, so if this is 0 before calling,
			///		and remains 0 after the call but an error is returned, this means that no data was sent.
			NET_Error send_context(const void* p_buffer, const uintptr_t p_size, uintptr_t& p_context);

			///	\brief Sends data over the socket using a size tracking algorithm to handle unsent data
			///
			///	\param[in] p_buffer - p_buffer containing the data to send
			///	\param[in] p_size - amount of data in buffer
			///	\param[out] p_sent - amount of data actually sent
			///	\return \ref core::NET_Error
			///
			///	\remarks
			///		On blocking sockets, this call will block until data is sent (which may be less than the amount requested).
			///		On a non-blocking socket this can return \ref core::NET_Error::WouldBlock indicating
			///		that the request could not be serviced at the moment without blocking.
			///		When a data sent is requested on a TCP socket it is not guaranteed that the system
			///		is capable of processing the full amount of data, and the system can return having
			///		sent less data than what was actually requested. Thus it is necessary that users handles
			///		this conditions with specialized algorithms to make sure that the right data is sent.
			///		With a size type algorithm the user is responsible for keeping track of the data that was already
			///		sent and adjust the buffer for the next send call.
			///		If an error is returned \ref p_sent is left unchanged.
			NET_Error send_size(const void* p_buffer, const uintptr_t p_size, uintptr_t& p_sent);

			///	\brief Receives data pending on the socket using a context algorithm to handle unreceived data
			///	\param[in] p_buffer - p_buffer to receive the data
			///	\param[in] p_size - amount of data to receive
			///	\param[in,out] p_context - context tracking variable
			///	\return \ref core::NET_Error
			///
			///	\remarks
			///		On blocking sockets, this call will block until data is received (which may be less than the amount requested).
			///		On a non-blocking socket this can return \ref core::NET_Error::WouldBlock indicating
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
			///		\ref core::NET_Error::NoErr and \ref p_context is 0, this means that all requested data was
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
			NET_Error receive_context(void* p_buffer, const uintptr_t p_size, uintptr_t& p_context);

			///	\brief Receives data pending on the socket using a size tracking algorithm to handle unreceived data
			///	\param[in] p_buffer - p_buffer to receive the data
			///	\param[in] p_size - amount of data to receive
			///	\param[out] p_received - amount of data actually received
			///	\return \ref core::NET_Error
			///
			///	\remarks
			///		On blocking sockets, this call will block until data is received (which may be less than the amount requested).
			///		On a non-blocking socket this can return \ref core::NET_Error::WouldBlock indicating
			///		that the request could not be serviced at the moment without blocking.
			///		When a data receive is requested on a TCP socket it is not guaranteed that the system
			///		has the full amount pending on the socket, and can return having received less data than what was
			///		actually requested. Thus it is necessary that users handles this conditions with specialized algorithms
			///		to make sure that the right data is received.
			///		This type of algorithm is ideal in situations where the amount of data to be received is unknown.
			///		With a size type algorithm the user is responsible for keeping track of the data that was already
			///		received and adjust the buffer for the next send call.
			///		If an error is returned \ref p_received is left unchanged.
			NET_Error receive_size(void* p_buffer, const uintptr_t p_size, uintptr_t& p_received);

			///	\brief Turns on or off the Nagle's algorithm on the socket. By default the Nagle's algorithm is on.
			///	\param[in] p_useNagle - If true turns on the Nagle's algorithm, if false turns off the Nagle's algorithm
			///	\return \ref core::NET_Error
			///
			///	\remarks
			///		The Nagle's algorithm is a network optimization algorithm tha delays sending TCP data in case there is
			///		more data to send, allowing the system to concatenate messages togheter in the same packet thus reducing
			///		network traffic, but at the cost of a time delay.
			///		Please see: https://en.wikipedia.org/wiki/Nagle%27s_algorithm
			NET_Error set_nagle(const bool p_useNagle);

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
			NET_Error set_keep_alive(const bool p_keepAlive, const uint32_t p_probePeriod, const uint32_t p_maxProbes);
		};
	} //namesapce _p


	class NetTCP_C_V4;
	class NetTCP_C_V6;
	class NetTCP_C;

	/// \brief provides a TCP IPv4 server interface
	class NetTCP_S_V4: public _p::NetTCP_S_p
	{
	public:
		using Endpoint = _p::Net_Socket::Endpoint;

	public:
		NetTCP_S_V4() = default;
		NetTCP_S_V4(NetTCP_S_V4&& p_other);

		using Net_Socket::close;

		///	\brief Creates the socket
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		NET_Error open(const bool p_blocking = true);

		///	\brief Binds a previously created socket with \ref open to a specific network interface
		///	\param[in] p_IP - IP address of the interface to bind too. If 0.0.0.0 is used, then the socket is bound to any address
		///	\param[in] p_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\return \ref core::NET_Error
		NET_Error bind(const IPv4_address& p_IP, const uint16_t p_Port);

		///	\brief performs both a \ref open and \ref bind
		///	\param[in] p_IP - IP address of the interface to bind too. If 0.0.0.0 is used, then the socket is bound to any address
		///	\param[in] p_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		//
		NET_Error open_bind(const IPv4_address& p_IP, const uint16_t p_Port, bool p_blocking = true);

		///	\brief performs a \ref open, \ref bind, and \ref listen in one method
		///	\param[in] p_IP - IP address of the interface to bind too. If 0.0.0.0 is used, then the socket is bound to any address
		///	\param[in] p_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\param[in] p_max_connections - Number of connections allowed to be pending on the socket before starting to refuse them
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		NET_Error open_bind_listen(const IPv4_address& p_IP, const uint16_t p_Port, const int p_max_connections, const bool p_blocking = true);

		///	\brief Accepts a connection request pending on the socket.
		///	\param[out] p_Client - A client object that will manage the client communication. Object must be unused.
		///	\param[in] p_blocking - Determines if the new socket should be a blocking or non-blocking
		///	\return \ref core::NET_Error
		NET_Error accept(NetTCP_C_V4& p_Client, const bool p_blocking = true);

		///	\brief Accepts a connection request pending on the socket.
		///	\param[out] p_Client - A client object that will manage the client communication. Object must be unused.
		///	\param[out] p_other_IP - The peer's IP address.
		///	\param[out] p_other_port - The peer's port number
		///	\param[in] p_blocking - Determines if the new socket should be a blocking or non-blocking
		///	\return \ref core::NET_Error
		NET_Error accept(NetTCP_C_V4& p_Client, IPv4_address& p_other_IP, uint16_t& p_other_port, const bool p_blocking = true);

		///	\brief Gets the address information of the interface. Usefull if \ref bind is used without specifying the address.
		///	\param[in] p_IP - receives the IP address
		///	\param[in] p_Port - receives the port number
		///	\return \ref core::NET_Error
		NET_Error get_address(IPv4_address& p_IP, uint16_t& p_Port);

		///	\brief Swaps this socket with another
		void swap(NetTCP_S_V4& p_other);
	};

	/// \brief provides a TCP IPv6 server interface
	class NetTCP_S_V6: public _p::NetTCP_S_p
	{
	public:
		using Endpoint = _p::Net_Socket::Endpoint;

	public:
		NetTCP_S_V6() = default;
		NetTCP_S_V6(NetTCP_S_V6&& p_other);

		using Net_Socket::close;

		///	\brief Creates the socket
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		NET_Error open(const bool p_blocking = true);

		///	\brief Binds a previously created socket with \ref open to a specific network interface
		///	\param[in] p_IP - IP address of the interface to bind too. If ::0 is used, then the socket is bound to any address
		///	\param[in] p_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\return \ref core::NET_Error
		NET_Error bind(const IPv6_address& p_IP, const uint16_t p_Port);

		///	\brief performs both a \ref open and \ref bind
		///	\param[in] p_IP - IP address of the interface to bind too. If ::0 is used, then the socket is bound to any address
		///	\param[in] p_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		NET_Error open_bind(const IPv6_address& p_IP, const uint16_t p_Port, bool p_blocking = true);

		///	\brief performs a \ref open, \ref bind, and \ref listen in one method
		///	\param[in] p_IP - IP address of the interface to bind too. If ::0 is used, then the socket is bound to any address
		///	\param[in] p_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\param[in] p_max_connections - Number of connections allowed to be pending on the socket before starting to refuse them
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		NET_Error open_bind_listen(const IPv6_address& p_IP, const uint16_t p_Port, const int p_max_connections, const bool p_blocking = true);

		///	\brief Accepts a connection request pending on the socket.
		///	\param[out] p_Client - A client object that will manage the client communication. Object must be unused.
		///	\param[in] p_blocking - Determines if the new socket should be a blocking or non-blocking
		///	\return \ref core::NET_Error
		///	\remarks
		///		On blocking sockets, this call will block until a clients is pending to connect.
		///		On a non-blocking socket this can return \ref core::NET_Error::WouldBlock if no client is pending.
		NET_Error accept(NetTCP_C_V6& p_Client, const bool p_blocking = true);

		///	\brief Accepts a connection request pending on the socket.
		///	\param[out] p_Client - A client object that will manage the client communication. Object must be unused.
		///	\param[out] p_other_IP - The peer's IP address.
		///	\param[out] p_other_port - The peer's port number
		///	\param[in] p_blocking - Determines if the new socket should be a blocking or non-blocking
		///	\return \ref core::NET_Error
		///	\remarks
		///		On blocking sockets, this call will block until a clients is pending to connect.
		///		On a non-blocking socket this can return \ref core::NET_Error::WouldBlock if no client is pending.
		NET_Error accept(NetTCP_C_V6& p_Client, IPv6_address& p_other_IP, uint16_t& p_other_port, const bool p_blocking = true);

		///	\brief Gets the address information of the interface. Usefull if \ref bind is used without specifying the address.
		///	\param[in] p_IP - receives the IP address
		///	\param[in] p_Port - receives the port number
		///	\return \ref core::NET_Error
		//
		NET_Error get_address(IPv6_address& p_IP, uint16_t& p_Port);

		///	\brief Swaps this socket with another
		void swap(NetTCP_S_V6& p_other);
	};

	/// \brief Provides an IPv agnostic TCP server interface
	class NetTCP_S: public _p::NetTCP_S_p
	{
	public:
		using IPv		= IP_address::IPv;
		using Endpoint	= _p::Net_Socket::Endpoint;

	private:
		IPv m_IpV;

	public:
		NetTCP_S();
		NetTCP_S(NetTCP_S&& p_other);

		///	\brief Closes a previously open socket
		///	\return \ref core::NET_Error specifically it return \ref core::NET_Error::NoErr if the socket is successfully released.
		///	\remarks
		///		It closes the socket if one was previously open.
		///		This function may fail or block depending on the blocking and linger properties set for the socket.
		///		For more details please see:
		///			https://docs.microsoft.com/en-us/windows/desktop/api/winsock/nf-winsock-closesocket
		///			https://linux.die.net/man/7/socket
		NET_Error close();

		///	\brief Creates the socket
		///	\param[in] p_ipV - Internet Protocol version too use for this socket
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		///	\remarks System should not mix IPv4 and IPv6 protocols
		NET_Error open(const IPv p_ipV, const bool p_blocking = true);

		///	\brief Binds a previously created socket with \ref open to a specific network interface
		///	\param[in] p_IP - IP address of the interface to bind too. If 0.0.0.0 (on IPv4) or ::0 (on IPv6) is used, then the socket is bound to any address
		///	\param[in] p_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\return \ref core::NET_Error
		NET_Error bind(const IP_address& p_IP, const uint16_t p_Port);

		///	\brief performs both a \ref open and \ref bind
		///	\param[in] p_IP - IP address of the interface to bind too. If 0.0.0.0 (on IPv4) or ::0 (on IPv6) is used, then the socket is bound to any address
		///	\param[in] p_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		NET_Error open_bind(const IP_address& p_IP, const uint16_t p_Port, const bool p_blocking = true);

		///	\brief performs a \ref open, \ref bind, and \ref listen in one method
		///	\param[in] p_IP - IP address of the interface to bind too. If 0.0.0.0 (on IPv4) or ::0 (on IPv6) is used, then the socket is bound to any address
		///	\param[in] p_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\param[in] p_max_connections - Number of connections allowed to be pending on the socket before starting to refuse them
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		NET_Error open_bind_listen(const IP_address& p_IP, const uint16_t p_Port, const int p_max_connections, const bool p_blocking = true);

		///	\brief Accepts a connection request pending on the socket.
		///	\param[out] p_Client - A client object that will manage the client communication. Object must be unused.
		///	\param[in] p_blocking - Determines if the new socket should be a blocking or non-blocking
		///	\return \ref core::NET_Error
		NET_Error accept(NetTCP_C& p_Client, const bool p_blocking = true);

		///	\brief Accepts a connection request pending on the socket.
		///	\param[out] p_Client - A client object that will manage the client communication. Object must be unused.
		///	\param[out] p_other_IP - The peer's IP address.
		///	\param[out] p_other_port - The peer's port number
		///	\param[in] p_blocking - Determines if the new socket should be a blocking or non-blocking
		///	\return \ref core::NET_Error
		NET_Error accept(NetTCP_C& p_Client, IP_address& p_other_IP, uint16_t& p_other_port, const bool p_blocking = true);

		///	\brief Gets the address information of the interface. Usefull if \ref bind is used without specifying the address.
		///	\param[in] p_IP - receives the IP address
		///	\param[in] p_Port - receives the port number
		///	\return \ref core::NET_Error
		NET_Error get_address(IP_address& p_IP, uint16_t& p_Port);

		///	\brief Retrieves the Internet Protocol version currently used on this socket.
		///	\return \ref IPv
		///
		///	\remarks System should not mix IPv4 and IPv6 protocols
		IPv IPversion() const;

		///	\brief Swaps this socket with another
		void swap(NetTCP_S& p_other);
	};


	/// \brief provides a TCP IPv4 client interface
	class NetTCP_C_V4: public _p::NetTCP_C_p
	{
		friend class NetTCP_S_V4;

	public:
		using Endpoint = _p::Net_Socket::Endpoint;

	public:
		NetTCP_C_V4() = default;
		NetTCP_C_V4(NetTCP_C_V4&& p_other);

		using Net_Socket::close;

		///	\brief Creates the socket
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		NET_Error open(const bool p_blocking = true);

		///	\brief Binds a previously created socket with \ref open to a specific network interface
		///	\param[in] p_IP - IP address of the interface to bind too. If 0.0.0.0 is used, then the socket is bound to any address
		///	\param[in] p_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\return \ref core::NET_Error
		NET_Error bind(const IPv4_address& my_IP, const uint16_t my_Port);

		///	\brief Connects a previously created socket with \ref open to a server.
		///	\param[in] dest_IP - IP address of the server to connect too.
		///	\param[in] dest_Port - Port number to the server to connect too.
		///	\return \ref core::NET_Error
		///	\remarks
		///		An implicit bind will be made on the socket if it hasn't be bound before.
		///		On blocking sockets, this call will block until the connection is established.
		///		On a non-blocking socket this can return \ref core::NET_Error::WouldBlock if the connection
		///		could not be completed immediately. The user must then use \ref NonBlock_Connect_state to check
		///		when the connections is actually established.
		NET_Error connect(const IPv4_address& dest_IP, const uint16_t dest_Port);

		///	\brief performs both a \ref open and \ref bind
		///	\param[in] my_IP - IP address of the interface to bind too. If 0.0.0.0 is used, then the socket is bound to any address
		///	\param[in] my_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		NET_Error open_bind(const IPv4_address& my_IP, const uint16_t my_Port, bool p_blocking = true);

		///	\brief performs a \ref open, \ref bind, and \ref connect in one method
		///	\param[in] my_IP - IP address of the interface to bind too. If 0.0.0.0 is used, then the socket is bound to any address
		///	\param[in] my_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\param[in] dest_IP - IP address of the server to connect too.
		///	\param[in] dest_Port - Port number to the server to connect too.
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		///	\remarks
		///		An implicit bind will be made on the socket if it hasn't be bound before.
		///		On blocking sockets, this call will block until the connection is established.
		///		On a non-blocking socket this can return \ref core::NET_Error::WouldBlock if the connection
		///		could not be completed immediately. The user must then use \ref NonBlock_Connect_state to check
		///		when the connections is actually established.
		NET_Error open_bind_connect(const IPv4_address& my_IP, const uint16_t my_Port, const IPv4_address& dest_IP, const uint16_t dest_Port, const bool p_blocking = true);

		///	\brief Gets the address information of the interface. Usefull if \ref bind is used without specifying the address.
		///	\param[in] p_IP - receives the IP address
		///	\param[in] p_Port - receives the port number
		///	\return \ref core::NET_Error
		NET_Error get_address(IPv4_address& p_IP, uint16_t& p_Port);

		///	\brief Gets the peer's address information.
		///	\param[in] p_IP - receives the IP address
		///	\param[in] p_Port - receives the port number
		///	\return \ref core::NET_Error
		NET_Error get_peer_address(IPv4_address& p_IP, uint16_t& p_Port);

		///	\brief Swaps this socket with another
		void swap(NetTCP_C_V4& p_other);
	};

	/// \brief provides a TCP IPv6 client interface
	class NetTCP_C_V6: public _p::NetTCP_C_p
	{
		friend class NetTCP_S_V6;

	public:
		using Endpoint = _p::Net_Socket::Endpoint;

	public:
		NetTCP_C_V6() = default;
		NetTCP_C_V6(NetTCP_C_V6&& p_other);

		using Net_Socket::close;

		///	\brief Creates the socket
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		NET_Error open(const bool p_blocking = true);

		///	\brief Binds a previously created socket with \ref open to a specific network interface
		///	\param[in] p_IP - IP address of the interface to bind too. If ::0 is used, then the socket is bound to any address
		///	\param[in] p_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\return \ref core::NET_Error
		NET_Error bind(const IPv6_address& my_IP, const uint16_t my_Port);

		///	\brief Connects a previously created socket with \ref open to a server.
		///	\param[in] dest_IP - IP address of the server to connect too.
		///	\param[in] dest_Port - Port number to the server to connect too.
		///	\return \ref core::NET_Error
		///	\remarks
		///		An implicit bind will be made on the socket if it hasn't be bound before.
		///		On blocking sockets, this call will block until the connection is established.
		///		On a non-blocking socket this can return \ref core::NET_Error::WouldBlock if the connection
		///		could not be completed immediately. The user must then use \ref NonBlock_Connect_state to check
		///		when the connections is actually established.
		NET_Error connect (const IPv6_address& dest_IP, const uint16_t dest_Port);

		///	\brief performs both a \ref open and \ref bind
		///	\param[in] my_IP - IP address of the interface to bind too. If ::0 is used, then the socket is bound to any address
		///	\param[in] my_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		NET_Error open_bind(const IPv6_address& my_IP, const uint16_t my_Port, bool p_blocking = true);

		///	\brief performs a \ref open, \ref bind, and \ref connect in one method
		///	\param[in] my_IP - IP address of the interface to bind too. If ::0 is used, then the socket is bound to any address
		///	\param[in] my_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\param[in] dest_IP - IP address of the server to connect too.
		///	\param[in] dest_Port - Port number to the server to connect too.
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		///	\remarks
		///		An implicit bind will be made on the socket if it hasn't be bound before.
		///		On blocking sockets, this call will block until the connection is established.
		///		On a non-blocking socket this can return \ref core::NET_Error::WouldBlock if the connection
		///		could not be completed immediately. The user must then use \ref NonBlock_Connect_state to check
		///		when the connections is actually established.
		NET_Error open_bind_connect(const IPv6_address& my_IP, const uint16_t my_Port, const IPv6_address& dest_IP, const uint16_t dest_Port, const bool p_blocking = true);

		///	\brief Gets the address information of the interface. Usefull if \ref bind is used without specifying the address.
		///	\param[in] p_IP - receives the IP address
		///	\param[in] p_Port - receives the port number
		///	\return \ref core::NET_Error
		NET_Error get_address	(IPv6_address& p_IP, uint16_t& p_Port);

		///	\brief Gets the peer's address information.
		///	\param[in] p_IP - receives the IP address
		///	\param[in] p_Port - receives the port number
		///	\return \ref core::NET_Error
		NET_Error get_peer_address(IPv6_address& p_IP, uint16_t& p_Port);

		///	\brief Swaps this socket with another
		void swap(NetTCP_C_V6& p_other);
	};

	/// \brief Provides an IPv agnostic TCP client interface
	class NetTCP_C: public _p::NetTCP_C_p
	{
		friend class NetTCP_S;

	public:
		using IPv			= IP_address::IPv;
		using Endpoint	= _p::Net_Socket::Endpoint;
	private:
		IPv m_IpV;

	public:
		NetTCP_C();
		NetTCP_C(NetTCP_C&& p_other);

		///	\brief Closes a previously open socket
		///	\return \ref core::NET_Error specifically it return \ref core::NET_Error::NoErr if the socket is successfully released.
		///	\remarks
		///		It closes the socket if one was previously open.
		///		This function may fail or block depending on the blocking and linger properties set for the socket.
		///		For more details please see:
		///			https://docs.microsoft.com/en-us/windows/desktop/api/winsock/nf-winsock-closesocket
		///			https://linux.die.net/man/7/socket
		NET_Error close();

		///	\brief Creates the socket
		///	\param[in] p_ipV - Internet Protocol version too use for this socket
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		///	\remarks System should not mix IPv4 and IPv6 protocols
		NET_Error open(const IPv p_ipV, const bool p_blocking = true);

		///	\brief Binds a previously created socket with \ref open to a specific network interface
		///	\param[in] p_IP - IP address of the interface to bind too. If 0.0.0.0 (on IPv4) or ::0 (on IPv6) is used, then the socket is bound to any address
		///	\param[in] p_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\return \ref core::NET_Error
		NET_Error bind(const IP_address& my_IP, const uint16_t my_Port);

		///	\brief Connects a previously created socket with \ref open to a server.
		///	\param[in] dest_IP - IP address of the server to connect too.
		///	\param[in] dest_Port - Port number to the server to connect too.
		///	\return \ref core::NET_Error
		///	\remarks
		///		An implicit bind will be made on the socket if it hasn't be bound before.
		///		On blocking sockets, this call will block until the connection is established.
		///		On a non-blocking socket this can return \ref core::NET_Error::WouldBlock if the connection
		///		could not be completed immediately. The user must then use \ref NonBlock_Connect_state to check
		///		when the connections is actually established.
		NET_Error connect(const IP_address& dest_IP, const uint16_t dest_Port);

		///	\brief performs both a \ref open and \ref bind
		///	\param[in] p_IP - IP address of the interface to bind too. If 0.0.0.0 (on IPv4) or ::0 (on IPv6) is used, then the socket is bound to any address
		///	\param[in] p_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		NET_Error open_bind(const IP_address& my_IP, const uint16_t my_Port, const bool p_blocking = true);

		///	\brief performs a \ref open, \ref bind, and \ref connect in one method
		///	\param[in] my_IP - IP address of the interface to bind too.  If 0.0.0.0 (on IPv4) or ::0 (on IPv6) is used, then the socket is bound to any address
		///	\param[in] my_Port - Port number to bind too. If 0 the system will automatically pickup an available free port number.
		///	\param[in] dest_IP - IP address of the server to connect too.
		///	\param[in] dest_Port - Port number to the server to connect too.
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		///	\remarks
		///		An implicit bind will be made on the socket if it hasn't be bound before.
		///		On blocking sockets, this call will block until the connection is established.
		///		On a non-blocking socket this can return \ref core::NET_Error::WouldBlock if the connection
		///		could not be completed immediately. The user must then use \ref NonBlock_Connect_state to check
		///		when the connections is actually established.
		NET_Error open_bind_connect(const IP_address& my_IP, const uint16_t my_Port, const IP_address& dest_IP, const uint16_t dest_Port, const bool p_blocking = true);

		///	\brief Gets the address information of the interface. Usefull if \ref bind is used without specifying the address.
		///	\param[in] p_IP - receives the IP address
		///	\param[in] p_Port - receives the port number
		///	\return \ref core::NET_Error
		NET_Error get_address(IP_address& p_IP, uint16_t& p_Port);

		///	\brief Gets the peer's address information.
		///	\param[in] p_IP - receives the IP address
		///	\param[in] p_Port - receives the port number
		///	\return \ref core::NET_Error
		NET_Error get_peer_address(IP_address& p_IP, uint16_t& p_Port);

		///	\brief Retrieves the Internet Protocol version currently used on this socket.
		///	\return \ref IPv
		///
		///	\remarks System should not mix IPv4 and IPv6 protocols
		IPv IPversion() const;

		///	\brief Swaps this socket with another
		void swap(NetTCP_C& p_other);
	};

	//======== ======== NetTCP_S_V4 ======== ========
	inline void NetTCP_S_V4::swap(NetTCP_S_V4& p_other) { Net_Socket::swap(p_other); }

	//======== ======== NetTCP_S_V6 ======== ========
	inline void NetTCP_S_V6::swap(NetTCP_S_V6& p_other) { Net_Socket::swap(p_other); }

	//======== ======== NetTCP_S ======== ========
	inline void NetTCP_S::swap(NetTCP_S& p_other) { Net_Socket::swap(p_other); std::swap(m_IpV, p_other.m_IpV); }
	inline NetTCP_S::IPv NetTCP_S::IPversion() const { return m_IpV; }

	//======== ======== NetTCP_C_V4 ======== ========
	inline void NetTCP_C_V4::swap(NetTCP_C_V4& p_other) { Net_Socket::swap(p_other); }

	//======== ======== NetTCP_C_V6 ======== ========
	inline void NetTCP_C_V6::swap(NetTCP_C_V6& p_other) { Net_Socket::swap(p_other); }

	//======== ======== NetTCP_C ======== ========
	inline void NetTCP_C::swap(NetTCP_C& p_other) { Net_Socket::swap(p_other); std::swap(m_IpV, p_other.m_IpV); }
	inline NetTCP_C::IPv NetTCP_C::IPversion() const { return m_IpV; }

} //namespace core

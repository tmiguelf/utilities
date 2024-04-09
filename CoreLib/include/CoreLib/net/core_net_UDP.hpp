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

#include "core_net_address.hpp"
#include "core_net_socket.hpp"

/// \n
namespace core
{
	/// \n
	namespace _p
	{
		///	\brief Private class to implement generic UDP functionality
		class NetUDP_p: protected Net_Socket
		{
		protected:
			NetUDP_p() = default;

		public:
			using Net_Socket::is_open;
			using Net_Socket::set_blocking;
			using Net_Socket::set_reuse_address;
			using Net_Socket::set_linger;
			using Net_Socket::poll;
			using Net_Socket::shutdown;

			///	\brief Sets/Unsets the broadcast mode on the socket
			///	\param[in] p_broadcast - if true turns the broadcast mode on, if false turns it of
			///	\return \ref core::NET_Error
			NET_Error set_broadcasting(const bool p_broadcast);

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
			///		On a non-blocking socket this can return \ref core::NET_Error::WouldBlock if no data is ready to be read
			NET_Error receive(void* p_data, uintptr_t& p_size);

			///	\brief Checks the size of the next data pack ready to be received on the socket
			///	\param[out] p_size - Returns the size of the packet
			///	\return \ref core::NET_Error
			///
			///	\remarks
			///		On blocking sockets, this call will block until data is ready to be read on the socket.
			///		On a non-blocking socket this can return \ref core::NET_Error::WouldBlock if no data is ready to be read.
			NET_Error peek_size(uintptr_t& p_size);
		};
	} //namespace _p

	/// \brief provides a UDP IPv4 interface
	class NetUDP_V4: public _p::NetUDP_p
	{
	public:
		using Endpoint = _p::Net_Socket::Endpoint;

	public:
		NetUDP_V4() = default;
		NetUDP_V4(NetUDP_V4&& p_other);

		using Net_Socket::close;
		using NetUDP_p::peek_size;
		using NetUDP_p::receive;

		///	\brief Creates the socket
		///	\param[in] p_blocking - If true the socket is a blocking socket, if false the socket is non-blocking
		///	\return \ref core::NET_Error
		//
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
		NET_Error open_bind(const IPv4_address& p_IP, const uint16_t p_Port, bool p_blocking = true);

		///	\brief Joins a multicast group
		///	\param[in] p_group - address of the multicast group
		///	\param[in] p_interface - address of the interface to use (in raw form). If 0 is used, then the socket uses the default interface.
		///	\return \ref core::NET_Error
		///	\remarks Must be called after a socket has been created (\ref open), but before it is bound (\ref bind)
		NET_Error join_multicast_group(const IPv4_address& p_group, const uint32_t p_interface);

		///	\brief leaves a multicast group
		///	\param[in] p_group - address of the multicast group
		///	\param[in] p_interface - address of the interface to use (in raw form). If 0 is used, then the socket uses the default interface.
		///	\return \ref core::NET_Error
		NET_Error leave_multicast_group(const IPv4_address& p_group, const uint32_t p_interface);

		///	\brief Gets the address information of the interface. Usefull if an implicit binding is used.
		///	\param[in] p_IP - receives the IP address
		///	\param[in] p_Port - receives the port number
		///	\return \ref core::NET_Error
		NET_Error get_address(IPv4_address& p_IP, uint16_t& p_Port);

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
		///		On a non-blocking socket this can return \ref core::NET_Error::WouldBlock
		///		if data could not be sent at tthe time without blocking.
		NET_Error send(const void* p_data, const uintptr_t p_size, const IPv4_address& p_IP, const uint16_t p_Port, const uint8_t p_repeat = 0);

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
		///		On a non-blocking socket this can return \ref core::NET_Error::WouldBlock if no data is ready to be read.
		NET_Error receive(void* p_data, uintptr_t& p_size, IPv4_address& p_other_IP, uint16_t& p_other_port);

		///	\brief Checks the size of the next data pack ready to be received on the socket as well as the address of the sender
		///	\param[out] p_size - Returns the size of the packet
		///	\param[out] p_other_IP - The sender's IP address as reported by the sender
		///	\param[out] p_other_port - The sender's port number as reported by the sender
		///	\return \ref core::NET_Error
		///
		///	\remarks
		///		On blocking sockets, this call will block until data is ready to be read on the socket.
		///		On a non-blocking socket this can return \ref core::NET_Error::WouldBlock if no data is ready to be read.
		NET_Error peek_size(uintptr_t& p_size, IPv4_address& p_other_IP, uint16_t& p_other_port);

		///	\brief Sends a magic packet commonly used for Wake On Lan
		///	\param[in] p_MacAddress - The MAC address of the interface to wake up
		///	\param[in] p_subNet - IP address of the subsystem responsibel for reaching the target device (typically a broadcast address is used)
		///	\param[in] p_port - Target port number where WOL is configured. (tipically port 7 or port 9)
		///	\return \ref core::NET_Error
		///
		///	\remarks
		///		Has the same properties as \ref send, except the data payload is managed internally by the method.
		NET_Error WakeOnLan(std::span<const uint8_t, 6> p_MacAddress, const IPv4_address& p_subNet, const uint16_t p_port);

		///	\brief Sends a magic packet commonly used for Wake On Lan on systems with password
		///	\param[in] p_MacAddress - The MAC address of the interface to wake up
		///	\param[in] p_subNet - IP address of the subsystem responsibel for reaching the target device (typically a broadcast address is used)
		///	\param[in] p_port - Target port number where WOL is configured. (tipically port 7 or port 9)
		///	\param[in] p_password - Buffer containing the password message to be appended at the end of the magic packet.
		///	\param[in] p_password_size - The size of the password buffer. If 0 the behaviour is identical to \ref WakeOnLan
		///	\return \ref core::NET_Error
		///
		///	\remarks
		///		Has the same properties as \ref send, except the data payload is managed internally by the method.
		NET_Error WakeOnLan_password(std::span<const uint8_t, 6> p_MacAddress, const IPv4_address& p_subNet, const uint16_t p_port, const void* p_password = nullptr, const uint16_t p_password_size = 0);

		///	\brief Swaps this socket with another
		void swap(NetUDP_V4& p_other);
	};

	/// \brief provides a UDP IPv6 interface
	class NetUDP_V6: public _p::NetUDP_p
	{
	public:
		using Endpoint = _p::Net_Socket::Endpoint;

	public:
		NetUDP_V6() = default;
		NetUDP_V6(NetUDP_V6&& p_other);

		using Net_Socket::close;
		using NetUDP_p::peek_size;
		using NetUDP_p::receive;

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
		NET_Error open_bind(const IPv6_address& p_IP, const uint16_t p_Port, const bool p_blocking = true);

		///	\brief Joins a multicast group
		///	\param[in] p_group - address of the multicast group
		///	\param[in] p_interface - Interface index to use. If 0 is used, then the socket picks up the default.
		///	\return \ref core::NET_Error
		///	\remarks Must be called after a socket has been created (\ref open), but before it is bound (\ref bind)
		NET_Error join_multicast_group(const IPv6_address& p_group, const uint32_t p_interface);

		///	\brief Leavess a multicast group
		///	\param[in] p_group - address of the multicast group
		///	\param[in] p_interface - Interface index to use. If 0 is used, then the socket picks up the default.
		///	\return \ref core::NET_Error
		NET_Error leave_multicast_group(const IPv6_address& p_group, const uint32_t p_interface);

		///	\brief Gets the address information of the interface. Usefull if an implicit binding is used.
		///	\param[in] p_IP - receives the IP address
		///	\param[in] p_Port - receives the port number
		///	\return \ref core::NET_Error
		NET_Error get_address(IPv6_address&	p_IP, uint16_t& p_Port);

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
		///		On a non-blocking socket this can return \ref core::NET_Error::WouldBlock
		///		if data could not be sent at tthe time without blocking.
		NET_Error send(const void* p_data, const uintptr_t p_size, const IPv6_address& p_IP, const uint16_t p_Port, const uint8_t p_repeat = 0);

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
		///		On a non-blocking socket this can return \ref core::NET_Error::WouldBlock if no data is ready to be read.
		NET_Error receive(void* p_data, uintptr_t& p_size, IPv6_address& p_other_IP, uint16_t& p_other_port);

		///	\brief Checks the size of the next data pack ready to be received on the socket as well as the address of the sender
		///	\param[out] p_size - Returns the size of the packet
		///	\param[out] p_other_IP - The sender's IP address as reported by the sender
		///	\param[out] p_other_port - The sender's port number as reported by the sender
		///	\return \ref core::NET_Error
		///
		///	\remarks
		///		On blocking sockets, this call will block until data is ready to be read on the socket.
		///		On a non-blocking socket this can return \ref core::NET_Error::WouldBlock if no data is ready to be read.
		NET_Error peek_size(uintptr_t& p_size, IPv6_address& p_other_IP, uint16_t& p_other_port);

		///	\brief Sends a magic packet commonly used for Wake On Lan
		///	\param[in] p_MacAddress - The MAC address of the interface to wake up
		///	\param[in] p_subNet - IP address of the subsystem responsibel for reaching the target device (typically a broadcast address is used)
		///	\param[in] p_port - Target port number where WOL is configured. (tipically port 7 or port 9)
		///	\return \ref core::NET_Error
		///
		///	\remarks
		///		Has the same properties as \ref send, except the data payload is managed internally by the method.
		NET_Error WakeOnLan(std::span<const uint8_t, 6> p_MacAddress, const IPv6_address& p_subNet, const uint16_t p_port);

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
		///		Has the same properties as \ref send, except the data payload is managed internally by the method.
		NET_Error WakeOnLan_password(std::span<const uint8_t, 6> p_MacAddress, const IPv6_address& p_subNet, const uint16_t p_port, const void* p_password = nullptr, const uint16_t p_password_size = 0);

		///	\brief Swaps this socket with another
		void swap(NetUDP_V6& p_other);
	};

	/// \brief Provides an IPv agnostic UDP interface
	class NetUDP: public _p::NetUDP_p
	{
	public:
		using IPv			= IP_address::IPv;
		using Endpoint	= _p::Net_Socket::Endpoint;

	private:
		IPv m_IpV;

	public:
		NetUDP();
		NetUDP(NetUDP&& p_other);

		using NetUDP_p::peek_size;
		using NetUDP_p::receive;

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

		///	\brief Joins a multicast group
		///	\param[in] p_group - address of the multicast group
		///	\param[in] p_interface - On IPv4 it represents the RAW address of the interface in integer form, on IPv6 it represents the interface index. If 0 is used, then the socket uses the default interface.
		///	\return \ref core::NET_Error
		///	\remarks Must be called after a socket has been created (\ref open), but before it is bound (\ref bind)
		NET_Error join_multicast_group(const IP_address& p_group, const uint32_t p_interface);

		///	\brief Leaves a multicast group
		///	\param[in] p_group - address of the multicast group
		///	\param[in] p_interface - On IPv4 it represents the RAW address of the interface in integer form, on IPv6 it represents the interface index. If 0 is used, then the socket uses the default interface.
		///	\return \ref core::NET_Error
		NET_Error leave_multicast_group(const IP_address& p_group, const uint32_t p_interface);

		///	\brief Gets the address information of the interface. Usefull if an implicit binding is used.
		///	\param[in] p_IP - receives the IP address
		///	\param[in] p_Port - receives the port number
		///	\return \ref core::NET_Error
		NET_Error get_address(IP_address& p_IP, uint16_t& p_Port);

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
		///		On a non-blocking socket this can return \ref core::NET_Error::WouldBlock
		///		if data could not be sent at tthe time without blocking.
		///		IPv4 sockets can not send to IPv6 networks and vice-versa.
		NET_Error send(const void* p_data, const uintptr_t p_size, const IP_address& p_IP, const uint16_t p_Port, const uint8_t p_repeat = 0);

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
		///		On a non-blocking socket this can return \ref core::NET_Error::WouldBlock if no data is ready to be read.
		NET_Error receive(void* p_data, uintptr_t& p_size, IP_address& p_other_IP, uint16_t& p_other_port);

		///	\brief Checks the size of the next data pack ready to be received on the socket as well as the address of the sender
		///	\param[out] p_size - Returns the size of the packet
		///	\param[out] p_other_IP - The sender's IP address as reported by the sender
		///	\param[out] p_other_port - The sender's port number as reported by the sender
		///	\return \ref core::NET_Error
		///
		///	\remarks
		///		On blocking sockets, this call will block until data is ready to be read on the socket.
		///		On a non-blocking socket this can return \ref core::NET_Error::WouldBlock if no data is ready to be read.
		NET_Error peek_size(uintptr_t& p_size, IP_address& p_other_IP, uint16_t& p_other_port);

		///	\brief Sends a magic packet commonly used for Wake On Lan
		///	\param[in] p_MacAddress - The MAC address of the interface to wake up
		///	\param[in] p_subNet - IP address of the subsystem responsibel for reaching the target device (typically a broadcast address is used)
		///	\param[in] p_port - Target port number where WOL is configured. (tipically port 7 or port 9)
		///	\return \ref core::NET_Error
		///
		///	\remarks
		///		Has the same properties as \ref send, except the data payload is managed internally by the method.
		NET_Error WakeOnLan(std::span<const uint8_t, 6> p_MacAddress, const IP_address& p_subNet, const uint16_t p_port);

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
		///		Has the same properties as \ref send, except the data payload is managed internally by the method.
		NET_Error WakeOnLan_password(std::span<const uint8_t, 6> p_MacAddress, const IP_address& p_subNet, const uint16_t p_port, const void* p_password = nullptr, const uint16_t p_password_size = 0);

		///	\brief Retrieves the Internet Protocol version currently used on this socket.
		///	\return \ref IPv
		///
		///	\remarks System should not mix IPv4 and IPv6 protocols
		IPv IPversion() const;

		///	\brief Swaps this socket with another
		void swap(NetUDP& p_other);
	};


	//======== ======== NetUDP_V4 ======== ========
	inline void NetUDP_V4::swap(NetUDP_V4& p_other) { Net_Socket::swap(p_other); }

	//======== ======== NetUDP_V6 ======== ========
	inline void NetUDP_V6::swap(NetUDP_V6& p_other) { Net_Socket::swap(p_other); }

	//======== ======== NetUDP ======== ========
	inline void NetUDP::swap(NetUDP& p_other) { Net_Socket::swap(p_other); std::swap(m_IpV, p_other.m_IpV); }
	inline NetUDP::IPv NetUDP::IPversion() const { return m_IpV; }


} //namespace core

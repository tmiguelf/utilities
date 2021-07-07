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

#include <queue>
#include <CoreLib/net/core_net_address.hpp>
#include <CoreLib/string/core_string_numeric.hpp>
#include <CoreLib/Core_Endian.hpp>


namespace core
{

//========	========	========	========	========
//========		IP String Conversions			========
//========	========	========	========	========

template<typename CharT>
static inline bool from_string_IPv4(std::basic_string_view<CharT> p_address, void* p_raw)
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
		pos1 = p_address.find(CharT{'.'}, pos2);
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

template<typename CharT>
static inline bool from_string_IPv6(std::basic_string_view<CharT> p_address, uint16_t* p_raw)
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
		if(pos1 == std::basic_string_view<CharT>::npos)
		{
			if(size == pos2) break;
			if(size - pos2 > 4) return false;

			from_chars_result<uint16_t> auxRet = from_chars_hex<uint16_t>({tdata + pos2, size - pos2});
			if(!auxRet.has_value())
			{
				return false;
			}
			auxI = core::endian_host2big(auxRet.value());

			if(b_hasEliad)	postEliad.push_back(auxI);
			else			preEliad .push_back(auxI);
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

		from_chars_result<uint16_t> auxRet = from_chars_hex<uint16_t>({tdata + pos2, pos1 - pos2});
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

	if(pos1 != std::basic_string_view<CharT>::npos) return false;

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

template<typename CharT>
static inline uintptr_t to_string_IPv4(std::span<const uint8_t, 4> p_raw, std::span<CharT, 15> p_output)
{
	char8_t* pivot = p_output.data();
	pivot += core::to_chars(p_raw[0], std::span<CharT, 3>{pivot, 3});
	*pivot = '.';
	++pivot;
	pivot += core::to_chars(p_raw[1], std::span<CharT, 3>{pivot, 3});
	*pivot = '.';
	++pivot;
	pivot += core::to_chars(p_raw[2], std::span<CharT, 3>{pivot, 3});
	*pivot = '.';
	++pivot;
	pivot += core::to_chars(p_raw[3], std::span<CharT, 3>{pivot, 3});

	return static_cast<uintptr_t>(pivot - p_output.data());
}

template<typename CharT>
static inline uintptr_t to_string_IPv6(std::span<const uint16_t, 8> p_raw, std::span<CharT, 39> p_output)
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

	CharT* pivot = p_output.data();

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
				pivot += core::to_chars_hex(core::endian_big2host(p_raw[it]), std::span<CharT, 4>{pivot, 4});
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
				pivot += core::to_chars_hex(core::endian_big2host(p_raw[it]), std::span<CharT, 4>{pivot, 4});
				*pivot = ':';
				++pivot;
			}
			pivot += core::to_chars_hex(core::endian_big2host(p_raw[7]), std::span<CharT, 4>{pivot, 4});
		}
	}
	else
	{
		for(it = 0; it < 7; ++it)
		{
			pivot += core::to_chars_hex(core::endian_big2host(p_raw[it]), std::span<CharT, 4>{pivot, 4});
			*pivot = ':';
			++pivot;
		}
		pivot += core::to_chars_hex(core::endian_big2host(p_raw[7]), std::span<CharT, 4>{pivot, 4});
	}
	return static_cast<uintptr_t> (pivot - p_output.data());
}
















//========= ======== ======== IP_address ========= ======== ========

IP_address::IP_address():
	m_ipv(IPv::None)
{
}

IP_address::IP_address(IPv p_version, std::span<const uint8_t> p_init)
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

IP_address::IP_address(std::u8string_view p_address)
{
	from_string(p_address);
}

bool IP_address::from_string_v4(std::u8string_view p_address)
{
	m_ipv = IPv::None;
	if(from_string_IPv4(p_address, v4.byteField))
	{
		m_ipv = IPv::IPv_4;
		return true;
	}

	return false;
}

bool IP_address::from_string_v6(std::u8string_view p_address)
{
	m_ipv = IPv::None;
	if(from_string_IPv6(p_address, v6.wordField))
	{
		m_ipv = IPv::IPv_6;
		return true;
	}

	return false;
}

bool IP_address::from_string(std::u8string_view p_address)
{
	if(from_string_v4(p_address)) return true;
	return from_string_v6(p_address);
}

uintptr_t IP_address::to_string(std::span<char8_t, 39> p_output) const
{
	if(m_ipv == IPv::IPv_4)
	{
		return to_string_IPv4(v4.byteField, p_output.subspan<0, 15>());
	}
	else if(m_ipv == IPv::IPv_6)
	{
		return to_string_IPv6(v6.wordField, p_output);
	}
	return 0;
}

std::u8string IP_address::to_string() const
{
	std::array<char8_t, 39> buff;
	return {buff.data(), to_string(buff)};
}

void IP_address::set_any_v4()
{
	m_ipv		= IPv::IPv_4;
	v4.ui32Type	= 0;
}

void IP_address::set_any_v6()
{
	m_ipv			= IPv::IPv_6;
	v6.ui64Type[0]	= 0;
	v6.ui64Type[1]	= 0;
}

void IP_address::set_loopback_v4()
{
	m_ipv			= IPv::IPv_4;
	v4.ui32Type		= 0;
	v4.byteField[0]	= 127;
	v4.byteField[3]	= 1;
}

void IP_address::set_loopback_v6()
{
	m_ipv				= IPv::IPv_6;
	v6.ui64Type[0]		= 0;
	v6.ui64Type[1]		= 0;
	v6.byteField[15]	= 1;
}

bool IP_address::is_null() const
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

bool IP_address::is_valid() const
{
	return (m_ipv == IPv::IPv_4) || (m_ipv == IPv::IPv_6);
}

IP_address& IP_address::operator |= (const IP_address& p_other)
{
	v6.ui64Type[0] |= p_other.v6.ui64Type[0];
	v6.ui64Type[1] |= p_other.v6.ui64Type[1];
	return *this;
}

IP_address& IP_address::operator &= (const IP_address& p_other)
{
	v6.ui64Type[0] &= p_other.v6.ui64Type[0];
	v6.ui64Type[1] &= p_other.v6.ui64Type[1];
	return *this;
}

IP_address& IP_address::operator ^= (const IP_address& p_other)
{
	v6.ui64Type[0] ^= p_other.v6.ui64Type[0];
	v6.ui64Type[1] ^= p_other.v6.ui64Type[1];
	return *this;
}

IP_address IP_address::operator | (const IP_address& p_other) const
{
	IP_address ret(*this);
	return ret |= p_other;
}

IP_address IP_address::operator & (const IP_address& p_other) const
{
	IP_address ret(*this);
	return ret &= p_other;
}

IP_address IP_address::operator ^ (const IP_address& p_other) const
{
	IP_address ret(*this);
	return ret ^= p_other;
}

IP_address IP_address::operator ~ () const
{
	IP_address out;
	out.m_ipv = m_ipv;
	out.v6.ui64Type[0] = ~v6.ui64Type[0];
	out.v6.ui64Type[1] = ~v6.ui64Type[1];
	return out;
}

bool IP_address::operator == (const IP_address& p_other) const
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

bool IP_address::operator != (const IP_address& p_other) const
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

bool IP_address::operator < (const IP_address& p_other) const
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







//========= ======== ======== IPv4_address ========= ======== ========

IPv4_address::IPv4_address(std::u8string_view p_address)
{
	if(!from_string(p_address)) ui32Type = 0;
}

bool IPv4_address::from_string(std::u8string_view p_address)
{
	return from_string_IPv4(p_address, byteField);
}

uintptr_t IPv4_address::to_string(std::span<char8_t, 15> p_output) const
{
	return to_string_IPv4(byteField, p_output);
}
std::u8string IPv4_address::to_string() const
{
	std::array<char8_t, 15> buff;
	return {buff.data(), to_string(buff)};
}

//========= ======== ======== IPv6_address ========= ======== ========

IPv6_address::IPv6_address()
{
	ui64Type[0]	= 0;
	ui64Type[1]	= 0;
}

IPv6_address::IPv6_address(std::span<const uint8_t, 16> p_init)
{
	memcpy(byteField, p_init.data(), 16);
}

IPv6_address::IPv6_address(const IPv6_address& p_other)
{
	ui64Type[0]	= p_other.ui64Type[0];
	ui64Type[1]	= p_other.ui64Type[1];
}

IPv6_address::IPv6_address(std::u8string_view p_address)
{
	if(!from_string(p_address))
	{
		ui64Type[0] = 0;
		ui64Type[1] = 0;
	}
}

bool IPv6_address::from_string(std::u8string_view p_address)
{
	return from_string_IPv6(p_address, wordField);
}

uintptr_t IPv6_address::to_string(std::span<char8_t, 39> p_output) const
{
	return to_string_IPv6(wordField, p_output);
}

std::u8string IPv6_address::to_string() const
{
	std::array<char8_t, 39> buff;
	return {buff.data(), to_string(buff)};
}

void IPv6_address::set_any()
{
	ui64Type[0]	= 0;
	ui64Type[1]	= 0;
}

void IPv6_address::swap(IPv6_address& p_other)
{
	std::swap(ui64Type[0], p_other.ui64Type[0]);
	std::swap(ui64Type[1], p_other.ui64Type[1]);
}

IPv6_address& IPv6_address::operator = (const IPv6_address& p_other)
{
	ui64Type[0] = p_other.ui64Type[0];
	ui64Type[1] = p_other.ui64Type[1];
	return *this;
}

IPv6_address& IPv6_address::operator |= (const IPv6_address& p_other)
{
	ui64Type[0] |= p_other.ui64Type[0];
	ui64Type[1] |= p_other.ui64Type[1];
	return *this;
}

IPv6_address& IPv6_address::operator &= (const IPv6_address& p_other)
{
	ui64Type[0] &= p_other.ui64Type[0];
	ui64Type[1] &= p_other.ui64Type[1];
	return *this;
}

IPv6_address& IPv6_address::operator ^= (const IPv6_address& p_other)
{
	ui64Type[0] ^= p_other.ui64Type[0];
	ui64Type[1] ^= p_other.ui64Type[1];
	return *this;
}

IPv6_address IPv6_address::operator | (const IPv6_address& p_other) const
{
	IPv6_address out;
	out.ui64Type[0] = ui64Type[0] | p_other.ui64Type[0];
	out.ui64Type[1] = ui64Type[1] | p_other.ui64Type[1];
	return out;
}

IPv6_address IPv6_address::operator & (const IPv6_address& p_other) const
{
	IPv6_address out;
	out.ui64Type[0] = ui64Type[0] & p_other.ui64Type[0];
	out.ui64Type[1] = ui64Type[1] & p_other.ui64Type[1];
	return out;
}

IPv6_address IPv6_address::operator ^ (const IPv6_address& p_other) const
{
	IPv6_address out;
	out.ui64Type[0] = ui64Type[0] ^ p_other.ui64Type[0];
	out.ui64Type[1] = ui64Type[1] ^ p_other.ui64Type[1];
	return out;
}

IPv6_address IPv6_address::operator ~ () const
{
	IPv6_address out;
	out.ui64Type[0] = ~ui64Type[0];
	out.ui64Type[1] = ~ui64Type[1];
	return out;
}

bool IPv6_address::operator == (const IPv6_address& p_other) const
{
	return memcmp(byteField, p_other.byteField, 16) == 0;
}

bool IPv6_address::operator != (const IPv6_address& p_other) const
{
	return memcmp(byteField, p_other.byteField, 16) != 0;
}

bool IPv6_address::operator < (const IPv6_address& p_other) const
{
	if(ui64Type[0] == p_other.ui64Type[0])
	{
		return ui64Type[1] < p_other.ui64Type[1];
	}
	return ui64Type[0] < p_other.ui64Type[0];
}





























} //namespace core
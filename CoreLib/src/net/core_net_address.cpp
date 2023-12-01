//======== ======== ======== ======== ======== ======== ======== ========
///	\file
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
//======== ======== ======== ======== ======== ======== ======== ========

#include <CoreLib/net/core_net_address.hpp>
#include <CoreLib/string/core_string_numeric.hpp>
#include <CoreLib/Core_Endian.hpp>


namespace core
{

	namespace _p
	{
		[[nodiscard]] uintptr_t to_chars_IPv4_estimate(std::span<const uint8_t, 4> const p_raw)
		{
			return
				to_chars_estimate(p_raw[0]) +
				to_chars_estimate(p_raw[1]) +
				to_chars_estimate(p_raw[2]) +
				to_chars_estimate(p_raw[3]) + 3;
		}

		template<c_ipconv_char CharT>
		CharT* to_chars_IPv4_unsafe(std::span<const uint8_t, 4> const p_raw, CharT* p_out)
		{
			p_out += core::to_chars(p_raw[0], std::span<CharT, 3>{p_out, 3});
			*(p_out++) = '.';
			p_out += core::to_chars(p_raw[1], std::span<CharT, 3>{p_out, 3});
			*(p_out++) = '.';
			p_out += core::to_chars(p_raw[2], std::span<CharT, 3>{p_out, 3});
			*(p_out++) = '.';
			return core::_p::to_chars_unsafe(p_raw[3], p_out);
		}

		template<c_ipconv_char CharT>
		uintptr_t to_chars_IPv4(std::span<const uint8_t, 4> const p_raw, std::span<CharT, 15> const p_output)
		{
			CharT* pivot = p_output.data();
			pivot += core::to_chars(p_raw[0], std::span<CharT, 3>{pivot, 3});
			*(pivot++) = '.';
			pivot += core::to_chars(p_raw[1], std::span<CharT, 3>{pivot, 3});
			*(pivot++) = '.';
			pivot += core::to_chars(p_raw[2], std::span<CharT, 3>{pivot, 3});
			*(pivot++) = '.';
			pivot += core::to_chars(p_raw[3], std::span<CharT, 3>{pivot, 3});
			return static_cast<uintptr_t>(pivot - p_output.data());
		}

		uintptr_t to_chars_IPv6_estimate(std::span<const uint16_t, 8> const p_raw)
		{
			uint8_t size_elide = 0;
			uint8_t pos_elide = 0;
			{
				for(uint8_t it = 0; it < 8;)
				{
					if(p_raw[it] == 0)
					{
						const uint8_t curret_pos = it;
						uint8_t curret_size = 1;
						while(++it < 8 && p_raw[it] == 0)
						{
							++curret_size;
						}
						if(curret_size > size_elide)
						{
							size_elide = curret_size;
							pos_elide  = curret_pos;
						}
					}
					else
					{
						++it;
					}
				}
			}

			if(size_elide > 1)
			{
				uintptr_t res_size = 8 - size_elide;
				if(pos_elide)
				{
					for(uint8_t it = 0; it < pos_elide; ++it)
					{
						res_size += to_chars_hex_estimate(endian_big2host(p_raw[it]));
					}
				}
				else
				{
					++res_size;
				}
				uint8_t it = pos_elide + size_elide;
				if(it < 8)
				{
					for(; it < 8; ++it)
					{
						res_size += to_chars_hex_estimate(endian_big2host(p_raw[it]));
					}
				}
				else
				{
					++res_size;
				}
				return res_size;
			}
			else
			{
				return
					to_chars_hex_estimate(endian_big2host(p_raw[0])) +
					to_chars_hex_estimate(endian_big2host(p_raw[1])) +
					to_chars_hex_estimate(endian_big2host(p_raw[2])) +
					to_chars_hex_estimate(endian_big2host(p_raw[3])) +
					to_chars_hex_estimate(endian_big2host(p_raw[4])) +
					to_chars_hex_estimate(endian_big2host(p_raw[5])) +
					to_chars_hex_estimate(endian_big2host(p_raw[6])) +
					to_chars_hex_estimate(endian_big2host(p_raw[7])) + 7;
			}
		}

		template<c_ipconv_char CharT>
		CharT* to_chars_IPv6_unsafe(std::span<const uint16_t, 8> const p_raw, CharT* p_out)
		{
			uint8_t size_elide = 0;
			uint8_t pos_elide = 0;
			{
				for(uint8_t it = 0; it < 8;)
				{
					if(p_raw[it] == 0)
					{
						const uint8_t curret_pos = it;
						uint8_t curret_size = 1;
						while(++it < 8 && p_raw[it] == 0)
						{
							++curret_size;
						}
						if(curret_size > size_elide)
						{
							size_elide = curret_size;
							pos_elide  = curret_pos;
						}
					}
					else
					{
						++it;
					}
				}
			}

			if(size_elide > 1)
			{
				if(pos_elide)
				{
					for(uint8_t it = 0; it < pos_elide; ++it)
					{
						p_out += core::to_chars_hex(core::endian_big2host(p_raw[it]), std::span<CharT, 4>{p_out, 4});
						*(p_out++) = ':';
					}
				}
				else
				{
					*(p_out++) = ':';
				}
				*(p_out++) = ':';

				uint8_t it = pos_elide + size_elide;
				if(it < 8)
				{
					for(; it < 7; ++it)
					{
						p_out += core::to_chars_hex(core::endian_big2host(p_raw[it]), std::span<CharT, 4>{p_out, 4});
						*(p_out++) = ':';
					}
					p_out = core::_p::to_chars_hex_unsafe(core::endian_big2host(p_raw[7]), p_out);
				}
			}
			else
			{
				for(uint8_t it = 0; it < 7; ++it)
				{
					p_out += core::to_chars_hex(core::endian_big2host(p_raw[it]), std::span<CharT, 4>{p_out, 4});
					*(p_out++) = ':';
				}
				p_out = core::_p::to_chars_hex_unsafe(core::endian_big2host(p_raw[7]), p_out);
			}
			return p_out;
		}

		template<c_ipconv_char CharT>
		uintptr_t to_chars_IPv6(std::span<const uint16_t, 8> const p_raw, std::span<CharT, 39> const p_out)
		{
			uint8_t size_elide = 0;
			uint8_t pos_elide = 0;
			{
				for(uint8_t it = 0; it < 8;)
				{
					if(p_raw[it] == 0)
					{
						const uint8_t curret_pos = it;
						uint8_t curret_size = 1;
						while(++it < 8 && p_raw[it] == 0)
						{
							++curret_size;
						}
						if(curret_size > size_elide)
						{
							size_elide = curret_size;
							pos_elide  = curret_pos;
						}
					}
					else
					{
						++it;
					}
				}
			}

			CharT* pivot = p_out.data();
			if(size_elide > 1)
			{
				if(pos_elide)
				{
					for(uint8_t it = 0; it < pos_elide; ++it)
					{
						pivot += core::to_chars_hex(core::endian_big2host(p_raw[it]), std::span<CharT, 4>{pivot, 4});
						*(pivot++) = ':';
					}
				}
				else
				{
					*(pivot++) = ':';
				}
				*(pivot++) = ':';

				uint8_t it = pos_elide + size_elide;
				if(it < 8)
				{
					for(; it < 7; ++it)
					{
						pivot += core::to_chars_hex(core::endian_big2host(p_raw[it]), std::span<CharT, 4>{pivot, 4});
						*(pivot++) = ':';
					}
					pivot += core::to_chars_hex(core::endian_big2host(p_raw[7]), std::span<CharT, 4>{pivot, 4});
				}
			}
			else
			{
				for(uint8_t it = 0; it < 7; ++it)
				{
					pivot += core::to_chars_hex(core::endian_big2host(p_raw[it]), std::span<CharT, 4>{pivot, 4});
					*(pivot++) = ':';
				}
				pivot += core::to_chars_hex(core::endian_big2host(p_raw[7]), std::span<CharT, 4>{pivot, 4});
			}
			return static_cast<uintptr_t> (pivot - p_out.data());
		}


		template<c_ipconv_char CharT>
		bool from_chars_IPv4(std::basic_string_view<CharT> const p_address, std::span<uint8_t, 4> const p_out)
		{
			const uintptr_t size = p_address.size();

			if(size < 7 || size > 15)
			{
				return false;
			}

			uintptr_t pos2 = 0;
			for(uintptr_t i = 0; i < 3; ++i)
			{
				const uintptr_t pos1 = p_address.find(CharT{'.'}, pos2);
				if(pos1 - pos2 > 3 || pos1 == pos2) return false;

				const from_chars_result<uint8_t> auxRet = from_chars<uint8_t>(p_address.substr(pos2, pos1 - pos2));
				if(!auxRet.has_value()) return false;

				p_out[i] = auxRet.value();
				pos2 = pos1 + 1;
			}

			if(size - pos2 > 3 || size == pos2) return false;

			const from_chars_result<uint8_t> auxRet = from_chars<uint8_t>(p_address.substr(pos2, size - pos2));
			if(!auxRet.has_value()) return false;
			p_out[3] = auxRet.value();
			return true;
		}

		template<c_ipconv_char CharT>
		bool from_chars_IPv6(std::basic_string_view<CharT> const p_address, std::span<uint16_t, 8> const p_out)
		{
			const uintptr_t size = p_address.size();
			if(size < 2 || size > 39) return false;

			bool b_has_elide = false;
			uintptr_t pos2;
			uint8_t   count;

			if(p_address[0] == ':')
			{
				if(p_address[1] != ':') return false;
				count	= 2;
				pos2	= 2;
				b_has_elide = true;
			}
			else
			{
				count	= 0;
				pos2	= 0;
			}

			std::array<uint16_t, 8> pre_elide;
			std::array<uint16_t, 8> post_elide;
			uint8_t pre_elide_size = 0;
			uint8_t post_elide_size = 0;

			const CharT* tdata = p_address.data();
			uintptr_t pos1 = 0;
			for(; count < 8; ++count)
			{
				pos1 = p_address.find(':', pos2);
				if(pos1 == std::basic_string_view<CharT>::npos)
				{
					if(size == pos2) break;
					if(size - pos2 > 4) return false;

					from_chars_result<uint16_t> auxRet =
						from_chars_hex<uint16_t, CharT>(std::basic_string_view<CharT>{tdata + pos2, size - pos2});
					if(!auxRet.has_value())
					{
						return false;
					}
					const uint16_t auxI = core::endian_host2big(auxRet.value());

					if(b_has_elide)	post_elide[post_elide_size++] = auxI;
					else			pre_elide [pre_elide_size++ ] = auxI;
					break;
				}

				if(pos1 - pos2 > 4) return false;
				if(pos1 == pos2)
				{
					if(b_has_elide) return false;
					b_has_elide = true;
					pos2 = pos1 + 1;
					continue;
				}

				from_chars_result<uint16_t> auxRet =
					from_chars_hex<uint16_t, CharT>(std::basic_string_view<CharT>{tdata + pos2, pos1 - pos2});
				if(!auxRet.has_value())
				{
					return false;
				}

				const uint16_t auxI = core::endian_host2big(auxRet.value());

				if(b_has_elide)	post_elide[post_elide_size++] = auxI;
				else			pre_elide [pre_elide_size++ ] = auxI;

				pos2 = pos1 + 1;
				if(pos2 >= size) return false;
			}

			if(pos1 != std::basic_string_view<CharT>::npos) return false;

			if(b_has_elide)
			{
				const uintptr_t post_elid_start = 8 - post_elide_size;

				uint16_t* pivot = p_out.data();
				{
					uint8_t i = 0;
					for(; i < pre_elide_size; ++i)
					{
						*(pivot++) = pre_elide[i];
					}

					for(; i < post_elid_start; ++i)
					{
						*(pivot++) = 0;
					}
				}
				for(uint8_t i = 0; i < post_elide_size; ++i)
				{
					*(pivot++) = post_elide[i];
				}
				return true;
			}
			else
			{
				if(count < 8) return false;
				memcpy(p_out.data(), pre_elide.data(), 16);
				return true;
			}
		}

		template char8_t * to_chars_IPv4_unsafe<char8_t >(std::span<const uint8_t, 4>, char8_t *);
		template char16_t* to_chars_IPv4_unsafe<char16_t>(std::span<const uint8_t, 4>, char16_t*);
		template char32_t* to_chars_IPv4_unsafe<char32_t>(std::span<const uint8_t, 4>, char32_t*);

		template uintptr_t to_chars_IPv4<char8_t >(std::span<const uint8_t, 4>, std::span<char8_t , 15>);
		template uintptr_t to_chars_IPv4<char16_t>(std::span<const uint8_t, 4>, std::span<char16_t, 15>);
		template uintptr_t to_chars_IPv4<char32_t>(std::span<const uint8_t, 4>, std::span<char32_t, 15>);

		template char8_t * to_chars_IPv6_unsafe<char8_t >(std::span<const uint16_t, 8>, char8_t *);
		template char16_t* to_chars_IPv6_unsafe<char16_t>(std::span<const uint16_t, 8>, char16_t*);
		template char32_t* to_chars_IPv6_unsafe<char32_t>(std::span<const uint16_t, 8>, char32_t*);

		template uintptr_t to_chars_IPv6<char8_t >(std::span<const uint16_t, 8>, std::span<char8_t , 39>);
		template uintptr_t to_chars_IPv6<char16_t>(std::span<const uint16_t, 8>, std::span<char16_t, 39>);
		template uintptr_t to_chars_IPv6<char32_t>(std::span<const uint16_t, 8>, std::span<char32_t, 39>);

	} //namespace _p

//========= ======== ======== IP_address ========= ======== ========
IP_address::IP_address()
{
}

IP_address::IP_address(std::span<const uint8_t, 4> const p_init)
	: m_ipv(IPv::IPv_4)
{
	v4.byteField[0] = p_init[0];
	v4.byteField[1] = p_init[1];
	v4.byteField[2] = p_init[2];
	v4.byteField[3] = p_init[3];
}

IP_address::IP_address(std::span<const uint16_t, 8> const p_init)
	: m_ipv(IPv::IPv_6)
{
	v6.doubletField[0] = p_init[0];
	v6.doubletField[1] = p_init[1];
	v6.doubletField[2] = p_init[2];
	v6.doubletField[3] = p_init[3];
	v6.doubletField[4] = p_init[4];
	v6.doubletField[5] = p_init[5];
	v6.doubletField[6] = p_init[6];
	v6.doubletField[7] = p_init[7];
}

bool IP_address::from_string_v4(std::u8string_view const p_address)
{
	if(_p::from_chars_IPv4(p_address, v4.byteField))
	{
		m_ipv = IPv::IPv_4;
		return true;
	}
	m_ipv = IPv::None;
	return false;
}

bool IP_address::from_string_v4(std::u16string_view const p_address)
{
	if(_p::from_chars_IPv4(p_address, v4.byteField))
	{
		m_ipv = IPv::IPv_4;
		return true;
	}
	m_ipv = IPv::None;
	return false;
}

bool IP_address::from_string_v4(std::u32string_view const p_address)
{
	if(_p::from_chars_IPv4(p_address, v4.byteField))
	{
		m_ipv = IPv::IPv_4;
		return true;
	}
	m_ipv = IPv::None;
	return false;
}

bool IP_address::from_string_v6(std::u8string_view const p_address)
{
	if(_p::from_chars_IPv6(p_address, v6.doubletField))
	{
		m_ipv = IPv::IPv_6;
		return true;
	}
	m_ipv = IPv::None;
	return false;
}

bool IP_address::from_string_v6(std::u16string_view const p_address)
{
	if(_p::from_chars_IPv6(p_address, v6.doubletField))
	{
		m_ipv = IPv::IPv_6;
		return true;
	}
	m_ipv = IPv::None;
	return false;
}

bool IP_address::from_string_v6(std::u32string_view const p_address)
{
	if(_p::from_chars_IPv6(p_address, v6.doubletField))
	{
		m_ipv = IPv::IPv_6;
		return true;
	}
	m_ipv = IPv::None;
	return false;
}

bool IP_address::from_string(std::u8string_view const p_address)
{
	if(_p::from_chars_IPv4(p_address, v4.byteField))
	{
		m_ipv = IPv::IPv_4;
		return true;
	}
	if(_p::from_chars_IPv6(p_address, v6.doubletField))
	{
		m_ipv = IPv::IPv_6;
		return true;
	}
	m_ipv = IPv::None;
	return false;
}

bool IP_address::from_string(std::u16string_view const p_address)
{
	if(_p::from_chars_IPv4(p_address, v4.byteField))
	{
		m_ipv = IPv::IPv_4;
		return true;
	}
	if(_p::from_chars_IPv6(p_address, v6.doubletField))
	{
		m_ipv = IPv::IPv_6;
		return true;
	}
	m_ipv = IPv::None;
	return false;
}

bool IP_address::from_string(std::u32string_view const p_address)
{
	if(_p::from_chars_IPv4(p_address, v4.byteField))
	{
		m_ipv = IPv::IPv_4;
		return true;
	}
	if(_p::from_chars_IPv6(p_address, v6.doubletField))
	{
		m_ipv = IPv::IPv_6;
		return true;
	}
	m_ipv = IPv::None;
	return false;
}

uintptr_t IP_address::to_string(std::span<char8_t, 39> const p_output) const
{
	if(m_ipv == IPv::IPv_4)
	{
		return _p::to_chars_IPv4(v4.byteField, p_output.subspan<0, 15>());
	}
	else if(m_ipv == IPv::IPv_6)
	{
		return _p::to_chars_IPv6(v6.doubletField, p_output);
	}
	return 0;
}

uintptr_t IP_address::to_string(std::span<char16_t, 39> const p_output) const
{
	if(m_ipv == IPv::IPv_4)
	{
		return _p::to_chars_IPv4(v4.byteField, p_output.subspan<0, 15>());
	}
	else if(m_ipv == IPv::IPv_6)
	{
		return _p::to_chars_IPv6(v6.doubletField, p_output);
	}
	return 0;
}

uintptr_t IP_address::to_string(std::span<char32_t, 39> const p_output) const
{
	if(m_ipv == IPv::IPv_4)
	{
		return _p::to_chars_IPv4(v4.byteField, p_output.subspan<0, 15>());
	}
	else if(m_ipv == IPv::IPv_6)
	{
		return _p::to_chars_IPv6(v6.doubletField, p_output);
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

bool IPv4_address::from_string(std::u8string_view const p_address)
{
	if(_p::from_chars_IPv4(p_address, byteField))
	{
		return true;
	}
	ui32Type = 0;
	return false;
}

bool IPv4_address::from_string(std::u16string_view const p_address)
{ 
	if(_p::from_chars_IPv4(p_address, byteField))
	{
		return true;
	}
	ui32Type = 0;
	return false;
}

bool IPv4_address::from_string(std::u32string_view const p_address)
{
	if(_p::from_chars_IPv4(p_address, byteField))
	{
		return true;
	}
	ui32Type = 0;
	return false;
}


std::u8string IPv4_address::to_string() const
{
	std::array<char8_t, 15> buff;
	return {buff.data(), to_string(buff)};
}

//========= ======== ======== IPv6_address ========= ======== ========

IPv6_address::IPv6_address(std::span<const uint16_t, 8> const p_init)
{
	memcpy(doubletField, p_init.data(), 16);
}

bool IPv6_address::from_string(std::u8string_view const p_address)
{
	if(_p::from_chars_IPv6(p_address, doubletField))
	{
		return true;
	}
	ui64Type[0] = 0;
	ui64Type[1] = 0;
	return false;
}

bool IPv6_address::from_string(std::u16string_view const p_address)
{
	if(_p::from_chars_IPv6(p_address, doubletField))
	{
		return true;
	}
	ui64Type[0] = 0;
	ui64Type[1] = 0;
	return false;
}

bool IPv6_address::from_string(std::u32string_view const p_address)
{
	if(_p::from_chars_IPv6(p_address, doubletField))
	{
		return true;
	}
	ui64Type[0] = 0;
	ui64Type[1] = 0;
	return false;
}

std::u8string IPv6_address::to_string() const
{
	std::array<char8_t, 39> buff;
	return {buff.data(), to_string(buff)};
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
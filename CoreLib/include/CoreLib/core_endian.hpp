//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///		Provides endianess conversions
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

#include <bit>
#include <cstdint>
#include <type_traits>

#include "core_type.hpp"


#if (defined(_MSC_VER) && defined(_WIN32))
#    include    <stdlib.h>
#endif

namespace core
{

namespace _p
{
	///	\brief Swaps the byte order of a 2Byte variable
	[[nodiscard]] inline constexpr uint16_t byte_swap_16(uint16_t const p_in)
	{
#if (defined (__GNUG__) or defined(__GNUC__) or defined(__clang__))
		if(!std::is_constant_evaluated())
		{
			return __builtin_bswap16(p_in);
		}
		else
#elif (defined(_MSC_VER) && defined(_WIN32))
		if(!std::is_constant_evaluated())
		{
			return _byteswap_ushort(p_in);
		}
		else
#endif
		{
			return static_cast<uint16_t>((p_in >> 8) bitor (p_in << 8));
		}
	}

	///	\brief Reverses the byte order of a 4Byte variable
	[[nodiscard]] inline constexpr uint32_t byte_swap_32(uint32_t const p_in)
	{
#if (defined (__GNUG__) or defined(__GNUC__) or defined(__clang__))
		if(!std::is_constant_evaluated())
		{
			return __builtin_bswap32(p_in);
		}
		else
#elif (defined(_MSC_VER) && defined(_WIN32))
		if(!std::is_constant_evaluated())
		{
			return _byteswap_ulong(p_in);
		}
		else
#endif
		{
			return static_cast<uint32_t>(
				 (p_in >> 24) bitor
				((p_in >>  8) & 0x0000FF00)bitor
				((p_in <<  8) & 0x00FF0000) bitor 
				 (p_in << 24));
		}
	}

	///	\brief Reverses the byte order of a 8Byte variable
	[[nodiscard]] inline constexpr uint64_t byte_swap_64(uint64_t const p_in)
	{
#if (defined (__GNUG__) or defined(__GNUC__) or defined(__clang__))
		if(!std::is_constant_evaluated())
		{
			return __builtin_bswap64(p_in);
		}
		else
#elif (defined(_MSC_VER) && defined(_WIN32))
		if(!std::is_constant_evaluated())
		{
			return _byteswap_uint64(p_in);
		}
		else
#endif
		{
			return static_cast<uint64_t>(
				 (p_in >> 56) bitor
				((p_in >> 40) & 0x0000'0000'0000'FF00) bitor
				((p_in >> 24) & 0x0000'0000'00FF'0000) bitor
				((p_in >>  8) & 0x0000'0000'FF00'0000) bitor
				((p_in <<  8) & 0x0000'00FF'0000'0000) bitor
				((p_in << 24) & 0x0000'FF00'0000'0000) bitor
				((p_in << 40) & 0x00FF'0000'0000'0000) bitor
				 (p_in << 56));
		}
	}


	template<typename T>
	constexpr bool endian_swap_support_base_v = std::is_integral_v<T> &&
		(
			(sizeof(T) == sizeof(uint8_t ) && alignof(T) == alignof(uint8_t )) or
			(sizeof(T) == sizeof(uint16_t) && alignof(T) == alignof(uint16_t)) or
			(sizeof(T) == sizeof(uint32_t) && alignof(T) == alignof(uint32_t)) or
			(sizeof(T) == sizeof(uint64_t) && alignof(T) == alignof(uint64_t))
		);


	template<uintptr_t, uintptr_t>
	struct suitable_uint
	{
		static constexpr bool has_type = false;
	};

	template<> struct suitable_uint<sizeof(uint8_t ), alignof(uint8_t )> { using type = uint8_t;  static constexpr bool has_type = true; };
	template<> struct suitable_uint<sizeof(uint16_t), alignof(uint16_t)> { using type = uint16_t; static constexpr bool has_type = true; };
	template<> struct suitable_uint<sizeof(uint32_t), alignof(uint32_t)> { using type = uint32_t; static constexpr bool has_type = true; };
	template<> struct suitable_uint<sizeof(uint64_t), alignof(uint64_t)> { using type = uint64_t; static constexpr bool has_type = true; };

	template<typename T>
	constexpr bool endian_swap_support_cast_v =
		endian_swap_support_base_v<T> or
		(
			suitable_uint<sizeof(T), alignof(T)>::has_type &&
			std::is_enum_v<T>
		);


	template<typename T>
	constexpr bool endian_swap_support_alias_ex_v =
		suitable_uint<sizeof(T), alignof(T)>::has_type &&
		std::is_floating_point_v<T>;

	template<typename T>
	constexpr bool endian_swap_supported_v = endian_swap_support_cast_v<T> or endian_swap_support_alias_ex_v<T>;

	template<typename T>
	concept endian_support_cast_c = endian_swap_support_cast_v<T>;

	template<typename T>
	concept endian_support_alias_ex_c = endian_swap_support_alias_ex_v<T>;

	template<typename T>
	concept endian_swap_supported_c = endian_swap_supported_v<T>;

	template<endian_swap_supported_c T>
	struct endianess_uint_align
	{
		using type = typename suitable_uint<sizeof(T), alignof(T)>::type;
	};

	template<endian_swap_supported_c T>
	using endianess_uint_align_t = typename endianess_uint_align<T>::type;


} //namespace _p

template<_p::endian_support_cast_c T>
[[nodiscard]] inline constexpr T byte_swap(T const& p_value)
{
	if constexpr(std::is_same_v<uint8_t, _p::endianess_uint_align_t<T>>)
	{
		return p_value;
	}
	else if constexpr(std::is_same_v<uint16_t, _p::endianess_uint_align_t<T>>)
	{
		return static_cast<T const>(_p::byte_swap_16(static_cast<uint16_t const>(p_value)));
	}
	else if constexpr(std::is_same_v<uint32_t, _p::endianess_uint_align_t<T>>)
	{
		return static_cast<T const>(_p::byte_swap_32(static_cast<uint32_t const>(p_value)));
	}
	else if constexpr(std::is_same_v<uint64_t, _p::endianess_uint_align_t<T>>)
	{
		return static_cast<T const>(_p::byte_swap_64(static_cast<uint64_t const>(p_value)));
	}
	else
	{
		static_assert(
			std::is_same_v<uint8_t, _p::endianess_uint_align_t<T>> or
			std::is_same_v<uint16_t, _p::endianess_uint_align_t<T>> or
			std::is_same_v<uint32_t, _p::endianess_uint_align_t<T>> or
			std::is_same_v<uint64_t, _p::endianess_uint_align_t<T>>
			, "Unsuported type");
	}
}

template<_p::endian_support_alias_ex_c T>
[[nodiscard]] inline constexpr T byte_swap(T const& p_value)
{
	return std::bit_cast<T const>(byte_swap(std::bit_cast<_p::endianess_uint_align_t<T> const>(p_value)));
}

template <_p::endian_swap_supported_c T>
[[nodiscard]] inline constexpr T endian_host2little(T const p_in)
{
	if constexpr(std::endian::native == std::endian::little)
	{
		return p_in;
	}
	else if constexpr(std::endian::native == std::endian::big)
	{
		return byte_swap(p_in);
	}
	else
	{
		static_assert(
			std::endian::native == std::endian::little or
			std::endian::native == std::endian::big,
			"Unsuported host endianess");
	}
}

template <_p::endian_swap_supported_c T>
[[nodiscard]] inline constexpr T endian_little2host(T const p_in)
{
	return endian_host2little(p_in);
}

template <_p::endian_swap_supported_c T>
[[nodiscard]] inline constexpr T endian_host2big(T const p_in)
{
	if constexpr(std::endian::native == std::endian::little)
	{
		return byte_swap(p_in);
	}
	else if constexpr(std::endian::native == std::endian::big)
	{
		return p_in;
	}
	else
	{
		static_assert(
			std::endian::native == std::endian::little or
			std::endian::native == std::endian::big,
			"Unsuported host endianess");
	}
}

template <_p::endian_swap_supported_c T>
[[nodiscard]] inline constexpr T endian_big2host(T const p_in)
{
	return endian_host2big(p_in);
}

} //namespace core

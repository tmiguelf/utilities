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
#include <bit>

#include "core_type.hpp"

#ifdef _WIN32
#	include	<stdlib.h>
#endif

namespace core
{

namespace _p
{
	///	\brief Swaps the byte order of a 2Byte variable
	[[nodiscard]] inline constexpr uint16_t byte_swap_16(uint16_t const p_in)
	{
	#ifdef _WIN32
		if(std::is_constant_evaluated())
		{
			return (p_in >> 8) | (p_in << 8);
		}
		else
		{
			return _byteswap_ushort(p_in);
		}
	#else
		return __builtin_bswap16(p_in);
	#endif
	}

	///	\brief Reverses the byte order of a 4Byte variable
	[[nodiscard]] inline constexpr uint32_t byte_swap_32(uint32_t const p_in)
	{
	#ifdef _WIN32
		if(std::is_constant_evaluated())
		{
			return
				(p_in >> 24) |
				(p_in >> 8) & 0x0000FF00 |
				(p_in << 8) & 0x00FF0000 | 
				(p_in << 24);
		}
		else
		{
			return _byteswap_ulong(p_in);
		}
	#else
		return __builtin_bswap32(p_in);
	#endif
	}

	///	\brief Reverses the byte order of a 8Byte variable
	[[nodiscard]] inline constexpr uint64_t byte_swap_64(uint64_t const p_in)
	{
	#ifdef _WIN32
		if(std::is_constant_evaluated())
		{
			return
				(p_in >> 56) |
				(p_in >> 40) & 0x0000'0000'0000'FF00 |
				(p_in >> 24) & 0x0000'0000'00FF'0000 |
				(p_in >>  8) & 0x0000'0000'FF00'0000 |
				(p_in <<  8) & 0x0000'00FF'0000'0000 |
				(p_in << 24) & 0x0000'FF00'0000'0000 |
				(p_in << 40) & 0x00FF'0000'0000'0000 |
				(p_in << 56);
		}
		else
		{
			return _byteswap_uint64(p_in);
		}
	#else
		return __builtin_bswap64(p_in);
	#endif
	}

	template<typename T>
	constexpr bool endian_supported_base_type_v = std::is_fundamental_v<T> && !std::is_floating_point_v<T> &&
		(
			(sizeof(T) == sizeof(uint8_t ) && alignof(T) == alignof(uint8_t )) ||
			(sizeof(T) == sizeof(uint16_t) && alignof(T) == alignof(uint16_t)) ||
			(sizeof(T) == sizeof(uint32_t) && alignof(T) == alignof(uint32_t)) ||
			(sizeof(T) == sizeof(uint64_t) && alignof(T) == alignof(uint64_t))
		);

	template<typename T>
	struct assist_underlying_type { using type = T; };

	template<typename T> requires std::is_enum_v<T>
	struct assist_underlying_type<T> { using type = std::underlying_type_t<T>; };

	template<typename T>
	constexpr bool endian_supported_type_v = endian_supported_base_type_v<typename assist_underlying_type<T>::type>;

	template<typename T>
	concept endian_supported_type_c = endian_supported_type_v<T>;

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
	constexpr bool is_endian_swap_suitable_v =
		(
			endian_supported_type_v<T> ||
			std::is_trivially_constructible_v<T> &&
			std::is_trivially_copy_constructible_v<T> &&
			std::is_trivially_destructible_v<T>
		) &&
		suitable_uint<sizeof(T), alignof(T)>::has_type;

	template<typename T>
	constexpr bool is_endian_alias_exclusive_v =
		(
			!endian_supported_type_v<T> &&
			std::is_trivially_constructible_v<T> &&
			std::is_trivially_copy_constructible_v<T> &&
			std::is_trivially_destructible_v<T>
		) &&
		suitable_uint<sizeof(T), alignof(T)>::has_type;

	template<typename T>
	concept is_endian_swap_suitable_c = is_endian_swap_suitable_v<T>;

	template<typename T>
	concept is_endian_alias_exclusive_c = is_endian_alias_exclusive_v<T>;


	template<typename T> requires is_endian_swap_suitable_v<T>
	struct endianess_uint_align
	{
		using type = typename suitable_uint<sizeof(T), alignof(T)>::type;
	};

	template<typename T>
	using endianess_uint_align_t = typename endianess_uint_align<T>::type;
} //namespace _p

template<_p::endian_supported_type_c T>
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
			std::is_same_v<uint8_t, _p::endianess_uint_align_t<T>> ||
			std::is_same_v<uint16_t, _p::endianess_uint_align_t<T>> ||
			std::is_same_v<uint32_t, _p::endianess_uint_align_t<T>> ||
			std::is_same_v<uint64_t, _p::endianess_uint_align_t<T>>
			, "Unsuported type");
	}
}

template<_p::is_endian_alias_exclusive_c T>
[[nodiscard]] inline constexpr T byte_swap(T const& p_value)
{
	return std::bit_cast<const T>(byte_swap(std::bit_cast<_p::endianess_uint_align_t<T> const>(p_value)));
}

template <_p::is_endian_swap_suitable_c T>
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
			std::endian::native == std::endian::little ||
			std::endian::native == std::endian::big,
			"Unsuported host endianess");
	}
}

template <_p::is_endian_swap_suitable_c T>
[[nodiscard]] inline constexpr T endian_little2host(T const p_in)
{
	return endian_host2little(p_in);
}

template <_p::is_endian_swap_suitable_c T>
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
			std::endian::native == std::endian::little ||
			std::endian::native == std::endian::big,
			"Unsuported host endianess");
	}
}

template <_p::is_endian_swap_suitable_c T>
[[nodiscard]] inline constexpr T endian_big2host(T const p_in)
{
	return endian_host2big(p_in);
}

} //namespace core

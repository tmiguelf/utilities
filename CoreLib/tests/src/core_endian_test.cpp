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

#include <utility>
#include <type_traits>

#include <CoreLib/Core_Endian.hpp>

#include <gtest/gtest.h>

template<typename T> requires (sizeof(T) == sizeof(uint8_t))
consteval std::pair<T, T> getTestCase_const()
{
	return {static_cast<const T>(uint8_t{0x01}), static_cast<const T>(uint8_t{0x01})};
}

template<typename T> requires (sizeof(T) == sizeof(uint16_t))
consteval std::pair<T, T> getTestCase_const()
{
	return {static_cast<const T>(uint16_t{0x0123}), static_cast<const T>(uint16_t{0x2301})};
}

template<typename T> requires (sizeof(T) == sizeof(uint32_t))
consteval std::pair<T, T> getTestCase_const()
{
	return {static_cast<const T>(uint32_t{0x01234567}), static_cast<const T>(uint32_t{0x67452301})};
}

template<typename T> requires (sizeof(T) == sizeof(uint64_t))
consteval std::pair<T, T> getTestCase_const()
{
	return {static_cast<const T>(uint64_t{0x01456789ABCDEF23}), static_cast<const T>(uint64_t{0x23EFCDAB89674501})};
}


template<typename T> requires (sizeof(T) == sizeof(uint8_t))
std::pair<T, T> getTestCase()
{
	const uint8_t var1{0x01};
	const uint8_t var2{0x01};
	return { std::bit_cast<const T>(var1), std::bit_cast<const T>(var2)};
}

template<typename T> requires (sizeof(T) == sizeof(uint16_t))
std::pair<T, T> getTestCase()
{
	const uint16_t var1{0x0123};
	const uint16_t var2{0x2301};
	return { std::bit_cast<const T>(var1), std::bit_cast<const T>(var2)};
}

template<typename T> requires (sizeof(T) == sizeof(uint32_t))
std::pair<T, T> getTestCase()
{
	const uint32_t var1{0x01234567};
	const uint32_t var2{0x67452301};
	return { std::bit_cast<const T>(var1), std::bit_cast<const T>(var2)};
}

template<typename T> requires (sizeof(T) == sizeof(uint64_t))
std::pair<T, T> getTestCase()
{
	const uint64_t var1{0x0123456789ABCDEF};
	const uint64_t var2{0xEFCDAB8967452301};
	return { std::bit_cast<const T>(var1), std::bit_cast<const T>(var2)};
}

enum class enumu8 : uint8_t {};
enum class enums8 :  int8_t {};
enum class enumu16: uint16_t{};
enum class enums16:  int16_t{};
enum class enumu32: uint32_t{};
enum class enums32:  int32_t{};
enum class enumu64: uint64_t{};
enum class enums64:  int64_t{};


TEST(core_endian, validateTestCase)
{
	{
		std::pair<uint8_t, uint8_t> res = getTestCase<uint8_t>();
		ASSERT_EQ(res.first,	uint8_t{0x01});
		ASSERT_EQ(res.second,	uint8_t{0x01});
	}

	{
		std::pair<uint16_t, uint16_t> res = getTestCase<uint16_t>();
		ASSERT_EQ(res.first,	uint16_t{0x0123});
		ASSERT_EQ(res.second,	uint16_t{0x2301});
	}

	{
		std::pair<uint32_t, uint32_t> res = getTestCase<uint32_t>();
		ASSERT_EQ(res.first,	uint32_t{0x01234567});
		ASSERT_EQ(res.second,	uint32_t{0x67452301});
	}

	{
		std::pair<uint64_t, uint64_t> res = getTestCase<uint64_t>();
		ASSERT_EQ(res.first,	uint64_t{0x0123456789ABCDEF});
		ASSERT_EQ(res.second,	uint64_t{0xEFCDAB8967452301});
	}

	{
		std::pair<float, float> res = getTestCase<float>();
		ASSERT_EQ(res.first,	float{2.9988165e-38f});
		ASSERT_EQ(res.second,	float{9.3095191e+23f});
	}

	{
		std::pair<double, double> res = getTestCase<double>();
		ASSERT_EQ(res.first,	double{3.5127005640885037e-303});
		ASSERT_EQ(res.second,	double{-3.59869634924474792e+230});
	}
}

template<typename T>
class endian_conv_constexpr_T : public testing::Test {
protected:
	endian_conv_constexpr_T() {}
};

using endianConstexprTypes = ::testing::Types<
	uint8_t,
	uint16_t,
	uint32_t,
	uint64_t,
	int8_t,
	int16_t,
	int32_t,
	int64_t,
	enumu8,
	enums8,
	enumu16,
	enums16,
	enumu32,
	enums32,
	enumu64,
	enums64
	>;

TYPED_TEST_SUITE(endian_conv_constexpr_T, endianConstexprTypes);

TYPED_TEST(endian_conv_constexpr_T, byte_swap)
{
	using num_T = TypeParam;
	constexpr std::pair<num_T, num_T> tcase = getTestCase_const<num_T>();
	constexpr num_T result = core::byte_swap(tcase.first);

	static_assert(result == tcase.second, "Test failed");
}

TYPED_TEST(endian_conv_constexpr_T, endian_host2little)
{
	using num_T = TypeParam;
	constexpr std::pair<num_T, num_T> tcase = getTestCase_const<num_T>();

	constexpr num_T result = core::endian_host2little(tcase.first);

	if constexpr(std::endian::native == std::endian::little)
	{
		static_assert(result == tcase.first, "Test failed");
	}
	else if constexpr(std::endian::native == std::endian::big)
	{
		static_assert(result == tcase.second, "Test failed");
	}
}

TYPED_TEST(endian_conv_constexpr_T, endian_little2host)
{
	using num_T = TypeParam;
	constexpr std::pair<num_T, num_T> tcase = getTestCase_const<num_T>();

	constexpr num_T result = core::endian_little2host(tcase.first);

	if constexpr(std::endian::native == std::endian::little)
	{
		static_assert(result == tcase.first, "Test failed");
	}
	else if constexpr(std::endian::native == std::endian::big)
	{
		static_assert(result == tcase.second, "Test failed");
	}
}

TYPED_TEST(endian_conv_constexpr_T, endian_host2big)
{
	using num_T = TypeParam;
	constexpr std::pair<num_T, num_T> tcase = getTestCase_const<num_T>();

	constexpr num_T result = core::endian_host2big(tcase.first);

	if constexpr(std::endian::native == std::endian::little)
	{
		static_assert(result == tcase.second, "Test failed");
	}
	else if constexpr(std::endian::native == std::endian::big)
	{
		static_assert(result == tcase.first, "Test failed");
	}
}

TYPED_TEST(endian_conv_constexpr_T, endian_big2host)
{
	using num_T = TypeParam;
	constexpr std::pair<num_T, num_T> tcase = getTestCase_const<num_T>();

	constexpr num_T result = core::endian_big2host(tcase.first);

	if constexpr(std::endian::native == std::endian::little)
	{
		static_assert(result == tcase.second, "Test failed");
	}
	else if constexpr(std::endian::native == std::endian::big)
	{
		static_assert(result == tcase.first, "Test failed");
	}
}

template<typename T>
class endian_conv_T : public testing::Test {
protected:
	endian_conv_T() {}
};

using endianTypes = ::testing::Types<
	uint8_t,
	uint16_t,
	uint32_t,
	uint64_t,
	int8_t,
	int16_t,
	int32_t,
	int64_t,
	enumu8,
	enums8,
	enumu16,
	enums16,
	enumu32,
	enums32,
	enumu64,
	enums64,
	float,
	double
>;
TYPED_TEST_SUITE(endian_conv_T, endianTypes);

TYPED_TEST(endian_conv_T, byte_swap)
{
	using num_T = TypeParam;
	std::pair<num_T, num_T> tcase = getTestCase<num_T>();
	
	num_T input = tcase.first;
	num_T result = core::byte_swap(input);

	ASSERT_EQ(result, tcase.second);
}

TYPED_TEST(endian_conv_T, endian_host2little)
{
	using num_T = TypeParam;
	std::pair<num_T, num_T> tcase = getTestCase<num_T>();

	num_T input = tcase.first;
	num_T result = core::endian_host2little(input);

	if constexpr(std::endian::native == std::endian::little)
	{
		ASSERT_EQ(result, tcase.first);
	}
	else if constexpr(std::endian::native == std::endian::big)
	{
		ASSERT_EQ(result, tcase.second);
	}
}

TYPED_TEST(endian_conv_T, endian_little2host)
{
	using num_T = TypeParam;
	std::pair<num_T, num_T> tcase = getTestCase<num_T>();

	num_T input = tcase.first;
	num_T result = core::endian_little2host(input);

	if constexpr(std::endian::native == std::endian::little)
	{
		ASSERT_EQ(result, tcase.first);
	}
	else if constexpr(std::endian::native == std::endian::big)
	{
		ASSERT_EQ(result, tcase.second);
	}
}

TYPED_TEST(endian_conv_T, endian_host2big)
{
	using num_T = TypeParam;
	std::pair<num_T, num_T> tcase = getTestCase<num_T>();

	num_T input = tcase.first;
	num_T result = core::endian_host2big(input);

	if constexpr(std::endian::native == std::endian::little)
	{
		ASSERT_EQ(result, tcase.second);
	}
	else if constexpr(std::endian::native == std::endian::big)
	{
		ASSERT_EQ(result, tcase.first);
	}
}

TYPED_TEST(endian_conv_T, endian_big2host)
{
	using num_T = TypeParam;
	std::pair<num_T, num_T> tcase = getTestCase<num_T>();

	num_T input = tcase.first;
	num_T result = core::endian_big2host(input);

	if constexpr(std::endian::native == std::endian::little)
	{
		ASSERT_EQ(result, tcase.second);
	}
	else if constexpr(std::endian::native == std::endian::big)
	{
		ASSERT_EQ(result, tcase.first);
	}
}

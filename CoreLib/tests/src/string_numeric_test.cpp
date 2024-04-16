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

#include <ostream>
#include <string>
#include <vector>
#include <utility>
#include <charconv>
#include <array>
#include <limits>
#include <type_traits>
#include <string_view>

#include <CoreLib/core_type.hpp>

using ::core::literals::operator "" _ui64;

//======== ======== ======== Stream helper ======== ======== ========

using stream_t = std::basic_ostream<char>;

static stream_t& operator << (stream_t& p_stream, std::u8string const& p_str)
{
	p_stream.write(reinterpret_cast<char const*>(p_str.data()), p_str.size());
	return p_stream;
}

/*
static stream_t& operator << (stream_t& p_stream, std::u8string_view p_str)
{
	p_stream.write(reinterpret_cast<char const*>(p_str.data()), p_str.size());
	return p_stream;
}*/

static stream_t& operator << (stream_t& p_stream, std::u16string const& p_str)
{
	std::u8string temp;
	temp.resize(p_str.size());

	{
		uintptr_t i = 0;
		for(char16_t t_char : p_str)
		{
			temp[i++] = static_cast<char8_t>(t_char);
		}
	}

	return p_stream << temp;
}

static stream_t& operator << (stream_t& p_stream, std::u32string const& p_str)
{
	std::u8string temp;
	temp.resize(p_str.size());

	{
		uintptr_t i = 0;
		for(char32_t t_char : p_str)
		{
			temp[i++] = static_cast<char8_t>(t_char);
		}
	}

	return p_stream << temp;
}



#include <gtest/gtest.h>

#include <CoreLib/string/core_string_numeric.hpp>
#include <CoreLib/string/core_string_misc.hpp>


namespace numeric
{

template <typename char_T>
std::basic_string<char_T> str2_Tstring(std::string_view p_str)
{
	std::basic_string<char_T> out;
	char const* first = p_str.data();
	char const* last = first + p_str.size();

	out.resize(p_str.size());

	uintptr_t i = 0;
	do
	{
		out[i++] = static_cast<char_T>(*first);
	}
	while(++first < last);
	return out;
}


//======== ======== ======== Decimal test Cases ======== ======== ========

static std::vector<int64_t> const s_numbers =
{
	-0,
	-1,
	-2,
	-3,
	-4,
	-5,
	-6,
	-7,
	-8,
	-9,
	-10,
	-11,
	-15,
	-21,
	-34,
	-42,
	-99,
	-100,
	-101,
	-123,
	-127,
	-128,
	-129,
	-255,
	-256,
	-999,
	-1000,
	-1234,
	-9999,
	-10000,
	-12345,
	-32767,
	-32768,
	-51234,
	-65535,
	-65536,
	-65537,
	-99999,
	-100000,
	-999999,
	-1000000,
	-9999999,
	-10000000,
	-99999999,
	-100000000,
	-999999999,
	-1000000000,
	-2147483647,
	-2147483648,
	-2147483649,
	-4294967295,
	-4294967296,
	-4294967297,
	-9999999999,
	-10000000000,
	-99999999999,
	-100000000000,
	-999999999999,
	-1000000000000,
	-9999999999999,
	-10000000000000,
	-99999999999999,
	-100000000000000,
	-999999999999999,
	-1000000000000000,
	-9999999999999999,
	-10000000000000000,
	-99999999999999999,
	-100000000000000000,
	-999999999999999999,
	-1000000000000000000,
	-9223372036854775807,
};

static std::vector<uint64_t> const u_numbers =
{
	0,
	1,
	2,
	3,
	4,
	5,
	6,
	7,
	8,
	9,
	10,
	11,
	15,
	21,
	34,
	42,
	99,
	100,
	101,
	123,
	127,
	128,
	255,
	256,
	999,
	1000,
	1234,
	9999,
	10000,
	12345,
	32767,
	32768,
	51234,
	65535,
	65536,
	99999,
	100000,
	999999,
	1000000,
	9999999,
	10000000,
	99999999,
	100000000,
	999999999,
	1000000000,
	2147483647,
	2147483648,
	4294967295,
	4294967296,
	9999999999,
	10000000000,
	99999999999,
	100000000000,
	999999999999,
	1000000000000,
	9999999999999,
	10000000000000,
	99999999999999,
	100000000000000,
	999999999999999,
	1000000000000000,
	9999999999999999,
	10000000000000000,
	99999999999999999,
	100000000000000000,
	999999999999999999,
	1000000000000000000,
	9223372036854775807,
	9223372036854775808_ui64,
	9999999999999999999_ui64,
	10000000000000000000_ui64,
	18446744073709551615_ui64
};

std::vector<std::string> const badCases_s =
{
	"",
	"/",
	":",
	"false",
	"true",
	"0G",
	"0f",
	"A",
	"a",
	"T",
	"G",
	"g",
	"!",
	" ",
	"-.E-",
	"-",
	"+",
	".",
	"E",
	std::string{"\0", 1},
	"12 3",
	" 123",
	"123 ",
};

template <typename num_T>
struct one_past_end;

template <typename num_T>
struct one_past_end_s;

template <> struct one_past_end<uint8_t >{ static constexpr std::string_view str = "256"; };
template <> struct one_past_end<uint16_t>{ static constexpr std::string_view str = "65536";};
template <> struct one_past_end<uint32_t>{ static constexpr std::string_view str = "4294967296";};
template <> struct one_past_end<uint64_t>{ static constexpr std::string_view str = "18446744073709551616";};
template <> struct one_past_end< int8_t >{ static constexpr std::string_view str = "128";};
template <> struct one_past_end< int16_t>{ static constexpr std::string_view str = "32768";};
template <> struct one_past_end< int32_t>{ static constexpr std::string_view str = "2147483648";};
template <> struct one_past_end< int64_t>{ static constexpr std::string_view str = "9223372036854775808";};

template <> struct one_past_end_s< int8_t >{ static constexpr std::string_view str = "-129";};
template <> struct one_past_end_s< int16_t>{ static constexpr std::string_view str = "-32769";};
template <> struct one_past_end_s< int32_t>{ static constexpr std::string_view str = "-2147483649";};
template <> struct one_past_end_s< int64_t>{ static constexpr std::string_view str = "-9223372036854775809"; };

template <typename num_T, typename char_T>
std::vector<std::pair<num_T, std::basic_string<char_T>>> get_goodCases()
{
	if constexpr(std::is_floating_point_v<num_T>)
	{
		if constexpr (std::is_same_v<num_T, float32_t>)
		{
			return
			{
				{ 0.0f		, str2_Tstring<char_T>("0")},
				{-0.0f		, str2_Tstring<char_T>("-0")},
				{ 0.1f		, str2_Tstring<char_T>("0.1")},
				{-0.1f		, str2_Tstring<char_T>("-0.1")},
				{ 1.0f		, str2_Tstring<char_T>("1")},
				{-1.0f		, str2_Tstring<char_T>("-1")},
				{ 1.1f		, str2_Tstring<char_T>("1.1")},
				{-1.1f		, str2_Tstring<char_T>("-1.1")},
				{ 123456.0f	, str2_Tstring<char_T>("123456")},
				{-123456.0f	, str2_Tstring<char_T>("-123456")},
				{ 0.1234f	, str2_Tstring<char_T>("0.1234")},
				{-0.1234f	, str2_Tstring<char_T>("-0.1234")},
			};
		}
		else if constexpr (std::is_same_v<num_T, float64_t>)
		{
			return
			{
				{ 0.0		, str2_Tstring<char_T>("0")},
				{-0.0		, str2_Tstring<char_T>("-0")},
				{ 0.1		, str2_Tstring<char_T>("0.1")},
				{-0.1		, str2_Tstring<char_T>("-0.1")},
				{ 1.0		, str2_Tstring<char_T>("1")},
				{-1.0		, str2_Tstring<char_T>("-1")},
				{ 1.1		, str2_Tstring<char_T>("1.1")},
				{-1.1		, str2_Tstring<char_T>("-1.1")},
				{ 123456.0	, str2_Tstring<char_T>("123456")},
				{-123456.0	, str2_Tstring<char_T>("-123456")},
				{ 0.1234	, str2_Tstring<char_T>("0.1234")},
				{-0.1234	, str2_Tstring<char_T>("-0.1234")},
			};
		}
	}
	else
	{
		std::vector<std::pair<num_T, std::basic_string<char_T>>> out;
		constexpr num_T max = std::numeric_limits<num_T>::max();
		std::array<char, 20> buff;
		char* first = buff.data();
		char* last = first + buff.size();

		for(uint64_t num : u_numbers)
		{
			if(num <= max)
			{
				std::to_chars_result res = std::to_chars(first, last, num);
				out.push_back({static_cast<num_T>(num), str2_Tstring<char_T>(std::string_view{first, static_cast<uintptr_t>(res.ptr - first)})});
			}
			else break;
		}
		{
			std::to_chars_result res = std::to_chars(first, last, max);
			out.push_back({max, str2_Tstring<char_T>(std::string_view{first, static_cast<uintptr_t>(res.ptr - first)})});
		}
		if constexpr(std::is_signed_v<num_T>)
		{
			constexpr num_T min = std::numeric_limits<num_T>::min();
			for(int64_t num : s_numbers)
			{
				if(num >= min)
				{
					std::to_chars_result res = std::to_chars(first, last, num);
					out.push_back({static_cast<num_T>(num), str2_Tstring<char_T>(std::string_view{first, static_cast<uintptr_t>(res.ptr - first)})});
				}
				else break;
			}
			{
				std::to_chars_result res = std::to_chars(first, last, min);
				out.push_back({min, str2_Tstring<char_T>(std::string_view{first, static_cast<uintptr_t>(res.ptr - first)})});
			}
		}

		return out;
	}
}

template <typename num_T, typename char_T>
std::vector<std::pair<num_T, std::basic_string<char_T>>> getFpCases_extra()
{
	if constexpr (std::is_same_v<num_T, float32_t>)
	{
		return
		{
			{ 3.402823466e+38f	, str2_Tstring<char_T>("3.402823466e+38")},
			{-3.402823466e+38f	, str2_Tstring<char_T>("-3.402823466e+38")},
			{ 1.175494351e-38f	, str2_Tstring<char_T>("1.175494351e-38")},
			{-1.175494351e-38f	, str2_Tstring<char_T>("-1.175494351e-38")},
		};
	}
	else if constexpr (std::is_same_v<num_T, float64_t>)
	{
		return 
		{
			{ 1.7976931348623158e+308	, str2_Tstring<char_T>("1.7976931348623158e+308")},
			{-1.7976931348623158e+308	, str2_Tstring<char_T>("-1.7976931348623158e+308")},
			{ 2.2250738585072014e-308	, str2_Tstring<char_T>("2.2250738585072014e-308")},
			{-2.2250738585072014e-308	, str2_Tstring<char_T>("-2.2250738585072014e-308")},
		};
	}
}

template <typename num_T, typename char_T>
std::vector<std::basic_string<char_T>> get_badCases()
{
	std::vector<std::basic_string<char_T>> out;

	for(std::string const& t_str : badCases_s)
	{
		out.emplace_back(str2_Tstring<char_T>(t_str));
	}
	if constexpr (!std::is_floating_point_v<num_T>)
	{
		if constexpr(std::is_signed_v<num_T>)
		{
			out.emplace_back(str2_Tstring<char_T>(one_past_end_s<num_T>::str));
		}
		else
		{
			out.emplace_back(str2_Tstring<char_T>("-1"));
			out.emplace_back(str2_Tstring<char_T>("-0"));
		}

		out.emplace_back(str2_Tstring<char_T>("1.0"));
		out.emplace_back(str2_Tstring<char_T>(one_past_end<num_T>::str));
	}
	return out;
}


//======== ======== ======== Decimal test Suit ======== ======== ========

template<typename T>
class charconv_Decimal_T : public testing::Test {
protected:
	charconv_Decimal_T() {}
};

using DecTypes = ::testing::Types<
	std::pair<uint8_t		, char8_t >,
	std::pair<uint16_t		, char8_t >,
	std::pair<uint32_t		, char8_t >,
	std::pair<uint64_t		, char8_t >,
	std::pair<int8_t		, char8_t >,
	std::pair<int16_t		, char8_t >,
	std::pair<int32_t		, char8_t >,
	std::pair<int64_t		, char8_t >,
	std::pair<float32_t		, char8_t >,
	std::pair<float64_t		, char8_t >,
	std::pair<uint8_t		, char16_t>,
	std::pair<uint16_t		, char16_t>,
	std::pair<uint32_t		, char16_t>,
	std::pair<uint64_t		, char16_t>,
	std::pair<int8_t		, char16_t>,
	std::pair<int16_t		, char16_t>,
	std::pair<int32_t		, char16_t>,
	std::pair<int64_t		, char16_t>,
	std::pair<float32_t		, char16_t>,
	std::pair<float64_t		, char16_t>,
	std::pair<uint8_t		, char32_t>,
	std::pair<uint16_t		, char32_t>,
	std::pair<uint32_t		, char32_t>,
	std::pair<uint64_t		, char32_t>,
	std::pair<int8_t		, char32_t>,
	std::pair<int16_t		, char32_t>,
	std::pair<int32_t		, char32_t>,
	std::pair<int64_t		, char32_t>,
	std::pair<float32_t		, char32_t>,
	std::pair<float64_t		, char32_t>
>;

TYPED_TEST_SUITE(charconv_Decimal_T, DecTypes);

TYPED_TEST(charconv_Decimal_T, from_chars_good)
{
	using num_T = typename TypeParam::first_type;
	using char_T = typename TypeParam::second_type;

	{
		std::vector<std::pair<num_T, std::basic_string<char_T>>> const& goodCases = get_goodCases<num_T, char_T>();
		for(std::pair<num_T, std::basic_string<char_T>> const& testCase: goodCases)
		{
			core::from_chars_result<num_T> result = core::from_chars<num_T>(testCase.second);

			ASSERT_TRUE(result.has_value()) << "Case " << testCase.second;
			ASSERT_EQ(result.value(), testCase.first);
		}
	}

	if constexpr (std::is_floating_point_v<num_T>)
	{
		std::vector<std::pair<num_T, std::basic_string<char_T>>> const& cases = getFpCases_extra<num_T, char_T>();
		for(std::pair<num_T, std::basic_string<char_T>> const& testCase: cases)
		{
			core::from_chars_result<num_T> result = core::from_chars<num_T>(testCase.second);

			ASSERT_TRUE(result.has_value()) << "Case " << testCase.second;
			ASSERT_EQ(result.value(), testCase.first);
		}
	}

}

TYPED_TEST(charconv_Decimal_T, from_chars_bad)
{
	using num_T = typename TypeParam::first_type;
	using char_T = typename TypeParam::second_type;

	std::vector<std::basic_string<char_T>> const& badCases = get_badCases<num_T, char_T>();
	for(std::basic_string<char_T> const& testCase: badCases)
	{
		core::from_chars_result<num_T> result = core::from_chars<num_T>(testCase);

		ASSERT_FALSE(result.has_value()) << "Case " << testCase;
	}
}


//======== ======== ======== Hex test Cases ======== ======== ========

static std::vector<uint64_t> const hex_numbers =
{
	0x0,
	0x1,
	0x2,
	0x3,
	0x4,
	0x5,
	0x6,
	0x7,
	0x8,
	0x9,
	0xA,
	0xB,
	0xC,
	0xD,
	0xE,
	0xF,
	0x10,
	0x20,
	0x40,
	0x42,
	0x80,
	0xFF,
	0x100,
	0x123,
	0x200,
	0xFFF,
	0x1000,
	0xFFFF,
	0x10000,
	0x12345,
	0x20000,
	0x40000,
	0x80000,
	0xFFFFFFFF,
	0x100000000,
	0x1000000000000000,
	0x123456789ABCDEF0,
	0x2000000000000000,
	0x4000000000000000,
	0x8000000000000000,
	0xFEDCBA9876543210,
	0xFFFFFFFFFFFFFFFF,
};

std::vector<std::string> const hexBadCases_s =
{
	"",
	"/",
	":",
	"@",
	"`",
	"false",
	"true",
	"0G",
	"G",
	"g",
	"!",
	" ",
	"-1",
	"1.0",
	std::string{"\0", 1},
	"12 3",
	" 123",
	"123 ",
};


template <typename num_T>
struct one_past_end_hex;

template <> struct one_past_end_hex<uint8_t >{ static constexpr std::string_view str = "100"; };
template <> struct one_past_end_hex<uint16_t>{ static constexpr std::string_view str = "10000";};
template <> struct one_past_end_hex<uint32_t>{ static constexpr std::string_view str = "100000000";};
template <> struct one_past_end_hex<uint64_t>{ static constexpr std::string_view str = "10000000000000000";};


template <typename num_T, typename char_T>
std::vector<std::pair<num_T, std::basic_string<char_T>>> get_goodCases_hex()
{
	std::vector<std::pair<num_T, std::basic_string<char_T>>> out;
	constexpr num_T max = std::numeric_limits<num_T>::max();
	std::array<char, 16> buff;
	char* first = buff.data();
	char* last = first + buff.size();

	for(uint64_t num : hex_numbers)
	{
		if(num <= max)
		{
			std::to_chars_result res = std::to_chars(first, last, num, 16);
			core::toUpperCase(std::span<char8_t>{reinterpret_cast<char8_t*>(first), static_cast<uintptr_t>(res.ptr - first)});
			out.push_back({static_cast<num_T>(num), str2_Tstring<char_T>(std::string_view{first, static_cast<uintptr_t>(res.ptr - first)})});
		}
		else break;
	}
	std::to_chars_result res = std::to_chars(first, last, max, 16);
	core::toUpperCase(std::span<char8_t>{reinterpret_cast<char8_t*>(first), static_cast<uintptr_t>(res.ptr - first)});
	out.push_back({max, str2_Tstring<char_T>(std::string_view{first, static_cast<uintptr_t>(res.ptr - first)})});

	return out;
}


template <typename num_T, typename char_T>
std::vector<std::basic_string<char_T>> get_badCases_hex()
{
	std::vector<std::basic_string<char_T>> out;

	for(std::string const& t_str : hexBadCases_s)
	{
		out.emplace_back(str2_Tstring<char_T>(t_str));
	}

	out.emplace_back(str2_Tstring<char_T>(one_past_end_hex<num_T>::str));

	return out;
}

//======== ======== ======== Hex test Suit ======== ======== ========


template<typename T>
class charconv_Hex_T : public testing::Test {
protected:
	charconv_Hex_T() {}
};

using hexTypes = ::testing::Types<
	std::pair<uint8_t , char8_t>,
	std::pair<uint16_t, char8_t>,
	std::pair<uint32_t, char8_t>,
	std::pair<uint64_t, char8_t>,
	std::pair<uint8_t , char16_t>,
	std::pair<uint16_t, char16_t>,
	std::pair<uint32_t, char16_t>,
	std::pair<uint64_t, char16_t>,
	std::pair<uint8_t , char32_t>,
	std::pair<uint16_t, char32_t>,
	std::pair<uint32_t, char32_t>,
	std::pair<uint64_t, char32_t>>;

TYPED_TEST_SUITE(charconv_Hex_T, hexTypes);


TYPED_TEST(charconv_Hex_T, from_string_good)
{
	using num_T = typename TypeParam::first_type;
	using char_T = typename TypeParam::second_type;

	std::vector<std::pair<num_T, std::basic_string<char_T>>> const& goodCases = get_goodCases_hex<num_T, char_T>();
	for(std::pair<num_T, std::basic_string<char_T>> const& testCase: goodCases)
	{
		core::from_chars_result<num_T> result = core::from_chars_hex<num_T>(testCase.second);

		ASSERT_TRUE(result.has_value()) << "Case" << testCase.second;
		ASSERT_EQ(result.value(), testCase.first);
	}
}

TYPED_TEST(charconv_Hex_T, from_string_bad)
{
	using num_T = typename TypeParam::first_type;
	using char_T = typename TypeParam::second_type;

	std::vector<std::basic_string<char_T>> const& badCases = get_badCases_hex<num_T, char_T>();
	for(std::basic_string<char_T> const& testCase: badCases)
	{
		core::from_chars_result<num_T> result = core::from_chars_hex<num_T>(testCase);

		ASSERT_FALSE(result.has_value()) << "Case " << testCase;
	}
}


//======== ======== ======== Bin test Cases ======== ======== ========

std::vector<std::string> const binBadCases_s =
{
	"",
	"false",
	"true",
	"0G",
	"0f",
	"120",
	"2",
	"A",
	"a",
	"T",
	"G",
	"g",
	"!",
	" ",
	"-.E-",
	"-",
	"+",
	".",
	"E",
	std::string{"\0", 1},
	"10 1",
	" 101",
	"101 ",
};


template <typename num_T>
struct one_past_end_bin;

template <> struct one_past_end_bin<uint8_t >{ static constexpr std::string_view str = "100000000"; };
template <> struct one_past_end_bin<uint16_t>{ static constexpr std::string_view str = "10000000000000000";};
template <> struct one_past_end_bin<uint32_t>{ static constexpr std::string_view str = "100000000000000000000000000000000";};
template <> struct one_past_end_bin<uint64_t>{ static constexpr std::string_view str = "10000000000000000000000000000000000000000000000000000000000000000";};



template <typename num_T, typename char_T>
std::vector<std::pair<num_T, std::basic_string<char_T>>> get_goodCases_bin()
{
	std::vector<std::pair<num_T, std::basic_string<char_T>>> out;
	constexpr num_T max = std::numeric_limits<num_T>::max();
	std::array<char, 64> buff;
	char* first = buff.data();
	char* last = first + buff.size();

	for(uint64_t num : hex_numbers)
	{
		if(num <= max)
		{
			std::to_chars_result res = std::to_chars(first, last, num, 2);
			out.push_back({static_cast<num_T>(num), str2_Tstring<char_T>(std::string_view{first, static_cast<uintptr_t>(res.ptr - first)})});
		}
		else break;
	}
	std::to_chars_result res = std::to_chars(first, last, max, 2);
	out.push_back({max, str2_Tstring<char_T>(std::string_view{first, static_cast<uintptr_t>(res.ptr - first)})});
	return out;
}


template <typename num_T, typename char_T>
std::vector<std::basic_string<char_T>> get_badCases_bin()
{
	std::vector<std::basic_string<char_T>> out;

	for(std::string const& t_str : binBadCases_s)
	{
		out.emplace_back(str2_Tstring<char_T>(t_str));
	}

	out.emplace_back(str2_Tstring<char_T>(one_past_end_bin<num_T>::str));

	return out;
}

//======== ======== ======== Bin test Suit ======== ======== ========


template<typename T>
class charconv_Bin_T : public testing::Test {
protected:
	charconv_Bin_T() {}
};

using binTypes = ::testing::Types<
	std::pair<uint8_t , char8_t>,
	std::pair<uint16_t, char8_t>,
	std::pair<uint32_t, char8_t>,
	std::pair<uint64_t, char8_t>,
	std::pair<uint8_t , char16_t>,
	std::pair<uint16_t, char16_t>,
	std::pair<uint32_t, char16_t>,
	std::pair<uint64_t, char16_t>,
	std::pair<uint8_t , char32_t>,
	std::pair<uint16_t, char32_t>,
	std::pair<uint32_t, char32_t>,
	std::pair<uint64_t, char32_t>>;

TYPED_TEST_SUITE(charconv_Bin_T, binTypes);


TYPED_TEST(charconv_Bin_T, from_string_good)
{
	using num_T = typename TypeParam::first_type;
	using char_T = typename TypeParam::second_type;

	std::vector<std::pair<num_T, std::basic_string<char_T>>> const& goodCases = get_goodCases_bin<num_T, char_T>();
	for(std::pair<num_T, std::basic_string<char_T>> const& testCase: goodCases)
	{
		core::from_chars_result<num_T> result = core::from_chars_bin<num_T>(testCase.second);

		ASSERT_TRUE(result.has_value()) << "Case" << testCase.second;
		ASSERT_EQ(result.value(), testCase.first);
	}
}

TYPED_TEST(charconv_Bin_T, from_string_bad)
{
	using num_T = typename TypeParam::first_type;
	using char_T = typename TypeParam::second_type;

	std::vector<std::basic_string<char_T>> const& badCases = get_badCases_bin<num_T, char_T>();
	for(std::basic_string<char_T> const& testCase: badCases)
	{
		core::from_chars_result<num_T> result = core::from_chars_bin<num_T>(testCase);

		ASSERT_FALSE(result.has_value()) << "Case " << testCase;
	}
}

//======== ======== ======== Simple is Number test Suit ======== ======== ========

template<typename T>
class charconv_char_T : public testing::Test {
protected:
	charconv_char_T() {}
};

using charTypes = ::testing::Types<
		char8_t,
		char16_t,
		char32_t
	>;

TYPED_TEST_SUITE(charconv_char_T, charTypes);

TYPED_TEST(charconv_char_T, is_uint)
{
	using char_T = TypeParam;

	//good
	{
		std::vector<std::pair<uint64_t, std::basic_string<char_T>>> const& goodCases = get_goodCases<uint64_t, char_T>();
		for(std::pair<uint64_t, std::basic_string<char_T>> const& testCase: goodCases)
		{
			ASSERT_TRUE(core::is_uint(testCase.second)) << "Case" << testCase.second;
		}
	}

	//bad
	{
		for(std::string tcase: badCases_s)
		{
			ASSERT_FALSE(core::is_uint(str2_Tstring<char_T>(tcase))) << "Case " << tcase;
		}

		ASSERT_FALSE(core::is_uint(str2_Tstring<char_T>("-1"))) << "Case " << "-1";
	}
}

TYPED_TEST(charconv_char_T, is_int)
{
	using char_T = TypeParam;

	//good
	{
		std::vector<std::pair<int64_t, std::basic_string<char_T>>> const& goodCases = get_goodCases<int64_t, char_T>();
		for(std::pair<int64_t, std::basic_string<char_T>> const& testCase: goodCases)
		{
			ASSERT_TRUE(core::is_int(testCase.second)) << "Case " << testCase.second;
		}
	}

	//bad
	{
		for(std::string tcase: badCases_s)
		{
			ASSERT_FALSE(core::is_int(str2_Tstring<char_T>(tcase))) << "Case " << tcase;
		}
	}
}


TYPED_TEST(charconv_char_T, is_hex)
{
	using char_T = TypeParam;

	//good
	{
		std::vector<std::pair<uint64_t, std::basic_string<char_T>>> const& goodCases = get_goodCases_hex<uint64_t, char_T>();
		for(std::pair<uint64_t, std::basic_string<char_T>> const& testCase: goodCases)
		{
			ASSERT_TRUE(core::is_hex(testCase.second)) << "Case " << testCase.second;
		}
	}

	//bad
	{
		for(std::string tcase: hexBadCases_s)
		{
			ASSERT_FALSE(core::is_hex(str2_Tstring<char_T>(tcase))) << "Case " << tcase;
		}
	}
}



TYPED_TEST(charconv_char_T, is_bin)
{
	using char_T = TypeParam;

	//good
	{
		std::vector<std::pair<uint64_t, std::basic_string<char_T>>> const& goodCases = get_goodCases_bin<uint64_t, char_T>();
		for(std::pair<uint64_t, std::basic_string<char_T>> const& testCase: goodCases)
		{
			ASSERT_TRUE(core::is_bin(testCase.second)) << "Case " << testCase.second;
		}
	}

	//bad
	{
		for(std::string tcase: binBadCases_s)
		{
			ASSERT_FALSE(core::is_bin(str2_Tstring<char_T>(tcase))) << "Case " << tcase;
		}
	}
}


} //namespace numeric

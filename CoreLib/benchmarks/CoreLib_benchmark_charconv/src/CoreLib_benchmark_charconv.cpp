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

#include <array>
#include <string>
#include <vector>
#include <charconv>
#include <limits>
#include <type_traits>
#include <bit>

#include <benchmark/benchmark.h>

#include <CoreLib/string/core_string_numeric.hpp>
#include <CoreLib/string/core_string_misc.hpp>
#include <CoreLib/string/core_fp_charconv.hpp>
#include <CoreLib/string/core_fp_to_chars_round.hpp>
#include <CoreLib/core_type.hpp>

using ::core::literals::operator "" _ui64;
using ::core::literals::operator "" _ui32;

//======== ======== ======== ======== Auxiliary Test case generator ======== ======== ======== ========

template <typename char_T>
std::basic_string<char_T> str2_Tstring(std::string_view const p_str)
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

//======== ======== ======== Decimal Int test Cases ======== ======== ========
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
	9223372036854775808ull,
	9999999999999999999ull,
	10000000000000000000ull,
	18446744073709551615ull
};

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
	0x42,
	0xFF,
	0x100,
	0x123,
	0xFFF,
	0x1000,
	0xFFFF,
	0x10000,
	0x12345,
	0xFFFFFFFF,
	0x100000000,
	0x123456789ABCDEF0,
	0xFEDCBA9876543210,
	0xFFFFFFFFFFFFFFFF,
};

std::vector<std::string> const badCases_s =
{
	"",
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

std::vector<std::string> const hexBadCases_s =
{
	"",
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

template <typename num_T>
struct one_past_end_hex;

template <> struct one_past_end_hex<uint8_t >{ static constexpr std::string_view str = "100"; };
template <> struct one_past_end_hex<uint16_t>{ static constexpr std::string_view str = "10000";};
template <> struct one_past_end_hex<uint32_t>{ static constexpr std::string_view str = "100000000";};
template <> struct one_past_end_hex<uint64_t>{ static constexpr std::string_view str = "10000000000000000";};

template <typename num_T>
struct one_past_end_bin;

template <> struct one_past_end_bin<uint8_t >{ static constexpr std::string_view str = "100000000"; };
template <> struct one_past_end_bin<uint16_t>{ static constexpr std::string_view str = "10000000000000000";};
template <> struct one_past_end_bin<uint32_t>{ static constexpr std::string_view str = "100000000000000000000000000000000";};
template <> struct one_past_end_bin<uint64_t>{ static constexpr std::string_view str = "10000000000000000000000000000000000000000000000000000000000000000";};



template <typename num_T, typename char_T>
std::vector<std::basic_string<char_T>> get_goodStr()
{
	static_assert(!std::is_floating_point_v<num_T>);
	
	std::vector<std::basic_string<char_T>> out;
	constexpr num_T max = std::numeric_limits<num_T>::max();
	std::array<char, 20> buff;
	char* const first = buff.data();
	char* const last = first + buff.size();

	for(uint64_t num : u_numbers)
	{
		if(num <= max)
		{
			std::to_chars_result const res = std::to_chars(first, last, num);
			out.push_back(str2_Tstring<char_T>(std::string_view{first, static_cast<uintptr_t>(res.ptr - first)}));
		}
		else break;
	}

	if constexpr(std::is_signed_v<num_T>)
	{
		constexpr num_T min = std::numeric_limits<num_T>::min();
		for(int64_t num : s_numbers)
		{
			if(num >= min)
			{
				std::to_chars_result const res = std::to_chars(first, last, num);
				out.push_back(str2_Tstring<char_T>(std::string_view{first, static_cast<uintptr_t>(res.ptr - first)}));
			}
			else break;
		}
	}
	return out;
}

template <typename num_T, typename char_T>
std::vector<std::basic_string<char_T>> get_badStr()
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

template <typename num_T>
std::vector<num_T> get_num()
{
	static_assert(!std::is_floating_point_v<num_T>);

	std::vector<num_T> out;
	constexpr num_T max = std::numeric_limits<num_T>::max();

	for(uint64_t const num : u_numbers)
	{
		if(num <= max)
		{
			out.push_back(static_cast<num_T const>(num));
		}
		else break;
	}

	if constexpr(std::is_signed_v<num_T>)
	{
		constexpr num_T min = std::numeric_limits<num_T>::min();
		for(int64_t const num : s_numbers)
		{
			if(num >= min)
			{
				out.push_back(static_cast<num_T const>(num));
			}
			else break;
		}
	}
	return out;
}

template <typename num_T, typename char_T>
std::vector<std::basic_string<char_T>> get_goodStr_hex()
{
	std::vector<std::basic_string<char_T>> out;
	constexpr num_T max = std::numeric_limits<num_T>::max();
	std::array<char, 16> buff;
	char* const first = buff.data();
	char* const last = first + buff.size();

	for(uint64_t const num : hex_numbers)
	{
		if(num <= max)
		{
			std::to_chars_result const res = std::to_chars(first, last, num, 16);
			core::toUpperCase(std::span<char8_t>{reinterpret_cast<char8_t*>(first), static_cast<uintptr_t>(res.ptr - first)});
			out.push_back(str2_Tstring<char_T>(std::string_view{first, static_cast<uintptr_t>(res.ptr - first)}));
		}
		else break;
	}

	return out;
}


template <typename num_T, typename char_T>
std::vector<std::basic_string<char_T>> get_goodStr_bin()
{
	std::vector<std::basic_string<char_T>> out;
	constexpr num_T max = std::numeric_limits<num_T>::max();
	std::array<char, 64> buff;
	char* const first = buff.data();
	char* const last = first + buff.size();

	for(uint64_t const num : hex_numbers)
	{
		if(num <= max)
		{
			std::to_chars_result const res = std::to_chars(first, last, num, 2);
			out.push_back(str2_Tstring<char_T>(std::string_view{first, static_cast<uintptr_t>(res.ptr - first)}));
		}
		else break;
	}

	return out;
}

template <typename num_T, typename char_T>
std::vector<std::basic_string<char_T>> get_badStr_hex()
{
	std::vector<std::basic_string<char_T>> out;

	for(std::string const& t_str : hexBadCases_s)
	{
		out.emplace_back(str2_Tstring<char_T>(t_str));
	}

	out.emplace_back(str2_Tstring<char_T>(one_past_end_hex<num_T>::str));

	return out;
}

template <typename num_T, typename char_T>
std::vector<std::basic_string<char_T>> get_badStr_bin()
{
	std::vector<std::basic_string<char_T>> out;

	for(std::string const& t_str : binBadCases_s)
	{
		out.emplace_back(str2_Tstring<char_T>(t_str));
	}

	out.emplace_back(str2_Tstring<char_T>(one_past_end_bin<num_T>::str));

	return out;
}

template <typename num_T>
std::vector<num_T> get_num_hex()
{
	std::vector<num_T> out;
	constexpr num_T max = std::numeric_limits<num_T>::max();

	for(uint64_t const num : hex_numbers)
	{
		if(num <= max)
		{
			out.push_back(static_cast<num_T const>(num));
		}
		else break;
	}

	return out;
}

//======== ======== ======== ======== Test templating ======== ======== ======== ========

template<typename num_T>
static void std_from_chars_good(benchmark::State& state)
{
	std::vector<std::u8string> const& testList = get_goodStr<num_T, char8_t>();
	uintptr_t index = 0;

	for (auto _ : state)
	{
		std::u8string const& testCase = testList[index];
		char const* const first = reinterpret_cast<char const*>(testCase.data());
		char const* const last = first + testCase.size();
		num_T result;
		std::from_chars_result const res = std::from_chars(first, last, result);

		bool const ok = (res.ec == std::error_code{} && res.ptr == last);

		benchmark::DoNotOptimize(result);
		benchmark::DoNotOptimize(ok);

		if(++index >= testList.size()) index = 0;
	}
}

template<typename num_T>
static void core_from_chars_good(benchmark::State& state)
{
	std::vector<std::u8string> const& testList = get_goodStr<num_T, char8_t>();
	uintptr_t index = 0;

	for (auto _ : state)
	{
		std::u8string const& testCase = testList[index];
		core::from_chars_result<num_T> const result = core::from_chars<num_T>(testCase);

		bool const ok = result.has_value();

		benchmark::DoNotOptimize(result.value());
		benchmark::DoNotOptimize(ok);

		if(++index >= testList.size()) index = 0;
	}
}


template<typename num_T>
static void std_from_chars_bad(benchmark::State& state)
{
	std::vector<std::u8string> const& testList = get_badStr<num_T, char8_t>();
	uintptr_t index = 0;

	for (auto _ : state)
	{
		std::u8string const& testCase = testList[index];
		char const* const first = reinterpret_cast<char const*>(testCase.data());
		char const* const last = first + testCase.size();
		num_T result;
		std::from_chars_result const res = std::from_chars(first, last, result);

		bool const ok = (res.ec == std::error_code{} && res.ptr == last);

		benchmark::DoNotOptimize(ok);

		if(++index >= testList.size()) index = 0;
	}
}

template<typename num_T>
static void core_from_chars_bad(benchmark::State& state)
{
	std::vector<std::u8string> const& testList = get_badStr<num_T, char8_t>();
	uintptr_t index = 0;

	for (auto _ : state)
	{
		std::u8string const& testCase = testList[index];
		core::from_chars_result<num_T> const result = core::from_chars<num_T>(testCase);

		bool const ok = result.has_value();

		benchmark::DoNotOptimize(ok);

		if(++index >= testList.size()) index = 0;
	}
}

template<typename num_T>
static void std_from_chars_hex_good(benchmark::State& state)
{
	std::vector<std::u8string> const& testList = get_goodStr_hex<num_T, char8_t>();
	uintptr_t index = 0;

	for (auto _ : state)
	{
		std::u8string const& testCase = testList[index];
		char const* const first = reinterpret_cast<char const*>(testCase.data());
		char const* const last = first + testCase.size();
		num_T result;
		std::from_chars_result const res = std::from_chars(first, last, result, 16);

		bool const ok = (res.ec == std::error_code{} && res.ptr == last);

		benchmark::DoNotOptimize(result);
		benchmark::DoNotOptimize(ok);

		if(++index >= testList.size()) index = 0;
	}
}

template<typename num_T>
static void core_from_chars_hex_good(benchmark::State& state)
{
	std::vector<std::u8string> const& testList = get_goodStr_hex<num_T, char8_t>();
	uintptr_t index = 0;

	for (auto _ : state)
	{
		std::u8string const& testCase = testList[index];
		core::from_chars_result<num_T> const result = core::from_chars_hex<num_T>(testCase);

		bool const ok = result.has_value();

		benchmark::DoNotOptimize(result.value());
		benchmark::DoNotOptimize(ok);

		if(++index >= testList.size()) index = 0;
	}
}


template<typename num_T>
static void std_from_chars_hex_bad(benchmark::State& state)
{
	std::vector<std::u8string> const& testList = get_badStr_hex<num_T, char8_t>();
	uintptr_t index = 0;

	for (auto _ : state)
	{
		std::u8string const& testCase = testList[index];
		char const* const first = reinterpret_cast<char const*>(testCase.data());
		char const* const last = first + testCase.size();
		num_T result;
		std::from_chars_result const res = std::from_chars(first, last, result, 16);

		bool const ok = (res.ec == std::error_code{} && res.ptr == last);

		benchmark::DoNotOptimize(ok);

		if(++index >= testList.size()) index = 0;
	}
}

template<typename num_T>
static void core_from_chars_hex_bad(benchmark::State& state)
{
	std::vector<std::u8string> const& testList = get_badStr_hex<num_T, char8_t>();
	uintptr_t index = 0;

	for (auto _ : state)
	{
		std::u8string const& testCase = testList[index];
		core::from_chars_result<num_T> const result = core::from_chars_hex<num_T>(testCase);

		bool const ok = result.has_value();

		benchmark::DoNotOptimize(ok);

		if(++index >= testList.size()) index = 0;
	}
}


template<typename num_T>
static void std_from_chars_bin_good(benchmark::State& state)
{
	std::vector<std::u8string> const& testList = get_goodStr_bin<num_T, char8_t>();
	uintptr_t index = 0;

	for (auto _ : state)
	{
		std::u8string const& testCase = testList[index];
		char const* const first = reinterpret_cast<char const*>(testCase.data());
		char const* const last = first + testCase.size();
		num_T result;
		std::from_chars_result const res = std::from_chars(first, last, result, 2);

		bool const ok = (res.ec == std::error_code{} && res.ptr == last);

		benchmark::DoNotOptimize(result);
		benchmark::DoNotOptimize(ok);

		if(++index >= testList.size()) index = 0;
	}
}

template<typename num_T>
static void core_from_chars_bin_good(benchmark::State& state)
{
	std::vector<std::u8string> const& testList = get_goodStr_bin<num_T, char8_t>();
	uintptr_t index = 0;

	for (auto _ : state)
	{
		std::u8string const& testCase = testList[index];
		core::from_chars_result<num_T> const result = core::from_chars_bin<num_T>(testCase);

		bool const ok = result.has_value();

		benchmark::DoNotOptimize(result.value());
		benchmark::DoNotOptimize(ok);

		if(++index >= testList.size()) index = 0;
	}
}


template<typename num_T>
static void std_from_chars_bin_bad(benchmark::State& state)
{
	std::vector<std::u8string> const& testList = get_badStr_bin<num_T, char8_t>();
	uintptr_t index = 0;

	for (auto _ : state)
	{
		std::u8string const& testCase = testList[index];
		char const* const first = reinterpret_cast<char const*>(testCase.data());
		char const* const last = first + testCase.size();
		num_T result;
		std::from_chars_result const res = std::from_chars(first, last, result, 2);

		bool const ok = (res.ec == std::error_code{} && res.ptr == last);

		benchmark::DoNotOptimize(ok);

		if(++index >= testList.size()) index = 0;
	}
}

template<typename num_T>
static void core_from_chars_bin_bad(benchmark::State& state)
{
	std::vector<std::u8string> const& testList = get_badStr_bin<num_T, char8_t>();
	uintptr_t index = 0;

	for (auto _ : state)
	{
		std::u8string const& testCase = testList[index];
		core::from_chars_result<num_T> const result = core::from_chars_bin<num_T>(testCase);

		bool const ok = result.has_value();

		benchmark::DoNotOptimize(ok);

		if(++index >= testList.size()) index = 0;
	}
}


template<typename num_T>
static void std_to_chars(benchmark::State& state)
{
	std::vector<num_T> const& testList = get_num<num_T>();
	uintptr_t index = 0;

	constexpr uintptr_t buffSize = core::to_chars_dec_max_size_v<num_T>;
	std::array<char8_t, buffSize> buffer;

	for (auto _ : state)
	{
		num_T const testCase = testList[index];
		char* const first = reinterpret_cast<char*>(buffer.data());
		char* const last = first + buffer.size();
		std::to_chars_result const res = std::to_chars(first, last, testCase);

		std::u8string_view const result {buffer.data(), static_cast<uintptr_t>(res.ptr - first)};

		benchmark::DoNotOptimize(result);
		if(++index >= testList.size()) index = 0;
	}
}


template<typename num_T>
static void core_to_chars(benchmark::State& state)
{
	std::vector<num_T> const& testList = get_num<num_T>();
	uintptr_t index = 0;

	constexpr uintptr_t buffSize = core::to_chars_dec_max_size_v<num_T>;
	std::array<char8_t, buffSize> buffer;

	for (auto _ : state)
	{
		num_T const testCase = testList[index];
		uintptr_t const res_size = core::to_chars(testCase, std::span<char8_t, buffSize>(buffer));

		std::u8string_view const result {buffer.data(), res_size};

		benchmark::DoNotOptimize(result);
		if(++index >= testList.size()) index = 0;
	}
}

template<typename num_T>
static void core_to_chars_size(benchmark::State& state)
{
	std::vector<num_T> const& testList = get_num<num_T>();
	uintptr_t index = 0;

	for (auto _ : state)
	{
		num_T const testCase = testList[index];
		uintptr_t const res_size = core::to_chars_size(testCase);

		benchmark::DoNotOptimize(res_size);
		if (++index >= testList.size()) index = 0;
	}
}



template<typename num_T>
static void std_to_chars_hex(benchmark::State& state)
{
	std::vector<num_T> const& testList = get_num_hex<num_T>();
	uintptr_t index = 0;

	constexpr uintptr_t buffSize = core::to_chars_hex_max_size_v<num_T>;
	std::array<char8_t, buffSize> buffer;

	for (auto _ : state)
	{
		num_T const testCase = testList[index];
		char* const first = reinterpret_cast<char*>(buffer.data());
		char* const last = first + buffer.size();
		std::to_chars_result const res = std::to_chars(first, last, testCase, 16);

		std::u8string_view const result {buffer.data(), static_cast<uintptr_t>(res.ptr - first)};

		benchmark::DoNotOptimize(result);
		if(++index >= testList.size()) index = 0;
	}
}

template<typename num_T>
static void core_to_chars_hex(benchmark::State& state)
{
	std::vector<num_T> const& testList = get_num_hex<num_T>();
	uintptr_t index = 0;

	constexpr uintptr_t buffSize = core::to_chars_hex_max_size_v<num_T>;
	std::array<char8_t, buffSize> buffer;

	for (auto _ : state)
	{
		num_T const testCase = testList[index];
		uintptr_t const res_size = core::to_chars_hex(testCase, std::span<char8_t, buffSize>(buffer));

		std::u8string_view const result {buffer.data(), res_size};

		benchmark::DoNotOptimize(result);
		if(++index >= testList.size()) index = 0;
	}
}

template<typename num_T>
static void core_to_chars_hex_fix(benchmark::State& state)
{
	std::vector<num_T> const& testList = get_num_hex<num_T>();
	uintptr_t index = 0;

	constexpr uintptr_t buffSize = core::to_chars_hex_max_size_v<num_T>;
	std::array<char8_t, buffSize> buffer;

	for (auto _ : state)
	{
		num_T const testCase = testList[index];
		core::to_chars_hex_fix(testCase, std::span<char8_t, buffSize>(buffer));

		std::u8string_view const result {buffer.data(), buffSize};

		benchmark::DoNotOptimize(result);
		if(++index >= testList.size()) index = 0;
	}
}

template<typename num_T>
static void core_to_chars_hex_size(benchmark::State& state)
{
	std::vector<num_T> const& testList = get_num_hex<num_T>();
	uintptr_t index = 0;

	for (auto _ : state)
	{
		num_T const testCase = testList[index];
		uintptr_t const res_size = core::to_chars_hex_size(testCase);

		benchmark::DoNotOptimize(res_size);
		if (++index >= testList.size()) index = 0;
	}
}





template<typename num_T>
static void std_to_chars_bin(benchmark::State& state)
{
	std::vector<num_T> const& testList = get_num_hex<num_T>();
	uintptr_t index = 0;

	constexpr uintptr_t buffSize = core::to_chars_bin_max_size_v<num_T>;
	std::array<char8_t, buffSize> buffer;

	for (auto _ : state)
	{
		num_T const testCase = testList[index];
		char* const first = reinterpret_cast<char*>(buffer.data());
		char* const last = first + buffer.size();
		std::to_chars_result const res = std::to_chars(first, last, testCase, 2);

		std::u8string_view const result {buffer.data(), static_cast<uintptr_t>(res.ptr - first)};

		benchmark::DoNotOptimize(result);
		if(++index >= testList.size()) index = 0;
	}
}


template<typename num_T>
static void core_to_chars_bin(benchmark::State& state)
{
	std::vector<num_T> const& testList = get_num_hex<num_T>();
	uintptr_t index = 0;

	constexpr uintptr_t buffSize = core::to_chars_bin_max_size_v<num_T>;
	std::array<char8_t, buffSize> buffer;

	for (auto _ : state)
	{
		num_T const testCase = testList[index];
		uintptr_t const res_size = core::to_chars_bin(testCase, std::span<char8_t, buffSize>(buffer));

		std::u8string_view const result {buffer.data(), res_size};

		benchmark::DoNotOptimize(result);
		if(++index >= testList.size()) index = 0;
	}
}

template<typename num_T>
static void core_to_chars_bin_fix(benchmark::State& state)
{
	std::vector<num_T> const& testList = get_num_hex<num_T>();
	uintptr_t index = 0;

	constexpr uintptr_t buffSize = core::to_chars_bin_max_size_v<num_T>;
	std::array<char8_t, buffSize> buffer;

	for (auto _ : state)
	{
		num_T const testCase = testList[index];
		core::to_chars_bin_fix(testCase, std::span<char8_t, buffSize>(buffer));

		std::u8string_view const result {buffer.data(), buffSize};

		benchmark::DoNotOptimize(result);
		if(++index >= testList.size()) index = 0;
	}
}

template<typename num_T>
static void core_to_chars_bin_size(benchmark::State& state)
{
	std::vector<num_T> const& testList = get_num_hex<num_T>();
	uintptr_t index = 0;

	for (auto _ : state)
	{
		num_T const testCase = testList[index];
		uintptr_t const res_size = core::to_chars_bin_size(testCase);

		benchmark::DoNotOptimize(res_size);
		if (++index >= testList.size()) index = 0;
	}
}

//======== ======== ======== ======== Benchmark Instantiation ======== ======== ======== ========

BENCHMARK_TEMPLATE( std_from_chars_good, uint8_t );
BENCHMARK_TEMPLATE(core_from_chars_good, uint8_t );
BENCHMARK_TEMPLATE( std_from_chars_good, uint16_t);
BENCHMARK_TEMPLATE(core_from_chars_good, uint16_t);
BENCHMARK_TEMPLATE( std_from_chars_good, uint32_t);
BENCHMARK_TEMPLATE(core_from_chars_good, uint32_t);
BENCHMARK_TEMPLATE( std_from_chars_good, uint64_t);
BENCHMARK_TEMPLATE(core_from_chars_good, uint64_t);
BENCHMARK_TEMPLATE( std_from_chars_good, int8_t  );
BENCHMARK_TEMPLATE(core_from_chars_good, int8_t  );
BENCHMARK_TEMPLATE( std_from_chars_good, int16_t );
BENCHMARK_TEMPLATE(core_from_chars_good, int16_t );
BENCHMARK_TEMPLATE( std_from_chars_good, int32_t );
BENCHMARK_TEMPLATE(core_from_chars_good, int32_t );
BENCHMARK_TEMPLATE( std_from_chars_good, int64_t );
BENCHMARK_TEMPLATE(core_from_chars_good, int64_t );

BENCHMARK_TEMPLATE( std_from_chars_bad, uint8_t );
BENCHMARK_TEMPLATE(core_from_chars_bad, uint8_t );
BENCHMARK_TEMPLATE( std_from_chars_bad, uint16_t);
BENCHMARK_TEMPLATE(core_from_chars_bad, uint16_t);
BENCHMARK_TEMPLATE( std_from_chars_bad, uint32_t);
BENCHMARK_TEMPLATE(core_from_chars_bad, uint32_t);
BENCHMARK_TEMPLATE( std_from_chars_bad, uint64_t);
BENCHMARK_TEMPLATE(core_from_chars_bad, uint64_t);
BENCHMARK_TEMPLATE( std_from_chars_bad, int8_t  );
BENCHMARK_TEMPLATE(core_from_chars_bad, int8_t  );
BENCHMARK_TEMPLATE( std_from_chars_bad, int16_t );
BENCHMARK_TEMPLATE(core_from_chars_bad, int16_t );
BENCHMARK_TEMPLATE( std_from_chars_bad, int32_t );
BENCHMARK_TEMPLATE(core_from_chars_bad, int32_t );
BENCHMARK_TEMPLATE( std_from_chars_bad, int64_t );
BENCHMARK_TEMPLATE(core_from_chars_bad, int64_t );

BENCHMARK_TEMPLATE( std_from_chars_hex_good, uint8_t );
BENCHMARK_TEMPLATE(core_from_chars_hex_good, uint8_t );
BENCHMARK_TEMPLATE( std_from_chars_hex_good, uint16_t);
BENCHMARK_TEMPLATE(core_from_chars_hex_good, uint16_t);
BENCHMARK_TEMPLATE( std_from_chars_hex_good, uint32_t);
BENCHMARK_TEMPLATE(core_from_chars_hex_good, uint32_t);
BENCHMARK_TEMPLATE( std_from_chars_hex_good, uint64_t);
BENCHMARK_TEMPLATE(core_from_chars_hex_good, uint64_t);

BENCHMARK_TEMPLATE( std_from_chars_hex_bad, uint8_t );
BENCHMARK_TEMPLATE(core_from_chars_hex_bad, uint8_t );
BENCHMARK_TEMPLATE( std_from_chars_hex_bad, uint16_t);
BENCHMARK_TEMPLATE(core_from_chars_hex_bad, uint16_t);
BENCHMARK_TEMPLATE( std_from_chars_hex_bad, uint32_t);
BENCHMARK_TEMPLATE(core_from_chars_hex_bad, uint32_t);
BENCHMARK_TEMPLATE( std_from_chars_hex_bad, uint64_t);
BENCHMARK_TEMPLATE(core_from_chars_hex_bad, uint64_t);

BENCHMARK_TEMPLATE( std_from_chars_bin_good, uint8_t );
BENCHMARK_TEMPLATE(core_from_chars_bin_good, uint8_t );
BENCHMARK_TEMPLATE( std_from_chars_bin_good, uint16_t);
BENCHMARK_TEMPLATE(core_from_chars_bin_good, uint16_t);
BENCHMARK_TEMPLATE( std_from_chars_bin_good, uint32_t);
BENCHMARK_TEMPLATE(core_from_chars_bin_good, uint32_t);
BENCHMARK_TEMPLATE( std_from_chars_bin_good, uint64_t);
BENCHMARK_TEMPLATE(core_from_chars_bin_good, uint64_t);

BENCHMARK_TEMPLATE( std_from_chars_bin_bad, uint8_t );
BENCHMARK_TEMPLATE(core_from_chars_bin_bad, uint8_t );
BENCHMARK_TEMPLATE( std_from_chars_bin_bad, uint16_t);
BENCHMARK_TEMPLATE(core_from_chars_bin_bad, uint16_t);
BENCHMARK_TEMPLATE( std_from_chars_bin_bad, uint32_t);
BENCHMARK_TEMPLATE(core_from_chars_bin_bad, uint32_t);
BENCHMARK_TEMPLATE( std_from_chars_bin_bad, uint64_t);
BENCHMARK_TEMPLATE(core_from_chars_bin_bad, uint64_t);

BENCHMARK_TEMPLATE( std_to_chars, uint8_t );
BENCHMARK_TEMPLATE(core_to_chars, uint8_t );
BENCHMARK_TEMPLATE( std_to_chars, uint16_t);
BENCHMARK_TEMPLATE(core_to_chars, uint16_t);
BENCHMARK_TEMPLATE( std_to_chars, uint32_t);
BENCHMARK_TEMPLATE(core_to_chars, uint32_t);
BENCHMARK_TEMPLATE( std_to_chars, uint64_t);
BENCHMARK_TEMPLATE(core_to_chars, uint64_t);
BENCHMARK_TEMPLATE( std_to_chars, int8_t  );
BENCHMARK_TEMPLATE(core_to_chars, int8_t  );
BENCHMARK_TEMPLATE( std_to_chars, int16_t );
BENCHMARK_TEMPLATE(core_to_chars, int16_t );
BENCHMARK_TEMPLATE( std_to_chars, int32_t );
BENCHMARK_TEMPLATE(core_to_chars, int32_t );
BENCHMARK_TEMPLATE( std_to_chars, int64_t );
BENCHMARK_TEMPLATE(core_to_chars, int64_t );

BENCHMARK_TEMPLATE(core_to_chars_size, uint8_t);
BENCHMARK_TEMPLATE(core_to_chars_size, uint16_t);
BENCHMARK_TEMPLATE(core_to_chars_size, uint32_t);
BENCHMARK_TEMPLATE(core_to_chars_size, uint64_t);

BENCHMARK_TEMPLATE( std_to_chars_hex,     uint8_t );
BENCHMARK_TEMPLATE(core_to_chars_hex,     uint8_t );
BENCHMARK_TEMPLATE(core_to_chars_hex_fix, uint8_t );
BENCHMARK_TEMPLATE( std_to_chars_hex,     uint16_t);
BENCHMARK_TEMPLATE(core_to_chars_hex,     uint16_t);
BENCHMARK_TEMPLATE(core_to_chars_hex_fix, uint16_t);
BENCHMARK_TEMPLATE( std_to_chars_hex,     uint32_t);
BENCHMARK_TEMPLATE(core_to_chars_hex,     uint32_t);
BENCHMARK_TEMPLATE(core_to_chars_hex_fix, uint32_t);
BENCHMARK_TEMPLATE( std_to_chars_hex,     uint64_t);
BENCHMARK_TEMPLATE(core_to_chars_hex,     uint64_t);
BENCHMARK_TEMPLATE(core_to_chars_hex_fix, uint64_t);

BENCHMARK_TEMPLATE(core_to_chars_hex_size, uint8_t);
BENCHMARK_TEMPLATE(core_to_chars_hex_size, uint16_t);
BENCHMARK_TEMPLATE(core_to_chars_hex_size, uint32_t);
BENCHMARK_TEMPLATE(core_to_chars_hex_size, uint64_t);

BENCHMARK_TEMPLATE( std_to_chars_bin,     uint8_t );
BENCHMARK_TEMPLATE(core_to_chars_bin,     uint8_t );
BENCHMARK_TEMPLATE(core_to_chars_bin_fix, uint8_t );
BENCHMARK_TEMPLATE( std_to_chars_bin,     uint16_t);
BENCHMARK_TEMPLATE(core_to_chars_bin,     uint16_t);
BENCHMARK_TEMPLATE(core_to_chars_bin_fix, uint16_t);
BENCHMARK_TEMPLATE( std_to_chars_bin,     uint32_t);
BENCHMARK_TEMPLATE(core_to_chars_bin,     uint32_t);
BENCHMARK_TEMPLATE(core_to_chars_bin_fix, uint32_t);
BENCHMARK_TEMPLATE( std_to_chars_bin,     uint64_t);
BENCHMARK_TEMPLATE(core_to_chars_bin,     uint64_t);
BENCHMARK_TEMPLATE(core_to_chars_bin_fix, uint64_t);

BENCHMARK_TEMPLATE(core_to_chars_bin_size, uint8_t);
BENCHMARK_TEMPLATE(core_to_chars_bin_size, uint16_t);
BENCHMARK_TEMPLATE(core_to_chars_bin_size, uint32_t);
BENCHMARK_TEMPLATE(core_to_chars_bin_size, uint64_t);

template<typename T>
struct fp_cases;

template<>
struct fp_cases<float32_t>
{
	static constexpr uint16_t buff_size = 256;

	static constexpr uint16_t sig_digits = 111;
	inline static constexpr float32_t sci_case()
	{
		return std::bit_cast<float32_t>(0xFFFFFF_ui32);
	}

	static constexpr uint16_t precision_digits = 2;
	inline static constexpr float32_t fix_case()
	{
		return 1.125f;
	}
};

template<>
struct fp_cases<float64_t>
{
	static constexpr uint16_t buff_size = 2048;

	static constexpr uint16_t sig_digits = 766;
	inline static constexpr float64_t sci_case()
	{
		return std::bit_cast<float64_t>(0x001FFFFFFFFFFFFF_ui64);
	}

	static constexpr uint16_t precision_digits = 2;
	inline static constexpr float64_t fix_case()
	{
		return 1.125;
	}
};


template<typename fp_t>
static inline void std_to_chars_sci(benchmark::State& state)
{
	using fp_case_t = fp_cases<fp_t>;

	fp_t const test_case = fp_case_t::sci_case();
	uint16_t const sig_digits = fp_case_t::sig_digits;

	std::array<char, fp_case_t::buff_size> buff;
	for (auto _ : state)
	{
		std::to_chars(buff.data(), buff.data() + buff.size(), test_case, std::chars_format::scientific, sig_digits);
		benchmark::DoNotOptimize(buff);
	}
}


template<typename fp_t>
static inline void std_to_chars_fix(benchmark::State& state)
{
	using fp_case_t = fp_cases<fp_t>;

	fp_t const test_case = fp_case_t::fix_case();
	uint16_t const precision_digits = fp_case_t::precision_digits;

	std::array<char, fp_case_t::buff_size> buff;
	for (auto _ : state)
	{
		std::to_chars(buff.data(), buff.data() + buff.size(), test_case, std::chars_format::fixed, precision_digits);
		benchmark::DoNotOptimize(buff);
	}
}

template<typename fp_t>
static inline void std_to_chars_short(benchmark::State& state)
{
	using fp_case_t = fp_cases<fp_t>;
	fp_t const test_case = fp_case_t::sci_case();

	std::array<char, fp_case_t::buff_size> buff;
	for (auto _ : state)
	{
		std::to_chars(buff.data(), buff.data() + buff.size(), test_case);
		benchmark::DoNotOptimize(buff);
	}
}



template<typename fp_t, typename char_t>
static inline void core_to_chars2(benchmark::State& state)
{
	using fp_case_t = fp_cases<fp_t>;
	fp_t const test_case = fp_case_t::sci_case();

	std::array<char_t, fp_case_t::buff_size> buff;

	constexpr uintptr_t expected_size = core::to_chars_dec_max_size_v<fp_t>;

	for (auto _ : state)
	{
		uintptr_t size = core::to_chars(test_case, std::span<char_t, expected_size>{buff.data(), expected_size});
		benchmark::DoNotOptimize(buff);
		benchmark::DoNotOptimize(size);
	}
}


template<typename fp_t>
static inline void core_to_chars_sci(benchmark::State& state)
{
	using fp_case_t = fp_cases<fp_t>;

	fp_t const test_case = fp_case_t::sci_case();
	uint16_t const sig_digits = fp_case_t::sig_digits;

	std::array<char8_t, fp_case_t::buff_size> buff;

	for (auto _ : state)
	{
		core::fp_to_chars_sci_context<fp_t> context;
		core::fp_to_chars_sci_result res =
			core::to_chars_sci_size(test_case, context, sig_digits, core::fp_round::nearest);

		if(res.classification == core::fp_classify::finite)
		{
			char8_t* pivot = buff.data();
			if(res.is_negative)
			{
				*(pivot++) = '-';
			}
			char8_t* const unit_pos = pivot++;
			char8_t* decimal_pos;
			if(res.size.mantissa_decimal_size)
			{
				*(pivot++) = '.';
				decimal_pos = pivot;
				pivot += res.size.mantissa_decimal_size;
			}
			else
			{
				decimal_pos = pivot;
			}
			*(pivot++) = 'E';
			char8_t* exp_pos = pivot;

			if(res.size.is_exp_negative)
			{
				*(exp_pos++) = '-';
			}

			core::to_chars_sci_mantissa_unsafe(context, unit_pos, decimal_pos);
			core::to_chars_sci_exp_unsafe(context, exp_pos);
		}
		benchmark::DoNotOptimize(buff);
	}
}

template<typename fp_t>
static inline void core_to_chars_fix(benchmark::State& state)
{
	using fp_case_t = fp_cases<fp_t>;

	fp_t const test_case = fp_case_t::fix_case();
	uint16_t const precision_digits = fp_case_t::precision_digits;

	//std::array<char, fp_case_t::buff_size> buff;

	for (auto _ : state)
	{
		core::fp_to_chars_fix_context<fp_t> context;
		core::fp_to_chars_fix_result res =
			core::to_chars_fix_size(test_case, context, precision_digits, core::fp_round::nearest);
		benchmark::DoNotOptimize(res);
	}
}

template<typename fp_t, typename char_t>
static inline void core_to_chars_shortest(benchmark::State& state)
{
	using fp_case_t = fp_cases<fp_t>;

	fp_t const test_case = fp_case_t::sci_case();

	std::array<char_t, fp_case_t::buff_size> buff;

	for (auto _ : state)
	{
		core::fp_to_chars_shortest_context<fp_t> context;
		core::fp_base_classify res =
			core::to_chars_shortest_classify(test_case, context);

		if(res.classification == core::fp_classify::finite)
		{
			core::fp_to_chars_sci_size sci_size = core::to_chars_shortest_sci_size(context);
			core::fp_to_chars_fix_size fix_size = core::to_chars_shortest_fix_size(context);

			uint8_t const sci_total_size = static_cast<uint8_t>(sci_size.exponent_size + sci_size.mantissa_decimal_size + sci_size.is_exp_negative + 3);
			uint8_t const fix_total_size = static_cast<uint8_t>(fix_size.unit_size + fix_size.decimal_size + 1);

			char_t* pivot = buff.data();
			if(res.is_negative)
			{
				*(pivot++) = char_t{'-'};
			}

			if(sci_total_size < fix_total_size)
			{
				char_t* const unit_pos = pivot++;
				*(pivot++) = char_t{'.'};
				char_t* const decimal_pos = pivot;
				pivot += sci_size.mantissa_decimal_size;

				*(pivot++) = char_t{'E'};
				if(sci_size.is_exp_negative)
				{
					*(pivot++) = char_t{'-'};
				}
				char_t* const exp_pos = pivot;

				core::to_chars_shortest_sci_unsafe(context, unit_pos, decimal_pos);
				core::to_chars_shortest_sci_exp_unsafe(context, exp_pos);
			}
			else
			{
				char_t* const unit_pos = pivot;
				pivot += fix_size.unit_size;
				*(pivot++) = char_t{'.'};
				char_t* const decimal_pos = pivot;
				core::to_chars_shortest_fix_unsafe(context, unit_pos, decimal_pos);
			}
		}
		benchmark::DoNotOptimize(buff);
	}
}

template<typename fp_t>
static inline void core_to_chars_shortest_classify(benchmark::State& state)
{
	using fp_case_t = fp_cases<fp_t>;
	fp_t const test_case = fp_case_t::sci_case();

	for (auto _ : state)
	{
		core::fp_to_chars_shortest_context<fp_t> context;
		core::fp_base_classify res =
			core::to_chars_shortest_classify(test_case, context);
		benchmark::DoNotOptimize(res);
	}
}


template<typename fp_t>
static inline void core_to_chars_shortest_size(benchmark::State& state)
{
	using fp_case_t = fp_cases<fp_t>;

	fp_t const test_case = fp_case_t::sci_case();

	core::fp_to_chars_shortest_context<fp_t> context;
	core::fp_base_classify res =
		core::to_chars_shortest_classify(test_case, context);

	for (auto _ : state)
	{
		if(res.classification == core::fp_classify::finite)
		{
			core::fp_to_chars_sci_size sci_size = core::to_chars_shortest_sci_size(context);
			core::fp_to_chars_fix_size fix_size = core::to_chars_shortest_fix_size(context);

			uint8_t const sci_total_size = static_cast<uint8_t>(sci_size.exponent_size + sci_size.mantissa_decimal_size + sci_size.is_exp_negative + 3);
			uint8_t const fix_total_size = static_cast<uint8_t>(fix_size.unit_size + fix_size.decimal_size + 1);
			benchmark::DoNotOptimize(sci_total_size);
			benchmark::DoNotOptimize(fix_total_size);
		}
	}
}

template<typename fp_t, typename char_t>
static inline void core_to_chars_shortest_convert(benchmark::State& state)
{
	using fp_case_t = fp_cases<fp_t>;

	fp_t const test_case = fp_case_t::sci_case();

	std::array<char_t, fp_case_t::buff_size> buff;

	core::fp_to_chars_shortest_context<fp_t> context;
	core::fp_base_classify res =
		core::to_chars_shortest_classify(test_case, context);

	core::fp_to_chars_sci_size sci_size = core::to_chars_shortest_sci_size(context);
	core::fp_to_chars_fix_size fix_size = core::to_chars_shortest_fix_size(context);

	uint8_t const sci_total_size = static_cast<uint8_t>(sci_size.exponent_size + sci_size.mantissa_decimal_size + sci_size.is_exp_negative + 3);
	uint8_t const fix_total_size = static_cast<uint8_t>(fix_size.unit_size + fix_size.decimal_size + 1);


	for (auto _ : state)
	{
		if(res.classification == core::fp_classify::finite)
		{
			char_t* pivot = buff.data();
			if(res.is_negative)
			{
				*(pivot++) = char_t{'-'};
			}

			if(sci_total_size < fix_total_size)
			{
				char_t* const unit_pos = pivot++;
				*(pivot++) = char_t{'.'};
				char_t* const decimal_pos = pivot;
				pivot += sci_size.mantissa_decimal_size;

				*(pivot++) = char_t{'E'};
				if(sci_size.is_exp_negative)
				{
					*(pivot++) = char_t{'-'};
				}
				char_t* const exp_pos = pivot;

				core::to_chars_shortest_sci_unsafe(context, unit_pos, decimal_pos);
				core::to_chars_shortest_sci_exp_unsafe(context, exp_pos);
			}
			else
			{
				char_t* const unit_pos = pivot;
				pivot += fix_size.unit_size;
				*(pivot++) = char_t{'.'};
				char_t* const decimal_pos = pivot;
				core::to_chars_shortest_fix_unsafe(context, unit_pos, decimal_pos);
			}
		}
		benchmark::DoNotOptimize(buff);
	}
}

BENCHMARK_TEMPLATE(std_to_chars_short, float32_t);
BENCHMARK_TEMPLATE(std_to_chars_short, float64_t);

BENCHMARK_TEMPLATE(core_to_chars_shortest, float32_t, char8_t);
BENCHMARK_TEMPLATE(core_to_chars_shortest, float64_t, char8_t);
BENCHMARK_TEMPLATE(core_to_chars_shortest, float32_t, char16_t);
BENCHMARK_TEMPLATE(core_to_chars_shortest, float64_t, char16_t);
BENCHMARK_TEMPLATE(core_to_chars_shortest, float32_t, char32_t);
BENCHMARK_TEMPLATE(core_to_chars_shortest, float64_t, char32_t);

BENCHMARK_TEMPLATE(core_to_chars_shortest_classify, float32_t);
BENCHMARK_TEMPLATE(core_to_chars_shortest_classify, float64_t);

BENCHMARK_TEMPLATE(core_to_chars_shortest_size, float32_t);
BENCHMARK_TEMPLATE(core_to_chars_shortest_size, float64_t);

BENCHMARK_TEMPLATE(core_to_chars_shortest_convert, float32_t, char8_t);
BENCHMARK_TEMPLATE(core_to_chars_shortest_convert, float64_t, char8_t);


//BENCHMARK_TEMPLATE(std_to_chars_sci, float32_t);
//BENCHMARK_TEMPLATE(std_to_chars_fix, float32_t);
//BENCHMARK_TEMPLATE(std_to_chars_short, float32_t);
//
//BENCHMARK_TEMPLATE(std_to_chars_sci, float64_t);
//BENCHMARK_TEMPLATE(std_to_chars_fix, float64_t);
//BENCHMARK_TEMPLATE(std_to_chars_short, float64_t);
//
//BENCHMARK_TEMPLATE(core_to_chars_sci, float32_t);
//BENCHMARK_TEMPLATE(core_to_chars_fix, float32_t);
//BENCHMARK_TEMPLATE(core_to_chars_sci, float64_t);


static void no_op(benchmark::State& state)
{
	bool const volatile ok = false;
	while(state.KeepRunning())
	{
		benchmark::DoNotOptimize(ok);
	}
}

BENCHMARK(no_op);

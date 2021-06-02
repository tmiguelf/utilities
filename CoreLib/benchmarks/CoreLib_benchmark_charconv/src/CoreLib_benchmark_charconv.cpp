//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///		Provides string conversion functions to be able to handle UNICODE
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

#include <benchmark/benchmark.h>

#include <array>
#include <string>
#include <vector>
#include <charconv>
#include <limits>
#include <type_traits>

#include <CoreLib/string/core_string_numeric.hpp>
#include <CoreLib/string/core_string_misc.hpp>

//======== ======== ======== ======== Auxiliary Test case generator ======== ======== ======== ========

template <typename char_T>
std::basic_string<char_T> str2_Tstring(std::string_view p_str)
{
	std::basic_string<char_T> out;
	const char* first = p_str.data();
	const char* last = first + p_str.size();

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
static const std::vector<int64_t> s_numbers =
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

static const std::vector<uint64_t> u_numbers =
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

static const std::vector<uint64_t> hex_numbers =
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

const std::vector<std::string> badCases_s =
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

const std::vector<std::string> hexBadCases_s =
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

template <typename num_T, typename char_T>
std::vector<std::basic_string<char_T>> get_goodStr()
{
	static_assert(!std::is_floating_point_v<num_T>);
	
	std::vector<std::basic_string<char_T>> out;
	constexpr num_T max = std::numeric_limits<num_T>::max();
	std::array<char, 20> buff;
	char* first = buff.data();
	char* last = first + buff.size();

	for(uint64_t num : u_numbers)
	{
		if(num <= max)
		{
			std::to_chars_result res = std::to_chars(first, last, num);
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
				std::to_chars_result res = std::to_chars(first, last, num);
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

	for(const std::string& t_str : badCases_s)
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

	for(uint64_t num : u_numbers)
	{
		if(num <= max)
		{
			out.push_back(static_cast<num_T>(num));
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
				out.push_back(static_cast<num_T>(num));
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
	char* first = buff.data();
	char* last = first + buff.size();

	for(uint64_t num : hex_numbers)
	{
		if(num <= max)
		{
			std::to_chars_result res = std::to_chars(first, last, num, 16);
			core::toUpperCase(std::span<char8_t>{reinterpret_cast<char8_t*>(first), static_cast<uintptr_t>(res.ptr - first)});
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

	for(const std::string& t_str : hexBadCases_s)
	{
		out.emplace_back(str2_Tstring<char_T>(t_str));
	}

	out.emplace_back(str2_Tstring<char_T>(one_past_end_hex<num_T>::str));

	return out;
}

template <typename num_T>
std::vector<num_T> get_num_hex()
{
	std::vector<num_T> out;
	constexpr num_T max = std::numeric_limits<num_T>::max();

	for(uint64_t num : hex_numbers)
	{
		if(num <= max)
		{
			out.push_back(static_cast<num_T>(num));
		}
		else break;
	}

	return out;
}

//======== ======== ======== ======== Test templating ======== ======== ======== ========

template<typename num_T>
static void std_from_chars_good(benchmark::State& state)
{
	const std::vector<std::u8string>& testList = get_goodStr<num_T, char8_t>();
	uintptr_t index = 0;

	for (auto _ : state)
	{
		const std::u8string& testCase = testList[index];
		const char* first = reinterpret_cast<const char*>(testCase.data());
		const char* last = first + testCase.size();
		num_T result;
		std::from_chars_result res = std::from_chars(first, last, result);

		volatile bool ok = res.ec == std::error_code{} && res.ptr == last;

		benchmark::DoNotOptimize(result);
		benchmark::DoNotOptimize(ok);

		if(index >= testList.size()) index = 0;
	}
}

template<typename num_T>
static void core_from_chars_good(benchmark::State& state)
{
	const std::vector<std::u8string>& testList = get_goodStr<num_T, char8_t>();
	uintptr_t index = 0;

	for (auto _ : state)
	{
		const std::u8string& testCase = testList[index];
		core::from_chars_result<num_T> result = core::from_chars<num_T>(testCase);

		volatile bool ok = result.has_value();

		benchmark::DoNotOptimize(result.value());
		benchmark::DoNotOptimize(ok);

		if(index >= testList.size()) index = 0;
	}
}


template<typename num_T>
static void std_from_chars_bad(benchmark::State& state)
{
	const std::vector<std::u8string>& testList = get_badStr<num_T, char8_t>();
	uintptr_t index = 0;

	for (auto _ : state)
	{
		const std::u8string& testCase = testList[index];
		const char* first = reinterpret_cast<const char*>(testCase.data());
		const char* last = first + testCase.size();
		num_T result;
		std::from_chars_result res = std::from_chars(first, last, result);

		volatile bool ok = res.ec == std::error_code{} && res.ptr == last;

		benchmark::DoNotOptimize(ok);

		if(index >= testList.size()) index = 0;
	}
}

template<typename num_T>
static void core_from_chars_bad(benchmark::State& state)
{
	const std::vector<std::u8string>& testList = get_badStr<num_T, char8_t>();
	uintptr_t index = 0;

	for (auto _ : state)
	{
		const std::u8string& testCase = testList[index];
		core::from_chars_result<num_T> result = core::from_chars<num_T>(testCase);

		volatile bool ok = result.has_value();

		benchmark::DoNotOptimize(ok);

		if(index >= testList.size()) index = 0;
	}
}

template<typename num_T>
static void std_from_hex_chars_good(benchmark::State& state)
{
	const std::vector<std::u8string>& testList = get_goodStr_hex<num_T, char8_t>();
	uintptr_t index = 0;

	for (auto _ : state)
	{
		const std::u8string& testCase = testList[index];
		const char* first = reinterpret_cast<const char*>(testCase.data());
		const char* last = first + testCase.size();
		num_T result;
		std::from_chars_result res = std::from_chars(first, last, result, 16);

		volatile bool ok = res.ec == std::error_code{} && res.ptr == last;

		benchmark::DoNotOptimize(result);
		benchmark::DoNotOptimize(ok);

		if(index >= testList.size()) index = 0;
	}
}

template<typename num_T>
static void core_from_hex_chars_good(benchmark::State& state)
{
	const std::vector<std::u8string>& testList = get_goodStr_hex<num_T, char8_t>();
	uintptr_t index = 0;

	for (auto _ : state)
	{
		const std::u8string& testCase = testList[index];
		core::from_chars_result<num_T> result = core::from_hex_chars<num_T>(testCase);

		volatile bool ok = result.has_value();

		benchmark::DoNotOptimize(result.value());
		benchmark::DoNotOptimize(ok);

		if(index >= testList.size()) index = 0;
	}
}


template<typename num_T>
static void std_from_hex_chars_bad(benchmark::State& state)
{
	const std::vector<std::u8string>& testList = get_badStr_hex<num_T, char8_t>();
	uintptr_t index = 0;

	for (auto _ : state)
	{
		const std::u8string& testCase = testList[index];
		const char* first = reinterpret_cast<const char*>(testCase.data());
		const char* last = first + testCase.size();
		num_T result;
		std::from_chars_result res = std::from_chars(first, last, result, 16);

		volatile bool ok = res.ec == std::error_code{} && res.ptr == last;

		benchmark::DoNotOptimize(ok);

		if(index >= testList.size()) index = 0;
	}
}

template<typename num_T>
static void core_from_hex_chars_bad(benchmark::State& state)
{
	const std::vector<std::u8string>& testList = get_badStr_hex<num_T, char8_t>();
	uintptr_t index = 0;

	for (auto _ : state)
	{
		const std::u8string& testCase = testList[index];
		core::from_chars_result<num_T> result = core::from_hex_chars<num_T>(testCase);

		volatile bool ok = result.has_value();

		benchmark::DoNotOptimize(ok);

		if(index >= testList.size()) index = 0;
	}
}


template<typename num_T>
static void std_to_chars(benchmark::State& state)
{
	const std::vector<num_T>& testList = get_num<num_T>();
	uintptr_t index = 0;

	constexpr uintptr_t buffSize = core::to_char_dec_max_digits_v<num_T>;
	std::array<char8_t, buffSize> buffer;

	for (auto _ : state)
	{
		num_T testCase = testList[index];
		char* first = reinterpret_cast<char*>(buffer.data());
		char* last = first + buffer.size();
		std::to_chars_result res = std::to_chars(first, last, testCase);

		std::u8string_view result {buffer.data(), static_cast<uintptr_t>(res.ptr - first)};

		benchmark::DoNotOptimize(result);
		if(index >= testList.size()) index = 0;
	}
}

template<typename num_T>
static void core_to_chars(benchmark::State& state)
{
	const std::vector<num_T>& testList = get_num<num_T>();
	uintptr_t index = 0;

	constexpr uintptr_t buffSize = core::to_char_dec_max_digits_v<num_T>;
	std::array<char8_t, buffSize> buffer;

	for (auto _ : state)
	{
		num_T testCase = testList[index];
		uintptr_t res_size = core::to_chars(testCase, std::span<char8_t, buffSize>(buffer));

		std::u8string_view result {buffer.data(), res_size};

		benchmark::DoNotOptimize(result);
		if(index >= testList.size()) index = 0;
	}
}


template<typename num_T>
static void std_to_hex_chars(benchmark::State& state)
{
	const std::vector<num_T>& testList = get_num_hex<num_T>();
	uintptr_t index = 0;

	constexpr uintptr_t buffSize = core::to_char_hex_max_digits_v<num_T>;
	std::array<char8_t, buffSize> buffer;

	for (auto _ : state)
	{
		num_T testCase = testList[index];
		char* first = reinterpret_cast<char*>(buffer.data());
		char* last = first + buffer.size();
		std::to_chars_result res = std::to_chars(first, last, testCase, 16);

		std::u8string_view result {buffer.data(), static_cast<uintptr_t>(res.ptr - first)};

		benchmark::DoNotOptimize(result);
		if(index >= testList.size()) index = 0;
	}
}

template<typename num_T>
static void core_to_hex_chars(benchmark::State& state)
{
	const std::vector<num_T>& testList = get_num_hex<num_T>();
	uintptr_t index = 0;

	constexpr uintptr_t buffSize = core::to_char_hex_max_digits_v<num_T>;
	std::array<char8_t, buffSize> buffer;

	for (auto _ : state)
	{
		num_T testCase = testList[index];
		uintptr_t res_size = core::to_hex_chars(testCase, std::span<char8_t, buffSize>(buffer));

		std::u8string_view result {buffer.data(), res_size};

		benchmark::DoNotOptimize(result);
		if(index >= testList.size()) index = 0;
	}
}

template<typename num_T>
static void core_to_hex_chars_fix(benchmark::State& state)
{
	const std::vector<num_T>& testList = get_num_hex<num_T>();
	uintptr_t index = 0;

	constexpr uintptr_t buffSize = core::to_char_hex_max_digits_v<num_T>;
	std::array<char8_t, buffSize> buffer;

	for (auto _ : state)
	{
		num_T testCase = testList[index];
		core::to_hex_chars_fix(testCase, std::span<char8_t, buffSize>(buffer));

		std::u8string_view result {buffer.data(), buffSize};

		benchmark::DoNotOptimize(result);
		if(index >= testList.size()) index = 0;
	}
}


//======== ======== ======== ======== Template Resolution ======== ======== ======== ========

//---- from Decimal ----
static inline void  std_from_chars_good_uint8 (benchmark::State& state) {  std_from_chars_good<uint8_t >(state); }
static inline void  std_from_chars_good_uint16(benchmark::State& state) {  std_from_chars_good<uint16_t>(state); }
static inline void  std_from_chars_good_uint32(benchmark::State& state) {  std_from_chars_good<uint32_t>(state); }
static inline void  std_from_chars_good_uint64(benchmark::State& state) {  std_from_chars_good<uint64_t>(state); }
static inline void  std_from_chars_good_int8  (benchmark::State& state) {  std_from_chars_good<int8_t  >(state); }
static inline void  std_from_chars_good_int16 (benchmark::State& state) {  std_from_chars_good<int16_t >(state); }
static inline void  std_from_chars_good_int32 (benchmark::State& state) {  std_from_chars_good<int32_t >(state); }
static inline void  std_from_chars_good_int64 (benchmark::State& state) {  std_from_chars_good<int64_t >(state); }

static inline void core_from_chars_good_uint8 (benchmark::State& state) { core_from_chars_good<uint8_t >(state); }
static inline void core_from_chars_good_uint16(benchmark::State& state) { core_from_chars_good<uint16_t>(state); }
static inline void core_from_chars_good_uint32(benchmark::State& state) { core_from_chars_good<uint32_t>(state); }
static inline void core_from_chars_good_uint64(benchmark::State& state) { core_from_chars_good<uint64_t>(state); }
static inline void core_from_chars_good_int8  (benchmark::State& state) { core_from_chars_good<int8_t  >(state); }
static inline void core_from_chars_good_int16 (benchmark::State& state) { core_from_chars_good<int16_t >(state); }
static inline void core_from_chars_good_int32 (benchmark::State& state) { core_from_chars_good<int32_t >(state); }
static inline void core_from_chars_good_int64 (benchmark::State& state) { core_from_chars_good<int64_t >(state); }

static inline void  std_from_chars_bad_uint8 (benchmark::State& state) {  std_from_chars_bad<uint8_t >(state); }
static inline void  std_from_chars_bad_uint16(benchmark::State& state) {  std_from_chars_bad<uint16_t>(state); }
static inline void  std_from_chars_bad_uint32(benchmark::State& state) {  std_from_chars_bad<uint32_t>(state); }
static inline void  std_from_chars_bad_uint64(benchmark::State& state) {  std_from_chars_bad<uint64_t>(state); }
static inline void  std_from_chars_bad_int8  (benchmark::State& state) {  std_from_chars_bad<int8_t  >(state); }
static inline void  std_from_chars_bad_int16 (benchmark::State& state) {  std_from_chars_bad<int16_t >(state); }
static inline void  std_from_chars_bad_int32 (benchmark::State& state) {  std_from_chars_bad<int32_t >(state); }
static inline void  std_from_chars_bad_int64 (benchmark::State& state) {  std_from_chars_bad<int64_t >(state); }

static inline void core_from_chars_bad_uint8 (benchmark::State& state) { core_from_chars_bad<uint8_t >(state); }
static inline void core_from_chars_bad_uint16(benchmark::State& state) { core_from_chars_bad<uint16_t>(state); }
static inline void core_from_chars_bad_uint32(benchmark::State& state) { core_from_chars_bad<uint32_t>(state); }
static inline void core_from_chars_bad_uint64(benchmark::State& state) { core_from_chars_bad<uint64_t>(state); }
static inline void core_from_chars_bad_int8  (benchmark::State& state) { core_from_chars_bad<int8_t  >(state); }
static inline void core_from_chars_bad_int16 (benchmark::State& state) { core_from_chars_bad<int16_t >(state); }
static inline void core_from_chars_bad_int32 (benchmark::State& state) { core_from_chars_bad<int32_t >(state); }
static inline void core_from_chars_bad_int64 (benchmark::State& state) { core_from_chars_bad<int64_t >(state); }


//---- from Hexadecimal ----
static inline void  std_from_hex_chars_good_uint8 (benchmark::State& state) {  std_from_hex_chars_good<uint8_t >(state); }
static inline void  std_from_hex_chars_good_uint16(benchmark::State& state) {  std_from_hex_chars_good<uint16_t>(state); }
static inline void  std_from_hex_chars_good_uint32(benchmark::State& state) {  std_from_hex_chars_good<uint32_t>(state); }
static inline void  std_from_hex_chars_good_uint64(benchmark::State& state) {  std_from_hex_chars_good<uint64_t>(state); }

static inline void core_from_hex_chars_good_uint8 (benchmark::State& state) { core_from_hex_chars_good<uint8_t >(state); }
static inline void core_from_hex_chars_good_uint16(benchmark::State& state) { core_from_hex_chars_good<uint16_t>(state); }
static inline void core_from_hex_chars_good_uint32(benchmark::State& state) { core_from_hex_chars_good<uint32_t>(state); }
static inline void core_from_hex_chars_good_uint64(benchmark::State& state) { core_from_hex_chars_good<uint64_t>(state); }

static inline void  std_from_hex_chars_bad_uint8 (benchmark::State& state) {  std_from_hex_chars_bad<uint8_t >(state); }
static inline void  std_from_hex_chars_bad_uint16(benchmark::State& state) {  std_from_hex_chars_bad<uint16_t>(state); }
static inline void  std_from_hex_chars_bad_uint32(benchmark::State& state) {  std_from_hex_chars_bad<uint32_t>(state); }
static inline void  std_from_hex_chars_bad_uint64(benchmark::State& state) {  std_from_hex_chars_bad<uint64_t>(state); }

static inline void core_from_hex_chars_bad_uint8 (benchmark::State& state) { core_from_hex_chars_bad<uint8_t >(state); }
static inline void core_from_hex_chars_bad_uint16(benchmark::State& state) { core_from_hex_chars_bad<uint16_t>(state); }
static inline void core_from_hex_chars_bad_uint32(benchmark::State& state) { core_from_hex_chars_bad<uint32_t>(state); }
static inline void core_from_hex_chars_bad_uint64(benchmark::State& state) { core_from_hex_chars_bad<uint64_t>(state); }


//---- to Decimal ----
static inline void  std_to_chars_uint8 (benchmark::State& state) {  std_to_chars<uint8_t >(state); }
static inline void  std_to_chars_uint16(benchmark::State& state) {  std_to_chars<uint16_t>(state); }
static inline void  std_to_chars_uint32(benchmark::State& state) {  std_to_chars<uint32_t>(state); }
static inline void  std_to_chars_uint64(benchmark::State& state) {  std_to_chars<uint64_t>(state); }
static inline void  std_to_chars_int8  (benchmark::State& state) {  std_to_chars<int8_t  >(state); }
static inline void  std_to_chars_int16 (benchmark::State& state) {  std_to_chars<int16_t >(state); }
static inline void  std_to_chars_int32 (benchmark::State& state) {  std_to_chars<int32_t >(state); }
static inline void  std_to_chars_int64 (benchmark::State& state) {  std_to_chars<int64_t >(state); }

static inline void core_to_chars_uint8 (benchmark::State& state) { core_to_chars<uint8_t >(state); }
static inline void core_to_chars_uint16(benchmark::State& state) { core_to_chars<uint16_t>(state); }
static inline void core_to_chars_uint32(benchmark::State& state) { core_to_chars<uint32_t>(state); }
static inline void core_to_chars_uint64(benchmark::State& state) { core_to_chars<uint64_t>(state); }
static inline void core_to_chars_int8  (benchmark::State& state) { core_to_chars<int8_t  >(state); }
static inline void core_to_chars_int16 (benchmark::State& state) { core_to_chars<int16_t >(state); }
static inline void core_to_chars_int32 (benchmark::State& state) { core_to_chars<int32_t >(state); }
static inline void core_to_chars_int64 (benchmark::State& state) { core_to_chars<int64_t >(state); }

//---- to Hexadecimal ----
static inline void  std_to_hex_chars_uint8 (benchmark::State& state) {  std_to_hex_chars<uint8_t >(state); }
static inline void  std_to_hex_chars_uint16(benchmark::State& state) {  std_to_hex_chars<uint16_t>(state); }
static inline void  std_to_hex_chars_uint32(benchmark::State& state) {  std_to_hex_chars<uint32_t>(state); }
static inline void  std_to_hex_chars_uint64(benchmark::State& state) {  std_to_hex_chars<uint64_t>(state); }

static inline void core_to_hex_chars_uint8 (benchmark::State& state) { core_to_hex_chars<uint8_t >(state); }
static inline void core_to_hex_chars_uint16(benchmark::State& state) { core_to_hex_chars<uint16_t>(state); }
static inline void core_to_hex_chars_uint32(benchmark::State& state) { core_to_hex_chars<uint32_t>(state); }
static inline void core_to_hex_chars_uint64(benchmark::State& state) { core_to_hex_chars<uint64_t>(state); }

static inline void core_to_hex_chars_fix_uint8 (benchmark::State& state) { core_to_hex_chars_fix<uint8_t >(state); }
static inline void core_to_hex_chars_fix_uint16(benchmark::State& state) { core_to_hex_chars_fix<uint16_t>(state); }
static inline void core_to_hex_chars_fix_uint32(benchmark::State& state) { core_to_hex_chars_fix<uint32_t>(state); }
static inline void core_to_hex_chars_fix_uint64(benchmark::State& state) { core_to_hex_chars_fix<uint64_t>(state); }
//======== ======== ======== ======== Benchmark Instantiation ======== ======== ======== ========

BENCHMARK( std_from_chars_good_uint8 );
BENCHMARK(core_from_chars_good_uint8 );
BENCHMARK( std_from_chars_good_uint16);
BENCHMARK(core_from_chars_good_uint16);
BENCHMARK( std_from_chars_good_uint32);
BENCHMARK(core_from_chars_good_uint32);
BENCHMARK( std_from_chars_good_uint64);
BENCHMARK(core_from_chars_good_uint64);
BENCHMARK( std_from_chars_good_int8  );
BENCHMARK(core_from_chars_good_int8  );
BENCHMARK( std_from_chars_good_int16 );
BENCHMARK(core_from_chars_good_int16 );
BENCHMARK( std_from_chars_good_int32 );
BENCHMARK(core_from_chars_good_int32 );
BENCHMARK( std_from_chars_good_int64 );
BENCHMARK(core_from_chars_good_int64 );

BENCHMARK( std_from_chars_bad_uint8 );
BENCHMARK(core_from_chars_bad_uint8 );
BENCHMARK( std_from_chars_bad_uint16);
BENCHMARK(core_from_chars_bad_uint16);
BENCHMARK( std_from_chars_bad_uint32);
BENCHMARK(core_from_chars_bad_uint32);
BENCHMARK( std_from_chars_bad_uint64);
BENCHMARK(core_from_chars_bad_uint64);
BENCHMARK( std_from_chars_bad_int8  );
BENCHMARK(core_from_chars_bad_int8  );
BENCHMARK( std_from_chars_bad_int16 );
BENCHMARK(core_from_chars_bad_int16 );
BENCHMARK( std_from_chars_bad_int32 );
BENCHMARK(core_from_chars_bad_int32 );
BENCHMARK( std_from_chars_bad_int64 );
BENCHMARK(core_from_chars_bad_int64 );


BENCHMARK( std_from_hex_chars_good_uint8 );
BENCHMARK(core_from_hex_chars_good_uint8 );
BENCHMARK( std_from_hex_chars_good_uint16);
BENCHMARK(core_from_hex_chars_good_uint16);
BENCHMARK( std_from_hex_chars_good_uint32);
BENCHMARK(core_from_hex_chars_good_uint32);
BENCHMARK( std_from_hex_chars_good_uint64);
BENCHMARK(core_from_hex_chars_good_uint64);

BENCHMARK( std_from_hex_chars_bad_uint8 );
BENCHMARK(core_from_hex_chars_bad_uint8 );
BENCHMARK( std_from_hex_chars_bad_uint16);
BENCHMARK(core_from_hex_chars_bad_uint16);
BENCHMARK( std_from_hex_chars_bad_uint32);
BENCHMARK(core_from_hex_chars_bad_uint32);
BENCHMARK( std_from_hex_chars_bad_uint64);
BENCHMARK(core_from_hex_chars_bad_uint64);


BENCHMARK( std_to_chars_uint8 );
BENCHMARK(core_to_chars_uint8 );
BENCHMARK( std_to_chars_uint16);
BENCHMARK(core_to_chars_uint16);
BENCHMARK( std_to_chars_uint32);
BENCHMARK(core_to_chars_uint32);
BENCHMARK( std_to_chars_uint64);
BENCHMARK(core_to_chars_uint64);
BENCHMARK( std_to_chars_int8  );
BENCHMARK(core_to_chars_int8  );
BENCHMARK( std_to_chars_int16 );
BENCHMARK(core_to_chars_int16 );
BENCHMARK( std_to_chars_int32 );
BENCHMARK(core_to_chars_int32 );
BENCHMARK( std_to_chars_int64 );
BENCHMARK(core_to_chars_int64 );

BENCHMARK( std_to_hex_chars_uint8 );
BENCHMARK(core_to_hex_chars_uint8 );
BENCHMARK(core_to_hex_chars_fix_uint8 );
BENCHMARK( std_to_hex_chars_uint16);
BENCHMARK(core_to_hex_chars_uint16);
BENCHMARK(core_to_hex_chars_fix_uint16);
BENCHMARK( std_to_hex_chars_uint32);
BENCHMARK(core_to_hex_chars_uint32);
BENCHMARK(core_to_hex_chars_fix_uint32);
BENCHMARK( std_to_hex_chars_uint64);
BENCHMARK(core_to_hex_chars_uint64);
BENCHMARK(core_to_hex_chars_fix_uint64);

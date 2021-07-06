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

#include <utility>
#include <type_traits>
#include <vector>
#include <gtest/gtest.h>

#include <CoreLib/toPrint/toPrint.hpp>
#include <CoreLib/toPrint/toPrint_filesystem.hpp>
#include <CoreLib/toPrint/toPrint_net.hpp>

using namespace core::literals;
using namespace std::literals;

class test_sink: public core::sink_toPrint_base
{
public:
	void write(std::u8string_view p_message)
	{
		m_print_cache.push_back(std::u8string{p_message});
	}

public:

	std::vector<std::u8string> m_print_cache;
};


class TestStr
{
public:
	uint64_t data = 0;
};

template<>
class core::toPrint<TestStr>: public core::toPrint_base
{
public:
	toPrint(const TestStr&)
	{
		m_size = preamble.size();
	}

	constexpr uintptr_t size(const char8_t&) const { return m_size; }

	void getPrint(char8_t* p_out) const //final
	{
		memcpy(p_out, preamble.data(), preamble.size());
	}

private:
	static constexpr std::u8string_view preamble = u8"TestStr"sv;
	uintptr_t m_size = 0;
};



TEST(toPrint, toPrint_interface)
{
	{
		test_sink tsink;

		TestStr test;
#ifdef _WIN32
		std::wstring_view help{__FILEW__};

		core::os_string_view fileName = std::wstring_view{__FILEW__};
#else
		core::os_string_view fileName = std::string_view{__FILE__};
#endif

		core_ToPrint(char8_t, tsink, test);
		core_ToPrint(char8_t, tsink, 32);
		core_ToPrint(char8_t, tsink, &test);
		core_ToPrint(char8_t, tsink, "string_view"sv);
		core_ToPrint(char8_t, tsink, u8"u8string_view"sv);
		core_ToPrint(char8_t, tsink, 'A');
		core_ToPrint(char8_t, tsink, u8'A');
		core_ToPrint(char8_t, tsink, uint8_t{5});
		core_ToPrint(char8_t, tsink, int8_t{-5});
		core_ToPrint(char8_t, tsink);
		core_ToPrint(char8_t, tsink, "Combination "sv, 32, ' ', test);

		ASSERT_EQ(tsink.m_print_cache.size(), 11);
		ASSERT_EQ(tsink.m_print_cache[0], std::u8string_view{u8"TestStr"});
		ASSERT_EQ(tsink.m_print_cache[1], std::u8string_view{u8"32"});
		{
			std::u8string_view log_message_2 = tsink.m_print_cache[2];
			ASSERT_EQ(log_message_2.size(), sizeof(void*) * 2 + 2);
			ASSERT_EQ(log_message_2.substr(0, 2), std::u8string_view{u8"0x"});
			ASSERT_TRUE(core::is_hex(log_message_2.substr(2)));
		}
		ASSERT_EQ(tsink.m_print_cache[3], std::u8string_view{u8"string_view"});
		ASSERT_EQ(tsink.m_print_cache[4], std::u8string_view{u8"u8string_view"});
		ASSERT_EQ(tsink.m_print_cache[5], std::u8string_view{u8"A"});
		ASSERT_EQ(tsink.m_print_cache[6], std::u8string_view{u8"A"});
		ASSERT_EQ(tsink.m_print_cache[7], std::u8string_view{u8"5"});
		ASSERT_EQ(tsink.m_print_cache[8], std::u8string_view{u8"-5"});
		ASSERT_EQ(tsink.m_print_cache[9], std::u8string_view{u8""});
		ASSERT_EQ(tsink.m_print_cache[10], std::u8string_view{u8"Combination 32 TestStr"});
	}
}
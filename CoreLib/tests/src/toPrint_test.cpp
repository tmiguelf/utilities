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

#include <utility>
#include <type_traits>
#include <vector>
#include <gtest/gtest.h>

#include <CoreLib/toPrint/toPrint.hpp>
#include <CoreLib/toPrint/toPrint_filesystem.hpp>
#include <CoreLib/toPrint/toPrint_net.hpp>
#include <CoreLib/toPrint/toPrint_enum.hpp>

using core::literals::operator "" _ui8;
using core::literals::operator "" _i8;
using core::literals::operator "" _uip;


class TestStr
{
public:
	uint64_t data = 0;
};

template<>
class core::toPrint<TestStr>: public core::toPrint_base
{
public:
	toPrint(TestStr const&)
	{
		m_size = preamble.size();
	}

	constexpr uintptr_t size(char8_t const&) const { return m_size; }

	void get_print(char8_t* p_out) const //final
	{
		memcpy(p_out, preamble.data(), preamble.size());
	}

private:
	static constexpr std::u8string_view preamble = u8"TestStr"sv;
	uintptr_t m_size = 0;
};

TEST(toPrint, toPrint_interface)
{
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

	{
		test_sink tsink;

		TestStr test;

		core::print<char8_t>(tsink, test);
		core::print<char8_t>(tsink, 32);
		core::print<char8_t>(tsink, &test);
		core::print<char8_t>(tsink, "string_view"sv);
		core::print<char8_t>(tsink, u8"u8string_view"sv);
		core::print<char8_t>(tsink, 'A');
		core::print<char8_t>(tsink, u8'A');
		core::print<char8_t>(tsink, 5_ui8);
		core::print<char8_t>(tsink, -5_i8);
		core::print<char8_t>(tsink);
		core::print<char8_t>(tsink, "Combination "sv, 32, ' ', test);

		ASSERT_EQ(tsink.m_print_cache.size(), 11_uip);
		ASSERT_EQ(tsink.m_print_cache[0], u8"TestStr"sv);
		ASSERT_EQ(tsink.m_print_cache[1], u8"32"sv);
		{
			std::u8string_view log_message_2 = tsink.m_print_cache[2];
			ASSERT_EQ(log_message_2.size(), sizeof(void*) * 2 + 2);
			ASSERT_EQ(log_message_2.substr(0, 2), u8"0x"sv);
			ASSERT_TRUE(core::is_hex(log_message_2.substr(2)));
		}
		ASSERT_EQ(tsink.m_print_cache[ 3], u8"string_view"sv);
		ASSERT_EQ(tsink.m_print_cache[ 4], u8"u8string_view"sv);
		ASSERT_EQ(tsink.m_print_cache[ 5], u8"A"sv);
		ASSERT_EQ(tsink.m_print_cache[ 6], u8"A"sv);
		ASSERT_EQ(tsink.m_print_cache[ 7], u8"5"sv);
		ASSERT_EQ(tsink.m_print_cache[ 8], u8"-5"sv);
		ASSERT_EQ(tsink.m_print_cache[ 9], u8""sv);
		ASSERT_EQ(tsink.m_print_cache[10], u8"Combination 32 TestStr"sv);
	}
}



enum class TestEnum : uint32_t
{
	Val0 = 0,
	Val1 = 1,
	Val2 = 2,
};

namespace core
{
	template<>
	struct toPrint_enum_string_view_table<TestEnum>
	{
		inline static std::u8string_view to_string(TestEnum const p_val)
		{
			switch(p_val)
			{
			case TestEnum::Val0: return u8"Val0"sv;
			case TestEnum::Val1: return u8"Val1"sv;
			case TestEnum::Val2: return u8"Val2"sv;
			}

			return std::u8string_view{};
		}

		static constexpr std::u8string_view enum_name = u8"TestEnum"sv;
	};

	template<>
	class toPrint<TestEnum> : public toPrint_enum_ASCII<TestEnum>
	{
	public:
		toPrint(TestEnum const p_val) : toPrint_enum_ASCII(p_val) {}
	};

}

template<typename char_t>
class test_type_sink: public core::sink_toPrint_base
{
public:
	void write([[maybe_unused]] std::basic_string_view<char_t> p_str) const
	{
	}

public:
};

TEST(toPrint, toPrint_type_support)
{
#define TYPE_TEST_PRINT(Type, ...) core::print<Type>(test_type_sink<Type>{} __VA_OPT__(,) __VA_ARGS__)


	TYPE_TEST_PRINT(char8_t, "string_view"sv);
	TYPE_TEST_PRINT(core::wchar_alias, L"string_view"sv);

	TYPE_TEST_PRINT(char8_t , u8"u8string_view"sv);
	TYPE_TEST_PRINT(char16_t, u"string_view"sv);
	TYPE_TEST_PRINT(char32_t, U"string_view"sv);

	TYPE_TEST_PRINT(char8_t, core::toPrint_net{ core::IP_address{}, 25 });
	TYPE_TEST_PRINT(char16_t, core::toPrint_net{ core::IP_address{}, 25 });
	TYPE_TEST_PRINT(char32_t, core::toPrint_net{ core::IP_address{}, 25 });


	TYPE_TEST_PRINT(char8_t, TestEnum::Val0);
	TYPE_TEST_PRINT(char16_t, TestEnum::Val1);
	TYPE_TEST_PRINT(char32_t, TestEnum::Val2);
	TYPE_TEST_PRINT(char8_t, TestEnum{36});

	TYPE_TEST_PRINT(char8_t , 0);
	TYPE_TEST_PRINT(char16_t, 0);
	TYPE_TEST_PRINT(char32_t, 0);

	TYPE_TEST_PRINT(char8_t );
	TYPE_TEST_PRINT(char16_t);
	TYPE_TEST_PRINT(char32_t);

#undef TYPE_TEST_PRINT

	[[maybe_unused]] constexpr bool test_derived = core::_p::is_sink_toPrint_v<test_type_sink<wchar_t> const>;

}

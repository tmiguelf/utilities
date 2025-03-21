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

#include <string_view>
#include <vector>
#include <utility>

#include <gtest/gtest.h>

#include <CoreLib/string/core_string_encoding.hpp>

namespace text_formating
{

TEST(string_encoding, UTF8_to_ANSI)
{
	std::vector<std::pair<std::vector<char8_t>, std::vector<char8_t>>> goodCases =
	{
		{	{'T', 'e', 'x', 't', ' ', 0x7F, 0xC2, 0x80, 0x00, 0xC2, 0xA7, 0xC3, 0xBF},
			{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0x00, 0xA7, 0xFF}},
	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char8_t>, std::vector<char8_t>> const& tcase : goodCases)
	{
		std::optional<std::u8string> result = core::UTF8_to_ANSI({tcase.first.data(), tcase.first.size()});
		ASSERT_TRUE(result.has_value()) << "Case " << num_case;
		ASSERT_EQ((std::u8string_view{result.value()}), (std::u8string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}

	std::vector<std::vector<char8_t>> badCases =
	{
		{'B', 'a', 'd', ' ', 0xC4, 0x80	},	//out of range
		{'B', 'a', 'd', ' ', 0xC2, 0x42	},	//bad code point
		{'B', 'a', 'd', ' ', 0xC2		},	//premature ending
	};

	num_case = 0;

	for(std::vector<char8_t> const& tcase : badCases)
	{
		std::optional<std::u8string> result = core::UTF8_to_ANSI({tcase.data(), tcase.size()});
		ASSERT_FALSE(result.has_value()) << "Case " << num_case;
		++num_case;
	}
}

TEST(string_encoding, UTF16_to_ANSI)
{
	std::vector<std::pair<std::vector<char16_t>, std::vector<char8_t>>> goodCases =
	{
		{	{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0x00, 0xA7, 0xFF},
			{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0x00, 0xA7, 0xFF}},
	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char16_t>, std::vector<char8_t>> const& tcase : goodCases)
	{
		std::optional<std::u8string> result = core::UTF16_to_ANSI({tcase.first.data(), tcase.first.size()});
		ASSERT_TRUE(result.has_value()) << "Case " << num_case;
		ASSERT_EQ((std::u8string_view{result.value()}), (std::u8string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}

	std::vector<std::vector<char16_t>> badCases =
	{
		{'B', 'a', 'd', ' ', 0x0100			},	//out of range
		{'B', 'a', 'd', ' ', 0xD801, 0x0020	},	//bad code point
		{'B', 'a', 'd', ' ', 0xD801			},	//premature ending
	};

	num_case = 0;

	for(std::vector<char16_t> const& tcase : badCases)
	{
		std::optional<std::u8string> result = core::UTF16_to_ANSI({tcase.data(), tcase.size()});
		ASSERT_FALSE(result.has_value()) << "Case " << num_case;
		++num_case;
	}
}

TEST(string_encoding, UCS2_to_ANSI)
{
	std::vector<std::pair<std::vector<char16_t>, std::vector<char8_t>>> goodCases =
	{
		{	{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0x00, 0xA7, 0xFF},
			{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0x00, 0xA7, 0xFF}},
	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char16_t>, std::vector<char8_t>> const& tcase : goodCases)
	{
		std::optional<std::u8string> result = core::UCS2_to_ANSI({tcase.first.data(), tcase.first.size()});
		ASSERT_TRUE(result.has_value()) << "Case " << num_case;
		ASSERT_EQ((std::u8string_view{result.value()}), (std::u8string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}

	std::vector<std::vector<char16_t>> badCases =
	{
		{'B', 'a', 'd', ' ', 0x0100			},	//out of range
	};

	num_case = 0;

	for(std::vector<char16_t> const& tcase : badCases)
	{
		std::optional<std::u8string> result = core::UCS2_to_ANSI({tcase.data(), tcase.size()});
		ASSERT_FALSE(result.has_value()) << "Case " << num_case;
		++num_case;
	}
}

TEST(string_encoding, UCS4_to_ANSI)
{
	std::vector<std::pair<std::vector<char32_t>, std::vector<char8_t>>> goodCases =
	{
		{	{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0x00, 0xA7, 0xFF},
			{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0x00, 0xA7, 0xFF}},
	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char32_t>, std::vector<char8_t>> const& tcase : goodCases)
	{
		std::optional<std::u8string> result = core::UCS4_to_ANSI({tcase.first.data(), tcase.first.size()});
		ASSERT_TRUE(result.has_value()) << "Case " << num_case;
		ASSERT_EQ((std::u8string_view{result.value()}), (std::u8string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}

	std::vector<std::vector<char32_t>> badCases =
	{
		{'B', 'a', 'd', ' ', 0x0100			},	//out of range
	};

	num_case = 0;

	for(std::vector<char32_t> const& tcase : badCases)
	{
		std::optional<std::u8string> result = core::UCS4_to_ANSI({tcase.data(), tcase.size()});
		ASSERT_FALSE(result.has_value()) << "Case " << num_case;
		++num_case;
	}
}

TEST(string_encoding, ANSI_to_UTF8)
{
	std::vector<std::pair<std::vector<char8_t>, std::vector<char8_t>>> goodCases =
	{
		{	{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0x00, 0xA7, 0xFF},
			{'T', 'e', 'x', 't', ' ', 0x7F, 0xC2, 0x80, 0x00, 0xC2, 0xA7, 0xC3, 0xBF}},
	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char8_t>, std::vector<char8_t>> const& tcase : goodCases)
	{
		std::u8string result = core::ANSI_to_UTF8({tcase.first.data(), tcase.first.size()});
		ASSERT_EQ((std::u8string_view{result}), (std::u8string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}
}

TEST(string_encoding, UTF16_to_UTF8)
{
	std::vector<std::pair<std::vector<char16_t>, std::vector<char8_t>>> goodCases =
	{
		{	{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0x00, 0xA7, 0xFF, 0xDBFF, 0xDFFF},
			{'T', 'e', 'x', 't', ' ', 0x7F, 0xC2, 0x80, 0x00, 0xC2, 0xA7, 0xC3, 0xBF, 0xF4, 0x8F, 0xBF, 0xBF}},
	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char16_t>, std::vector<char8_t>> const& tcase : goodCases)
	{
		std::optional<std::u8string> result = core::UTF16_to_UTF8({tcase.first.data(), tcase.first.size()});
		ASSERT_TRUE(result.has_value()) << "Case " << num_case;
		ASSERT_EQ((std::u8string_view{result.value()}), (std::u8string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}

	std::vector<std::vector<char16_t>> badCases =
	{
		{'B', 'a', 'd', ' ', 0xD801, 0x0020	},	//bad code point
		{'B', 'a', 'd', ' ', 0xD801			},	//premature ending
	};

	num_case = 0;

	for(std::vector<char16_t> const& tcase : badCases)
	{
		std::optional<std::u8string> result = core::UTF16_to_UTF8({tcase.data(), tcase.size()});
		ASSERT_FALSE(result.has_value()) << "Case " << num_case;
		++num_case;
	}

	//corner case
	{
		std::vector<char16_t> t2 = {'B', 'a', 'd', ' ', 0xD801, 0xDC20	};	//bad code point
		std::optional<std::u8string> result = core::UTF16_to_UTF8({t2.data(), t2.size() - 1});
		ASSERT_FALSE(result.has_value());
	}
}

TEST(string_encoding, UCS2_to_UTF8)
{
	std::vector<std::pair<std::vector<char16_t>, std::vector<char8_t>>> goodCases =
	{
		{	{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0x00, 0xA7, 0xFF, 0xFFFF},
			{'T', 'e', 'x', 't', ' ', 0x7F, 0xC2, 0x80, 0x00, 0xC2, 0xA7, 0xC3, 0xBF, 0xEF, 0xBF, 0xBF}},
	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char16_t>, std::vector<char8_t>> const& tcase : goodCases)
	{
		std::u8string result = core::UCS2_to_UTF8({tcase.first.data(), tcase.first.size()});
		ASSERT_EQ((std::u8string_view{result}), (std::u8string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}
}

TEST(string_encoding, UCS4_to_UTF8)
{
	std::vector<std::pair<std::vector<char32_t>, std::vector<char8_t>>> goodCases =
	{
		{	{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0x00, 0xA7, 0xFF, 0xFFFF, 0x0010FFFF},
			{'T', 'e', 'x', 't', ' ', 0x7F, 0xC2, 0x80, 0x00, 0xC2, 0xA7, 0xC3, 0xBF, 0xEF, 0xBF, 0xBF, 0xF4, 0x8F, 0xBF, 0xBF}},
	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char32_t>, std::vector<char8_t>> const& tcase : goodCases)
	{
		std::optional<std::u8string> result = core::UCS4_to_UTF8({tcase.first.data(), tcase.first.size()});
		ASSERT_TRUE(result.has_value()) << "Case " << num_case;
		ASSERT_EQ((std::u8string_view{result.value()}), (std::u8string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}

	std::vector<std::vector<char32_t>> badCases =
	{
		{'B', 'a', 'd', ' ', 0x00110000},	//to large code point
	};

	num_case = 0;

	for(std::vector<char32_t> const& tcase : badCases)
	{
		std::optional<std::u8string> result = core::UCS4_to_UTF8({tcase.data(), tcase.size()});
		ASSERT_FALSE(result.has_value()) << "Case " << num_case;
		++num_case;
	}
}

TEST(string_encoding, ANSI_to_UTF16)
{
	std::vector<std::pair<std::vector<char8_t>, std::vector<char16_t>>> goodCases =
	{
		{	{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0x00, 0xA7, 0xFF},
			{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0x00, 0xA7, 0xFF}},
	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char8_t>, std::vector<char16_t>> const& tcase : goodCases)
	{
		std::u16string result = core::ANSI_to_UTF16({tcase.first.data(), tcase.first.size()});
		ASSERT_EQ((std::u16string_view{result}), (std::u16string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}
}


TEST(string_encoding, UTF8_to_UTF16)
{
	std::vector<std::pair<std::vector<char8_t>, std::vector<char16_t>>> goodCases =
	{
		{	{'T', 'e', 'x', 't', ' ', 0x7F, 0xC2, 0x80, 0x00, 0xC2, 0xA7, 0xC3, 0xBF, 0xF4, 0x8F, 0xBF, 0xBF},
			{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0x00, 0xA7, 0xFF, 0xDBFF, 0xDFFF}},
	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char8_t>, std::vector<char16_t>> const& tcase : goodCases)
	{
		std::optional<std::u16string> result = core::UTF8_to_UTF16({tcase.first.data(), tcase.first.size()});
		ASSERT_TRUE(result.has_value()) << "Case " << num_case;
		ASSERT_EQ((std::u16string_view{result.value()}), (std::u16string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}

	std::vector<std::vector<char8_t>> badCases =
	{
		{'B', 'a', 'd', ' ', 0xF4, 0x90, 0x80, 0x80},	//out of range
		{'B', 'a', 'd', ' ', 0xED, 0xA0, 0x80},	//unencodable
		{'B', 'a', 'd', ' ', 0xC2, 0x42},	//bad code point
		{'B', 'a', 'd', ' ', 0xC2},	//premature ending
	};

	num_case = 0;

	for(std::vector<char8_t> const& tcase : badCases)
	{
		std::optional<std::u16string> result = core::UTF8_to_UTF16({tcase.data(), tcase.size()});
		ASSERT_FALSE(result.has_value()) << "Case " << num_case;
		++num_case;
	}
}

TEST(string_encoding, UCS2_to_UTF16)
{
	std::vector<std::pair<std::vector<char16_t>, std::vector<char16_t>>> goodCases =
	{
		{	{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0xFFFF, 0xD7FF, 0xE000},
			{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0xFFFF, 0xD7FF, 0xE000}},
	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char16_t>, std::vector<char16_t>> const& tcase : goodCases)
	{
		std::optional<std::u16string> result = core::UCS2_to_UTF16({tcase.first.data(), tcase.first.size()});
		ASSERT_TRUE(result.has_value()) << "Case " << num_case;
		ASSERT_EQ((std::u16string_view{result.value()}), (std::u16string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}

	std::vector<std::vector<char16_t>> badCases =
	{
		{'B', 'a', 'd', ' ', 0xD800},	//unencodable
	};

	num_case = 0;

	for(std::vector<char16_t> const& tcase : badCases)
	{
		std::optional<std::u16string> result = core::UCS2_to_UTF16({tcase.data(), tcase.size()});
		ASSERT_FALSE(result.has_value()) << "Case " << num_case;
		++num_case;
	}
}

TEST(string_encoding, UCS4_to_UTF16)
{
	std::vector<std::pair<std::vector<char32_t>, std::vector<char16_t>>> goodCases =
	{
		{	{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0xFFFF, 0xD7FF, 0x10FFFF},
			{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0xFFFF, 0xD7FF, 0xDBFF, 0xDFFF}},
	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char32_t>, std::vector<char16_t>> const& tcase : goodCases)
	{
		std::optional<std::u16string> result = core::UCS4_to_UTF16({tcase.first.data(), tcase.first.size()});
		ASSERT_TRUE(result.has_value()) << "Case " << num_case;
		ASSERT_EQ((std::u16string_view{result.value()}), (std::u16string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}

	std::vector<std::vector<char32_t>> badCases =
	{
		{'B', 'a', 'd', ' ', 0x110000},	//out of range
		{'B', 'a', 'd', ' ', 0xD800},	//unencodable
	};

	num_case = 0;

	for(std::vector<char32_t> const& tcase : badCases)
	{
		std::optional<std::u16string> result = core::UCS4_to_UTF16({tcase.data(), tcase.size()});
		ASSERT_FALSE(result.has_value()) << "Case " << num_case;
		++num_case;
	}
}

TEST(string_encoding, ANSI_to_UCS2)
{
	std::vector<std::pair<std::vector<char8_t>, std::vector<char16_t>>> goodCases =
	{
		{	{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0x00, 0xA7, 0xFF},
			{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0x00, 0xA7, 0xFF}},
	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char8_t>, std::vector<char16_t>> const& tcase : goodCases)
	{
		std::optional<std::u16string> result = core::ANSI_to_UCS2({tcase.first.data(), tcase.first.size()});
		ASSERT_TRUE(result.has_value()) << "Case " << num_case;
		ASSERT_EQ((std::u16string_view{result.value()}), (std::u16string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}
}

TEST(string_encoding, UTF8_to_UCS2)
{
	std::vector<std::pair<std::vector<char8_t>, std::vector<char16_t>>> goodCases =
	{
		{	{'T', 'e', 'x', 't', ' ', 0x7F, 0xC2, 0x80, 0x00, 0xC2, 0xA7, 0xED, 0xA0, 0x80, 0xEF, 0xBF, 0xBF},
			{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0x00, 0xA7, 0xD800, 0xFFFF}},
	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char8_t>, std::vector<char16_t>> const& tcase : goodCases)
	{
		std::optional<std::u16string> result = core::UTF8_to_UCS2({tcase.first.data(), tcase.first.size()});
		ASSERT_TRUE(result.has_value()) << "Case " << num_case;
		ASSERT_EQ((std::u16string_view{result.value()}), (std::u16string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}

	std::vector<std::vector<char8_t>> badCases =
	{
		{'B', 'a', 'd', ' ', 0xF0, 0x90, 0x80, 0x80},	//out of range
		{'B', 'a', 'd', ' ', 0xC2, 0x42},	//bad code point
		{'B', 'a', 'd', ' ', 0xC2},	//premature ending
	};

	num_case = 0;

	for(std::vector<char8_t> const& tcase : badCases)
	{
		std::optional<std::u16string> result = core::UTF8_to_UCS2({tcase.data(), tcase.size()});
		ASSERT_FALSE(result.has_value()) << "Case " << num_case;
		++num_case;
	}
}

TEST(string_encoding, UTF16_to_UCS2)
{
	std::vector<std::pair<std::vector<char16_t>, std::vector<char16_t>>> goodCases =
	{
		{	{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0xFFFF, 0xD7FF, 0xE000},
			{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0xFFFF, 0xD7FF, 0xE000}},
	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char16_t>, std::vector<char16_t>> const& tcase : goodCases)
	{
		std::optional<std::u16string> result = core::UTF16_to_UCS2({tcase.first.data(), tcase.first.size()});
		ASSERT_TRUE(result.has_value()) << "Case " << num_case;
		ASSERT_EQ((std::u16string_view{result.value()}), (std::u16string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}

	std::vector<std::vector<char16_t>> badCases =
	{
		{'B', 'a', 'd', ' ', 0xDA00, 0xDC00	},	//out of range
		{'B', 'a', 'd', ' ', 0xD801, 0x0020	},	//bad code point
		{'B', 'a', 'd', ' ', 0xD801			},	//premature ending
	};

	num_case = 0;

	for(std::vector<char16_t> const& tcase : badCases)
	{
		std::optional<std::u16string> result = core::UTF16_to_UCS2({tcase.data(), tcase.size()});
		ASSERT_FALSE(result.has_value()) << "Case " << num_case;
		++num_case;
	}
}

TEST(string_encoding, UCS4_to_UCS2)
{
	std::vector<std::pair<std::vector<char32_t>, std::vector<char16_t>>> goodCases =
	{
		{	{'T', 'e', 'x', 't', ' ', 0xFFFF},
			{'T', 'e', 'x', 't', ' ', 0xFFFF}},
	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char32_t>, std::vector<char16_t>> const& tcase : goodCases)
	{
		std::optional<std::u16string> result = core::UCS4_to_UCS2({tcase.first.data(), tcase.first.size()});
		ASSERT_TRUE(result.has_value()) << "Case " << num_case;
		ASSERT_EQ((std::u16string_view{result.value()}), (std::u16string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}

	std::vector<std::vector<char32_t>> badCases =
	{
		{'B', 'a', 'd', ' ', 0x010000},	//out of range
	};

	num_case = 0;

	for(std::vector<char32_t> const& tcase : badCases)
	{
		std::optional<std::u16string> result = core::UCS4_to_UCS2({tcase.data(), tcase.size()});
		ASSERT_FALSE(result.has_value()) << "Case " << num_case;
		++num_case;
	}
}

TEST(string_encoding, ANSI_to_UCS4)
{
	std::vector<std::pair<std::vector<char8_t>, std::vector<char32_t>>> goodCases =
	{
		{	{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0x00, 0xA7, 0xFF},
			{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0x00, 0xA7, 0xFF}},
	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char8_t>, std::vector<char32_t>> const& tcase : goodCases)
	{
		std::u32string result = core::ANSI_to_UCS4({tcase.first.data(), tcase.first.size()});
		ASSERT_EQ((std::u32string_view{result}), (std::u32string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}
}

TEST(string_encoding, UTF8_to_UCS4)
{
	std::vector<std::pair<std::vector<char8_t>, std::vector<char32_t>>> goodCases =
	{
		{	{'T', 'e', 'x', 't', ' ', 0x7F, 0xC2, 0x80, 0x00, 0xC2, 0xA7, 0xC3, 0xBF, 0xEF, 0xBF, 0xBF, 0xF4, 0x8F, 0xBF, 0xBF},
			{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0x00, 0xA7, 0xFF, 0xFFFF, 0x0010FFFF}},
		{	{0x00, 0x7F, 0xC2, 0x80, 0xDF, 0xBF, 0xE0, 0xA0, 0x80, 0xEF, 0xBF, 0xBF, 0xF0, 0x90, 0x80, 0x80, 0xF4, 0x8F, 0xBF, 0xBF },
			{0x00, 0x7F, 0x80, 0x07FF, 0x0800, 0xFFFF, 0x010000, 0x10FFFF}},
	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char8_t>, std::vector<char32_t>> const& tcase : goodCases)
	{
		std::optional<std::u32string> result = core::UTF8_to_UCS4({tcase.first.data(), tcase.first.size()});
		ASSERT_TRUE(result.has_value()) << "Case " << num_case;
		ASSERT_EQ((std::u32string_view{result.value()}), (std::u32string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}

	std::vector<std::vector<char8_t>> badCases =
	{
		{'B', 'a', 'd', ' ', 0xFE, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF},	//bad code point
		{'B', 'a', 'd', ' ', 0xFE, 0x83, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF},	//bad code point
		{'B', 'a', 'd', ' ', 0xF4, 0x9F, 0xBF, 0xBF},	//bad code point
		{'B', 'a', 'd', ' ', 0xC2, 0x42},	//bad code point
		{'B', 'a', 'd', ' ', 0xC2},	//premature ending
	};

	num_case = 0;

	for(std::vector<char8_t> const& tcase : badCases)
	{
		std::optional<std::u16string> result = core::UTF8_to_UCS2({tcase.data(), tcase.size()});
		ASSERT_FALSE(result.has_value()) << "Case " << num_case;
		++num_case;
	}
}

TEST(string_encoding, UTF16_to_UCS4)
{
	std::vector<std::pair<std::vector<char16_t>, std::vector<char32_t>>> goodCases =
	{
		{	{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0xFFFF, 0xD7FF, 0xDBFF, 0xDFFF},
			{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0xFFFF, 0xD7FF, 0x10FFFF}},
	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char16_t>, std::vector<char32_t>> const& tcase : goodCases)
	{
		std::optional<std::u32string> result = core::UTF16_to_UCS4({tcase.first.data(), tcase.first.size()});
		ASSERT_TRUE(result.has_value()) << "Case " << num_case;
		ASSERT_EQ((std::u32string_view{result.value()}), (std::u32string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}

	std::vector<std::vector<char16_t>> badCases =
	{
		{'B', 'a', 'd', ' ', 0xD801, 0x0020	},	//bad code point
		{'B', 'a', 'd', ' ', 0xD801			},	//premature ending
	};

	num_case = 0;

	for(std::vector<char16_t> const& tcase : badCases)
	{
		std::optional<std::u32string> result = core::UTF16_to_UCS4({tcase.data(), tcase.size()});
		ASSERT_FALSE(result.has_value()) << "Case " << num_case;
		++num_case;
	}
}

TEST(string_encoding, UCS2_to_UCS4)
{
	std::vector<std::pair<std::vector<char16_t>, std::vector<char32_t>>> goodCases =
	{
		{	{'T', 'e', 'x', 't', ' ', 0xFFFF},
			{'T', 'e', 'x', 't', ' ', 0xFFFF}},
	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char16_t>, std::vector<char32_t>> const& tcase : goodCases)
	{
		std::optional<std::u32string> result = core::UCS2_to_UCS4({tcase.first.data(), tcase.first.size()});
		ASSERT_TRUE(result.has_value()) << "Case " << num_case;
		ASSERT_EQ((std::u32string_view{result.value()}), (std::u32string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}
}

TEST(string_encoding, UTF8_to_ANSI_faulty)
{
	std::vector<std::pair<std::vector<char8_t>, std::vector<char8_t>>> goodCases =
	{
		{{'T', 'e', 'x', 't', ' ', 0x7F, 0xC2, 0x80, 0x00, 0xC2, 0xA7, 0xC3, 0xBF}, {'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0x00, 0xA7, 0xFF}},
		{{'O', 'k', ' ', 0xD0, 0x80, 'a'},	{'O', 'k', ' ', '?', 'a'}},	//out of range
		{{'O', 'k', ' ', 0xC2, 'a'},		{'O', 'k', ' ', '?', 'a'}},	//bad code point
		{{'O', 'k', ' ', 0xC2},				{'O', 'k', ' ', '?'}},		//premature ending

	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char8_t>, std::vector<char8_t>> const& tcase : goodCases)
	{
		std::u8string result = core::UTF8_to_ANSI_faulty({tcase.first.data(), tcase.first.size()}, '?');
		ASSERT_EQ((std::u8string_view{result}), (std::u8string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}
}

TEST(string_encoding, UTF16_to_ANSI_faulty)
{
	std::vector<std::pair<std::vector<char16_t>, std::vector<char8_t>>> goodCases =
	{
		{{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0x00, 0xA7, 0xFF}, {'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0x00, 0xA7, 0xFF}},
		{{'O', 'k', ' ', 0x0100, 'a'},	{'O', 'k', ' ', '?', 'a'}},	//out of range
		{{'O', 'k', ' ', 0xD801, 'a'},	{'O', 'k', ' ', '?', 'a'}},	//bad code point
		{{'O', 'k', ' ', 0xD801},		{'O', 'k', ' ', '?'}},		//premature ending
	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char16_t>, std::vector<char8_t>> const& tcase : goodCases)
	{
		std::u8string result = core::UTF16_to_ANSI_faulty({tcase.first.data(), tcase.first.size()}, '?');
		ASSERT_EQ((std::u8string_view{result}), (std::u8string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}
}

TEST(string_encoding, UCS2_to_ANSI_faulty)
{
	std::vector<std::pair<std::vector<char16_t>, std::vector<char8_t>>> goodCases =
	{
		{{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0x00, 0xA7, 0xFF}, {'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0x00, 0xA7, 0xFF}},
		{{'O', 'k', ' ', 0x0100, 'a'},	{'O', 'k', ' ', '?', 'a'}},	//out of range
	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char16_t>, std::vector<char8_t>> const& tcase : goodCases)
	{
		std::u8string result = core::UCS2_to_ANSI_faulty({tcase.first.data(), tcase.first.size()}, '?');
		ASSERT_EQ((std::u8string_view{result}), (std::u8string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}
}

TEST(string_encoding, UCS4_to_ANSI_faulty)
{
	std::vector<std::pair<std::vector<char32_t>, std::vector<char8_t>>> goodCases =
	{
		{{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0x00, 0xA7, 0xFF}, {'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0x00, 0xA7, 0xFF}},
		{{'O', 'k', ' ', 0x0100, 'a'},	{'O', 'k', ' ', '?', 'a'}},	//out of range
	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char32_t>, std::vector<char8_t>> const& tcase : goodCases)
	{
		std::u8string result = core::UCS4_to_ANSI_faulty({tcase.first.data(), tcase.first.size()}, '?');
		ASSERT_EQ((std::u8string_view{result}), (std::u8string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}
}

TEST(string_encoding, UTF16_to_UTF8_faulty)
{
	std::vector<std::pair<std::vector<char16_t>, std::vector<char8_t>>> goodCases =
	{
		{{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0x00, 0xA7, 0xFF, 0xDBFF, 0xDFFF}, {'T', 'e', 'x', 't', ' ', 0x7F, 0xC2, 0x80, 0x00, 0xC2, 0xA7, 0xC3, 0xBF, 0xF4, 0x8F, 0xBF, 0xBF}},
		{{'O', 'k', ' ', 0xD801, 'a'},	{'O', 'k', ' ', 0xC4, 0x80, 'a'}},	//bad code point
		{{'O', 'k', ' ', 0xD801		},	{'O', 'k', ' ', 0xC4, 0x80}},	//premature ending
	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char16_t>, std::vector<char8_t>> const& tcase : goodCases)
	{
		std::u8string result = core::UTF16_to_UTF8_faulty({tcase.first.data(), tcase.first.size()}, 0x0100);
		ASSERT_EQ((std::u8string_view{result}), (std::u8string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}
}

TEST(string_encoding, UCS4_to_UTF8_faulty)
{
	std::vector<std::pair<std::vector<char32_t>, std::vector<char8_t>>> goodCases =
	{
		{{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0x00, 0xA7, 0xFF, 0x0010FFFF}, {'T', 'e', 'x', 't', ' ', 0x7F, 0xC2, 0x80, 0x00, 0xC2, 0xA7, 0xC3, 0xBF, 0xF4, 0x8F, 0xBF, 0xBF}},
		{{'O', 'k', ' ', 0x00110000},	{'O', 'k', ' ', 0xC4, 0x80}},	//to large code point
	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char32_t>, std::vector<char8_t>> const& tcase : goodCases)
	{
		std::u8string result = core::UCS4_to_UTF8_faulty({tcase.first.data(), tcase.first.size()}, 0x0100);
		ASSERT_EQ((std::u8string_view{result}), (std::u8string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}
}

TEST(string_encoding, UTF8_to_UTF16_faulty)
{
	std::vector<std::pair<std::vector<char8_t>, std::vector<char16_t>>> goodCases =
	{
		{{'T', 'e', 'x', 't', ' ', 0x7F, 0xC2, 0x80, 0x00, 0xC2, 0xA7, 0xC3, 0xBF, 0xF4, 0x8F, 0xBF, 0xBF}, {'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0x00, 0xA7, 0xFF, 0xDBFF, 0xDFFF}},
		{{'O', 'k', ' ', 0xF4, 0xA0, 0x80, 0x80, 'a'}, {'O', 'k', ' ', 0xD800, 0xDC00, 'a'}},	//out of range
		{{'O', 'k', ' ', 0xED, 0xA0, 0x80, 'a'}, {'O', 'k', ' ', 0xD800, 0xDC00, 'a'}},	//unencodable
		{{'O', 'k', ' ', 0xC2, 'a'}, {'O', 'k', ' ', 0xD800, 0xDC00, 'a'}},	//bad code point
		{{'O', 'k', ' ', 0xC2}, {'O', 'k', ' ', 0xD800, 0xDC00}},	//premature ending
	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char8_t>, std::vector<char16_t>> const& tcase : goodCases)
	{
		std::u16string result = core::UTF8_to_UTF16_faulty({tcase.first.data(), tcase.first.size()}, 0x010000);
		ASSERT_EQ((std::u16string_view{result}), (std::u16string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}
}

TEST(string_encoding, UCS2_to_UTF16_faulty)
{
	std::vector<std::pair<std::vector<char16_t>, std::vector<char16_t>>> goodCases =
	{
		{{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0xFFFF, 0xD7FF, 0xE000}, {'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0xFFFF, 0xD7FF, 0xE000}},
		{{'O', 'k', ' ', 0xD800}, {'O', 'k', ' ', 0xD800, 0xDC00}},	//unencodable
	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char16_t>, std::vector<char16_t>> const& tcase : goodCases)
	{
		std::u16string result = core::UCS2_to_UTF16_faulty({tcase.first.data(), tcase.first.size()}, 0x010000);
		ASSERT_EQ((std::u16string_view{result}), (std::u16string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}
}

TEST(string_encoding, UCS4_to_UTF16_faulty)
{
	std::vector<std::pair<std::vector<char32_t>, std::vector<char16_t>>> goodCases =
	{
		{{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0xFFFF, 0xD7FF, 0x10FFFF}, {'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0xFFFF, 0xD7FF, 0xDBFF, 0xDFFF}},
		{{'O', 'k', ' ', 0x110000, 'a'},	{'O', 'k', ' ', 0xD800, 0xDC00, 'a'}},	//out of range
		{{'O', 'k', ' ', 0xD800, 'a'},		{'O', 'k', ' ', 0xD800, 0xDC00, 'a'}},	//unencodable
	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char32_t>, std::vector<char16_t>> const& tcase : goodCases)
	{
		std::u16string result = core::UCS4_to_UTF16_faulty({tcase.first.data(), tcase.first.size()}, 0x010000);
		ASSERT_EQ((std::u16string_view{result}), (std::u16string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}
}

TEST(string_encoding, UTF8_to_UCS2_faulty)
{
	std::vector<std::pair<std::vector<char8_t>, std::vector<char16_t>>> goodCases =
	{
		{{'T', 'e', 'x', 't', ' ', 0x7F, 0xC2, 0x80, 0x00, 0xC2, 0xA7, 0xED, 0xA0, 0x80, 0xEF, 0xBF, 0xBF}, {'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0x00, 0xA7, 0xD800, 0xFFFF}},
		{{'O', 'k', ' ', 0xF0, 0x90, 0x80, 0x80, 'a'}, {'O', 'k', ' ', '?' , 'a'}},	//out of range
		{{'O', 'k', ' ', 0xC2, 'a'}, {'O', 'k', ' ', '?' , 'a'}},	//bad code point
		{{'O', 'k', ' ', 0xC2}, {'O', 'k', ' ', '?'}},	//premature ending
	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char8_t>, std::vector<char16_t>> const& tcase : goodCases)
	{
		std::u16string result = core::UTF8_to_UCS2_faulty({tcase.first.data(), tcase.first.size()}, '?');
		ASSERT_EQ((std::u16string_view{result}), (std::u16string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}
}

TEST(string_encoding, UTF16_to_UCS2_faulty)
{
	std::vector<std::pair<std::vector<char16_t>, std::vector<char16_t>>> goodCases =
	{
		{{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0xFFFF, 0xD7FF, 0xE000}, {'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0xFFFF, 0xD7FF, 0xE000}},
		{{'O', 'k', ' ', 0xDA00, 0xDC00, 'a'}, {'O', 'k', ' ', '?' , 'a'}},	//out of range
		{{'O', 'k', ' ', 0xD801, 'a'}, {'O', 'k', ' ', '?' , 'a'}},	//bad code point
		{{'O', 'k', ' ', 0xD801}, {'O', 'k', ' ', '?'}},	//premature ending
	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char16_t>, std::vector<char16_t>> const& tcase : goodCases)
	{
		std::u16string result = core::UTF16_to_UCS2_faulty({tcase.first.data(), tcase.first.size()}, '?');
		ASSERT_EQ((std::u16string_view{result}), (std::u16string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}
}

TEST(string_encoding, UCS4_to_UCS2_faulty)
{
	std::vector<std::pair<std::vector<char32_t>, std::vector<char16_t>>> goodCases =
	{
		{{'T', 'e', 'x', 't', ' ', 0xFFFF}, {'T', 'e', 'x', 't', ' ', 0xFFFF}},
		{{'O', 'k', ' ', 0x010000, 'a'}, {'O', 'k', ' ', '?' , 'a'}},	//out of range

	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char32_t>, std::vector<char16_t>> const& tcase : goodCases)
	{
		std::u16string result = core::UCS4_to_UCS2_faulty({tcase.first.data(), tcase.first.size()}, '?');
		ASSERT_EQ((std::u16string_view{result}), (std::u16string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}
}

TEST(string_encoding, UTF8_to_UCS4_faulty)
{
	std::vector<std::pair<std::vector<char8_t>, std::vector<char32_t>>> goodCases =
	{
		{{'T', 'e', 'x', 't', ' ', 0x7F, 0xC2, 0x80, 0x00, 0xC2, 0xA7, 0xC3, 0xBF, 0xEF, 0xBF, 0xBF, 0xF4, 0x8F, 0xBF, 0xBF}, {'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0x00, 0xA7, 0xFF, 0xFFFF, 0x0010FFFF}},
		{{'O', 'k', ' ', 0xFE, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 'a'}, {'O', 'k', ' ', '?', 'a'}},	//bad code point
		{{'O', 'k', ' ', 0xFE, 0x83, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 'a'}, {'O', 'k', ' ', '?', 'a'}},	//bad code point
		{{'O', 'k', ' ', 0xF4, 0x9F, 0xBF, 0xBF, 'a'}, {'O', 'k', ' ', '?', 'a'}},	//bad code point
		{{'O', 'k', ' ', 0xC2, 'a'},	{'O', 'k', ' ', '?', 'a'}},	//bad code point
		{{'O', 'k', ' ', 0xC2},			{'O', 'k', ' ', '?'}},		//premature ending
	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char8_t>, std::vector<char32_t>> const& tcase : goodCases)
	{
		std::u32string result = core::UTF8_to_UCS4_faulty({tcase.first.data(), tcase.first.size()}, '?');
		ASSERT_EQ((std::u32string_view{result}), (std::u32string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}
}

TEST(string_encoding, UTF16_to_UCS4_faulty)
{
	std::vector<std::pair<std::vector<char16_t>, std::vector<char32_t>>> goodCases =
	{
		{{'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0xFFFF, 0xD7FF, 0xDBFF, 0xDFFF}, {'T', 'e', 'x', 't', ' ', 0x7F, 0x80, 0xFFFF, 0xD7FF, 0x10FFFF}},
		{{'O', 'k', ' ', 0xD801, 'a'},	{'O', 'k', ' ', '?', 'a'}},	//bad code point
		{{'O', 'k', ' ', 0xD801 },		{'O', 'k', ' ', '?'}},		//premature ending
	};

	uintptr_t num_case = 0;
	for(std::pair<std::vector<char16_t>, std::vector<char32_t>> const& tcase : goodCases)
	{
		std::u32string result = core::UTF16_to_UCS4_faulty({tcase.first.data(), tcase.first.size()}, '?');
		ASSERT_EQ((std::u32string_view{result}), (std::u32string_view{tcase.second.data(), tcase.second.size()})) << "Case " << num_case;
		++num_case;
	}
}

TEST(string_encoding, UNICODE_Compliant)
{
	std::vector<char32_t> goodCases =
	{
		'T', 'e', 'x', 't', ' ', 0x00, 0x7F, 0x80, 0xFF, 0xFFFF, 0xD7FF, 0xE000, 0xFFFF, 0x10FFFF
	};

	for(char32_t tcase : goodCases)
	{
		ASSERT_TRUE(core::UNICODE_Compliant(tcase)) << "Code point " << static_cast<uint32_t>(tcase);
	}

	std::vector<char32_t> badCases =
	{
		0xDFFF, 0xD800, 0x110000
	};

	for(char32_t tcase : badCases)
	{
		ASSERT_FALSE(core::UNICODE_Compliant(tcase)) << "Code point " << static_cast<uint32_t>(tcase);
	}
}

TEST(string_encoding, ASCII_Compliant_char32_t)
{
	std::vector<char32_t> goodCases =
	{
		'T', 'e', 'x', 't', ' ', 0x00, 0x7F
	};

	for(char32_t tcase : goodCases)
	{
		ASSERT_TRUE(core::ASCII_Compliant(tcase)) << "Code point " << static_cast<uint32_t>(tcase);
	}

	std::vector<char32_t> badCases =
	{
		0x80, 0x100, 0xD800, 0x110000, 0xFFFFFFFF
	};

	for(char32_t tcase : badCases)
	{
		ASSERT_FALSE(core::ASCII_Compliant(tcase)) << "Code point " << static_cast<uint32_t>(tcase);
	}
}

TEST(string_encoding, ASCII_Compliant_char8_t)
{
	std::vector<char8_t> goodCases =
	{
		'T', 'e', 'x', 't', ' ', 0x00, 0x7F
	};

	for(char8_t tcase : goodCases)
	{
		ASSERT_TRUE(core::ASCII_Compliant(tcase)) << "Code point " << static_cast<uint32_t>(tcase);
	}

	std::vector<char8_t> badCases =
	{
		0x80, 0xFF
	};

	for(char8_t tcase : badCases)
	{
		ASSERT_FALSE(core::ASCII_Compliant(tcase)) << "Code point " << static_cast<uint32_t>(tcase);
	}
}

TEST(string_encoding, UTF8_UNICODE_Compliant)
{
	std::vector<std::vector<char8_t>> goodCases =
	{
		{'T', 'e', 'x', 't', ' ', 0x7F, 0xC2, 0x80, 0x00, 0xC2, 0xA7, 0xC3, 0xBF, 0xF4, 0x8F, 0xBF, 0xBF},
	};

	for(std::vector<char8_t> const& tcase : goodCases)
	{
		ASSERT_TRUE(core::UTF8_UNICODE_Compliant({tcase.data(), tcase.size()}));
	}

	std::vector<std::vector<char8_t>> badCases =
	{
		{'B', 'a', 'd', ' ', 0xF4, 0x90, 0x80, 0x80},	//out of range
		{'B', 'a', 'd', ' ', 0xED, 0xA0, 0x80},	//unencodable
		{'B', 'a', 'd', ' ', 0xC2, 0x42},	//bad code point
		{'B', 'a', 'd', ' ', 0xC2},	//premature ending
	};

	uintptr_t num_case = 0;
	for(std::vector<char8_t> const& tcase : badCases)
	{
		ASSERT_FALSE(core::UTF8_UNICODE_Compliant({tcase.data(), tcase.size()})) << "Case: " << num_case;
		++num_case;
	}
}

TEST(string_encoding, UTF16_UNICODE_Compliant)
{
	std::vector<std::vector<char16_t>> goodCases =
	{
		{'T', 'e', 'x', 't', ' ', 0xFF, 0xD7FF, 0xE000, 0xD800, 0xDC00, 0xDBFF, 0xDFFF, 0xFFFF},
	};

	for(std::vector<char16_t> const& tcase : goodCases)
	{
		ASSERT_TRUE(core::UTF16_UNICODE_Compliant({tcase.data(), tcase.size()}));
	}

	std::vector<std::vector<char16_t>> badCases =
	{
		{'B', 'a', 'd', ' ', 0xD801, 0x0020	},	//bad code point
		{'B', 'a', 'd', ' ', 0xD801			},	//premature ending
	};

	uintptr_t num_case = 0;
	for(std::vector<char16_t> const& tcase : badCases)
	{
		ASSERT_FALSE(core::UTF16_UNICODE_Compliant({tcase.data(), tcase.size()})) << "Case: " << num_case;
		++num_case;
	}
}

TEST(string_encoding, UCS2_UNICODE_Compliant)
{
	std::vector<std::vector<char16_t>> goodCases =
	{
		{'T', 'e', 'x', 't', ' ', 0xFF, 0xD7FF, 0xE000,  0xFFFF},
	};

	for(std::vector<char16_t> const& tcase : goodCases)
	{
		ASSERT_TRUE(core::UCS2_UNICODE_Compliant({tcase.data(), tcase.size()}));
	}

	std::vector<std::vector<char16_t>> badCases =
	{
		{'B', 'a', 'd', ' ', 0xD801 },	//bad code point
	};

	for(std::vector<char16_t> const& tcase : badCases)
	{
		ASSERT_FALSE(core::UCS2_UNICODE_Compliant({tcase.data(), tcase.size()}));
	}
}

TEST(string_encoding, UCS4_UNICODE_Compliant)
{
	std::vector<std::vector<char32_t>> goodCases =
	{
		{'T', 'e', 'x', 't', ' ', 0xFF, 0xD7FF, 0xE000,  0xFFFF, 0x100000, 0x10FFFF},
	};

	for(std::vector<char32_t> const& tcase : goodCases)
	{
		ASSERT_TRUE(core::UCS4_UNICODE_Compliant({tcase.data(), tcase.size()}));
	}

	std::vector<std::vector<char32_t>> badCases =
	{
		{'B', 'a', 'd', ' ', 0xD801, 0x110000 },
	};

	for(std::vector<char32_t> const& tcase : badCases)
	{
		ASSERT_FALSE(core::UCS4_UNICODE_Compliant({tcase.data(), tcase.size()}));
	}
}

TEST(string_encoding, ASCII_Compliant_uint8_t)
{
	std::vector<std::vector<char8_t>> goodCases =
	{
		{'T', 'e', 'x', 't', ' ', 0x7F},
	};

	for(std::vector<char8_t> const& tcase : goodCases)
	{
		ASSERT_TRUE(core::ASCII_Compliant({tcase.data(), tcase.size()}));
	}

	std::vector<std::vector<char8_t>> badCases =
	{
		{'B', 'a', 'd', ' ', 0x80},
	};

	for(std::vector<char8_t> const& tcase : badCases)
	{
		ASSERT_FALSE(core::ASCII_Compliant({tcase.data(), tcase.size()}));
	}
}

TEST(string_encoding, ASCII_Compliant_uint32_t)
{
	std::vector<std::vector<char32_t>> goodCases =
	{
		{'T', 'e', 'x', 't', ' ', 0x7F},
	};

	for(std::vector<char32_t> const& tcase : goodCases)
	{
		ASSERT_TRUE(core::ASCII_Compliant({tcase.data(), tcase.size()}));
	}

	std::vector<std::vector<char32_t>> badCases =
	{
		{'B', 'a', 'd', ' ', 0x80},
	};

	for(std::vector<char32_t> const& tcase : badCases)
	{
		ASSERT_FALSE(core::ASCII_Compliant({tcase.data(), tcase.size()}));
	}
}

} //namespace text_formating
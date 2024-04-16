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

#include <vector>
#include <utility>

#include <ostream>
#include <string>

#include <CoreLib/string/core_string_misc.hpp>
#include <CoreLib/toPrint/toPrint_std_ostream.hpp>
#include <CoreLib/toPrint/toPrint_encoders.hpp>

using core::toPrint;

#include <gtest/gtest.h>

namespace text_misc
{

TEST(string_misc, toLowerCaseX)
{
	std::vector<std::pair<std::u8string, std::u8string>> testCases =
	{
		{u8"Some RandOM text! wiTh miSC ChaRaCTErs aNd !34#$%", u8"some random text! with misc characters and !34#$%"},
	};

	for(std::pair<std::u8string, std::u8string> const& tcase : testCases)
	{
		std::u8string result = core::toLowerCaseX(tcase.first);
		ASSERT_EQ(result, tcase.second) << "Case \"" << toPrint{tcase.second} << '\"';
	}
}


TEST(string_misc, toUpperCaseX)
{
	std::vector<std::pair<std::u8string, std::u8string>> testCases =
	{
		{u8"Some RandOM text! wiTh miSC ChaRaCTErs aNd !34#$%", u8"SOME RANDOM TEXT! WITH MISC CHARACTERS AND !34#$%"},
	};

	for(std::pair<std::u8string, std::u8string> const& tcase : testCases)
	{
		std::u8string result = core::toUpperCaseX(tcase.first);
		ASSERT_EQ(result, tcase.second) << "Case \"" << toPrint{tcase.second} << '\"';
	}
}


TEST(string_misc, compareNoCase)
{
	std::vector<std::pair<std::u8string, std::u8string>> goodCases =
	{
		{u8"Some RandOM text! wiTh miSC ChaRaCTErs aNd !34#$%", u8"soMe rANDom teXt! wiTh miSC cHaRACteRS AnD !34#$%"},
	};

	std::vector<std::pair<std::u8string, std::u8string>> badCases =
	{
		{u8"Some RandOM text! wiTh miSC ChaRaCTErs aNd !34#$%", u8"s0m3 rand0m t3xt! w1th m1sc characters and !34#$%"},
	};

	uintptr_t auxI = 0;
	for(std::pair<std::u8string, std::u8string> const& tcase : goodCases)
	{
		ASSERT_TRUE(core::compareNoCase(tcase.first, tcase.second)) << "Case " << auxI;
		++auxI;
	}

	auxI = 0;
	for(std::pair<std::u8string, std::u8string> const& tcase : badCases)
	{
		ASSERT_FALSE(core::compareNoCase(tcase.first, tcase.second)) << "Case " << auxI;
		++auxI;
	}
}


TEST(string_misc, string_star_match)
{
	std::vector<std::pair<std::u8string, std::vector<std::u8string>>> goodCases =
	{
		{u8"" , {u8""}},
		{u8"*" , {u8"Anything goes", u8"This as well", u8""}},
		{u8"starts*with*" , {u8"starts something with ending", u8"startswith ending", u8"startswith"}},
		{u8"*ends*with" , {u8"starts ends something with", u8"ends something with", u8"endswith"}},
		{u8"starts*ends*with" , {u8"starts something ends something with", u8"startsendswith"}},
		{u8"*complicated*pattern*" , {u8"must pattern match complicated pattern something with"}},
	};

	std::vector<std::pair<std::u8string, std::vector<std::u8string>>> badCases =
	{
		{u8"" , {u8"something"}},
		{u8"starts*with*" , {u8"doesn't starts something with ending", u8"starss with"}},
		{u8"*ends*with" , {u8"starts ends something with ends", u8"with ends"}},
		{u8"starts*ends*with" , {u8"not starts something ends something with", u8"starts something ends something with not"}},
		{u8"*some*complicated*pattern*" , {u8"must some pattern match complicated some with"}},
	};


	for(std::pair<std::u8string, std::vector<std::u8string>> const& pattern : goodCases)
	{
		for(std::u8string const& tcase : pattern.second)
		{
			ASSERT_TRUE(core::string_star_match(tcase, pattern.first)) << "Case \"" << toPrint{pattern.first} << "\" and \"" << toPrint{tcase} << "\"";
		}
	}

	for(std::pair<std::u8string, std::vector<std::u8string>> const& pattern : badCases)
	{
		for(std::u8string const& tcase : pattern.second)
		{
			ASSERT_FALSE(core::string_star_match(tcase, pattern.first)) << "Case \"" << toPrint{pattern.first} << "\" and \"" << toPrint{tcase} << "\"";
		}
	}
}

} //namespace text_formating

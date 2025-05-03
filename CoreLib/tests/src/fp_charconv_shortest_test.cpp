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



#include <charconv>
#include <array>
#include <random>
#include <limits>
#include <bit>

#include <CoreLib/toPrint/toPrint.hpp>
#include <CoreLib/toPrint/toPrint_std_ostream.hpp>
#include <CoreLib/string/core_fp_charconv.hpp>


#include <CoreLib/core_type.hpp>

using ::core::literals::operator "" _ui32;

//======== ======== ======== Stream helper ======== ======== ========



#include <gtest/gtest.h>


static uint32_t make_valid_fp(uint32_t tcase)
{
	tcase &= 0x7FFFFFFF_ui32;
	if(tcase == 0)
	{
		return 1;
	}
	if((tcase & 0x7F800000_ui32) == 0x7F800000_ui32)
	{
		tcase ^= 0x70000000_ui32;
	}

	return tcase;
}


TEST(fp_charconv, round_trip)
{
	std::random_device rd;  //Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
	std::uniform_int_distribution<uint32_t> distrib(0, std::numeric_limits<uint32_t>::max());

	std::array fix_cases
	{
		0x1_ui32,
		0x2_ui32,
		0x3_ui32,
		0x4_ui32,
		0x7F7FFFFF_ui32,
		0x3F800000_ui32,
		0x3DCCCCCD_ui32,
		0x4CBC76AC_ui32,
		0x4CBC76AB_ui32,
		0x4E313FD4_ui32,
	};

	core::fp_to_chars_shortest_context<float32_t> context;

	std::array<char8_t, 1024> buff;


	for(uint32_t const tcase : fix_cases)
	{
		float32_t const f_case = std::bit_cast<float32_t const>(tcase);

		core::fp_base_classify const classification = core::to_chars_shortest_classify(f_case, context);
		core::fp_to_chars_sci_size const size = core::to_chars_shortest_sci_size(context);

		char8_t* const units = buff.data();
		char8_t* const decimal = units + 1;
		uint16_t const decimal_size = size.mantissa_decimal_size;
		char8_t* const exp = decimal + decimal_size;
		uint16_t const exp_size = size.exponent_size;

		core::to_chars_shortest_sci_unsafe(context, units, decimal);
		core::to_chars_shortest_sci_exp_unsafe(context, exp);

		core::from_chars_result<float32_t> const result = core::from_chars_fp<float32_t>(
			classification.is_negative,
			std::u8string_view(units, 1),
			std::u8string_view(decimal, decimal_size),
			size.is_exp_negative,
			std::u8string_view(exp, exp_size));

		ASSERT_TRUE(result.has_value());

		uint32_t const round_trip = std::bit_cast<uint32_t const>(result.value());
		ASSERT_EQ(round_trip, tcase) << core::toPrint_hex_fix{tcase};
	}

	for(uint16_t i = 0; i < 255; ++i)
	{
		uint32_t const tcase = make_valid_fp(distrib(gen));
		float32_t const f_case = std::bit_cast<float32_t const>(tcase);

		core::fp_base_classify classification = core::to_chars_shortest_classify(f_case, context);
		core::fp_to_chars_sci_size size = core::to_chars_shortest_sci_size(context);

		char8_t* const units = buff.data();
		char8_t* const decimal = units + 1;
		uint16_t const decimal_size = size.mantissa_decimal_size;
		char8_t* const exp = decimal + decimal_size;
		uint16_t const exp_size = size.exponent_size;

		core::to_chars_shortest_sci_unsafe(context, units, decimal);
		core::to_chars_shortest_sci_exp_unsafe(context, exp);

		core::from_chars_result<float32_t> const result = core::from_chars_fp<float32_t>(
			classification.is_negative,
			std::u8string_view(units, 1),
			std::u8string_view(decimal, decimal_size),
			size.is_exp_negative,
			std::u8string_view(exp, exp_size));

		ASSERT_TRUE(result.has_value());

		uint32_t const round_trip = std::bit_cast<uint32_t const>(result.value());
		ASSERT_EQ(round_trip, tcase) << core::toPrint(i) << core::toPrint_hex_fix{tcase};
	}

}

/*
template<typename T>
class float_char_conv_T : public testing::Test {
protected:
	float_char_conv_T() {}
};

using floatTypes = ::testing::Types<
	float32_t,
	float64_t
>;


TYPED_TEST_SUITE(float_char_conv_T, floatTypes);
*/

using ::core::literals::operator "" _ui32;

TEST(fp_charconv, special_cases)
{
	struct TestCase
	{
		std::u8string_view units;
		std::u8string_view decimals;
		std::u8string_view exp;
		bool sign;
		bool exp_sign;
		float32_t expected;
	};

	std::array const fix_cases
	{
		TestCase{ .units = u8"0"sv, .decimals = u8"0"sv, .exp=u8"2789"sv, .sign = false, .exp_sign = false, .expected = 0.f },
		TestCase{ .units = u8"1"sv, .decimals = u8"0"sv, .exp=u8"2789"sv, .sign = false, .exp_sign = true , .expected = 0.f },
		TestCase{ .units = u8"1"sv, .decimals = u8"0"sv, .exp=u8"2789"sv, .sign = false, .exp_sign = false, .expected = std::bit_cast<float>(0x7F800000_ui32) },
	};

	for( const TestCase& tcase: fix_cases )
	{
		core::from_chars_result<float32_t> const result = core::from_chars_fp<float32_t>(
			tcase.sign,
			tcase.units,
			tcase.decimals,
			tcase.exp_sign,
			tcase.exp);

		ASSERT_TRUE(result.has_value());
		ASSERT_EQ(tcase.expected, result.value());
	}
}






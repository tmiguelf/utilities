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



#include <charconv>
#include <array>
#include <random>
#include <limits>

#include <CoreLib/toPrint/toPrint.hpp>
#include <CoreLib/toPrint/toPrint_std_ostream.hpp>
#include <CoreLib/string/core_fp_charconv.hpp>


#include <CoreLib/Core_Type.hpp>

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

	core::fp_to_chars_shortest_context<float> context;

	std::array<char8_t, 1024> buff;


	for(const uint32_t tcase : fix_cases)
	{
		const float f_case = reinterpret_cast<const float&>(tcase);

		const core::fp_base_classify classification = core::to_chars_shortest_classify(f_case, context);
		const core::fp_to_chars_sci_size size = core::to_chars_shortest_sci_size(context);

		char8_t* const units = buff.data();
		char8_t* const decimal = units + 1;
		const uint16_t decimal_size = size.mantissa_decimal_size;
		char8_t* const exp = decimal + decimal_size;
		const uint16_t exp_size = size.exponent_size;

		core::to_chars_shortest_sci_unsafe(context, units, decimal);
		core::to_chars_shortest_sci_exp_unsafe(context, exp);

		float new_float = 0.0;
		const bool result = core::from_chars(
			classification.is_negative,
			std::u8string_view(units, 1),
			std::u8string_view(decimal, decimal_size),
			size.is_exp_negative,
			std::u8string_view(exp, exp_size),
			new_float);

		ASSERT_TRUE(result);

		const uint32_t round_trip = reinterpret_cast<const uint32_t&>(new_float);
		ASSERT_EQ(round_trip, tcase) << core::toPrint_hex_fix{tcase};
	}

	for(uint16_t i = 0; i < 255; ++i)
	{
		const uint32_t tcase = make_valid_fp(distrib(gen));
		const float f_case = reinterpret_cast<const float&>(tcase);

		const core::fp_base_classify classification = core::to_chars_shortest_classify(f_case, context);
		const core::fp_to_chars_sci_size size = core::to_chars_shortest_sci_size(context);

		char8_t* const units = buff.data();
		char8_t* const decimal = units + 1;
		const uint16_t decimal_size = size.mantissa_decimal_size;
		char8_t* const exp = decimal + decimal_size;
		const uint16_t exp_size = size.exponent_size;

		core::to_chars_shortest_sci_unsafe(context, units, decimal);
		core::to_chars_shortest_sci_exp_unsafe(context, exp);

		float new_float = 0.0;
		const bool result = core::from_chars(
			classification.is_negative,
			std::u8string_view(units, 1),
			std::u8string_view(decimal, decimal_size),
			size.is_exp_negative,
			std::u8string_view(exp, exp_size),
			new_float);

		ASSERT_TRUE(result);

		const uint32_t round_trip = reinterpret_cast<const uint32_t&>(new_float);
		ASSERT_EQ(round_trip, tcase) << core::toPrint(i) << core::toPrint_hex_fix{tcase};
	}

}

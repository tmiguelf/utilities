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

//======== Headers ========
#include <ostream>

#include <string_view>
#include <string>
#include <vector>
#include <limits>

#include <CoreLib/string/core_string_numeric.hpp>

#include <MathLib/HyperComplex/quaternions.hpp>

#include <CoreLib/toPrint/toPrint_encoders.hpp>
#include <CoreLib/toPrint/toPrint_std_ostream.hpp>
#include <CoreLib/toPrint/toPrint_base.hpp>

template<typename T>
static uintptr_t to_chars_quat_estimate(mathlib::Quaternion<T> const& p_data)
{
	constexpr std::string_view filler = "[; i; j; k]";

	return
		filler.size() +
		core::to_chars_size(p_data.r()) +
		core::to_chars_size(p_data.i()) +
		core::to_chars_size(p_data.j()) +
		core::to_chars_size(p_data.k());
}

template<typename T>
static uintptr_t to_chars_quat_unsafe(mathlib::Quaternion<T> const& p_data, char8_t* p_out)
{
	constexpr uintptr_t max_size = core::to_chars_dec_max_size_v<T>;
	char8_t* const begin = p_out;
	*(p_out++) = u8'[';
	p_out += core::_p::to_chars(p_data.r(), std::span<char8_t, max_size>{p_out, max_size});
	*(p_out++) = u8';';
	*(p_out++) = u8' ';
	p_out += core::_p::to_chars(p_data.i(), std::span<char8_t, max_size>{p_out, max_size});
	*(p_out++) = u8'i';
	*(p_out++) = u8';';
	*(p_out++) = u8' ';
	p_out += core::_p::to_chars(p_data.j(), std::span<char8_t, max_size>{p_out, max_size});
	*(p_out++) = u8'j';
	*(p_out++) = u8';';
	*(p_out++) = u8' ';
	p_out += core::_p::to_chars(p_data.k(), std::span<char8_t, max_size>{p_out, max_size});
	*(p_out++) = u8'k';
	*(p_out++) = u8']';
	return p_out - begin;
}




template<typename T>
class core::toPrint<mathlib::Quaternion<T>>: public core::toPrint_base
{
public:
	toPrint(mathlib::Quaternion<T> const& p_data): m_data(p_data) {}

	uintptr_t size(char8_t const&) const
	{
		return to_chars_quat_estimate(m_data);
	}

	void get_print(char8_t* p_out) const
	{
		to_chars_quat_unsafe(m_data, p_out);
	}

private:
	mathlib::Quaternion<T> const& m_data;
};



using core::toPrint;

//======== Delayed include headers ========

//For some reason if <gtest/gtest.h> is included before the operator << definitions, template resolution will fail
#include <gtest/gtest.h>

namespace mathlib
{

//======== ======== ======== Typed test suit  ======== ======== ========

template<typename T>
class Quaternion_T : public testing::Test {
protected:
	Quaternion_T() {}
};

using TestTypes = ::testing::Types<float, double/*, long double*/>;
TYPED_TEST_SUITE(Quaternion_T, TestTypes);


//======== Actual tests ========

TYPED_TEST(Quaternion_T, Getters)
{
	using real_t = TypeParam;

	struct TestCase
	{
		Quaternion<real_t> const m_testObj;
		real_t const m_r;
		real_t const m_i;
		real_t const m_j;
		real_t const m_k;
	};

	struct CasePrint: public core::toPrint_base
	{
	public:
		CasePrint(TestCase const& p_data): m_data{p_data}{}

		uintptr_t size(char8_t const&) const
		{
			constexpr std::string_view filler = "[; i; j; k]";

			return
				filler.size() +
				core::to_chars_size(m_data.m_r) +
				core::to_chars_size(m_data.m_i) +
				core::to_chars_size(m_data.m_j) +
				core::to_chars_size(m_data.m_k);
		}

		void get_print(char8_t* p_out) const
		{
			constexpr uintptr_t max_size = core::to_chars_dec_max_size_v<real_t>;
			*(p_out++) = u8'[';
			p_out += core::_p::to_chars(m_data.m_r, std::span<char8_t, max_size>{p_out, max_size});
			*(p_out++) = u8';';
			*(p_out++) = u8' ';
			p_out += core::_p::to_chars(m_data.m_i, std::span<char8_t, max_size>{p_out, max_size});
			*(p_out++) = u8'i';
			*(p_out++) = u8';';
			*(p_out++) = u8' ';
			p_out += core::_p::to_chars(m_data.m_j, std::span<char8_t, max_size>{p_out, max_size});
			*(p_out++) = u8'j';
			*(p_out++) = u8';';
			*(p_out++) = u8' ';
			p_out += core::_p::to_chars(m_data.m_k, std::span<char8_t, max_size>{p_out, max_size});
			*(p_out++) = u8'k';
			*(p_out) = u8']';
		}
	private:
		TestCase const& m_data;
	};

//saves all that work of setting up and type casting
#define TESTCASE_2(R, I, J, K) TestCase{{R, I, J, K}, R, I, J, K}
#define TESTCASE(R, I, J, K) TESTCASE_2(static_cast<real_t>(R), static_cast<real_t>(I), static_cast<real_t>(J), static_cast<real_t>(K))
	std::vector<TestCase> const testData =
		{
			TESTCASE(0.0, 0.0, 0.0, 0.0),
			TESTCASE(1.0, 0.0, 0.0, 0.0),
			TESTCASE(0.0, 1.0, 0.0, 0.0),
			TESTCASE(0.0, 0.0, 1.0, 0.0),
			TESTCASE(0.0, 0.0, 0.0, 1.0),
			TESTCASE(1.0, 1.0, 1.0, 1.0),
			TESTCASE(1.0, 2.0, 3.0, 4.0),
			TESTCASE(8.0, -7.0, 6.0, 5.0),
			TESTCASE(9.10, 13.14, -11.12, 15.16),
			TESTCASE(21.22, -17.18, 23.24, -19.20),
			TESTCASE(-31.32, 29.30, 25.26, -27.28),
		};
#undef TESTCASE
#undef TESTCASE_2

	for(TestCase const& tcase: testData)
	{
		ASSERT_EQ(tcase.m_testObj.r(), tcase.m_r) << CasePrint{tcase};
		ASSERT_EQ(tcase.m_testObj.i(), tcase.m_i) << CasePrint{tcase};
		ASSERT_EQ(tcase.m_testObj.j(), tcase.m_j) << CasePrint{tcase};
		ASSERT_EQ(tcase.m_testObj.k(), tcase.m_k) << CasePrint{tcase};
	}
}

TYPED_TEST(Quaternion_T, Setters)
{
	using real_t = TypeParam;

	struct TestCase
	{
		real_t const m_r;
		real_t const m_i;
		real_t const m_j;
		real_t const m_k;
	};

	struct CasePrint: public core::toPrint_base
	{
	public:
		CasePrint(TestCase const& p_data): m_data{p_data}{}

		uintptr_t size(char8_t const&) const
		{
			constexpr std::string_view filler = "[; i; j; k]";

			return
				filler.size() +
				core::to_chars_size(m_data.m_r) +
				core::to_chars_size(m_data.m_i) +
				core::to_chars_size(m_data.m_j) +
				core::to_chars_size(m_data.m_k);
		}

		void get_print(char8_t* p_out) const
		{
			constexpr uintptr_t max_size = core::to_chars_dec_max_size_v<real_t>;
			*(p_out++) = u8'[';
			p_out += core::_p::to_chars(m_data.m_r, std::span<char8_t, max_size>{p_out, max_size});
			*(p_out++) = u8';';
			*(p_out++) = u8' ';
			p_out += core::_p::to_chars(m_data.m_i, std::span<char8_t, max_size>{p_out, max_size});
			*(p_out++) = u8'i';
			*(p_out++) = u8';';
			*(p_out++) = u8' ';
			p_out += core::_p::to_chars(m_data.m_j, std::span<char8_t, max_size>{p_out, max_size});
			*(p_out++) = u8'j';
			*(p_out++) = u8';';
			*(p_out++) = u8' ';
			p_out += core::_p::to_chars(m_data.m_k, std::span<char8_t, max_size>{p_out, max_size});
			*(p_out++) = u8'k';
			*(p_out) = u8']';
		}
	private:
		TestCase const& m_data;
	};

	//saves all that work of setting up and type casting
#define TESTCASE(R, I, J, K) {static_cast<real_t>(R), static_cast<real_t>(I), static_cast<real_t>(J), static_cast<real_t>(K)}
	std::vector<TestCase> const testData =
	{
		TESTCASE(0.0, 0.0, 0.0, 0.0),
		TESTCASE(1.0, 0.0, 0.0, 0.0),
		TESTCASE(0.0, 1.0, 0.0, 0.0),
		TESTCASE(0.0, 0.0, 1.0, 0.0),
		TESTCASE(0.0, 0.0, 0.0, 1.0),
		TESTCASE(1.0, 1.0, 1.0, 1.0),
		TESTCASE(1.0, 2.0, 3.0, 4.0),
		TESTCASE(8.0, -7.0, 6.0, 5.0),
		TESTCASE(9.10, 13.14, -11.12, 15.16),
		TESTCASE(21.22, -17.18, 23.24, -19.20),
		TESTCASE(-31.32, 29.30, 25.26, -27.28),
	};
#undef TESTCASE

	std::vector<real_t> const testDataR =
	{
		static_cast<real_t>( 0.0 ),
		static_cast<real_t>( 1.0 ),
		static_cast<real_t>( 2.0 ),
		static_cast<real_t>( 3.0 ),
		static_cast<real_t>( 4.0 ),
		static_cast<real_t>( 5.0 ),
		static_cast<real_t>( 6.0 ),
		static_cast<real_t>( 7.0 ),
		static_cast<real_t>( 8.0 ),
		static_cast<real_t>( 9.10),
		static_cast<real_t>(11.12),
		static_cast<real_t>(13.14), 
		static_cast<real_t>(15.16),
		static_cast<real_t>(17.18),
		static_cast<real_t>(19.20),
		static_cast<real_t>(21.22),
		static_cast<real_t>(23.24),
		static_cast<real_t>(25.26),
		static_cast<real_t>(27.28),
		static_cast<real_t>(29.30),
		static_cast<real_t>(31.32),
	};

	//all test
	for(TestCase const& tcase: testData)
	{
		Quaternion<real_t> testObj;
		testObj.set(tcase.m_r, tcase.m_i, tcase.m_j, tcase.m_k);
		ASSERT_EQ(testObj.r(), tcase.m_r) << "All set " << CasePrint{tcase};
		ASSERT_EQ(testObj.i(), tcase.m_i) << "All set " << CasePrint{tcase};
		ASSERT_EQ(testObj.j(), tcase.m_j) << "All set " << CasePrint{tcase};
		ASSERT_EQ(testObj.k(), tcase.m_k) << "All set " << CasePrint{tcase};
	}

	//individual test
	for(real_t const& tcase: testDataR)
	{
		Quaternion<real_t> testR{-41.0, 42.0, -43.0, 44.0};
		Quaternion<real_t> testI{-41.0, 42.0, -43.0, 44.0};
		Quaternion<real_t> testJ{-41.0, 42.0, -43.0, 44.0};
		Quaternion<real_t> testK{-41.0, 42.0, -43.0, 44.0};
		testR.setR(tcase);
		testI.setI(tcase);
		testJ.setJ(tcase); 
		testK.setK(tcase);

		//independence test
		ASSERT_EQ(testR, (Quaternion<real_t>{tcase, 42.0, -43.0, 44.0} )) << toPrint{tcase};
		ASSERT_EQ(testI, (Quaternion<real_t>{-41.0, tcase, -43.0, 44.0})) << toPrint{tcase};
		ASSERT_EQ(testJ, (Quaternion<real_t>{-41.0, 42.0, tcase, 44.0} )) << toPrint{tcase};
		ASSERT_EQ(testK, (Quaternion<real_t>{-41.0, 42.0, -43.0, tcase})) << toPrint{tcase};

		//negated test
		testR.setR(-tcase);
		testI.setI(-tcase);
		testJ.setJ(-tcase); 
		testK.setK(-tcase);

		ASSERT_EQ(testR, (Quaternion<real_t>{-tcase, 42.0, -43.0, 44.0} )) << toPrint{tcase};
		ASSERT_EQ(testI, (Quaternion<real_t>{-41.0, -tcase, -43.0, 44.0})) << toPrint{tcase};
		ASSERT_EQ(testJ, (Quaternion<real_t>{-41.0, 42.0, -tcase, 44.0} )) << toPrint{tcase};
		ASSERT_EQ(testK, (Quaternion<real_t>{-41.0, 42.0, -43.0, -tcase})) << toPrint{tcase};
	}
}

TYPED_TEST(Quaternion_T, Comparison)
{
	using real_t = TypeParam;
	using TestCase = Quaternion<real_t>;

#define TESTCASE(R, I, J, K) {static_cast<real_t>(R), static_cast<real_t>(I), static_cast<real_t>(J), static_cast<real_t>(K)}
	std::vector<TestCase> const testData =
		{
			TESTCASE(0.0, 0.0, 0.0, 0.0),
			TESTCASE(1.0, 0.0, 0.0, 0.0),
			TESTCASE(0.0, 1.0, 0.0, 0.0),
			TESTCASE(0.0, 0.0, 1.0, 0.0),
			TESTCASE(0.0, 0.0, 0.0, 1.0),
			TESTCASE(1.0, 1.0, 1.0, 1.0),
			TESTCASE(1.0, 2.0, 3.0, 4.0),
			TESTCASE(8.0, -7.0, 6.0, 5.0),
			TESTCASE(9.10, 13.14, -11.12, 15.16),
			TESTCASE(21.22, -17.18, 23.24, -19.20),
			TESTCASE(-31.32, 29.30, 25.26, -27.28),
		};
#undef TESTCASE

	uintptr_t const caseSize = testData.size();

	for(uintptr_t i = 0; i < caseSize; ++i)
	{
		//before case
		for(uintptr_t j = 0; j < i; ++j)
		{
			ASSERT_FALSE(testData[i] == testData[j]) << "Case " << toPrint{testData[i]} << " == " << toPrint{testData[j]};
			ASSERT_TRUE (testData[i] != testData[j]) << "Case " << toPrint{testData[i]} << " != " << toPrint{testData[j]};
		}

		ASSERT_TRUE (testData[i] == testData[i]) << "Case " << toPrint{testData[i]} << " == self";
		ASSERT_FALSE(testData[i] != testData[i]) << "Case " << toPrint{testData[i]} << " != self";

		for(uintptr_t j = i + 1; j < caseSize; ++j)
		{
			ASSERT_FALSE(testData[i] == testData[j]) << "Case " << toPrint{testData[i]} << " == " << toPrint{testData[j]};
			ASSERT_TRUE (testData[i] != testData[j]) << "Case " << toPrint{testData[i]} << " != " << toPrint{testData[j]};
		}
	}
}

TYPED_TEST(Quaternion_T, Operator_unary_minus)
{
	using real_t = TypeParam;

	struct TestCase
	{
		Quaternion<real_t> const m_sideA;
		Quaternion<real_t> const m_sideB;
	};

	struct CasePrint: public core::toPrint_base
	{
	public:
		CasePrint(TestCase const& p_data): m_data{p_data}{}

		uintptr_t size(char8_t const&) const
		{
			return to_chars_quat_estimate(m_data.m_sideA) + to_chars_quat_estimate(m_data.m_sideB) + 1;
		}

		void get_print(char8_t* p_out) const
		{
			p_out += to_chars_quat_unsafe(m_data.m_sideA, p_out);
			*(p_out++) = ' ';
			to_chars_quat_unsafe(m_data.m_sideB, p_out);
		}
	private:
		TestCase const& m_data;
	};



#define TESTCASE_D(R, I, J, K) {{R, I, J, K}, {-R, -I, -J, -K}}
#define TESTCASE(R, I, J, K) TESTCASE_D(static_cast<real_t>(R), static_cast<real_t>(I), static_cast<real_t>(J), static_cast<real_t>(K))
	std::vector<TestCase> const testData =
		{
			TESTCASE(0.0, 0.0, 0.0, 0.0),
			TESTCASE(1.0, 0.0, 0.0, 0.0),
			TESTCASE(0.0, 1.0, 0.0, 0.0),
			TESTCASE(0.0, 0.0, 1.0, 0.0),
			TESTCASE(0.0, 0.0, 0.0, 1.0),
			TESTCASE(1.0, 1.0, 1.0, 1.0),
			TESTCASE(1.0, 2.0, 3.0, 4.0),
			TESTCASE(8.0, -7.0, 6.0, 5.0),
			TESTCASE(9.10, 13.14, -11.12, 15.16),
			TESTCASE(21.22, -17.18, 23.24, -19.20),
			TESTCASE(-31.32, 29.30, 25.26, -27.28),
		};
#undef TESTCASE
#undef TESTCASE_D

	for(TestCase const& tcase: testData)
	{
		ASSERT_EQ(-tcase.m_sideA, tcase.m_sideB) << CasePrint{tcase};
		ASSERT_EQ(tcase.m_sideA, -tcase.m_sideB) << CasePrint{tcase};
	}
}

TYPED_TEST(Quaternion_T, Operator_add)
{
	using real_t = TypeParam;

	struct TestCase
	{
		Quaternion<real_t> const m_A;
		Quaternion<real_t> const m_B;
		Quaternion<real_t> const m_sum;
	};

	struct CasePrint: public core::toPrint_base
	{
	public:
		CasePrint(TestCase const& p_data): m_data{p_data}{}

		uintptr_t size(char8_t const&) const
		{
			return to_chars_quat_estimate(m_data.m_A) + to_chars_quat_estimate(m_data.m_B) + 1;
		}

		void get_print(char8_t* p_out) const
		{
			p_out += to_chars_quat_unsafe(m_data.m_A, p_out);
			*(p_out++) = ' ';
			to_chars_quat_unsafe(m_data.m_B, p_out);
		}
	private:
		TestCase const& m_data;
	};


#define TESTCASE_R(R1, I1, J1, K1, R2, I2, J2, K2) {{R1, I1, J1, K1}, {R2, I2, J2, K2}, {(R1 + R2), (I1 + I2), (J1 + J2), (K1 + K2)}} 
#define TESTCASE(R1, I1, J1, K1, R2, I2, J2, K2) TESTCASE_R(static_cast<real_t>(R1), static_cast<real_t>(I1), static_cast<real_t>(J1), static_cast<real_t>(K1), \
	static_cast<real_t>(R2), static_cast<real_t>(I2), static_cast<real_t>(J2), static_cast<real_t>(K2))
	std::vector<TestCase> const testData =
		{
			TESTCASE(0.0, 0.0, 0.0, 0.0,	0.0, 0.0, 0.0, 0.0),
			TESTCASE(1.0, 0.0, 0.0, 0.0,	0.0, 0.0, 0.0, 0.0),
			TESTCASE(0.0, 1.0, 0.0, 0.0,	0.0, 0.0, 0.0, 0.0),
			TESTCASE(0.0, 0.0, 1.0, 0.0,	0.0, 0.0, 0.0, 0.0),
			TESTCASE(0.0, 0.0, 0.0, 1.0,	0.0, 0.0, 0.0, 0.0),
			TESTCASE(1.2, 3.4, 5.6, 7.8,	9.10, 11.12, 13.14, 15.16),
			TESTCASE(-1.2, 3.4, -5.6, 7.8,	13.14, 9.10, -15.16, -11.12),
		};
#undef TESTCASE_R
#undef TESTCASE

	//+
	for(TestCase const& tcase: testData)
	{
		ASSERT_EQ(tcase.m_A + tcase.m_B, tcase.m_sum) << "A + B - " << CasePrint{tcase};
		ASSERT_EQ(tcase.m_B + tcase.m_A, tcase.m_sum) << "B + A - " << CasePrint{tcase};
	}

	//+=
	for(TestCase const& tcase: testData)
	{
		{
			Quaternion<real_t> tval{tcase.m_A};
			tval += tcase.m_B;
			ASSERT_EQ(tval, tcase.m_sum) << "A += B - " << CasePrint{tcase};
		}
		{
			Quaternion<real_t> tval{tcase.m_B};
			tval += tcase.m_A;
			ASSERT_EQ(tval, tcase.m_sum) << "B += A" << CasePrint{tcase};
		}
	}
}

TYPED_TEST(Quaternion_T, Operator_minus)
{
	using real_t = TypeParam;

	struct TestCase
	{
		Quaternion<real_t> const m_A;
		Quaternion<real_t> const m_B;
		Quaternion<real_t> const m_sub;
	};

	struct CasePrint: public core::toPrint_base
	{
	public:
		CasePrint(TestCase const& p_data): m_data{p_data}{}

		uintptr_t size(char8_t const&) const
		{
			return to_chars_quat_estimate(m_data.m_A) + to_chars_quat_estimate(m_data.m_B) + 1;
		}

		void get_print(char8_t* p_out) const
		{
			p_out += to_chars_quat_unsafe(m_data.m_A, p_out);
			*(p_out++) = ' ';
			to_chars_quat_unsafe(m_data.m_B, p_out);
		}
	private:
		TestCase const& m_data;
	};

#define TESTCASE_R(R1, I1, J1, K1, R2, I2, J2, K2) {{R1, I1, J1, K1}, {R2, I2, J2, K2}, {(R1 - R2), (I1 - I2), (J1 - J2), (K1 - K2)}} 
#define TESTCASE(R1, I1, J1, K1, R2, I2, J2, K2) TESTCASE_R(static_cast<real_t>(R1), static_cast<real_t>(I1), static_cast<real_t>(J1), static_cast<real_t>(K1), \
	static_cast<real_t>(R2), static_cast<real_t>(I2), static_cast<real_t>(J2), static_cast<real_t>(K2))
	std::vector<TestCase> const testData =
	{
		TESTCASE(0.0, 0.0, 0.0, 0.0,	0.0, 0.0, 0.0, 0.0),
		TESTCASE(1.0, 0.0, 0.0, 0.0,	0.0, 0.0, 0.0, 0.0),
		TESTCASE(0.0, 1.0, 0.0, 0.0,	0.0, 0.0, 0.0, 0.0),
		TESTCASE(0.0, 0.0, 1.0, 0.0,	0.0, 0.0, 0.0, 0.0),
		TESTCASE(0.0, 0.0, 0.0, 1.0,	0.0, 0.0, 0.0, 0.0),
		TESTCASE(1.2, 3.4, 5.6, 7.8,	9.10, 11.12, 13.14, 15.16),
		TESTCASE(-1.2, 3.4, -5.6, 7.8,	13.14, 9.10, -15.16, -11.12),
	};
#undef TESTCASE_R
#undef TESTCASE

	//-
	for(TestCase const& tcase: testData)
	{
		ASSERT_EQ(tcase.m_A - tcase.m_B, tcase.m_sub) << "A-B - " << CasePrint{tcase};
		ASSERT_EQ(tcase.m_B - tcase.m_A, -tcase.m_sub) << "B-A - " << CasePrint{tcase};
	}

	//-=
	for(TestCase const& tcase: testData)
	{
		{
			Quaternion<real_t> tval{tcase.m_A};
			tval -= tcase.m_B;
			ASSERT_EQ(tval, tcase.m_sub) << "A-=B - " << CasePrint{tcase};
		}
		{
			Quaternion<real_t> tval{tcase.m_B};
			tval -= tcase.m_A;
			ASSERT_EQ(tval, -tcase.m_sub) << "B-=A - " << CasePrint{tcase};
		}
	}
}


TYPED_TEST(Quaternion_T, scalar_multiply)
{
	using real_t = TypeParam;

	struct TestCase
	{
		Quaternion<real_t> const	m_quat;
		real_t const				m_scalar;
		Quaternion<real_t> const	m_result;
	};

	struct CasePrint: public core::toPrint_base
	{
	public:
		CasePrint(TestCase const& p_data): m_data{p_data}{}

		uintptr_t size(char8_t const&) const
		{
			return to_chars_quat_estimate(m_data.m_quat) + core::to_chars_size(m_data.m_scalar) + 3;
		}

		void get_print(char8_t* p_out) const
		{
			p_out += to_chars_quat_unsafe(m_data.m_quat, p_out);
			*(p_out++) = ' ';
			*(p_out++) = 'x';
			*(p_out++) = ' ';
			core::to_chars_unsafe(m_data.m_scalar, p_out);
		}
	private:
		TestCase const& m_data;
	};

#define TESTCASE_R(R, I, J, K, S) {{R, I, J, K}, S, {(R * S), (I * S), (J * S), (K * S)}} 
#define TESTCASE(R, I, J, K, S) TESTCASE_R(static_cast<real_t>(R), static_cast<real_t>(I), static_cast<real_t>(J), static_cast<real_t>(K), static_cast<real_t>(S))
	std::vector<TestCase> const testData =
	{
		TESTCASE(0.0, 0.0, 0.0, 0.0,	0.0),
		TESTCASE(0.0, 0.0, 0.0, 0.0,	1.0),
		TESTCASE(0.0, 0.0, 0.0, 0.0,	-1.0),
		TESTCASE(1.0, 1.0, 1.0, 1.0,	0.0),
		TESTCASE(1.0, 1.0, 1.0, 1.0,	1.0),
		TESTCASE(1.0, 1.0, 1.0, 1.0,	-1.0),
		TESTCASE(1.2, 3.4, 5.6, 7.8,	9.10),
		TESTCASE(-1.2, 3.4, -5.6, 7.8,	-9.10),
		TESTCASE(10.2, 34.4, 55.6, 79.8,	29.10),
		TESTCASE(-11.2, 34.4, -57.6, 71.8,	-30.10),
	};
#undef TESTCASE_R
#undef TESTCASE

	for(TestCase const& tcase: testData)
	{
		ASSERT_EQ(tcase.m_quat * tcase.m_scalar, tcase.m_result) << "* - " << CasePrint{tcase};
	}

	for(TestCase const& tcase: testData)
	{
		Quaternion<real_t> test{tcase.m_quat};
		test *= tcase.m_scalar;
		ASSERT_EQ(test, tcase.m_result) << "*= - " << CasePrint{tcase};
	}

}

TYPED_TEST(Quaternion_T, scalar_division)
{
	using real_t = TypeParam;

	struct TestCase
	{
		Quaternion<real_t> const	m_quat;
		real_t const				m_scalar;
		Quaternion<real_t> const	m_result;
	};

	struct CasePrint: public core::toPrint_base
	{
	public:
		CasePrint(TestCase const& p_data): m_data{p_data}{}

		uintptr_t size(char8_t const&) const
		{
			return to_chars_quat_estimate(m_data.m_quat) + core::to_chars_size(m_data.m_scalar) + 3;
		}

		void get_print(char8_t* p_out) const
		{
			p_out += to_chars_quat_unsafe(m_data.m_quat, p_out);
			*(p_out++) = ' ';
			*(p_out++) = '/';
			*(p_out++) = ' ';
			core::to_chars_unsafe(m_data.m_scalar, p_out);
		}
	private:
		TestCase const& m_data;
	};

#define TESTCASE_R(R, I, J, K, S) {{R, I, J, K}, S, {(R / S), (I / S), (J / S), (K / S)}} 
#define TESTCASE(R, I, J, K, S) TESTCASE_R(static_cast<real_t>(R), static_cast<real_t>(I), static_cast<real_t>(J), static_cast<real_t>(K), static_cast<real_t>(S))
	std::vector<TestCase> const testData =
	{
		TESTCASE(0.0, 0.0, 0.0, 0.0,	0.1),
		TESTCASE(0.0, 0.0, 0.0, 0.0,	1.0),
		TESTCASE(0.0, 0.0, 0.0, 0.0,	-1.0),
		TESTCASE(1.0, 1.0, 1.0, 1.0,	-0.1),
		TESTCASE(1.0, 1.0, 1.0, 1.0,	1.0),
		TESTCASE(1.0, 1.0, 1.0, 1.0,	-1.0),
		TESTCASE(1.2, 3.4, 5.6, 7.8,	9.10),
		TESTCASE(-1.2, 3.4, -5.6, 7.8,	-9.10),
		TESTCASE(10.2, 34.4, 55.6, 79.8,	29.10),
		TESTCASE(-11.2, 34.4, -57.6, 71.8,	-30.10),
	};
#undef TESTCASE_R
#undef TESTCASE

	for(TestCase const& tcase: testData)
	{
		ASSERT_EQ(tcase.m_quat / tcase.m_scalar, tcase.m_result) << "/ - " << CasePrint{tcase};
	}

	for(TestCase const& tcase: testData)
	{
		Quaternion<real_t> test{tcase.m_quat};
		test /= tcase.m_scalar;
		ASSERT_EQ(test, tcase.m_result) << "/= - " << CasePrint{tcase};
	}
}


TYPED_TEST(Quaternion_T, quaternion_multiplication)
{
	using real_t = TypeParam;

	struct TestCase
	{
		Quaternion<real_t> const m_first;
		Quaternion<real_t> const m_second;
		Quaternion<real_t> const m_result;
	};

	struct CasePrint: public core::toPrint_base
	{
	public:
		CasePrint(TestCase const& p_data): m_data{p_data}{}

		uintptr_t size(char8_t const&) const
		{
			return to_chars_quat_estimate(m_data.m_first) + to_chars_quat_estimate(m_data.m_second) + 1;
		}

		void get_print(char8_t* p_out) const
		{
			p_out += to_chars_quat_unsafe(m_data.m_first, p_out);
			*(p_out++) = ' ';
			to_chars_quat_unsafe(m_data.m_second, p_out);
		}
	private:
		TestCase const& m_data;
	};


#define QUAT_T(R, I, J, K) {static_cast<real_t>(R), static_cast<real_t>(I), static_cast<real_t>(J), static_cast<real_t>(K)}
	std::vector<TestCase> const testData =
	{
		//null
		{QUAT_T(0.0, 0.0, 0.0, 0.0), QUAT_T(0.0, 0.0, 0.0, 0.0), QUAT_T(0.0, 0.0, 0.0, 0.0)},
		{QUAT_T(1.0, 2.0, 3.0, 4.0), QUAT_T(0.0, 0.0, 0.0, 0.0), QUAT_T(0.0, 0.0, 0.0, 0.0)},
		{QUAT_T(0.0, 0.0, 0.0, 0.0), QUAT_T(1.0, 2.0, 3.0, 4.0), QUAT_T(0.0, 0.0, 0.0, 0.0)},
		//identities
		{QUAT_T(1.0, 0.0, 0.0, 0.0), QUAT_T(1.0, 0.0, 0.0, 0.0), QUAT_T(1.0, 0.0, 0.0, 0.0)},
		{QUAT_T(1.0, 0.0, 0.0, 0.0), QUAT_T(0.0, 1.0, 0.0, 0.0), QUAT_T(0.0, 1.0, 0.0, 0.0)},
		{QUAT_T(1.0, 0.0, 0.0, 0.0), QUAT_T(0.0, 0.0, 1.0, 0.0), QUAT_T(0.0, 0.0, 1.0, 0.0)},
		{QUAT_T(1.0, 0.0, 0.0, 0.0), QUAT_T(0.0, 0.0, 0.0, 1.0), QUAT_T(0.0, 0.0, 0.0, 1.0)},
		{QUAT_T(0.0, 1.0, 0.0, 0.0), QUAT_T(1.0, 0.0, 0.0, 0.0), QUAT_T(0.0, 1.0, 0.0, 0.0)},
		{QUAT_T(0.0, 1.0, 0.0, 0.0), QUAT_T(0.0, 1.0, 0.0, 0.0), QUAT_T(-1.0, 0.0, 0.0, 0.0)},
		{QUAT_T(0.0, 1.0, 0.0, 0.0), QUAT_T(0.0, 0.0, 1.0, 0.0), QUAT_T(0.0, 0.0, 0.0, 1.0)},
		{QUAT_T(0.0, 1.0, 0.0, 0.0), QUAT_T(0.0, 0.0, 0.0, 1.0), QUAT_T(0.0, 0.0,-1.0, 0.0)},
		{QUAT_T(0.0, 0.0, 1.0, 0.0), QUAT_T(1.0, 0.0, 0.0, 0.0), QUAT_T(0.0, 0.0, 1.0, 0.0)},
		{QUAT_T(0.0, 0.0, 1.0, 0.0), QUAT_T(0.0, 1.0, 0.0, 0.0), QUAT_T(0.0, 0.0, 0.0, -1.0)},
		{QUAT_T(0.0, 0.0, 1.0, 0.0), QUAT_T(0.0, 0.0, 1.0, 0.0), QUAT_T(-1.0, 0.0, 0.0, 0.0)},
		{QUAT_T(0.0, 0.0, 1.0, 0.0), QUAT_T(0.0, 0.0, 0.0, 1.0), QUAT_T(0.0, 1.0, 0.0, 0.0)},
		{QUAT_T(0.0, 0.0, 0.0, 1.0), QUAT_T(1.0, 0.0, 0.0, 0.0), QUAT_T(0.0, 0.0, 0.0, 1.0)},
		{QUAT_T(0.0, 0.0, 0.0, 1.0), QUAT_T(0.0, 1.0, 0.0, 0.0), QUAT_T(0.0, 0.0, 1.0, 0.0)},
		{QUAT_T(0.0, 0.0, 0.0, 1.0), QUAT_T(0.0, 0.0, 1.0, 0.0), QUAT_T(0.0, -1.0, 0.0, 0.0)},
		{QUAT_T(0.0, 0.0, 0.0, 1.0), QUAT_T(0.0, 0.0, 0.0, 1.0), QUAT_T(-1.0, 0.0, 0.0, 0.0)},
		//general
		{QUAT_T(1.0, 2.0, 3.0, 4.0), QUAT_T(5.0, 6.0, 7.0, 8.0), QUAT_T(-60.0, 12, 30.0, 24.0)},
		{QUAT_T(1.25, -3.5, 5.625, -7.75), QUAT_T(-9.0, 11.125, -13.75, 15.5), QUAT_T(225.15625, 26.03125, -99.78125, 74.671875)},
	};
#undef QUAT_T

	constexpr real_t epsilon =  std::numeric_limits<real_t>::epsilon();

	for(TestCase const& tcase: testData)
	{
		Quaternion<real_t> const res = tcase.m_first * tcase.m_second;

		ASSERT_NEAR(static_cast<double>(res.r()), static_cast<double>(tcase.m_result.r()), epsilon) << "* " << CasePrint{tcase};
		ASSERT_NEAR(static_cast<double>(res.i()), static_cast<double>(tcase.m_result.i()), epsilon) << "* " << CasePrint{tcase};
		ASSERT_NEAR(static_cast<double>(res.j()), static_cast<double>(tcase.m_result.j()), epsilon) << "* " << CasePrint{tcase};
		ASSERT_NEAR(static_cast<double>(res.k()), static_cast<double>(tcase.m_result.k()), epsilon) << "* " << CasePrint{tcase};
	}

	for(TestCase const& tcase: testData)
	{
		Quaternion<real_t> res{tcase.m_first};
		res *= tcase.m_second;
		ASSERT_NEAR(static_cast<double>(res.r()), static_cast<double>(tcase.m_result.r()), epsilon) << "*= " << CasePrint{tcase};
		ASSERT_NEAR(static_cast<double>(res.i()), static_cast<double>(tcase.m_result.i()), epsilon) << "*= " << CasePrint{tcase};
		ASSERT_NEAR(static_cast<double>(res.j()), static_cast<double>(tcase.m_result.j()), epsilon) << "*= " << CasePrint{tcase};
		ASSERT_NEAR(static_cast<double>(res.k()), static_cast<double>(tcase.m_result.k()), epsilon) << "*= " << CasePrint{tcase};
	}
}

TYPED_TEST(Quaternion_T, isZero)
{
	using real_t = TypeParam;
	using TestCase = Quaternion<real_t>;

	real_t const infinitesimal	= std::numeric_limits<real_t>::denorm_min();

#define TESTCASE(R, I, J, K) {static_cast<real_t>(R), static_cast<real_t>(I), static_cast<real_t>(J), static_cast<real_t>(K)}
	std::vector<TestCase> const testData =
	{
		TESTCASE(1.0, 0.0, 0.0, 0.0),
		TESTCASE(0.0, 1.0, 0.0, 0.0),
		TESTCASE(0.0, 0.0, 1.0, 0.0),
		TESTCASE(0.0, 0.0, 0.0, 1.0),
		TESTCASE(1.0, 1.0, 1.0, 1.0),
		TESTCASE(1.0, 2.0, 3.0, 4.0),
		TESTCASE(infinitesimal, 0.0, 0.0, 0.0),
	};
#undef TESTCASE

	{
		constexpr Quaternion<real_t> qt0 {real_t{0}, real_t{0}, real_t{0}, real_t{0}};
		ASSERT_TRUE(qt0.isZero()) << toPrint{qt0};
	}

	for(TestCase const& tcase: testData)
	{
		ASSERT_FALSE(tcase.isZero()) << toPrint{tcase};
	}
}

TYPED_TEST(Quaternion_T, norm_squared)
{
	using real_t = TypeParam;

	struct TestCase
	{
		Quaternion<real_t> const m_quat;
		real_t const norm_sqrd;
	};

#define TESTCASE(R, I, J, K, N) {{static_cast<real_t>(R), static_cast<real_t>(I), static_cast<real_t>(J), static_cast<real_t>(K)}, static_cast<real_t>(N)}
	std::vector<TestCase> const testData =
	{
		TESTCASE(0.0, 0.0, 0.0, 0.0,	0.0),
		TESTCASE(1.0, 0.0, 0.0, 0.0,	1.0),
		TESTCASE(0.0, 1.0, 0.0, 0.0,	1.0),
		TESTCASE(0.0, 0.0, 1.0, 0.0,	1.0),
		TESTCASE(0.0, 0.0, 0.0, 1.0,	1.0),
		TESTCASE(-1.0, 0.0, 0.0, 0.0,	1.0),
		TESTCASE(0.0, -1.0, 0.0, 0.0,	1.0),
		TESTCASE(0.0, 0.0, -1.0, 0.0,	1.0),
		TESTCASE(0.0, 0.0, 0.0, -1.0,	1.0),
		TESTCASE(1.0, 1.0, 1.0, 1.0,	4.0),
		TESTCASE(1.0, 1.0, 0.0, 0.0,	2.0),
		TESTCASE(2.0, -3.0, 5.0, -7.0,	87.0),
		TESTCASE(1.25, 2.125, 3.5, 4.0,	34.328125),
	};
#undef TESTCASE

	for(TestCase const& tcase: testData)
	{
		ASSERT_EQ(tcase.m_quat.norm_squared(), tcase.norm_sqrd) << toPrint{tcase.m_quat};
	}
}

TYPED_TEST(Quaternion_T, norm)
{
	using real_t = TypeParam;

	struct TestCase
	{
		Quaternion<real_t> const m_quat;
		real_t const norm;
		real_t const error;
	};

	//manual calculations where not done with more than 12 digits of precision
	constexpr real_t epsilon = std::max(std::numeric_limits<real_t>::epsilon(), static_cast<real_t>(0.000000000001));

	//computations may loose further precision proportional to the size of the number
#define TESTCASE(R, I, J, K, N, E) {{real_t{R}, real_t{I}, real_t{J}, real_t{K}}, static_cast<real_t>(N), static_cast<real_t>(E * N * 3.0)}
	std::vector<TestCase> const testData =
	{
		TESTCASE(0.0, 0.0, 0.0, 0.0,	0.0,	0.0),
		TESTCASE(1.0, 0.0, 0.0, 0.0,	1.0,	0.0),
		TESTCASE(0.0, 1.0, 0.0, 0.0,	1.0,	0.0),
		TESTCASE(0.0, 0.0, 1.0, 0.0,	1.0,	0.0),
		TESTCASE(0.0, 0.0, 0.0, 1.0,	1.0,	0.0),
		TESTCASE(-1.0, 0.0, 0.0, 0.0,	1.0,	0.0),
		TESTCASE(0.0, -1.0, 0.0, 0.0,	1.0,	0.0),
		TESTCASE(0.0, 0.0, -1.0, 0.0,	1.0,	0.0),
		TESTCASE(0.0, 0.0, 0.0, -1.0,	1.0,	0.0),
		TESTCASE(1.0, 1.0, 1.0, 1.0,	2.0,	0.0),
		TESTCASE(1.0, 1.0, 0.0, 0.0,	1.414213562373, epsilon),
		TESTCASE(2.0, -3.0, 5.0, -7.0,	9.327379053089, epsilon),
		TESTCASE(1.25, 2.125, 3.5, 4.0,	5.859020822629, epsilon),
	};
#undef TESTCASE

	for(TestCase const& tcase: testData)
	{
		ASSERT_NEAR(static_cast<double>(tcase.m_quat.norm()), static_cast<double>(tcase.norm), static_cast<double>(tcase.error)) << toPrint{tcase.m_quat};
	}
}

TYPED_TEST(Quaternion_T, renormalized)
{
	using real_t = TypeParam;

	struct TestCase
	{
		Quaternion<real_t> const m_quat;
		Quaternion<real_t> const m_result;
		real_t const m_error;
	};

	//manual calculations where not done with more than 12 digits of precision
	constexpr real_t epsilon = std::max(std::numeric_limits<real_t>::epsilon(), static_cast<real_t>(0.000000000001));
	//computations may loose further precision proportional to the size of the number

#define QUAT_T(R, I, J, K) {static_cast<real_t>(R), static_cast<real_t>(I), static_cast<real_t>(J), static_cast<real_t>(K)}
	std::vector<TestCase> const testData =
	{
		{ QUAT_T(1.0, 0.0, 0.0, 0.0),		QUAT_T(1.0, 0.0, 0.0, 0.0),		real_t{0.0}},
		{ QUAT_T(0.0, 1.0, 0.0, 0.0),		QUAT_T(0.0, 1.0, 0.0, 0.0),		real_t{0.0}},
		{ QUAT_T(0.0, 0.0, 1.0, 0.0),		QUAT_T(0.0, 0.0, 1.0, 0.0),		real_t{0.0}},
		{ QUAT_T(0.0, 0.0, 0.0, 1.0),		QUAT_T(0.0, 0.0, 0.0, 1.0),		real_t{0.0}},
		{ QUAT_T(-1.0, 0.0, 0.0, 0.0),		QUAT_T(-1.0, 0.0, 0.0, 0.0),	real_t{0.0}},
		{ QUAT_T(0.0, -1.0, 0.0, 0.0),		QUAT_T(0.0, -1.0, 0.0, 0.0),	real_t{0.0}},
		{ QUAT_T(0.0, 0.0, -1.0, 0.0),		QUAT_T(0.0, 0.0, -1.0, 0.0),	real_t{0.0}},
		{ QUAT_T(0.0, 0.0, 0.0, -1.0),		QUAT_T(0.0, 0.0, 0.0, -1.0),	real_t{0.0}},
		{ QUAT_T(1.0, 1.0, 1.0, 1.0),		QUAT_T(0.5, 0.5, 0.5, 0.5),		epsilon},
		{ QUAT_T(1.0, 1.0, 0.0, 0.0),		QUAT_T(0.707106781187, 0.707106781187, 0.0, 0.0), epsilon * 3.0},
		{ QUAT_T(2.0, -3.0, 5.0, -7.0),		QUAT_T(0.214422506968, -0.321633760451, 0.536056267419, -0.750478774386), epsilon * 15.0},
		{ QUAT_T(1.25, 2.125, 3.5, 4.0),	QUAT_T(0.213346229317, 0.362688589840, 0.597369442089, 0.682707933816), epsilon * 15.0},
	};
#undef QUAT_T

	{
		Quaternion<real_t> qt0{0.0, 0.0, 0.0, 0.0};
		std::optional<Quaternion<real_t>> val = qt0.renormalized();
		ASSERT_FALSE(val.has_value());
	}

	for(TestCase const& tcase: testData)
	{
		std::optional<Quaternion<real_t>> res = tcase.m_quat.renormalized();
		ASSERT_TRUE(res.has_value());
		Quaternion<real_t> val = res.value();
		ASSERT_NEAR(static_cast<double>(val.r()), static_cast<double>(tcase.m_result.r()), static_cast<double>(tcase.m_error)) << toPrint{tcase.m_quat};
		ASSERT_NEAR(static_cast<double>(val.i()), static_cast<double>(tcase.m_result.i()), static_cast<double>(tcase.m_error)) << toPrint{tcase.m_quat};
		ASSERT_NEAR(static_cast<double>(val.j()), static_cast<double>(tcase.m_result.j()), static_cast<double>(tcase.m_error)) << toPrint{tcase.m_quat};
		ASSERT_NEAR(static_cast<double>(val.k()), static_cast<double>(tcase.m_result.k()), static_cast<double>(tcase.m_error)) << toPrint{tcase.m_quat};
	}
}


TYPED_TEST(Quaternion_T, inverse)
{
	using real_t = TypeParam;

	struct TestCase
	{
		Quaternion<real_t> const m_quat;
		real_t const m_error;
	};

	constexpr real_t epsilon = std::numeric_limits<real_t>::epsilon();

#define QUAT_T(R, I, J, K) {static_cast<real_t>(R), static_cast<real_t>(I), static_cast<real_t>(J), static_cast<real_t>(K)}
	std::vector<TestCase> const testData =
	{
		{ QUAT_T(1.0, 0.0, 0.0, 0.0),		real_t{0.0}},
		{ QUAT_T(0.0, 1.0, 0.0, 0.0),		real_t{0.0}},
		{ QUAT_T(0.0, 0.0, 1.0, 0.0),		real_t{0.0}},
		{ QUAT_T(0.0, 0.0, 0.0, 1.0),		real_t{0.0}},
		{ QUAT_T(-1.0, 0.0, 0.0, 0.0),		real_t{0.0}},
		{ QUAT_T(0.0, -1.0, 0.0, 0.0),		real_t{0.0}},
		{ QUAT_T(0.0, 0.0, -1.0, 0.0),		real_t{0.0}},
		{ QUAT_T(0.0, 0.0, 0.0, -1.0),		real_t{0.0}},
		{ QUAT_T(1.0, 1.0, 1.0, 1.0),		epsilon * 10.0},
		{ QUAT_T(1.0, 1.0, 0.0, 0.0),		epsilon * 10.0},
		{ QUAT_T(2.0, -3.0, 5.0, -7.0),		epsilon * 10.0},
		{ QUAT_T(1.25, 2.125, 3.5, 4.0),	epsilon * 10.0},
	};
#undef QUAT_T

	{
		Quaternion<real_t> qt0{0.0, 0.0, 0.0, 0.0};
		std::optional<Quaternion<real_t>> res = qt0.inverse();
		ASSERT_FALSE(res.has_value());
	}

	for(TestCase const& tcase: testData)
	{
		std::optional<Quaternion<real_t>> res = tcase.m_quat.inverse();
		ASSERT_TRUE(res.has_value());

		Quaternion<real_t> const val = res.value() * tcase.m_quat;

		ASSERT_NEAR(static_cast<double>(val.r()), 1.0, static_cast<double>(tcase.m_error)) << toPrint{tcase.m_quat};
		ASSERT_NEAR(static_cast<double>(val.i()), 0.0, static_cast<double>(tcase.m_error)) << toPrint{tcase.m_quat};
		ASSERT_NEAR(static_cast<double>(val.j()), 0.0, static_cast<double>(tcase.m_error)) << toPrint{tcase.m_quat};
		ASSERT_NEAR(static_cast<double>(val.k()), 0.0, static_cast<double>(tcase.m_error)) << toPrint{tcase.m_quat};
	}
}


TYPED_TEST(Quaternion_T, isFinite)
{
	using real_t = TypeParam;
	using TestCase = Quaternion<real_t>;

	real_t const nan			= std::numeric_limits<real_t>::quiet_NaN();
	real_t const infinity		= std::numeric_limits<real_t>::infinity();
	real_t const infinitesimal	= std::numeric_limits<real_t>::denorm_min();

#define TESTCASE(R, I, J, K) {real_t{R}, real_t{I}, real_t{J}, real_t{K}}
	std::vector<TestCase> const testDataPositive =
	{
		TESTCASE(0.0, 0.0, 0.0, 0.0),
		TESTCASE(1.0, 0.0, 0.0, 0.0),
		TESTCASE(0.0, 1.0, 0.0, 0.0),
		TESTCASE(0.0, 0.0, 1.0, 0.0),
		TESTCASE(0.0, 0.0, 0.0, 1.0),
		TESTCASE(1.0, 1.0, 1.0, 1.0),
		TESTCASE(1.0, 2.0, 3.0, 4.0),
		TESTCASE(infinitesimal, 0.0, 0.0, 0.0),
		TESTCASE(0.0, 0.0, 0.0, infinitesimal),
	};

	std::vector<TestCase> const testDataNegative =
	{
		TESTCASE(infinity, 0.0, 0.0, 0.0),
		TESTCASE(0.0, infinity, 0.0, 0.0),
		TESTCASE(0.0, 0.0, infinity, 0.0),
		TESTCASE(0.0, 0.0, 0.0, infinity),
		TESTCASE(-infinity, 0.0, 0.0, 0.0),
		TESTCASE(0.0, -infinity, 0.0, 0.0),
		TESTCASE(0.0, 0.0, -infinity, 0.0),
		TESTCASE(0.0, 0.0, 0.0, -infinity),
		TESTCASE(nan, 0.0, 0.0, 0.0),
		TESTCASE(0.0, nan, 0.0, 0.0),
		TESTCASE(0.0, 0.0, nan, 0.0),
		TESTCASE(0.0, 0.0, 0.0, nan),
	};
#undef TESTCASE

	for(TestCase const& tcase: testDataPositive)
	{
		ASSERT_TRUE(tcase.isFinite()) << toPrint{tcase};
	}

	for(TestCase const& tcase: testDataNegative)
	{
		ASSERT_FALSE(tcase.isFinite()) << toPrint{tcase};
	}
}

} //namespace mathlib
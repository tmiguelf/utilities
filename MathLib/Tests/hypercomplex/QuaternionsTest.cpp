//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///
///	\author Tiago Freire
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

//======== Headers ========
#include <ostream>
#include <string_view>
#include <string>
#include <vector>
#include <limits>

#include <CoreLib/Core_String.hpp>

#include <MathLib/HyperComplex/quaternions.hpp>

//TODO: Needs to be removed temporarilly
//there's a weird bug in google test where it can not find the operator << with operands std::basic_ostream<char> and const mathlib::Quaternion<double>
//nor being defined, where one clearly is, need to figure out the source of that bug
#ifdef _WIN32

//======== Stream assists ========

using stream_t = std::basic_ostream<char>;

//For some reason if this is not defined before #include <gtest/gtest.h> then template resolution will fail
static stream_t& operator << (stream_t& p_stream, const std::u8string& p_str)
{
	p_stream.write(reinterpret_cast<const char*>(p_str.data()), p_str.size());
	return p_stream;
}

template<typename T>
static stream_t& operator << (stream_t& p_stream, const mathlib::Quaternion<T>& p_data)
{
	p_stream
		<< "["
		<< core::to_chars<char8_t>(p_data.r()) << "; "
		<< core::to_chars<char8_t>(p_data.i()) << "i; "
		<< core::to_chars<char8_t>(p_data.j()) << "j; "
		<< core::to_chars<char8_t>(p_data.k()) << "k]";
	return p_stream;
}

/// \brief used to allow streaming using "operator <<" for classess defined in function body
template<class T>
struct toStream
{
public:
	toStream(const T& p_obj): m_obj(p_obj) {}
	void stream(stream_t& p_stream) const { m_obj.stream(p_stream); }

private:
	const T& m_obj;
};

template<typename T>
static stream_t& operator << (stream_t& p_stream, const toStream<T>& p_data)
{
	p_data.stream(p_stream);
	return p_stream;
}


//======== Delayed include headers ========

//For some reason if <gtest/gtest.h> is included before the operator << definitions, template resolution will fail
#include <gtest/gtest.h>

//======== ======== ======== Typed test suit  ======== ======== ========

template<typename T>
class Quaternion_T : public testing::Test {
protected:
	Quaternion_T() {}
};

using TestTypes = ::testing::Types<float, double, long double>;
TYPED_TEST_SUITE(Quaternion_T, TestTypes);


//======== Actual tests ========

TYPED_TEST(Quaternion_T, Getters)
{
	using real_t = TypeParam;

	struct TestCase
	{
		const mathlib::Quaternion<real_t> m_testObj;
		const real_t m_r;
		const real_t m_i;
		const real_t m_j;
		const real_t m_k;

		void stream(stream_t& p_stream) const
		{
			p_stream
				<< "["
				<< core::to_chars<char8_t>(m_r) << "; "
				<< core::to_chars<char8_t>(m_i) << "i; "
				<< core::to_chars<char8_t>(m_j) << "j; "
				<< core::to_chars<char8_t>(m_k) << "k]";
		}
	};

//saves all that work of setting up and type casting
#define TESTCASE_2(R, I, J, K) TestCase{{R, I, J, K}, R, I, J, K}
#define TESTCASE(R, I, J, K) TESTCASE_2(static_cast<real_t>(R), static_cast<real_t>(I), static_cast<real_t>(J), static_cast<real_t>(K))
	const std::vector<TestCase> testData =
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

	for(const TestCase& tcase: testData)
	{
		ASSERT_EQ(tcase.m_testObj.r(), tcase.m_r) << toStream{tcase};
		ASSERT_EQ(tcase.m_testObj.i(), tcase.m_i) << toStream{tcase};
		ASSERT_EQ(tcase.m_testObj.j(), tcase.m_j) << toStream{tcase};
		ASSERT_EQ(tcase.m_testObj.k(), tcase.m_k) << toStream{tcase};
	}
}

TYPED_TEST(Quaternion_T, Setters)
{
	using real_t = TypeParam;

	struct TestCase
	{
		const real_t m_r;
		const real_t m_i;
		const real_t m_j;
		const real_t m_k;

		void stream(stream_t& p_stream) const
		{
			p_stream
				<< "["
				<< core::to_chars<char8_t>(m_r) << "; "
				<< core::to_chars<char8_t>(m_i) << "i; "
				<< core::to_chars<char8_t>(m_j) << "j; "
				<< core::to_chars<char8_t>(m_k) << "k]";
		}
	};

	//saves all that work of setting up and type casting
#define TESTCASE(R, I, J, K) {static_cast<real_t>(R), static_cast<real_t>(I), static_cast<real_t>(J), static_cast<real_t>(K)}
	const std::vector<TestCase> testData =
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

	const std::vector<real_t> testDataR =
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
	for(const TestCase& tcase: testData)
	{
		mathlib::Quaternion<real_t> testObj;
		testObj.set(tcase.m_r, tcase.m_i, tcase.m_j, tcase.m_k);
		ASSERT_EQ(testObj.r(), tcase.m_r) << "All set " << toStream{tcase};
		ASSERT_EQ(testObj.i(), tcase.m_i) << "All set " << toStream{tcase};
		ASSERT_EQ(testObj.j(), tcase.m_j) << "All set " << toStream{tcase};
		ASSERT_EQ(testObj.k(), tcase.m_k) << "All set " << toStream{tcase};
	}

	//individual test
	for(const real_t& tcase: testDataR)
	{
		mathlib::Quaternion<real_t> testR{-41.0, 42.0, -43.0, 44.0};
		mathlib::Quaternion<real_t> testI{-41.0, 42.0, -43.0, 44.0};
		mathlib::Quaternion<real_t> testJ{-41.0, 42.0, -43.0, 44.0};
		mathlib::Quaternion<real_t> testK{-41.0, 42.0, -43.0, 44.0};
		testR.setR(tcase);
		testI.setI(tcase);
		testJ.setJ(tcase); 
		testK.setK(tcase);

		//independence test
		ASSERT_EQ(testR, (mathlib::Quaternion<real_t>{tcase, 42.0, -43.0, 44.0} )) << tcase;
		ASSERT_EQ(testI, (mathlib::Quaternion<real_t>{-41.0, tcase, -43.0, 44.0})) << tcase;
		ASSERT_EQ(testJ, (mathlib::Quaternion<real_t>{-41.0, 42.0, tcase, 44.0} )) << tcase;
		ASSERT_EQ(testK, (mathlib::Quaternion<real_t>{-41.0, 42.0, -43.0, tcase})) << tcase;

		//negated test
		testR.setR(-tcase);
		testI.setI(-tcase);
		testJ.setJ(-tcase); 
		testK.setK(-tcase);

		ASSERT_EQ(testR, (mathlib::Quaternion<real_t>{-tcase, 42.0, -43.0, 44.0} )) << tcase;
		ASSERT_EQ(testI, (mathlib::Quaternion<real_t>{-41.0, -tcase, -43.0, 44.0})) << tcase;
		ASSERT_EQ(testJ, (mathlib::Quaternion<real_t>{-41.0, 42.0, -tcase, 44.0} )) << tcase;
		ASSERT_EQ(testK, (mathlib::Quaternion<real_t>{-41.0, 42.0, -43.0, -tcase})) << tcase;
	}
}

TYPED_TEST(Quaternion_T, Comparison)
{
	using real_t = TypeParam;
	using TestCase = mathlib::Quaternion<real_t>;

#define TESTCASE(R, I, J, K) {static_cast<real_t>(R), static_cast<real_t>(I), static_cast<real_t>(J), static_cast<real_t>(K)}
	const std::vector<TestCase> testData =
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

	const size_t caseSize = testData.size();

	for(size_t i = 0; i < caseSize; ++i)
	{
		//before case
		for(size_t j = 0; j < i; ++j)
		{
			ASSERT_FALSE(testData[i] == testData[j]) << "Case " << testData[i] << " == " << testData[j];
			ASSERT_TRUE(testData[i] != testData[j]) << "Case " << testData[i] << " != " << testData[j];
		}

		ASSERT_TRUE(testData[i] == testData[i]) << "Case " << testData[i] << " == self";
		ASSERT_FALSE(testData[i] != testData[i]) << "Case " << testData[i] << " != self";

		for(size_t j = i + 1; j < caseSize; ++j)
		{
			ASSERT_FALSE(testData[i] == testData[j]) << "Case " << testData[i] << " == " << testData[j];
			ASSERT_TRUE(testData[i] != testData[j]) << "Case " << testData[i] << " != " << testData[j];
		}
	}
}

TYPED_TEST(Quaternion_T, Operator_unary_minus)
{
	using real_t = TypeParam;

	struct TestCase
	{
		const mathlib::Quaternion<real_t> m_sideA;
		const mathlib::Quaternion<real_t> m_sideB;

		void stream(stream_t& p_stream) const
		{
			p_stream << m_sideA << ' ' << m_sideB;
		}
	};

#define TESTCASE_D(R, I, J, K) {{R, I, J, K}, {-R, -I, -J, -K}}
#define TESTCASE(R, I, J, K) TESTCASE_D(static_cast<real_t>(R), static_cast<real_t>(I), static_cast<real_t>(J), static_cast<real_t>(K))
	const std::vector<TestCase> testData =
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

	for(const TestCase& tcase: testData)
	{
		ASSERT_EQ(-tcase.m_sideA, tcase.m_sideB) << toStream{tcase};
		ASSERT_EQ(tcase.m_sideA, -tcase.m_sideB) << toStream{tcase};
	}
}

TYPED_TEST(Quaternion_T, Operator_add)
{
	using real_t = TypeParam;

	struct TestCase
	{
		const mathlib::Quaternion<real_t> m_A;
		const mathlib::Quaternion<real_t> m_B;
		const mathlib::Quaternion<real_t> m_sum;

		void stream(stream_t& p_stream) const
		{
			p_stream << m_A << " " << m_B;
		}
	};

#define TESTCASE_R(R1, I1, J1, K1, R2, I2, J2, K2) {{R1, I1, J1, K1}, {R2, I2, J2, K2}, {(R1 + R2), (I1 + I2), (J1 + J2), (K1 + K2)}} 
#define TESTCASE(R1, I1, J1, K1, R2, I2, J2, K2) TESTCASE_R(static_cast<real_t>(R1), static_cast<real_t>(I1), static_cast<real_t>(J1), static_cast<real_t>(K1), \
	static_cast<real_t>(R2), static_cast<real_t>(I2), static_cast<real_t>(J2), static_cast<real_t>(K2))
	const std::vector<TestCase> testData =
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
	for(const TestCase& tcase: testData)
	{
		ASSERT_EQ(tcase.m_A + tcase.m_B, tcase.m_sum) << "A + B - " << toStream{tcase};
		ASSERT_EQ(tcase.m_B + tcase.m_A, tcase.m_sum) << "B + A - " << toStream{tcase};
	}

	//+=
	for(const TestCase& tcase: testData)
	{
		{
			mathlib::Quaternion<real_t> tval{tcase.m_A};
			tval += tcase.m_B;
			ASSERT_EQ(tval, tcase.m_sum) << "A += B - " << toStream{tcase};
		}
		{
			mathlib::Quaternion<real_t> tval{tcase.m_B};
			tval += tcase.m_A;
			ASSERT_EQ(tval, tcase.m_sum) << "B += A" << toStream{tcase};
		}
	}
}

TYPED_TEST(Quaternion_T, Operator_minus)
{
	using real_t = TypeParam;

	struct TestCase
	{
		const mathlib::Quaternion<real_t> m_A;
		const mathlib::Quaternion<real_t> m_B;
		const mathlib::Quaternion<real_t> m_sub;

		void stream(stream_t& p_stream) const
		{
			p_stream << m_A << " " << m_B;
		}
	};

#define TESTCASE_R(R1, I1, J1, K1, R2, I2, J2, K2) {{R1, I1, J1, K1}, {R2, I2, J2, K2}, {(R1 - R2), (I1 - I2), (J1 - J2), (K1 - K2)}} 
#define TESTCASE(R1, I1, J1, K1, R2, I2, J2, K2) TESTCASE_R(static_cast<real_t>(R1), static_cast<real_t>(I1), static_cast<real_t>(J1), static_cast<real_t>(K1), \
	static_cast<real_t>(R2), static_cast<real_t>(I2), static_cast<real_t>(J2), static_cast<real_t>(K2))
	const std::vector<TestCase> testData =
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
	for(const TestCase& tcase: testData)
	{
		ASSERT_EQ(tcase.m_A - tcase.m_B, tcase.m_sub) << "A-B - " << toStream{tcase};
		ASSERT_EQ(tcase.m_B - tcase.m_A, -tcase.m_sub) << "B-A - " << toStream{tcase};
	}

	//-=
	for(const TestCase& tcase: testData)
	{
		{
			mathlib::Quaternion<real_t> tval{tcase.m_A};
			tval -= tcase.m_B;
			ASSERT_EQ(tval, tcase.m_sub) << "A-=B - " << toStream{tcase};
		}
		{
			mathlib::Quaternion<real_t> tval{tcase.m_B};
			tval -= tcase.m_A;
			ASSERT_EQ(tval, -tcase.m_sub) << "B-=A - " << toStream{tcase};
		}
	}
}


TYPED_TEST(Quaternion_T, scalar_multiply)
{
	using real_t = TypeParam;

	struct TestCase
	{
		const mathlib::Quaternion<real_t>	m_quat;
		const real_t						m_scalar;
		const mathlib::Quaternion<real_t>	m_result;

		void stream(stream_t& p_stream) const
		{
			p_stream << m_quat << " x " << core::to_chars<char8_t>(m_scalar);
		}
	};

#define TESTCASE_R(R, I, J, K, S) {{R, I, J, K}, S, {(R * S), (I * S), (J * S), (K * S)}} 
#define TESTCASE(R, I, J, K, S) TESTCASE_R(static_cast<real_t>(R), static_cast<real_t>(I), static_cast<real_t>(J), static_cast<real_t>(K), static_cast<real_t>(S))
	const std::vector<TestCase> testData =
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

	for(const TestCase& tcase: testData)
	{
		ASSERT_EQ(tcase.m_quat * tcase.m_scalar, tcase.m_result) << "* - " << toStream{tcase};
	}

	for(const TestCase& tcase: testData)
	{
		mathlib::Quaternion<real_t> test{tcase.m_quat};
		test *= tcase.m_scalar;
		ASSERT_EQ(test, tcase.m_result) << "*= - " << toStream{tcase};
	}

}

TYPED_TEST(Quaternion_T, scalar_division)
{
	using real_t = TypeParam;

	struct TestCase
	{
		const mathlib::Quaternion<real_t>	m_quat;
		const real_t						m_scalar;
		const mathlib::Quaternion<real_t>	m_result;

		void stream(stream_t& p_stream) const
		{
			p_stream << m_quat << " / " << core::to_chars<char8_t>(m_scalar);
		}
	};

#define TESTCASE_R(R, I, J, K, S) {{R, I, J, K}, S, {(R / S), (I / S), (J / S), (K / S)}} 
#define TESTCASE(R, I, J, K, S) TESTCASE_R(static_cast<real_t>(R), static_cast<real_t>(I), static_cast<real_t>(J), static_cast<real_t>(K), static_cast<real_t>(S))
	const std::vector<TestCase> testData =
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

	for(const TestCase& tcase: testData)
	{
		ASSERT_EQ(tcase.m_quat / tcase.m_scalar, tcase.m_result) << "/ - " << toStream{tcase};
	}

	for(const TestCase& tcase: testData)
	{
		mathlib::Quaternion<real_t> test{tcase.m_quat};
		test /= tcase.m_scalar;
		ASSERT_EQ(test, tcase.m_result) << "/= - " << toStream{tcase};
	}
}


TYPED_TEST(Quaternion_T, quaternion_multiplication)
{
	using real_t = TypeParam;

	struct TestCase
	{
		const mathlib::Quaternion<real_t> m_first;
		const mathlib::Quaternion<real_t> m_second;
		const mathlib::Quaternion<real_t> m_result;

		void stream(stream_t& p_stream) const
		{
			p_stream << m_first << " " << m_second;
		}
	};

#define QUAT_T(R, I, J, K) {static_cast<real_t>(R), static_cast<real_t>(I), static_cast<real_t>(J), static_cast<real_t>(K)}
	const std::vector<TestCase> testData =
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

	for(const TestCase& tcase: testData)
	{
		const mathlib::Quaternion<real_t> res = tcase.m_first * tcase.m_second;

		ASSERT_NEAR(static_cast<double>(res.r()), static_cast<double>(tcase.m_result.r()), epsilon) << "* " << toStream{tcase};
		ASSERT_NEAR(static_cast<double>(res.i()), static_cast<double>(tcase.m_result.i()), epsilon) << "* " << toStream{tcase};
		ASSERT_NEAR(static_cast<double>(res.j()), static_cast<double>(tcase.m_result.j()), epsilon) << "* " << toStream{tcase};
		ASSERT_NEAR(static_cast<double>(res.k()), static_cast<double>(tcase.m_result.k()), epsilon) << "* " << toStream{tcase};
	}

	for(const TestCase& tcase: testData)
	{
		mathlib::Quaternion<real_t> res{tcase.m_first};
		res *= tcase.m_second;
		ASSERT_NEAR(static_cast<double>(res.r()), static_cast<double>(tcase.m_result.r()), epsilon) << "*= " << toStream{tcase};
		ASSERT_NEAR(static_cast<double>(res.i()), static_cast<double>(tcase.m_result.i()), epsilon) << "*= " << toStream{tcase};
		ASSERT_NEAR(static_cast<double>(res.j()), static_cast<double>(tcase.m_result.j()), epsilon) << "*= " << toStream{tcase};
		ASSERT_NEAR(static_cast<double>(res.k()), static_cast<double>(tcase.m_result.k()), epsilon) << "*= " << toStream{tcase};
	}
}

TYPED_TEST(Quaternion_T, isZero)
{
	using real_t = TypeParam;
	using TestCase = mathlib::Quaternion<real_t>;

	const real_t infinitesimal	= std::numeric_limits<real_t>::denorm_min();

#define TESTCASE(R, I, J, K) {static_cast<real_t>(R), static_cast<real_t>(I), static_cast<real_t>(J), static_cast<real_t>(K)}
	const std::vector<TestCase> testData =
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
		constexpr mathlib::Quaternion<real_t> qt0 {real_t{0}, real_t{0}, real_t{0}, real_t{0}};
		ASSERT_TRUE(qt0.isZero()) << qt0;
	}

	for(const TestCase& tcase: testData)
	{
		ASSERT_FALSE(tcase.isZero()) << tcase;
	}
}

TYPED_TEST(Quaternion_T, norm_squared)
{
	using real_t = TypeParam;

	struct TestCase
	{
		const mathlib::Quaternion<real_t> m_quat;
		const real_t norm_sqrd;
	};

#define TESTCASE(R, I, J, K, N) {{static_cast<real_t>(R), static_cast<real_t>(I), static_cast<real_t>(J), static_cast<real_t>(K)}, static_cast<real_t>(N)}
	const std::vector<TestCase> testData =
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

	for(const TestCase& tcase: testData)
	{
		ASSERT_EQ(tcase.m_quat.norm_squared(), tcase.norm_sqrd) << tcase.m_quat;
	}
}

TYPED_TEST(Quaternion_T, norm)
{
	using real_t = TypeParam;

	struct TestCase
	{
		const mathlib::Quaternion<real_t> m_quat;
		const real_t norm;
		const real_t error;
	};

	//manual calculations where not done with more than 12 digits of precision
	constexpr real_t epsilon = std::max(std::numeric_limits<real_t>::epsilon(), static_cast<real_t>(0.000000000001));

	//computations may loose further precision proportional to the size of the number
#define TESTCASE(R, I, J, K, N, E) {{real_t{R}, real_t{I}, real_t{J}, real_t{K}}, static_cast<real_t>(N), static_cast<real_t>(E * N * 3.0)}
	const std::vector<TestCase> testData =
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

	for(const TestCase& tcase: testData)
	{
		ASSERT_NEAR(static_cast<double>(tcase.m_quat.norm()), static_cast<double>(tcase.norm), static_cast<double>(tcase.error)) << tcase.m_quat;
	}
}

TYPED_TEST(Quaternion_T, renormalized)
{
	using real_t = TypeParam;

	struct TestCase
	{
		const mathlib::Quaternion<real_t> m_quat;
		const mathlib::Quaternion<real_t> m_result;
		const real_t m_error;
	};

	//manual calculations where not done with more than 12 digits of precision
	constexpr real_t epsilon = std::max(std::numeric_limits<real_t>::epsilon(), static_cast<real_t>(0.000000000001));
	//computations may loose further precision proportional to the size of the number

#define QUAT_T(R, I, J, K) {static_cast<real_t>(R), static_cast<real_t>(I), static_cast<real_t>(J), static_cast<real_t>(K)}
	const std::vector<TestCase> testData =
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
		mathlib::Quaternion<real_t> qt0{0.0, 0.0, 0.0, 0.0};
		std::optional<mathlib::Quaternion<real_t>> val = qt0.renormalized();
		ASSERT_FALSE(val.has_value());
	}

	for(const TestCase& tcase: testData)
	{
		std::optional<mathlib::Quaternion<real_t>> res = tcase.m_quat.renormalized();
		ASSERT_TRUE(res.has_value());
		mathlib::Quaternion<real_t> val = res.value();
		ASSERT_NEAR(static_cast<double>(val.r()), static_cast<double>(tcase.m_result.r()), static_cast<double>(tcase.m_error)) << tcase.m_quat;
		ASSERT_NEAR(static_cast<double>(val.i()), static_cast<double>(tcase.m_result.i()), static_cast<double>(tcase.m_error)) << tcase.m_quat;
		ASSERT_NEAR(static_cast<double>(val.j()), static_cast<double>(tcase.m_result.j()), static_cast<double>(tcase.m_error)) << tcase.m_quat;
		ASSERT_NEAR(static_cast<double>(val.k()), static_cast<double>(tcase.m_result.k()), static_cast<double>(tcase.m_error)) << tcase.m_quat;
	}
}


TYPED_TEST(Quaternion_T, inverse)
{
	using real_t = TypeParam;

	struct TestCase
	{
		const mathlib::Quaternion<real_t> m_quat;
		const real_t m_error;
	};

	constexpr real_t epsilon = std::numeric_limits<real_t>::epsilon();

#define QUAT_T(R, I, J, K) {static_cast<real_t>(R), static_cast<real_t>(I), static_cast<real_t>(J), static_cast<real_t>(K)}
	const std::vector<TestCase> testData =
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
		mathlib::Quaternion<real_t> qt0{0.0, 0.0, 0.0, 0.0};
		std::optional<mathlib::Quaternion<real_t>> res = qt0.inverse();
		ASSERT_FALSE(res.has_value());
	}

	for(const TestCase& tcase: testData)
	{
		std::optional<mathlib::Quaternion<real_t>> res = tcase.m_quat.inverse();
		ASSERT_TRUE(res.has_value());

		const mathlib::Quaternion<real_t> val = res.value() * tcase.m_quat;

		ASSERT_NEAR(static_cast<double>(val.r()), 1.0, static_cast<double>(tcase.m_error)) << tcase.m_quat;
		ASSERT_NEAR(static_cast<double>(val.i()), 0.0, static_cast<double>(tcase.m_error)) << tcase.m_quat;
		ASSERT_NEAR(static_cast<double>(val.j()), 0.0, static_cast<double>(tcase.m_error)) << tcase.m_quat;
		ASSERT_NEAR(static_cast<double>(val.k()), 0.0, static_cast<double>(tcase.m_error)) << tcase.m_quat;
	}
}


TYPED_TEST(Quaternion_T, isFinite)
{
	using real_t = TypeParam;
	using TestCase = mathlib::Quaternion<real_t>;

	const real_t nan			= std::numeric_limits<real_t>::quiet_NaN();
	const real_t infinity		= std::numeric_limits<real_t>::infinity();
	const real_t infinitesimal	= std::numeric_limits<real_t>::denorm_min();

#define TESTCASE(R, I, J, K) {real_t{R}, real_t{I}, real_t{J}, real_t{K}}
	const std::vector<TestCase> testDataPositive =
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

	const std::vector<TestCase> testDataNegative =
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

	for(const TestCase& tcase: testDataPositive)
	{
		ASSERT_TRUE(tcase.isFinite()) << tcase;
	}

	for(const TestCase& tcase: testDataNegative)
	{
		ASSERT_FALSE(tcase.isFinite()) << tcase;
	}
}

#endif
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

#include <cmath>
#include <vector>
#include <limits>
#include <utility>

#include <ostream>
#include <string_view>
#include <string>

#include <CoreLib/Core_String.hpp>

#include <MathLib/LinearAlgebra/Vector.hpp>
#include <MathLib/constants.hpp>

#include <MathLib/HyperComplex/quaternionRotator.hpp>


using stream_t = std::basic_ostream<char>;


//======== Stream assists ========

//For some reason if this is not defined before #include <gtest/gtest.h> then template resolution will fail
#ifdef _WIN32
static stream_t& operator << (stream_t& p_stream, const std::u8string& p_str)
{
	p_stream.write(reinterpret_cast<const char*>(p_str.data()), p_str.size());
	return p_stream;
}
#endif

template<typename T>
static stream_t& operator << (stream_t& p_stream, const typename mathlib::Vector<T, 3>& p_data)
{
#ifdef _WIN32
	p_stream
		<< "["
		<< core::to_chars<char8_t>(p_data[0]) << "; "
		<< core::to_chars<char8_t>(p_data[1]) << "; "
		<< core::to_chars<char8_t>(p_data[2]) << "]";
#else
	p_stream
		<< "["
		<< p_data[0] << "; "
		<< p_data[1] << "; "
		<< p_data[2] << "]";
#endif
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


namespace mathlib
{

//======== ======== ======== Typed test suit  ======== ======== =======

template<typename T>
class QuaternionRotator_T : public testing::Test {
protected:
	QuaternionRotator_T() {}
};

using TestTypes = ::testing::Types<float, double, long double>;
TYPED_TEST_SUITE(QuaternionRotator_T, TestTypes);


template<typename real_t>
void vect_permutate(Vector<real_t, 3>& p_vect)
{
	real_t aux = p_vect[0];
	p_vect[0] = p_vect[1];
	p_vect[1] = p_vect[2];
	p_vect[2] = aux;
}

//======== Actual tests ========

TYPED_TEST(QuaternionRotator_T, Rotator)
{
	using real_t = TypeParam;

	constexpr real_t epsilon = std::numeric_limits<real_t>::epsilon();

	struct testVect
	{
		Vector<real_t, 3> vect;
		Vector<real_t, 3> result;

		void permutate()
		{
			vect_permutate(vect);
			vect_permutate(result);
		}
	};

	struct TestCase
	{
		Vector<real_t, 3>	rotationAxis;
		real_t				rotationValue;
		std::vector<testVect> tests;

		void stream(stream_t& p_stream) const
		{
#ifdef _WIN32
			p_stream << "R: " << rotationAxis << " A: " << core::to_chars<char8_t>(rotationValue);
#else
			p_stream << "R: " << rotationAxis << " A: " << rotationValue;
#endif
		}

		void permutate()
		{
			vect_permutate(rotationAxis);
			for(testVect& tvect: tests)
			{
				tvect.permutate();
			}
		}

		void flip()
		{
			rotationAxis = -rotationAxis;
			rotationValue = -rotationValue;
		}
	};


#define VECT(X,Y,Z) Vector<real_t, 3>{real_t{X}, real_t{Y}, real_t{Z}}
	std::vector<TestCase> testData =
	{
		{VECT(3.0, 4.0, 5.0), real_t{0.0}, std::vector<testVect>{
			{VECT(1.0, 2.0, 3.0), VECT(1.0, 2.0, 3.0)},
		}},

		{VECT(1.0, 0.0, 0.0), tau<real_t> / real_t{2.0}, std::vector<testVect>{
			{VECT(1.0, 0.0, 0.0), VECT(1.0,  0.0,  0.0)},
			{VECT(0.0, 1.0, 0.0), VECT(0.0, -1.0,  0.0)},
			{VECT(1.0, 2.0, 3.0), VECT(1.0, -2.0, -3.0)},
		}},

		{VECT(1.0, 0.0, 0.0), tau<real_t> / real_t{4.0}, std::vector<testVect>{
			{VECT(1.0, 0.0, 0.0), VECT(1.0,  0.0, 0.0)},
			{VECT(0.0, 1.0, 0.0), VECT(0.0,  0.0, 1.0)},
			{VECT(0.0, 0.0, 1.0), VECT(0.0, -1.0, 0.0)},
			{VECT(1.0, 1.0, 0.0), VECT(1.0,  0.0, 1.0)},
			{VECT(1.0, 0.0, 1.0), VECT(1.0, -1.0, 0.0)},
		}},

		{VECT(1.0, 0.0, 0.0), -tau<real_t> / real_t{4.0}, std::vector<testVect>{
			{VECT(1.0, 0.0, 0.0), VECT(1.0, 0.0,  0.0)},
			{VECT(0.0, 1.0, 0.0), VECT(0.0, 0.0, -1.0)},
			{VECT(0.0, 0.0, 1.0), VECT(0.0, 1.0,  0.0)},
			{VECT(1.0, 1.0, 0.0), VECT(1.0, 0.0, -1.0)},
			{VECT(1.0, 0.0, 1.0), VECT(1.0, 1.0,  0.0)},
		}},

		{VECT(1.0, 0.0, 0.0), tau<real_t> / real_t{8.0}, std::vector<testVect>{
			{VECT(1.0, 0.0, 0.0), VECT(1.0, 0.0, 0.0)},
			{VECT(0.0, 1.0, 0.0), VECT(0.0,  real_t{1.0} / squareRoot_2<real_t>, real_t{1.0} / squareRoot_2<real_t>)},
			{VECT(0.0, 0.0, 1.0), VECT(0.0, -real_t{1.0} / squareRoot_2<real_t>, real_t{1.0} / squareRoot_2<real_t>)},
			{VECT(1.0, 1.0, 0.0), VECT(1.0,  real_t{1.0} / squareRoot_2<real_t>, real_t{1.0} / squareRoot_2<real_t>)},
			{VECT(1.0, 0.0, 1.0), VECT(1.0, -real_t{1.0} / squareRoot_2<real_t>, real_t{1.0} / squareRoot_2<real_t>)},
		}},

		{VECT(1.0, 1.0, 1.0), tau<real_t> / real_t{3.0}, std::vector<testVect>{
			{VECT(1.0, 0.0, 0.0), VECT(0.0, 1.0, 0.0)},
			{VECT(1.0, 2.0, 3.0), VECT(3.0, 1.0, 2.0)},
		}},
	};
#undef VECT


#define HELP_NEAR(A, B, C) ASSERT_NEAR(static_cast<double>(A), static_cast<double>(B), static_cast<double>(C))

	for(TestCase& tcase: testData)
	{
		size_t perm = 0;
		do
		{
			size_t neg = 0;
			do
			{
				QuaternionRotator<real_t> rotator{tcase.rotationAxis, tcase.rotationValue};
				for(const testVect& vect : tcase.tests)
				{
					Vector<real_t, 3> res = rotator.rotate(vect.vect);

					HELP_NEAR(res[0], vect.result[0], epsilon * 10) << toStream{tcase} << " & " << vect.vect;
					HELP_NEAR(res[1], vect.result[1], epsilon * 10) << toStream{tcase} << " & " << vect.vect;
					HELP_NEAR(res[2], vect.result[2], epsilon * 10) << toStream{tcase} << " & " << vect.vect;
				}
				tcase.flip();
			}
			while(++neg < 2);
		}
		while(++perm < 3 && (tcase.permutate(), true));
	}

#undef HELP_NEAR

}

TYPED_TEST(QuaternionRotator_T, fromVector)
{
	using real_t = TypeParam;
	constexpr real_t epsilon = std::numeric_limits<real_t>::epsilon();

	struct TestCase
	{
		Vector<real_t, 3>	vect;
		Quaternion<real_t>	result;
	};

	const real_t cos_1_2 = std::cos(real_t{1.0} / real_t{2.0});
	const real_t sin_1_2 = std::sin(real_t{1.0} / real_t{2.0});

	const real_t sqrt_14	= std::sqrt(real_t{14.0});
	const real_t cos_s14_2	= std::cos(sqrt_14 / real_t{2.0});
	const real_t sin_s14_2	= std::sin(sqrt_14 / real_t{2.0});
	const real_t div_1_s14	= real_t{1.0} / sqrt_14;

#define VECT(X,Y,Z) Vector<real_t, 3>{real_t{X}, real_t{Y}, real_t{Z}}
#define QUAT(R, I, J, K) Quaternion<real_t>{real_t{R}, real_t{I}, real_t{J}, real_t{K}}
	std::vector<TestCase> testData =
	{
		{VECT(0.0, 0.0, 0.0), QUAT(1.0, 0.0, 0.0, 0.0)},
		{VECT(1.0, 0.0, 0.0), QUAT(cos_1_2, sin_1_2, 0.0, 0.0)},
		{VECT(0.0, 1.0, 0.0), QUAT(cos_1_2, 0.0, sin_1_2, 0.0)},
		{VECT(0.0, 0.0, 1.0), QUAT(cos_1_2, 0.0, 0.0, sin_1_2)},

		{VECT(pi<real_t>, 0.0, 0.0), QUAT(0.0, 1.0, 0.0, 0.0)},
		{VECT(0.0, pi<real_t>, 0.0), QUAT(0.0, 0.0, 1.0, 0.0)},
		{VECT(0.0, 0.0, pi<real_t>), QUAT(0.0, 0.0, 0.0, 1.0)},

		{VECT(real_t{2.0} * pi<real_t>, 0.0, 0.0), QUAT(-1.0, 0.0, 0.0, 0.0)},
		{VECT(0.0, real_t{2.0} * pi<real_t>, 0.0), QUAT(-1.0, 0.0, 0.0, 0.0)},
		{VECT(0.0, 0.0, real_t{2.0} * pi<real_t>), QUAT(-1.0, 0.0, 0.0, 0.0)},

		{VECT(-pi<real_t>, 0.0, 0.0), QUAT(0.0, -1.0, 0.0, 0.0)},

		{VECT(1.0, 2.0, 3.0), QUAT(cos_s14_2, div_1_s14 * sin_s14_2, real_t{2.0} * div_1_s14 * sin_s14_2, real_t{3.0} * div_1_s14 * sin_s14_2)},
	};
#undef VECT
#undef QUAT

#define HELP_NEAR(A, B, C) ASSERT_NEAR(static_cast<double>(A), static_cast<double>(B), static_cast<double>(C))
	for(const TestCase& tcase: testData)
	{
		Quaternion<real_t> res = QuaternionRotator<real_t>{tcase.vect}.identity();
		HELP_NEAR(res.r(), tcase.result.r(), epsilon) << tcase.vect;
		HELP_NEAR(res.i(), tcase.result.i(), epsilon) << tcase.vect;
		HELP_NEAR(res.j(), tcase.result.j(), epsilon) << tcase.vect;
		HELP_NEAR(res.k(), tcase.result.k(), epsilon) << tcase.vect;
	}
#undef HELP_NEAR

}

TYPED_TEST(QuaternionRotator_T, toVector)
{
	using real_t = TypeParam;
	constexpr real_t epsilon = std::numeric_limits<real_t>::epsilon() * real_t{10.0};

	struct TestCase
	{
		Vector<real_t, 3> vect;
		Vector<real_t, 3> result;
	};

#define VECT(X,Y,Z) Vector<real_t, 3>{real_t{X}, real_t{Y}, real_t{Z}}
#define MIRROR(X,Y,Z) {VECT(X,Y,Z), VECT(X,Y,Z)}
	std::vector<TestCase> testData =
	{
		MIRROR(0.0, 0.0, 0.0),
		MIRROR(1.0, 0.0, 0.0),
		MIRROR(0.0, 1.0, 0.0),
		MIRROR(0.0, 0.0, 1.0),
		MIRROR(1.0, 2.0, 3.0),


		{VECT(2.0 * pi<real_t>, 0.0, 0.0), VECT(0.0, 0.0, 0.0)},
		{VECT(0.0, 2.0 * pi<real_t>, 0.0), VECT(0.0, 0.0, 0.0)},
		{VECT(0.0, 0.0, 2.0 * pi<real_t>), VECT(0.0, 0.0, 0.0)},

		MIRROR(-2.0, 3.0, -1.0),
	};
#undef MIRROR
#undef VECT

#define HELP_NEAR(A, B, C) ASSERT_NEAR(static_cast<double>(A), static_cast<double>(B), static_cast<double>(C))
	for(const TestCase& tcase: testData)
	{
		QuaternionRotator<real_t> quat{tcase.vect};
		Vector<real_t, 3> res = quat.vector();
		HELP_NEAR(res[0], tcase.result[0], epsilon) << tcase.vect;
		HELP_NEAR(res[1], tcase.result[1], epsilon) << tcase.vect;
		HELP_NEAR(res[2], tcase.result[2], epsilon) << tcase.vect;
	}
#undef HELP_NEAR

}

TYPED_TEST(QuaternionRotator_T, toMatrix)
{
	//TODO:
	using real_t = TypeParam;
	constexpr real_t epsilon = std::numeric_limits<real_t>::epsilon() * real_t{10.0};


#define VECT(X,Y,Z) Vector<real_t, 3>{real_t{X}, real_t{Y}, real_t{Z}}
	std::vector<Vector<real_t, 3>> testCases =
	{
		VECT(0.0, 0.0, 0.0),
		VECT(1.0, 0.0, 0.0),
		VECT(0.0, 1.0, 0.0),
		VECT(0.0, 0.0, 1.0),
		VECT(1.0, 2.0, 3.0),
		VECT(-2.0, 3.0, -1.0),
	};

	std::vector<Vector<real_t, 3>> testData = testCases;
#undef VECT

#define HELP_NEAR(A, B, C) ASSERT_NEAR(static_cast<double>(A), static_cast<double>(B), static_cast<double>(C))
	for(const Vector<real_t, 3>& tcase: testCases)
	{
		QuaternionRotator<real_t> quat{tcase};
		Matrix<real_t, 3, 3> res = quat.matrix();
		
		for(const Vector<real_t, 3>& tdata: testData)
		{
			Vector<real_t, 3> mode1 = quat.rotate(tdata);
			Vector<real_t, 3> mode2 = res * tdata;

			HELP_NEAR(mode1[0], mode2[0], epsilon) << tcase << ":" << tdata;
			HELP_NEAR(mode1[1], mode2[1], epsilon) << tcase << ":" << tdata;
			HELP_NEAR(mode1[2], mode2[2], epsilon) << tcase << ":" << tdata;
		}
	}
#undef HELP_NEAR
}

} //namespace mathlib
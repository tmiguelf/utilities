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

#include <cmath>
#include <vector>
#include <limits>
#include <utility>

#include <ostream>
#include <string_view>
#include <string>

#include <CoreLib/string/core_string_numeric.hpp>

#include <MathLib/LinearAlgebra/Vector.hpp>
#include <MathLib/constants.hpp>

#include <MathLib/HyperComplex/quaternionRotator.hpp>

#include <CoreLib/toPrint/toPrint_encoders.hpp>
#include <CoreLib/toPrint/toPrint_std_ostream.hpp>
#include <CoreLib/toPrint/toPrint_base.hpp>

using core::toPrint;

template<typename T>
static std::ostream& operator << (std::ostream& p_stream, mathlib::Vector3<T> const& p_data)
{
	p_stream
		<< "["
		<< toPrint{p_data[0]} << "; "
		<< toPrint{p_data[1]} << "; "
		<< toPrint{p_data[2]} << "]";
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

using TestTypes = ::testing::Types<float, double/*, long double*/>;
TYPED_TEST_SUITE(QuaternionRotator_T, TestTypes);


template<typename real_t>
void vect_permutate(Vector3<real_t>& p_vect)
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
		Vector3<real_t> vect;
		Vector3<real_t> result;

		void permutate()
		{
			vect_permutate(vect);
			vect_permutate(result);
		}
	};

	struct TestCase
	{
		Vector3<real_t>	rotationAxis;
		real_t				rotationValue;
		std::vector<testVect> tests;

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

	struct case_toPrint: public core::toPrint_base
	{
		case_toPrint(TestCase const& p_data): m_data{p_data} {}

		uintptr_t size(char8_t const&) const
		{
			constexpr std::string_view filler = "R: [; ; ] A: ";

			return
				filler.size() +
				core::to_chars_size(m_data.rotationAxis[0]) +
				core::to_chars_size(m_data.rotationAxis[1]) +
				core::to_chars_size(m_data.rotationAxis[2]) +
				core::to_chars_size(m_data.rotationValue);
		}

		void get_print(char8_t* p_out) const
		{
			constexpr uintptr_t max_size = core::to_chars_dec_max_size_v<real_t>;

			*(p_out++) = u8'R';
			*(p_out++) = u8':';
			*(p_out++) = u8' ';
			*(p_out++) = u8'[';
			p_out += core::_p::to_chars(m_data.rotationAxis[0], std::span<char8_t, max_size>{p_out, max_size});
			*(p_out++) = u8';';
			*(p_out++) = u8' ';
			p_out += core::_p::to_chars(m_data.rotationAxis[1], std::span<char8_t, max_size>{p_out, max_size});
			*(p_out++) = u8';';
			*(p_out++) = u8' ';
			p_out += core::_p::to_chars(m_data.rotationAxis[2], std::span<char8_t, max_size>{p_out, max_size});
			*(p_out++) = u8']';
			*(p_out++) = u8' ';
			*(p_out++) = u8'A';
			*(p_out++) = u8':';
			*(p_out++) = u8' ';
			core::to_chars_unsafe(m_data.rotationValue, p_out);
		}

		TestCase const& m_data;
	};


#define VECT(X,Y,Z) Vector3<real_t>{real_t{X}, real_t{Y}, real_t{Z}}
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
		uintptr_t perm = 0;
		do
		{
			uintptr_t neg = 0;
			do
			{
				QuaternionRotator<real_t> rotator{tcase.rotationAxis, tcase.rotationValue};
				for(testVect const& vect : tcase.tests)
				{
					Vector3<real_t> res = rotator.rotate(vect.vect);

					HELP_NEAR(res[0], vect.result[0], epsilon * 10) << case_toPrint{tcase} << " & " << vect.vect;
					HELP_NEAR(res[1], vect.result[1], epsilon * 10) << case_toPrint{tcase} << " & " << vect.vect;
					HELP_NEAR(res[2], vect.result[2], epsilon * 10) << case_toPrint{tcase} << " & " << vect.vect;
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
		Vector3<real_t>	vect;
		Quaternion<real_t>	result;
	};

	real_t const cos_1_2 = std::cos(real_t{1.0} / real_t{2.0});
	real_t const sin_1_2 = std::sin(real_t{1.0} / real_t{2.0});

	real_t const sqrt_14	= std::sqrt(real_t{14.0});
	real_t const cos_s14_2	= std::cos(sqrt_14 / real_t{2.0});
	real_t const sin_s14_2	= std::sin(sqrt_14 / real_t{2.0});
	real_t const div_1_s14	= real_t{1.0} / sqrt_14;

#define VECT(X,Y,Z) Vector3<real_t>{real_t{X}, real_t{Y}, real_t{Z}}
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
	for(TestCase const& tcase: testData)
	{
		Quaternion<real_t> res = QuaternionRotator<real_t>{tcase.vect}.identity();
		HELP_NEAR(res.r(), tcase.result.r(), epsilon) << tcase.vect;
		HELP_NEAR(res.i(), tcase.result.i(), epsilon) << tcase.vect;
		HELP_NEAR(res.j(), tcase.result.j(), epsilon) << tcase.vect;
		HELP_NEAR(res.k(), tcase.result.k(), epsilon) << tcase.vect;
	}
#undef HELP_NEAR

}

TYPED_TEST(QuaternionRotator_T, to_axis_angle)
{
	using real_t = TypeParam;
	constexpr real_t epsilon = std::numeric_limits<real_t>::epsilon() * real_t{10.0};

	struct TestCase
	{
		Vector3<real_t> vect;
		Vector3<real_t> result;
	};

#define VECT(X,Y,Z) Vector3<real_t>{real_t{X}, real_t{Y}, real_t{Z}}
#define MIRROR(X,Y,Z) {VECT(X,Y,Z), VECT(X,Y,Z)}
	std::vector<TestCase> testData =
	{
		MIRROR(0.0, 0.0, 0.0),
		MIRROR(1.0, 0.0, 0.0),
		MIRROR(0.0, 1.0, 0.0),
		MIRROR(0.0, 0.0, 1.0),
		MIRROR(0.5, 1.5, 2.0),


		{VECT(tau<real_t>, 0.0, 0.0), VECT(0.0, 0.0, 0.0)},
		{VECT(0.0, tau<real_t>, 0.0), VECT(0.0, 0.0, 0.0)},
		{VECT(0.0, 0.0, tau<real_t>), VECT(0.0, 0.0, 0.0)},

		{VECT(pi<real_t> + real_t{0.125}, 0.0, 0.0), VECT(-pi<real_t> + real_t{0.125}, 0.0, 0.0)},
		{VECT(0.0, pi<real_t> + real_t{0.125}, 0.0), VECT(0.0, -pi<real_t> + real_t{0.125}, 0.0)},
		{VECT(0.0, 0.0, pi<real_t> + real_t{0.125}), VECT(0.0, 0.0, -pi<real_t> + real_t{0.125})},

		MIRROR(-1.5, 2.0, -0.5),
	};
#undef MIRROR
#undef VECT

#define HELP_NEAR(A, B, C) ASSERT_NEAR(static_cast<double>(A), static_cast<double>(B), static_cast<double>(C))
	for(TestCase const& tcase: testData)
	{
		QuaternionRotator<real_t> quat{tcase.vect};
		Vector3<real_t> res = quat.axis_angle();
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


#define VECT(X,Y,Z) Vector3<real_t>{real_t{X}, real_t{Y}, real_t{Z}}
	std::vector<Vector3<real_t>> testCases =
	{
		VECT(0.0, 0.0, 0.0),
		VECT(1.0, 0.0, 0.0),
		VECT(0.0, 1.0, 0.0),
		VECT(0.0, 0.0, 1.0),
		VECT(1.0, 2.0, 3.0),
		VECT(-2.0, 3.0, -1.0),
	};

	std::vector<Vector3<real_t>> testData = testCases;
#undef VECT

#define HELP_NEAR(A, B, C) ASSERT_NEAR(static_cast<double>(A), static_cast<double>(B), static_cast<double>(C))
	for(Vector3<real_t> const& tcase: testCases)
	{
		QuaternionRotator<real_t> quat{tcase};
		Matrix3<real_t> res = quat.matrix();
		
		for(Vector3<real_t> const& tdata: testData)
		{
			Vector3<real_t> mode1 = quat.rotate(tdata);
			Vector3<real_t> mode2 = res * tdata;

			HELP_NEAR(mode1[0], mode2[0], epsilon) << tcase << ":" << tdata;
			HELP_NEAR(mode1[1], mode2[1], epsilon) << tcase << ":" << tdata;
			HELP_NEAR(mode1[2], mode2[2], epsilon) << tcase << ":" << tdata;
		}
	}
#undef HELP_NEAR
}

} //namespace mathlib

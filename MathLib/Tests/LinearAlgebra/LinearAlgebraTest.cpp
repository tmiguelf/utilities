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

#include <gtest/gtest.h>

#include <MathLib/LinearAlgebra/Vector.hpp>
#include <MathLib/LinearAlgebra/Matrix.hpp>
#include <string>
#include <vector>

template<typename T1, typename T2, typename TR>
struct testCase_t
{
	T1 const m_arg1;
	T2 const m_arg2;
	TR const m_result;
	testCase_t(T1 const& p_1, T2 const& p_2, TR const& p_r): m_arg1(p_1), m_arg2(p_2), m_result(p_r) {}
};

template<typename T1, typename T2, typename TR>
using testList_t = std::vector<testCase_t<T1, T2, TR>>;


TEST(LinearAlgebra_Vector, MultiplicationByScalar)
{
	using T1 = mathlib::Vector<double, 3>;
	using T2 = double;
	using TR = mathlib::Vector<double, 3>;
	using thisCase_t = testCase_t<T1, T2, TR>;
	using thisTest_t = testList_t<T1, T2, TR>;
	
	thisTest_t const TestCases =
		{
			{T1{1.0,   2.0, 3.0},   0.0 , TR{   0.0 ,   0.0,    0.0 }},
			{T1{1.0,   2.0, 3.0},   1.0 , TR{   1.0 ,   2.0,    3.0 }},
			{T1{1.0,   2.0, 3.0},   4.0 , TR{   4.0 ,   8.0,   12.0 }},
			{T1{1.0,   2.0, 3.0},   0.25, TR{   0.25,   0.5,    0.75}},
			{T1{3.0, -10.0, 4.0}, -80.0 , TR{-240.0 , 800.0, -320.0 }},
		};

	uintptr_t testNum = 0;

	for(thisCase_t const& testCase : TestCases)
	{
		TR const result = testCase.m_arg1 * testCase.m_arg2;
		EXPECT_EQ(result, testCase.m_result) << "Failed case " << testNum;
		++testNum;
	}
}

TEST(LinearAlgebra_Vector, DivisionByScalar)
{
	using T1 = mathlib::Vector<double, 3>;
	using T2 = double;
	using TR = mathlib::Vector<double, 3>;
	using thisCase_t = testCase_t<T1, T2, TR>;
	using thisTest_t = testList_t<T1, T2, TR>;
	
	thisTest_t const TestCases =
		{
			{T1{ 1.0,   2.0,  3.0},   1.0 , TR{  1.0 ,   2.0  ,   3.0 }},
			{T1{ 1.0,   2.0,  3.0},   4.0 , TR{  0.25,   0.5  ,   0.75}},
			{T1{ 1.0,   2.0,  3.0},   0.25, TR{  4.0 ,   8.0  ,  12.0 }},
			{T1{40.0,  60.0, 80.0},   4.0 , TR{ 10.0 ,  15.0  ,  20.0 }},
			{T1{32.0, -10.0,  4.0}, -80.0 , TR{- 0.4 ,   0.125, - 0.05}},
		};

	uintptr_t testNum = 0;

	for(thisCase_t const& testCase : TestCases)
	{
		TR const result = testCase.m_arg1 / testCase.m_arg2;
		EXPECT_EQ(result, testCase.m_result) << "Failed case " << testNum;
		++testNum;
	}
}

TEST(LinearAlgebra_Vector, Addition)
{
	using T1 = mathlib::Vector<double, 3>;
	using T2 = mathlib::Vector<double, 3>;
	using TR = mathlib::Vector<double, 3>;
	using thisCase_t = testCase_t<T1, T2, TR>;
	using thisTest_t = testList_t<T1, T2, TR>;
	
	thisTest_t const TestCases =
		{
			{T1{1.0, 2.0, 3.0}, T2{-1.0, -2.0, -3.0}, TR{ 0.0,  0.0,  0.0}},
			{T1{1.0, 2.0, 3.0}, T2{ 1.0,  2.0,  3.0}, TR{ 2.0,  4.0,  6.0}},
			{T1{1.0, 2.0, 3.0}, T2{ 0.0,  0.0,  0.0}, TR{ 1.0,  2.0,  3.0}},
			{T1{1.0, 2.0, 3.0}, T2{-6.0, -5.0, -4.0}, TR{-5.0, -3.0, -1.0}},
			{T1{1.0, 2.0, 3.0}, T2{ 4.0,  5.0,  6.0}, TR{ 5.0,  7.0,  9.0}},
		};

	uintptr_t testNum = 0;

	for(thisCase_t const& testCase : TestCases)
	{
		TR const result = testCase.m_arg1 + testCase.m_arg2;
		EXPECT_EQ(result, testCase.m_result) << "Failed case " << testNum;
		++testNum;
	}
}

TEST(LinearAlgebra_Vector, Subtraction)
{
	using T1 = mathlib::Vector<double, 3>;
	using T2 = mathlib::Vector<double, 3>;
	using TR = mathlib::Vector<double, 3>;
	using thisCase_t = testCase_t<T1, T2, TR>;
	using thisTest_t = testList_t<T1, T2, TR>;
	
	thisTest_t const TestCases =
		{
			{T1{1.0, 2.0, 3.0}, T2{-1.0, -2.0, -3.0}, TR{ 2.0,  4.0,  6.0}},
			{T1{1.0, 2.0, 3.0}, T2{ 1.0,  2.0,  3.0}, TR{ 0.0,  0.0,  0.0}},
			{T1{1.0, 2.0, 3.0}, T2{ 0.0,  0.0,  0.0}, TR{ 1.0,  2.0,  3.0}},
			{T1{1.0, 2.0, 3.0}, T2{ 6.0,  5.0,  4.0}, TR{-5.0, -3.0, -1.0}},
			{T1{1.0, 2.0, 3.0}, T2{-4.0, -5.0, -6.0}, TR{ 5.0,  7.0,  9.0}},
		};

	uintptr_t testNum = 0;

	for(thisCase_t const& testCase : TestCases)
	{
		TR const result = testCase.m_arg1 - testCase.m_arg2;
		EXPECT_EQ(result, testCase.m_result) << "Failed case " << testNum;
		++testNum;
	}
}


TEST(LinearAlgebra_Vector, InternalProduct)
{
	using T1 = mathlib::Vector<double, 3>;
	using T2 = mathlib::Vector<double, 3>;
	using TR = double;
	using thisCase_t = testCase_t<T1, T2, TR>;
	using thisTest_t = testList_t<T1, T2, TR>;

	thisTest_t const TestCases =
		{
			{T1{1.0, 2.0, 3.0}, T2{-1.0, -2.0, -3.0}, -14.0},
			{T1{1.0, 2.0, 3.0}, T2{ 1.0,  2.0,  3.0},  14.0},
			{T1{1.0, 2.0, 3.0}, T2{ 0.0,  0.0,  0.0},  0.0},
			{T1{1.0, 3.0, 2.0}, T2{ 3.0,  2.0, -4.5},  0.0},
		};

	uintptr_t testNum = 0;

	for(thisCase_t const& testCase : TestCases)
	{
		TR const result = testCase.m_arg1 * testCase.m_arg2;
		EXPECT_EQ(result, testCase.m_result) << "Failed case " << testNum;
		++testNum;
	}
}

TEST(LinearAlgebra_Vector, CrossProduct)
{
	using T1 = mathlib::Vector<double, 3>;
	using T2 = mathlib::Vector<double, 3>;
	using TR = mathlib::Vector<double, 3>;
	using thisCase_t = testCase_t<T1, T2, TR>;
	using thisTest_t = testList_t<T1, T2, TR>;

	thisTest_t const TestCases =
		{
			{T1{1.0, 0.0, 0.0}, T2{ 0.0,  1.0,  0.0}, TR{ 0.0,  0.0,  1.0}},
			{T1{0.0, 1.0, 0.0}, T2{ 0.0,  0.0,  1.0}, TR{ 1.0,  0.0,  0.0}},
			{T1{0.0, 1.0, 0.0}, T2{ 1.0,  0.0,  0.0}, TR{ 0.0,  0.0, -1.0}},
			{T1{1.0, 2.0, 3.0}, T2{ 4.0,  5.0,  6.5}, TR{-2.0,  5.5, -3.0}},
		};

	uintptr_t testNum = 0;

	for(thisCase_t const& testCase : TestCases)
	{
		TR const result = crossProduct(testCase.m_arg1, testCase.m_arg2);
		EXPECT_EQ(result, testCase.m_result) << "Failed case " << testNum;
		++testNum;
	}
}

TEST(LinearAlgebra_Vector, Assignemnt)
{
	mathlib::Vector<double, 3> t1 {1.0, 2.0, 3.0};
	mathlib::Vector<double, 3> t2;
	t2 = t1;
	
	EXPECT_EQ(t1, t2);
}

TEST(LinearAlgebra_Vector, CompareEqual)
{
	mathlib::Vector<double, 3> t1 {1.0, 2.0, 3.0};
	mathlib::Vector<double, 3> t2 {1.0, 2.0, 3.0};
	mathlib::Vector<double, 3> t3 {4.0, 5.0, 6.0};
	EXPECT_EQ(t1, t2);
	EXPECT_FALSE(t1 == t3);
}

TEST(LinearAlgebra_Vector, CompareDifferent)
{
	mathlib::Vector<double, 3> t1 {1.0, 2.0, 3.0};
	mathlib::Vector<double, 3> t2 {1.0, 2.0, 3.0};
	mathlib::Vector<double, 3> t3 {4.0, 5.0, 6.0};

	EXPECT_TRUE (t1 != t3);
	EXPECT_FALSE(t1 != t2);
}

TEST(LinearAlgebra_Matrix, MatrixMultiplication)
{
	mathlib::Matrix<double, 3, 3> expect
		{{
			 9.0, 12.0, 15.0,
			19.0, 26.0, 33.0,
			29.0, 40.0, 51.0
		}};

	mathlib::Matrix<double, 3, 2> mat1
		{{
			1.0, 2.0,
			3.0, 4.0,
			5.0, 6.0
		}};

	mathlib::Matrix<double, 2, 3> mat2
		{{
			1.0, 2.0, 3.0,
			4.0, 5.0, 6.0
		}};

	auto result = mat1 * mat2;
	EXPECT_EQ(result, expect);
}

TEST(LinearAlgebra_Matrix, VectorMultiplication)
{
	mathlib::Matrix<double, 3, 3> mat
		{{
			1.0, 2.0, 3.0,
			4.0, 5.0, 6.0,
			7.0, 8.0, 9.0
		}};

	mathlib::Vector<double, 3> vect {1.0, 2.0, 3.0};

	mathlib::Vector<double, 3> expect{14.0, 32.0, 50.0};

	mathlib::Vector<double, 3> result = mat * vect;
	EXPECT_EQ(result, expect);
}

TEST(LinearAlgebra_Matrix, ScalarMultiplication)
{
	using T1 = mathlib::Matrix<double, 3, 3>;
	using T2 = double;
	using TR = mathlib::Matrix<double, 3, 3>;
	using thisCase_t = testCase_t<T1, T2, TR>;
	using thisTest_t = testList_t<T1, T2, TR>;

	mathlib::Matrix<double, 3, 3> const mat
		{{
			1.0, 2.0, 3.0,
			4.0, 5.0, 6.0,
			7.0, 8.0, 9.0
		}};

	thisTest_t const TestCases =
		{
			{mat, 0.0 , TR{{ 0.0,  0.0,   0.0,	  0.0,   0.0, 0.0,		  0.0,   0.0,   0.0		}} },
			{mat, 1.0 , TR{{ 1.0,  2.0,   3.0,	  4.0,   5.0, 6.0,		  7.0,   8.0,   9.0		}} },
			{mat, 4.0 , TR{{ 4.0,  8.0,  12.0,	 16.0,  20.0, 24.0,		 28.0,  32.0,  36.0		}} },
			{mat, 0.25, TR{{ 0.25, 0.5,   0.75,	  1.0,   1.25, 1.5,		  1.75,  2.0,   2.25	}} },
			{mat, -4.0, TR{{-4.0, -8.0, -12.0,	-16.0, -20.0, -24.0,	-28.0, -32.0, -36.0		}} },
		};

	uintptr_t testNum = 0;

	for(thisCase_t const& testCase : TestCases)
	{
		TR const result = testCase.m_arg1 * testCase.m_arg2;
		EXPECT_EQ(result, testCase.m_result) << "Failed case " << testNum;
		++testNum;
	}
}

TEST(LinearAlgebra_Matrix, ScalarDivision)
{
	using T1 = mathlib::Matrix<double, 3, 3>;
	using T2 = double;
	using TR = mathlib::Matrix<double, 3, 3>;
	using thisCase_t = testCase_t<T1, T2, TR>;
	using thisTest_t = testList_t<T1, T2, TR>;

	mathlib::Matrix<double, 3, 3> const mat
		{{
			1.0, 2.0, 3.0,
			4.0, 5.0, 6.0,
			7.0, 8.0, 9.0
		}};

	thisTest_t const TestCases =
		{
			{mat, 1.0 , TR{{ 1.0,   2.0,   3.0,		  4.0,   5.0,    6.0,	  7.0,    8.0,   9.0	}} },
			{mat, 4.0 , TR{{ 0.25,  0.5,   0.75,	  1.0,   1.25,   1.5,	  1.75,   2.0,   2.25	}} },
			{mat, 0.25, TR{{ 4.0,   8.0,  12.0,		 16.0,  20.0,   24.0,	 28.0,   32.0,  36.0	}} },
			{mat, -4.0, TR{{-0.25, -0.5, - 0.75,	- 1.0, - 1.25, - 1.5,	- 1.75, - 2.0, - 2.25	}} }
		};

	uintptr_t testNum = 0;

	for(thisCase_t const& testCase : TestCases)
	{
		TR const result = testCase.m_arg1 / testCase.m_arg2;
		EXPECT_EQ(result, testCase.m_result) << "Failed case " << testNum;
		++testNum;
	}
}

TEST(LinearAlgebra_Matrix, Transpose)
{
	mathlib::Matrix<double, 3, 2> const mat
		{{
			1.0, 2.0,
			3.0, 4.0,
			5.0, 6.0
		}};

	mathlib::Matrix<double, 2, 3> const expect 
		{{
			1.0, 3.0, 5.0,
			2.0, 4.0, 6.0
		}};

	mathlib::Matrix<double, 2, 3> const result = mat.transpose();

	EXPECT_EQ(result, expect);
}

TEST(LinearAlgebra_Matrix, Trace)
{
	mathlib::Matrix<double, 3, 3> const mat
		{{
			1.0, 2.0, 3.0,
			4.0, 5.0, 6.0,
			7.0, 8.0, 9.0
		}};

	double const expect = 15.0;
	double const result = mathlib::trace(mat);

	EXPECT_EQ(result, expect);
}

TEST(LinearAlgebra_Matrix, Addition)
{
	mathlib::Matrix<double, 3, 3> const mat1 
		{{
			1.0, 2.0, 3.0,
			4.0, 5.0, 6.0,
			7.0, 8.0, 9.0
		}};

	mathlib::Matrix<double, 3, 3> const mat2
		{{
			10.0, 11.0, 12.0,
			13.0, 14.0, 15.0,
			16.0, 17.0, 18.0
		}};

	mathlib::Matrix<double, 3, 3> const expect
		{{
			11.0, 13.0, 15.0,
			17.0, 19.0, 21.0,
			23.0, 25.0, 27.0
		}};

	mathlib::Matrix<double, 3, 3> const result = mat1 + mat2;

	EXPECT_EQ(result, expect);
}

TEST(LinearAlgebra_Matrix, Subtraction)
{
	mathlib::Matrix<double, 3, 3> const mat1
		{{
			1.0, 2.0, 3.0,
			4.0, 5.0, 6.0,
			7.0, 8.0, 9.0
		}};

	mathlib::Matrix<double, 3, 3> const mat2
		{{
			9.0, 8.0, 7.0,
			6.0, 5.0, 4.0,
			3.0, 2.0, 1.0
		}};

	mathlib::Matrix<double, 3, 3> const expect
		{{
			-8.0, -6.0, -4.0,
			-2.0,  0.0,  2.0,
			 4.0,  6.0,  8.0
		}};

	mathlib::Matrix<double, 3, 3> result = mat1 - mat2;

	EXPECT_EQ(result, expect);
}


TEST(LinearAlgebra_Matrix, Assign)
{
	mathlib::Matrix<double, 3, 3> const mat1
		{{
			1.0, 2.0, 3.0,
			4.0, 5.0, 6.0,
			7.0, 8.0, 9.0
		}};

	mathlib::Matrix<double, 3, 3> mat2;

	mat2 = mat1;

	EXPECT_EQ(mat2, mat1);
}

TEST(LinearAlgebra_Matrix, CompareEqual)
{
	mathlib::Matrix<double, 3, 3> const mat1
		{{
			1.0, 2.0, 3.0,
			4.0, 5.0, 6.0,
			7.0, 8.0, 9.0
		}};

	mathlib::Matrix<double, 3, 3> const mat2
		{{
			1.0, 2.0, 3.0,
			4.0, 5.0, 6.0,
			7.0, 8.0, 9.0
		}};

	mathlib::Matrix<double, 3, 3> const mat3
		{{
			9.0, 8.0, 7.0,
			6.0, 5.0, 4.0,
			3.0, 2.0, 1.0
		}};

	EXPECT_EQ(mat1, mat2);
	EXPECT_FALSE(mat1 == mat3);
}

TEST(LinearAlgebra_Matrix, CompareDifferent)
{
	mathlib::Matrix<double, 3, 3> const mat1
		{{
			1.0, 2.0, 3.0,
			4.0, 5.0, 6.0,
			7.0, 8.0, 9.0
		}};

	mathlib::Matrix<double, 3, 3> const mat2
		{{
			1.0, 2.0, 3.0,
			4.0, 5.0, 6.0,
			7.0, 8.0, 9.0
		}};

	mathlib::Matrix<double, 3, 3> const mat3
		{{
			9.0, 8.0, 7.0,
			6.0, 5.0, 4.0,
			3.0, 2.0, 1.0
		}};

	EXPECT_TRUE (mat1 != mat3);
	EXPECT_FALSE(mat1 != mat2);
}

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

#include <type_traits>

#include <gtest/gtest.h>

#include <CoreLib/core_pack.hpp>


TEST(core_pack, pack_equivalence)
{
	static_assert(std::is_same_v<core::pack<>, core::pack<>>);
	static_assert(!std::is_same_v<core::pack<>, core::pack<void>>);
	static_assert(!std::is_same_v<core::pack<void>, core::pack<void*>>);
	static_assert(!std::is_same_v<core::pack<uint32_t>, core::pack<uint32_t const>>);
	static_assert(!std::is_same_v<core::pack<uint32_t>, core::pack<uint32_t&>>);
	static_assert(std::is_same_v<core::pack<uint32_t, float, void*>, core::pack<uint32_t, float, void*>>);
	static_assert(!std::is_same_v<core::pack<uint32_t, float, void*>, core::pack<uint32_t, float>>);
	static_assert(!std::is_same_v<core::pack<uint32_t, float>, core::pack<float, uint32_t>>);
}

TEST(core_pack, pack_count)
{
	static_assert(core::pack_count_v<core::pack<>> == 0);
	static_assert(core::pack_count_v<core::pack<void>> == 1);
	static_assert(core::pack_count_v<core::pack<uint32_t, uint32_t&, uint32_t*, uint32_t const>> == 4);
	static_assert(core::pack_count_v<core::pack<float, double, void*, core::pack<float, double>, void*, void>> == 6);
}

TEST(core_pack, pack_get)
{
	using test_t = core::pack<uint32_t, float, void*, void, int64_t const, double, uint32_t&>;
	static_assert(std::is_same_v<core::pack_get_t<test_t, 0>, uint32_t>);
	static_assert(std::is_same_v<core::pack_get_t<test_t, 1>, float>);
	static_assert(std::is_same_v<core::pack_get_t<test_t, 2>, void*>);
	static_assert(std::is_same_v<core::pack_get_t<test_t, 3>, void>);
	static_assert(std::is_same_v<core::pack_get_t<test_t, 4>, int64_t const>);
	static_assert(std::is_same_v<core::pack_get_t<test_t, 5>, double>);
	static_assert(std::is_same_v<core::pack_get_t<test_t, 6>, uint32_t&>);
}

TEST(core_pack, pack_cat)
{
	using test_t1 = core::pack<>;
	using test_t2 = core::pack<void>;
	using test_t3 = core::pack<uint32_t, float>;
	using test_t4 = core::pack<int32_t const&, double>;
	using test_t5 = core::pack<void*, core::pack<uint64_t>>;

	static_assert(std::is_same_v<core::pack_cat_t<test_t1>, core::pack<>>);
	static_assert(std::is_same_v<core::pack_cat_t<test_t3, test_t4>, core::pack<uint32_t, float, int32_t const&, double>>);
	static_assert(std::is_same_v<core::pack_cat_t<test_t1, test_t5, test_t2>, core::pack<void*, core::pack<uint64_t>, void>>);
}

TEST(core_pack, sub_pack)
{
	using test_t = core::pack<uint32_t, float, void*, void, int64_t const, double, uint32_t&>;

	static_assert(std::is_same_v<core::sub_pack_t<test_t, 0>, test_t>);
	static_assert(std::is_same_v<core::sub_pack_t<test_t, 3>, core::pack<void, int64_t const, double, uint32_t&>>);
	static_assert(std::is_same_v<core::sub_pack_t<test_t, 3, 2>, core::pack<void, int64_t const>>);
	static_assert(std::is_same_v<core::sub_pack_t<test_t, 4, 3>, core::pack<int64_t const, double, uint32_t&>>);
	static_assert(std::is_same_v<core::sub_pack_t<test_t, 0, 3>, core::pack<uint32_t, float, void*>>);
	static_assert(std::is_same_v<core::sub_pack_t<test_t, 0, 0>, core::pack<>>);
	static_assert(std::is_same_v<core::sub_pack_t<test_t, 6, 0>, core::pack<>>);
	static_assert(std::is_same_v<core::sub_pack_t<core::pack<>, 0, 0>, core::pack<>>);
}


template<typename T>
struct filter_test_t: std::false_type {};

template<typename T> requires std::is_integral_v<T>
struct filter_test_t<T>: std::true_type {};

TEST(core_pack, pack_filter)
{
	using test_t = core::pack<uint32_t, float, void*, void, int64_t const, double, uint32_t&>;
	static_assert(std::is_same_v<core::pack_filter_t<test_t, filter_test_t>, core::pack<uint32_t, int64_t const>>);
}


template<typename T>
struct transform_test_t { using type = T; };

template<typename T> requires std::is_integral_v<T>
struct transform_test_t<T>
{
	using type = std::conditional_t<std::is_signed_v<T>, std::make_unsigned_t<T>, std::make_signed_t<T>>;
};

template<typename T> requires std::is_floating_point_v<T>
struct transform_test_t<T>
{
	using type = T*;
};

TEST(core_pack, pack_transform)
{
	using test_t = core::pack<uint32_t, float, void*, void, int64_t const, double, uint32_t&>;

	static_assert(std::is_same_v<core::pack_transform_t<test_t, transform_test_t>, core::pack<int32_t, float*, void*, void, uint64_t const, double*, uint32_t&>>);
}


template<typename>
struct find_test_t: std::false_type {};

template<typename T> requires std::is_pointer_v<T>
struct find_test_t<T>: std::true_type {};

TEST(core_pack, pack_find)
{
	using test_t = core::pack<uint32_t, float, void*, void, int64_t const, double*, uint32_t&, int8_t>;
	static_assert(core::pack_find_v<test_t, find_test_t> == 2);
	static_assert(core::pack_find_v<test_t, find_test_t, 3> == 5);
	static_assert(core::pack_find_v<test_t, find_test_t, 3, 5> == core::pack_npos);
	static_assert(core::pack_find_v<test_t, find_test_t, 6> == core::pack_npos);
	static_assert(core::pack_find_v<test_t, find_test_t, 5> == 5);
	static_assert(core::pack_find_v<core::pack<>, find_test_t> == core::pack_npos);
}


TEST(core_pack, pack_contains)
{
	using test_t = core::pack<uint32_t, float, void*, void, int64_t const, double*, uint32_t&, int8_t>;
	static_assert(core::pack_contains_v<test_t, find_test_t> == true);
	static_assert(core::pack_contains_v<test_t, find_test_t, 3> == true);
	static_assert(core::pack_contains_v<test_t, find_test_t, 3, 5> == false);
	static_assert(core::pack_contains_v<test_t, find_test_t, 6> == false);
	static_assert(core::pack_contains_v<test_t, find_test_t, 5> == true);
	static_assert(core::pack_contains_v<core::pack<>, find_test_t> == false);
}


TEST(core_pack, pack_element_swap)
{
	using test_t = core::pack<uint32_t, float, void*, void, int64_t const, double*, uint32_t&, int8_t>;

	static_assert(std::is_same_v<core::pack_element_swap_t<core::pack<>, 0, 0>, core::pack<>>);
	static_assert(std::is_same_v<core::pack_element_swap_t<test_t, 0, 0>, test_t>);
	static_assert(std::is_same_v<core::pack_element_swap_t<test_t, 1, 1>, test_t>);
	static_assert(std::is_same_v<core::pack_element_swap_t<test_t, 2, 2>, test_t>);

	static_assert(std::is_same_v<core::pack_element_swap_t<test_t, 1, 2>, core::pack<uint32_t, void*, float, void, int64_t const, double*, uint32_t&, int8_t>>);
	static_assert(std::is_same_v<core::pack_element_swap_t<test_t, 7, 3>, core::pack<uint32_t, float, void*, int8_t, int64_t const, double*, uint32_t&, void>>);
	static_assert(std::is_same_v<core::pack_element_swap_t<test_t, 4, 6>, core::pack<uint32_t, float, void*, void, uint32_t&, double*, int64_t const, int8_t>>);
}


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
	static_assert(core::pack_count_v<core::pack<uint32_t, uint32_t&, uint32_t*, const uint32_t>> == 4);
	static_assert(core::pack_count_v<core::pack<float, double, void*, core::pack<float, double>, void*, void>> == 6);
}

TEST(core_pack, pack_get)
{
	using test_t = core::pack<uint32_t, float, void*, void, const int64_t, double, uint32_t&>;
	static_assert(std::is_same_v<core::pack_get_t<test_t, 0>, uint32_t>);
	static_assert(std::is_same_v<core::pack_get_t<test_t, 1>, float>);
	static_assert(std::is_same_v<core::pack_get_t<test_t, 2>, void*>);
	static_assert(std::is_same_v<core::pack_get_t<test_t, 3>, void>);
	static_assert(std::is_same_v<core::pack_get_t<test_t, 4>, const int64_t>);
	static_assert(std::is_same_v<core::pack_get_t<test_t, 5>, double>);
	static_assert(std::is_same_v<core::pack_get_t<test_t, 6>, uint32_t&>);
}

TEST(core_pack, pack_cat)
{
	using test_t1 = core::pack<>;
	using test_t2 = core::pack<void>;
	using test_t3 = core::pack<uint32_t, float>;
	using test_t4 = core::pack<const int32_t&, double>;
	using test_t5 = core::pack<void*, core::pack<uint64_t>>;

	static_assert(std::is_same_v<core::pack_cat_t<test_t1>, core::pack<>>);
	static_assert(std::is_same_v<core::pack_cat_t<test_t3, test_t4>, core::pack<uint32_t, float, const int32_t&, double>>);
	static_assert(std::is_same_v<core::pack_cat_t<test_t1, test_t5, test_t2>, core::pack<void*, core::pack<uint64_t>, void>>);
}

TEST(core_pack, sub_pack)
{
	using test_t = core::pack<uint32_t, float, void*, void, const int64_t, double, uint32_t&>;

	static_assert(std::is_same_v<core::sub_pack_t<test_t, 0>, test_t>);
	static_assert(std::is_same_v<core::sub_pack_t<test_t, 3>, core::pack<void, const int64_t, double, uint32_t&>>);
	static_assert(std::is_same_v<core::sub_pack_t<test_t, 3, 2>, core::pack<void, const int64_t>>);
	static_assert(std::is_same_v<core::sub_pack_t<test_t, 4, 3>, core::pack<const int64_t, double, uint32_t&>>);
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
	using test_t = core::pack<uint32_t, float, void*, void, const int64_t, double, uint32_t&>;
	static_assert(std::is_same_v<core::pack_filter_t<test_t, filter_test_t>, core::pack<uint32_t, const int64_t>>);
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
	using test_t = core::pack<uint32_t, float, void*, void, const int64_t, double, uint32_t&>;

	static_assert(std::is_same_v<core::pack_transform_t<test_t, transform_test_t>, core::pack<int32_t, float*, void*, void, const uint64_t, double*, uint32_t&>>);
}


template<typename>
struct find_test_t: std::false_type {};

template<typename T> requires std::is_pointer_v<T>
struct find_test_t<T>: std::true_type {};

TEST(core_pack, pack_find)
{
	using test_t = core::pack<uint32_t, float, void*, void, const int64_t, double*, uint32_t&, int8_t>;
	static_assert(core::pack_find_v<test_t, find_test_t> == 2);
	static_assert(core::pack_find_v<test_t, find_test_t, 3> == 5);
	static_assert(core::pack_find_v<test_t, find_test_t, 3, 5> == core::pack_npos);
	static_assert(core::pack_find_v<test_t, find_test_t, 6> == core::pack_npos);
	static_assert(core::pack_find_v<test_t, find_test_t, 5> == 5);
	static_assert(core::pack_find_v<core::pack<>, find_test_t> == core::pack_npos);
}


TEST(core_pack, pack_contains)
{
	using test_t = core::pack<uint32_t, float, void*, void, const int64_t, double*, uint32_t&, int8_t>;
	static_assert(core::pack_contains_v<test_t, find_test_t> == true);
	static_assert(core::pack_contains_v<test_t, find_test_t, 3> == true);
	static_assert(core::pack_contains_v<test_t, find_test_t, 3, 5> == false);
	static_assert(core::pack_contains_v<test_t, find_test_t, 6> == false);
	static_assert(core::pack_contains_v<test_t, find_test_t, 5> == true);
	static_assert(core::pack_contains_v<core::pack<>, find_test_t> == false);
}


TEST(core_pack, pack_element_swap)
{
	using test_t = core::pack<uint32_t, float, void*, void, const int64_t, double*, uint32_t&, int8_t>;

	static_assert(std::is_same_v<core::pack_element_swap_t<core::pack<>, 0, 0>, core::pack<>>);
	static_assert(std::is_same_v<core::pack_element_swap_t<test_t, 0, 0>, test_t>);
	static_assert(std::is_same_v<core::pack_element_swap_t<test_t, 1, 1>, test_t>);
	static_assert(std::is_same_v<core::pack_element_swap_t<test_t, 2, 2>, test_t>);

	static_assert(std::is_same_v<core::pack_element_swap_t<test_t, 1, 2>, core::pack<uint32_t, void*, float, void, const int64_t, double*, uint32_t&, int8_t>>);
	static_assert(std::is_same_v<core::pack_element_swap_t<test_t, 7, 3>, core::pack<uint32_t, float, void*, int8_t, const int64_t, double*, uint32_t&, void>>);
	static_assert(std::is_same_v<core::pack_element_swap_t<test_t, 4, 6>, core::pack<uint32_t, float, void*, void, uint32_t&, double*, const int64_t, int8_t>>);
}

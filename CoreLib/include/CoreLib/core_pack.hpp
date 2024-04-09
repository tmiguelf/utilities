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

#pragma once

#include <cstdint>
#include <type_traits>
#include <limits>
#include <utility>

namespace core
{

	template<typename ...>
	struct pack {};


	template<typename>
	struct is_pack: std::false_type {};

	template<typename ...T>
	struct is_pack<pack<T...>> : public std::true_type{};

	template<typename T>
	constexpr bool is_pack_v = is_pack<T>::value;

	template<typename T>
	concept c_pack = is_pack<T>::value;


	//======== ======== pack utilities ======== ========

	/// \brief Counts elements in a pack
	template<c_pack>
	struct pack_count {};

	template<>
	struct pack_count<pack<>> { static constexpr uintptr_t value = uintptr_t{0}; };

	template<typename T1, typename ...T2>
	struct pack_count<pack<T1, T2...>> { static constexpr uintptr_t value = uintptr_t{1} + pack_count<pack<T2...>>::value; };

	template<c_pack T>
	constexpr uintptr_t pack_count_v = pack_count<T>::value;

	template<c_pack T>
	struct is_pack_empty: std::false_type {};

	template<>
	struct is_pack_empty<pack<>>: std::true_type {};

	template<c_pack T>
	constexpr bool is_pack_empty_v = is_pack_empty<T>::value;


	/// \brief Retrieves a specific element from a pack
	template<c_pack, uintptr_t>
	struct pack_get {};

	template<typename T1, typename ...T2>
	struct pack_get<pack<T1, T2...>, 0> { using type = T1; };

	template<uintptr_t Index, typename T1, typename ...T2>
	struct pack_get<pack<T1, T2...>, Index> { using type = typename pack_get<pack<T2...>, Index -1>::type; };

	template<c_pack T, uintptr_t Index>
	using pack_get_t = typename pack_get<T, Index>::type;


	/// \brief Joins multiple packs toghether
	template<c_pack ...>
	struct pack_cat {};

	template<>
	struct pack_cat<> { using type = pack<>; };

	template<typename ...T1>
	struct pack_cat<pack<T1...>> { using type = pack<T1...>; };

	template<typename ...T1, typename ...T2, c_pack ...T3>
	struct pack_cat<pack<T1...>, pack<T2...>, T3...> { using type = pack_cat<pack<T1..., T2...>, T3...>::type; };

	template<c_pack ...T>
	using pack_cat_t = pack_cat<T...>::type;

	/// \brief Gets a sub pack interval
	/// \tparam Pack_t - source pack
	/// \tparam StartIndex - first index to keep
	/// \tparam Size - size of new pack
	template<c_pack Pack_t, uintptr_t StartIndex, uintptr_t Size = pack_count_v<Pack_t> - StartIndex>
	struct sub_pack
	{
	private:

		template <class IndexSequence>
		struct conv_pack;

		template <std::uintptr_t... indices>
		struct conv_pack<std::index_sequence<indices...>> { using type = pack<pack_get_t<Pack_t, StartIndex + indices> ...>; };

	public:
		using type = conv_pack<std::make_index_sequence<Size>>::type;
	};

	template<c_pack Pack_t, uintptr_t StartIndex, uintptr_t Size = pack_count_v<Pack_t> - StartIndex>
	using sub_pack_t = typename sub_pack<Pack_t, StartIndex, Size>::type;



	/// \brief Gets a pack with only the members that satisfy a preicate
	/// \tparam Pack_t - source pack
	/// \tparam Predicate - Predicate template with a value that evaluates to true when to keep, and false when to discard
	template<c_pack Pack_t, template <typename> typename Predicate>
	struct pack_filter
	{
	private:
		template <class IndexSequence>
		struct conv_filter;

		template <std::uintptr_t... indices>
		struct conv_filter<std::index_sequence<indices...>>
		{
			
			using type = pack_cat_t<
				std::conditional_t<Predicate<pack_get_t<Pack_t, indices>>::value, pack<pack_get_t<Pack_t, indices>>, pack<>>...
			>;
		};

	public:
		using type = typename conv_filter<std::make_index_sequence<pack_count_v<Pack_t>>>::type;
	};

	template<c_pack Pack_t, template <typename> typename Predicate>
	using pack_filter_t = typename pack_filter<Pack_t, Predicate>::type;

	/// \brief Transforms all types in a pack in acordance to a transformation template
	/// \tparam Pack_t - pack to transform
	/// \tparam Transformation - Transformation template
	template<c_pack Pack_t, template <typename> typename Transformation>
	struct pack_transform
	{
	private:
		template <class IndexSequence>
		struct conv_pack;

		template <std::uintptr_t... indices>
		struct conv_pack<std::index_sequence<indices...>>
		{
			using type = pack<typename Transformation<pack_get_t<Pack_t, indices>>::type ...>;
		};

	public:
		using type = typename conv_pack<std::make_index_sequence<pack_count_v<Pack_t>>>::type;
	};

	template<c_pack Pack_t, template <typename> typename Transformation>
	using pack_transform_t = typename pack_transform<Pack_t, Transformation>::type;



	static constexpr uintptr_t pack_npos = std::numeric_limits<uintptr_t>::max();
	
	/// \brief Gets the index of the type that satisfies the Predicate
	/// \tparam Pack_t - pack to evaluate
	/// \tparam Predicate - Checking condition
	/// \tparam StartIndex - first index to start searching
	/// \tparam EndIndex - 1 past last index to check
	template<c_pack Pack_t, template <typename> typename Predicate,
		uintptr_t StartIndex = 0, uintptr_t EndIndex = pack_count_v<Pack_t>>
	struct pack_find
	{
	private:
		template<uintptr_t Index = StartIndex>
		static consteval uintptr_t find()
		{
			if constexpr (Index < EndIndex)
			{
				if constexpr (Predicate<pack_get_t<Pack_t, Index>>::value)
				{
					return Index;
				}
				else
				{
					return find<Index + 1>();
				}
			}
			else
			{
				return pack_npos;
			}
		}

	public:
		static constexpr uintptr_t value = find();
	};

	template<c_pack Pack_t, template <typename> typename Predicate,
		uintptr_t StartIndex = 0, uintptr_t EndIndex = pack_count_v<Pack_t>>
	static constexpr uintptr_t pack_find_v = pack_find<Pack_t, Predicate, StartIndex, EndIndex>::value;


	/// \brief Checks if any of the types in the pack satisfies the Predicate
	/// \tparam Pack_t - pack to evaluate
	/// \tparam Predicate - Checking condition
	/// \tparam StartIndex - first index to start checking
	/// \tparam EndIndex - 1 past last index to check
	template<c_pack Pack_t, template <typename> typename Predicate,
		uintptr_t StartIndex = 0, uintptr_t EndIndex = pack_count_v<Pack_t>>
	struct pack_contains
	{
	private:
		template<uintptr_t Index = StartIndex>
		static consteval bool check()
		{
			if constexpr (Index < EndIndex)
			{
				if constexpr (Predicate<pack_get_t<Pack_t, Index>>::value)
				{
					return true;
				}
				else
				{
					return check<Index + 1>();
				}
			}
			else
			{
				return false;
			}
		}

	public:
		static constexpr bool value = check();
	};

	template<c_pack Pack_t, template <typename> typename Predicate,
		uintptr_t StartIndex = 0, uintptr_t EndIndex = pack_count_v<Pack_t>>
	static constexpr bool pack_contains_v = pack_contains<Pack_t, Predicate, StartIndex, EndIndex>::value;


	/// \brief Swap 2 elements in a pack
	template <c_pack Pack_t, std::uintptr_t I, std::uintptr_t J>
	struct pack_element_swap
	{
	private:
		template <class IndexSequence>
		struct pack_element_swap_impl;

		template <std::uintptr_t... indices>
		struct pack_element_swap_impl<std::index_sequence<indices...>>
		{
			using type = pack<pack_get_t<Pack_t, (indices != I && indices != J) ? indices : ((indices == I) ? J : I)>...>;
		};

	public:
		using type = typename pack_element_swap_impl<std::make_index_sequence<pack_count_v<Pack_t>>>::type;
	};

	template <c_pack Pack_t, std::uintptr_t I, std::uintptr_t J>
	using pack_element_swap_t = typename pack_element_swap<Pack_t, I, J>::type;

} //namespace core

//======== ======== ======== ======== ======== ======== ======== ========
///	\file
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
///
///	\todo	Provide a comprehensive and consistent set of error codes, to give
///			extra information regarding he nature of the failure
//======== ======== ======== ======== ======== ======== ======== ========

#pragma once

#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include <CoreLib/Core_Type.hpp>
#include <CoreLib/Core_Alloca.hpp>
#include <CoreLib/string/core_wchar_alias.hpp>

#include "toPrint_sink.hpp"
#include "toPrint_encoders.hpp"
#include "toPrint_support.hpp"

namespace core::_p
{
	template<typename T>
	struct transform_toPrint_sink
	{
	private:
		using evaluated_t = std::remove_cvref_t<T>;
	public:
		using type = std::conditional_t<is_sink_toPrint_v<evaluated_t>, T&, ::core::sink_toPrint<evaluated_t>>;
	};
	
	template <c_tuple Tuple>
	struct tuple_toPrint
	{
	private:
		static constexpr uintptr_t tuple_size = std::tuple_size_v<Tuple>;
	
		template<uintptr_t Pos>
		struct transform
		{
			using evaluated_t = std::remove_cvref_t<decltype(std::get<Pos>(std::declval<Tuple>()))>;
			using current_t = std::conditional_t<is_toPrint_v<evaluated_t>, const evaluated_t&, ::core::toPrint<evaluated_t>>;
			using type = decltype(std::tuple_cat(std::declval<std::tuple<current_t>>(), std::declval<transform<Pos + 1>::type>()));
		};
	
		template<>
		struct transform<tuple_size>
		{
			using type = std::tuple<>;
		};
	
	public:
		using type = transform<0>::type;
	};
	
	template <c_tuple Tuple>
	struct is_all_toPrint
	{
	private:
		static constexpr uintptr_t tuple_size = std::tuple_size_v<Tuple>;
	
		template<uintptr_t Pos>
		struct count
		{
			using evaluated_t = std::remove_cvref_t<decltype(std::get<Pos>(std::declval<Tuple>()))>;
			static constexpr bool value = is_toPrint_v<evaluated_t> && count<Pos + 1>::value;
		};
	
		template<>
		struct count<tuple_size>: public std::true_type{};
	
	public:
		static constexpr bool value = count<0>::value;
	};
	
	template<typename> struct is_tuple_toPrint: public std::false_type{};
	template<c_tuple T> struct is_tuple_toPrint<T>: public is_all_toPrint<T>{};
	template<typename T>
	concept c_tuple_toPrint = is_tuple_toPrint<T>::value;

	template<c_toPrint_char Char_t, c_tuple Tuple>
	struct tuple_toPrint_or_string_view { using type = tuple_toPrint<Tuple>::type; };
	
	template<c_toPrint_char Char_t>
	struct tuple_toPrint_or_string_view<Char_t, std::tuple<std::basic_string_view<Char_t>>> { using type = std::basic_string_view<Char_t>; };
	
	template<c_toPrint_char Char_t>
	struct tuple_toPrint_or_string_view<Char_t, std::tuple<std::basic_string<Char_t>>> { using type = std::basic_string_view<Char_t>; };
	
	template<>
	struct tuple_toPrint_or_string_view<char8_t, std::tuple<std::string_view>> { using type = std::string_view; };
	
	template<>
	struct tuple_toPrint_or_string_view<char8_t, std::tuple<std::string>> { using type = std::string_view; };

	template<>
	struct tuple_toPrint_or_string_view<wchar_alias, std::tuple<std::wstring_view>> { using type = std::wstring_view; };

	template<>
	struct tuple_toPrint_or_string_view<wchar_alias, std::tuple<std::wstring>> { using type = std::wstring_view; };

	template<c_toPrint_char CharT>
	struct toPrint_assist
	{
	private:
		template<c_tuple_toPrint Tuple, uintptr_t Pos = 0>
		static inline uintptr_t count_toPrint(const Tuple& p_tuple, uintptr_t* p_sizeTable)
		{
			const auto& res = std::get<Pos>(p_tuple);
			const uintptr_t size = res.size(CharT{0});
			if constexpr (Pos + 1 < std::tuple_size_v<Tuple>)
			{
				*p_sizeTable = size;
				return size + count_toPrint<Tuple, Pos + 1>(p_tuple, p_sizeTable + 1);
			}
			else
			{
				return size;
			}
		}

		template<c_tuple_toPrint Tuple, uintptr_t Pos = 0>
		static inline void fill_toPrint(
			const Tuple& p_tuple,
			const uintptr_t* p_sizeTable,
			CharT* p_buff)
		{
			const auto& res = std::get<Pos>(p_tuple);
			res.getPrint(p_buff);

			if constexpr (Pos + 1 < std::tuple_size_v<Tuple>)
			{
				fill_toPrint<Tuple, Pos + 1>(p_tuple, p_sizeTable + 1, p_buff + *p_sizeTable);
			}
		}

		template<typename Sink, c_tuple_toPrint Tuple> requires is_sink_toPrint_v<Sink>
		static void finish_toPrint(
			Sink& p_sink,
			const Tuple& p_data,
			const uintptr_t* p_sizeTable,
			CharT* p_buff,
			uintptr_t p_size)
		{
			constexpr uintptr_t tuple_size = std::tuple_size_v<Tuple>;
			fill_toPrint(p_data, p_sizeTable, p_buff);
			p_sink.write(std::basic_string_view<CharT>{p_buff, p_size});
		}

	public:
		template<typename Sink> requires (is_sink_toPrint_v<Sink>)
		static inline void push_toPrint(Sink& p_sink, std::basic_string_view<CharT> p_message)
		{
			p_sink.write(p_message);
		};

		template<typename Sink> requires (is_sink_toPrint_v<Sink> && std::is_same_v<CharT, char8_t>)
		static inline void push_toPrint(Sink& p_sink, std::string_view p_message)
		{
			p_sink.write(std::u8string_view{reinterpret_cast<const char8_t*>(p_message.data()), p_message.size()});
		};

		template<typename Sink> requires (is_sink_toPrint_v<Sink> && std::is_same_v<CharT, wchar_alias>)
		static inline void push_toPrint(Sink& p_sink, std::wstring_view p_message)
		{
			p_sink.write(std::basic_string_view<wchar_alias>{reinterpret_cast<const wchar_alias*>(p_message.data()), p_message.size()});
		};

		template<typename Sink> requires is_sink_toPrint_v<Sink>
		static inline void push_toPrint(Sink& p_sink)
		{
			p_sink.write(std::u8string_view{nullptr, 0});
		};

		template<typename Sink, c_tuple_toPrint Tuple> requires is_sink_toPrint_v<Sink>
	#if defined(_MSC_BUILD)
		__declspec(noinline)
	#else
		__attribute__((noinline))
	#endif
		static void push_toPrint(Sink& p_sink, const Tuple& p_data)
		{
			constexpr uintptr_t tuple_size = std::tuple_size_v<Tuple>;
			if constexpr (tuple_size > 0)
			{
				std::array<uintptr_t, tuple_size -1> sizeTable;
				const uintptr_t char_count = count_toPrint(p_data, sizeTable.data());
				if(char_count > 0)
				{
					constexpr uintptr_t alloca_treshold = (0x10000 / sizeof(CharT));

					if(char_count > alloca_treshold)
					{
						std::vector<CharT> buff;
						buff.resize(char_count);
						finish_toPrint(p_sink, p_data, sizeTable.data(), buff.data(), char_count);
					}
					else
					{
						CharT* buff = reinterpret_cast<CharT*>(core_alloca(char_count * sizeof(CharT)));
						finish_toPrint(p_sink, p_data, sizeTable.data(), buff, char_count);
					}
					return;
				}
			}
			p_sink.write(std::u8string_view{nullptr, 0});
		};
	};

} //namespace core::_p

#define core_ToPrint(CharT, Sink, ...) \
	::core::_p::toPrint_assist<CharT>::push_toPrint( \
			::core::_p::transform_toPrint_sink<decltype(Sink)>::type(Sink), \
			::core::_p::tuple_toPrint_or_string_view<CharT, decltype(::std::make_tuple(__VA_ARGS__))>::type(__VA_ARGS__));

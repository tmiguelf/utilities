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

#include "toPrint_sink.hpp"
#include "toPrint_encoders.hpp"

namespace core::_p
{
	template<typename T>
	struct transform_toPrint_sink
	{
	private:
		using evaluated_t = std::remove_cvref_t<T>;
	public:
		using type = std::conditional_t<is_toPrint_sink_v<evaluated_t>, const T&, ::core::toPrint_sink<evaluated_t>>;
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
			static constexpr bool value = is_valid_toPrint_v<evaluated_t> && count<Pos + 1>::value;
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
	
	
	template <c_tuple Tuple>
	struct tuple_toPrint_or_string_view { using type = tuple_toPrint<Tuple>::type; };
	
	template <>
	struct tuple_toPrint_or_string_view<std::tuple<std::u8string_view>> { using type = std::u8string_view; };
	
	template <>
	struct tuple_toPrint_or_string_view<std::tuple<std::u8string>> { using type = std::u8string_view; };
	
	template <>
	struct tuple_toPrint_or_string_view<std::tuple<std::string_view>> { using type = std::string_view; };
	
	template <>
	struct tuple_toPrint_or_string_view<std::tuple<std::string>> { using type = std::string_view; };


	template<typename Tuple, uintptr_t Pos = 0>
	inline uintptr_t count_toPrint(const Tuple& p_tuple)
	{
		const auto& res = std::get<Pos>(p_tuple);
		if constexpr (Pos + 1 < std::tuple_size_v<Tuple>)
		{
			return res.size() + count_toPrint<Tuple, Pos + 1>(p_tuple);
		}
		else
		{
			return res.size();
		}
	}

	template <typename Tuple, uintptr_t Pos = 0>
	inline void fill_toPrint(const Tuple& p_tuple, char8_t* p_buff)
	{
		const auto& res = std::get<Pos>(p_tuple);
		res.get(p_buff);

		if constexpr (Pos + 1 < std::tuple_size_v<Tuple>)
		{
			fill_toPrint<Tuple, Pos + 1>(p_tuple, p_buff + res.size());
		}
	}

	template<c_toPrint_sink Sink>
	inline void push_toPrint(const Sink& p_sink, const std::u8string_view& p_message)
	{
		p_sink.write(p_message);
	};

	template<c_toPrint_sink Sink>
	inline void push_toPrint(const Sink& p_sink, const std::string_view& p_message)
	{
		p_sink.write(std::u8string_view{reinterpret_cast<const char8_t*>(p_message.data()), p_message.size()});
	};

	template<c_toPrint_sink Sink>
	inline void push_toPrint(const Sink& p_sink)
	{
		p_sink.write(std::u8string_view{nullptr, 0});
	};

	template<c_toPrint_sink Sink>
	inline void push_toPrint(const Sink& p_sink, const std::tuple<>&)
	{
		p_sink.write(std::u8string_view{nullptr, 0});
	};

	template<c_toPrint_sink Sink, c_tuple_toPrint Tuple>
	void finish_toPrint(const Sink& p_sink, const Tuple& p_data, char8_t* p_buff, uintptr_t p_size)
	{
		constexpr uintptr_t tuple_size = std::tuple_size_v<Tuple>;
		fill_toPrint(p_data, p_buff);
		p_sink.write(std::u8string_view{p_buff, p_size});
	}

	template<c_toPrint_sink Sink, c_tuple_toPrint Tuple> requires (std::tuple_size_v<Tuple> > 0)
#if defined(_MSC_BUILD)
	__declspec(noinline)
#else
	__attribute__((noinline))
#endif
	void push_toPrint(const Sink& p_sink, const Tuple& p_data)
	{
		//constexpr uintptr_t tuple_size = std::tuple_size_v<Tuple>;
		//if constexpr (tuple_size > 0)
		{
			const uintptr_t char_count = count_toPrint(p_data);
			if(char_count > 0)
			{
				constexpr uintptr_t alloca_treshold = 0x10000;

				if(char_count > alloca_treshold)
				{
					std::vector<char8_t> buff;
					buff.resize(char_count);
					finish_toPrint(p_sink, p_data, buff.data(), char_count);
				}
				else
				{
					char8_t* buff = reinterpret_cast<char8_t*>(core_alloca(char_count));
					finish_toPrint(p_sink, p_data, buff, char_count);
				}
				return;
			}
		}
		p_sink.write(std::u8string_view{nullptr, 0});
	};

} //namespace core::_p

#define core_ToPrint(Sink, ...) \
	::core::_p::push_toPrint( \
			::core::_p::transform_toPrint_sink<decltype(Sink)>::type{Sink}, \
			::core::_p::tuple_toPrint_or_string_view<decltype(::std::make_tuple(__VA_ARGS__))>::type(__VA_ARGS__));

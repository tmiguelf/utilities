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
///
///	\todo	Provide a comprehensive and consistent set of error codes, to give
///			extra information regarding he nature of the failure
//======== ======== ======== ======== ======== ======== ======== ========

#pragma once

#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include <CoreLib/core_extra_compiler.hpp>
#include <CoreLib/core_type.hpp>
#include <CoreLib/core_alloca.hpp>
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
		static constexpr bool is_sink = is_sink_toPrint_v<evaluated_t>;
		
	public:
		using type = std::conditional_t<is_sink_toPrint_v<evaluated_t>,
			evaluated_t,
			::core::sink_toPrint<evaluated_t>>;
	};
	
	template <c_tuple Tuple>
	struct tuple_toPrint
	{
	private:
		static constexpr uintptr_t tuple_size = std::tuple_size_v<Tuple>;
	
		template<uintptr_t Pos, uintptr_t TSize = tuple_size>
		struct transform
		{
			using evaluated_t = std::remove_cvref_t<std::tuple_element_t<Pos, Tuple>>;
			using current_t   = std::conditional_t<is_toPrint_v<evaluated_t>, evaluated_t const&, ::core::toPrint<evaluated_t>>;
			using type = decltype(std::tuple_cat(std::declval<std::tuple<current_t>>(), std::declval<typename transform<Pos + 1>::type>()));
		};

		template<uintptr_t Pos>
		struct transform<Pos, Pos>
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
	
		template<uintptr_t Pos, uintptr_t TSize = tuple_size>
		struct check
		{
			using evaluated_t = std::remove_cvref_t<std::tuple_element_t<Pos, Tuple>>;
			static constexpr bool value = is_toPrint_v<evaluated_t> && check<Pos + 1>::value;
		};

		template<uintptr_t Pos>
		struct check<Pos, Pos>: public std::true_type{};

	public:
		static constexpr bool value = check<0>::value;
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
	struct toPrint_fill_assist
	{
		template<c_tuple_toPrint Tuple, uintptr_t Pos = 0>
		static inline uintptr_t count_toPrint(Tuple const& p_tuple, uintptr_t* const p_sizeTable)
		{
			auto const& res = std::get<Pos>(p_tuple);
			uintptr_t const size = res.size(CharT{0});
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
			Tuple const&			p_tuple,
			uintptr_t const* const	p_sizeTable,
			CharT* const			p_buff)
		{
			auto const& res = std::get<Pos>(p_tuple);
			res.get_print(p_buff);

			if constexpr (Pos + 1 < std::tuple_size_v<Tuple>)
			{
				fill_toPrint<Tuple, Pos + 1>(p_tuple, p_sizeTable + 1, p_buff + *p_sizeTable);
			}
		}
	};


	template<c_toPrint_char CharT>
	struct toPrint_assist
	{
	private:

		template<typename Sink, c_tuple_toPrint Tuple> requires is_valid_sink_toPrint_v<CharT, Sink>
		static void finish_toPrint(
			Sink& p_sink,
			Tuple const& p_data,
			uintptr_t const* const	p_sizeTable,
			CharT* const			p_buff,
			uintptr_t const			p_size)
		{
			toPrint_fill_assist<CharT>::fill_toPrint(p_data, p_sizeTable, p_buff);
			p_sink.write(std::basic_string_view<CharT>{p_buff, p_size});
		}

		template<typename Sink, c_tuple_toPrint Tuple> requires is_valid_sink_toPrint_v<CharT, Sink>
		static void finish_toPrint(
			Sink const& p_sink,
			Tuple const& p_data,
			uintptr_t const* const	p_sizeTable,
			CharT* const			p_buff,
			uintptr_t const			p_size)
		{
			toPrint_fill_assist<CharT>::fill_toPrint(p_data, p_sizeTable, p_buff);
			p_sink.write(std::basic_string_view<CharT>{p_buff, p_size});
		}

	public:
		template<typename Sink> requires is_valid_sink_toPrint_v<CharT, Sink>
		static inline void push_toPrint(Sink const& p_sink)
		{
			p_sink.write(std::basic_string_view<CharT>{nullptr, 0});
		};

		template<typename Sink> requires is_valid_sink_toPrint_v<CharT, Sink>
		static inline void push_toPrint(Sink& p_sink)
		{
			p_sink.write(std::basic_string_view<CharT>{nullptr, 0});
		};


		template<typename Sink> requires is_valid_sink_toPrint_v<CharT, Sink>
		static inline void push_toPrint(Sink& p_sink, std::basic_string_view<CharT> const p_message)
		{
			p_sink.write(p_message);
		};

		template<typename Sink> requires is_valid_sink_toPrint_v<CharT, Sink>
		static inline void push_toPrint(Sink const& p_sink, std::basic_string_view<CharT> const p_message)
		{
			p_sink.write(p_message);
		};

		template<typename Sink> requires is_valid_sink_toPrint_v<CharT, Sink> && std::is_same_v<CharT, char8_t>
		static inline void push_toPrint(Sink& p_sink, std::string_view const p_message)
		{
			push_toPrint(p_sink, std::u8string_view{reinterpret_cast<char8_t const*>(p_message.data()), p_message.size()});
		};

		template<typename Sink> requires is_valid_sink_toPrint_v<CharT, Sink> && std::is_same_v<CharT, char8_t>
		static inline void push_toPrint(Sink const& p_sink, std::string_view const p_message)
		{
			push_toPrint(p_sink, std::u8string_view{reinterpret_cast<char8_t const*>(p_message.data()), p_message.size()});
		};

		template<typename Sink> requires is_valid_sink_toPrint_v<CharT, Sink> && std::is_same_v<CharT, wchar_alias>
		static inline void push_toPrint(Sink& p_sink, std::wstring_view const p_message)
		{
			push_toPrint(p_sink, std::basic_string_view<wchar_alias>{reinterpret_cast<wchar_alias const*>(p_message.data()), p_message.size()});
		};

		template<typename Sink> requires is_valid_sink_toPrint_v<CharT, Sink> && std::is_same_v<CharT, wchar_alias>
		static inline void push_toPrint(Sink const& p_sink, std::wstring_view const p_message)
		{
			push_toPrint(p_sink, std::basic_string_view<wchar_alias>{reinterpret_cast<wchar_alias const*>(p_message.data()), p_message.size()});
		};

		template<typename Sink, c_tuple_toPrint Tuple> requires is_valid_sink_toPrint_v<CharT, Sink>
		NO_INLINE static void push_toPrint(Sink& p_sink, Tuple const& p_data)
		{
			constexpr uintptr_t tuple_size = std::tuple_size_v<Tuple>;
			if constexpr (tuple_size > 0)
			{
				std::array<uintptr_t, tuple_size -1> sizeTable;
				uintptr_t const char_count = toPrint_fill_assist<CharT>::count_toPrint(p_data, sizeTable.data());
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
			p_sink.write(std::basic_string_view<CharT>{nullptr, 0});
		};

		template<typename Sink, c_tuple_toPrint Tuple> requires is_valid_sink_toPrint_v<CharT, Sink>
		NO_INLINE static void push_toPrint(Sink const& p_sink, Tuple const& p_data)
		{
			constexpr uintptr_t tuple_size = std::tuple_size_v<Tuple>;
			if constexpr (tuple_size > 0)
			{
				std::array<uintptr_t, tuple_size -1> sizeTable;
				uintptr_t const char_count = toPrint_fill_assist<CharT>::count_toPrint(p_data, sizeTable.data());
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
			p_sink.write(std::basic_string_view<CharT>{nullptr, 0});
		};
	};

} //namespace core::_p

namespace core
{
	template<typename Char_t, typename Sink, typename... Args>
	inline void print(Sink& sink, Args const&... args)
	{
		using args_t = ::core::_p::tuple_toPrint_or_string_view<Char_t, decltype(::std::make_tuple(args...))>::type;
		if constexpr(::core::_p::is_sink_toPrint_v<Sink>)
		{
			::core::_p::toPrint_assist<Char_t>::push_toPrint(sink, args_t(args...));
		}
		else
		{
			using compatible_sink_t = ::core::_p::transform_toPrint_sink<Sink>::type;
			compatible_sink_t real_sink{sink};
			::core::_p::toPrint_assist<Char_t>::push_toPrint(real_sink, args_t(args...));
		}
	}

	template<typename Char_t, typename Sink, typename... Args>
	inline void print(Sink const& sink, Args const&... args)
	{
		using args_t = ::core::_p::tuple_toPrint_or_string_view<Char_t, decltype(::std::make_tuple(args...))>::type;
		if constexpr(::core::_p::is_sink_toPrint_v<Sink>)
		{
			::core::_p::toPrint_assist<Char_t>::push_toPrint(sink, args_t(args...));
		}
		else
		{
			using compatible_sink_t = ::core::_p::transform_toPrint_sink<Sink>::type;
			compatible_sink_t real_sink{sink};
			::core::_p::toPrint_assist<Char_t>::push_toPrint(real_sink, args_t(args...));
		}
	}
} //namespace core

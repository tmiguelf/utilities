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
#include <vector>

#include <CoreLib/core_extra_compiler.hpp>
#include <CoreLib/core_type.hpp>
#include <CoreLib/core_alloca.hpp>
#include <CoreLib/string/core_wchar_alias.hpp>
#include <CoreLib/core_pack.hpp>

#include "toPrint_sink.hpp"
#include "toPrint_encoders.hpp"
#include "toPrint_support.hpp"


namespace core
{

	namespace _p
	{

		FORCE_INLINE uintptr_t add_ret(uintptr_t ret, uintptr_t& state)
		{
			state += ret;
			return ret;
		}

		template<typename T> requires is_toPrint_v<std::remove_cvref_t<T>>
		FORCE_INLINE T const& to_print_transform(T const& obj)
		{
			return obj;
		}

		template<typename T> requires (!is_toPrint_v<std::remove_cvref_t<T>>)
		FORCE_INLINE auto to_print_transform(T const& obj)
		{
			return ::core::toPrint<std::remove_cvref_t<T>>(obj);
		}

		template<typename CharT, typename T>
		struct to_print_short_opt: std::false_type{};

		template<typename CharT>
		struct to_print_short_opt<CharT, core::pack<std::basic_string_view<CharT>>>: std::true_type{ static constexpr bool type_alias = false; };

		template<typename CharT>
		struct to_print_short_opt<CharT, core::pack<std::basic_string<CharT>>>: std::true_type{ static constexpr bool type_alias = false; };

		template<>
		struct to_print_short_opt<char8_t, core::pack<std::basic_string_view<char>>>: std::true_type{ static constexpr bool type_alias = true; };

		template<>
		struct to_print_short_opt<char8_t, core::pack<std::basic_string<char>>>: std::true_type{ static constexpr bool type_alias = true; };

		template<>
		struct to_print_short_opt<wchar_alias, core::pack<std::basic_string_view<wchar_t>>>: std::true_type{ static constexpr bool type_alias = true; };

		template<>
		struct to_print_short_opt<wchar_alias, core::pack<std::basic_string<wchar_t>>>: std::true_type{ static constexpr bool type_alias = true; };


		template<c_toPrint_char CharT>
		struct toPrint_assist
		{
		private:

			template<typename Sink, typename... Args> requires 
			(
				is_valid_sink_toPrint_v<CharT, Sink> and
				not _p::toPrint_has_own_buffer<CharT, Sink>::value and
				( is_toPrint_v<Args> and... )
			)
			NO_INLINE static void push_toPrint(Sink& sink,  Args const&... args)
			{
				constexpr uintptr_t arg_count = sizeof...(Args);
				static_assert( arg_count > 0); 

				uintptr_t char_count = 0;
				std::array<uintptr_t const, arg_count> sizeTable { add_ret(args.size(CharT{}), char_count)... };

				if(char_count)
				{
					constexpr uintptr_t alloca_treshold = (0x10000 / sizeof(CharT));

					std::vector<CharT> buff2;
					CharT* buff;
					if(char_count > alloca_treshold)
					{
						buff2.resize(char_count);
						buff = buff2.data();
					}
					else
					{
						buff = reinterpret_cast<CharT*>(core_alloca(char_count * sizeof(CharT)));
					}

					CharT* const origin = buff;
					uintptr_t const* pSize = sizeTable.data();
					((args.get_print(buff), buff += *(pSize++)), ...);
					//((buff = args.get_print(buff)), ...);

					sink.write(std::basic_string_view<CharT>{origin, char_count});
					return;
				}
				sink.write(std::basic_string_view<CharT>{nullptr, 0});
			}

			template<typename Sink, typename... Args> requires (
				is_valid_sink_toPrint_v<CharT, Sink> and
				_p::toPrint_has_own_buffer<CharT, Sink>::value and
				( is_toPrint_v<Args> and... )
				)
			NO_INLINE static void push_toPrint(Sink& sink,  Args const&... args)
			{
				constexpr uintptr_t arg_count = sizeof...(Args);
				static_assert( arg_count > 0); 

				uintptr_t char_count = 0;
				std::array<uintptr_t const, arg_count> sizeTable { add_ret(args.size(CharT{}), char_count)... };

				if (char_count)
				{
					CharT* const buff = sink.buffer_acquire(char_count);
					CharT* pivot = buff;
					uintptr_t const* pSize = sizeTable.data();
					((args.get_print(pivot), pivot += *(pSize++)), ...);

					sink.buffer_released(buff, char_count);
				}
				else
				{
					CharT* const buff = sink.buffer_acquire(0);
					sink.buffer_released(buff, 0);
				}
			}

		public:
			template<typename Sink, typename... Args> requires
			(
				_p::is_sink_toPrint_v<Sink> and
				_p::toPrint_has_own_buffer<CharT, Sink>::value
			)
			FORCE_INLINE static void print(Sink& sink)
			{
				CharT* const buff = sink.buffer_acquire(0);
				sink.buffer_released(buff, 0);
			}

			template<typename Sink, typename... Args> requires
			(
				_p::is_sink_toPrint_v<Sink> and
				_p::toPrint_has_own_buffer<CharT, Sink>::value
			)
			FORCE_INLINE static void print(Sink& sink, Args const&... args)
			{
				push_toPrint(sink, _p::to_print_transform(args)...);
			}


			template<typename Sink, typename... Args> requires
			(
				_p::is_sink_toPrint_v<Sink> and
				not _p::toPrint_has_own_buffer<CharT, Sink>::value
			)
			FORCE_INLINE static void print(Sink& sink)
			{
				sink.write(std::basic_string_view<CharT>{nullptr, 0});
			}

			template<typename Sink, typename... Args> requires
			(
				_p::is_sink_toPrint_v<Sink> and
				not _p::toPrint_has_own_buffer<CharT, Sink>::value
			)
			FORCE_INLINE static void print(Sink& sink, Args const&... args)
			{
				static_assert(sizeof...(Args) > 0);
				if constexpr(sizeof...(Args) == 1 && _p::to_print_short_opt<CharT, core::pack<std::remove_cvref_t<Args>...>>::value)
				{
					if constexpr(_p::to_print_short_opt<CharT, core::pack<std::remove_cvref_t<Args>...>>::type_alias)
					{
						sink.write((std::basic_string_view<CharT>{reinterpret_cast<CharT const* const>(args.data()), args.size()})...);
					}
					else
					{
						sink.write(args...);
					}
				}
				else
				{
					push_toPrint(sink, _p::to_print_transform(args)...);
				}
			}

		};

	} //namespace _p


	template<_p::c_toPrint_char CharT, typename Sink, typename... Args>
	FORCE_INLINE void print(Sink& sink, Args const&... args)
	{
		if constexpr(::core::_p::is_sink_toPrint_v<Sink>)
		{
			::core::_p::toPrint_assist<CharT>::print(sink, args...);
		}
		else
		{
			using compatible_sink_t = ::core::sink_toPrint<std::remove_cvref_t<Sink>>;
			compatible_sink_t real_sink{sink};
			::core::_p::toPrint_assist<CharT>::print(real_sink, args...);
		}
	}

	template<_p::c_toPrint_char CharT, typename Sink, typename... Args>
	FORCE_INLINE void print(Sink&& sink, Args const&... args)
	{
		if constexpr(::core::_p::is_sink_toPrint_v<Sink>)
		{
			::core::_p::toPrint_assist<CharT>::print(sink, args...);
		}
		else
		{
			using compatible_sink_t = ::core::sink_toPrint<std::remove_cvref_t<Sink>>;
			compatible_sink_t real_sink{sink};
			::core::_p::toPrint_assist<CharT>::print(real_sink, args...);
		}
	}

} //namespace core

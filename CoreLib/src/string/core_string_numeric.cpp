//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///		Numeric string conversion utilities.
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

#include <array>
#include <bit>

#include <CoreLib/string/core_string_numeric.hpp>
#include <CoreLib/string/core_fp_charconv.hpp>

#include <CoreLib/core_type.hpp>

#include "fp_traits.hpp"

namespace core
{
	using ::core::literals::operator "" _ui8 ;
	using ::core::literals::operator "" _ui16;
	using ::core::literals::operator "" _ui32;
	using ::core::literals::operator "" _ui64;
	using ::core::literals::operator "" _uip;

	//======== Private implementation ========

	namespace
	{
		template <typename T>
		struct dec_unsigned_help
		{
			static constexpr uint64_t threshold_val	= std::numeric_limits<T>::max() / 10;
			static constexpr uint64_t last_digit	= std::numeric_limits<T>::max() % 10;
		};

		template <typename T>
		struct dec_signed_help
		{
			static constexpr int64_t max_threshold_val	= std::numeric_limits<T>::max() / 10;
			static constexpr int64_t max_last_digit		= std::numeric_limits<T>::max() % 10;

			static constexpr int64_t min_threshold_val	= std::numeric_limits<T>::min() / 10;
			static constexpr int64_t min_last_digit		= -(std::numeric_limits<T>::min() % 10);
		};

		template <typename T>
		struct hex_unsigned_help
		{
			static constexpr uint64_t threshold_val	= std::numeric_limits<T>::max() / 16;
		};

		template <typename T>
		struct bin_unsigned_help
		{
			static constexpr uint64_t threshold_val	= std::numeric_limits<T>::max() / 2;
		};
	} //namespace

	namespace _p
	{
		///	\brief	Converts a base 10 encoded string to unsigned integer.
		template <charconv_uint_c uint_T, charconv_char_c char_T>
		[[nodiscard]] static inline from_chars_result<uint_T> dec2uint(std::basic_string_view<char_T> const p_str)
		{
			if(p_str.empty())
			{
				return std::errc::invalid_argument;
			}

			uint_T r_val = 0;
			for(const char_T tchar: p_str)
			{
				const uint8_t t_val = static_cast<uint8_t>(tchar - '0');
				if(t_val > 9)
				{
					return std::errc::invalid_argument;
				}

				if(r_val >= dec_unsigned_help<uint_T>::threshold_val)
				{
					if(r_val > dec_unsigned_help<uint_T>::threshold_val || t_val > dec_unsigned_help<uint_T>::last_digit)
					{
						return std::errc::value_too_large;
					}
				}

				r_val = static_cast<uint_T>(r_val * 10 + t_val);
			}

			return r_val;
		}

		///	\brief		Converts a base 10 encoded string to signed integer.
		template <charconv_sint_c int_T, charconv_char_c char_T>
		[[nodiscard]] static inline from_chars_result<int_T> dec2int(std::basic_string_view<char_T> const p_str)
		{
			const uintptr_t size = p_str.size();
			if(size == 0)
			{
				return std::errc::invalid_argument;
			}

			const char_T* pivot	= p_str.data();
			const char_T* end = pivot + size;

			int_T r_val = 0;

			if(*pivot == '-') //negative handling
			{
				if(++pivot >= end)
				{
					return std::errc::invalid_argument;
				}

				do
				{
					uint8_t const t_val = static_cast<uint8_t>(*pivot - '0');
					if(t_val > 9)
					{
						return std::errc::invalid_argument;
					}


					if(r_val <= dec_signed_help<int_T>::min_threshold_val)
					{
						if(r_val < dec_signed_help<int_T>::min_threshold_val || t_val > dec_signed_help<int_T>::min_last_digit)
						{
							return std::errc::value_too_large;
						}
					}
					r_val = static_cast<int_T>(r_val * 10 - t_val);
				}
				while(++pivot < end);
			}
			else	//positive handling
			{
				if(*pivot == '+')
				{
					if(++pivot >= end)
					{
						return std::errc::invalid_argument;
					}
				}
				do
				{
					uint8_t const t_val = static_cast<uint8_t>(*pivot - '0');
					if(t_val > 9)
					{
						return std::errc::invalid_argument;
					}

					if(r_val >= dec_signed_help<int_T>::max_threshold_val)
					{
						if(r_val > dec_signed_help<int_T>::max_threshold_val || t_val > dec_signed_help<int_T>::max_last_digit)
						{
							return std::errc::value_too_large;
						}
					}
					r_val = static_cast<int_T>(r_val * 10 + t_val);
				}
				while(++pivot < end);
			}

			return r_val;
		}

		///	\brief		Converts a base 16 encoded string to unsigned integer.
		template <charconv_uint_c uint_T, charconv_char_c char_T>
		[[nodiscard]] static inline from_chars_result<uint_T> hex2uint(std::basic_string_view<char_T> const p_str)
		{
			constexpr char_T num_A_offset = static_cast<char_T>('A' - '9' - 1);
			constexpr char_T A_a_offset = static_cast<char_T>('a' - 'A');

			if(p_str.empty())
			{
				return std::errc::invalid_argument;
			}

			uint_T r_val = 0;
			for(char_T t_val : p_str)
			{
				if(r_val > hex_unsigned_help<uint_T>::threshold_val)
				{
					return {};
				}

				t_val -= '0';
				if(t_val > 9)
				{
					t_val -= num_A_offset;

					if(t_val > 15)
					{
						t_val -= A_a_offset;
						if(t_val < 10 || t_val > 15)
						{
							return std::errc::invalid_argument;
						}
					}
					else if(t_val < 10)
					{
						return std::errc::invalid_argument;
					}
				}

				r_val <<= 4;
				r_val |= static_cast<uint_T>(t_val);
			}
			return r_val;
		}


		template <charconv_uint_c uint_T, charconv_char_c char_T>
		[[nodiscard]] static inline from_chars_result<uint_T> bin2uint(std::basic_string_view<char_T> const p_str)
		{
			if(p_str.empty())
			{
				return std::errc::invalid_argument;
			}

			uint_T r_val = 0;
			for(char_T t_val : p_str)
			{
				if(r_val > bin_unsigned_help<uint_T>::threshold_val)
				{
					return {};
				}

				t_val -= '0';

				if(t_val > 1)
				{
					return std::errc::invalid_argument;
				}

				r_val <<= 1;
				r_val |= static_cast<uint_T>(t_val);
			}
			return r_val;
		}


		template<charconv_char_c char_T>
		static bool is_inf(std::basic_string_view<char_T> p_str)
		{
			if(p_str.size() == 3)
			{
				if(p_str[0] == 'I')
				{
					if(p_str[1] == 'n')
					{
						return p_str[2] == 'f';
					}
					return p_str[1] == 'N' && p_str[2] == 'F';
				}

				if constexpr (std::is_same_v<char_T, char8_t>)
				{
					if(p_str[0] == 'i')
					{
						return p_str[1] == 'n' && p_str[2] == 'f';
					}
					return
						p_str[0] == 0xE2 &&
						p_str[1] == 0x88 &&
						p_str[2] == 0x9E;
				}
				else
				{
					return p_str[0] == 'i' && p_str[1] == 'n' && p_str[2] == 'f';
				}
			}
			
			if(p_str.size() == 8)
			{
				if(p_str[0] == 'I')
				{
					if(p_str[1] == 'n')
					{
						return
							p_str[2] == 'f' &&
							p_str[3] == 'i' &&
							p_str[4] == 'n' &&
							p_str[5] == 'i' &&
							p_str[6] == 't' &&
							(p_str[7] == 'e' || p_str[7] == 'y');
					}
					return
						p_str[1] == 'N' &&
						p_str[2] == 'F' &&
						p_str[3] == 'I' &&
						p_str[4] == 'N' &&
						p_str[5] == 'I' &&
						p_str[6] == 'T' &&
						(p_str[7] == 'E' || p_str[7] == 'Y');
				}
				return
					p_str[0] == 'i' &&
					p_str[1] == 'n' &&
					p_str[2] == 'f' &&
					p_str[3] == 'i' &&
					p_str[4] == 'n' &&
					p_str[5] == 'i' &&
					p_str[6] == 't' &&
					(p_str[7] == 'e' || p_str[7] == 'y');
			}

			if constexpr (!std::is_same_v<char_T, char8_t>)
			{
				if(p_str.size() == 1)
				{
					return p_str[0] == 0x221E;
				}
			}
			return false;
		}

		template<_p::charconv_char_c char_T>
		static bool is_nan(std::basic_string_view<char_T> p_str)
		{
			if(p_str.size() == 3)
			{
				if(p_str[0] == 'n')
				{
					return p_str[1] == 'a' && p_str[2] == 'n';
				}
				else if(p_str[0] == 'N')
				{
					if(p_str[1] == 'a')
					{
						return p_str[2] == 'n' || p_str[2] == 'N';
					}
					return p_str[1] == 'A' && p_str[2] == 'N';
				}
			}
			else if(p_str.size() == 9)
			{
				return p_str[0] == 'n' &&
					p_str[1] == 'a' &&
					p_str[2] == 'n' &&
					p_str[3] == '(' &&
					(p_str[4] == 's' || p_str[4] == 'q') &&
					p_str[5] == 'n' &&
					p_str[6] == 'a' &&
					p_str[7] == 'n' &&
					p_str[8] == ')';
			}
			return false;
		}


		template<charconv_fp_c fp_T, charconv_char_c char_T>
		[[nodiscard]] static from_chars_result<fp_T> dec2fp(std::basic_string_view<char_T> p_str)
		{
			if(p_str.empty())
			{
				return std::errc::invalid_argument;
			}

			const bool sig_bit = (p_str[0] == '-');
			if(sig_bit || p_str[0] == '+')
			{
				p_str = p_str.substr(1);
				if(p_str.empty())
				{
					return std::errc::invalid_argument;
				}
			}

			{
				const char_T first_char = p_str[0];
				if(is_digit(first_char) || first_char == '.')
				{
					std::basic_string_view<char_T> exp_str;
					bool exp_sign = false;
					{
						uintptr_t pos = p_str.find(char_T{'E'});
						if(pos != std::basic_string_view<char_T>::npos ||
							(pos = p_str.find(char_T{'e'}), pos != std::basic_string_view<char_T>::npos))
						{
							exp_str = p_str.substr(pos  + 1);
							p_str = p_str.substr(0, pos);
							if(exp_str.empty() || p_str.empty())
							{
								return std::errc::invalid_argument;
							}
							exp_sign = exp_str[0] == '-';
							if(exp_sign || exp_str[0] == '+')
							{
								exp_str = exp_str.substr(1);
								if(exp_str.empty())
								{
									return std::errc::invalid_argument;
								}
							}
						}
					}
					std::basic_string_view<char_T> unit_str;
					std::basic_string_view<char_T> decimal_str;
					if(first_char == '.')
					{
						decimal_str = p_str.substr(1);
						if(decimal_str.empty()) return std::errc::invalid_argument;
					}
					else
					{
						const uintptr_t pos = p_str.find(char_T{'.'});
						if(pos == std::basic_string_view<char_T>::npos)
						{
							unit_str = p_str;
						}
						else
						{
							unit_str = p_str.substr(0, pos);
							decimal_str = p_str.substr(pos + 1);
						}
					}

					return from_chars_fp<fp_T, char_T>(sig_bit, unit_str, decimal_str, exp_sign, exp_str);
				}
			}
			using uint_t = fp_traits<fp_T>::uint_t;

			if(is_inf(p_str))
			{
				uint_t bits = core::fp_traits<fp_T>::exponent_mask;

				if(sig_bit)
				{
					bits |= core::fp_traits<fp_T>::sign_mask;
				}
				return std::bit_cast<const fp_T>(bits);
			}
			if(is_nan(p_str))
			{
				uint_t bits =
					core::fp_traits<fp_T>::exponent_mask |
					core::fp_traits<fp_T>::mantissa_mask;

				if(sig_bit)
				{
					bits |= core::fp_traits<fp_T>::sign_mask;
				}
				return std::bit_cast<const fp_T>(bits);
			}

			return std::errc::invalid_argument;

			//const uintptr_t tsize = p_str.size();
			//if(tsize > 126) return std::errc::no_buffer_space; //large enough to not make any sense
			//for(const char_T t_char: p_str)
			//{
			//	//last allowable character also checks that conversion to to char8_t will not alias
			//	if(t_char > 'e') return std::errc::invalid_argument;
			//}
			//
			//std::array<char8_t, 126> buff;
			//{
			//	char8_t* pos = buff.data();
			//	for(const char_T t_char : p_str)
			//	{
			//		*(pos++) = static_cast<char8_t>(t_char);
			//	}
			//}
			//
			//return dec2fp<fp_T>(std::basic_string_view<char8_t>{buff.data(), tsize});
		}

#if 1
		template <charconv_uint_c num_T>
		[[nodiscard]] static constexpr uintptr_t uint2dec_estimate(num_T);

		template <>
		[[nodiscard]] inline constexpr uintptr_t uint2dec_estimate<uint8_t>(uint8_t const p_val)
		{
			if(p_val <  10_ui8) return 1;
			if(p_val < 100_ui8) return 2;
			return 3;
		}

		template <>
		[[nodiscard]] inline constexpr uintptr_t uint2dec_estimate<uint16_t>(uint16_t const p_val)
		{
			if(p_val <    10_ui16) return 1;
			if(p_val <   100_ui16) return 2;
			if(p_val <  1000_ui16) return 3;
			if(p_val < 10000_ui16) return 4;
			return 5;
		}

		template <>
		[[nodiscard]] inline constexpr uintptr_t uint2dec_estimate<uint32_t>(uint32_t const p_val)
		{
			if(p_val <         10_ui32) return 1;
			if(p_val <        100_ui32) return 2;
			if(p_val <       1000_ui32) return 3;
			if(p_val <      10000_ui32) return 4;
			if(p_val <     100000_ui32) return 5;
			if(p_val <    1000000_ui32) return 6;
			if(p_val <   10000000_ui32) return 7;
			if(p_val <  100000000_ui32) return 8;
			if(p_val < 1000000000_ui32) return 9;
			return 10;
		}

		template <>
		[[nodiscard]] inline constexpr uintptr_t uint2dec_estimate<uint64_t>(uint64_t const p_val)
		{
			if(p_val <                   10_ui64) return  1;
			if(p_val <                  100_ui64) return  2;
			if(p_val <                 1000_ui64) return  3;
			if(p_val <                10000_ui64) return  4;
			if(p_val <               100000_ui64) return  5;
			if(p_val <              1000000_ui64) return  6;
			if(p_val <             10000000_ui64) return  7;
			if(p_val <            100000000_ui64) return  8;
			if(p_val <           1000000000_ui64) return  9;
			if(p_val <          10000000000_ui64) return 10;
			if(p_val <         100000000000_ui64) return 11;
			if(p_val <        1000000000000_ui64) return 12;
			if(p_val <       10000000000000_ui64) return 13;
			if(p_val <      100000000000000_ui64) return 14;
			if(p_val <     1000000000000000_ui64) return 15;
			if(p_val <    10000000000000000_ui64) return 16;
			if(p_val <   100000000000000000_ui64) return 17;
			if(p_val <  1000000000000000000_ui64) return 18;
			if(p_val < 10000000000000000000_ui64) return 19;
			return 20;
		}
#else

		static constexpr std::array<uintptr_t, 65> dec_base_digits_table =
		{
			 1_uip,
			 1_uip,
			 1_uip,
			 1_uip,
			 1_uip,
			 2_uip,
			 2_uip,
			 2_uip,
			 3_uip,
			 3_uip,
			 3_uip,
			 4_uip,
			 4_uip,
			 4_uip,
			 4_uip,
			 5_uip,
			 5_uip,
			 5_uip,
			 6_uip,
			 6_uip,
			 6_uip,
			 7_uip,
			 7_uip,
			 7_uip,
			 7_uip,
			 8_uip,
			 8_uip,
			 8_uip,
			 9_uip,
			 9_uip,
			 9_uip,
			10_uip,
			10_uip,
			10_uip,
			10_uip,
			11_uip,
			11_uip,
			11_uip,
			12_uip,
			12_uip,
			12_uip,
			13_uip,
			13_uip,
			13_uip,
			13_uip,
			14_uip,
			14_uip,
			14_uip,
			15_uip,
			15_uip,
			15_uip,
			16_uip,
			16_uip,
			16_uip,
			16_uip,
			17_uip,
			17_uip,
			17_uip,
			18_uip,
			18_uip,
			18_uip,
			19_uip,
			19_uip,
			19_uip,
			19_uip,
		};

		template<charconv_uint_c T>
		struct count_helper;

		template<>
		struct count_helper<uint64_t>
		{
			static constexpr std::array<uint64_t, 65>  dec_extra_digit_table =
			{
				0xFFFFFFFFFFFFFFFF_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				9_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				99_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				999_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				9999_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				99999_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				999999_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				9999999_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				99999999_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				999999999_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				9999999999_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				99999999999_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				999999999999_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				9999999999999_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				99999999999999_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				999999999999999_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				9999999999999999_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				99999999999999999_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				999999999999999999_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				0xFFFFFFFFFFFFFFFF_ui64,
				9999999999999999999_ui64,
			};
		};

		template<>
		struct count_helper<uint32_t>
		{
			static constexpr std::array<uint32_t, 33>  dec_extra_digit_table =
			{
				0xFFFFFFFF_ui32,
				0xFFFFFFFF_ui32,
				0xFFFFFFFF_ui32,
				0xFFFFFFFF_ui32,
				9_ui32,
				0xFFFFFFFF_ui32,
				0xFFFFFFFF_ui32,
				99_ui32,
				0xFFFFFFFF_ui32,
				0xFFFFFFFF_ui32,
				999_ui32,
				0xFFFFFFFF_ui32,
				0xFFFFFFFF_ui32,
				0xFFFFFFFF_ui32,
				9999_ui32,
				0xFFFFFFFF_ui32,
				0xFFFFFFFF_ui32,
				99999_ui32,
				0xFFFFFFFF_ui32,
				0xFFFFFFFF_ui32,
				999999_ui32,
				0xFFFFFFFF_ui32,
				0xFFFFFFFF_ui32,
				0xFFFFFFFF_ui32,
				9999999_ui32,
				0xFFFFFFFF_ui32,
				0xFFFFFFFF_ui32,
				99999999_ui32,
				0xFFFFFFFF_ui32,
				0xFFFFFFFF_ui32,
				999999999_ui32,
				0xFFFFFFFF_ui32,
				0xFFFFFFFF_ui32,
			};
		};

		template<>
		struct count_helper<uint16_t>
		{
			static constexpr std::array<uint16_t, 17>  dec_extra_digit_table =
			{
				0xFFFF_ui16,
				0xFFFF_ui16,
				0xFFFF_ui16,
				0xFFFF_ui16,
				9_ui16,
				0xFFFF_ui16,
				0xFFFF_ui16,
				99_ui16,
				0xFFFF_ui16,
				0xFFFF_ui16,
				999_ui16,
				0xFFFF_ui16,
				0xFFFF_ui16,
				0xFFFF_ui16,
				9999_ui16,
				0xFFFF_ui16,
				0xFFFF_ui16,
			};
		};

		template<>
		struct count_helper<uint8_t>
		{
			static constexpr std::array<uint8_t, 9>  dec_extra_digit_table =
			{
				0xFF_ui8,
				0xFF_ui8,
				0xFF_ui8,
				0xFF_ui8,
				9_ui8,
				0xFF_ui8,
				0xFF_ui8,
				99_ui8,
				0xFF_ui8,
			};
		};


		template <charconv_uint_c num_T>
		[[nodiscard]] static constexpr uintptr_t uint2dec_estimate(num_T const p_val)
		{
			constexpr uintptr_t num_bits = sizeof(num_T) * 8;
			uintptr_t const sig_bits = (num_bits - std::countl_zero(p_val));
			return dec_base_digits_table[sig_bits] + (p_val > count_helper<uint64_t>::dec_extra_digit_table[sig_bits]);
		}
#endif

		template <charconv_sint_c num_T>
		[[nodiscard]] inline constexpr uintptr_t int2dec_estimate(num_T const p_val)
		{
			using unsigned_t = std::make_unsigned_t<num_T>;

			if(p_val < 0)
			{
				return uint2dec_estimate<unsigned_t>(static_cast<unsigned_t>(-p_val)) + 1;
			}
			return uint2dec_estimate<unsigned_t>(static_cast<unsigned_t>(p_val));
		}

		template<charconv_fp_c Fp_t>
		[[nodiscard]] static inline uintptr_t fp2dec_estimate(Fp_t const p_val)
		{

			fp_to_chars_shortest_context<Fp_t> context;
			const fp_base_classify classification = to_chars_shortest_classify(p_val, context);

			switch(classification.classification)
			{
			default:
			case fp_classify::zero:
				return classification.is_negative ? 2 : 1;
			case fp_classify::finite:
				break;
			case fp_classify::inf:
				return classification.is_negative ? 4 : 3;
			case fp_classify::nan:
				return 3;
			}

			const fp_to_chars_sci_size sci_size_data = to_chars_shortest_sci_size(context);
			const fp_to_chars_fix_size fix_size_data = to_chars_shortest_fix_size(context);

			uint8_t sci_size = 1;
			if(sci_size_data.mantissa_decimal_size)
			{
				sci_size += static_cast<uint8_t>(sci_size_data.mantissa_decimal_size + 1);
			}
			if(sci_size_data.exponent_size)
			{
				sci_size += static_cast<uint8_t>(sci_size_data.exponent_size + 1);
				if(sci_size_data.is_exp_negative)
				{
					++sci_size;
				}
			}

			uint8_t fix_size = 1;
			if(fix_size_data.unit_size)
			{
				fix_size = static_cast<uint8_t>(fix_size_data.unit_size);
			}

			if(fix_size_data.decimal_size)
			{
				fix_size += static_cast<uint8_t>(fix_size_data.decimal_size + 1);
			}

			uint8_t min_size = std::min(sci_size, fix_size); 

			if(classification.is_negative)
			{
				++min_size;
			}
			return min_size;
		}

		template <charconv_uint_c num_T>
		[[nodiscard]] static inline constexpr uintptr_t uint2hex_estimate(num_T const p_val)
		{
			if(p_val)
			{
				//skip ahead algorithm
				constexpr uintptr_t lastBit = sizeof(num_T) * 8 - 1;
				return ((lastBit - std::countl_zero<num_T>(p_val)) / 4) + 1;
			}
			return 1;
		}

		template <charconv_uint_c num_T>
		[[nodiscard]] static inline constexpr uintptr_t uint2bin_estimate(num_T const p_val)
		{
			if (p_val)
			{
				//skip ahead algorithm
				constexpr uintptr_t bit_count = sizeof(num_T) * 8;
				return bit_count - std::countl_zero<num_T>(p_val);
			}
			return 1;
		}

		static constexpr std::array x2NibTable = {u8'0', u8'1', u8'2', u8'3', u8'4', u8'5', u8'6', u8'7', u8'8', u8'9', u8'A', u8'B', u8'C', u8'D', u8'E', u8'F'};
		[[nodiscard]] static inline constexpr char8_t Hex2Nible(uintptr_t const p_val)
		{
			return x2NibTable[p_val & 0x0F];
		}

		template <charconv_char_c char_T, charconv_uint_c num_T>
		static inline char_T* uint2dec(num_T p_val, char_T* p_str)
		{
			p_str += uint2dec_estimate(p_val);
			char_T* pivot = p_str;
			while(p_val > 9)
			{
				*(--pivot) = static_cast<char_T>('0' + p_val % 10);
				p_val /= 10;
			}
			*(--pivot) = static_cast<char_T>('0' + p_val);
			return p_str;
		}

		template <charconv_char_c char_T, charconv_sint_c num_T>
		static inline char_T* int2dec(const num_T p_val, char_T* const p_str)
		{
			using unsigned_t = std::make_unsigned_t<num_T>;

			if(p_val < 0)
			{
				*p_str = '-';
				return uint2dec<char_T, unsigned_t>(static_cast<unsigned_t>(-p_val), p_str + 1);
			}

			return uint2dec<char_T, unsigned_t>(static_cast<unsigned_t>(p_val), p_str);
		}

		template <charconv_char_c char_T, charconv_uint_c num_T>
		static inline char_T* uint2hex(num_T const p_val, char_T* pivot)
		{
			if(!p_val)
			{
				*(pivot++) = '0';
				return pivot;
			};

			//skip ahead algorithm
			constexpr uintptr_t lastBit = sizeof(num_T) * 8 - 1;
			const uint8_t index = static_cast<uint8_t>((lastBit - std::countl_zero<num_T>(p_val)) / 4);

			for(uint8_t t_bias = static_cast<uint8_t>(index * 4); t_bias; t_bias -= 4)
			{
				*(pivot++) = _p::Hex2Nible(static_cast<uint8_t> (p_val >> t_bias));
			}
			*(pivot++) = _p::Hex2Nible(static_cast<uint8_t> (p_val));
			return pivot;
		}

		template <charconv_char_c char_T, charconv_uint_c num_T>
		static inline char_T* uint2bin(num_T const p_val, char_T* pivot)
		{
			if (!p_val)
			{
				*(pivot++) = '0';
				return pivot;
			};

			//skip ahead algorithm
			constexpr uintptr_t bit_count = sizeof(num_T) * 8;
			const uint8_t index = static_cast<uint8_t>(bit_count - std::countl_zero<num_T>(p_val) - 1);

			for (uint8_t t_bias = index; t_bias; --t_bias)
			{
				*(pivot++) = ((p_val >> t_bias) & 1) ? '1' : '0';
			}
			*(pivot++) = (p_val & 1) ? '1' : '0';
			return pivot;
		}

		template <charconv_char_c T>
		static inline void uint2hex_fix(const uint8_t p_val, T* p_str)
		{
			*p_str   = _p::Hex2Nible(p_val >> 4);
			*++p_str = _p::Hex2Nible(p_val);
		}

		template <charconv_char_c T>
		static inline void uint2hex_fix(const uint16_t p_val, T* p_str)
		{
			*p_str   = _p::Hex2Nible(p_val >> 12);
			*++p_str = _p::Hex2Nible(p_val >> 8);
			*++p_str = _p::Hex2Nible(p_val >> 4);
			*++p_str = _p::Hex2Nible(p_val);
		}

		template <charconv_char_c T>
		static inline void uint2hex_fix(const uint32_t p_val, T* p_str)
		{
			*p_str   = _p::Hex2Nible(p_val >> 28);
			*++p_str = _p::Hex2Nible(p_val >> 24);
			*++p_str = _p::Hex2Nible(p_val >> 20);
			*++p_str = _p::Hex2Nible(p_val >> 16);
			*++p_str = _p::Hex2Nible(p_val >> 12);
			*++p_str = _p::Hex2Nible(p_val >> 8);
			*++p_str = _p::Hex2Nible(p_val >> 4);
			*++p_str = _p::Hex2Nible(p_val);
		}

		template <charconv_char_c T>
		static inline void uint2hex_fix(const uint64_t p_val, T* p_str)
		{
			*p_str   = _p::Hex2Nible(p_val >> 60);
			*++p_str = _p::Hex2Nible(p_val >> 56);
			*++p_str = _p::Hex2Nible(p_val >> 52);
			*++p_str = _p::Hex2Nible(p_val >> 48);
			*++p_str = _p::Hex2Nible(p_val >> 44);
			*++p_str = _p::Hex2Nible(p_val >> 40);
			*++p_str = _p::Hex2Nible(p_val >> 36);
			*++p_str = _p::Hex2Nible(p_val >> 32);
			*++p_str = _p::Hex2Nible(p_val >> 28);
			*++p_str = _p::Hex2Nible(p_val >> 24);
			*++p_str = _p::Hex2Nible(p_val >> 20);
			*++p_str = _p::Hex2Nible(p_val >> 16);
			*++p_str = _p::Hex2Nible(p_val >> 12);
			*++p_str = _p::Hex2Nible(p_val >> 8);
			*++p_str = _p::Hex2Nible(p_val >> 4);
			*++p_str = _p::Hex2Nible(p_val);
		}

		//template <charconv_char_c T, charconv_uint_c NUM_T>
		//static inline void uint2hex_fix(NUM_T const p_val, T* p_str)
		//{
		//	for(uint8_t offset = static_cast<uint8_t>(sizeof(NUM_T)*8); offset -= 4;)
		//	{
		//		*(p_str++) = _p::Hex2Nible(p_val >> offset);
		//	}
		//	*(p_str) = _p::Hex2Nible(p_val);
		//}

		template <charconv_char_c T>
		static inline void uint2bin_fix(const uint8_t p_val, T* p_str)
		{
			*p_str   = ((p_val >> 7) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 6) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 5) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 4) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 3) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 2) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 1) & 1) ? '1' : '0';
			*++p_str = ( p_val       & 1) ? '1' : '0';
		}

		template <charconv_char_c T>
		static inline void uint2bin_fix(const uint16_t p_val, T* p_str)
		{
			*p_str   = ((p_val >> 15) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 14) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 13) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 12) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 11) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 10) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 9 ) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 8 ) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 7 ) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 6 ) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 5 ) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 4 ) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 3 ) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 2 ) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 1 ) & 1) ? '1' : '0';
			*++p_str = ( p_val        & 1) ? '1' : '0';
		}

		template <charconv_char_c T>
		static inline void uint2bin_fix(const uint32_t p_val, T* p_str)
		{
			*p_str   = ((p_val >> 31) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 30) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 29) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 28) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 27) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 26) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 25) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 24) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 23) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 22) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 21) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 20) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 19) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 18) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 17) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 16) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 15) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 14) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 13) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 12) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 11) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 10) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 9 ) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 8 ) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 7 ) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 6 ) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 5 ) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 4 ) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 3 ) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 2 ) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 1 ) & 1) ? '1' : '0';
			*++p_str = ( p_val        & 1) ? '1' : '0';
		}

		template <charconv_char_c T>
		static inline void uint2bin_fix(const uint64_t p_val, T* p_str)
		{
			*p_str   = ((p_val >> 63) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 62) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 61) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 60) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 59) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 58) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 57) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 56) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 55) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 54) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 53) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 52) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 51) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 50) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 49) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 48) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 47) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 46) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 45) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 44) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 43) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 42) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 41) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 40) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 39) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 38) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 37) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 36) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 35) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 34) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 33) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 32) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 31) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 30) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 29) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 28) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 27) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 26) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 25) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 24) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 23) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 22) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 21) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 20) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 19) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 18) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 17) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 16) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 15) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 14) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 13) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 12) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 11) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 10) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 9 ) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 8 ) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 7 ) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 6 ) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 5 ) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 4 ) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 3 ) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 2 ) & 1) ? '1' : '0';
			*++p_str = ((p_val >> 1 ) & 1) ? '1' : '0';
			*++p_str = ( p_val        & 1) ? '1' : '0';
		}

		//template <charconv_char_c T, charconv_uint_c NUM_T>
		//static inline void uint2bin_fix(NUM_T const p_val, T* p_str)
		//{
		//	for(uint8_t offset = static_cast<uint8_t>(sizeof(NUM_T)*8); --offset;)
		//	{
		//		*++p_str = ((p_val >> offset ) & 1) ? '1' : '0';
		//	}
		//	*p_str = ( p_val        & 1) ? '1' : '0';
		//}


		template<charconv_char_c char_T, charconv_fp_c Fp_t>
		static inline char_T* fp2dec(const Fp_t p_val, char_T* pivot)
		{
			fp_to_chars_shortest_context<Fp_t> context;
			const fp_base_classify classification = to_chars_shortest_classify(p_val, context);

			if(classification.classification == fp_classify::nan)
			{
				*(pivot++) = char_T{'n'};
				*(pivot++) = char_T{'a'};
				*(pivot++) = char_T{'n'};
				return pivot;
			}

			if(classification.is_negative)
			{
				*(pivot++) = char_T{'-'};
			}

			switch(classification.classification)
			{
			default:
			case fp_classify::zero:
				*(pivot++) = char_T{'0'};
				break;
			case fp_classify::finite:
				{
					const fp_to_chars_sci_size sci_size_data = to_chars_shortest_sci_size(context);
					const fp_to_chars_fix_size fix_size_data = to_chars_shortest_fix_size(context);

					uint8_t sci_size = 1;
					if(sci_size_data.mantissa_decimal_size)
					{
						sci_size += static_cast<uint8_t>(sci_size_data.mantissa_decimal_size + 1);
					}
					if(sci_size_data.exponent_size)
					{
						sci_size += static_cast<uint8_t>(sci_size_data.exponent_size + 1);
						if(sci_size_data.is_exp_negative)
						{
							++sci_size;
						}
					}

					uint8_t fix_size = 1;
					if(fix_size_data.unit_size)
					{
						fix_size = static_cast<uint8_t>(fix_size_data.unit_size);
					}

					if(fix_size_data.decimal_size)
					{
						fix_size += static_cast<uint8_t>(fix_size_data.decimal_size + 1);
					}

					if(sci_size < fix_size)
					{
						{
							char_T* const unit_digit = pivot++;
							char_T* decimal_digit = pivot;
							if(sci_size_data.mantissa_decimal_size)
							{
								*(pivot++) = char_T{'.'};
								decimal_digit = pivot;
								pivot += sci_size_data.mantissa_decimal_size;
							}
							to_chars_shortest_sci_unsafe(context, unit_digit, decimal_digit);
						}
						if(sci_size_data.exponent_size)
						{
							*(pivot++) = char_T{'E'};
							if(sci_size_data.is_exp_negative)
							{
								*(pivot++) = char_T{'-'};
							}
							to_chars_shortest_sci_exp_unsafe(context, pivot);
							pivot += sci_size_data.exponent_size;
						}
					}
					else
					{
						char_T* const unit_digit = pivot;
						if(fix_size_data.unit_size)
						{
							pivot += fix_size_data.unit_size;
						}
						else
						{
							*(pivot++) = char_T{'0'};
						}

						char_T* decimal_digit = pivot;
						if(fix_size_data.decimal_size)
						{
							*(pivot++) = char_T{'.'};
							decimal_digit = pivot;
							pivot += fix_size_data.decimal_size;
						}
						to_chars_shortest_fix_unsafe(context, unit_digit, decimal_digit);
					}
				}
				break;
			case fp_classify::inf:
				*(pivot++) = char_T{'i'};
				*(pivot++) = char_T{'n'};
				*(pivot++) = char_T{'f'};
				break;
			}

			return pivot;
		}

		namespace
		{

			template<typename>
			struct help_char_conv;

			template<charconv_fp_c num_T>
			struct help_char_conv<num_T>
			{
				template<typename char_T>
				[[nodiscard]] static inline from_chars_result<num_T> from_chars(std::basic_string_view<char_T> const p_str)
				{
					return dec2fp<num_T>(p_str);
				}

				template<typename char_T>
				static inline char_T* to_chars(const num_T p_val, char_T* const p_str)
				{
					return fp2dec(p_val, p_str);
				}

				static inline uintptr_t estimate(const num_T p_val)
				{
					return fp2dec_estimate(p_val);
				}
			};

			template<charconv_sint_c num_T>
			struct help_char_conv<num_T>
			{
				template<typename char_T>
				[[nodiscard]] static inline from_chars_result<num_T> from_chars(std::basic_string_view<char_T> const p_str)
				{
					return dec2int<num_T>(p_str);
				}

				template<typename char_T>
				static inline char_T* to_chars(const num_T p_val, char_T* const p_str)
				{
					return int2dec(p_val, p_str);
				}

				static inline uintptr_t estimate(const num_T p_val)
				{
					return int2dec_estimate(p_val);
				}
			};

			template<charconv_uint_c num_T>
			struct help_char_conv<num_T>
			{
				template<typename char_T>
				[[nodiscard]] static inline from_chars_result<num_T> from_chars(std::basic_string_view<char_T> const p_str)
				{
					return dec2uint<num_T>(p_str);
				}

				template<typename char_T>
				static inline char_T* to_chars(const num_T p_val, char_T* const p_str)
				{
					return uint2dec(p_val, p_str);
				}

				static inline uintptr_t estimate(const num_T p_val)
				{
					return uint2dec_estimate(p_val);
				}

			};
		}	//namespace
	} //namespace _p


	//======== Public implementation ========

	template <char_conv_dec_supported_c num_T>
	[[nodiscard]] uintptr_t to_chars_size(const num_T p_val)
	{
		return _p::help_char_conv<num_T>::estimate(p_val);
	}

	template <_p::charconv_char_c char_T, char_conv_dec_supported_c num_T>
	char_T* to_chars_unsafe(const num_T p_val, char_T* const p_str)
	{
		return _p::help_char_conv<num_T>::to_chars(p_val, p_str);
	}

	template <char_conv_hex_supported_c num_T>
	[[nodiscard]] uintptr_t to_chars_hex_size(const num_T p_val)
	{
		return _p::uint2hex_estimate(p_val);
	}

	template <_p::charconv_char_c char_T, char_conv_hex_supported_c num_T>
	char_T* to_chars_hex_unsafe(const num_T p_val, char_T* const p_str)
	{
		return _p::uint2hex(p_val, p_str);
	}


	template <_p::charconv_char_c char_T, char_conv_hex_supported_c num_T>
	void to_chars_hex_fix_unsafe(const num_T p_val, char_T* const p_str)
	{
		_p::uint2hex_fix(p_val, p_str);
	}

	template <char_conv_bin_supported_c num_T>
	[[nodiscard]] uintptr_t to_chars_bin_size(const num_T p_val)
	{
		return _p::uint2bin_estimate(p_val);
	}

	template <_p::charconv_char_c char_T, char_conv_bin_supported_c num_T>
	char_T* to_chars_bin_unsafe(const num_T p_val, char_T* const p_str)
	{
		return _p::uint2bin(p_val, p_str);
	}

	template <_p::charconv_char_c char_T, char_conv_bin_supported_c num_T>
	void to_chars_bin_fix_unsafe(const num_T p_val, char_T* const p_str)
	{
		_p::uint2bin_fix(p_val, p_str);
	}

	namespace _p
	{
		template <_p::charconv_char_c T>
		[[nodiscard]] bool is_uint(std::basic_string_view<T> const p_str)
		{
			if(p_str.size() == 0) return false;
			for(T const tchar : p_str)
			{
				if(!is_digit(tchar)) return false;
			}
			return true;
		}

		template <_p::charconv_char_c T>
		[[nodiscard]] bool is_int(std::basic_string_view<T> const p_str)
		{
			if(p_str.size() == 0) return false;
			if((p_str[0] == '-') || (p_str[0] == '+'))
			{
				return is_uint(p_str.substr(1));
			}
			return is_uint(p_str);
		}

		template <_p::charconv_char_c T>
		[[nodiscard]] bool is_hex(std::basic_string_view<T> const p_str)
		{
			if(p_str.size() == 0) return false;
			for(T tchar : p_str)
			{
				if(!is_xdigit(tchar)) return false;
			}
			return true;
		}

		template <_p::charconv_char_c T>
		[[nodiscard]] bool is_bin(std::basic_string_view<T> const p_str)
		{
			if(p_str.size() == 0) return false;
			for(T tchar : p_str)
			{
				if(!is_bdigit(tchar)) return false;
			}
			return true;
		}

		template<char_conv_dec_supported_c num_T, _p::charconv_char_c char_T>
		[[nodiscard]] from_chars_result<num_T> from_chars(std::basic_string_view<char_T> const p_str)
		{
			return _p::help_char_conv<num_T>::from_chars(p_str);
		}

		template<char_conv_hex_supported_c num_T, _p::charconv_char_c char_T>
		[[nodiscard]] from_chars_result<num_T> from_chars_hex(std::basic_string_view<char_T> const p_str)
		{
			return _p::hex2uint<num_T>(p_str);
		}

		template<char_conv_bin_supported_c num_T, _p::charconv_char_c char_T>
		[[nodiscard]] from_chars_result<num_T> from_chars_bin(std::basic_string_view<char_T> const p_str)
		{
			return _p::bin2uint<num_T>(p_str);
		}

		template <_p::charconv_char_c char_T, char_conv_dec_supported_c num_T>
		[[nodiscard]] uintptr_t to_chars(const num_T p_val, std::span<char_T, to_chars_dec_max_size_v<num_T>> const p_str)
		{
			return static_cast<uintptr_t>(_p::help_char_conv<num_T>::to_chars(p_val, p_str.data()) - p_str.data());
		}

		template <_p::charconv_char_c char_T, char_conv_hex_supported_c num_T>
		[[nodiscard]] uintptr_t to_chars_hex(const num_T p_val, std::span<char_T, to_chars_hex_max_size_v<num_T>> const p_str)
		{
			return static_cast<uintptr_t>(_p::uint2hex(p_val, p_str.data()) - p_str.data());
		}

		template <_p::charconv_char_c char_T, char_conv_hex_supported_c num_T>
		void to_chars_hex_fix(const num_T p_val, std::span<char_T, to_chars_hex_max_size_v<num_T>> const p_str)
		{
			_p::uint2hex_fix(p_val, p_str.data());
		}

		template <_p::charconv_char_c char_T, char_conv_bin_supported_c num_T>
		[[nodiscard]] uintptr_t to_chars_bin(const num_T p_val, std::span<char_T, to_chars_bin_max_size_v<num_T>> const p_str)
		{
			return static_cast<uintptr_t>(_p::uint2bin(p_val, p_str.data()) - p_str.data());
		}

		template <_p::charconv_char_c char_T, char_conv_bin_supported_c num_T>
		void to_chars_bin_fix(const num_T p_val, std::span<char_T, to_chars_bin_max_size_v<num_T>> const p_str)
		{
			_p::uint2bin_fix(p_val, p_str.data());
		}

	} //namespace _p

//======== Explicit instantiation ========

	template uintptr_t to_chars_size<uint8_t  >(uint8_t  );
	template uintptr_t to_chars_size<uint16_t >(uint16_t );
	template uintptr_t to_chars_size<uint32_t >(uint32_t );
	template uintptr_t to_chars_size<uint64_t >(uint64_t );
	template uintptr_t to_chars_size<int8_t   >(int8_t   );
	template uintptr_t to_chars_size<int16_t  >(int16_t  );
	template uintptr_t to_chars_size<int32_t  >(int32_t  );
	template uintptr_t to_chars_size<int64_t  >(int64_t  );
	template uintptr_t to_chars_size<float32_t>(float32_t);
	template uintptr_t to_chars_size<float64_t>(float64_t);


	template char8_t * to_chars_unsafe<char8_t , uint8_t  >(uint8_t  , char8_t *);
	template char8_t * to_chars_unsafe<char8_t , uint16_t >(uint16_t , char8_t *);
	template char8_t * to_chars_unsafe<char8_t , uint32_t >(uint32_t , char8_t *);
	template char8_t * to_chars_unsafe<char8_t , uint64_t >(uint64_t , char8_t *);
	template char8_t * to_chars_unsafe<char8_t , int8_t   >(int8_t   , char8_t *);
	template char8_t * to_chars_unsafe<char8_t , int16_t  >(int16_t  , char8_t *);
	template char8_t * to_chars_unsafe<char8_t , int32_t  >(int32_t  , char8_t *);
	template char8_t * to_chars_unsafe<char8_t , int64_t  >(int64_t  , char8_t *);
	template char8_t * to_chars_unsafe<char8_t , float32_t>(float32_t, char8_t *);
	template char8_t * to_chars_unsafe<char8_t , float64_t>(float64_t, char8_t *);

	template char16_t* to_chars_unsafe<char16_t, uint8_t  >(uint8_t  , char16_t*);
	template char16_t* to_chars_unsafe<char16_t, uint16_t >(uint16_t , char16_t*);
	template char16_t* to_chars_unsafe<char16_t, uint32_t >(uint32_t , char16_t*);
	template char16_t* to_chars_unsafe<char16_t, uint64_t >(uint64_t , char16_t*);
	template char16_t* to_chars_unsafe<char16_t, int8_t   >(int8_t   , char16_t*);
	template char16_t* to_chars_unsafe<char16_t, int16_t  >(int16_t  , char16_t*);
	template char16_t* to_chars_unsafe<char16_t, int32_t  >(int32_t  , char16_t*);
	template char16_t* to_chars_unsafe<char16_t, int64_t  >(int64_t  , char16_t*);
	template char16_t* to_chars_unsafe<char16_t, float32_t>(float32_t, char16_t*);
	template char16_t* to_chars_unsafe<char16_t, float64_t>(float64_t, char16_t*);

	template char32_t* to_chars_unsafe<char32_t, uint8_t  >(uint8_t  , char32_t*);
	template char32_t* to_chars_unsafe<char32_t, uint16_t >(uint16_t , char32_t*);
	template char32_t* to_chars_unsafe<char32_t, uint32_t >(uint32_t , char32_t*);
	template char32_t* to_chars_unsafe<char32_t, uint64_t >(uint64_t , char32_t*);
	template char32_t* to_chars_unsafe<char32_t, int8_t   >(int8_t   , char32_t*);
	template char32_t* to_chars_unsafe<char32_t, int16_t  >(int16_t  , char32_t*);
	template char32_t* to_chars_unsafe<char32_t, int32_t  >(int32_t  , char32_t*);
	template char32_t* to_chars_unsafe<char32_t, int64_t  >(int64_t  , char32_t*);
	template char32_t* to_chars_unsafe<char32_t, float32_t>(float32_t, char32_t*);
	template char32_t* to_chars_unsafe<char32_t, float64_t>(float64_t, char32_t*);


	template uintptr_t to_chars_hex_size<uint8_t >(uint8_t );
	template uintptr_t to_chars_hex_size<uint16_t>(uint16_t);
	template uintptr_t to_chars_hex_size<uint32_t>(uint32_t);
	template uintptr_t to_chars_hex_size<uint64_t>(uint64_t);


	template char8_t* to_chars_hex_unsafe<char8_t, uint8_t >(uint8_t , char8_t*);
	template char8_t* to_chars_hex_unsafe<char8_t, uint16_t>(uint16_t, char8_t*);
	template char8_t* to_chars_hex_unsafe<char8_t, uint32_t>(uint32_t, char8_t*);
	template char8_t* to_chars_hex_unsafe<char8_t, uint64_t>(uint64_t, char8_t*);

	template char16_t* to_chars_hex_unsafe<char16_t, uint8_t >(uint8_t , char16_t*);
	template char16_t* to_chars_hex_unsafe<char16_t, uint16_t>(uint16_t, char16_t*);
	template char16_t* to_chars_hex_unsafe<char16_t, uint32_t>(uint32_t, char16_t*);
	template char16_t* to_chars_hex_unsafe<char16_t, uint64_t>(uint64_t, char16_t*);

	template char32_t* to_chars_hex_unsafe<char32_t, uint8_t >(uint8_t , char32_t*);
	template char32_t* to_chars_hex_unsafe<char32_t, uint16_t>(uint16_t, char32_t*);
	template char32_t* to_chars_hex_unsafe<char32_t, uint32_t>(uint32_t, char32_t*);
	template char32_t* to_chars_hex_unsafe<char32_t, uint64_t>(uint64_t, char32_t*);


	template void to_chars_hex_fix_unsafe<char8_t, uint8_t >(uint8_t , char8_t*);
	template void to_chars_hex_fix_unsafe<char8_t, uint16_t>(uint16_t, char8_t*);
	template void to_chars_hex_fix_unsafe<char8_t, uint32_t>(uint32_t, char8_t*);
	template void to_chars_hex_fix_unsafe<char8_t, uint64_t>(uint64_t, char8_t*);

	template void to_chars_hex_fix_unsafe<char16_t, uint8_t >(uint8_t , char16_t*);
	template void to_chars_hex_fix_unsafe<char16_t, uint16_t>(uint16_t, char16_t*);
	template void to_chars_hex_fix_unsafe<char16_t, uint32_t>(uint32_t, char16_t*);
	template void to_chars_hex_fix_unsafe<char16_t, uint64_t>(uint64_t, char16_t*);

	template void to_chars_hex_fix_unsafe<char32_t, uint8_t >(uint8_t , char32_t*);
	template void to_chars_hex_fix_unsafe<char32_t, uint16_t>(uint16_t, char32_t*);
	template void to_chars_hex_fix_unsafe<char32_t, uint32_t>(uint32_t, char32_t*);
	template void to_chars_hex_fix_unsafe<char32_t, uint64_t>(uint64_t, char32_t*);


	template uintptr_t to_chars_bin_size<uint8_t >(uint8_t );
	template uintptr_t to_chars_bin_size<uint16_t>(uint16_t);
	template uintptr_t to_chars_bin_size<uint32_t>(uint32_t);
	template uintptr_t to_chars_bin_size<uint64_t>(uint64_t);


	template char8_t* to_chars_bin_unsafe<char8_t, uint8_t >(uint8_t , char8_t*);
	template char8_t* to_chars_bin_unsafe<char8_t, uint16_t>(uint16_t, char8_t*);
	template char8_t* to_chars_bin_unsafe<char8_t, uint32_t>(uint32_t, char8_t*);
	template char8_t* to_chars_bin_unsafe<char8_t, uint64_t>(uint64_t, char8_t*);

	template char16_t* to_chars_bin_unsafe<char16_t, uint8_t >(uint8_t , char16_t*);
	template char16_t* to_chars_bin_unsafe<char16_t, uint16_t>(uint16_t, char16_t*);
	template char16_t* to_chars_bin_unsafe<char16_t, uint32_t>(uint32_t, char16_t*);
	template char16_t* to_chars_bin_unsafe<char16_t, uint64_t>(uint64_t, char16_t*);

	template char32_t* to_chars_bin_unsafe<char32_t, uint8_t >(uint8_t , char32_t*);
	template char32_t* to_chars_bin_unsafe<char32_t, uint16_t>(uint16_t, char32_t*);
	template char32_t* to_chars_bin_unsafe<char32_t, uint32_t>(uint32_t, char32_t*);
	template char32_t* to_chars_bin_unsafe<char32_t, uint64_t>(uint64_t, char32_t*);


	template void to_chars_bin_fix_unsafe<char8_t, uint8_t >(uint8_t , char8_t*);
	template void to_chars_bin_fix_unsafe<char8_t, uint16_t>(uint16_t, char8_t*);
	template void to_chars_bin_fix_unsafe<char8_t, uint32_t>(uint32_t, char8_t*);
	template void to_chars_bin_fix_unsafe<char8_t, uint64_t>(uint64_t, char8_t*);

	template void to_chars_bin_fix_unsafe<char16_t, uint8_t >(uint8_t , char16_t*);
	template void to_chars_bin_fix_unsafe<char16_t, uint16_t>(uint16_t, char16_t*);
	template void to_chars_bin_fix_unsafe<char16_t, uint32_t>(uint32_t, char16_t*);
	template void to_chars_bin_fix_unsafe<char16_t, uint64_t>(uint64_t, char16_t*);

	template void to_chars_bin_fix_unsafe<char32_t, uint8_t >(uint8_t , char32_t*);
	template void to_chars_bin_fix_unsafe<char32_t, uint16_t>(uint16_t, char32_t*);
	template void to_chars_bin_fix_unsafe<char32_t, uint32_t>(uint32_t, char32_t*);
	template void to_chars_bin_fix_unsafe<char32_t, uint64_t>(uint64_t, char32_t*);


	namespace _p
	{
		template bool is_uint(std::basic_string_view<char8_t> );
		template bool is_uint(std::basic_string_view<char16_t>);
		template bool is_uint(std::basic_string_view<char32_t>);
	
		template bool is_int(std::basic_string_view<char8_t> );
		template bool is_int(std::basic_string_view<char16_t>);
		template bool is_int(std::basic_string_view<char32_t>);
	
		template bool is_hex(std::basic_string_view<char8_t> );
		template bool is_hex(std::basic_string_view<char16_t>);
		template bool is_hex(std::basic_string_view<char32_t>);

		template bool is_bin(std::basic_string_view<char8_t> );
		template bool is_bin(std::basic_string_view<char16_t>);
		template bool is_bin(std::basic_string_view<char32_t>);
	

		template from_chars_result<uint8_t  > from_chars<uint8_t  , char8_t >(std::basic_string_view<char8_t >);
		template from_chars_result<uint16_t > from_chars<uint16_t , char8_t >(std::basic_string_view<char8_t >);
		template from_chars_result<uint32_t > from_chars<uint32_t , char8_t >(std::basic_string_view<char8_t >);
		template from_chars_result<uint64_t > from_chars<uint64_t , char8_t >(std::basic_string_view<char8_t >);
		template from_chars_result<int8_t   > from_chars<int8_t   , char8_t >(std::basic_string_view<char8_t >);
		template from_chars_result<int16_t  > from_chars<int16_t  , char8_t >(std::basic_string_view<char8_t >);
		template from_chars_result<int32_t  > from_chars<int32_t  , char8_t >(std::basic_string_view<char8_t >);
		template from_chars_result<int64_t  > from_chars<int64_t  , char8_t >(std::basic_string_view<char8_t >);
		template from_chars_result<float32_t> from_chars<float32_t, char8_t >(std::basic_string_view<char8_t >);
		template from_chars_result<float64_t> from_chars<float64_t, char8_t >(std::basic_string_view<char8_t >);

		template from_chars_result<uint8_t  > from_chars<uint8_t  , char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<uint16_t > from_chars<uint16_t , char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<uint32_t > from_chars<uint32_t , char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<uint64_t > from_chars<uint64_t , char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<int8_t   > from_chars<int8_t   , char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<int16_t  > from_chars<int16_t  , char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<int32_t  > from_chars<int32_t  , char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<int64_t  > from_chars<int64_t  , char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<float32_t> from_chars<float32_t, char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<float64_t> from_chars<float64_t, char16_t>(std::basic_string_view<char16_t>);

		template from_chars_result<uint8_t  > from_chars<uint8_t  , char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<uint16_t > from_chars<uint16_t , char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<uint32_t > from_chars<uint32_t , char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<uint64_t > from_chars<uint64_t , char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<int8_t   > from_chars<int8_t   , char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<int16_t  > from_chars<int16_t  , char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<int32_t  > from_chars<int32_t  , char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<int64_t  > from_chars<int64_t  , char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<float32_t> from_chars<float32_t, char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<float64_t> from_chars<float64_t, char32_t>(std::basic_string_view<char32_t>);


		template from_chars_result<uint8_t > from_chars_hex<uint8_t , char8_t >(std::basic_string_view<char8_t >);
		template from_chars_result<uint16_t> from_chars_hex<uint16_t, char8_t >(std::basic_string_view<char8_t >);
		template from_chars_result<uint32_t> from_chars_hex<uint32_t, char8_t >(std::basic_string_view<char8_t >);
		template from_chars_result<uint64_t> from_chars_hex<uint64_t, char8_t >(std::basic_string_view<char8_t >);

		template from_chars_result<uint8_t > from_chars_hex<uint8_t , char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<uint16_t> from_chars_hex<uint16_t, char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<uint32_t> from_chars_hex<uint32_t, char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<uint64_t> from_chars_hex<uint64_t, char16_t>(std::basic_string_view<char16_t>);

		template from_chars_result<uint8_t > from_chars_hex<uint8_t , char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<uint16_t> from_chars_hex<uint16_t, char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<uint32_t> from_chars_hex<uint32_t, char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<uint64_t> from_chars_hex<uint64_t, char32_t>(std::basic_string_view<char32_t>);


		template from_chars_result<uint8_t > from_chars_bin<uint8_t , char8_t >(std::basic_string_view<char8_t >);
		template from_chars_result<uint16_t> from_chars_bin<uint16_t, char8_t >(std::basic_string_view<char8_t >);
		template from_chars_result<uint32_t> from_chars_bin<uint32_t, char8_t >(std::basic_string_view<char8_t >);
		template from_chars_result<uint64_t> from_chars_bin<uint64_t, char8_t >(std::basic_string_view<char8_t >);

		template from_chars_result<uint8_t > from_chars_bin<uint8_t , char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<uint16_t> from_chars_bin<uint16_t, char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<uint32_t> from_chars_bin<uint32_t, char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<uint64_t> from_chars_bin<uint64_t, char16_t>(std::basic_string_view<char16_t>);

		template from_chars_result<uint8_t > from_chars_bin<uint8_t , char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<uint16_t> from_chars_bin<uint16_t, char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<uint32_t> from_chars_bin<uint32_t, char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<uint64_t> from_chars_bin<uint64_t, char32_t>(std::basic_string_view<char32_t>);


		template uintptr_t to_chars<char8_t , uint8_t  >(uint8_t  , std::span<char8_t , to_chars_dec_max_size_v<uint8_t  >>);
		template uintptr_t to_chars<char8_t , uint16_t >(uint16_t , std::span<char8_t , to_chars_dec_max_size_v<uint16_t >>);
		template uintptr_t to_chars<char8_t , uint32_t >(uint32_t , std::span<char8_t , to_chars_dec_max_size_v<uint32_t >>);
		template uintptr_t to_chars<char8_t , uint64_t >(uint64_t , std::span<char8_t , to_chars_dec_max_size_v<uint64_t >>);
		template uintptr_t to_chars<char8_t , int8_t   >(int8_t   , std::span<char8_t , to_chars_dec_max_size_v<int8_t   >>);
		template uintptr_t to_chars<char8_t , int16_t  >(int16_t  , std::span<char8_t , to_chars_dec_max_size_v<int16_t  >>);
		template uintptr_t to_chars<char8_t , int32_t  >(int32_t  , std::span<char8_t , to_chars_dec_max_size_v<int32_t  >>);
		template uintptr_t to_chars<char8_t , int64_t  >(int64_t  , std::span<char8_t , to_chars_dec_max_size_v<int64_t  >>);
		template uintptr_t to_chars<char8_t , float32_t>(float32_t, std::span<char8_t , to_chars_dec_max_size_v<float32_t>>);
		template uintptr_t to_chars<char8_t , float64_t>(float64_t, std::span<char8_t , to_chars_dec_max_size_v<float64_t>>);

		template uintptr_t to_chars<char16_t, uint8_t  >(uint8_t  , std::span<char16_t, to_chars_dec_max_size_v<uint8_t  >>);
		template uintptr_t to_chars<char16_t, uint16_t >(uint16_t , std::span<char16_t, to_chars_dec_max_size_v<uint16_t >>);
		template uintptr_t to_chars<char16_t, uint32_t >(uint32_t , std::span<char16_t, to_chars_dec_max_size_v<uint32_t >>);
		template uintptr_t to_chars<char16_t, uint64_t >(uint64_t , std::span<char16_t, to_chars_dec_max_size_v<uint64_t >>);
		template uintptr_t to_chars<char16_t, int8_t   >(int8_t   , std::span<char16_t, to_chars_dec_max_size_v<int8_t   >>);
		template uintptr_t to_chars<char16_t, int16_t  >(int16_t  , std::span<char16_t, to_chars_dec_max_size_v<int16_t  >>);
		template uintptr_t to_chars<char16_t, int32_t  >(int32_t  , std::span<char16_t, to_chars_dec_max_size_v<int32_t  >>);
		template uintptr_t to_chars<char16_t, int64_t  >(int64_t  , std::span<char16_t, to_chars_dec_max_size_v<int64_t  >>);
		template uintptr_t to_chars<char16_t, float32_t>(float32_t, std::span<char16_t, to_chars_dec_max_size_v<float32_t>>);
		template uintptr_t to_chars<char16_t, float64_t>(float64_t, std::span<char16_t, to_chars_dec_max_size_v<float64_t>>);

		template uintptr_t to_chars<char32_t, uint8_t  >(uint8_t  , std::span<char32_t, to_chars_dec_max_size_v<uint8_t  >>);
		template uintptr_t to_chars<char32_t, uint16_t >(uint16_t , std::span<char32_t, to_chars_dec_max_size_v<uint16_t >>);
		template uintptr_t to_chars<char32_t, uint32_t >(uint32_t , std::span<char32_t, to_chars_dec_max_size_v<uint32_t >>);
		template uintptr_t to_chars<char32_t, uint64_t >(uint64_t , std::span<char32_t, to_chars_dec_max_size_v<uint64_t >>);
		template uintptr_t to_chars<char32_t, int8_t   >(int8_t   , std::span<char32_t, to_chars_dec_max_size_v<int8_t   >>);
		template uintptr_t to_chars<char32_t, int16_t  >(int16_t  , std::span<char32_t, to_chars_dec_max_size_v<int16_t  >>);
		template uintptr_t to_chars<char32_t, int32_t  >(int32_t  , std::span<char32_t, to_chars_dec_max_size_v<int32_t  >>);
		template uintptr_t to_chars<char32_t, int64_t  >(int64_t  , std::span<char32_t, to_chars_dec_max_size_v<int64_t  >>);
		template uintptr_t to_chars<char32_t, float32_t>(float32_t, std::span<char32_t, to_chars_dec_max_size_v<float32_t>>);
		template uintptr_t to_chars<char32_t, float64_t>(float64_t, std::span<char32_t, to_chars_dec_max_size_v<float64_t>>);


		template uintptr_t to_chars_hex<char8_t , uint8_t >(uint8_t , std::span<char8_t , to_chars_hex_max_size_v<uint8_t >>);
		template uintptr_t to_chars_hex<char8_t , uint16_t>(uint16_t, std::span<char8_t , to_chars_hex_max_size_v<uint16_t>>);
		template uintptr_t to_chars_hex<char8_t , uint32_t>(uint32_t, std::span<char8_t , to_chars_hex_max_size_v<uint32_t>>);
		template uintptr_t to_chars_hex<char8_t , uint64_t>(uint64_t, std::span<char8_t , to_chars_hex_max_size_v<uint64_t>>);

		template uintptr_t to_chars_hex<char16_t, uint8_t >(uint8_t , std::span<char16_t, to_chars_hex_max_size_v<uint8_t >>);
		template uintptr_t to_chars_hex<char16_t, uint16_t>(uint16_t, std::span<char16_t, to_chars_hex_max_size_v<uint16_t>>);
		template uintptr_t to_chars_hex<char16_t, uint32_t>(uint32_t, std::span<char16_t, to_chars_hex_max_size_v<uint32_t>>);
		template uintptr_t to_chars_hex<char16_t, uint64_t>(uint64_t, std::span<char16_t, to_chars_hex_max_size_v<uint64_t>>);

		template uintptr_t to_chars_hex<char32_t, uint8_t >(uint8_t , std::span<char32_t, to_chars_hex_max_size_v<uint8_t >>);
		template uintptr_t to_chars_hex<char32_t, uint16_t>(uint16_t, std::span<char32_t, to_chars_hex_max_size_v<uint16_t>>);
		template uintptr_t to_chars_hex<char32_t, uint32_t>(uint32_t, std::span<char32_t, to_chars_hex_max_size_v<uint32_t>>);
		template uintptr_t to_chars_hex<char32_t, uint64_t>(uint64_t, std::span<char32_t, to_chars_hex_max_size_v<uint64_t>>);


		template void to_chars_hex_fix<char8_t , uint8_t >(uint8_t , std::span<char8_t , to_chars_hex_max_size_v<uint8_t >>);
		template void to_chars_hex_fix<char8_t , uint16_t>(uint16_t, std::span<char8_t , to_chars_hex_max_size_v<uint16_t>>);
		template void to_chars_hex_fix<char8_t , uint32_t>(uint32_t, std::span<char8_t , to_chars_hex_max_size_v<uint32_t>>);
		template void to_chars_hex_fix<char8_t , uint64_t>(uint64_t, std::span<char8_t , to_chars_hex_max_size_v<uint64_t>>);

		template void to_chars_hex_fix<char16_t, uint8_t >(uint8_t , std::span<char16_t, to_chars_hex_max_size_v<uint8_t >>);
		template void to_chars_hex_fix<char16_t, uint16_t>(uint16_t, std::span<char16_t, to_chars_hex_max_size_v<uint16_t>>);
		template void to_chars_hex_fix<char16_t, uint32_t>(uint32_t, std::span<char16_t, to_chars_hex_max_size_v<uint32_t>>);
		template void to_chars_hex_fix<char16_t, uint64_t>(uint64_t, std::span<char16_t, to_chars_hex_max_size_v<uint64_t>>);

		template void to_chars_hex_fix<char32_t, uint8_t >(uint8_t , std::span<char32_t, to_chars_hex_max_size_v<uint8_t >>);
		template void to_chars_hex_fix<char32_t, uint16_t>(uint16_t, std::span<char32_t, to_chars_hex_max_size_v<uint16_t>>);
		template void to_chars_hex_fix<char32_t, uint32_t>(uint32_t, std::span<char32_t, to_chars_hex_max_size_v<uint32_t>>);
		template void to_chars_hex_fix<char32_t, uint64_t>(uint64_t, std::span<char32_t, to_chars_hex_max_size_v<uint64_t>>);


		template uintptr_t to_chars_bin<char8_t , uint8_t >(uint8_t , std::span<char8_t , to_chars_bin_max_size_v<uint8_t >>);
		template uintptr_t to_chars_bin<char8_t , uint16_t>(uint16_t, std::span<char8_t , to_chars_bin_max_size_v<uint16_t>>);
		template uintptr_t to_chars_bin<char8_t , uint32_t>(uint32_t, std::span<char8_t , to_chars_bin_max_size_v<uint32_t>>);
		template uintptr_t to_chars_bin<char8_t , uint64_t>(uint64_t, std::span<char8_t , to_chars_bin_max_size_v<uint64_t>>);

		template uintptr_t to_chars_bin<char16_t, uint8_t >(uint8_t , std::span<char16_t, to_chars_bin_max_size_v<uint8_t >>);
		template uintptr_t to_chars_bin<char16_t, uint16_t>(uint16_t, std::span<char16_t, to_chars_bin_max_size_v<uint16_t>>);
		template uintptr_t to_chars_bin<char16_t, uint32_t>(uint32_t, std::span<char16_t, to_chars_bin_max_size_v<uint32_t>>);
		template uintptr_t to_chars_bin<char16_t, uint64_t>(uint64_t, std::span<char16_t, to_chars_bin_max_size_v<uint64_t>>);

		template uintptr_t to_chars_bin<char32_t, uint8_t >(uint8_t , std::span<char32_t, to_chars_bin_max_size_v<uint8_t >>);
		template uintptr_t to_chars_bin<char32_t, uint16_t>(uint16_t, std::span<char32_t, to_chars_bin_max_size_v<uint16_t>>);
		template uintptr_t to_chars_bin<char32_t, uint32_t>(uint32_t, std::span<char32_t, to_chars_bin_max_size_v<uint32_t>>);
		template uintptr_t to_chars_bin<char32_t, uint64_t>(uint64_t, std::span<char32_t, to_chars_bin_max_size_v<uint64_t>>);


		template void to_chars_bin_fix<char8_t , uint8_t >(uint8_t , std::span<char8_t , to_chars_bin_max_size_v<uint8_t >>);
		template void to_chars_bin_fix<char8_t , uint16_t>(uint16_t, std::span<char8_t , to_chars_bin_max_size_v<uint16_t>>);
		template void to_chars_bin_fix<char8_t , uint32_t>(uint32_t, std::span<char8_t , to_chars_bin_max_size_v<uint32_t>>);
		template void to_chars_bin_fix<char8_t , uint64_t>(uint64_t, std::span<char8_t , to_chars_bin_max_size_v<uint64_t>>);
		
		template void to_chars_bin_fix<char16_t, uint8_t >(uint8_t , std::span<char16_t, to_chars_bin_max_size_v<uint8_t >>);
		template void to_chars_bin_fix<char16_t, uint16_t>(uint16_t, std::span<char16_t, to_chars_bin_max_size_v<uint16_t>>);
		template void to_chars_bin_fix<char16_t, uint32_t>(uint32_t, std::span<char16_t, to_chars_bin_max_size_v<uint32_t>>);
		template void to_chars_bin_fix<char16_t, uint64_t>(uint64_t, std::span<char16_t, to_chars_bin_max_size_v<uint64_t>>);
		
		template void to_chars_bin_fix<char32_t, uint8_t >(uint8_t , std::span<char32_t, to_chars_bin_max_size_v<uint8_t >>);
		template void to_chars_bin_fix<char32_t, uint16_t>(uint16_t, std::span<char32_t, to_chars_bin_max_size_v<uint16_t>>);
		template void to_chars_bin_fix<char32_t, uint32_t>(uint32_t, std::span<char32_t, to_chars_bin_max_size_v<uint32_t>>);
		template void to_chars_bin_fix<char32_t, uint64_t>(uint64_t, std::span<char32_t, to_chars_bin_max_size_v<uint64_t>>);

	} //namespace

} //namespace core

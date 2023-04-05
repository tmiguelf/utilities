//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///
///	\copyright
///		Copyright (c) Tiago Miguel Oliveira Freire
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
//======== ======== ======== ======== ======== ======== ======== ========


#include <array>
#include <algorithm>

#include <CoreLib/string/core_fp_to_chars_round.hpp>
#include <CoreLib/Core_Type.hpp>
#include <CoreLib/cpu/x64.hpp>

#include "fp_traits.hpp"

#define USE_ORDER_REDUCE 1

namespace core
{
	using ::core::literals::operator "" _ui64;
	using ::core::literals::operator "" _ui32;
	using ::core::literals::operator "" _ui16;
	using ::core::literals::operator "" _i16;
	using ::core::literals::operator "" _ui8;

	CORE_MAKE_ENUM_ORDERABLE(fp_round)


	struct fp_common_utils
	{
		static constexpr uint64_t max_pow_10		= 10000000000000000000_ui64;
		static constexpr uint8_t  max_pow_10_digits	= 19_ui8;

		static constexpr std::array<uint64_t, 16> pow_5_low_table
		{
			          1_ui64,
			          5_ui64,
			         25_ui64,
			        125_ui64,
			        625_ui64,
			       3125_ui64,
			      15625_ui64,
			      78125_ui64,
			     390625_ui64,
			    1953125_ui64,
			    9765625_ui64,
			   48828125_ui64,
			  244140625_ui64,
			 1220703125_ui64,
			 6103515625_ui64,
			30517578125_ui64,
		};

		static constexpr std::array<uint64_t, max_pow_10_digits> pow_10_table
		{
			                  1_ui64,
			                 10_ui64,
			                100_ui64,
			               1000_ui64,
			              10000_ui64,
			             100000_ui64,
			            1000000_ui64,
			           10000000_ui64,
			          100000000_ui64,
			         1000000000_ui64,
			        10000000000_ui64,
			       100000000000_ui64,
			      1000000000000_ui64,
			     10000000000000_ui64,
			    100000000000000_ui64,
			   1000000000000000_ui64,
			  10000000000000000_ui64,
			 100000000000000000_ui64,
			1000000000000000000_ui64,
		};

		static constexpr uint16_t pow_5_low_mask  = 0x0F_ui16;
		static constexpr uint8_t  pow_5_hi_offset = 4_ui8;

		[[nodiscard]] inline static constexpr uint64_t pow_2_low_table(uint16_t p_offset)
		{
			return 1_ui64 << p_offset;
		}

		static constexpr uint16_t pow_2_low_mask = 0x1F_ui16;
		static constexpr uint8_t pow_2_hi_offset = 5_ui8;

		[[nodiscard]] inline static constexpr uint16_t num_digits(const uint64_t p_val)
		{
			if(p_val <                  10_ui64) return  1;
			if(p_val <                 100_ui64) return  2;
			if(p_val <                1000_ui64) return  3;
			if(p_val <               10000_ui64) return  4;
			if(p_val <              100000_ui64) return  5;
			if(p_val <             1000000_ui64) return  6;
			if(p_val <            10000000_ui64) return  7;
			if(p_val <           100000000_ui64) return  8;
			if(p_val <          1000000000_ui64) return  9;
			if(p_val <         10000000000_ui64) return 10;
			if(p_val <        100000000000_ui64) return 11;
			if(p_val <       1000000000000_ui64) return 12;
			if(p_val <      10000000000000_ui64) return 13;
			if(p_val <     100000000000000_ui64) return 14;
			if(p_val <    1000000000000000_ui64) return 15;
			if(p_val <   10000000000000000_ui64) return 16;
			if(p_val <  100000000000000000_ui64) return 17;
			if(p_val < 1000000000000000000_ui64) return 18;
			return 19;
		}

		//
		[[nodiscard]] inline static uint16_t leading_0(uint64_t p_val)
		{
			uint16_t out = 0;
			while(!(p_val % 10))
			{
				p_val /= 10;
				++out;
			}
			return out;
		}

		template<_p::charconv_char_c char_t>
		inline static void output_19_digits(uint64_t p_val, char_t* p_out)
		{
			p_out += 18;
#if 0
			uint64_t rem;
			p_val = core::udiv(0, p_val, 10, rem); *(  p_out) = static_cast<char_t>('0' + rem);	// 1
			p_val = core::udiv(0, p_val, 10, rem); *(--p_out) = static_cast<char_t>('0' + rem);	// 2
			p_val = core::udiv(0, p_val, 10, rem); *(--p_out) = static_cast<char_t>('0' + rem);	// 3
			p_val = core::udiv(0, p_val, 10, rem); *(--p_out) = static_cast<char_t>('0' + rem);	// 4
			p_val = core::udiv(0, p_val, 10, rem); *(--p_out) = static_cast<char_t>('0' + rem);	// 5
			p_val = core::udiv(0, p_val, 10, rem); *(--p_out) = static_cast<char_t>('0' + rem);	// 6
			p_val = core::udiv(0, p_val, 10, rem); *(--p_out) = static_cast<char_t>('0' + rem);	// 7
			p_val = core::udiv(0, p_val, 10, rem); *(--p_out) = static_cast<char_t>('0' + rem);	// 8
			p_val = core::udiv(0, p_val, 10, rem); *(--p_out) = static_cast<char_t>('0' + rem);	// 9
			p_val = core::udiv(0, p_val, 10, rem); *(--p_out) = static_cast<char_t>('0' + rem);	// 10
			p_val = core::udiv(0, p_val, 10, rem); *(--p_out) = static_cast<char_t>('0' + rem);	// 11
			p_val = core::udiv(0, p_val, 10, rem); *(--p_out) = static_cast<char_t>('0' + rem);	// 12
			p_val = core::udiv(0, p_val, 10, rem); *(--p_out) = static_cast<char_t>('0' + rem);	// 13
			p_val = core::udiv(0, p_val, 10, rem); *(--p_out) = static_cast<char_t>('0' + rem);	// 14
			p_val = core::udiv(0, p_val, 10, rem); *(--p_out) = static_cast<char_t>('0' + rem);	// 15
			p_val = core::udiv(0, p_val, 10, rem); *(--p_out) = static_cast<char_t>('0' + rem);	// 16
			p_val = core::udiv(0, p_val, 10, rem); *(--p_out) = static_cast<char_t>('0' + rem);	// 17
			p_val = core::udiv(0, p_val, 10, rem); *(--p_out) = static_cast<char_t>('0' + rem);	// 18
#else
			*(  p_out) = static_cast<char_t>('0' + p_val % 10); p_val /= 10;	// 1
			*(--p_out) = static_cast<char_t>('0' + p_val % 10); p_val /= 10;	// 2
			*(--p_out) = static_cast<char_t>('0' + p_val % 10); p_val /= 10;	// 3
			*(--p_out) = static_cast<char_t>('0' + p_val % 10); p_val /= 10;	// 4
			*(--p_out) = static_cast<char_t>('0' + p_val % 10); p_val /= 10;	// 5
			*(--p_out) = static_cast<char_t>('0' + p_val % 10); p_val /= 10;	// 6
			*(--p_out) = static_cast<char_t>('0' + p_val % 10); p_val /= 10;	// 7
			*(--p_out) = static_cast<char_t>('0' + p_val % 10); p_val /= 10;	// 8
			*(--p_out) = static_cast<char_t>('0' + p_val % 10); p_val /= 10;	// 9
			*(--p_out) = static_cast<char_t>('0' + p_val % 10); p_val /= 10;	// 10
			*(--p_out) = static_cast<char_t>('0' + p_val % 10); p_val /= 10;	// 11
			*(--p_out) = static_cast<char_t>('0' + p_val % 10); p_val /= 10;	// 12
			*(--p_out) = static_cast<char_t>('0' + p_val % 10); p_val /= 10;	// 13
			*(--p_out) = static_cast<char_t>('0' + p_val % 10); p_val /= 10;	// 14
			*(--p_out) = static_cast<char_t>('0' + p_val % 10); p_val /= 10;	// 15
			*(--p_out) = static_cast<char_t>('0' + p_val % 10); p_val /= 10;	// 16
			*(--p_out) = static_cast<char_t>('0' + p_val % 10); p_val /= 10;	// 17
			*(--p_out) = static_cast<char_t>('0' + p_val % 10); p_val /= 10;	// 18
#endif
			*(--p_out) = static_cast<char_t>('0' + p_val);
		}

		template<_p::charconv_char_c char_t>
		inline static void output_sig_digits(uint64_t p_val, char_t* p_out, uint16_t sig_digits)
		{
			if(sig_digits)
			{
				p_out += sig_digits;
				while(--sig_digits)
				{
					*(--p_out) = static_cast<char_t>('0' + p_val % 10);
					p_val /= 10;
				}
				*(--p_out) = static_cast<char_t>('0' + p_val);
			}
		}


		inline static void fix_rounding_mode(fp_round& rounding_mode, const bool sign_bit)
		{
			if(rounding_mode >= fp_round::to_inf)
			{
				using enum_round_t = std::underlying_type_t<fp_round>;
				if(static_cast<bool>(
					static_cast<enum_round_t>(rounding_mode) - static_cast<enum_round_t>(fp_round::to_inf)) == sign_bit)
				{
					rounding_mode = fp_round::away_zero;
				}
				else
				{
					rounding_mode = fp_round::to_zero;
				}
			}
		}

	};

	template<_p::charconv_fp_c T>
	struct fp_utils_pre;

	template<>
	struct fp_utils_pre<float32_t>: public fp_common_utils, public fp_traits<float32_t>
	{
		using fp_type = float32_t;
		using uint_t = uint32_t;
		using bignum_t = fp_to_chars_round_context<fp_type>::bignum_t;


		static constexpr uint8_t bignum_width = fp_to_chars_round_context<fp_type>::bignum_width;

		static constexpr std::array pow_2_hack_table
		{
			bignum_t{                  1_ui64, 0, 0, 0, 0, 0},
			bignum_t{         4294967296_ui64, 0, 0, 0, 0, 0},
			bignum_t{8446744073709551616_ui64,          1_ui64, 0, 0, 0, 0},
			bignum_t{4264337593543950336_ui64, 7922816251_ui64, 0, 0, 0, 0},
		};

		static constexpr std::array pow_5_hack_table
		{
			bignum_t{                  1_ui64, 0, 0, 0, 0, 0},
			bignum_t{       152587890625_ui64, 0, 0, 0, 0, 0},
			bignum_t{3064365386962890625_ui64,                2328_ui64, 0, 0, 0, 0},
			bignum_t{ 929355621337890625_ui64,     355271367880050_ui64, 0, 0, 0, 0},
			bignum_t{ 434970855712890625_ui64, 8624275221700372640_ui64,             5421010_ui64, 0, 0, 0},
			bignum_t{6581211090087890625_ui64, 8714086920699628535_ui64,  827180612553027674_ui64, 0, 0, 0},
			bignum_t{4368076324462890625_ui64, 2457967477130296174_ui64, 5361888865876570445_ui64,         12621774483_ui64, 0, 0},
			bignum_t{8795566558837890625_ui64,  164821538819523993_ui64, 9779425849273185381_ui64, 5929944387235853055_ui64,                 192_ui64, 0},
			bignum_t{4863681793212890625_ui64, 8037718792656960431_ui64, 4194546663891930218_ui64, 1876992184134305561_ui64,      29387358770557_ui64, 0},
			bignum_t{7572422027587890625_ui64, 8447331464594753924_ui64, 1400485046962261850_ui64, 6665277316200968382_ui64, 5085839414626955934_ui64, 448415_ui64},
		};

		[[nodiscard]] static inline exp_ut last_block(const bignum_t& p_val)
		{
			if(p_val[5]) return 5;
			if(p_val[4]) return 4;
			if(p_val[3]) return 3;
			if(p_val[2]) return 2;
			if(p_val[1]) return 1;
			return 0;
		}

		[[nodiscard]] static inline exp_ut leading_zeros(const bignum_t& p_val)
		{
			if(p_val[0]) return                         leading_0(p_val[0]);
			if(p_val[1]) return     max_pow_10_digits + leading_0(p_val[1]);
			if(p_val[2]) return 2 * max_pow_10_digits + leading_0(p_val[2]);
			if(p_val[3]) return 3 * max_pow_10_digits + leading_0(p_val[3]);
			if(p_val[4]) return 4 * max_pow_10_digits + leading_0(p_val[4]);
			if(p_val[5]) return 5 * max_pow_10_digits + leading_0(p_val[5]); //can skip last check
			return 6 * max_pow_10_digits;
		}


		static inline void exp_load(const exp_st exponent, fp_to_chars_sci_size& p_out)
		{
			if(exponent < 0)
			{
				p_out.is_exp_negative = true;
				p_out.exponent_size = (-exponent < 10) ? 1 : 2;
			}
			else
			{
				p_out.is_exp_negative = false;
				p_out.exponent_size = (exponent < 10) ? 1 : 2;
			}
		}

		template<_p::charconv_char_c char_t>
		static inline void to_chars_exp(exp_st exponent, char_t* exp_char)
		{
			if(exponent < 0)
			{
				exponent = -exponent;
			}

			if(exponent < 10)
			{
				*exp_char = static_cast<char_t>('0' + exponent);
			}
			else
			{
				*(exp_char++) = static_cast<char_t>('0' + exponent / 10);
				*exp_char = static_cast<char_t>('0' + exponent % 10);
			}
		}
	};

	template<>
	struct fp_utils_pre<float64_t>: public fp_common_utils, public fp_traits<float64_t>
	{
		using fp_type = float64_t;
		using uint_t = uint64_t;
		using bignum_t = fp_to_chars_round_context<fp_type>::bignum_t;

		static constexpr uint8_t bignum_width = fp_to_chars_round_context<fp_type>::bignum_width;

		[[nodiscard]] static inline uint_t get_mantissa(fp_type input)
		{
			return reinterpret_cast<const uint_t&>(input) & mantissa_mask;
		}

		[[nodiscard]] static inline uint_t get_exponent_bits(fp_type input)
		{
			return (reinterpret_cast<const uint_t&>(input) & exponent_mask);
		}

		[[nodiscard]] static inline bool get_sign(const fp_type input)
		{
			return reinterpret_cast<const uint_t&>(input) & sign_mask;
		}


		static constexpr std::array pow_2_hack_table
		{
			bignum_t{1_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{4294967296_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{8446744073709551616_ui64, 1_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{4264337593543950336_ui64, 7922816251_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{3374607431768211456_ui64, 4028236692093846346_ui64, 3_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{6283019655932542976_ui64, 3090291820368483271_ui64, 14615016373_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{2355444464034512896_ui64, 3578942320766641610_ui64, 2771017353866807638_ui64, 6_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{572481103610249216_ui64, 1963067363714442254_ui64, 1506397946670150870_ui64, 26959946667_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{7584007913129639936_ui64, 6998466564056403945_ui64, 5709850086879078532_ui64, 5792089237316195423_ui64, 11_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{9497012533375533056_ui64, 4771744046397689315_ui64, 8208401004561507973_ui64, 9786642155382248146_ui64, 49732323640_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{550022962086936576_ui64, 4160782221972578064_ui64, 7045223566527699470_ui64, 5021706169552114602_ui64, 3598703592091008239_ui64, 21_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{4212440502746218496_ui64, 9150435393923228007_ui64, 1244949503553575476_ui64, 8347763186259956673_ui64, 6028604644328358120_ui64, 91739944639_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{5640806627990306816_ui64, 1061141426625488491_ui64, 2934042457217714972_ui64, 9739270465446667948_ui64, 7904010014361380507_ui64, 4020061963944792122_ui64, 39_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{278405979725889536_ui64, 3787256711292098335_ui64, 8825564957045312484_ui64, 5992136870091590247_ui64, 8938619607159883885_ui64, 3036413316903188563_ui64, 169230328010_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{7933534601628614656_ui64, 6556076252160626617_ui64, 9233261910507137635_ui64, 199180639288113397_ui64, 4136068731806028149_ui64, 3238078880045343536_ui64, 6838724295606890549_ui64, 72_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{5679288285306290176_ui64, 3112140662289544797_ui64, 1030634919711598269_ui64, 1738819765620120306_ui64, 7115085915695962537_ui64, 1663057485981426649_ui64, 5992231381597229793_ui64, 312174855031_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{9946433649006084096_ui64, 5085375388281194656_ui64, 4276900318581864860_ui64, 6976801874298166903_ui64, 6144372176403007354_ui64, 3658205923933777235_ui64, 4024998205846127479_ui64, 780792994259709957_ui64, 134_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{4315074097345724416_ui64, 551032703601930899_ui64, 6561090674575770658_ui64, 782129802971518987_ui64, 2103259504474008372_ui64, 5329036896713294315_ui64, 567793532123114264_ui64, 5291369997489289838_ui64, 575860965701_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{7916606772148699136_ui64, 996754615510189316_ui64, 3393513650343067512_ui64, 4989597671426016139_ui64, 6717031640106124304_ui64, 560928972251065318_ui64, 1349101211839914063_ui64, 252101964719003513_ui64, 3304014731045340605_ui64, 247_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{1670786438063456256_ui64, 5753317470388766254_ui64, 6753054144788190399_ui64, 8972493002030693158_ui64, 453045857133777865_ui64, 1197217044854783250_ui64, 3846398888276400807_ui64, 3725465918623545406_ui64, 5341973791764131049_ui64, 1062275985633_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{82874192246603776_ui64, 8825199467436026495_ui64, 9982526613197222141_ui64, 7310688704721375437_ui64, 9977864230957359440_ui64, 3865059128113173713_ui64, 2517899275167208677_ui64, 2850724855993057919_ui64, 1716057002913248932_ui64, 2440617622195218641_ui64, 456_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{5381579984990109696_ui64, 3314350768250101714_ui64, 5131711097432495045_ui64, 8014908298340170885_ui64, 5890049568456791129_ui64, 2352355288387350103_ui64, 1465266200982457647_ui64, 6384493366220246528_ui64, 5584180889271304874_ui64, 9369747791401605606_ui64, 1959553324262_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{6340692027772502016_ui64, 7106662012614916347_ui64, 5992835787380551135_ui64, 1810045036330430093_ui64, 8340831091600294086_ui64, 2196317275016988514_ui64, 3153818664580441415_ui64, 9444909971446875329_ui64, 4368451707817519724_ui64, 5838126082058648805_ui64, 6217442477397611585_ui64, 841_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{3290819886814068736_ui64, 7906605154864094201_ui64, 7097876634332969052_ui64, 5326329101623141899_ui64, 9883243404129619879_ui64, 7837803515619997819_ui64, 1887389439612274926_ui64, 1028623340798795186_ui64, 9231594754471504248_ui64, 2366508973300717001_ui64, 5183960948593180219_ui64, 3614737867146_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{892846853816057856_ui64, 2526299919371646875_ui64, 5502652856315984448_ui64, 9204454976020849905_ui64, 9144428727504118113_ui64, 2061715800441148143_ui64, 7950487730697131073_ui64, 8633087840882864647_ui64, 380260509526863768_ui64, 6017116696611139052_ui64, 7948846250255525688_ui64, 5180923007089351489_ui64, 1552_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{7476461291163877376_ui64, 3588660198169074803_ui64, 9118140247289650161_ui64, 9514014558463078285_ui64, 5233083004609515757_ui64, 6541193650803633560_ui64, 593433240445888801_ui64, 87155429074292991_ui64, 2378176320715214322_ui64, 8160396257811764037_ui64, 5779714475832231590_ui64, 8542740798517907212_ui64, 6668014432879_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{5358787106474295296_ui64, 7593055360673758361_ui64, 2450400082317452474_ui64, 2266801261478978776_ui64, 2051287430797921210_ui64, 7004450260415645796_ui64, 74396860757073376_ui64, 7542935950921899972_ui64, 1593452842658246283_ui64, 9702311064005352904_ui64, 3917217065252944144_ui64, 7839336748384907217_ui64, 8903918474961204418_ui64, 2863_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{8533568160966639616_ui64, 811276688203143392_ui64, 5669166269125482749_ui64, 4583758435450548517_ui64, 9972934782708331888_ui64, 4943882061420908858_ui64, 3876695953600699775_ui64, 5032221429955268920_ui64, 6935401493438227090_ui64, 5521953492303010368_ui64, 6594493070361825495_ui64, 6644157318691807150_ui64, 6208567847447683223_ui64, 12300315572313_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{9538580897737998336_ui64, 3839689876703647648_ui64, 1480282927515617388_ui64, 3024232908211188404_ui64, 6873151767642644105_ui64, 9083866839990050841_ui64, 7250538978462939576_ui64, 7888234755950268553_ui64, 8946743949932571286_ui64, 9474417255887657187_ui64, 902667390255672485_ui64, 3260361215221279607_ui64, 9784916516606518847_ui64, 9453113566524635233_ui64, 5282_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{6035023229622419456_ui64, 7224438936164109108_ui64, 2506715212358077092_ui64, 6254024057110212260_ui64, 2469747447241088223_ui64, 8976133330424292142_ui64, 871573830845597595_ui64, 9976944840666325467_ui64, 646255081946633685_ui64, 2695551072066953619_ui64, 313782405631281786_ui64, 2522213315724425364_ui64, 2915239349672942191_ui64, 3597228708266929611_ui64, 22690077338833_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{1828589991914110976_ui64, 2773880310427754708_ui64, 7463636154689656128_ui64, 3685597925394874945_ui64, 5279959347921837887_ui64, 8707896554900536483_ui64, 9531277699956473029_ui64, 4657798729631265341_ui64, 1834591698652030940_ui64, 9225304916231408668_ui64, 246561506588201025_ui64, 4572129486907664261_ui64, 6857595007526867906_ui64, 8238787518831087622_ui64, 3140113999990803533_ui64, 9745_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
		};


		static constexpr std::array pow_5_hack_table
		{
			bignum_t{1_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{152587890625_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{3064365386962890625_ui64, 2328_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{929355621337890625_ui64, 355271367880050_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{434970855712890625_ui64, 8624275221700372640_ui64, 5421010_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{6581211090087890625_ui64, 8714086920699628535_ui64, 827180612553027674_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{4368076324462890625_ui64, 2457967477130296174_ui64, 5361888865876570445_ui64, 12621774483_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{8795566558837890625_ui64, 164821538819523993_ui64, 9779425849273185381_ui64, 5929944387235853055_ui64, 192_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{4863681793212890625_ui64, 8037718792656960431_ui64, 4194546663891930218_ui64, 1876992184134305561_ui64, 29387358770557_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{7572422027587890625_ui64, 8447331464594753924_ui64, 1400485046962261850_ui64, 6665277316200968382_ui64, 5085839414626955934_ui64, 448415_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{1921787261962890625_ui64, 2705371819647552911_ui64, 7825599799306205209_ui64, 9766904013068924666_ui64, 5411977335590779360_ui64, 68422776578360208_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{2911777496337890625_ui64, 6338029661892505828_ui64, 1557191355977963556_ui64, 1034647641381832875_ui64, 8104760891218628129_ui64, 8797639242736470574_ui64, 1044048714_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{5542392730712890625_ui64, 6859409834469261113_ui64, 8160943089874720182_ui64, 8585338616290151305_ui64, 1045551926187860738_ui64, 8880397767711805591_ui64, 9309191113245227702_ui64, 15_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{4813632965087890625_ui64, 4044969719579967204_ui64, 4653348188162381507_ui64, 1070450716567124784_ui64, 5911367623677652226_ui64, 6103148056725340670_ui64, 5084793531500210078_ui64, 2430865342914_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{5725498199462890625_ui64, 2704956738489272538_ui64, 5363389191360011590_ui64, 8997312082723208437_ui64, 5891758134009562935_ui64, 7787577910024530390_ui64, 5261547639513367564_ui64, 615068742138573173_ui64, 37092_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{3277988433837890625_ui64, 7957845851524325553_ui64, 548643534926708045_ui64, 7559881215872027456_ui64, 1205281967122071574_ui64, 8489134306209688324_ui64, 6292658199883696136_ui64, 2296931995568048698_ui64, 5659799424266695_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{2471103668212890625_ui64, 3003777058074774686_ui64, 9509894683194669365_ui64, 5472939461496635969_ui64, 1507503472288226560_ui64, 6859180316242705797_ui64, 4362813850237034701_ui64, 2800399571116000364_ui64, 5509444462538635186_ui64, 86361685_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{8304843902587890625_ui64, 9397992896592768374_ui64, 4577234695492203644_ui64, 8095088188322367602_ui64, 6323632896010428794_ui64, 1994903129692130619_ui64, 7203781385180363712_ui64, 3745172796063443759_ui64, 4357175640875237596_ui64, 3177747429038154030_ui64, 1_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{5779209136962890625_ui64, 9824275944592955056_ui64, 719550314300428719_ui64, 9462829964862589958_ui64, 9091307801743899357_ui64, 933016911676527343_ui64, 8312334014814806056_ui64, 7454186969051839983_ui64, 3785033626482739197_ui64, 5948796148028192762_ui64, 201076468338_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{9894199371337890625_ui64, 3368386318652483168_ui64, 7658190915287413705_ui64, 1425469676506810215_ui64, 707299491315154493_ui64, 173239732228207424_ui64, 1028756117752333247_ui64, 1982419407842010724_ui64, 9695312155978194718_ui64, 5747186642227685950_ui64, 1834158110790956848_ui64, 3068_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{5649814605712890625_ui64, 3291499174411001149_ui64, 7244522909063511942_ui64, 8075289270907454151_ui64, 9914931204612269233_ui64, 3142001963917533741_ui64, 5392646334430537906_ui64, 6673999649486635097_ui64, 1633715545386038327_ui64, 5699150233387941833_ui64, 8327155849413858676_ui64, 468167635469219_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{8046054840087890625_ui64, 7303642206570657435_ui64, 8489981434501635351_ui64, 4462608600532881302_ui64, 3776433381275776228_ui64, 3783835142744921945_ui64, 7394089708587676451_ui64, 7155013662607518530_ui64, 1727039744171137229_ui64, 7642491632594536520_ui64, 7690838099657443746_ui64, 1955142186388486471_ui64, 7143671_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{2082920074462890625_ui64, 337133148896100465_ui64, 6016136433466220193_ui64, 295608955230429603_ui64, 4707066108133065804_ui64, 4193412075446065998_ui64, 5414497585812914165_ui64, 336689871125636422_ui64, 8613468889694422342_ui64, 6812823512680660158_ui64, 5112876398353448983_ui64, 2969737513593110651_ui64, 1090037719042086584_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{2760410308837890625_ui64, 9820017274214478675_ui64, 4821196446794634229_ui64, 8471314026125128684_ui64, 2452667851190322192_ui64, 3718566661179597597_ui64, 3347498385519593436_ui64, 9863956076271020291_ui64, 2532705998789823472_ui64, 344872873130323634_ui64, 1100003613476921275_ui64, 9105018846326849340_ui64, 318387496486473290_ui64, 16632655625_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{5078525543212890625_ui64, 3449504894415440503_ui64, 5138279721374775302_ui64, 8409528271807807110_ui64, 6882658820668243806_ui64, 7843871649973569506_ui64, 7027807063576761797_ui64, 5846602071342443626_ui64, 8622966459105775089_ui64, 4739324074290557842_ui64, 321239476697889299_ui64, 1884970109558160606_ui64, 245558305435468236_ui64, 7941837315649223274_ui64, 253_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{4037265777587890625_ui64, 9465408860451134387_ui64, 5789688468162611303_ui64, 6454942400181270162_ui64, 7317475409080936764_ui64, 2705308027384320383_ui64, 653350492002436065_ui64, 9899235031552231805_ui64, 5075544675393708363_ui64, 4277014596908057542_ui64, 4809768596486268223_ui64, 8654877629413444163_ui64, 1847570219192048790_ui64, 1827281803063328635_ui64, 38725919148493_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{4636631011962890625_ui64, 9423582062336208764_ui64, 2820328360983411566_ui64, 9534631741175666323_ui64, 1969110816268041994_ui64, 9453183552817923112_ui64, 3457651561055115163_ui64, 5660370645955665466_ui64, 1276300657642136101_ui64, 4535147477349009329_ui64, 2206454990799172926_ui64, 9697900921779916328_ui64, 8083616770246545858_ui64, 6879979734002231127_ui64, 6315382870899685715_ui64, 590910_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{1876621246337890625_ui64, 3469354929147812072_ui64, 2322321615960419652_ui64, 2165881848454717422_ui64, 1212462608865840777_ui64, 5430162383561871586_ui64, 7638422232867180676_ui64, 2043680777857232071_ui64, 2913833857122354827_ui64, 1975300465031443140_ui64, 5049579842150077474_ui64, 4640553012183456280_ui64, 2795693812529005764_ui64, 4149036835992687534_ui64, 9839739332275081390_ui64, 90165806814313825_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{757236480712890625_ui64, 8110972929025592748_ui64, 2241768241414717546_ui64, 8681247288853784423_ui64, 8523067812589749884_ui64, 8928159503363992444_ui64, 6305645224459264885_ui64, 4094249231543307996_ui64, 7755615900890775_ui64, 9729483076688138689_ui64, 6200786468128974380_ui64, 2563519364159848665_ui64, 7165186228035782046_ui64, 9548268780451870918_ui64, 1700651613602246642_ui64, 8297397763667897526_ui64, 1375821026_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{6278476715087890625_ui64, 6493034069171699229_ui64, 7587516298080031265_ui64, 199039188259403097_ui64, 6902753428363758680_ui64, 1859198738450131417_ui64, 9843885176516105703_ui64, 4220580540552447882_ui64, 814632478244769768_ui64, 3478193512081231932_ui64, 7843991237327317454_ui64, 3492533206188758363_ui64, 1280219460502919985_ui64, 9692217821734408997_ui64, 7569373009464550482_ui64, 4676746546945977990_ui64, 9933628361471523493_ui64, 20_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{3440341949462890625_ui64, 1169926395850779954_ui64, 6840708259476227862_ui64, 8214584008598576325_ui64, 8512982180771471988_ui64, 2766634779399924138_ui64, 9298355588671855750_ui64, 5820294644834673713_ui64, 9985621402291573189_ui64, 4035634167703742648_ui64, 4759133048678489869_ui64, 110834317687854573_ui64, 5216062217598468112_ui64, 6485747852786551826_ui64, 8003918910098538539_ui64, 6239317437276809836_ui64, 9614790873363442183_ui64, 3203332952292_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{7242832183837890625_ui64, 9379265494389983359_ui64, 4492776812285253842_ui64, 3913623914195377494_ui64, 2131345950539917123_ui64, 8396627632339205639_ui64, 6618614238992840261_ui64, 1306413868688287180_ui64, 5525694211125101392_ui64, 4091590728540365635_ui64, 1576212982721468623_ui64, 4850856548448685435_ui64, 2110002443097447474_ui64, 2322971576172824365_ui64, 5485006660243955273_ui64, 3849157226622297822_ui64, 7029141788319663044_ui64, 9818159936749128316_ui64, 48878_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{2685947418212890625_ui64, 815331989178957883_ui64, 5518469686573264990_ui64, 6628646003612927433_ui64, 5021533720605833651_ui64, 2227422975636992653_ui64, 8507120031686823003_ui64, 6391476541421726058_ui64, 4352629414465543650_ui64, 8781377495204074266_ui64, 9208469148126079231_ui64, 2253026604861648295_ui64, 5836101453412728095_ui64, 42780336151160325_ui64, 701183049361748904_ui64, 4271518333206278385_ui64, 3737647153460040689_ui64, 7432909653154629338_ui64, 7458340731200206_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{4769687652587890625_ui64, 7902509043669851962_ui64, 2219389355702697588_ui64, 1132726066871180520_ui64, 9552253976877533014_ui64, 2109474716515101937_ui64, 8775482669485824213_ui64, 4711617866624756342_ui64, 5626167394889473593_ui64, 5035746221201010352_ui64, 7949101543905296252_ui64, 7844283435187822694_ui64, 9744887577103067090_ui64, 3533985226991294149_ui64, 4114517822918401125_ui64, 9961911784380844869_ui64, 7003279026359837847_ui64, 1065267696192125754_ui64, 9736359671522669268_ui64, 113805247_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{8494052886962890625_ui64, 8568719860377314034_ui64, 2252439695778031061_ui64, 9826129184255819026_ui64, 6010286577342621869_ui64, 9809255669437469129_ui64, 3085970033408732856_ui64, 9334158764274332725_ui64, 9336180644403348794_ui64, 5876823816317251508_ui64, 8439979935237826861_ui64, 9938765685258507444_ui64, 7924055600651297533_ui64, 6513389583509637101_ui64, 8086109215301288253_ui64, 1002940278847012678_ui64, 551429442778194952_ui64, 2909813644459213659_ui64, 3961985489887507571_ui64, 7365302730352167839_ui64, 1_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{8859043121337890625_ui64, 1518865180878492537_ui64, 8786477944468992010_ui64, 4347385593451533443_ui64, 8461589047639910648_ui64, 785694824375710713_ui64, 9799325100849238120_ui64, 9477039384246368917_ui64, 3460204305025615909_ui64, 6611863545779123602_ui64, 5264108518628293095_ui64, 9728308844604186023_ui64, 8598866881389857257_ui64, 6582809721741571054_ui64, 6197548469747765861_ui64, 2114971742036578602_ui64, 2043907486006779157_ui64, 9875131081412192425_ui64, 8792048842007654486_ui64, 8990454009429710233_ui64, 264973491368_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{864658355712890625_ui64, 4008260785813035909_ui64, 9609376549054949659_ui64, 8283301373084685431_ui64, 4976600408132714743_ui64, 6469530310998368036_ui64, 7200640881129376592_ui64, 2202288510121537793_ui64, 5402818290360098026_ui64, 769592643130558853_ui64, 8584761911191765530_ui64, 6683658375001655862_ui64, 6450387263920728219_ui64, 6289528634626264669_ui64, 5008218636822658107_ui64, 8843170565758491054_ui64, 2421136049117610809_ui64, 8061588318541068817_ui64, 3921888273241135885_ui64, 9953343792603284332_ui64, 1746119521949066305_ui64, 4043_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{9510898590087890625_ui64, 2116074994883092585_ui64, 1636605743534252713_ui64, 158299554586434159_ui64, 485037815602132768_ui64, 9741216506600958376_ui64, 9672941993248448348_ui64, 7119414324960950944_ui64, 6216129489170662080_ui64, 5810371939129917760_ui64, 6595082104028208055_ui64, 9617898995739164904_ui64, 6029086253107580184_ui64, 3158061164098926756_ui64, 1582352848656137613_ui64, 6176003189212320508_ui64, 1002626019945839931_ui64, 3322268255937744890_ui64, 788556794152653786_ui64, 8772633726835453388_ui64, 3341416320088864022_ui64, 616939485466338_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{9797763824462890625_ui64, 3518766166853311005_ui64, 651353944642257612_ui64, 1221032587228481194_ui64, 6087153322727959512_ui64, 3671627204666714715_ui64, 7763725124225105386_ui64, 1199790186659881488_ui64, 4410107530508641433_ui64, 8525048922975608660_ui64, 2351082699743224168_ui64, 4145035501023591675_ui64, 7870511621366867000_ui64, 4587212616410617875_ui64, 899904375580767274_ui64, 5180741601293225840_ui64, 9274891909423876788_ui64, 3938124099771515196_ui64, 7765775502353946776_ui64, 3554288952462109285_ui64, 2309573803293361310_ui64, 4730581410849248048_ui64, 9413749_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{6725254058837890625_ui64, 2763520199550839606_ui64, 3235109306621798200_ui64, 9580260324880278696_ui64, 6019212665115853686_ui64, 1459154603380635417_ui64, 7824918574090991282_ui64, 5006330684871294009_ui64, 9741429614254611002_ui64, 1776223822175713050_ui64, 8748805061722548263_ui64, 6929882085435221858_ui64, 3918914858170097366_ui64, 6483440519264398519_ui64, 4077037728686715948_ui64, 4518103041075201043_ui64, 3867918169872155781_ui64, 3612548359297056723_ui64, 1488460753450379940_ui64, 7934156154873283422_ui64, 7292680413120864142_ui64, 1323249546960049488_ui64, 1436424174966147016_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{5293369293212890625_ui64, 9041688029865326825_ui64, 8726532024589846794_ui64, 1858931934129475503_ui64, 3312635339054265211_ui64, 5609652344043162004_ui64, 2927136557909463552_ui64, 5722352566908547578_ui64, 1018524551488200147_ui64, 3667152553427575901_ui64, 7566568478030759005_ui64, 6536538524609452235_ui64, 7146204634550202719_ui64, 7209246733381879549_ui64, 8847038062796638782_ui64, 4062659211380580576_ui64, 902676613130234391_ui64, 5942493192564669340_ui64, 7091665704537175892_ui64, 1475131553473827414_ui64, 366232726046083906_ui64, 1120832630808722124_ui64, 840303975269310714_ui64, 21918093490_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{502109527587890625_ui64, 6462223133748921100_ui64, 3675353226344116666_ui64, 4268112773537170785_ui64, 6122010984175597139_ui64, 7132874196054616379_ui64, 2728203210596501831_ui64, 7129452254137215544_ui64, 1358665095861092634_ui64, 7596420456215279411_ui64, 2330170496630135397_ui64, 9205968590798469208_ui64, 614428658089271920_ui64, 1912773088778757241_ui64, 1225400195453593954_ui64, 2788784314682587628_ui64, 4061644196512744314_ui64, 6864828504984689531_ui64, 2982161345558062630_ui64, 8950716355768824837_ui64, 5215439780809502463_ui64, 8772296758517955414_ui64, 146288164075204937_ui64, 4435652173466552357_ui64, 334_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{7351474761962890625_ui64, 6825119026216270867_ui64, 9636942542453355922_ui64, 3655210169606830235_ui64, 8434622104921154381_ui64, 9466610658600021120_ui64, 1272862505079257141_ui64, 449149211550952310_ui64, 3257543138571964327_ui64, 4489656240214005979_ui64, 7401031152289130496_ui64, 9942200963289955087_ui64, 7391345355315682842_ui64, 1016424722510202912_ui64, 5726616545299882336_ui64, 5502262591940310599_ui64, 5152635845776127848_ui64, 7986073517786318916_ui64, 2116490278088813857_ui64, 9452032058443624047_ui64, 5434335921489202205_ui64, 8779817889639889233_ui64, 9639425335270641456_ui64, 5457818147984133362_ui64, 51032038149619_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{841464996337890625_ui64, 4894846761344524566_ui64, 7282092694475082797_ui64, 1354711298171120337_ui64, 8916378992541659082_ui64, 3919238409952119520_ui64, 5697195731607218301_ui64, 6441697712293434220_ui64, 4638111426531207451_ui64, 5623470966454210488_ui64, 7711562568506717532_ui64, 8257307089249132572_ui64, 8511175993991103565_ui64, 6932817495595479664_ui64, 5533803726446910438_ui64, 9017120296121468281_ui64, 5742164390598489256_ui64, 5187822954599545986_ui64, 1891762969758201870_ui64, 7789313844550938081_ui64, 7702973319275391516_ui64, 1789739005040526145_ui64, 6130593395688139185_ui64, 5743005823355489124_ui64, 1055544974637117736_ui64, 778687_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{5972080230712890625_ui64, 6173792432273330632_ui64, 5675475691144512349_ui64, 3786672626985056983_ui64, 4994366780925160189_ui64, 1072924940740646248_ui64, 3699081633289160308_ui64, 2743258753924495416_ui64, 8106605780665553471_ui64, 2178114084443838905_ui64, 1147084436393294776_ui64, 1383754390130044075_ui64, 1240168125264085224_ui64, 1009470344713989166_ui64, 1298589888501243524_ui64, 7050213860073909400_ui64, 3412057022355961549_ui64, 8299863674044256448_ui64, 7889689715390364907_ui64, 6136792173641402736_ui64, 8886645016316434395_ui64, 8420127223250935657_ui64, 7609128597865890579_ui64, 2905445039834493952_ui64, 9696920253646939443_ui64, 118818222893447488_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{7743320465087890625_ui64, 7175694671204837504_ui64, 5205131472378424564_ui64, 9077298517997370775_ui64, 8941679287516366988_ui64, 6568336257848614210_ui64, 3272752091566847850_ui64, 9904696101365381367_ui64, 188212779895916044_ui64, 5888504914956753245_ui64, 8019832796907258368_ui64, 3026744860032416326_ui64, 4407535421505041353_ui64, 8399225787966432575_ui64, 3358692032004857554_ui64, 3816741963174284187_ui64, 3514640763095304261_ui64, 7475654348938858373_ui64, 7172379734800165924_ui64, 9950368772730861135_ui64, 2893431947888064172_ui64, 9998727094827957402_ui64, 2720120215150867053_ui64, 5214531366106088224_ui64, 2826453138815994870_ui64, 9122236476088260706_ui64, 1813022199_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{1155185699462890625_ui64, 8199082149403693620_ui64, 6024256090937723864_ui64, 4657405932695487554_ui64, 7375334000813270843_ui64, 826143019739979362_ui64, 742164712041263700_ui64, 9004725239147218194_ui64, 2985236357188356038_ui64, 8195990756045727232_ui64, 5272578214851454390_ui64, 2407276428525280714_ui64, 2424522903850476990_ui64, 8901454992790762657_ui64, 2616204046619045836_ui64, 685453389872620117_ui64, 5352823086803313647_ui64, 6188106285318615159_ui64, 4654224357233495314_ui64, 1872121538885188824_ui64, 5224635771947765697_ui64, 4832367292685424009_ui64, 6291969909648545758_ui64, 2027622111818240587_ui64, 2342967303688527925_ui64, 8740952496741533490_ui64, 6645233140903266541_ui64, 27_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{1207675933837890625_ui64, 8600711077197047416_ui64, 995462771592440023_ui64, 4365166412723634200_ui64, 8939032511251921580_ui64, 6691186941151947033_ui64, 6686965954988754573_ui64, 9172698070406566040_ui64, 430303787303546739_ui64, 7016484420551091876_ui64, 9511473208556987854_ui64, 9956163715707065045_ui64, 543974340931341987_ui64, 3317055853645674192_ui64, 8189367201255111584_ui64, 2418841134890808816_ui64, 4118903666465048063_ui64, 71893239636582745_ui64, 5755510653505039754_ui64, 2119876780002202716_ui64, 5428115415529198286_ui64, 6110841296762630797_ui64, 9243432385747791941_ui64, 6953214665099638513_ui64, 3176258118097261227_ui64, 1117781102645375117_ui64, 1772848796706428478_ui64, 4221271257643_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{2900791168212890625_ui64, 4569002703974547330_ui64, 3006595437632920489_ui64, 4597503677298931451_ui64, 227291936724341014_ui64, 7921605513596077137_ui64, 2922755991991988237_ui64, 3345648295394377975_ui64, 2596853625071686865_ui64, 5066509405979977223_ui64, 9911476852668524483_ui64, 6903235771354266487_ui64, 6838071898745173884_ui64, 3102141121581639723_ui64, 3077329152176755579_ui64, 9969607058553576679_ui64, 3480234259320696027_ui64, 6343802705671747840_ui64, 8049278870725910028_ui64, 5453295375908060508_ui64, 4645734590452221374_ui64, 7149474722719046257_ui64, 6066885621808491466_ui64, 369646289725225612_ui64, 993229323626932756_ui64, 3144396563358430757_ui64, 6503368852505204813_ui64, 4876959713330822703_ui64, 64411_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{1234531402587890625_ui64, 9397480818188341800_ui64, 1156080544570964110_ui64, 9724647720437552454_ui64, 838167489585932575_ui64, 2995168613087340343_ui64, 9636877104516685693_ui64, 7355037733806701537_ui64, 1573313191919682241_ui64, 206485482740076066_ui64, 7204053269447470184_ui64, 7992322996696118967_ui64, 1614667463733455983_ui64, 3214069165619993572_ui64, 9470761003145463891_ui64, 4801129044759050783_ui64, 604252318872178017_ui64, 9619726615604160451_ui64, 6448673802853434839_ui64, 4877396973408185893_ui64, 702724387409902586_ui64, 6456032938319924781_ui64, 4899210619346232951_ui64, 6529787079878015469_ui64, 9129143682398167262_ui64, 1362115035907429864_ui64, 95949297247262014_ui64, 254929145864392891_ui64, 9828413039546407_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{1208896636962890625_ui64, 6258209247353079263_ui64, 8684709310874646273_ui64, 2780831011292732918_ui64, 6369106441110957648_ui64, 7204031842623405297_ui64, 558730056688974198_ui64, 8844839166822465584_ui64, 7510107686691859951_ui64, 5993052086995135374_ui64, 5124234622291343425_ui64, 9539637408509113608_ui64, 1906735809209030100_ui64, 4808589502176932735_ui64, 3475325213837725305_ui64, 8204770499736953783_ui64, 1970522686784805980_ui64, 4209053201358830549_ui64, 6102728265681863641_ui64, 3114302619150906415_ui64, 5622342908895827841_ui64, 3758053752177801321_ui64, 3641502631470229161_ui64, 8964762800586099538_ui64, 9681283974148362424_ui64, 7710855852190302770_ui64, 3910829793733444761_ui64, 6280653535399616962_ui64, 3895630954817644437_ui64, 149969681_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{7823886871337890625_ui64, 3475229358045908156_ui64, 7659648685580585439_ui64, 7743670481639241193_ui64, 5221809124463363819_ui64, 1237380620866492326_ui64, 8957244677795896036_ui64, 2822507332744294077_ui64, 8909294415967394513_ui64, 341701469575617777_ui64, 3029697509659608261_ui64, 1747071611392740208_ui64, 6358352806696127449_ui64, 8697011595256588885_ui64, 5375565411508018526_ui64, 7088895164113238736_ui64, 5201107913280799861_ui64, 3747337218788493145_ui64, 7960150363297320577_ui64, 4149538514751102686_ui64, 8840918168858377367_ui64, 177203607518660108_ui64, 1429010219696865115_ui64, 4900442255271951774_ui64, 6915653212963299569_ui64, 9155085902119201265_ui64, 9190179713605588564_ui64, 3076318724225308131_ui64, 9079046268930870596_ui64, 2883557340936751629_ui64, 2_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{6079502105712890625_ui64, 2297997555906476918_ui64, 1295385642627401093_ui64, 8409605982355479211_ui64, 8242770351313331323_ui64, 8270923846768327704_ui64, 6883550433919992383_ui64, 7046825850788035244_ui64, 9556047558937184642_ui64, 6006130659626684813_ui64, 775186315028403771_ui64, 3237946823573265223_ui64, 3310462825730462820_ui64, 1369153691991467387_ui64, 8718647288467584156_ui64, 3802297445305868705_ui64, 512674210951159120_ui64, 5490287097184874165_ui64, 3365566616017718792_ui64, 3066205870973974049_ui64, 4337366854088417278_ui64, 2412736340956392957_ui64, 5112474825701959343_ui64, 1564906943191684260_ui64, 73786848456455311_ui64, 5044138441706134231_ui64, 3743372332078337129_ui64, 9730855090182185571_ui64, 2938018235417461001_ui64, 9772892747177830031_ui64, 349175374464_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{975742340087890625_ui64, 7174822785636933986_ui64, 4425215641754113096_ui64, 5003541737295313150_ui64, 3191425146630950658_ui64, 8389847537884069081_ui64, 2655087864241391704_ui64, 3467291975151831572_ui64, 405371506451810467_ui64, 575685723206292098_ui64, 1550868469331827692_ui64, 4963565533068638840_ui64, 688296558196210076_ui64, 2408964131324200832_ui64, 644555074602564880_ui64, 8048813762186390510_ui64, 6873645077549704670_ui64, 5094324501230990767_ui64, 6063048179478257708_ui64, 3909614461754861694_ui64, 2143749741610807357_ui64, 816796483419294038_ui64, 7276511114696294265_ui64, 6035806352424237452_ui64, 1837053093946944323_ui64, 413548432522382331_ui64, 5820484848695960109_ui64, 8443843288320500672_ui64, 135040584272080843_ui64, 6226506239898959891_ui64, 9933847805372508959_ui64, 5327_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{7512607574462890625_ui64, 8526304031001927796_ui64, 6015792345807232446_ui64, 8039726324847650783_ui64, 1998086319023769184_ui64, 1079876910269548746_ui64, 8630326055753288847_ui64, 9407893717271238162_ui64, 8960337424732848741_ui64, 6975722967611259558_ui64, 2166090707026454516_ui64, 9897283959211249837_ui64, 9607250441948936966_ui64, 45292979252571993_ui64, 5244885290664125539_ui64, 5491716013253098672_ui64, 8223979309826870150_ui64, 2092097120264359703_ui64, 4333756791600410480_ui64, 6169081818588991996_ui64, 281884367517310512_ui64, 4867864717769618860_ui64, 874979413098090872_ui64, 7389748035835435710_ui64, 1494190002987767828_ui64, 9865468015091752532_ui64, 7288834997765326423_ui64, 2888896573188550547_ui64, 2844367041684620488_ui64, 9582502901230477665_ui64, 1576684512733554685_ui64, 812987271847616_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{690097808837890625_ui64, 8018767814828606788_ui64, 4746161934300293441_ui64, 786557556393315823_ui64, 6507749146113988111_ui64, 2672843353242283920_ui64, 3370490586545973362_ui64, 2608362253270356075_ui64, 8230093679930018879_ui64, 2167291091313742442_ui64, 7581560775386397821_ui64, 2693151430756999313_ui64, 3087305754047911975_ui64, 4271849664417273589_ui64, 2528921356892287891_ui64, 3824870685499348307_ui64, 125450218074279987_ui64, 3775589572509449918_ui64, 2074353059266618656_ui64, 1533202580776318231_ui64, 9628679293351468179_ui64, 2327096561918401603_ui64, 4938182447324595913_ui64, 8366044630050345048_ui64, 8865940664516419003_ui64, 1256184715182342574_ui64, 2687747800681579820_ui64, 6631858479138717617_ui64, 3927686811659188433_ui64, 6701303906926987330_ui64, 9119062188453718585_ui64, 2916201195933296397_ui64, 12405201_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{5508213043212890625_ui64, 6337705699006619397_ui64, 9551611603069577252_ui64, 5210544353426543186_ui64, 2178361675820275213_ui64, 2291858009456621492_ui64, 2469079935803884174_ui64, 2395641378333901772_ui64, 6665478402837558796_ui64, 3918219424897669676_ui64, 1449862856556565724_ui64, 2911272240929577261_ui64, 4595943492021963970_ui64, 545896806141750397_ui64, 4707014683103174811_ui64, 3743333015883199415_ui64, 4400632840345316856_ui64, 4962450868613437165_ui64, 5008950252923424873_ui64, 1464573853673600147_ui64, 7116116203814736645_ui64, 3818604120310658162_ui64, 8660265080674691191_ui64, 3990638503085059315_ui64, 4971161474091257172_ui64, 5040065748076487190_ui64, 7985202045535045092_ui64, 5297487986568417368_ui64, 6707219492063270250_ui64, 5060322630732525812_ui64, 4349149424763548384_ui64, 5564025560288424506_ui64, 1892883497866839537_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{6966953277587890625_ui64, 3461211784488114062_ui64, 1661568089869776901_ui64, 7360715782953057908_ui64, 1755861438149175314_ui64, 4997175936559473813_ui64, 8825099827451446677_ui64, 3937648353104857418_ui64, 5477111242609273961_ui64, 1036022440574316449_ui64, 7503314879123221205_ui64, 8560983451078229622_ui64, 9327998396473270277_ui64, 8094237893614860412_ui64, 5616273659452771089_ui64, 537020182907328538_ui64, 3394295711415603880_ui64, 1923395605478406377_ui64, 9145641097449506662_ui64, 6582053756850437208_ui64, 2473242972787076624_ui64, 137048753602847348_ui64, 3496580716226259160_ui64, 2656755961002579966_ui64, 7850520690775123643_ui64, 303831557787498078_ui64, 2627730179444362611_ui64, 1753138549578888634_ui64, 2818336567649657998_ui64, 5426912333789359925_ui64, 7601987381901895610_ui64, 7994462041460409923_ui64, 8372732171081932987_ui64, 28883110013_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{66318511962890625_ui64, 1433420211287739221_ui64, 3039688315753353645_ui64, 952439600548829496_ui64, 6961565358897640731_ui64, 1618929012388022432_ui64, 5867718124346556649_ui64, 3275362988664242972_ui64, 8221734831812261247_ui64, 7399360587724475747_ui64, 589173142029151009_ui64, 5559939994862592760_ui64, 1238750893569693772_ui64, 3634706563232397407_ui64, 8647932405002409265_ui64, 2880941552982691336_ui64, 2390728767959902693_ui64, 7344723104771357901_ui64, 3130289030813531577_ui64, 8164857317285783347_ui64, 684299416840419873_ui64, 5043845862934005322_ui64, 9016595461075573022_ui64, 4778437248177806338_ui64, 3294012949474425674_ui64, 8102123838051158001_ui64, 3068025364743523525_ui64, 3614598733943636771_ui64, 8963927944231240075_ui64, 9714344067662606317_ui64, 2276551901655766811_ui64, 8075244714385793791_ui64, 3468778655921356833_ui64, 7212831701244082200_ui64, 440_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{9806308746337890625_ui64, 9637942658482643309_ui64, 8263190482077334766_ui64, 5463485153803837825_ui64, 2262074446906121834_ui64, 1902837020287557356_ui64, 6122535577681411932_ui64, 1472621549495210637_ui64, 4322089741140182322_ui64, 4638023044154889401_ui64, 5131684835805673030_ui64, 7656365975786943695_ui64, 9633449181517639081_ui64, 4474702014751447960_ui64, 6900827844767861377_ui64, 3540548227660101394_ui64, 9506636549086632583_ui64, 1762375066997204100_ui64, 8412411153331346203_ui64, 8734031267246841900_ui64, 1597270784986226154_ui64, 2732745676945486707_ui64, 4470981217099698836_ui64, 3381094192758109452_ui64, 1737316051917072268_ui64, 755310261444289390_ui64, 210498933443051728_ui64, 8255122574890523754_ui64, 4737560327371196501_ui64, 119372237608763797_ui64, 1985902705602173884_ui64, 8808867069649534632_ui64, 2062505158618846349_ui64, 5964691215038736815_ui64, 67248730952472_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{1186923980712890625_ui64, 2571305844212474767_ui64, 2757380212981302772_ui64, 9931308872424171174_ui64, 118688132682573757_ui64, 8838707920258704329_ui64, 4922475004620926212_ui64, 6393224733315057032_ui64, 2532702964364952842_ui64, 7735809544709621294_ui64, 7887120126922959047_ui64, 8349562913996655819_ui64, 909283716680389034_ui64, 6361078027854144005_ui64, 9392910377116424785_ui64, 4737151289380307689_ui64, 3658536278389712946_ui64, 3196426536661324296_ui64, 7053558834264756271_ui64, 1991256842230394795_ui64, 7986168651337147022_ui64, 9699509759479839858_ui64, 1238225749706202345_ui64, 7397072211776446794_ui64, 979044851396090587_ui64, 1201383998677307921_ui64, 2887531749806369956_ui64, 3363609927528365073_ui64, 2255325595933877180_ui64, 5907561870385136831_ui64, 4316083375844913293_ui64, 3847447540068095483_ui64, 6830802899305222620_ui64, 2229128296852190530_ui64, 2003245940623340073_ui64, 1026134_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{9208164215087890625_ui64, 116389025679382029_ui64, 9930232583180325992_ui64, 8551509746664126784_ui64, 8254052428151075323_ui64, 2756397859477052805_ui64, 9394258557719542240_ui64, 8122716139564132760_ui64, 2132696935931773884_ui64, 3982741432688949041_ui64, 3156592983029320445_ui64, 7477999845689699202_ui64, 7920690129845625518_ui64, 1298828901814021555_ui64, 3868528081681488462_ui64, 8040113821302953752_ui64, 4524070201677723531_ui64, 6925704401527958349_ui64, 9793132220404457651_ui64, 8534362137658063266_ui64, 3036349822864636874_ui64, 5629861266878794118_ui64, 5228629450981289054_ui64, 5771300372897731776_ui64, 1796049154618411434_ui64, 8798205733933179495_ui64, 5669243792745727518_ui64, 6862346388054378506_ui64, 6121395925348051872_ui64, 8747685428700646910_ui64, 1804396605034818578_ui64, 9335858677918752683_ui64, 118197690543327942_ui64, 9174580487530561818_ui64, 2809459415117314099_ui64, 156575653125700998_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{8870029449462890625_ui64, 315861499148013535_ui64, 3130797120916580592_ui64, 2607236479648324328_ui64, 9731952492546230265_ui64, 868653232636298036_ui64, 8279760784246793510_ui64, 1734124899204384730_ui64, 6230145997966249688_ui64, 8797155946849013304_ui64, 2120429290508139536_ui64, 7866699572091567734_ui64, 7401354789398191004_ui64, 586785246592561795_ui64, 7356027546674024991_ui64, 7525902158638428980_ui64, 3422169769899197712_ui64, 1429191505799411036_ui64, 3238777418910284447_ui64, 5109751431169620062_ui64, 507335640477937891_ui64, 4425372055608365217_ui64, 4986733179166584272_ui64, 3740810626657553878_ui64, 2037877590504461256_ui64, 5643843868958269571_ui64, 3945239086886684172_ui64, 1305314112384701427_ui64, 9329204206273596009_ui64, 6480546663229307942_ui64, 3174221630061117991_ui64, 6723737057562204056_ui64, 6752920979856242033_ui64, 1572543031588709877_ui64, 9297195421975410271_ui64, 3682403302146292344_ui64, 2389154863_ui64, 0_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{5172519683837890625_ui64, 4645620099945517722_ui64, 5484099125913413988_ui64, 88551073162797865_ui64, 340311471668037104_ui64, 2560125270568122750_ui64, 7813951124094440195_ui64, 9887806827999279544_ui64, 5455579860445021044_ui64, 8865555514986358611_ui64, 8102346325640196942_ui64, 6042439912726277015_ui64, 1511078611247246888_ui64, 7429473226949383383_ui64, 6383207864178241677_ui64, 6772000234314385172_ui64, 9560197868742403543_ui64, 9099584402804104154_ui64, 5402289908282341091_ui64, 247201892681540205_ui64, 9411909715419791125_ui64, 6100649574391718146_ui64, 8729289618307068719_ui64, 9260540542400103441_ui64, 7033272762158365274_ui64, 1181270020109216163_ui64, 9340236292172910598_ui64, 1825779326556992902_ui64, 5165406666646028326_ui64, 9042349690692202854_ui64, 7275084174813028918_ui64, 560921679294153290_ui64, 3572087657527726098_ui64, 7163982799574599912_ui64, 7634623894844711785_ui64, 5037284077410818816_ui64, 4556100977819874605_ui64, 36_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{3115634918212890625_ui64, 5788226702461543027_ui64, 1534116892175599669_ui64, 6491373527284227385_ui64, 7315231934944932146_ui64, 1747245793766959711_ui64, 2418242684328083352_ui64, 1882254757903306947_ui64, 1537635038571271179_ui64, 604036185540464179_ui64, 2657004224633110275_ui64, 1211702851903477869_ui64, 8771803070715941051_ui64, 5118314079453714160_ui64, 5869396628257342940_ui64, 6037776586657834841_ui64, 9023978222229007481_ui64, 8028588097537198924_ui64, 9527144151299810833_ui64, 2783844839512480289_ui64, 8849964145617704661_ui64, 8736289511290126272_ui64, 2186999452219384374_ui64, 2125158977192529576_ui64, 8012279499214790991_ui64, 7016626541645397371_ui64, 1735625933552093812_ui64, 6064590901798039126_ui64, 3830005826199025368_ui64, 6344429705282694498_ui64, 4038836502721400309_ui64, 9327589527512881085_ui64, 9753127755141016834_ui64, 870347954896357078_ui64, 9576383318542218011_ui64, 1016054803995115582_ui64, 34577255817933310_ui64, 5562684646268_ui64, 0_ui64, 0_ui64, 0_ui64},
			bignum_t{7699375152587890625_ui64, 7906345720148237887_ui64, 9255320675479533423_ui64, 7266141533418531679_ui64, 5884421790945030628_ui64, 4304155232471190413_ui64, 959984157520230007_ui64, 7335654791538126897_ui64, 6680782957187725187_ui64, 2790554680444870583_ui64, 8479961404480532616_ui64, 6248454172622745126_ui64, 8443153552424665098_ui64, 80896129940842753_ui64, 7275228959935212394_ui64, 3131537107382150699_ui64, 5861737305263924967_ui64, 182941421874667697_ui64, 7143987091500672875_ui64, 8501020805773073597_ui64, 6685891918698804313_ui64, 7072490369414736108_ui64, 2186336498051292591_ui64, 2590572543136588321_ui64, 3116301149370711932_ui64, 3059901703068229663_ui64, 4760408432260536829_ui64, 8929310024042143761_ui64, 1169642426522688320_ui64, 7676772824554532260_ui64, 9510545293341517719_ui64, 9030927338239082377_ui64, 3109258260003502311_ui64, 7417767177629512865_ui64, 2794496320653502729_ui64, 1012509606573231789_ui64, 8859452835327984680_ui64, 8316386108926044552_ui64, 84879_ui64, 0_ui64, 0_ui64},
			bignum_t{3923740386962890625_ui64, 9416181605520250741_ui64, 9372165495397120973_ui64, 7036694324574595267_ui64, 8086948963411583456_ui64, 5335451292181728987_ui64, 9429627598635181159_ui64, 3976882491454836387_ui64, 724668538446880974_ui64, 2803738963024642291_ui64, 1096895794533610236_ui64, 7484302802733786079_ui64, 7455001621222750436_ui64, 7399101843920189703_ui64, 426695863050783232_ui64, 9356491047290480956_ui64, 8094019670586198537_ui64, 1793777097499653705_ui64, 4316539767195761442_ui64, 2151122698119920219_ui64, 984544960037548916_ui64, 4621598612760801982_ui64, 4096159623298498068_ui64, 8253833193761238068_ui64, 4739979897927437486_ui64, 1026254777775591268_ui64, 2098515213926722764_ui64, 5258752666632208744_ui64, 8603564589375458130_ui64, 6099282854761389564_ui64, 4504046595409741585_ui64, 9547481987160420125_ui64, 2292222885603749667_ui64, 1847055864093906555_ui64, 7841603048977160303_ui64, 4558073315345708017_ui64, 4372662368965295510_ui64, 4076622449637647023_ui64, 12951634466340773_ui64, 0_ui64, 0_ui64},
			bignum_t{6788730621337890625_ui64, 8260916350154730025_ui64, 1054836111697107017_ui64, 9746592397646568228_ui64, 9003823349558934395_ui64, 6440580415145095266_ui64, 5026422047965260195_ui64, 4588071890710870340_ui64, 3911281493787879813_ui64, 1055037089914986243_ui64, 3316991619930098923_ui64, 7923901231336910927_ui64, 8334722404240460246_ui64, 3329727744369531573_ui64, 1332890737901334288_ui64, 7751610724284299668_ui64, 2005392141059738880_ui64, 8907120637420954446_ui64, 5329809902859147605_ui64, 1677279576363216662_ui64, 7604509920518839998_ui64, 6596982131722182617_ui64, 2412424997277810863_ui64, 6634226747072748158_ui64, 9650495426236468410_ui64, 4605601268615415922_ui64, 7549249893589092678_ui64, 2554963661425993_ui64, 8745442279949228922_ui64, 3268671395928414754_ui64, 285276124561811138_ui64, 991840431275125052_ui64, 6626841423759185595_ui64, 8625878178287092370_ui64, 1993454465439695191_ui64, 2702029080835007461_ui64, 5729905770234273000_ui64, 4728854894602392104_ui64, 3364986176706275171_ui64, 197626258_ui64, 0_ui64},
		};

		[[nodiscard]] static inline exp_ut last_block(const bignum_t& p_val)
		{
			for(uint16_t i = static_cast<uint16_t>(p_val.size()); --i;)
			{
				if(p_val[i]) return i;
			}
			return 0;
		}

		[[nodiscard]] static inline exp_ut leading_zeros(const bignum_t& p_val)
		{
			if(p_val[0]) return                     leading_0(p_val[0]);
			if(p_val[1]) return max_pow_10_digits + leading_0(p_val[1]);

			const uint16_t t_size = static_cast<uint16_t>(p_val.size());
			for(uint16_t i = 2; i < t_size; ++i)
			{
				if(p_val[i]) return static_cast<exp_ut>(i * max_pow_10_digits + leading_0(p_val[0]));
			}
			return t_size * max_pow_10_digits;
		}

		static inline void exp_load(exp_st exponent, fp_to_chars_sci_size& p_out)
		{
			if(exponent < 0)
			{
				p_out.is_exp_negative = true;
				exponent = -exponent;
			}
			else
			{
				p_out.is_exp_negative = false;
			}

			if(exponent < 10)
			{
				p_out.exponent_size = 1;
			}
			else
			{
				p_out.exponent_size = (exponent < 100) ? 2 : 3;
			}
		}

		template<_p::charconv_char_c char_t>
		static inline void to_chars_exp(exp_st exponent, char_t* exp_char)
		{
			if(exponent < 0)
			{
				exponent = -exponent;
			}

			if(exponent < 10)
			{
				*exp_char = static_cast<char_t>('0' + exponent);
			}
			else
			{
				if(exponent < 100)
				{
					*(exp_char++) = static_cast<char_t>('0' + exponent / 10);
					*exp_char = static_cast<char_t>('0' + exponent % 10);
				}
				else
				{
					exp_char += 2;
					*(exp_char--) = static_cast<char_t>('0' + exponent / 10); exponent %= 10;
					*(exp_char--) = static_cast<char_t>('0' + exponent / 10); exponent %= 10;
					*(exp_char) = static_cast<char_t>('0' + exponent);
				}

			}
		}

	};


	template<_p::charconv_fp_c fp_type>
	struct fp_utils: public fp_utils_pre<fp_type>
	{
		using fp_utils_p = fp_utils_pre<fp_type>;
		using uint_t = typename fp_utils_p::uint_t;
		using bignum_t = typename fp_utils_p::bignum_t;
		using exp_st = typename fp_utils_p::exp_st;
		using exp_ut = typename fp_utils_p::exp_ut;


		[[nodiscard]] static inline uint_t get_mantissa(fp_type input)
		{
			return reinterpret_cast<const uint_t&>(input) & fp_utils_p::mantissa_mask;
		}

		[[nodiscard]] static inline uint_t get_exponent_bits(fp_type input)
		{
			return (reinterpret_cast<const uint_t&>(input) & fp_utils_p::exponent_mask);
		}

		[[nodiscard]] static inline bool get_sign(const fp_type input)
		{
			return reinterpret_cast<const uint_t&>(input) & fp_utils_p::sign_mask;
		}

		[[nodiscard]] static void mul_hack(bignum_t& p_1, const uint64_t p_2)
		{
			uint64_t mul_carry = 0;
			uint64_t carry = 0;
			uint64_t temp_res = core::umul(p_1[0], p_2, mul_carry);

			if(mul_carry || temp_res >= fp_utils_p::max_pow_10)
			{
				carry = core::udiv(mul_carry, temp_res, fp_utils_p::max_pow_10, p_1[0]);
			}
			else
			{
				p_1[0] = temp_res;
			}

			constexpr uint8_t last_index = fp_utils_p::bignum_width - 1;

			for(uint8_t i = 1; i < last_index; ++i)
			{
				if(!p_1[i])
				{
					p_1[i] = carry;
					return;
				}

				if(core::addcarry(0, carry, core::umul(p_1[i], p_2, mul_carry), temp_res))
				{
					++mul_carry;
				}

				if(mul_carry || temp_res >= fp_utils_p::max_pow_10)
				{
					carry = core::udiv(mul_carry, temp_res, fp_utils_p::max_pow_10, p_1[i]);
				}
				else
				{
					p_1[i] = temp_res;
					carry = 0;
				}
			}

			if(!p_1[last_index])
			{
				p_1[last_index] = carry;
				return;
			}

			core::addcarry(0, carry, core::umul(p_1[last_index], p_2, mul_carry), p_1[last_index]);
		}

		static inline void pow2_load(bignum_t& p_out, const exp_ut p_pow)
		{
			p_out = fp_utils_p::pow_2_hack_table[p_pow >> fp_utils_p::pow_2_hi_offset];
			const exp_ut low = p_pow & fp_utils_p::pow_2_low_mask;
			if(low)
			{
				mul_hack(p_out, fp_utils_p::pow_2_low_table(low));
			}
		}

		static inline void pow5_load(bignum_t& p_out, const exp_ut p_pow)
		{
			p_out = fp_utils_p::pow_5_hack_table[p_pow >> fp_utils_p::pow_5_hi_offset];
			const exp_ut low = p_pow & fp_utils_p::pow_5_low_mask;
			if(low)
			{
				mul_hack(p_out, fp_utils_p::pow_5_low_table[low]);
			}
		}

		[[nodiscard]] static inline exp_ut load_digits(bignum_t& digits, uint_t mantissa, exp_st exponent)
		{
#if USE_ORDER_REDUCE
			{
				exp_st offset = static_cast<exp_st>(std::countr_zero(mantissa));
				mantissa >>= offset;
				exponent += offset;
			}
#endif

			if(exponent < 0)
			{
				const exp_ut decimal_seperator_offset = static_cast<exp_ut>(-exponent);
				pow5_load(digits, decimal_seperator_offset);
				mul_hack(digits, mantissa);
				return decimal_seperator_offset;
			}
			pow2_load(digits, static_cast<exp_ut>(exponent));
			mul_hack(digits, mantissa);
			return 0;
		}

		static inline void round_nearest_at(bignum_t& p_out, const exp_ut pos)
		{
			const exp_ut block = pos / fp_utils_p::max_pow_10_digits;
			const exp_ut block_offset = pos % fp_utils_p::max_pow_10_digits;

			if(block_offset)
			{
				const uint64_t val = p_out[block];

				const uint64_t over_val = fp_utils_p::pow_10_table[block_offset];
				const uint64_t remain = val % over_val;
				p_out[block] -= remain;

				if(((block_offset == 1) ? remain : remain / fp_utils_p::pow_10_table[block_offset - 1]) < 5)
				{
					goto zero_loop;
				}
				p_out[block] += over_val;
			}
			else
			{
				if(p_out[block - 1] / 1000000000000000000_ui64 < 5)
				{
					goto zero_loop;
				}
				++p_out[block];
			}

			if(p_out[block] < fp_utils_p::max_pow_10)
			{
				goto zero_loop;
			}
			p_out[block] -= fp_utils_p::max_pow_10;

			{
				exp_ut test_block = block + 1;
				while(++p_out[test_block] == fp_utils_p::max_pow_10)
				{
					p_out[test_block++] = 0;
				}
			}

		zero_loop:
			for(exp_ut i = 0; i < block; ++i)
			{
				p_out[i] = 0;
			}
		}

		static inline void round_down_at(bignum_t& p_out, const exp_ut pos)
		{
			const exp_ut block = pos / fp_utils_p::max_pow_10_digits;
			const exp_ut block_offset = pos % fp_utils_p::max_pow_10_digits;
			const uint64_t val = p_out[block];

			for(exp_ut i = 0; i < block; ++i)
			{
				p_out[i] = 0;
			}

			if(block_offset)
			{
				const uint64_t over_val = fp_utils_p::pow_10_table[block_offset];
				const uint64_t remain = val % over_val;
				p_out[block] -= remain;
			}
		}

		static inline void round_up_at(bignum_t& p_out, const exp_ut pos)
		{
			exp_ut block = pos / fp_utils_p::max_pow_10_digits;
			const exp_ut block_offset = pos % fp_utils_p::max_pow_10_digits;
			const uint64_t val = p_out[block];

			for(exp_ut i = 0; i < block; ++i)
			{
				p_out[i] = 0;
			}

			if(block_offset)
			{
				const uint64_t over_val = fp_utils_p::pow_10_table[block_offset];
				const uint64_t remain = val % over_val;
				p_out[block] -= remain;
				p_out[block] += over_val;
			}
			else
			{
				++p_out[block];
			}

			if(p_out[block] < fp_utils_p::max_pow_10)
			{
				return;
			}

			p_out[block++] -= fp_utils_p::max_pow_10;

			while(++p_out[block] == fp_utils_p::max_pow_10)
			{
				p_out[block++] = 0;
			}
		}

		template<typename char_t>
		static inline void to_chars_sci_mantissa(const bignum_t& digits, char_t* const unit_char, char_t* decimal_chars,
			exp_ut last_block,
			exp_ut last_num_digits,
			exp_ut sig_digits)
		{
			if(last_num_digits == 1)
			{
				*unit_char = static_cast<char_t>('0' + digits[last_block--]);
			}
			else
			{
				uint64_t this_block = digits[last_block--];
				const uint64_t this_div = fp_utils_p::pow_10_table[--last_num_digits];
				*unit_char = static_cast<char_t>('0' + this_block / this_div);

				this_block %= this_div;

				if(last_num_digits > sig_digits)
				{
					this_block /= fp_utils_p::pow_10_table[last_num_digits - sig_digits];
					fp_utils_p::output_sig_digits(this_block, decimal_chars, sig_digits);
					return;
				}

				fp_utils_p::output_sig_digits(this_block, decimal_chars, last_num_digits);
				decimal_chars += last_num_digits;
				sig_digits -= last_num_digits;
			}

			while(sig_digits)
			{
				if(sig_digits < 19)
				{
					fp_utils_p::output_sig_digits(digits[last_block] / fp_utils_p::pow_10_table[19 - sig_digits], decimal_chars, sig_digits);
					break;
				}
				else
				{
					fp_utils_p::output_19_digits(digits[last_block--], decimal_chars);
					decimal_chars += 19;
					sig_digits -= 19;
				}
			}
		}

		template<_p::charconv_char_c char_t>
		static inline void fill_digits(const bignum_t& digits,
			exp_ut last_block,
			exp_ut last_num_digits,
			exp_ut sig_digits,
			char_t* out_chars
		)
		{
			if(last_num_digits > sig_digits)
			{
				fp_utils_p::output_sig_digits(
					digits[last_block] / fp_utils_p::pow_10_table[last_num_digits - sig_digits],
					out_chars,
					sig_digits);
				return;
			}
			fp_utils_p::output_sig_digits(
				digits[last_block--],
				out_chars,
				last_num_digits);
			sig_digits -= last_num_digits;

			while(sig_digits)
			{
				if(sig_digits < 19)
				{
					fp_utils_p::output_sig_digits(digits[last_block] / fp_utils_p::pow_10_table[19 - sig_digits], out_chars, sig_digits);
					out_chars += sig_digits;
					break;
				}
				else
				{
					fp_utils_p::output_19_digits(digits[last_block--], out_chars);
					out_chars += 19;
					sig_digits -= 19;
				}
			}
		}


		template<_p::charconv_char_c char_t>
		static inline void to_chars_fix(const bignum_t& digits,
			exp_st decimal_offset,
			char_t* unit_chars, char_t* decimal_chars,
			exp_ut last_block,
			exp_ut last_num_digits,
			exp_ut leading_zeros
		)
		{
			const exp_ut num_digits = static_cast<exp_ut>(last_block * fp_utils_p::max_pow_10_digits + last_num_digits);
			exp_ut sig_digits = num_digits - leading_zeros;
			if(decimal_offset < num_digits)
			{
				if(decimal_offset <= leading_zeros)
				{
					if(last_num_digits > sig_digits)
					{
						fp_utils_p::output_sig_digits(
							digits[last_block] / fp_utils_p::pow_10_table[last_num_digits - sig_digits],
							unit_chars,
							sig_digits);
						unit_chars += sig_digits;
					}
					else
					{
						fp_utils_p::output_sig_digits(digits[last_block], unit_chars, last_num_digits);
						unit_chars += last_num_digits;
						sig_digits -= last_num_digits;

						while(sig_digits)
						{
							if(sig_digits < 19)
							{
								fp_utils_p::output_sig_digits(digits[last_block] / fp_utils_p::pow_10_table[19 - sig_digits], unit_chars, sig_digits);
								unit_chars += sig_digits;
								break;
							}
							else
							{
								fp_utils_p::output_19_digits(digits[last_block--], unit_chars);
								unit_chars += 19;
								sig_digits -= 19;
							}
						}
					}

					exp_ut remain_digits = static_cast<exp_ut>(leading_zeros - static_cast<exp_ut>(decimal_offset));
					while(remain_digits--)
					{
						*(unit_chars++) = char_t{'0'};
					}
				}
				else
				{
					exp_ut unit_digits = static_cast<exp_ut>(num_digits - static_cast<exp_ut>(decimal_offset));
					exp_ut decimal_digits = sig_digits - unit_digits;


					if(unit_digits < last_num_digits)
					{
						const uint64_t this_div = fp_utils_p::pow_10_table[last_num_digits - unit_digits];
						const uint64_t this_val = digits[last_block--];
						const uint64_t div = this_val / this_div;
						const uint64_t rem = this_val % this_div;
						fp_utils_p::output_sig_digits(div, unit_chars, unit_digits);

						last_num_digits -= unit_digits;

						if(last_num_digits > decimal_digits)
						{
							fp_utils_p::output_sig_digits(rem / fp_utils_p::pow_10_table[last_num_digits - decimal_digits], decimal_chars, decimal_digits);
							return;
						}

						fp_utils_p::output_sig_digits(rem, decimal_chars, last_num_digits);
						decimal_chars += last_num_digits;
						decimal_digits -= last_num_digits;
					}
					else
					{
						fp_utils_p::output_sig_digits(digits[last_block--], unit_chars, last_num_digits);

						unit_digits -= last_num_digits;
						while(unit_digits)
						{
							if(unit_digits < 19)
							{
								const exp_ut rem_digits = 19 - sig_digits;
								const uint64_t this_div = fp_utils_p::pow_10_table[rem_digits];
								const uint64_t this_val = digits[last_block--];
								const uint64_t div = this_val / this_div;
								const uint64_t rem = this_val % this_div;
								fp_utils_p::output_sig_digits(div, unit_chars, unit_digits);


								if(rem_digits > decimal_digits)
								{
									fp_utils_p::output_sig_digits(rem / fp_utils_p::pow_10_table[rem_digits - decimal_digits], decimal_chars, decimal_digits);
									return;
								}

								fp_utils_p::output_sig_digits(rem, decimal_chars, rem_digits);
								decimal_chars += rem_digits;
								decimal_digits -= rem_digits;
								break;
							}
							else
							{
								fp_utils_p::output_19_digits(digits[last_block--], unit_chars);
								unit_chars += 19;
								unit_digits -= 19;
							}
						}
					}

					while(decimal_digits)
					{
						if(decimal_digits < 19)
						{
							fp_utils_p::output_sig_digits(digits[last_block] / fp_utils_p::pow_10_table[19 - decimal_digits], decimal_chars, decimal_digits);
							break;
						}
						else
						{
							fp_utils_p::output_19_digits(digits[last_block--], decimal_chars);
							decimal_chars += 19;
							decimal_digits -= 19;
						}
					}
				}
			}
			else
			{
				{
					exp_ut leading_z = static_cast<exp_ut>(decimal_offset - num_digits);
					while(leading_z-- < 0)
					{
						*(decimal_chars++) = char_t{'0'};
					}
				}

				if(last_num_digits > sig_digits)
				{
					fp_utils_p::output_sig_digits(
						digits[last_block] / fp_utils_p::pow_10_table[last_num_digits - sig_digits],
						decimal_chars,
						sig_digits);
					decimal_chars += sig_digits;
				}
				else
				{
					fp_utils_p::output_sig_digits(digits[last_block], decimal_chars, last_num_digits);
					decimal_chars += last_num_digits;
					sig_digits -= last_num_digits;

					while(sig_digits)
					{
						if(sig_digits < 19)
						{
							fp_utils_p::output_sig_digits(digits[last_block] / fp_utils_p::pow_10_table[19 - sig_digits], decimal_chars, sig_digits);
							break;
						}
						else
						{
							fp_utils_p::output_19_digits(digits[last_block--], decimal_chars);
							decimal_chars += 19;
							sig_digits -= 19;
						}
					}
				}
			}
		}

	};

	fp_to_chars_sci_result to_chars_sci_size(float32_t value, fp_to_chars_sci_context<float32_t>& context, uint16_t significant_digits, fp_round rounding_mode)
	{
		using fp_type = float32_t;
		using fp_utils_t = fp_utils<fp_type>;
		using uint_t = fp_utils_t::uint_t;
		using exp_st = fp_utils_t::exp_st;
		using exp_ut = fp_utils_t::exp_ut;
		using bignum_t = fp_utils_t::bignum_t;

		const uint_t exponent_bits = fp_utils_t::get_exponent_bits(value);
		const uint_t mantissa_bits = fp_utils_t::get_mantissa(value);
		const bool sign_bit        = fp_utils_t::get_sign(value);

		if(exponent_bits == fp_utils_t::exponent_mask)
		{ //nan or inf
			if(mantissa_bits)
			{ //nan
				return fp_to_chars_sci_result{fp_base_classify{.classification=fp_classify::nan}};
			} //else inf
			return fp_to_chars_sci_result{fp_base_classify{.classification=fp_classify::inf, .is_negative = sign_bit}};
		} // else number


		fp_to_chars_sci_result res;
		res.is_negative = sign_bit;

		exp_st exponent;
		uint_t mantissa = mantissa_bits;

		if(exponent_bits)
		{	//normal
			exponent = static_cast<exp_st>(exponent_bits >> fp_utils_t::exponent_offset) - fp_utils_t::exponent_fix_bias;
			mantissa |= fp_utils_t::mantissa_implicit_bit;
		}
		else
		{	//denormal
			if(mantissa == 0)
			{ //Zero
				res.classification = fp_classify::zero;
				return res;
			}
			exponent = 1 - fp_utils_t::exponent_fix_bias;
		}

		res.classification = fp_classify::finite;

		bignum_t& digits = context.digits;
		const exp_ut decimal_seperator_offset = fp_utils_t::load_digits(digits, mantissa, exponent);

		exp_ut last_block      = fp_utils_t::last_block(digits);
		exp_ut last_num_digits = fp_common_utils::num_digits(digits[last_block]);
		exp_ut num_digits      = static_cast<exp_ut>(last_block * fp_utils_t::max_pow_10_digits + last_num_digits);
		exp_ut leading_zeros   = fp_utils_t::leading_zeros(digits);
		exp_ut sig_digits      = static_cast<exp_ut>((num_digits - 1) - leading_zeros);

		significant_digits = std::min(significant_digits, fp_utils_t::max_scientific_decimal_digits_10);
		if(significant_digits < sig_digits)
		{
			const exp_ut round_pos = static_cast<exp_ut>((num_digits - 1) - significant_digits);

			fp_common_utils::fix_rounding_mode(rounding_mode, sign_bit);

			switch(rounding_mode)
			{
			default:
			case fp_round::nearest:
				fp_utils_t::round_nearest_at(digits, round_pos);
				break;
			case fp_round::to_zero:
				fp_utils_t::round_down_at(digits, round_pos);
				goto lbl$leading;
				break;
			case fp_round::away_zero:
				fp_utils_t::round_up_at(digits, round_pos);
				break;
			}

			last_block      = fp_utils_t::last_block(digits);
			last_num_digits = fp_common_utils::num_digits(digits[last_block]);
			num_digits      = static_cast<exp_ut>(last_block * fp_utils_t::max_pow_10_digits + last_num_digits);
		lbl$leading:
			leading_zeros   = fp_utils_t::leading_zeros(digits);

			sig_digits = static_cast<exp_ut>((num_digits - 1) - leading_zeros);
		}

		res.size.mantissa_decimal_size = sig_digits;
		context.exponent = static_cast<exp_st>(num_digits - 1) - static_cast<exp_st>(decimal_seperator_offset);

		fp_utils_t::exp_load(
			context.exponent,
			res.size);

		return res;
	}

	fp_to_chars_fix_result to_chars_fix_size(const float32_t value, fp_to_chars_fix_context<float32_t>& context, int16_t precision, fp_round rounding_mode)
	{
		using fp_type = float32_t;
		using fp_utils_t = fp_utils<fp_type>;
		using uint_t = fp_utils_t::uint_t;
		using exp_st = fp_utils_t::exp_st;
		using exp_ut = fp_utils_t::exp_ut;
		using bignum_t = fp_utils_t::bignum_t;

		const uint_t exponent_bits = fp_utils_t::get_exponent_bits(value);
		const uint_t mantissa_bits = fp_utils_t::get_mantissa(value);
		const bool sign_bit        = fp_utils_t::get_sign(value);

		if(exponent_bits == fp_utils_t::exponent_mask)
		{ //nan or inf
			if(mantissa_bits)
			{ //nan
				return fp_to_chars_fix_result{fp_base_classify{.classification=fp_classify::nan}};
			} //else inf
			return fp_to_chars_fix_result{fp_base_classify{.classification=fp_classify::inf, .is_negative = sign_bit}};
		} // else number


		fp_to_chars_fix_result res;
		res.is_negative = sign_bit;

		exp_st exponent;
		uint_t mantissa = mantissa_bits;

		if(exponent_bits)
		{	//normal
			exponent = static_cast<exp_st>(exponent_bits >> fp_utils_t::exponent_offset) - fp_utils_t::exponent_fix_bias;
			mantissa |= fp_utils_t::mantissa_implicit_bit;
		}
		else
		{	//denormal
			if(mantissa == 0)
			{ //Zero
				res.classification = fp_classify::zero;
				return res;
			}
			exponent = 1 - fp_utils_t::exponent_fix_bias;
		}

		res.classification = fp_classify::finite;

		bignum_t& digits = context.digits;
		const exp_ut decimal_seperator_offset = fp_utils_t::load_digits(digits, mantissa, exponent);

		exp_ut last_block      = fp_utils_t::last_block(digits);
		exp_ut last_num_digits = fp_common_utils::num_digits(digits[last_block]);
		exp_ut num_digits      = static_cast<exp_ut>(last_block * fp_utils_t::max_pow_10_digits + last_num_digits);
		exp_ut leading_zeros   = fp_utils_t::leading_zeros(digits);

		precision = std::clamp(precision, fp_utils_t::min_fixed_precision_10, fp_utils_t::max_fixed_precision_10);

		const int16_t digits_to_precision = static_cast<int16_t>(decimal_seperator_offset) - precision;

		if(digits_to_precision <= leading_zeros)
		{ //all digits make it exactly
			goto lbl$res_size;
		}

		fp_common_utils::fix_rounding_mode(rounding_mode, sign_bit);

		if(digits_to_precision >= num_digits)
		{ //no digits make it
			switch(rounding_mode)
			{
			default:
			case fp_round::nearest:
				if(digits_to_precision == num_digits)
				{
					if(digits[last_block] / fp_common_utils::pow_10_table[last_num_digits] < 5)
					{
						res.classification = fp_classify::zero;
					}
					else
					{
						++num_digits;
						if(num_digits < decimal_seperator_offset)
						{
							res.size.decimal_size = decimal_seperator_offset - num_digits;
							res.size.unit_size = 0;
						}
						else
						{
							res.size.decimal_size = 0;
							res.size.unit_size = static_cast<exp_ut>(1 + num_digits - static_cast<exp_ut>(decimal_seperator_offset));
						}

						while(last_block)
						{
							digits[last_block--] = 0;
						}
						digits[0] = 1;
						context.decimal_offset = -precision;
					}
				}
				else
				{
					res.classification = fp_classify::zero;
				}
				break;
			case fp_round::to_zero:
				res.classification = fp_classify::zero;
				break;
			case fp_round::away_zero:
				if(precision > 0)
				{
					res.size.unit_size = 0;
					res.size.decimal_size = static_cast<uint16_t>(precision);
				}
				else
				{
					res.size.unit_size = static_cast<uint16_t>(-precision);
					res.size.decimal_size = 0;
				}
				while(last_block)
				{
					digits[last_block--] = 0;
				}
				digits[0] = 1;
				context.decimal_offset = -precision;
				break;
			}
			return res;
		}

		switch(rounding_mode)
		{
		default:
		case fp_round::nearest:
			fp_utils_t::round_nearest_at(digits, digits_to_precision);
			break;
		case fp_round::to_zero:
			fp_utils_t::round_down_at(digits, digits_to_precision);
			goto lbl$leading;
			break;
		case fp_round::away_zero:
			fp_utils_t::round_up_at(digits, digits_to_precision);
			break;
		}

		last_block      = fp_utils_t::last_block(digits);
		last_num_digits = fp_common_utils::num_digits(digits[last_block]);
		num_digits      = static_cast<exp_ut>(last_block * fp_utils_t::max_pow_10_digits + last_num_digits);
	lbl$leading:
		leading_zeros   = fp_utils_t::leading_zeros(digits);

	lbl$res_size:
		if(leading_zeros < decimal_seperator_offset)
		{
			res.size.decimal_size = decimal_seperator_offset - leading_zeros;
		}
		else
		{
			res.size.decimal_size = 0;
		}

		if(num_digits > decimal_seperator_offset)
		{
			res.size.unit_size = num_digits - decimal_seperator_offset;
		}
		else
		{
			res.size.unit_size = 0;
		}
		return res;
	}


	void to_chars_sci_mantissa_unsafe(const fp_to_chars_sci_context<float32_t>& context, char8_t* const unit_char, char8_t* const decimal_chars)
	{
		using fp_type = float32_t;
		using fp_utils_t = fp_utils<fp_type>;
		using exp_ut = fp_utils_t::exp_ut;

		exp_ut last_block      = fp_utils_t::last_block(context.digits);
		exp_ut last_num_digits = fp_common_utils::num_digits(context.digits[last_block]);
		exp_ut num_digits      = static_cast<exp_ut>(last_block * fp_utils_t::max_pow_10_digits + last_num_digits);
		exp_ut leading_zeros   = fp_utils_t::leading_zeros(context.digits);
		exp_ut sig_digits      = static_cast<exp_ut>((num_digits - 1) - leading_zeros);

		fp_utils_t::to_chars_sci_mantissa(context.digits, unit_char, decimal_chars, last_block, last_num_digits, sig_digits);
	}

	void to_chars_sci_exp_unsafe(const fp_to_chars_sci_context<float32_t>& context, char8_t* exp_chars)
	{
		using fp_type = float32_t;
		using fp_utils_t = fp_utils<fp_type>;

		fp_utils_t::to_chars_exp(context.exponent, exp_chars);
	}

	void to_chars_fix_unsafe(const fp_to_chars_fix_context<float32_t>& context, char8_t* unit_chars, char8_t* decimal_chars)
	{
		using fp_type = float32_t;
		using fp_utils_t = fp_utils<fp_type>;
		using exp_ut = fp_utils_t::exp_ut;

		exp_ut last_block      = fp_utils_t::last_block(context.digits);
		exp_ut last_num_digits = fp_common_utils::num_digits(context.digits[last_block]);
		//exp_ut num_digits      = static_cast<exp_ut>(last_block * fp_utils_t::max_pow_10_digits + last_num_digits);
		exp_ut leading_zeros   = fp_utils_t::leading_zeros(context.digits);

		fp_utils_t::to_chars_fix(context.digits, context.decimal_offset,
			unit_chars, decimal_chars, last_block, last_num_digits, leading_zeros);
	}

	fp_to_chars_sci_result to_chars_sci_size(float64_t value, fp_to_chars_sci_context<float64_t>& context, uint16_t significant_digits, fp_round rounding_mode)
	{
		using fp_type = float64_t;
		using fp_utils_t = fp_utils<fp_type>;
		using uint_t = fp_utils_t::uint_t;
		using exp_st = fp_utils_t::exp_st;
		using exp_ut = fp_utils_t::exp_ut;
		using bignum_t = fp_utils_t::bignum_t;

		const uint_t exponent_bits = fp_utils_t::get_exponent_bits(value);
		const uint_t mantissa_bits = fp_utils_t::get_mantissa(value);
		const bool sign_bit        = fp_utils_t::get_sign(value);

		if(exponent_bits == fp_utils_t::exponent_mask)
		{ //nan or inf
			if(mantissa_bits)
			{ //nan
				return fp_to_chars_sci_result{fp_base_classify{.classification=fp_classify::nan}};
			} //else inf
			return fp_to_chars_sci_result{fp_base_classify{.classification=fp_classify::inf, .is_negative = sign_bit}};
		} // else number


		fp_to_chars_sci_result res;
		res.is_negative = sign_bit;

		exp_st exponent;
		uint_t mantissa = mantissa_bits;

		if(exponent_bits)
		{	//normal
			exponent = static_cast<exp_st>(exponent_bits >> fp_utils_t::exponent_offset) - fp_utils_t::exponent_fix_bias;
			mantissa |= fp_utils_t::mantissa_implicit_bit;
		}
		else
		{	//denormal
			if(mantissa == 0)
			{ //Zero
				res.classification = fp_classify::zero;
				return res;
			}
			exponent = 1 - fp_utils_t::exponent_fix_bias;
		}

		res.classification = fp_classify::finite;

		bignum_t& digits = context.digits;
		const exp_ut decimal_seperator_offset = fp_utils_t::load_digits(digits, mantissa, exponent);

		exp_ut last_block      = fp_utils_t::last_block(digits);
		exp_ut last_num_digits = fp_common_utils::num_digits(digits[last_block]);
		exp_ut num_digits      = static_cast<exp_ut>(last_block * fp_utils_t::max_pow_10_digits + last_num_digits);
		exp_ut leading_zeros   = fp_utils_t::leading_zeros(digits);
		exp_ut sig_digits      = static_cast<exp_ut>((num_digits - 1) - leading_zeros);

		significant_digits = std::min(significant_digits, fp_utils_t::max_scientific_decimal_digits_10);
		if(significant_digits < sig_digits)
		{
			const exp_ut round_pos = static_cast<exp_ut>((num_digits - 1) - significant_digits);

			fp_common_utils::fix_rounding_mode(rounding_mode, sign_bit);

			switch(rounding_mode)
			{
			default:
			case fp_round::nearest:
				fp_utils_t::round_nearest_at(digits, round_pos);
				break;
			case fp_round::to_zero:
				fp_utils_t::round_down_at(digits, round_pos);
				goto lbl$leading;
				break;
			case fp_round::away_zero:
				fp_utils_t::round_up_at(digits, round_pos);
				break;
			}

			last_block      = fp_utils_t::last_block(digits);
			last_num_digits = fp_common_utils::num_digits(digits[last_block]);
			num_digits      = static_cast<exp_ut>(last_block * fp_utils_t::max_pow_10_digits + last_num_digits);
		lbl$leading:
			leading_zeros   = fp_utils_t::leading_zeros(digits);

			sig_digits = static_cast<exp_ut>((num_digits - 1) - leading_zeros);
		}

		res.size.mantissa_decimal_size = sig_digits;
		context.exponent = static_cast<exp_st>(num_digits - 1) - static_cast<exp_st>(decimal_seperator_offset);

		fp_utils_t::exp_load(
			context.exponent,
			res.size);

		return res;
	}

	void to_chars_sci_mantissa_unsafe(const fp_to_chars_sci_context<float64_t>& context, char8_t* unit_char, char8_t* decimal_chars)
	{
		using fp_type = float64_t;
		using fp_utils_t = fp_utils<fp_type>;
		using exp_ut = fp_utils_t::exp_ut;

		exp_ut last_block      = fp_utils_t::last_block(context.digits);
		exp_ut last_num_digits = fp_common_utils::num_digits(context.digits[last_block]);
		exp_ut num_digits      = static_cast<exp_ut>(last_block * fp_utils_t::max_pow_10_digits + last_num_digits);
		exp_ut leading_zeros   = fp_utils_t::leading_zeros(context.digits);
		exp_ut sig_digits      = static_cast<exp_ut>((num_digits - 1) - leading_zeros);

		fp_utils_t::to_chars_sci_mantissa(context.digits, unit_char, decimal_chars, last_block, last_num_digits, sig_digits);
	}

	void to_chars_sci_exp_unsafe(const fp_to_chars_sci_context<float64_t>& context, char8_t* exp_chars)
	{
		using fp_type = float64_t;
		using fp_utils_t = fp_utils<fp_type>;
		fp_utils_t::to_chars_exp(context.exponent, exp_chars);
	}

	void to_chars_fix_unsafe(const fp_to_chars_fix_context<float64_t>& context, char8_t* unit_chars, char8_t* decimal_chars)
	{
		using fp_type = float64_t;
		using fp_utils_t = fp_utils<fp_type>;
		using exp_ut = fp_utils_t::exp_ut;

		exp_ut last_block      = fp_utils_t::last_block(context.digits);
		exp_ut last_num_digits = fp_common_utils::num_digits(context.digits[last_block]);
		//exp_ut num_digits      = static_cast<exp_ut>(last_block * fp_utils_t::max_pow_10_digits + last_num_digits);
		exp_ut leading_zeros   = fp_utils_t::leading_zeros(context.digits);

		fp_utils_t::to_chars_fix(context.digits, context.decimal_offset,
			unit_chars, decimal_chars, last_block, last_num_digits, leading_zeros);
	}

} //namespace core

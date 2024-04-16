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

#include <CoreLib/toPrint/toPrint_encoders.hpp>
#include <array>

#include "../string/fp_traits.hpp"

namespace core::_p
{
	namespace
	{
		constexpr char16_t mul_char     = 0x00D7;
		constexpr char16_t sup_neg_char = 0x207B;
		constexpr char16_t inf_char     = 0x221E;


		constexpr std::array<char16_t, 10> sup_table =
		{
			0x2070,
			0x00B9,
			0x00B2,
			0x00B3,
			0x2074,
			0x2075,
			0x2076,
			0x2077,
			0x2078,
			0x2079,
		};


		template<_p::charconv_fp_c T>
		struct fp_utils_pre_ex;

		template<>
		struct fp_utils_pre_ex<float32_t>: public fp_utils_pre<float32_t>
		{
			[[nodiscard]] static inline constexpr exp_ut exp_digits_size(exp_st exp)
			{
				if(exp == 0) return 0;
				if(exp < 0)
				{
					exp = -exp;
				}
				return (exp < 10) ? 1 : 2;
			}
		};

		template<>
		struct fp_utils_pre_ex<float64_t>: public fp_utils_pre<float64_t>
		{
			[[nodiscard]] static inline constexpr exp_ut exp_digits_size(exp_st exp)
			{
				if(exp == 0) return 0;
				if(exp < 0)
				{
					exp = -exp;
				}
				if(exp < 10) return 1;
				if(exp < 100) return 2;
				return 3;
			}
		};

		template<_p::charconv_fp_c fp_t>
		void fancy_sci_exp_unsafe(fp_to_chars_shortest_context<fp_t> context, char16_t* exp_chars)
		{
			using fp_props_p = fp_utils_pre_ex<fp_t>;
			using exp_st = typename fp_props_p::exp_st;
			using exp_ut = typename fp_props_p::exp_ut;

			exp_st const sci_exp = static_cast<exp_st>(context.sig_digits + context.exponent - 1);
			exp_ut exp = static_cast<exp_ut>((sci_exp < 0) ? -sci_exp: sci_exp);

			exp_ut digits_size = fp_props_p::exp_digits_size(exp);

			exp_chars += digits_size;
			while(exp)
			{
				*(--exp_chars) = sup_table[exp % 10];
				exp /= 10;
			}
		}

		template<charconv_fp_c Fp_t>
		static inline char16_t* fancy_fp2dec(Fp_t const p_val, char16_t* pivot)
		{
			fp_to_chars_shortest_context<Fp_t> context;
			fp_base_classify const classification = to_chars_shortest_classify(p_val, context);

			if(classification.classification == fp_classify::nan)
			{
				*(pivot++) = u'n';
				*(pivot++) = u'a';
				*(pivot++) = u'n';
				return pivot;
			}

			if(classification.is_negative)
			{
				*(pivot++) = u'-';
			}

			switch(classification.classification)
			{
			default:
			case fp_classify::zero:
				*(pivot++) = u'0';
				break;
			case fp_classify::finite:
				{
					fp_to_chars_sci_size const sci_size_data = to_chars_shortest_sci_size(context);
					fp_to_chars_fix_size const fix_size_data = to_chars_shortest_fix_size(context);

					uint8_t sci_size = 1;
					if(sci_size_data.mantissa_decimal_size)
					{
						sci_size += static_cast<uint8_t>(sci_size_data.mantissa_decimal_size + 1);
					}
					if(sci_size_data.exponent_size)
					{
						sci_size += static_cast<uint8_t>(sci_size_data.exponent_size + 3);
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
							char16_t* const unit_digit = pivot++;
							char16_t* decimal_digit = pivot;
							if(sci_size_data.mantissa_decimal_size)
							{
								*(pivot++) = u'.';
								decimal_digit = pivot;
								pivot += sci_size_data.mantissa_decimal_size;
							}
							to_chars_shortest_sci_unsafe(context, unit_digit, decimal_digit);
						}
						if(sci_size_data.exponent_size)
						{
							*(pivot++) = mul_char;
							*(pivot++) = u'1';
							*(pivot++) = u'0';
							if(sci_size_data.is_exp_negative)
							{
								*(pivot++) = sup_neg_char;
							}
							fancy_sci_exp_unsafe(context, pivot);
							pivot += sci_size_data.exponent_size;
						}
					}
					else
					{
						char16_t* const unit_digit = pivot;
						if(fix_size_data.unit_size)
						{
							pivot += fix_size_data.unit_size;
						}
						else
						{
							*(pivot++) = u'0';
						}

						char16_t* decimal_digit = pivot;
						if(fix_size_data.decimal_size)
						{
							*(pivot++) = u'.';
							decimal_digit = pivot;
							pivot += fix_size_data.decimal_size;
						}
						to_chars_shortest_fix_unsafe(context, unit_digit, decimal_digit);
					}
				}
				break;
			case fp_classify::inf:
				*(pivot++) = inf_char;
				break;
			}

			return pivot;
		}

	} //namespace

	template<core::_p::charconv_fp_c fp_t>
	uintptr_t to_chars_fp_fancy(fp_t const p_val, std::span<char16_t, fp_fancy_props<fp_t>::max_size> p_buff)
	{
		return fancy_fp2dec(p_val, p_buff.data()) - p_buff.data();

	}

	template uintptr_t to_chars_fp_fancy<float32_t>(float32_t, std::span<char16_t, fp_fancy_props<float32_t>::max_size>);
	template uintptr_t to_chars_fp_fancy<float64_t>(float64_t, std::span<char16_t, fp_fancy_props<float64_t>::max_size>);

} //namespace core::_p

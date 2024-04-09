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

#include <CoreLib/string/core_fp_charconv.hpp>

#include <CoreLib/core_type.hpp>
#include <algorithm>

#include "ryu/common.hpp"
#include "ryu/f2s_intrinsics.hpp"
#include "ryu/d2s_intrinsics.hpp"

#include "fp_traits.hpp"

namespace core
{
	using ::core::literals::operator "" _ui64;
	using ::core::literals::operator "" _ui32;
	using ::core::literals::operator "" _ui16;
	using ::core::literals::operator "" _i16;
	using ::core::literals::operator "" _ui8;

	namespace
	{
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


		template<typename fp_type>
		struct fp_utils: public fp_utils_pre_ex<fp_type>
		{
			using fp_utils_p = fp_utils_pre_ex<fp_type>;
			using uint_t = typename fp_utils_p::uint_t;
			using exp_st = typename fp_utils_p::exp_st;
			using exp_ut = typename fp_utils_p::exp_ut;

			[[nodiscard]] static inline constexpr fp_to_chars_sci_size sci_size(const uint8_t sig_digits, const exp_st ryu_exp)
			{
				const exp_st sci_exp = static_cast<exp_st>(sig_digits + ryu_exp - 1);
				return fp_to_chars_sci_size
				{
					.mantissa_decimal_size = static_cast<uint16_t>(sig_digits -1),
					.exponent_size = fp_utils_p::exp_digits_size(sci_exp),
					.is_exp_negative = sci_exp < 0
				};
			}

			[[nodiscard]] static inline constexpr fp_to_chars_fix_size fix_size(const uint8_t sig_digits, const exp_st ryu_exp)
			{
				if(ryu_exp >= 0)
				{
					return fp_to_chars_fix_size
					{
						.unit_size = static_cast<uint16_t>(sig_digits + ryu_exp),
						.decimal_size = 0
					};
				}

				exp_st unit_digits = static_cast<exp_st>(sig_digits) + ryu_exp;

				return fp_to_chars_fix_size
				{
					.unit_size = (unit_digits > 0) ? static_cast<uint16_t>(unit_digits) : 0_ui16,
					.decimal_size = static_cast<uint16_t>(-ryu_exp)
				};
			}
		};

	} //namespace


	template<>
	[[nodiscard]] fp_to_chars_sci_size to_chars_shortest_sci_size<float32_t>(fp_to_chars_shortest_context<float32_t> context)
	{
		using fp_type = float32_t;
		using fp_utils_t = fp_utils<fp_type>;
		return fp_utils_t::sci_size(context.sig_digits, context.exponent);
	}

	template<>
	[[nodiscard]] fp_to_chars_sci_size to_chars_shortest_sci_size<float64_t>(fp_to_chars_shortest_context<float64_t> context)
	{
		using fp_type = float64_t;
		using fp_utils_t = fp_utils<fp_type>;
		return fp_utils_t::sci_size(context.sig_digits, context.exponent);
	}


	template<>
	[[nodiscard]] fp_to_chars_fix_size to_chars_shortest_fix_size<float32_t>(fp_to_chars_shortest_context<float32_t> context)
	{
		using fp_type = float32_t;
		using fp_utils_t = fp_utils<fp_type>;
		return fp_utils_t::fix_size(context.sig_digits, context.exponent);
	}

	template<>
	[[nodiscard]] fp_to_chars_fix_size to_chars_shortest_fix_size<float64_t>(fp_to_chars_shortest_context<float64_t> context)
	{
		using fp_type = float64_t;
		using fp_utils_t = fp_utils<fp_type>;
		return fp_utils_t::fix_size(context.sig_digits, context.exponent);
	}


	template<_p::charconv_fp_c fp_t, _p::charconv_char_c char_t>
	void to_chars_shortest_sci_unsafe(fp_to_chars_shortest_context<fp_t> context, char_t* unit_char, char_t* decimal_chars)
	{
		using fp_utils_t = fp_utils<fp_t>;
		using uint_t = fp_utils_t::uint_t;

		uint_t mantissa = context.mantissa;
		uint8_t dec_digits = context.sig_digits;
		char_t* pivot = decimal_chars + dec_digits - 1;

		while(--dec_digits)
		{
			*(--pivot) = static_cast<char_t>('0' + mantissa % 10);
			mantissa /= 10;
		}
		*unit_char = static_cast<char_t>('0' + mantissa);
	}

	template<_p::charconv_fp_c fp_t, _p::charconv_char_c char_t>
	void to_chars_shortest_sci_exp_unsafe(fp_to_chars_shortest_context<fp_t> context, char_t* exp_chars)
	{
		using fp_props_p = fp_utils<fp_t>;
		using exp_st = typename fp_props_p::exp_st;
		using exp_ut = typename fp_props_p::exp_ut;

		const exp_st sci_exp = static_cast<exp_st>(context.sig_digits + context.exponent - 1);
		exp_ut exp = static_cast<exp_ut>((sci_exp < 0) ? -sci_exp: sci_exp);

		exp_ut digits_size = fp_props_p::exp_digits_size(exp);

		exp_chars += digits_size;
		while(exp)
		{
			*(--exp_chars) = static_cast<char_t>('0' + exp % 10);
			exp /= 10;
		}
	}

	template<_p::charconv_fp_c fp_t, _p::charconv_char_c char_t>
	void to_chars_shortest_fix_unsafe(fp_to_chars_shortest_context<fp_t> context, char_t* unit_chars, char_t* decimal_chars)
	{
		using fp_props_p = fp_utils<fp_t>;
		using exp_st = typename fp_props_p::exp_st;
		using exp_ut = typename fp_props_p::exp_ut;
		using uint_t = typename fp_props_p::uint_t;

		exp_st ryu_exp = context.exponent;
		uint_t mantissa = context.mantissa;

		if(ryu_exp >= 0)
		{
			unit_chars += ryu_exp + context.sig_digits;
			while(ryu_exp--)
			{
				*(--unit_chars) = char_t{'0'};
			}
			do
			{
				*(--unit_chars) = static_cast<char_t>('0' + mantissa % 10);
				mantissa /= 10;
			}
			while(mantissa);

		}
		else
		{
			exp_st unit_digits = static_cast<exp_st>(context.sig_digits) + ryu_exp;
			if(unit_digits > 0)
			{
				exp_ut decimal_digits = static_cast<exp_ut>(-ryu_exp);
				decimal_chars += decimal_digits;
				unit_chars += unit_digits;

				do
				{
					*(--decimal_chars) = static_cast<char_t>('0' + mantissa % 10);
					mantissa /= 10;
				}
				while(--decimal_digits);

				do
				{
					*(--unit_chars) = static_cast<char_t>('0' + mantissa % 10);
					mantissa /= 10;
				}
				while(mantissa);
			}
			else
			{
				decimal_chars += static_cast<exp_ut>(-ryu_exp);
				do
				{
					*(--decimal_chars) = static_cast<char_t>('0' + mantissa % 10);
					mantissa /= 10;
				}
				while(mantissa);

				while(unit_digits++)
				{
					*(--decimal_chars) = char_t{'0'};
				}
			}
		}
	}


	template void to_chars_shortest_sci_unsafe<float32_t , char8_t >(fp_to_chars_shortest_context<float32_t >, char8_t *, char8_t *);
	template void to_chars_shortest_sci_unsafe<float32_t , char16_t>(fp_to_chars_shortest_context<float32_t >, char16_t*, char16_t*);
	template void to_chars_shortest_sci_unsafe<float32_t , char32_t>(fp_to_chars_shortest_context<float32_t >, char32_t*, char32_t*);
	template void to_chars_shortest_sci_unsafe<float64_t, char8_t >(fp_to_chars_shortest_context<float64_t>, char8_t *, char8_t *);
	template void to_chars_shortest_sci_unsafe<float64_t, char16_t>(fp_to_chars_shortest_context<float64_t>, char16_t*, char16_t*);
	template void to_chars_shortest_sci_unsafe<float64_t, char32_t>(fp_to_chars_shortest_context<float64_t>, char32_t*, char32_t*);

	template void to_chars_shortest_sci_exp_unsafe<float32_t , char8_t >(fp_to_chars_shortest_context<float32_t >, char8_t *);
	template void to_chars_shortest_sci_exp_unsafe<float32_t , char16_t>(fp_to_chars_shortest_context<float32_t >, char16_t*);
	template void to_chars_shortest_sci_exp_unsafe<float32_t , char32_t>(fp_to_chars_shortest_context<float32_t >, char32_t*);
	template void to_chars_shortest_sci_exp_unsafe<float64_t, char8_t >(fp_to_chars_shortest_context<float64_t>, char8_t *);
	template void to_chars_shortest_sci_exp_unsafe<float64_t, char16_t>(fp_to_chars_shortest_context<float64_t>, char16_t*);
	template void to_chars_shortest_sci_exp_unsafe<float64_t, char32_t>(fp_to_chars_shortest_context<float64_t>, char32_t*);

	template void to_chars_shortest_fix_unsafe<float32_t , char8_t >(fp_to_chars_shortest_context<float32_t >, char8_t *, char8_t *);
	template void to_chars_shortest_fix_unsafe<float32_t , char16_t>(fp_to_chars_shortest_context<float32_t >, char16_t*, char16_t*);
	template void to_chars_shortest_fix_unsafe<float32_t , char32_t>(fp_to_chars_shortest_context<float32_t >, char32_t*, char32_t*);
	template void to_chars_shortest_fix_unsafe<float64_t, char8_t >(fp_to_chars_shortest_context<float64_t>, char8_t *, char8_t *);
	template void to_chars_shortest_fix_unsafe<float64_t, char16_t>(fp_to_chars_shortest_context<float64_t>, char16_t*, char16_t*);
	template void to_chars_shortest_fix_unsafe<float64_t, char32_t>(fp_to_chars_shortest_context<float64_t>, char32_t*, char32_t*);

} //namespace core

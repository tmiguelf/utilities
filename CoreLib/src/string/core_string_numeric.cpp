//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///		Numeric string conversion utilities.
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
//======== ======== ======== ======== ======== ======== ======== ========

#include <charconv>
#include <array>

#include <CoreLib/string/core_string_numeric.hpp>
#include <CoreLib/Core_Type.hpp>

namespace core
{
	using namespace ::core::literals;

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
	} //namespace

	namespace _p
	{
		///	\brief	Converts a base 10 encoded string to unsigned integer.
		template <typename uint_T, typename char_T>
		[[nodiscard]] static inline from_chars_result<uint_T> dec2uint(std::basic_string_view<char_T> const p_str)
		{
			if(p_str.empty())
			{
				return std::errc::invalid_argument;
			}

			uint_T r_val = 0;
			for(const char_T tchar: p_str)
			{
				if(!is_digit(tchar))
				{
					return std::errc::invalid_argument;
				}
				const uint8_t t_val = static_cast<uint8_t>(tchar - '0');

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
		template <typename int_T, typename char_T>
		[[nodiscard]] static inline from_chars_result<int_T> dec2int(std::basic_string_view<char_T> const p_str)
		{
			const uintptr_t size = p_str.size();
			if(size == 0)
			{
				return std::errc::invalid_argument;
			}

			const char_T* pivot	= p_str.data();
			const char_T* end = pivot + size;

			int_T	r_val = 0;
			int8_t	t_val;

			if((*pivot == '-')) //negative handling
			{
				if(++pivot >= end)
				{
					return std::errc::invalid_argument;
				}

				do
				{
					if(!is_digit(*pivot))
					{
						return std::errc::invalid_argument;
					}

					t_val = static_cast<uint8_t>(*pivot - '0');

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
			else		//positive handling
			{
				if((*pivot == '+'))
				{
					if(++pivot >= end)
					{
						return std::errc::invalid_argument;
					}
				}

				do
				{
					if(!is_digit(*pivot))
					{
						return std::errc::invalid_argument;
					}

					t_val = static_cast<uint8_t>(*pivot - '0');

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
		template <typename uint_T, typename char_T>
		[[nodiscard]] static inline from_chars_result<uint_T> hex2uint(std::basic_string_view<char_T> const p_str)
		{
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

				if(is_digit(t_val))
				{
					t_val -= '0';
				}
				else if(t_val >= 'a' && t_val < 'g')
				{
					t_val -= 'a' - 10;
				}
				else if(t_val >= 'A' && t_val < 'G')
				{
					t_val -= 'A' - 10;
				}
				else
				{
					return std::errc::invalid_argument;
				}

				r_val = static_cast<uint_T>(r_val * 0x10 + static_cast<uint_T>(t_val));
			}
			return r_val;
		}


		template<typename fp_T>
		[[nodiscard]] static inline from_chars_result<fp_T> dec2fp(std::basic_string_view<char8_t> const p_str)
		{
			if(p_str.empty())
			{
				return std::errc::invalid_argument;
			}

			fp_T				ret;
			const char* const	pivot = reinterpret_cast<const char*>(p_str.data());
			const char* const	last  = pivot + p_str.size();

			const std::from_chars_result res = std::from_chars(pivot, last, ret);

			if(res.ec != std::errc{})
			{
				return res.ec;
			}

			if(res.ptr == last)
			{
				return ret;
			}

			return std::errc::invalid_argument;
		}

		template<typename fp_T, _p::is_supported_charconv_c char_T>
		[[nodiscard]] static from_chars_result<fp_T> dec2fp(std::basic_string_view<char_T> const p_str)
		{
			const uintptr_t tsize = p_str.size();
			if(tsize > 126) return std::errc::no_buffer_space; //large enough to not make any sense
			for(const char_T t_char: p_str)
			{
				//last allowable character also checks that conversion to to char8_t will not alias
				if(t_char > 'e') return std::errc::invalid_argument;
			}

			std::array<char8_t, 126> buff;
			{
				char8_t* pos = buff.data();
				for(const char_T t_char : p_str)
				{
					*(pos++) = static_cast<char8_t>(t_char);
				}
			}

			return dec2fp<fp_T>(std::basic_string_view<char8_t>{buff.data(), tsize});
		}

		template <typename num_T>
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

		template <typename num_T>
		[[nodiscard]] inline constexpr uintptr_t int2dec_estimate(num_T const p_val)
		{
			using unsigned_t = std::make_unsigned_t<num_T>;

			if(p_val < 0)
			{
				return uint2dec_estimate<unsigned_t>(static_cast<unsigned_t>(-p_val)) + 1;
			}
			return uint2dec_estimate<unsigned_t>(static_cast<unsigned_t>(p_val));
		}

		template<typename Fp_t>
		[[nodiscard]] static inline uintptr_t fp2dec_estimate(Fp_t const p_val)
		{
			std::array<char, to_chars_dec_max_digits_v<Fp_t>> buff;
			char* const start = reinterpret_cast<char*>(buff.data());
			const std::to_chars_result res = std::to_chars(start, start + buff.size(), p_val);
			if(res.ec == std::errc{})
			{
				return static_cast<uintptr_t>(res.ptr - start);
			}
			return 0;
		}

		template <typename num_T>
		[[nodiscard]] static inline constexpr uintptr_t uint2hex_estimate(num_T const p_val)
		{
			if(p_val)
			{
				//skip ahead algorithm
				constexpr uint8_t lastBit = sizeof(num_T) * 8 - 1;
				return static_cast<uintptr_t>(((lastBit - static_cast<uint8_t>(std::countl_zero<num_T>(p_val))) / 4) + 1);
			}
			return 1;
		}

		static constexpr std::array x2NibTable = {u8'0', u8'1', u8'2', u8'3', u8'4', u8'5', u8'6', u8'7', u8'8', u8'9', u8'A', u8'B', u8'C', u8'D', u8'E', u8'F'};
		[[nodiscard]] static inline constexpr char8_t Hex2Nible(uintptr_t const p_val)
		{
			return x2NibTable[p_val & 0x0F];
		}

		template <typename char_T, typename num_T>
		[[nodiscard]] static inline uintptr_t uint2dec(num_T p_val, std::span<char_T, to_chars_dec_max_digits_v<num_T>> const p_str)
		{
			const uintptr_t size = uint2dec_estimate(p_val);
			char_T* pivot = p_str.data() + size;
			while(p_val > 9)
			{
				*(--pivot) = static_cast<char8_t>('0' + p_val % 10);
				p_val /= 10;
			}
			*(--pivot) = static_cast<char8_t>('0' + p_val);
			return size;
		}

		template <typename char_T, typename num_T>
		static inline void uint2dec_unsafe(num_T p_val, char_T* const p_str)
		{
			const uintptr_t size = uint2dec_estimate(p_val);
			char_T* pivot = p_str + size;
			while(p_val > 9)
			{
				*(--pivot) = static_cast<char8_t>('0' + p_val % 10);
				p_val /= 10;
			}
			*(--pivot) = static_cast<char8_t>('0' + p_val);
		}

		template <typename char_T, typename num_T>
		[[nodiscard]] static inline uintptr_t int2dec(const num_T p_val, std::span<char_T, to_chars_dec_max_digits_v<num_T>> const p_str)
		{
			using unsigned_t = std::make_unsigned_t<num_T>;
			constexpr uintptr_t usize = to_chars_dec_max_digits_v<unsigned_t>;

			if(p_val < 0)
			{
				p_str[0] = '-';
				return uint2dec<char_T, unsigned_t>(static_cast<unsigned_t>(-p_val), std::span<char_T, usize>{p_str.data() + 1, usize}) + 1;
			}

			return uint2dec<char_T, unsigned_t>(static_cast<unsigned_t>(p_val), std::span<char_T, usize>{p_str.data(), usize});
		}


		template <typename char_T, typename num_T>
		static inline void int2dec_unsafe(num_T p_val, char_T* p_str)
		{
			using unsigned_t = std::make_unsigned_t<num_T>;
			if(p_val < 0)
			{
				*(p_str++) = '-';
				uint2dec_unsafe<char_T, unsigned_t>(static_cast<unsigned_t>(-p_val), p_str);
			}
			else uint2dec_unsafe<char_T, unsigned_t>(static_cast<unsigned_t>(p_val), p_str);
		}


		template <typename char_T, typename num_T>
		[[nodiscard]] static inline uintptr_t uint2hex(const num_T p_val, std::span<char_T, to_chars_hex_max_digits_v<num_T>> const p_str)
		{
			if(!p_val)
			{
				p_str[0] = '0';
				return 1;
			};

			//skip ahead algorithm
			constexpr uint8_t lastBit = sizeof(num_T) * 8 - 1;
			const uint8_t index = static_cast<uint8_t>((lastBit - static_cast<uint8_t>(std::countl_zero<num_T>(p_val))) / 4);

			char_T* pivot = p_str.data();
			for(uint8_t t_bias = static_cast<uint8_t>(index * 4); t_bias; t_bias -= 4)
			{
				*(pivot++) = _p::Hex2Nible(static_cast<uint8_t> (p_val >> t_bias));
			}
			*(pivot++) = _p::Hex2Nible(static_cast<uint8_t> (p_val));
			return static_cast<uintptr_t> (pivot - p_str.data());
		}


		template <typename char_T, typename num_T>
		static inline void uint2hex_unsafe(const num_T p_val, char_T* p_str)
		{
			uint8_t index;
			if(p_val)
			{
				//skip ahead algorithm
				constexpr uint8_t lastBit = sizeof(num_T) * 8 - 1;
				index = static_cast<uint8_t>((lastBit - static_cast<uint8_t>(std::countl_zero<num_T>(p_val))) / 4);
			}
			else index = 0;

			for(uint8_t t_bias = static_cast<uint8_t>(index * 4); t_bias; t_bias -= 4)
			{
				*(p_str++) = _p::Hex2Nible(p_val >> t_bias);
			}
			*(p_str++) = _p::Hex2Nible(p_val);
		}

		template <typename T>
		static inline void uint2hex_fix(const uint8_t p_val, T* p_str)
		{
			*p_str   = _p::Hex2Nible(p_val >> 4);
			*++p_str = _p::Hex2Nible(p_val);
		}

		template <typename T>
		static inline void uint2hex_fix(const uint16_t p_val, T* p_str)
		{
			*p_str   = _p::Hex2Nible(p_val >> 12);
			*++p_str = _p::Hex2Nible(p_val >> 8);
			*++p_str = _p::Hex2Nible(p_val >> 4);
			*++p_str = _p::Hex2Nible(p_val);
		}

		template <typename T>
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

		template <typename T>
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

		template<typename Fp_t>
		static inline uintptr_t fp2dec(const Fp_t p_val, std::span<char8_t, to_chars_dec_max_digits_v<Fp_t>> const p_out)
		{
			char* const start = reinterpret_cast<char*>(p_out.data());
			std::to_chars_result res = std::to_chars(start, start + p_out.size(), p_val);

			if(res.ec == std::errc{})
			{
				return static_cast<uintptr_t>(res.ptr - start);
			}
			return 0;
		}

		template<typename char_T, typename Fp_t>
		static inline uintptr_t fp2dec(const Fp_t p_val, std::span<char_T, to_chars_dec_max_digits_v<Fp_t>> const p_out)
		{
			std::array<char8_t, to_chars_dec_max_digits_v<Fp_t>> buff;
			const uintptr_t ret = fp2dec<Fp_t>(p_val, buff);
			for(uintptr_t i = 0; i < ret; ++i)
			{
				p_out[i] = static_cast<char_T>(buff[i]);
			}
			return ret;
		}

		template<typename Fp_t>
		static inline void fp2dec_unsafe(const Fp_t p_val, char8_t* const p_out)
		{
			constexpr uintptr_t size = to_chars_dec_max_digits_v<Fp_t>;
			char* const start = reinterpret_cast<char*>(p_out);
			std::to_chars(start, start + size, p_val);
		}

		template<typename char_T, typename Fp_t>
		static inline void fp2dec_unsafe(const Fp_t p_val, char_T* p_out)
		{
			std::array<char8_t, to_chars_dec_max_digits_v<Fp_t>> buff;
			const uintptr_t ret = fp2dec<Fp_t>(p_val, buff);
			for(uintptr_t i = 0; i < ret; ++i)
			{
				p_out[i] = static_cast<char_T>(buff[i]);
			}
		}

		namespace
		{

			template<typename>
			struct help_char_conv;

			template<std::floating_point num_T>
			struct help_char_conv<num_T>
			{
				template<typename char_T>
				[[nodiscard]] static inline from_chars_result<num_T> from_chars(std::basic_string_view<char_T> const p_str)
				{
					return dec2fp<num_T>(p_str);
				}

				template<typename char_T>
				static inline uintptr_t to_chars(const num_T p_val, std::span<char_T, to_chars_dec_max_digits_v<num_T>> const p_str)
				{
					return fp2dec(p_val, p_str);
				}

				static inline uintptr_t estimate(const num_T p_val)
				{
					return fp2dec_estimate(p_val);
				}

				template<typename char_T>
				static inline void to_chars_unsafe(const num_T p_val, char_T* const p_str)
				{
					return fp2dec_unsafe(p_val, p_str);
				}
			};

			template<std::signed_integral num_T>
			struct help_char_conv<num_T>
			{
				template<typename char_T>
				[[nodiscard]] static inline from_chars_result<num_T> from_chars(std::basic_string_view<char_T> const p_str)
				{
					return dec2int<num_T>(p_str);
				}

				template<typename char_T>
				static inline uintptr_t to_chars(const num_T p_val, std::span<char_T, to_chars_dec_max_digits_v<num_T>> const p_str)
				{
					return int2dec(p_val, p_str);
				}

				static inline uintptr_t estimate(const num_T p_val)
				{
					return int2dec_estimate(p_val);
				}

				template<typename char_T>
				static inline void to_chars_unsafe(const num_T p_val, char_T* const p_str)
				{
					return int2dec_unsafe(p_val, p_str);
				}
			};

			template<std::unsigned_integral num_T>
			struct help_char_conv<num_T>
			{
				template<typename char_T>
				[[nodiscard]] static inline from_chars_result<num_T> from_chars(std::basic_string_view<char_T> const p_str)
				{
					return dec2uint<num_T>(p_str);
				}

				template<typename char_T>
				static inline uintptr_t to_chars(const num_T p_val, std::span<char_T, to_chars_dec_max_digits_v<num_T>> const p_str)
				{
					return uint2dec(p_val, p_str);
				}

				static inline uintptr_t estimate(const num_T p_val)
				{
					return uint2dec_estimate(p_val);
				}

				template<typename char_T>
				static inline void to_chars_unsafe(const num_T p_val, char_T* const p_str)
				{
					return uint2dec_unsafe(p_val, p_str);
				}
			};
		}	//namespace
	} //namespace _p


	//======== Public implementation ========

	namespace _p
	{
		template <_p::is_internal_charconv_c T>
		[[nodiscard]] bool is_uint(std::basic_string_view<T> const p_str)
		{
			if(p_str.size() == 0) return false;
			for(T tchar : p_str)
			{
				if(!is_digit(tchar)) return false;
			}
			return true;
		}

		template <_p::is_internal_charconv_c T>
		[[nodiscard]] bool is_int(std::basic_string_view<T> const p_str)
		{
			if(p_str.size() == 0) return false;
			if((p_str[0] == '-') || (p_str[0] == '+'))
			{
				return is_uint(p_str.substr(1));
			}
			return is_uint(p_str);
		}

		template <_p::is_internal_charconv_c T>
		[[nodiscard]] bool is_hex(std::basic_string_view<T> const p_str)
		{
			if(p_str.size() == 0) return false;
			for(T tchar : p_str)
			{
				if(!is_xdigit(tchar)) return false;
			}
			return true;
		}

		template<char_conv_dec_supported_c num_T, _p::is_internal_charconv_c char_T>
		[[nodiscard]] from_chars_result<num_T> from_chars(std::basic_string_view<char_T> const p_str)
		{
			return _p::help_char_conv<num_T>::from_chars(p_str);
		}

		template<char_conv_hex_supported_c num_T, _p::is_internal_charconv_c char_T>
		[[nodiscard]] from_chars_result<num_T> from_chars_hex(std::basic_string_view<char_T> const p_str)
		{
			return _p::hex2uint<num_T>(p_str);
		}

		template <char_conv_dec_supported_c num_T>
		[[nodiscard]] uintptr_t to_chars_estimate(const num_T p_val)
		{
			return help_char_conv<num_T>::estimate(p_val);
		}

		template <_p::is_internal_charconv_c char_T, char_conv_dec_supported_c num_T>
		[[nodiscard]] uintptr_t to_chars(const num_T p_val, std::span<char_T, to_chars_dec_max_digits_v<num_T>> const p_str)
		{
			return _p::help_char_conv<num_T>::to_chars(p_val, p_str);
		}

		template <_p::is_internal_charconv_c char_T, char_conv_dec_supported_c num_T>
		void to_chars_unsafe(const num_T p_val, char_T* const p_str)
		{
			return _p::help_char_conv<num_T>::to_chars_unsafe(p_val, p_str);
		}

		template <char_conv_hex_supported_c num_T>
		[[nodiscard]] uintptr_t to_chars_hex_estimate(const num_T p_val)
		{
			return _p::uint2hex_estimate(p_val);
		}

		template <_p::is_internal_charconv_c char_T, char_conv_hex_supported_c num_T>
		[[nodiscard]] uintptr_t to_chars_hex(const num_T p_val, std::span<char_T, to_chars_hex_max_digits_v<num_T>> const p_str)
		{
			return _p::uint2hex(p_val, p_str);
		}

		template <_p::is_internal_charconv_c char_T, char_conv_hex_supported_c num_T>
		void to_chars_hex_unsafe(const num_T p_val, char_T* const p_str)
		{
			_p::uint2hex_unsafe(p_val, p_str);
		}

		template <_p::is_internal_charconv_c char_T, char_conv_hex_supported_c num_T>
		void to_chars_hex_fix(const num_T p_val, std::span<char_T, to_chars_hex_max_digits_v<num_T>> const p_str)
		{
			_p::uint2hex_fix(p_val, p_str.data());
		}

		template <_p::is_internal_charconv_c char_T, char_conv_hex_supported_c num_T>
		void to_chars_hex_fix_unsafe(const num_T p_val, char_T* const p_str)
		{
			_p::uint2hex_fix(p_val, p_str);
		}
	} //namespace _p

//======== Explicit instantiation ========

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
	
		template from_chars_result<uint8_t    > from_chars<uint8_t    , char8_t>(std::basic_string_view<char8_t>);
		template from_chars_result<uint16_t   > from_chars<uint16_t   , char8_t>(std::basic_string_view<char8_t>);
		template from_chars_result<uint32_t   > from_chars<uint32_t   , char8_t>(std::basic_string_view<char8_t>);
		template from_chars_result<uint64_t   > from_chars<uint64_t   , char8_t>(std::basic_string_view<char8_t>);
		template from_chars_result<int8_t     > from_chars<int8_t     , char8_t>(std::basic_string_view<char8_t>);
		template from_chars_result<int16_t    > from_chars<int16_t    , char8_t>(std::basic_string_view<char8_t>);
		template from_chars_result<int32_t    > from_chars<int32_t    , char8_t>(std::basic_string_view<char8_t>);
		template from_chars_result<int64_t    > from_chars<int64_t    , char8_t>(std::basic_string_view<char8_t>);
		template from_chars_result<float      > from_chars<float      , char8_t>(std::basic_string_view<char8_t>);
		template from_chars_result<double     > from_chars<double     , char8_t>(std::basic_string_view<char8_t>);
		template from_chars_result<long double> from_chars<long double, char8_t>(std::basic_string_view<char8_t>);

		template from_chars_result<uint8_t    > from_chars<uint8_t    , char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<uint16_t   > from_chars<uint16_t   , char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<uint32_t   > from_chars<uint32_t   , char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<uint64_t   > from_chars<uint64_t   , char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<int8_t     > from_chars<int8_t     , char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<int16_t    > from_chars<int16_t    , char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<int32_t    > from_chars<int32_t    , char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<int64_t    > from_chars<int64_t    , char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<float      > from_chars<float      , char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<double     > from_chars<double     , char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<long double> from_chars<long double, char16_t>(std::basic_string_view<char16_t>);

		template from_chars_result<uint8_t    > from_chars<uint8_t    , char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<uint16_t   > from_chars<uint16_t   , char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<uint32_t   > from_chars<uint32_t   , char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<uint64_t   > from_chars<uint64_t   , char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<int8_t     > from_chars<int8_t     , char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<int16_t    > from_chars<int16_t    , char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<int32_t    > from_chars<int32_t    , char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<int64_t    > from_chars<int64_t    , char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<float      > from_chars<float      , char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<double     > from_chars<double     , char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<long double> from_chars<long double, char32_t>(std::basic_string_view<char32_t>);

		template from_chars_result<uint8_t > from_chars_hex<uint8_t , char8_t>(std::basic_string_view<char8_t>);
		template from_chars_result<uint16_t> from_chars_hex<uint16_t, char8_t>(std::basic_string_view<char8_t>);
		template from_chars_result<uint32_t> from_chars_hex<uint32_t, char8_t>(std::basic_string_view<char8_t>);
		template from_chars_result<uint64_t> from_chars_hex<uint64_t, char8_t>(std::basic_string_view<char8_t>);

		template from_chars_result<uint8_t > from_chars_hex<uint8_t , char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<uint16_t> from_chars_hex<uint16_t, char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<uint32_t> from_chars_hex<uint32_t, char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<uint64_t> from_chars_hex<uint64_t, char16_t>(std::basic_string_view<char16_t>);

		template from_chars_result<uint8_t > from_chars_hex<uint8_t , char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<uint16_t> from_chars_hex<uint16_t, char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<uint32_t> from_chars_hex<uint32_t, char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<uint64_t> from_chars_hex<uint64_t, char32_t>(std::basic_string_view<char32_t>);


		template uintptr_t to_chars_estimate<uint8_t    >(uint8_t    );
		template uintptr_t to_chars_estimate<uint16_t   >(uint16_t   );
		template uintptr_t to_chars_estimate<uint32_t   >(uint32_t   );
		template uintptr_t to_chars_estimate<uint64_t   >(uint64_t   );
		template uintptr_t to_chars_estimate<int8_t     >(int8_t     );
		template uintptr_t to_chars_estimate<int16_t    >(int16_t    );
		template uintptr_t to_chars_estimate<int32_t    >(int32_t    );
		template uintptr_t to_chars_estimate<int64_t    >(int64_t    );
		template uintptr_t to_chars_estimate<float      >(float      );
		template uintptr_t to_chars_estimate<double     >(double     );
		template uintptr_t to_chars_estimate<long double>(long double);

		template uintptr_t to_chars<char8_t, uint8_t    >(uint8_t    , std::span<char8_t, to_chars_dec_max_digits_v<uint8_t    >>);
		template uintptr_t to_chars<char8_t, uint16_t   >(uint16_t   , std::span<char8_t, to_chars_dec_max_digits_v<uint16_t   >>);
		template uintptr_t to_chars<char8_t, uint32_t   >(uint32_t   , std::span<char8_t, to_chars_dec_max_digits_v<uint32_t   >>);
		template uintptr_t to_chars<char8_t, uint64_t   >(uint64_t   , std::span<char8_t, to_chars_dec_max_digits_v<uint64_t   >>);
		template uintptr_t to_chars<char8_t, int8_t     >(int8_t     , std::span<char8_t, to_chars_dec_max_digits_v<int8_t     >>);
		template uintptr_t to_chars<char8_t, int16_t    >(int16_t    , std::span<char8_t, to_chars_dec_max_digits_v<int16_t    >>);
		template uintptr_t to_chars<char8_t, int32_t    >(int32_t    , std::span<char8_t, to_chars_dec_max_digits_v<int32_t    >>);
		template uintptr_t to_chars<char8_t, int64_t    >(int64_t    , std::span<char8_t, to_chars_dec_max_digits_v<int64_t    >>);
		template uintptr_t to_chars<char8_t, float      >(float      , std::span<char8_t, to_chars_dec_max_digits_v<float      >>);
		template uintptr_t to_chars<char8_t, double     >(double     , std::span<char8_t, to_chars_dec_max_digits_v<double     >>);
		template uintptr_t to_chars<char8_t, long double>(long double, std::span<char8_t, to_chars_dec_max_digits_v<long double>>);

		template uintptr_t to_chars<char16_t, uint8_t    >(uint8_t    , std::span<char16_t, to_chars_dec_max_digits_v<uint8_t    >>);
		template uintptr_t to_chars<char16_t, uint16_t   >(uint16_t   , std::span<char16_t, to_chars_dec_max_digits_v<uint16_t   >>);
		template uintptr_t to_chars<char16_t, uint32_t   >(uint32_t   , std::span<char16_t, to_chars_dec_max_digits_v<uint32_t   >>);
		template uintptr_t to_chars<char16_t, uint64_t   >(uint64_t   , std::span<char16_t, to_chars_dec_max_digits_v<uint64_t   >>);
		template uintptr_t to_chars<char16_t, int8_t     >(int8_t     , std::span<char16_t, to_chars_dec_max_digits_v<int8_t     >>);
		template uintptr_t to_chars<char16_t, int16_t    >(int16_t    , std::span<char16_t, to_chars_dec_max_digits_v<int16_t    >>);
		template uintptr_t to_chars<char16_t, int32_t    >(int32_t    , std::span<char16_t, to_chars_dec_max_digits_v<int32_t    >>);
		template uintptr_t to_chars<char16_t, int64_t    >(int64_t    , std::span<char16_t, to_chars_dec_max_digits_v<int64_t    >>);
		template uintptr_t to_chars<char16_t, float      >(float      , std::span<char16_t, to_chars_dec_max_digits_v<float      >>);
		template uintptr_t to_chars<char16_t, double     >(double     , std::span<char16_t, to_chars_dec_max_digits_v<double     >>);
		template uintptr_t to_chars<char16_t, long double>(long double, std::span<char16_t, to_chars_dec_max_digits_v<long double>>);

		template uintptr_t to_chars<char32_t, uint8_t    >(uint8_t    , std::span<char32_t, to_chars_dec_max_digits_v<uint8_t    >>);
		template uintptr_t to_chars<char32_t, uint16_t   >(uint16_t   , std::span<char32_t, to_chars_dec_max_digits_v<uint16_t   >>);
		template uintptr_t to_chars<char32_t, uint32_t   >(uint32_t   , std::span<char32_t, to_chars_dec_max_digits_v<uint32_t   >>);
		template uintptr_t to_chars<char32_t, uint64_t   >(uint64_t   , std::span<char32_t, to_chars_dec_max_digits_v<uint64_t   >>);
		template uintptr_t to_chars<char32_t, int8_t     >(int8_t     , std::span<char32_t, to_chars_dec_max_digits_v<int8_t     >>);
		template uintptr_t to_chars<char32_t, int16_t    >(int16_t    , std::span<char32_t, to_chars_dec_max_digits_v<int16_t    >>);
		template uintptr_t to_chars<char32_t, int32_t    >(int32_t    , std::span<char32_t, to_chars_dec_max_digits_v<int32_t    >>);
		template uintptr_t to_chars<char32_t, int64_t    >(int64_t    , std::span<char32_t, to_chars_dec_max_digits_v<int64_t    >>);
		template uintptr_t to_chars<char32_t, float      >(float      , std::span<char32_t, to_chars_dec_max_digits_v<float      >>);
		template uintptr_t to_chars<char32_t, double     >(double     , std::span<char32_t, to_chars_dec_max_digits_v<double     >>);
		template uintptr_t to_chars<char32_t, long double>(long double, std::span<char32_t, to_chars_dec_max_digits_v<long double>>);

		template void to_chars_unsafe<char8_t, uint8_t    >(uint8_t    , char8_t*);
		template void to_chars_unsafe<char8_t, uint16_t   >(uint16_t   , char8_t*);
		template void to_chars_unsafe<char8_t, uint32_t   >(uint32_t   , char8_t*);
		template void to_chars_unsafe<char8_t, uint64_t   >(uint64_t   , char8_t*);
		template void to_chars_unsafe<char8_t, int8_t     >(int8_t     , char8_t*);
		template void to_chars_unsafe<char8_t, int16_t    >(int16_t    , char8_t*);
		template void to_chars_unsafe<char8_t, int32_t    >(int32_t    , char8_t*);
		template void to_chars_unsafe<char8_t, int64_t    >(int64_t    , char8_t*);
		template void to_chars_unsafe<char8_t, float      >(float      , char8_t*);
		template void to_chars_unsafe<char8_t, double     >(double     , char8_t*);
		template void to_chars_unsafe<char8_t, long double>(long double, char8_t*);

		template void to_chars_unsafe<char16_t, uint8_t    >(uint8_t    , char16_t*);
		template void to_chars_unsafe<char16_t, uint16_t   >(uint16_t   , char16_t*);
		template void to_chars_unsafe<char16_t, uint32_t   >(uint32_t   , char16_t*);
		template void to_chars_unsafe<char16_t, uint64_t   >(uint64_t   , char16_t*);
		template void to_chars_unsafe<char16_t, int8_t     >(int8_t     , char16_t*);
		template void to_chars_unsafe<char16_t, int16_t    >(int16_t    , char16_t*);
		template void to_chars_unsafe<char16_t, int32_t    >(int32_t    , char16_t*);
		template void to_chars_unsafe<char16_t, int64_t    >(int64_t    , char16_t*);
		template void to_chars_unsafe<char16_t, float      >(float      , char16_t*);
		template void to_chars_unsafe<char16_t, double     >(double     , char16_t*);
		template void to_chars_unsafe<char16_t, long double>(long double, char16_t*);

		template void to_chars_unsafe<char32_t, uint8_t    >(uint8_t    , char32_t*);
		template void to_chars_unsafe<char32_t, uint16_t   >(uint16_t   , char32_t*);
		template void to_chars_unsafe<char32_t, uint32_t   >(uint32_t   , char32_t*);
		template void to_chars_unsafe<char32_t, uint64_t   >(uint64_t   , char32_t*);
		template void to_chars_unsafe<char32_t, int8_t     >(int8_t     , char32_t*);
		template void to_chars_unsafe<char32_t, int16_t    >(int16_t    , char32_t*);
		template void to_chars_unsafe<char32_t, int32_t    >(int32_t    , char32_t*);
		template void to_chars_unsafe<char32_t, int64_t    >(int64_t    , char32_t*);
		template void to_chars_unsafe<char32_t, float      >(float      , char32_t*);
		template void to_chars_unsafe<char32_t, double     >(double     , char32_t*);
		template void to_chars_unsafe<char32_t, long double>(long double, char32_t*);


		template uintptr_t to_chars_hex_estimate<uint8_t >(uint8_t );
		template uintptr_t to_chars_hex_estimate<uint16_t>(uint16_t);
		template uintptr_t to_chars_hex_estimate<uint32_t>(uint32_t);
		template uintptr_t to_chars_hex_estimate<uint64_t>(uint64_t);

		template uintptr_t to_chars_hex<char8_t, uint8_t >(uint8_t , std::span<char8_t, to_chars_hex_max_digits_v<uint8_t >>);
		template uintptr_t to_chars_hex<char8_t, uint16_t>(uint16_t, std::span<char8_t, to_chars_hex_max_digits_v<uint16_t>>);
		template uintptr_t to_chars_hex<char8_t, uint32_t>(uint32_t, std::span<char8_t, to_chars_hex_max_digits_v<uint32_t>>);
		template uintptr_t to_chars_hex<char8_t, uint64_t>(uint64_t, std::span<char8_t, to_chars_hex_max_digits_v<uint64_t>>);

		template uintptr_t to_chars_hex<char16_t, uint8_t >(uint8_t , std::span<char16_t, to_chars_hex_max_digits_v<uint8_t >>);
		template uintptr_t to_chars_hex<char16_t, uint16_t>(uint16_t, std::span<char16_t, to_chars_hex_max_digits_v<uint16_t>>);
		template uintptr_t to_chars_hex<char16_t, uint32_t>(uint32_t, std::span<char16_t, to_chars_hex_max_digits_v<uint32_t>>);
		template uintptr_t to_chars_hex<char16_t, uint64_t>(uint64_t, std::span<char16_t, to_chars_hex_max_digits_v<uint64_t>>);

		template uintptr_t to_chars_hex<char32_t, uint8_t >(uint8_t , std::span<char32_t, to_chars_hex_max_digits_v<uint8_t >>);
		template uintptr_t to_chars_hex<char32_t, uint16_t>(uint16_t, std::span<char32_t, to_chars_hex_max_digits_v<uint16_t>>);
		template uintptr_t to_chars_hex<char32_t, uint32_t>(uint32_t, std::span<char32_t, to_chars_hex_max_digits_v<uint32_t>>);
		template uintptr_t to_chars_hex<char32_t, uint64_t>(uint64_t, std::span<char32_t, to_chars_hex_max_digits_v<uint64_t>>);


		template void to_chars_hex_unsafe<char8_t, uint8_t >(uint8_t , char8_t*);
		template void to_chars_hex_unsafe<char8_t, uint16_t>(uint16_t, char8_t*);
		template void to_chars_hex_unsafe<char8_t, uint32_t>(uint32_t, char8_t*);
		template void to_chars_hex_unsafe<char8_t, uint64_t>(uint64_t, char8_t*);

		template void to_chars_hex_unsafe<char16_t, uint8_t >(uint8_t , char16_t*);
		template void to_chars_hex_unsafe<char16_t, uint16_t>(uint16_t, char16_t*);
		template void to_chars_hex_unsafe<char16_t, uint32_t>(uint32_t, char16_t*);
		template void to_chars_hex_unsafe<char16_t, uint64_t>(uint64_t, char16_t*);

		template void to_chars_hex_unsafe<char32_t, uint8_t >(uint8_t , char32_t*);
		template void to_chars_hex_unsafe<char32_t, uint16_t>(uint16_t, char32_t*);
		template void to_chars_hex_unsafe<char32_t, uint32_t>(uint32_t, char32_t*);
		template void to_chars_hex_unsafe<char32_t, uint64_t>(uint64_t, char32_t*);


		template void to_chars_hex_fix<char8_t, uint8_t >(uint8_t , std::span<char8_t, to_chars_hex_max_digits_v<uint8_t >>);
		template void to_chars_hex_fix<char8_t, uint16_t>(uint16_t, std::span<char8_t, to_chars_hex_max_digits_v<uint16_t>>);
		template void to_chars_hex_fix<char8_t, uint32_t>(uint32_t, std::span<char8_t, to_chars_hex_max_digits_v<uint32_t>>);
		template void to_chars_hex_fix<char8_t, uint64_t>(uint64_t, std::span<char8_t, to_chars_hex_max_digits_v<uint64_t>>);

		template void to_chars_hex_fix<char16_t, uint8_t >(uint8_t , std::span<char16_t, to_chars_hex_max_digits_v<uint8_t >>);
		template void to_chars_hex_fix<char16_t, uint16_t>(uint16_t, std::span<char16_t, to_chars_hex_max_digits_v<uint16_t>>);
		template void to_chars_hex_fix<char16_t, uint32_t>(uint32_t, std::span<char16_t, to_chars_hex_max_digits_v<uint32_t>>);
		template void to_chars_hex_fix<char16_t, uint64_t>(uint64_t, std::span<char16_t, to_chars_hex_max_digits_v<uint64_t>>);

		template void to_chars_hex_fix<char32_t, uint8_t >(uint8_t , std::span<char32_t, to_chars_hex_max_digits_v<uint8_t >>);
		template void to_chars_hex_fix<char32_t, uint16_t>(uint16_t, std::span<char32_t, to_chars_hex_max_digits_v<uint16_t>>);
		template void to_chars_hex_fix<char32_t, uint32_t>(uint32_t, std::span<char32_t, to_chars_hex_max_digits_v<uint32_t>>);
		template void to_chars_hex_fix<char32_t, uint64_t>(uint64_t, std::span<char32_t, to_chars_hex_max_digits_v<uint64_t>>);


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

	} //namespace

} //namespace core

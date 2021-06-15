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

#include <CoreLib/string/core_string_numeric.hpp>
#include <charconv>


namespace core
{
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
		[[nodiscard]] static inline from_chars_result<uint_T> dec2uint(std::basic_string_view<char_T> p_str)
		{
			uintptr_t size = p_str.size();
			if(size == 0)
			{
				return std::errc::invalid_argument;
			}

			uint_T	r_val = 0;
			uint8_t	t_val;

			for(char_T tchar : p_str)
			{
				if(!is_digit(tchar))
				{
					return std::errc::invalid_argument;
				}
				t_val = static_cast<uint8_t>(tchar - '0');

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
		[[nodiscard]] static inline from_chars_result<int_T> dec2int(std::basic_string_view<char_T> p_str)
		{
			uintptr_t size = p_str.size();
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
		[[nodiscard]] static inline from_chars_result<uint_T> hex2uint(std::basic_string_view<char_T> p_str)
		{
			const uintptr_t size = p_str.size();
			if(size == 0)
			{
				return std::errc::invalid_argument;
			}

			uint_T	r_val = 0;
			char_T	t_val;

			const char_T* pivot	= p_str.data();
			const char_T* end	= pivot + size;

			for(; pivot != end; ++pivot)
			{
				if(r_val > hex_unsigned_help<uint_T>::threshold_val)
				{
					return {};
				}

				t_val = *pivot;
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
		[[nodiscard]] static inline from_chars_result<fp_T> dec2fp(std::basic_string_view<char8_t> p_str)
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
		[[nodiscard]] static from_chars_result<fp_T> dec2fp(std::basic_string_view<char_T> p_str)
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


		namespace
		{
			template<typename num_T>
			struct help_from_chars;

			template<std::floating_point num_T>
			struct help_from_chars<num_T>
			{
				template<typename char_T>
				[[nodiscard]] static inline from_chars_result<num_T> from_chars(std::basic_string_view<char_T> p_str)
				{
					return dec2fp<num_T>(p_str);
				}
			};

			template<std::signed_integral num_T>
			struct help_from_chars<num_T>
			{
				template<typename char_T>
				[[nodiscard]] static inline from_chars_result<num_T> from_chars(std::basic_string_view<char_T> p_str)
				{
					return dec2int<num_T>(p_str);
				}
			};

			template<std::unsigned_integral num_T>
			struct help_from_chars<num_T>
			{
				template<typename char_T>
				[[nodiscard]] static inline from_chars_result<num_T> from_chars(std::basic_string_view<char_T> p_str)
				{
					return dec2uint<num_T>(p_str);
				}
			};
		}	//namespace


		static constexpr char8_t x2NibTable[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
		[[nodiscard]] static inline constexpr char8_t Hex2Nible(uint8_t p_val)
		{
			return x2NibTable[p_val & 0x0F];
		}

		template <typename char_T, typename num_T>
		static inline uintptr_t uint2dec(num_T p_val, std::span<char_T, to_chars_dec_max_digits_v<num_T>> p_str)
		{
			uint8_t		t_rem;

			std::array<char_T, to_chars_dec_max_digits_v<num_T>> t_buff;
			char_T* const	last	= t_buff.data() + t_buff.size();
			char_T*			pivot	= last;

			do
			{
				t_rem = static_cast<uint8_t>(p_val % 10);
				p_val /= 10;
				*(--pivot) = t_rem + '0';
			}
			while(p_val);

			uintptr_t t_dif = last - pivot;
			{
				char_T* p2 = p_str.data();
				do
				{
					*(p2++) = *pivot;
				}
				while(++pivot < last);
			}

			return t_dif;
		}

		template <typename char_T, typename num_T>
		static inline uintptr_t int2dec(num_T p_val, std::span<char_T, to_chars_dec_max_digits_v<num_T>> p_str)
		{
			if(p_val < 0)
			{
				p_str[0] = '-';

				uint8_t		t_rem;

				std::array<char_T, to_chars_dec_max_digits_v<num_T>> t_buff;
				char_T* const	last	= t_buff.data() + t_buff.size();
				char_T*			pivot	= last;

				do
				{
					t_rem = -static_cast<uint8_t>(p_val % 10);
					p_val /= 10;
					*(--pivot) = t_rem + '0';
				}
				while(p_val);

				uintptr_t t_dif = last - pivot;
				{
					char_T* p2 = p_str.data() + 1;
					do
					{
						*(p2++) = *pivot;
					}
					while(++pivot < last);
				}

				return t_dif + 1;
			}
			return uint2dec(p_val, p_str);
		}


		template <typename char_T, typename num_T>
		static inline uintptr_t uint2hex(num_T p_val, std::span<char_T, to_chars_hex_max_digits_v<num_T>> p_str)
		{
			uint8_t index;
			if(p_val)
			{
				//skip ahead algorithm
				constexpr uint8_t lastBit = sizeof(num_T) * 8 - 1;
				index = static_cast<uint8_t>((lastBit - static_cast<uint8_t>(std::countl_zero<num_T>(p_val))) / 4);
			}
			else index = 0;

			char_T* pivot = p_str.data();

			for(uint8_t t_bias = static_cast<uint8_t>(index * 4); t_bias; t_bias -= 4)
			{
				*(pivot++) = _p::Hex2Nible(static_cast<uint8_t> (p_val >> t_bias));
			}
			*(pivot++) = _p::Hex2Nible(static_cast<uint8_t> (p_val));
			return static_cast<uintptr_t> (pivot - p_str.data());
		}

		template <typename T>
		static inline void uint2hex_fix(uint8_t p_val, std::span<T, 2> p_str)
		{
			p_str[0] = _p::Hex2Nible(p_val >> 4);
			p_str[1] = _p::Hex2Nible(p_val);
		}

		template <typename T>
		static inline void uint2hex_fix(uint16_t p_val, std::span<T, 4> p_str)
		{
			p_str[0] = _p::Hex2Nible(static_cast<uint8_t>(p_val >> 12));
			p_str[1] = _p::Hex2Nible(static_cast<uint8_t>(p_val >> 8));
			p_str[2] = _p::Hex2Nible(static_cast<uint8_t>(p_val >> 4));
			p_str[3] = _p::Hex2Nible(static_cast<uint8_t>(p_val));
		}

		template <typename T>
		static inline void uint2hex_fix(uint32_t p_val, std::span<T, 8> p_str)
		{
			p_str[0] = _p::Hex2Nible(static_cast<uint8_t>(p_val >> 28));
			p_str[1] = _p::Hex2Nible(static_cast<uint8_t>(p_val >> 24));
			p_str[2] = _p::Hex2Nible(static_cast<uint8_t>(p_val >> 20));
			p_str[3] = _p::Hex2Nible(static_cast<uint8_t>(p_val >> 16));
			p_str[4] = _p::Hex2Nible(static_cast<uint8_t>(p_val >> 12));
			p_str[5] = _p::Hex2Nible(static_cast<uint8_t>(p_val >> 8));
			p_str[6] = _p::Hex2Nible(static_cast<uint8_t>(p_val >> 4));
			p_str[7] = _p::Hex2Nible(static_cast<uint8_t>(p_val));
		}

		template <typename T>
		static inline void uint2hex_fix(uint64_t p_val, std::span<T, 16> p_str)
		{
			p_str[ 0] = _p::Hex2Nible(static_cast<uint8_t>(p_val >> 60));
			p_str[ 1] = _p::Hex2Nible(static_cast<uint8_t>(p_val >> 56));
			p_str[ 2] = _p::Hex2Nible(static_cast<uint8_t>(p_val >> 52));
			p_str[ 3] = _p::Hex2Nible(static_cast<uint8_t>(p_val >> 48));
			p_str[ 4] = _p::Hex2Nible(static_cast<uint8_t>(p_val >> 44));
			p_str[ 5] = _p::Hex2Nible(static_cast<uint8_t>(p_val >> 40));
			p_str[ 6] = _p::Hex2Nible(static_cast<uint8_t>(p_val >> 36));
			p_str[ 7] = _p::Hex2Nible(static_cast<uint8_t>(p_val >> 32));
			p_str[ 8] = _p::Hex2Nible(static_cast<uint8_t>(p_val >> 28));
			p_str[ 9] = _p::Hex2Nible(static_cast<uint8_t>(p_val >> 24));
			p_str[10] = _p::Hex2Nible(static_cast<uint8_t>(p_val >> 20));
			p_str[11] = _p::Hex2Nible(static_cast<uint8_t>(p_val >> 16));
			p_str[12] = _p::Hex2Nible(static_cast<uint8_t>(p_val >> 12));
			p_str[13] = _p::Hex2Nible(static_cast<uint8_t>(p_val >> 8));
			p_str[14] = _p::Hex2Nible(static_cast<uint8_t>(p_val >> 4));
			p_str[15] = _p::Hex2Nible(static_cast<uint8_t>(p_val));
		}


		template<typename Fp_t>
		static inline uintptr_t fp2dec(Fp_t p_val, std::span<char8_t, to_chars_dec_max_digits_v<Fp_t>> p_out)
		{
			char* start = reinterpret_cast<char*>(p_out.data());
			std::to_chars_result res = std::to_chars(start, start + p_out.size(), p_val);

			if(res.ec == std::errc{})
			{
				return static_cast<uintptr_t>(res.ptr - start);
			}
			return 0;
		}

		template<typename char_T, typename Fp_t>
		static inline uintptr_t fp2dec(Fp_t p_val, std::span<char_T, to_chars_dec_max_digits_v<Fp_t>> p_out)
		{
			std::array<char8_t, to_chars_dec_max_digits_v<Fp_t>> buff;
			uintptr_t ret = fp2dec<Fp_t>(p_val, buff);
			for(uintptr_t i = 0; i < ret; ++i)
			{
				p_out[i] = static_cast<char32_t>(buff[i]);
			}
			return ret;
		}


		namespace
		{
			template<typename>
			struct help_to_chars;

			template<std::floating_point num_T>
			struct help_to_chars<num_T>
			{
				template<typename char_T>
				static inline uintptr_t to_chars(num_T p_val, std::span<char_T, to_chars_dec_max_digits_v<num_T>> p_str)
				{
					return fp2dec(p_val, p_str);
				}
			};

			template<std::signed_integral num_T>
			struct help_to_chars<num_T>
			{
				template<typename char_T>
				static inline uintptr_t to_chars(num_T p_val, std::span<char_T, to_chars_dec_max_digits_v<num_T>> p_str)
				{
					return int2dec(p_val, p_str);
				}
			};

			template<std::unsigned_integral num_T>
			struct help_to_chars<num_T>
			{
				template<typename char_T>
				static inline uintptr_t to_chars(num_T p_val, std::span<char_T, to_chars_dec_max_digits_v<num_T>> p_str)
				{
					return uint2dec(p_val, p_str);
				}
			};
		}	//namespace

	} //namespace _p


	//======== Public implementation ========

	namespace _p
	{
		template <_p::is_internal_charconv_c T>
		[[nodiscard]] bool is_uint(std::basic_string_view<T> p_str)
		{
			if(p_str.size() == 0) return false;
			for(T tchar : p_str)
			{
				if(!is_digit(tchar)) return false;
			}
			return true;
		}

		template <_p::is_internal_charconv_c T>
		[[nodiscard]] bool is_int(std::basic_string_view<T> p_str)
		{
			if(p_str.size() == 0) return false;
			if((p_str[0] == '-') || (p_str[0] == '+'))
			{
				return is_uint(p_str.substr(1));
			}
			return is_uint(p_str);
		}

		template <_p::is_internal_charconv_c T>
		[[nodiscard]] bool is_hex(std::basic_string_view<T> p_str)
		{
			if(p_str.size() == 0) return false;
			for(T tchar : p_str)
			{
				if(!is_xdigit(tchar)) return false;
			}
			return true;
		}

		template<char_conv_dec_supported_c num_T, _p::is_internal_charconv_c char_T>
		[[nodiscard]] from_chars_result<num_T> from_chars(std::basic_string_view<char_T> p_str)
		{
			return _p::help_from_chars<num_T>::from_chars(p_str);
		}

		template<char_conv_hex_supported_c num_T, _p::is_internal_charconv_c char_T>
		[[nodiscard]] from_chars_result<num_T> from_chars_hex(std::basic_string_view<char_T> p_str)
		{
			return _p::hex2uint<num_T>(p_str);
		}


		template <_p::is_internal_charconv_c char_T, char_conv_dec_supported_c num_T>
		[[nodiscard]] uintptr_t to_chars(num_T p_val, std::span<char_T, to_chars_dec_max_digits_v<num_T>> p_str)
		{
			return _p::help_to_chars<num_T>::to_chars(p_val, p_str);
		}


		template <_p::is_internal_charconv_c char_T, char_conv_hex_supported_c num_T>
		[[nodiscard]] uintptr_t to_chars_hex(num_T p_val, std::span<char_T, to_chars_hex_max_digits_v<num_T>> p_str)
		{
			return _p::uint2hex(p_val, p_str);
		}

		template <_p::is_internal_charconv_c char_T, char_conv_hex_supported_c num_T>
		void to_chars_hex_fix(num_T p_val, std::span<char_T, to_chars_hex_max_digits_v<num_T>> p_str)
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
	
		template from_chars_result<long double> from_chars<long double, char8_t>(std::basic_string_view<char8_t>);
		template from_chars_result<double     > from_chars<double     , char8_t>(std::basic_string_view<char8_t>);
		template from_chars_result<float      > from_chars<float      , char8_t>(std::basic_string_view<char8_t>);
		template from_chars_result<uint64_t   > from_chars<uint64_t   , char8_t>(std::basic_string_view<char8_t>);
		template from_chars_result<uint32_t   > from_chars<uint32_t   , char8_t>(std::basic_string_view<char8_t>);
		template from_chars_result<uint16_t   > from_chars<uint16_t   , char8_t>(std::basic_string_view<char8_t>);
		template from_chars_result<uint8_t    > from_chars<uint8_t    , char8_t>(std::basic_string_view<char8_t>);
		template from_chars_result<int64_t    > from_chars<int64_t    , char8_t>(std::basic_string_view<char8_t>);
		template from_chars_result<int32_t    > from_chars<int32_t    , char8_t>(std::basic_string_view<char8_t>);
		template from_chars_result<int16_t    > from_chars<int16_t    , char8_t>(std::basic_string_view<char8_t>);
		template from_chars_result<int8_t     > from_chars<int8_t     , char8_t>(std::basic_string_view<char8_t>);
	
		template from_chars_result<long double> from_chars<long double, char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<double     > from_chars<double     , char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<float      > from_chars<float      , char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<uint64_t   > from_chars<uint64_t   , char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<uint32_t   > from_chars<uint32_t   , char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<uint16_t   > from_chars<uint16_t   , char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<uint8_t    > from_chars<uint8_t    , char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<int64_t    > from_chars<int64_t    , char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<int32_t    > from_chars<int32_t    , char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<int16_t    > from_chars<int16_t    , char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<int8_t     > from_chars<int8_t     , char16_t>(std::basic_string_view<char16_t>);
	
		template from_chars_result<long double> from_chars<long double, char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<double     > from_chars<double     , char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<float      > from_chars<float      , char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<uint64_t   > from_chars<uint64_t   , char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<uint32_t   > from_chars<uint32_t   , char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<uint16_t   > from_chars<uint16_t   , char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<uint8_t    > from_chars<uint8_t    , char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<int64_t    > from_chars<int64_t    , char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<int32_t    > from_chars<int32_t    , char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<int16_t    > from_chars<int16_t    , char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<int8_t     > from_chars<int8_t     , char32_t>(std::basic_string_view<char32_t>);
	
		template from_chars_result<uint64_t> from_chars_hex<uint64_t, char8_t>(std::basic_string_view<char8_t>);
		template from_chars_result<uint32_t> from_chars_hex<uint32_t, char8_t>(std::basic_string_view<char8_t>);
		template from_chars_result<uint16_t> from_chars_hex<uint16_t, char8_t>(std::basic_string_view<char8_t>);
		template from_chars_result<uint8_t > from_chars_hex<uint8_t , char8_t>(std::basic_string_view<char8_t>);

		template from_chars_result<uint64_t> from_chars_hex<uint64_t, char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<uint32_t> from_chars_hex<uint32_t, char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<uint16_t> from_chars_hex<uint16_t, char16_t>(std::basic_string_view<char16_t>);
		template from_chars_result<uint8_t > from_chars_hex<uint8_t , char16_t>(std::basic_string_view<char16_t>);

		template from_chars_result<uint64_t> from_chars_hex<uint64_t, char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<uint32_t> from_chars_hex<uint32_t, char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<uint16_t> from_chars_hex<uint16_t, char32_t>(std::basic_string_view<char32_t>);
		template from_chars_result<uint8_t > from_chars_hex<uint8_t , char32_t>(std::basic_string_view<char32_t>);


		template uintptr_t to_chars(uint8_t    , std::span<char8_t, to_chars_dec_max_digits_v<uint8_t    >>);
		template uintptr_t to_chars(uint16_t   , std::span<char8_t, to_chars_dec_max_digits_v<uint16_t   >>);
		template uintptr_t to_chars(uint32_t   , std::span<char8_t, to_chars_dec_max_digits_v<uint32_t   >>);
		template uintptr_t to_chars(uint64_t   , std::span<char8_t, to_chars_dec_max_digits_v<uint64_t   >>);
		template uintptr_t to_chars(int8_t     , std::span<char8_t, to_chars_dec_max_digits_v<int8_t     >>);
		template uintptr_t to_chars(int16_t    , std::span<char8_t, to_chars_dec_max_digits_v<int16_t    >>);
		template uintptr_t to_chars(int32_t    , std::span<char8_t, to_chars_dec_max_digits_v<int32_t    >>);
		template uintptr_t to_chars(int64_t    , std::span<char8_t, to_chars_dec_max_digits_v<int64_t    >>);
		template uintptr_t to_chars(float      , std::span<char8_t, to_chars_dec_max_digits_v<float      >>);
		template uintptr_t to_chars(double     , std::span<char8_t, to_chars_dec_max_digits_v<double     >>);
		template uintptr_t to_chars(long double, std::span<char8_t, to_chars_dec_max_digits_v<long double>>);

		template uintptr_t to_chars(uint8_t    , std::span<char16_t, to_chars_dec_max_digits_v<uint8_t    >>);
		template uintptr_t to_chars(uint16_t   , std::span<char16_t, to_chars_dec_max_digits_v<uint16_t   >>);
		template uintptr_t to_chars(uint32_t   , std::span<char16_t, to_chars_dec_max_digits_v<uint32_t   >>);
		template uintptr_t to_chars(uint64_t   , std::span<char16_t, to_chars_dec_max_digits_v<uint64_t   >>);
		template uintptr_t to_chars(int8_t     , std::span<char16_t, to_chars_dec_max_digits_v<int8_t     >>);
		template uintptr_t to_chars(int16_t    , std::span<char16_t, to_chars_dec_max_digits_v<int16_t    >>);
		template uintptr_t to_chars(int32_t    , std::span<char16_t, to_chars_dec_max_digits_v<int32_t    >>);
		template uintptr_t to_chars(int64_t    , std::span<char16_t, to_chars_dec_max_digits_v<int64_t    >>);
		template uintptr_t to_chars(float      , std::span<char16_t, to_chars_dec_max_digits_v<float      >>);
		template uintptr_t to_chars(double     , std::span<char16_t, to_chars_dec_max_digits_v<double     >>);
		template uintptr_t to_chars(long double, std::span<char16_t, to_chars_dec_max_digits_v<long double>>);

		template uintptr_t to_chars(uint8_t    , std::span<char32_t, to_chars_dec_max_digits_v<uint8_t    >>);
		template uintptr_t to_chars(uint16_t   , std::span<char32_t, to_chars_dec_max_digits_v<uint16_t   >>);
		template uintptr_t to_chars(uint32_t   , std::span<char32_t, to_chars_dec_max_digits_v<uint32_t   >>);
		template uintptr_t to_chars(uint64_t   , std::span<char32_t, to_chars_dec_max_digits_v<uint64_t   >>);
		template uintptr_t to_chars(int8_t     , std::span<char32_t, to_chars_dec_max_digits_v<int8_t     >>);
		template uintptr_t to_chars(int16_t    , std::span<char32_t, to_chars_dec_max_digits_v<int16_t    >>);
		template uintptr_t to_chars(int32_t    , std::span<char32_t, to_chars_dec_max_digits_v<int32_t    >>);
		template uintptr_t to_chars(int64_t    , std::span<char32_t, to_chars_dec_max_digits_v<int64_t    >>);
		template uintptr_t to_chars(float      , std::span<char32_t, to_chars_dec_max_digits_v<float      >>);
		template uintptr_t to_chars(double     , std::span<char32_t, to_chars_dec_max_digits_v<double     >>);
		template uintptr_t to_chars(long double, std::span<char32_t, to_chars_dec_max_digits_v<long double>>);

		template uintptr_t to_chars_hex(uint8_t , std::span<char8_t, to_chars_hex_max_digits_v<uint8_t >>);
		template uintptr_t to_chars_hex(uint16_t, std::span<char8_t, to_chars_hex_max_digits_v<uint16_t>>);
		template uintptr_t to_chars_hex(uint32_t, std::span<char8_t, to_chars_hex_max_digits_v<uint32_t>>);
		template uintptr_t to_chars_hex(uint64_t, std::span<char8_t, to_chars_hex_max_digits_v<uint64_t>>);

		template uintptr_t to_chars_hex(uint8_t , std::span<char16_t, to_chars_hex_max_digits_v<uint8_t >>);
		template uintptr_t to_chars_hex(uint16_t, std::span<char16_t, to_chars_hex_max_digits_v<uint16_t>>);
		template uintptr_t to_chars_hex(uint32_t, std::span<char16_t, to_chars_hex_max_digits_v<uint32_t>>);
		template uintptr_t to_chars_hex(uint64_t, std::span<char16_t, to_chars_hex_max_digits_v<uint64_t>>);

		template uintptr_t to_chars_hex(uint8_t , std::span<char32_t, to_chars_hex_max_digits_v<uint8_t >>);
		template uintptr_t to_chars_hex(uint16_t, std::span<char32_t, to_chars_hex_max_digits_v<uint16_t>>);
		template uintptr_t to_chars_hex(uint32_t, std::span<char32_t, to_chars_hex_max_digits_v<uint32_t>>);
		template uintptr_t to_chars_hex(uint64_t, std::span<char32_t, to_chars_hex_max_digits_v<uint64_t>>);

		template void to_chars_hex_fix(uint8_t , std::span<char8_t, to_chars_hex_max_digits_v<uint8_t >>);
		template void to_chars_hex_fix(uint16_t, std::span<char8_t, to_chars_hex_max_digits_v<uint16_t>>);
		template void to_chars_hex_fix(uint32_t, std::span<char8_t, to_chars_hex_max_digits_v<uint32_t>>);
		template void to_chars_hex_fix(uint64_t, std::span<char8_t, to_chars_hex_max_digits_v<uint64_t>>);

		template void to_chars_hex_fix(uint8_t , std::span<char16_t, to_chars_hex_max_digits_v<uint8_t >>);
		template void to_chars_hex_fix(uint16_t, std::span<char16_t, to_chars_hex_max_digits_v<uint16_t>>);
		template void to_chars_hex_fix(uint32_t, std::span<char16_t, to_chars_hex_max_digits_v<uint32_t>>);
		template void to_chars_hex_fix(uint64_t, std::span<char16_t, to_chars_hex_max_digits_v<uint64_t>>);

		template void to_chars_hex_fix(uint8_t , std::span<char32_t, to_chars_hex_max_digits_v<uint8_t >>);
		template void to_chars_hex_fix(uint16_t, std::span<char32_t, to_chars_hex_max_digits_v<uint16_t>>);
		template void to_chars_hex_fix(uint32_t, std::span<char32_t, to_chars_hex_max_digits_v<uint32_t>>);
		template void to_chars_hex_fix(uint64_t, std::span<char32_t, to_chars_hex_max_digits_v<uint64_t>>);
	} //namespace

} //namespace core

//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///		Provides string conversion functions to be able to handle UNICODE
///
///	\author Tiago Freire
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
#include <span>

#include <CoreLib/Core_Alternate.hpp>

//BOM
//	UTF8:		EF BB BF
//	UTF16 BE:	FE FF
//	UTF16 LE:	FF FE
//	UCS4 BE:	00 00 FE FF
//	UCS4 LE:	FF FE 00 00
//
//Note:
//	For all practical applications UTF-32 == UCS4
//
/// \n
namespace core
{

	/// \brief
	///		Helper class to assist on returning a potentially failing encoding conversion
	///		In case of failure has_value() shall return false.
	///		In case of success has_value() shall return true, and value() shall return
	///		a reference to a std::basic_string container.
	///		The class has been designed such that move semantics can be used on value()
	template<typename T>
	using encodeResult = alternate<std::basic_string<T>, bool, true, false>;

	//======== ======== Conversion

	///	\brief	Converts a UTF8 string to ANSI.
	///			Failure can occur if input string has an invalid UTF8 sequences, or has code points not representable in 8bits
	///	\warning	Input does not require to have all valid Unicode code points, or convert to valid ASCII (7bit).
	[[nodiscard]] encodeResult<char8_t> UTF8_to_ANSI(std::u8string_view p_input);

	///	\brief	Converts a UTF16 string to ANSI.
	///			Failure can occur if input string has an invalid UTF16 sequences, or has code points not representable in 8bits
	///	\warning	Input does not require to convert to valid ASCII (7bit).
	[[nodiscard]] encodeResult<char8_t> UTF16_to_ANSI(std::u16string_view p_input);
	
	///	\brief	Converts a UCS2 string to ANSI.
	///				Failure can occur if input string has code points not representable in 8bits
	///	\warning	Input does not require to have all valid Unicode code points, or convert to valid ASCII (7bit).
	[[nodiscard]] encodeResult<char8_t> UCS2_to_ANSI(std::u16string_view p_input);

	///	\brief	Converts a UCS4 (or UTF32) string to ANSI.
	///			Failure can occur if input string has code points not representable in 8bits
	///	\warning	Input does not require to have all valid Unicode code points, or convert to valid ASCII (7bit).
	[[nodiscard]] encodeResult<char8_t> UCS4_to_ANSI(std::u32string_view p_input);

	///	\brief	Converts a ANSI string to UTF8.
	///			Note: always convertible.
	///	\warning	Input does not require to have all valid ASCII (7bit) code points.
	[[nodiscard]] std::u8string ANSI_to_UTF8(std::u8string_view p_input);

	///	\brief	Converts a UTF16 string to UTF8.
	///			Failure can occur if input string has an invalid UTF16 sequences
	[[nodiscard]] encodeResult<char8_t> UTF16_to_UTF8(std::u16string_view p_input);

	///	\brief	Converts a UCS2 string to UTF8.
	///			Note: always convertible.
	///	\warning	Input/Output does not require to have all valid Unicode code points.
	[[nodiscard]] std::u8string UCS2_to_UTF8(std::u16string_view p_input);

	///	\brief	Converts a UCS4 (or UTF32) string to UTF8.
	///			Note: always convertible.
	///	\warning	Input/Output does not require to have all valid Unicode code points.
	[[nodiscard]] std::u8string UCS4_to_UTF8(std::u32string_view p_input);

	///	\brief	Converts a ANSI string to UTF16.
	///			Note: always convertible.
	///	\warning	Input does not require to have all valid ASCII code points.
	[[nodiscard]] std::u16string ANSI_to_UTF16(std::u8string_view p_input);

	///	\brief	Converts a UTF8 string to UTF16.
	///			Failure can occur if input string has an invalid UTF8 sequences, or has code points not representable in UTF16
	[[nodiscard]] encodeResult<char16_t> UTF8_to_UTF16(std::u8string_view p_input);

	///	\brief	Converts a UCS2 string to UTF16.
	///			Failure can occur if input string has code points not representable in UTF16
	[[nodiscard]] encodeResult<char16_t> UCS2_to_UTF16(std::u16string_view p_input);

	///	\brief	Converts a UCS4 (or UTF32) string to UTF16.
	///			Failure can occur if input string has code points not representable in UTF16
	[[nodiscard]] encodeResult<char16_t> UCS4_to_UTF16(std::u32string_view p_input);

	///	\brief	Converts a ANSI string to UCS2.
	///			Note: always convertible.
	///	\warning	Input does not require to have all valid ASCII code points.
	[[nodiscard]] std::u16string ANSI_to_UCS2(std::u8string_view p_input);

	///	\brief	Converts a UTF8 string to UCS2.
	///			Failure can occur if input string has an invalid UTF8 sequences, or has code points not representable in 16bits
	///	\warning	Input/Output does not require to have all valid Unicode code points.
	[[nodiscard]] encodeResult<char16_t> UTF8_to_UCS2(std::u8string_view p_input);

	///	\brief	Converts a UTF16 string to UCS2.
	///			Failure can occur if input string has an invalid UTF16 sequences, or has code points not representable in 16bits
	[[nodiscard]] encodeResult<char16_t> UTF16_to_UCS2(std::u16string_view p_input);

	///	\brief	Converts a UCS4 (or UTF32) string to UCS2.
	///			Failure can occur if input string has code points not representable in 16bits
	///	\warning	Input/Output does not require to have all valid Unicode code points.
	[[nodiscard]] encodeResult<char16_t> UCS4_to_UCS2(std::u32string_view p_input);

	///	\brief	Converts a ANSI string to UCS4 (or UTF32).
	///			Note: always convertible.
	///	\warning	Input does not require to have all valid ASCII code points.
	[[nodiscard]] std::u32string ANSI_to_UCS4(std::u8string_view p_input);

	///	\brief	Converts a UTF8 string to UCS4 (or UTF32).
	///			Failure can occur if input string has an invalid UTF8 sequences.
	///	\warning	Input/Output does not require to have all valid Unicode code points.
	[[nodiscard]] encodeResult<char32_t> UTF8_to_UCS4(std::u8string_view p_input);

	///	\brief	Converts a UTF16 string to UCS4 (or UTF32).
	///			Failure can occur if input string has an invalid UTF16 sequences
	///	\warning	Input/Output does not require to have all valid Unicode code points.
	[[nodiscard]] encodeResult<char32_t> UTF16_to_UCS4(std::u16string_view p_input);

	///	\brief	Converts a UCS2 string to UCS4 (or UTF32).
	///			Note: always convertible.
	///	\warning	Input/Output does not require to have all valid Unicode code points.
	[[nodiscard]] std::u32string UCS2_to_UCS4(std::u16string_view p_input);


	///	\brief	Converts a UTF8 string to ANSI.
	///	\param[in]	p_input - UTF8 sequence to convert
	///	\param[in]	p_placeHolder - Replacement character to be used when a UTF8 sequence is not valid or the codepoint can not be encoded in ANSI
	///	\warning	Input does not require to have all valid Unicode code points, or convert to valid ASCII (7bit).
	[[nodiscard]] std::u8string UTF8_to_ANSI_faulty(std::u8string_view p_input, char8_t p_placeHolder);

	///	\brief	Converts a UTF16 string to ANSI.
	///	\param[in]	p_input - UTF16 sequence to convert
	///	\param[in]	p_placeHolder - Replacement character to be used when a UTF16 sequence is not valid or the codepoint can not be encoded in ANSI
	///	\warning	Input does not require to convert to valid ASCII.
	[[nodiscard]] std::u8string UTF16_to_ANSI_faulty(std::u16string_view p_input, char8_t p_placeHolder);

	///	\brief	Converts a UCS2 string to ANSI.
	///	\param[in]	p_input - UCS2 sequence to convert
	///	\param[in]	p_placeHolder - Replacement character to be used when codepoint can not be encoded in ANSI
	///	\warning	Input does not require to have all valid Unicode code points, or convert to valid ASCII.
	[[nodiscard]] std::u8string UCS2_to_ANSI_faulty(std::u16string_view p_input, char8_t p_placeHolder);

	///	\brief	Converts a UCS4 (or UTF32) string to ANSI.
	///	\param[in]	p_input - UCS4 (or UTF32) sequence to convert
	///	\param[in]	p_placeHolder - Replacement character to be used when the codepoint can not be encoded in ANSI
	///	\warning	Input does not require to have all valid Unicode code points, or convert to valid ASCII.
	[[nodiscard]] std::u8string UCS4_to_ANSI_faulty(std::u32string_view p_input, char8_t p_placeHolder);

	///	\brief	Converts a UTF16 string to UTF8.
	///	\param[in]	p_input - UTF16 sequence to convert
	///	\param[in]	p_placeHolder - Replacement codepoint to be used when a UTF16 sequence is invalid
	///	\warning	Output does not require to have all valid Unicode code points.
	[[nodiscard]] std::u8string UTF16_to_UTF8_faulty(std::u16string_view p_input, char32_t p_placeHolder);

	///	\brief	Converts a UTF8 string to UTF16.
	///	\param[in]	p_input - UTF8 sequence to convert
	///	\param[in]	p_placeHolder - Replacement codepoint to be used when a UTF8 sequence is invalid,
	///								or code point can not be represented in UTF16
	///								If replacement codepoint is not representable in UTF16, nothing is used
	///	\warning	Input does not require to have all valid Unicode code points
	[[nodiscard]] std::u16string UTF8_to_UTF16_faulty(std::u8string_view p_input, char32_t p_placeHolder);

	///	\brief	Converts a UCS2 string to UTF16.
	///	\param[in]	p_input - UCS2 sequence to convert
	///	\param[in]	p_placeHolder - Replacement codepoint to be used when code point can not be represented in UTF16
	///								If replacement codepoint is not representable in UTF16, nothing is used
	[[nodiscard]] std::u16string UCS2_to_UTF16_faulty(std::u16string_view p_input, char32_t p_placeHolder);

	///	\brief	Converts a UCS4 (or UTF32) string to UTF16.
	///	\param[in]	p_input - UCS4 (or UTF32) sequence to convert
	///	\param[in]	p_placeHolder - Replacement codepoint to be used when a code point can not be represented in UTF16
	///								If replacement codepoint is not representable in UTF16, nothing is used
	///	\warning	Input does not require to have all valid Unicode code points
	[[nodiscard]] std::u16string UCS4_to_UTF16_faulty(std::u32string_view p_input, char32_t p_placeHolder);

	///	\brief	Converts a UTF8 string to UCS2.
	///	\param[in]	p_input - UTF8 sequence to convert
	///	\param[in]	p_placeHolder - Replacement codepoint to be used when a UTF8 sequence is invalid,
	///								or code point can not be represented in UCS2.
	///	\warning	Input/Output does not require to have all valid Unicode code points.
	[[nodiscard]] std::u16string UTF8_to_UCS2_faulty(std::u8string_view p_input, char16_t p_placeHolder);

	///	\brief	Converts a UTF16 string to UCS2.
	///	\param[in]	p_input - UTF16 sequence to convert
	///	\param[in]	p_placeHolder - Replacement codepoint to be used when a UTF16 sequence is invalid,
	///								or code point can not be represented in UCS2.
	[[nodiscard]] std::u16string UTF16_to_UCS2_faulty(std::u16string_view p_input, char16_t p_placeHolder);

	///	\brief	Converts a UCS4 (or UTF32) string to UCS2.
	///	\param[in]	p_input - UCS4 (or UTF32) sequence to convert
	///	\param[in]	p_placeHolder - Replacement codepoint to be used when a code point can not be represented in UCS2
	///	\warning	Input/Output does not require to have all valid Unicode code points.
	[[nodiscard]] std::u16string UCS4_to_UCS2_faulty(std::u32string_view p_input, char16_t p_placeHolder);

	///	\brief	Converts a UTF8 string to ANSI.
	///	\param[in]	p_input - UTF8 sequence to convert
	///	\param[in]	p_placeHolder - Replacement character to be used when a UTF8 sequence is not valid
	///	\warning	Input does not require to have all valid Unicode code points.
	[[nodiscard]] std::u32string UTF8_to_UCS4_faulty(std::u8string_view p_input, char32_t p_placeHolder);

	///	\brief	Converts a UTF8 string to ANSI.
	///	\param[in]	p_input - UTF8 sequence to convert
	///	\param[in]	p_placeHolder - Replacement character to be used when a UTF16 sequence is not valid
	///	\warning	Input does not require to have all valid Unicode code points
	[[nodiscard]] std::u32string UTF16_to_UCS4_faulty(std::u16string_view p_input, char32_t p_placeHolder);


	///	\brief	Converts a code point to UTF8
	///	\param[in]	p_char - Code point to convert
	///	\param[out]	p_output - Resulting UTF8 encoding. Not Null terminated
	///	\return		The number of bytes (char8_t blocks) used, in the output encoding.
	///	\warning	Input/Output does not require to encode a valid Unicode code point. Never fails.
	//
	uint8_t encode_UTF8(char32_t p_char, std::span<char8_t, 7> p_output);

	///	\brief	Converts a code point to UTF16
	///	\param[in]	p_char - Code point to convert
	///	\param[out]	p_output - Resulting UTF16 encoding. Not Null terminated
	///	\return		The number of char16_t blocks used, in the output encoding. Or 0 on failure.
	//
	uint8_t encode_UTF16(char32_t p_char, std::span<char16_t, 2> p_output);

	//======== ======== Compliance

	///	\brief	Checks if character can be encoded as a valid Unicode code point.
	///	\param[in]	p_char - Character to check
	///	\return true if input is a valid Unicode code point, false if otherwise.
	//
	[[nodiscard]] inline constexpr bool UNICODE_Compliant(char32_t p_char) { return !((p_char > 0xD7FF && p_char < 0xE000) || p_char > 0x10FFFF); }

	///	\brief	Checks if a given character is a valid ASCII character (i.e. Only first 7bits are used).
	///	\param[in] p_char - Character to check
	///	\return true if character is ASCII compliant, false if otherwise.
	//
	[[nodiscard]] inline constexpr bool ASCII_Compliant(char32_t p_char) { return p_char < 0x80; }

	///	\brief	Same as \ref core::ASCII_Compliant(char32_t).
	//
	[[nodiscard]] inline constexpr bool ASCII_Compliant(char8_t p_char) { return p_char < 0x80; }

	///	\brief	Checks if string has a valid UTF8 and Unicode encoding.
	///	\param[in]	p_str - String to check
	///	\param[in]	p_size - Size of string to check
	///	\return true if input has a valid encoding, false if otherwise.
	///	\note		Agnostic to null termination.
	//
	[[nodiscard]] bool UTF8_UNICODE_Compliant(std::u8string_view p_str);

	///	\brief	Checks if string has a valid UTF16 encoding.
	///	\param[in]	p_str - String to check
	///	\param[in]	p_size - Size of string to check
	///	\return true if input has a valid encoding, false if otherwise.
	///	\note		Agnostic to null termination.
	//
	[[nodiscard]] bool UTF16_UNICODE_Compliant(std::u16string_view p_str);

	///	\brief	Checks if string has a valid UCS2 and Unicode encoding.
	///	\param[in]	p_str - String to check
	///	\param[in]	p_size - Size of string to check
	///	\return true if input has a valid encoding, false if otherwise.
	///	\note		Agnostic to null termination.
	//
	[[nodiscard]] bool UCS2_UNICODE_Compliant(std::u16string_view p_str);

	///	\brief	Checks if string has a valid UCS4 (or UTF32) and Unicode encoding.
	///	\param[in]	p_str - String to check
	///	\param[in]	p_size - Size of string to check
	///	\return true if input has a valid encoding, false if otherwise.
	///	\note		Agnostic to null termination.
	//
	[[nodiscard]] bool UCS4_UNICODE_Compliant(std::u32string_view p_str);

	///	\brief	Checks if a given string encodes a valid ASCII sequence.
	///	\param[in]	p_str - String to check
	///	\param[in]	p_size - Size of string to check
	///	\return		true if string is ASCII compliant. false if otherwise.
	///	\note		Agnostic to null termination.
	//
	[[nodiscard]] bool ASCII_Compliant(std::u8string_view p_str);

	///	\brief	Same as \ref core::ASCII_Compliant(const char8_t*, size_t), but for char32_t.
	//
	[[nodiscard]] bool ASCII_Compliant(std::u32string_view p_str);

	///	\brief	Checks if string has a valid UTF8 encoding without it necessarily encoding a valid Unicode code point.
	///	\param[in]	p_str - String to check
	///	\param[in]	p_size - Size of string to check
	///	\return true if input has a valid encoding, false if otherwise.
	///	\note		Agnostic to null termination.
	//
	[[nodiscard]] bool UTF8_valid(std::u8string_view p_str);

	///	\brief	Same as \ref UTF16_UNICODE_Compliant. UTF16 can only encode valid Unicode code points.
	//
	[[nodiscard]] inline bool UTF16_valid(std::u16string_view p_str) { return UTF16_UNICODE_Compliant(p_str); }
}	//namespace core

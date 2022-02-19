//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///		Miscelaneous string utilities.
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

#pragma once

#include <string>
#include <string_view>
#include <span>

namespace core
{
	//======== ======== Evaluation
	
	///	\brief		Converts in-place, the input string to all lower case
	///	\param[in,out]	p_str - String to convert
	///	\param[in]	p_size - Size of string
	///	\note
	///		- If the user does not require the input string to be intact, this function is faster than \ref toLowerCaseX
	///		- Agnostic to null termination.
	///	\warning	ASCII support only
	///	\sa \ref toLowerCaseX \ref toUpperCase
	void toLowerCase(std::span<char8_t> p_str);
	
	///	\brief		Converts in-place, the input string to all upper case.
	///	\param[in,out]	p_str - String to convert
	///	\param[in]	p_size - Size of string
	///	\note
	///		- If the user does not require the input string to be intact, this function is faster than \ref toUpperCaseX
	///		- Agnostic to null termination.
	///	\warning	ASCII support only
	///	\sa \ref toUpperCaseX \ref toLowerCase
	void toUpperCase(std::span<char8_t> p_str);
	
	///	\brief		Converts the input string to all lower case.
	///	\param[in]	p_str - String to convert
	///	\param[in]	p_size - Size of string
	///	\return		All lower case version of string.
	///	\note
	///		- This function requires a copy. If the user does not require the input string to be intact, use \ref toLowerCase instead.
	///		- Agnostic to null termination.
	///	\warning	ASCII support only
	///	\sa \ref toLowerCase \ref toUpperCaseX
	[[nodiscard]] std::u8string toLowerCaseX(std::u8string_view p_str);
	
	///	\brief		Converts the input string to all upper case.
	///	\param[in]	p_str - String to convert
	///	\param[in]	p_size - Size of string
	///	\return		All upper case version of string.
	///	\note
	///		- This function requires a copy. If the user does not require the input string to be intact, use \ref toUpperCase instead.
	///		- Agnostic to null termination.
	///	\warning	ASCII support only
	///	\sa \ref toUpperCase \ref toLowerCaseX
	[[nodiscard]] std::u8string toUpperCaseX(std::u8string_view p_str);
	
	///	\brief		Case insensitive comparison between 2 strings
	///	\param[in]	p_str1 - First string
	///	\param[in]	p_str2 - Second string
	///	\param[in]	p_size - Size of both strings. Note that if strings are of different sizes they are by default not the same.
	///					The user should be able to catch that in advance and not need to call this function.
	///	\return		true if strings are equal except for case.
	///	\note		Agnostic to null termination.
	///	\warning	ASCII support only
	[[nodiscard]] bool compareNoCase(const char8_t* p_str1, const char8_t* p_str2, uintptr_t p_size);
	
	///	\brief STL input version of \ref core::CompareNoCase, with additional size check.
	[[nodiscard]] inline bool compareNoCase(std::u8string_view const p_str1, std::u8string_view const p_str2)
	{
		uintptr_t size = p_str1.size();
		if(size != p_str2.size()) return false;
		return compareNoCase(p_str1.data(), p_str2.data(), size);
	}
	
	///	\brief
	///		Checks if a given input string satisfies string pattern with an * wild card. 
	///
	///	\details
	///		Checks if a given input string can match a string pattern containing asterisks (*),
	///		where the asterisk is a wild-card representing any char sequence.
	///		The comparison is case sensitive.\n
	///		Ex.\n
	///			("SomePattern", "*Pattern") evaluates to true\n
	///			("SomethingElse", "*Pattern") evaluates to false\n
	///
	///	\param[in] p_line - A string to be tested
	///	\param[in] p_star - The pattern to be compared against
	///
	///	\return true if given string fits the pattern, false otherwise
	///	\warning	ASCII support only
	[[nodiscard]] bool string_star_match(std::u8string_view p_line, std::u8string_view p_star);

}	//namespace core

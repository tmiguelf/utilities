//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///		Provides stack trace functionality
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
//======== ======== ======== ======== ======== ======== ======== ========

#pragma once

#include <list>
#include <filesystem>

#include "string/core_os_string.hpp"


namespace core
{

#ifdef _WIN32
///	\brief Generates a mini dump that can be used for debugging
///	\param[in] p_file - Where to output the file
///	\return 0 on success, other for otherwise.
///	\note Windows only
uint8_t generate_minidump(const std::filesystem::path& p_file);
#else
///	\brief Generates a core dump that can be used for debugging. Requires core dumps to be configured correctly on the OS side.
///	\return 0 if triggered (no guarantee of success), other for otherwise.
///	\note Linux only
uint8_t generate_coredump();
#endif


///	\brief Primes the application to output a stack trace on a crash
///
///	\param[in] p_output_file	the name of the output file without extensions
///	\param[in] p_file_extension	the desired extension for the output file
///	\param[in] decorated_file	specifies that the filename should be appended a time-stamp decoration such that multiple files can be kept
///
///	\return true if stack trace was registered successfully
///
///	\bug invalid arguments passed to C runtime function are known to not be captured by this
bool register_crash_trace(const std::filesystem::path& p_output_file);

///	\brief Pairs module name and base address
struct ModuleAddr
{
	uintptr_t	m_addr;	//!< Base address of the loaded module
	//uint64_t	m_size;	//todo
	os_string	m_name;	//!< Module name
};

///	\brief Pairs a stack address to a potential function name
///	\warning Function names may not necessarily be correct. \n
///		Name is determined based on last know name prior to current address, which is not necessarily be the same function the address belongs to.
///		use .map files to confirm names.
struct StackInfo
{
	uintptr_t		m_addr;	//!< Address in the call stack
	std::u8string	m_name;	//!< Name of function (not guaranteed to be correct)
};


///	\brief Adds information about the base of the corresponding module to \ref StackInfo
struct StackBaseInfo: public StackInfo
{
	uintptr_t m_modBase;	//!< Module base address
};

///	\brief Used to list all loaded modules and full stack
///	\warning Function names may not necessarily be correct. \n
///		Name is determined based on last know name prior to current address, which is not necessarily the same function as the one the address belongs to.
///		Use .map files to confirm names.
struct StackTrace_FullInfo
{
	std::list<ModuleAddr>		m_modules;	//!< Loaded modules
	std::list<StackBaseInfo>	m_stack;	//!< Stack trace
};

///	\brief Used to list all loaded modules and full stack
///	\param[out] p_list - list to be filled with \ref ModuleAddr data
///	\return 0 on success, other if otherwise
///	\warning Function names may not necessarily be correct. \n
///		Name is determined based on last know name prior to current address, which is not necessarily the same function as the one the address belongs to.
///		Use .map files to confirm names.
uint8_t list_modules(std::list<ModuleAddr>& p_list);

///	\brief Generates stack trace with full module context
///	\param[out] p_trace - \ref StackTrace_FullInfo to be filled
///	\return 0 on success, other if otherwise
///	\warning Function names may not necessarily be correct. \n
///		Name is determined based on last know name prior to current address, which is not necessarily the same function as the one the address belongs to.
///		Use .map files to confirm names.
uint8_t stack_trace(StackTrace_FullInfo& p_trace);

///	\brief Generates stack trace with module base addresses for entries in the stack
///	\param[out] p_trace - list to be filed with \ref StackBaseInfo data
///	\return 0 on success, other if otherwise
///	\warning Function names may not necessarily be correct. \n
///		Name is determined based on last know name prior to current address, which is not necessarily the same function as the one the address belongs to.
///		Use .map files to confirm names.
uint8_t stack_trace(std::list<StackBaseInfo>& p_trace);

///	\brief Generates stack trace with only function names and addresses
///	\param[out] p_trace - list to be filled with \ref StackInfo data
///	\return 0 on success, other if otherwise
///	\warning Function names may not necessarily be correct. \n
///		Name is determined based on last know name prior to current address, which is not necessarily the same function as the one the address belongs to.
///		Use .map files to confirm names.
uint8_t stack_trace(std::list<StackInfo>& p_trace);

///	\brief Generates stack trace with only addresses
///	\param[out] p_trace - list of addresses to be filled
///	\return 0 on success, other if otherwise
uint8_t stack_trace(std::list<uintptr_t>& p_trace);

} //namespace core

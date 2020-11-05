//======== ======== ======== ======== ======== ======== ======== ========
///	\file
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

#include "CoreLib/Core_OS.hpp"
#if	defined(_WIN32) //OS
#	include <windows.h>
#else
#	include <unistd.h>
#	include <sys/stat.h>
#endif

namespace core
{

#ifdef _WIN32
bool env_exists(const core::os_string& p_key)
{
	return GetEnvironmentVariableW(reinterpret_cast<const wchar_t*>(p_key.c_str()), nullptr, 0) != 0;
}

bool set_env(const core::os_string& p_key, const core::os_string& p_value)
{
	return (SetEnvironmentVariableW(reinterpret_cast<const wchar_t*>(p_key.c_str()), reinterpret_cast<const wchar_t*>(p_value.c_str())) != FALSE);
}

env_result get_env(const core::os_string& p_key)
{
	DWORD size;
	size = GetEnvironmentVariableW(reinterpret_cast<const wchar_t*>(p_key.c_str()), nullptr, 0);
	if(size == 0)
	{
		return false;
	}
	std::u16string outp;
	outp.resize(size - 1);
	if(GetEnvironmentVariableW(reinterpret_cast<const wchar_t*>(p_key.c_str()), reinterpret_cast<wchar_t*>(outp.data()), size) == (size - 1))
	{
		return core::os_string{std::move(outp)};
	}
	return false;
}

bool delete_env(const core::os_string& p_key)
{
	return SetEnvironmentVariableW(reinterpret_cast<const wchar_t*>(p_key.c_str()), L"") != FALSE;
}

env_result machine_name()
{
	char16_t buff[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
	if(GetComputerNameW(reinterpret_cast<wchar_t*>(buff), &size))
	{
		return core::os_string{buff, static_cast<uintptr_t>(size)};
	}

	return false;
}

std::filesystem::path applicationPath()
{
	// TODO: There's got to be a better algorithm to do this
	constexpr DWORD max_pathSize = 32767 + 1; //https://docs.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation
	DWORD pathSize = 256;
	std::wstring buff;
	do
	{
		buff.resize(pathSize);
		DWORD res = GetModuleFileNameW(nullptr, buff.data(), pathSize);
		if(res == 0) return {};
		if(res < pathSize)
		{
			buff.resize(res);
			return {buff};
		}
		pathSize *= 2;
	}
	while(pathSize < max_pathSize);

	buff.resize(max_pathSize);
	DWORD res = GetModuleFileNameW(nullptr, buff.data(), max_pathSize);
	if(res == 0) return {};
	if(res < max_pathSize)
	{
		buff.resize(res);
		return {buff};
	}
	return {};
}

#else
bool env_exists(const core::os_string& p_key)
{
	return getenv(reinterpret_cast<const char*>(p_key.c_str())) != nullptr;
}

bool set_env(const core::os_string& p_key, const core::os_string& p_value)
{
	return setenv(reinterpret_cast<const char*>(p_key.c_str()), reinterpret_cast<const char*>(p_value.c_str()), 1) == 0;
}

env_result get_env(const core::os_string& p_key)
{
	const char8_t* const retvar = reinterpret_cast<const char8_t*>(getenv(reinterpret_cast<const char*>(p_key.c_str())));
	if(retvar == nullptr)
	{
		return false;
	}
	return core::os_string{std::u8string{retvar}};
}

bool delete_env(const core::os_string& p_key)
{
	return unsetenv(reinterpret_cast<const char*>(p_key.c_str())) == 0;
}

env_result machine_name()
{
	constexpr uintptr_t host_name_max = 64; //https://man7.org/linux/man-pages/man2/gethostname.2.html
	char8_t buff[host_name_max + 1];

	if(gethostname(reinterpret_cast<char*>(buff), host_name_max + 1) == 0)
	{
		return core::os_string{std::u8string{buff}};
	}

	return false;
}

std::filesystem::path applicationPath()
{
	struct stat stats;
	if(lstat("/proc/self/exe", &stats) == 0)
	{
		ssize_t size = stats.st_size;
		std::string buff;
		buff.resize(size);

		ssize_t ret_size = readlink("/proc/self/exe", buff.data(), size);

		if(ret_size == size)
		{
			return buff;
		}
	}
	return {};
}
#endif

} //namespace core

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

#include <CoreLib/core_os.hpp>

#include <array>

#if	defined(_WIN32) //OS
#	include <windows.h>
#else
#	include <unistd.h>
#	include <climits>
#	include <sys/stat.h>
#endif

namespace core
{

#ifdef _WIN32
bool env_exists(core::os_string const& p_key)
{
	return GetEnvironmentVariableW(p_key.c_str(), nullptr, 0) != 0;
}

bool set_env(core::os_string const& p_key, core::os_string const& p_value)
{
	return (SetEnvironmentVariableW(p_key.c_str(), p_value.c_str()) != FALSE);
}

std::optional<core::os_string> get_env(core::os_string const& p_key)
{
	DWORD const size = GetEnvironmentVariableW(p_key.c_str(), nullptr, 0);
	if(size == 0)
	{
		return {};
	}
	os_string outp;
	outp.resize(size - 1);
	if(GetEnvironmentVariableW(p_key.c_str(), outp.data(), size) == (size - 1))
	{
		return core::os_string{std::move(outp)};
	}
	return {};
}

bool delete_env(core::os_string const& p_key)
{
	return SetEnvironmentVariableW(reinterpret_cast<wchar_t const*>(p_key.c_str()), L"") != FALSE;
}

std::optional<core::os_string> machine_name()
{
	constexpr uintptr_t maxSize = MAX_COMPUTERNAME_LENGTH + 1;
	std::array<os_char, maxSize> buff;
	DWORD size = maxSize;
	if(GetComputerNameW(buff.data(), &size))
	{
		return core::os_string{buff.data(), static_cast<uintptr_t>(size)};
	}

	return {};
}

std::filesystem::path application_path()
{
	constexpr DWORD max_pathSize = 32767 + 255 + 1; //https://docs.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation
	std::array<wchar_t, max_pathSize> buff;
	DWORD res = GetModuleFileNameW(nullptr, buff.data(), max_pathSize);
	if(res == 0) return {};

	if(res < max_pathSize)
	{
		return std::wstring{buff.data(), res};
	}

	return {};
}

static void __cdecl noop_invalid_parameter_handler(wchar_t const*, wchar_t const*, wchar_t const*, unsigned int, uintptr_t){}
void disable_critical_invalid_c_param()
{
	_set_invalid_parameter_handler(noop_invalid_parameter_handler);
}

#else
bool env_exists(core::os_string const& p_key)
{
	return getenv(p_key.c_str()) != nullptr;
}

bool set_env(core::os_string const& p_key, core::os_string const& p_value)
{
	return setenv(p_key.c_str(), p_value.c_str(), 1) == 0;
}

std::optional<core::os_string> get_env(core::os_string const& p_key)
{
	char const* const retvar = getenv(p_key.c_str());
	if(retvar == nullptr)
	{
		return {};
	}
	return core::os_string{retvar};
}

bool delete_env(core::os_string const& p_key)
{
	return unsetenv(p_key.c_str()) == 0;
}

std::optional<core::os_string> machine_name()
{
	constexpr uintptr_t host_name_max = 64; //https://man7.org/linux/man-pages/man2/gethostname.2.html
	std::array<os_char, host_name_max + 1> buff;

	if(gethostname(buff.data(), buff.size()) == 0)
	{
		return core::os_string{buff.data()};
	}

	return {};
}

std::filesystem::path application_path()
{
	std::array<char, PATH_MAX> buff;
	ssize_t ret_size = readlink("/proc/self/exe", buff.data(), buff.size());

	if(ret_size > 0)
	{
		return std::string{buff.data(), static_cast<uintptr_t>(ret_size)};
	}

	return {};
}
#endif

} //namespace core

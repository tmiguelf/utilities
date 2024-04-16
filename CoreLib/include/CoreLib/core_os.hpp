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

#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>

#include "string/core_os_string.hpp"

namespace core
{

bool env_exists	(core::os_string const& p_key);
bool set_env	(core::os_string const& p_key, core::os_string const& p_value);
bool delete_env	(core::os_string const& p_key);
std::optional<core::os_string>	get_env		(core::os_string const& p_key);
std::optional<core::os_string>	machine_name();

std::filesystem::path application_path();

inline std::filesystem::path to_absolute_lexical(std::filesystem::path const& p_path, std::filesystem::path const& p_base)
{
	if(p_path.is_absolute())
	{
		return p_path.lexically_normal();
	}
	else
	{
		return (p_base / p_path).lexically_normal();
	}
}


#ifdef _WIN32
void disable_critical_invalid_c_param();
#endif

} //namespace core

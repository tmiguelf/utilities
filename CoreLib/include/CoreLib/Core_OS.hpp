//======== ======== ======== ======== ======== ======== ======== ========
///	\file
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

#include <cstdint>
#include <filesystem>
#include <optional>

#include "string/core_os_string.hpp"

namespace core
{

bool env_exists	(const core::os_string& p_key);
bool set_env	(const core::os_string& p_key, const core::os_string& p_value);
bool delete_env	(const core::os_string& p_key);
std::optional<core::os_string>	get_env		(const core::os_string& p_key);
std::optional<core::os_string>	machine_name();

std::filesystem::path application_path();

inline std::filesystem::path to_absolute_lexical(const std::filesystem::path& p_path, const std::filesystem::path& p_base)
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

//======== ======== ======== ======== ======== ======== ======== ========
///	\file
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

#include <cstdint>
#include <filesystem>

#include "Core_Alternate.hpp"
#include "string/core_os_string.hpp"
#include "string/core_string_tostream.hpp"
#include "string/core_string_encoding.hpp"


namespace core
{

using env_result = alternate<core::os_string, bool, true, false>;

bool		env_exists	(const core::os_string& p_key);
bool		set_env		(const core::os_string& p_key, const core::os_string& p_value);
env_result	get_env		(const core::os_string& p_key);
bool		delete_env	(const core::os_string& p_key);

env_result	machine_name();

std::filesystem::path application_path();

std::filesystem::path to_absolute(const std::filesystem::path& p_path, const std::filesystem::path& p_base = std::filesystem::path{});

template<>
class toStream<std::filesystem::path>
{
public:
	toStream(const std::filesystem::path& p_data): m_data{p_data}{}
	inline void stream(std::ostream& p_stream) const
	{
#ifdef _WIN32
		const std::wstring& native = m_data.native();
		const std::u8string res = UTF16_to_UTF8_faulty({reinterpret_cast<const char16_t*>(native.data()), native.size()}, '?');
		p_stream.write(reinterpret_cast<const char*>(res.data()), res.size());
#else
		const std::string& native = m_data.native();
		p_stream.write(native.data(), native.size());
#endif
	}

private:
	const std::filesystem::path& m_data;
};

} //namespace core

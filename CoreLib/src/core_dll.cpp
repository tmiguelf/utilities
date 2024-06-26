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

#include <CoreLib/core_dll.hpp>
#include <CoreLib/core_alloca.hpp>

#ifdef _WIN32
#	include <windows.h>
#else
#	include <dlfcn.h>
#	include <cstring>
#endif

namespace core
{

#ifdef _WIN32
dll::~dll()
{
	if(handle)
	{
		FreeLibrary(reinterpret_cast<HMODULE>(handle));
	}
}

dll::Error dll::load(std::filesystem::path const& p_path, dll::native_attr* const p_attr)
{
	unload();

	if(p_attr)
	{
		handle = LoadLibraryExW(p_path.c_str(), nullptr, p_attr->Flags);
	}
	else
	{
		handle = LoadLibraryW(p_path.c_str());
	}
	if(handle)
	{
		return Error::no_error;
	}
	return Error::failed;
}

void dll::unload()
{
	if(handle)
	{
		FreeLibrary(reinterpret_cast<HMODULE>(handle));
		handle = nullptr;
	}
}

void* dll::resolve(std::u8string_view const p_name) const
{
	if(handle && !p_name.empty())
	{
		// :(
		//null terminated string are bad, but this function requires one
		//hopefully symbol names are small enough to fit on the stack
		//if they are not, then sorry my friend, you have bigger problems
		uintptr_t const size = p_name.size();
		char* const buff = reinterpret_cast<char*>(core_alloca(size + 1));
		memcpy(buff, p_name.data(), size);
		buff[size] = 0;
		return (void*) GetProcAddress(reinterpret_cast<HMODULE>(handle), buff);
	}
	return nullptr;
}
#else

dll::~dll()
{
	if(handle)
	{
		dlclose(handle);
	}
}

dll::Error dll::load(std::filesystem::path const& p_path, dll::native_attr* const p_attr)
{
	unload();

	constexpr int defaultFlags = RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND;

	if(p_path.empty())
	{
		if(p_attr)
		{
			handle = dlopen(nullptr, p_attr->Flags);
		}
		else
		{
			handle = dlopen(nullptr, defaultFlags);
		}
	}
	else
	{
		if(p_attr)
		{
			handle = dlopen(p_path.c_str(), p_attr->Flags);
		}
		else
		{
			handle = dlopen(p_path.c_str(), defaultFlags);
		}
	}
	if(handle)
	{
		return Error::no_error;
	}
	return Error::failed;
}


void dll::unload()
{
	if(handle)
	{
		dlclose(handle);
		handle = nullptr;
	}
}

void* dll::resolve(std::u8string_view const p_name) const
{
	if(handle)
	{
		// :(
		//null terminated string are bad, but this function requires one
		//hopefully symbol names are small enough to fit on the stack
		//if they are not, then sorry my friend, you have bigger problems
		uintptr_t const size = p_name.size();
		char* const buff = reinterpret_cast<char*>(core_alloca(size + 1));
		memcpy(buff, p_name.data(), size);
		buff[size] = 0;
		return dlsym(handle, buff);
	}
	return nullptr;
}
#endif

} //namespace core

//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///		Provides utilities to link dynamic libraries/binaries (.dll .so)
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

#include <string_view>
#include <filesystem>

namespace core
{

/// \brief	used to encapsulate a library loaded explicitly
///			object must remain in scope for the entire duration the module is used
///			otherwise module is automatically unloaded and all pointers previously returned
///			become invalid
///
///	\note
///		- On Windows it encapsulates LoadLibraryEx and LoadLibrary
///		- On Linux it encapsulates dlopen
class dll
{
public:
	//TODO: Should provid a more extensive error codes in the future

	enum Error: uint8_t
	{
		no_error = 0,
		failed
	};

private:
	void* handle = nullptr;

public:
	struct native_attr
	{
	#ifdef _WIN32
		uint32_t Flags;
	#else
		int Flags;
	#endif
	};

	~dll();

	inline constexpr bool is_loaded() const { return handle != nullptr; }

	///	\brief	loads a library,if this object was use previously to load a library
	///			it unloads the previous library first
	///
	///	\param	p_name		path of the library with the exact file extension to load
	///	\param	Native_attr	see manual for LoadLibraryEx on Windows, or dlopen on Linux for flag codes
	///
	///	\note
	///		- On Windows it encapsulates LoadLibraryExW and LoadLibraryW
	///		- On Linux it encapsulates dlopen
	Error load(std::filesystem::path const& p_path, native_attr* p_attr = nullptr);

	///	\brief	unloads a module previously loaded by this object
	void unload();

	///	\brief	retrieves to the specified symbol from the module
	///
	///	\p_name	the name of the symbol
	///
	///	\return		the pointer to the symbol if the symbol was found, nullptr if symbol was not found
	///
	///	\note
	///		- On Windows it encapsulates GetProcAddress
	///		- On Linux it encapsulates dlsym
	[[nodiscard]] void* resolve(std::u8string_view p_name) const;
};

} //namespace core

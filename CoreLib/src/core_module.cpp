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

#include <CoreLib/core_module.hpp>
#include <CoreLib/core_os.hpp>
#ifdef _WIN32
#include <Windows.h>
#include <filesystem>
#include <array>
#else
#include <dlfcn.h>
#include <link.h>
#endif

namespace core
{

	namespace
	{
		struct ModuleDataRetriever
		{
		public:
#ifdef _WIN32
			ModuleDataRetriever()
			{
				HMODULE mod_addr;
				if(GetModuleHandleExW(
					GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
					reinterpret_cast<wchar_t*>(get_current_module_base), &mod_addr))
				{
					module_addr = mod_addr;
					std::filesystem::path temp;
					{
						std::array<wchar_t, 32767> data;
						DWORD const ret_size = GetModuleFileNameW(mod_addr, data.data(), static_cast<DWORD>(data.size()));
						if(ret_size && (ret_size < data.size()))
						{
							temp = std::filesystem::path{data.data(), data.data() + ret_size};
						}
						else
						{
							return;
						}
					}
					module_name = temp.lexically_normal();
					short_name = module_name.filename();
				}
				else
				{
					module_addr = nullptr;
				}
			}
#else
			ModuleDataRetriever()
			{
				Dl_info t_info;

				void const * const addr = reinterpret_cast<void const * const>(get_current_module_base);
				if(dladdr(addr, &t_info) && (t_info.dli_fbase < addr))
				{
					module_addr = t_info.dli_fbase;
					std::filesystem::path temp{t_info.dli_fname};
					module_name = temp.lexically_normal();
					short_name = module_name.filename();
				}
				else
				{
					module_addr = nullptr;
				}

			}
#endif

		public:
			void const* module_addr;
			std::filesystem::path module_name;
			std::filesystem::path short_name;
		};

	} //namespace


static ModuleDataRetriever const g_module_data;

void const* get_current_module_base()
{
	return g_module_data.module_addr;
}

os_string_view get_current_module_name()
{
	return g_module_data.module_name.native();
}

os_string_view get_current_module_short_name()
{
	return g_module_data.short_name.native();
}

}

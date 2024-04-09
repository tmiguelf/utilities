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

#include <CoreLib/core_stacktrace.hpp>

#include <utility>

#ifdef _WIN32
#	include <Windows.h>
#	include <Dbghelp.h>
#else
#	include <execinfo.h>
#	include <signal.h>
#	include <ucontext.h>
#	include <unistd.h>
#	include <link.h>
#	include <cstring>
#	include <sys/utsname.h>
#	include <CoreLib/Core_extra_compiler.hpp>
#endif

#include <CoreLib/Core_OS.hpp>
#include <CoreLib/Core_Time.hpp>
#include <CoreLib/Core_File.hpp>
#include <CoreLib/Core_cpu.hpp>

#include <CoreLib/toPrint/toPrint.hpp>
#include <CoreLib/toPrint/toPrint_filesystem.hpp>
#include <CoreLib/toPrint/toPrint_file.hpp>

#include <CoreLib/string/core_string_encoding.hpp>
#include <CoreLib/string/core_string_numeric.hpp>


namespace core
{

#define OUTPUT(Sink, ...)  core::print<char8_t>(sink_file_UTF8_unlocked(Sink) __VA_OPT__(,) __VA_ARGS__)

namespace
{
	class toPrint_fix_2: public toPrint_base
	{
	public:
		toPrint_fix_2(const uint8_t p_data): m_data(p_data) {}

		static inline constexpr uintptr_t size(const char8_t&) { return 2; }

		void get_print(char8_t* p_out) const
		{
			if(m_data < 9)
			{
				*(p_out++) = u8'0';
				*(p_out) = u8'0' + m_data;
			}
			else
			{
				*(p_out++) = u8'0' + m_data / 10;
				*(p_out) = u8'0' + m_data % 10;
			}
		}

	private:
		const uint8_t m_data;
	};

	class toPrint_fix_3: public toPrint_base
	{
	public:
		toPrint_fix_3(const uint16_t p_data): m_data(p_data) {}

		static inline constexpr uintptr_t size(const char8_t&) { return 3; }

		void get_print(char8_t* p_out) const
		{
			if(m_data < 100)
			{
				*(p_out++) = u8'0';
				if(m_data < 10)
				{
					*(p_out++) = u8'0';
					*(p_out) = static_cast<char8_t>(u8'0' + m_data);
				}
				else
				{
					*(p_out++) = static_cast<char8_t>(u8'0' + m_data / 10);
					*(p_out) = static_cast<char8_t>(u8'0' + m_data % 10);
				}
			}
			else
			{
				*(p_out++) = static_cast<char8_t>(u8'0' + m_data / 100);
				const char8_t rem = static_cast<char8_t>(m_data % 100);
				*(p_out++) = u8'0' + rem / 10;
				*(p_out) = u8'0' + rem % 10;
			}
		}

	private:
		const uint16_t m_data;
	};


	static void PrintCPUinfo(file_write& p_file)
	{
#if defined(_M_AMD64) || defined(__amd64__)
		OUTPUT(p_file, "CPU: AMD64\n"sv);
		core::amd64::EX_Reg reg = core::amd64::CPU_feature_su::Fn0();
		const uint32_t maxId = reg.eax;
		OUTPUT(p_file, "FN0: "sv, toPrint_hex_fix{reg.eax}, ' ', toPrint_hex_fix{reg.ebx}, ' ', toPrint_hex_fix{reg.ecx}, ' ', toPrint_hex_fix{reg.edx}, '\n');

		if(maxId > 0)
		{
			reg = core::amd64::CPU_feature_su::Fn1();
			OUTPUT(p_file, "FN1:          "sv, toPrint_hex_fix{reg.ebx}, "          "sv, toPrint_hex_fix{reg.edx}, '\n');
			if(maxId > 6)
			{
				reg = core::amd64::CPU_feature_su::Fn7();
				OUTPUT(p_file, "FN7:          "sv, toPrint_hex_fix{reg.ebx}, ' ', toPrint_hex_fix{reg.ecx}, ' ', toPrint_hex_fix{reg.edx}, '\n');
			}
		}

#endif // AMD64
	}

} //namespace
} //namespace core

#ifdef _WIN32


#ifdef EnumerateLoadedModules64
#	undef EnumerateLoadedModules64
#endif

//static constexpr DWORD SYMBOL_OPTIONS = (SYMOPT_DEFERRED_LOADS | SYMOPT_FAIL_CRITICAL_ERRORS | SYMOPT_INCLUDE_32BIT_MODULES | SYMOPT_LOAD_LINES | SYMOPT_NO_PROMPTS);
static constexpr DWORD SYMBOL_OPTIONS = (SYMOPT_DEFERRED_LOADS | SYMOPT_FAIL_CRITICAL_ERRORS | SYMOPT_INCLUDE_32BIT_MODULES | SYMOPT_NO_PROMPTS);

namespace core
{

namespace
{

	///	\brief Used to store global sack trace options
	class Stack_Trace_Options
	{
	public:
		std::filesystem::path m_output_file;
	};

	static Stack_Trace_Options g_straceOpt;


	///	\brief Used to pass the process handle and the file we are outputting to to the module enumeration call back.
	struct enumerate2file_context
	{
		const HANDLE	m_proc;	//!< Process handle
		file_write*		m_file;	//!< Output file
	};

	///	\brief Used to pass the process handle and the list we are outputting to to the module enumeration call back.
	struct enumerate2list_context
	{
		const HANDLE			m_proc;	//!< Process handle
		std::list<ModuleAddr>*	m_list;	//!< output list
	};

	///	\brief Used to store the data for the module. The default structure is a prototype where
	///		the name has 0 bytes, we are supposed to define this to allocate space for the name
	///		make sure to correctly initialize the members after.
	struct core_symb
	{

		DWORD	SizeOfStruct;	//!< Set to sizeof(IMAGEHLP_SYMBOL)
		DWORD64	Address;		//!< Virtual address including dll base address
		DWORD	Size;			//!< Estimated size of symbol, can be zero
		DWORD	Flags;			//!< Info about the symbols, see the SYMF defines
		DWORD	MaxNameLength;	//!< Maximum size of symbol name in 'Name'
		CHAR	Name[255];		//!< Symbol name (null terminated string)
	};

	///	\brief	Used to generate the correct set of flags such that most of the necessary information is output
	///			yet still be able to succeed the function call no matter the version actually present.
	static MINIDUMP_TYPE GenerateMinidumpFlags()
	{
		API_VERSION* t_version = ImagehlpApiVersion();

		constexpr MINIDUMP_TYPE t_flags = MINIDUMP_TYPE( 0
								| MiniDumpWithDataSegs
								//| MiniDumpWithFullMemory
								| MiniDumpWithHandleData
								//| MiniDumpFilterMemory
								//| MiniDumpScanMemory
								| MiniDumpWithFullAuxiliaryState
								);

		constexpr MINIDUMP_TYPE t_5_2_flags = MINIDUMP_TYPE( t_flags
									| MiniDumpWithIndirectlyReferencedMemory
									| MiniDumpWithProcessThreadData
									//| MiniDumpWithPrivateReadWriteMemory
									);

		constexpr MINIDUMP_TYPE t_6_2_flags = MINIDUMP_TYPE( t_5_2_flags
									//| MiniDumpWithFullMemoryInfo
									//| MiniDumpWithThreadInfo
									| MiniDumpWithCodeSegs
									//| 0x00010000
									//| 0x00040000
									//| 0x00080000
									);

		if(t_version->MajorVersion >= 6)
		{
			if(t_version->MajorVersion > 6 || t_version->MinorVersion > 1)
			{
				return t_6_2_flags;
			}
			return t_5_2_flags;
		}
		else if(t_version->MajorVersion == 5 && t_version->MinorVersion > 1)
		{
			return t_5_2_flags;
		}
		
		return t_flags;
	}

	/// \brief Auxiliary function used to print operating system version
	static void PrintOS(file_write& p_file)
	{
#if 1
		using fsig =  DWORD (__stdcall*)(PRTL_OSVERSIONINFOW);

		HMODULE hDll = LoadLibraryW(L"Ntdll.dll");
		if(hDll)
		{
			fsig pRtlGetVersion = reinterpret_cast<fsig>(GetProcAddress(hDll, "RtlGetVersion"));
			if(pRtlGetVersion != nullptr)
			{
				RTL_OSVERSIONINFOEXW info;
				info.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);
				if(!pRtlGetVersion(reinterpret_cast<RTL_OSVERSIONINFOW*>(&info)))
				{
					OUTPUT(p_file, "OS:          Windows "sv,
						info.dwMajorVersion, '.', info.dwMinorVersion, " ("sv, info.wProductType, '/', toPrint_hex_fix(info.wSuiteMask), ") build "sv, info.dwBuildNumber);
					if(*info.szCSDVersion)
					{
						OUTPUT(p_file, ' ', std::wstring_view{info.szCSDVersion});
					}
					OUTPUT(p_file, '\n');

					CloseHandle(hDll);
					return;
				}
			}
			CloseHandle(hDll);
		}
		OUTPUT(p_file, "OS:          Windows\n"sv);
#else
		OSVERSIONINFOEXW info;
		info.dwOSVersionInfoSize = sizeof(decltype(info));
		if(GetVersionExW(reinterpret_cast<OSVERSIONINFOW*>(&info)))
		{
			OUTPUT(p_file, "OS:          Windows "sv,
				info.dwMajorVersion, '.', info.dwMinorVersion, " ("sv, info.wProductType, '/', toPrint_hex_fix(info.wSuiteMask), ") build "sv, info.dwBuildNumber);
			if(*info.szCSDVersion)
			{
				OUTPUT(p_file, ' ', std::wstring_view{info.szCSDVersion});
			}
			OUTPUT(p_file, '\n');
		}
		else
		{
			OUTPUT(p_file, "OS:          Windows\n"sv);
		}
#endif
	}

	/// \brief Auxiliary function used to print environment variables
	static void PrintEnv(file_write& p_file)
	{
		wchar_t* const t_base = GetEnvironmentStringsW();
		const wchar_t* t_pivot = t_base;
		if(t_base)
		{
			while(*t_pivot)
			{
				core::os_string_view data{t_pivot};
				OUTPUT(p_file, data, '\n');
				t_pivot += data.size() + 1;
			}
			FreeEnvironmentStringsW(t_base);
		}
	}

	/// \brief Auxiliary function used to enumerate modules and print onto file
	static BOOL CALLBACK EnumerateModules2File(PCWSTR const ModuleName, const DWORD64 BaseOfDll, const ULONG ModuleSize, PVOID const UserContext)
	{
		OUTPUT(*(reinterpret_cast<enumerate2file_context*>(UserContext)->m_file),
			"0x"sv, toPrint_hex_fix{BaseOfDll}, " 0x"sv, toPrint_hex_fix{BaseOfDll + ModuleSize},
			" \""sv, os_string_view{ModuleName}, "\"\n"sv);
		return TRUE;
	}

	/// \brief Auxiliary function used to enumerate modules onto list
	//
	static BOOL CALLBACK EnumerateModules2List(PCWSTR const ModuleName, const DWORD64 BaseOfDll, const ULONG ModuleSize, PVOID const UserContext)
	{
		ModuleAddr t_entry;
		enumerate2list_context* const t_context = reinterpret_cast<enumerate2list_context*>(UserContext);
		
		t_entry.m_addr = static_cast<uintptr_t>(BaseOfDll);
		t_entry.m_size = static_cast<uintptr_t>(ModuleSize);
		t_entry.m_name = ModuleName;

		t_context->m_list->push_back(std::move(t_entry)); //why the hell the standard didn't have in place construction from the start??
		return TRUE;
	}

	///	\brief This function prints 1 stack addresses and associated function name,
	///		first it pretyfies the address to make it more easy to correlated with the module it has been loaded from,
	///		secondly it tries to get the name of the symbol to which this address belongs too.
	///	\warning Function names may not necessarily be correct. \n
	///		Name is determined based on last know name prior to current address, which is not necessarily the same function as the one the address belongs to.
	///		Use .map files to confirm names.
	static void print_function_addr(HANDLE const p_proc, DWORD64 const p_address, file_write& p_file)
	{
		core_symb t_symb;
		DWORD64 displacement = 0;
		DWORD64 base;

		t_symb.SizeOfStruct = sizeof(core_symb);
		t_symb.MaxNameLength = 254;
		t_symb.Name[0] = '\0';

		//This gets the base address of the module that the input address belongs too
		//fortunately we had the good sense to also print the addresses of the modules and their names
		//which makes it easier to make sense of this data
		//see SymGetModuleBase64 on msdn: https://msdn.microsoft.com/en-us/library/windows/desktop/ms681335(v=vs.85).aspx
		base = SymGetModuleBase64(p_proc, p_address);
		if(base && (base <= p_address))
		{
			//Prints the module address + offset inside the module example: 0x0000123400+567
			OUTPUT(p_file, "0x"sv, toPrint_hex_fix{base}, '+', toPrint_hex{p_address - base});
		}
		else
		{	//in theory this should never be the case, this is just in case SymGetModuleBase64 fails, or it produces
			//data that makes no sense, you may not be able to accurately correlated it to the module, but at least you have something
			OUTPUT(p_file, "0x"sv, toPrint_hex_fix{base});
		}

		//======== IMPORTANT ======== 
		//this gets the name of the symbol (function name) the input address belongs too
		//see SymGetSymFromAddr64 on msdn: https://msdn.microsoft.com/en-us/library/windows/desktop/ms681335(v=vs.85).aspx
		//Note: It is common for most of the symbols names not to be found on release builds, the .pdb file helps with that
		if(SymGetSymFromAddr64(p_proc, p_address, &displacement, reinterpret_cast<IMAGEHLP_SYMBOL64*>(&t_symb)) == TRUE)
		{
			if(t_symb.Name[0] !='\0')
			{
				OUTPUT(p_file, ' ', std::string_view{t_symb.Name});
			}
		}
		//======== END ======== 
		
		OUTPUT(p_file, '\n');
	}

	///	\brief This function pushes onto a list 1 stack addresses and associated function name, plus the base address of the corresponding module.
	///	\warning Function names may not necessarily be correct. \n
	///		Name is determined based on last know name prior to current address, which is not necessarily the same function as the one the address belongs to.
	///		Use .map files to confirm names.
	static void AppendStackAddr2List(HANDLE const p_proc, uint64_t const p_addr, std::list<StackBaseInfo>& p_list)
	{
		StackBaseInfo t_info;
	
		core_symb t_symb;
		DWORD64 displacement = 0; //always 0
		
		t_symb.SizeOfStruct		= sizeof(core_symb);
		t_symb.MaxNameLength	= 254;
		t_symb.Name[0]			= '\0';
		
		t_info.m_addr			= static_cast<uintptr_t>(p_addr);
		
		//This gets the base address of the module that the input address belongs too
		//fortunately we had the good sense to also print the addresses of the modules and their names
		//which makes it easier to make sense of this data
		//see SymGetModuleBase64 on msdn: https://msdn.microsoft.com/en-us/library/windows/desktop/ms681335(v=vs.85).aspx
		t_info.m_modBase		= static_cast<uintptr_t>(SymGetModuleBase64(p_proc, p_addr));

		//======== IMPORTANT ======== 
		//this gets the name of the symbol (function name) the input address belongs too
		//see SymGetSymFromAddr64 on msdn: https://msdn.microsoft.com/en-us/library/windows/desktop/ms681335(v=vs.85).aspx
		//Note: It is common for most of the symbols names not to be found on release builds, the .pdb file helps with that
		if(SymGetSymFromAddr64(p_proc, p_addr, &displacement, reinterpret_cast<IMAGEHLP_SYMBOL64*>(&t_symb)))
		{
			t_info.m_name = reinterpret_cast<const char8_t*>(t_symb.Name);
		}
		//======== END ======== 

		p_list.push_back(t_info);
	}

	///	\brief This function pushes onto a list 1 stack addresses and associated function name.
	///	\warning Function names may not necessarily be correct. \n
	///		Name is determined based on last know name prior to current address, which is not necessarily the same function as the one the address belongs to.
	///		Use .map files to confirm names.
	static void AppendStackAddr2List(HANDLE const p_proc, const uint64_t p_addr, std::list<StackInfo>& p_list)
	{
		StackInfo t_info;
	
		core_symb t_symb;
		DWORD64 displacement = 0; //always 0
		
		t_symb.SizeOfStruct		= sizeof(core_symb);
		t_symb.MaxNameLength	= 254;
		t_symb.Name[0]			= '\0';
		
		t_info.m_addr		= static_cast<uintptr_t>(p_addr);

		//======== IMPORTANT ======== 
		//this gets the name of the symbol (function name) the input address belongs too
		//see SymGetSymFromAddr64 on msdn: https://msdn.microsoft.com/en-us/library/windows/desktop/ms681335(v=vs.85).aspx
		//Note: It is common for most of the symbols names not to be found on release builds, the .pdb file helps with that
		if(SymGetSymFromAddr64(p_proc, p_addr, &displacement, reinterpret_cast<IMAGEHLP_SYMBOL64*>(&t_symb)))
		{
			t_info.m_name = reinterpret_cast<const char8_t*>(t_symb.Name);
		}
		//======== END ======== 
		
		p_list.push_back(t_info);
	}

	///	\brief	This function performs all tasks related to printing the stack trace information.
	///	\details
	///			If the stack trace service is registered, the system will call this function when a critical error ocurs.
	///			allowing the opportunity to this function to print:
	///			- Date and time of event
	///			- Exception code and critical address
	///			- Name and addresses of loaded modules
	///			- Stack trace with relative addresses and associated names (not guaranteed to be correct)
	///			- Process ID and thread ID
	///			- Application directory and working directory
	///			- Machine name
	///			- Number of open handles
	///			- Environment variables
	///	\note Windows version
	static LONG WINAPI WIN_exception_handler(EXCEPTION_POINTERS* const ExceptionInfo)
	{
		if(!g_straceOpt.m_output_file.empty())
		{
			date_time t_time;
			date_time_local(t_time);	//current date

			file_write o_file;		// file to be output
			{
				std::error_code ec;
				std::filesystem::create_directories(g_straceOpt.m_output_file.parent_path(), ec);
				o_file.open(g_straceOpt.m_output_file, file_write::open_mode::create);
			}
			if(o_file.is_open())
			{
				constexpr std::array UTF8_BOM = {char8_t{0xEF}, char8_t{0xBB}, char8_t{0xBF}};
				o_file.write_unlocked(UTF8_BOM.data(), UTF8_BOM.size());

				constexpr std::string_view section_seperator = "-------- -------- -------- --------\n";
				OUTPUT(o_file, section_seperator);
				OUTPUT(o_file
					, t_time.date.year, '/', toPrint_fix_2{t_time.date.month}, '/', toPrint_fix_2{t_time.date.day}
					, ' ', toPrint_fix_2{t_time.time.hour}, ':', toPrint_fix_2{t_time.time.minute}, ':'
					, toPrint_fix_2{t_time.time.second}, '.', toPrint_fix_3{t_time.time.msecond}, '\n');


				//---- The reason of the crash see: https://msdn.microsoft.com/en-us/library/cc704588.aspx

				OUTPUT(o_file, "Exception: 0x"sv, toPrint_hex_fix{static_cast<uint32_t>(ExceptionInfo->ExceptionRecord->ExceptionCode)});
				OUTPUT(o_file, "\nAddress:   "sv, ExceptionInfo->ExceptionRecord->ExceptionAddress, '\n');
				//commits current known data to file
				//prevents from getting nothing if anything further causes a fatal terminate
				o_file.flush_unlocked();

				//---- Moduie list
				OUTPUT(o_file, section_seperator, "Modules:\n"sv);

				//ids are different from handles, handles can be used to collect data inside the application, but they are meaningless externally
				//ids don't have much use inside the application, but can be used externally to correlate data
				HANDLE t_proc	= GetCurrentProcess		();	// access to process information
				HANDLE t_thread	= GetCurrentThread		();	// access to current thread information
				DWORD proc_ID	= GetCurrentProcessId	();	// id of process
				DWORD thread_ID	= GetCurrentThreadId	();	// id of current thread

				{
					enumerate2file_context t_enum_context
					{
						.m_proc = t_proc,
						.m_file = &o_file
					};	// used to tell the callback function what file to write too, and also give them the process handle

					//This function will call the function (Core_EnumerateModules2File) for each module (dll + application) that is loaded
					//this will allow us to list all modules, and also help us make sense of all those address
					//see EnumerateLoadedModules64 on msdn: https://msdn.microsoft.com/en-us/library/windows/desktop/ms679316(v=vs.85).aspx
					//Note: This is useful to correlate the addresses 
					EnumerateLoadedModulesW64(t_proc, EnumerateModules2File, &t_enum_context);
				}

				OUTPUT(o_file, section_seperator, "Stack:\n"sv);

				SymSetOptions(SYMBOL_OPTIONS);
				//======== IMPORTANT ========
				//This will allows the program to know the symbols (function names)
				//based on an address, see SymInitialize on msdn: https://msdn.microsoft.com/en-us/library/windows/desktop/ms681351(v=vs.85).aspx
				SymInitialize(t_proc, nullptr, TRUE);
				//======== END ========
				
				//======== IMPORTANT ======== Actual stack trace starts here
				if(ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW) //In case of a stack overflow we can't walk the stack because it is damaged
				{
					print_function_addr(t_proc, static_cast<DWORD64>(reinterpret_cast<uintptr_t>(ExceptionInfo->ExceptionRecord->ExceptionAddress)), o_file); //Just print the last known address, better than nothing
				}
				else
				{
					CONTEXT t_alt_context = *(ExceptionInfo->ContextRecord); // used to retrieve information about the crash
					if((t_alt_context.ContextFlags & CONTEXT_CONTROL) == CONTEXT_CONTROL)
					{
						STACKFRAME64 t_frame;
						ZeroMemory(&t_frame, sizeof(STACKFRAME64));
						//t_frame.AddrPC.Offset		= ExceptionInfo->ExceptionRecord->ExceptionAddress;
						//t_frame.AddrPC.Offset		= t_alt_context.Eip;
						t_frame.AddrPC.Mode			= AddrModeFlat;	//necessary, otherwise StackWalk64 doesn't output the address
						//t_frame.AddrStack.Offset	= t_alt_context.Esp;
						t_frame.AddrStack.Mode		= AddrModeFlat;
						//t_frame.AddrFrame.Offset	= t_alt_context.Ebp;
						t_frame.AddrFrame.Mode		= AddrModeFlat;

#ifdef _WIN64
						while(StackWalk64(IMAGE_FILE_MACHINE_AMD64,
										  t_proc, t_thread, &t_frame,
										  &t_alt_context,
										  nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
#else
						while(StackWalk64(IMAGE_FILE_MACHINE_I386,
										  t_proc, t_thread, &t_frame,
										  &t_alt_context,
										  nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
#endif
						{
							//See StackWalk64 on msdn: https://msdn.microsoft.com/en-us/library/windows/desktop/ms680650(v=vs.85).aspx
							print_function_addr(t_proc, t_frame.AddrPC.Offset, o_file); //Prints the address while there are more addresses in the stack
						}
					}
				}

				SymCleanup(t_proc); //frees resources allocated when SymInitialize was called
				//======== END ========
				//commits current known data to file
				//prevents from getting nothing if anything further causes a fatal terminate
				o_file.flush_unlocked();

				//---- Extra information
				OUTPUT(o_file, section_seperator);
				PrintCPUinfo(o_file);

				OUTPUT(o_file, section_seperator,
					"Proc:        "sv, proc_ID,
					"\nThread:      "sv, thread_ID,
					"\nProcDir:     \""sv, application_path(), '\n');
				
				{
					std::error_code ec;
					OUTPUT(o_file, "WorkDir:     \""sv, std::filesystem::current_path(ec), "\"\n"sv);
				}

				{
					LPWSTR temp_str = GetCommandLineW();
					if(temp_str)
					{
						OUTPUT(o_file, "CommandLine: "sv, os_string_view{temp_str}, '\n');
					}
				}

				PrintOS(o_file);

				{
					const std::optional<core::os_string>& res = machine_name();
					if(res.has_value())
					{
						OUTPUT(o_file, "Machine:     \""sv, res.value(), "\"\n"sv);
					}
				}
				{
					DWORD hcount;
					if(GetProcessHandleCount(GetCurrentProcess(), &hcount) == TRUE)
					{
						//Number of open handles (can be open files, network ports, etc.)
						OUTPUT(o_file, "Handles:     "sv, hcount, '\n');
					}
				}

				OUTPUT(o_file, section_seperator, "Env:\n"sv);
				PrintEnv(o_file);			//Prints the environment variables at this moment

				OUTPUT(o_file, section_seperator);
				o_file.close();
			}
		}

		return EXCEPTION_CONTINUE_SEARCH;	//This will exit, and still cause a mini-dump
	}
} //namesapce

bool register_crash_trace(const std::filesystem::path& p_output_file)
{

	if(p_output_file.is_absolute())
	{
		g_straceOpt.m_output_file = p_output_file.lexically_normal();
	}
	else
	{
		g_straceOpt.m_output_file = (core::application_path().parent_path() / p_output_file).lexically_normal();
	}

	//======== IMPORTANT ========
	//Sets the exception handler, i.e. when a problem that would crash the program occurs,
	//run this function (Core_STrace::Core_WIN_exception_handler) before the program exists
	//see SetUnhandledExceptionFilter on msdn for more information: https://msdn.microsoft.com/en-us/library/windows/desktop/ms680634(v=vs.85).aspx
	SetUnhandledExceptionFilter(WIN_exception_handler);
	//======== END ========
	return true;
}

uint8_t list_modules(std::list<ModuleAddr>& p_list)
{
	p_list.clear();
	HANDLE const t_proc = GetCurrentProcess();
	enumerate2list_context t_enum_context
	{
		.m_proc = t_proc,
		.m_list = &p_list
	};
	
	return EnumerateLoadedModulesW64(t_proc, EnumerateModules2List, &t_enum_context) ? 0 : 1;
}

uint8_t stack_trace(StackTrace_FullInfo& p_trace)
{
	uint8_t ret;
	HANDLE const t_proc = GetCurrentProcess();

	p_trace.m_modules	.clear();
	p_trace.m_stack		.clear();

	enumerate2list_context t_enum_context
	{
		.m_proc = t_proc,
		.m_list = &p_trace.m_modules
	};

	ret = EnumerateLoadedModulesW64(t_proc, EnumerateModules2List, &t_enum_context) ? 0 : 1;;

	void* t_trace[64];
	USHORT t_numReturned = RtlCaptureStackBackTrace(0, 64, t_trace, nullptr);

	if(t_numReturned)
	{
		SymSetOptions(SYMBOL_OPTIONS);
		SymInitialize(t_proc, nullptr, TRUE);
		for(USHORT i = 0; i < t_numReturned; ++i)
		{
			AppendStackAddr2List(t_proc, (uint64_t) t_trace[i], p_trace.m_stack);
		}
		SymCleanup(t_proc);

		return ret;
	}
	return ret | 2;
}

uint8_t stack_trace(std::list<StackBaseInfo>& p_trace)
{
	void* t_trace[64];
	p_trace.clear();
	USHORT t_numReturned = RtlCaptureStackBackTrace(0, 64, t_trace, nullptr);

	if(t_numReturned)
	{
		HANDLE const t_proc = GetCurrentProcess();
		SymSetOptions(SYMBOL_OPTIONS);
		SymInitialize(t_proc, nullptr, TRUE);
		for(USHORT i = 0; i < t_numReturned; ++i)
		{
			AppendStackAddr2List(t_proc, (uint64_t) t_trace[i], p_trace);
		}
		SymCleanup(t_proc);
		return 0;
	}
	return 2;
}

uint8_t stack_trace(std::list<StackInfo>& p_trace)
{
	void* t_trace[64];
	p_trace.clear();
	const USHORT t_numReturned = RtlCaptureStackBackTrace(0, 64, t_trace, nullptr);
	if(t_numReturned)
	{
		HANDLE t_proc = GetCurrentProcess();
		SymSetOptions(SYMBOL_OPTIONS);
		SymInitialize(t_proc, nullptr, TRUE);
		for(USHORT i = 0; i < t_numReturned; ++i)
		{
			AppendStackAddr2List(t_proc, (uint64_t) t_trace[i], p_trace);
		}
		SymCleanup(t_proc);
		return 0;
	}
	return 2;
}

uint8_t stack_trace(std::list<uintptr_t>& p_trace)
{
	void* t_trace[64];
	p_trace.clear();
	const USHORT t_numReturned = RtlCaptureStackBackTrace(0, 64, t_trace, nullptr);
	if(t_numReturned)
	{
		for(USHORT i = 0; i < t_numReturned; ++i)
		{
			p_trace.push_back(reinterpret_cast<uintptr_t>(t_trace[i]));
		}
		return 0;
	}
	return 2;
}

uint8_t generate_minidump(const std::filesystem::path& p_file)
{
	HANDLE	const t_proc	= GetCurrentProcess		();
	DWORD	const t_pId		= GetCurrentProcessId	();
	HANDLE	const t_file	= CreateFileW(p_file.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
						 CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	if(t_file == INVALID_HANDLE_VALUE) return 1;

	const BOOL ret = MiniDumpWriteDump(t_proc, t_pId, t_file, GenerateMinidumpFlags(), nullptr, nullptr, nullptr);

	CloseHandle(t_file);
	return ret ? 0 : 2;
}
//======== ======== ======== ======== ======== ======== ======== ========

} //namespace core

#else
//======== ======== ======== ======== ======== ======== ======== ========
//======== ======== ========  LINUX SPECIFIC   ======== ======== ========
//======== ======== ======== ======== ======== ======== ======== ========
extern char** environ;

namespace core
{

namespace
{
	///	\brief Used to store global sack trace options
	class Stack_Trace_Options
	{
	public:
		std::filesystem::path m_output_file;
	};

	static Stack_Trace_Options g_straceOpt;

	static std::filesystem::path g_app_path = application_path();

	static std::array<uint8_t, 0x40000> Except_stack;	//!< Used a stack for the exception handling

	/// \brief Auxiliary function used to print operating system version
	static void PrintOS(file_write& p_file)
	{
		struct utsname temp;
		if(uname(&temp))
		{
			OUTPUT(p_file, "OS:      Linux\n"sv);
		}
		else
		{
			OUTPUT(p_file,
				"OS:      "sv, std::string_view{temp.sysname},
				' ', std::string_view{temp.machine},
				" - "sv, std::string_view{temp.release},
				" - "sv, std::string_view{temp.version}, '\n');
		}
	}

	/// \brief Helper function to print environment variables
	static void PrintEnv(file_write& p_file)
	{
		char** current;
		for(current = environ; *current; ++current)
		{
			OUTPUT(p_file, std::string_view{*current}, '\n');
		}
	}

	/// \brief Helper function to print the command line arguments
	static void PrintCMD(file_write& p_file)
	{
		file_read p_in;
		p_in.open("/proc/self/cmdline");
		if(p_in.is_open())
		{
			std::u8string res;
			{
				char8_t c;
				p_in.read_unlocked(&c, 1);
				while(p_in.good())
				{
					res.push_back(c);
					p_in.read_unlocked(&c, 1);
				}
			}

			OUTPUT(p_file, "Arguments:\n"sv);
			const char8_t* pivot = res.data();
			const char8_t* const last = pivot + res.size();
			uint32_t count = 0;
			while(pivot != last)
			{
				const char8_t* pos = reinterpret_cast<const char8_t*>(memchr(pivot, 0, last - pivot));
				if(pos)
				{
					OUTPUT(p_file, '\t', count, ": "sv, std::u8string_view{pivot, static_cast<uintptr_t>(pos - pivot)}, '\n');
					pivot = pos + 1;
				}
				else
				{
					OUTPUT(p_file, '\t', count, ": "sv, std::u8string_view{pivot, static_cast<uintptr_t>(last - pivot)}, '\n');
					break;
				}
				++count;
			}
			p_in.close();
		}
		else
		{
			OUTPUT(p_file, "Arguments: Unable to fetch!\n"sv);
		}
	}

	///	\brief Helper function used to enumerate modules onto a list
	///	\note This function is called for each loaded module, and twice more at the beginning
	///		I suspect that the first 2 address corresponds to:
	///			- the application address (which is different from the application base address)
	///			- the next free address where modules can be loaded
	///		however I'm not sure, this is not documented, take this information with a grain of salt.
	static int EnumerateModules2List(struct dl_phdr_info *const  p_info, size_t const /*p_size*/, void* const p_context)
	{
		ModuleAddr t_entry;
		std::list<ModuleAddr>* t_list = reinterpret_cast<std::list<ModuleAddr>*>(p_context);

		t_entry.m_addr = p_info->dlpi_addr;
		//t_entry.m_size = ???;
		t_entry.m_name = p_info->dlpi_name;

		t_list->push_back(std::move(t_entry)); //why the hell the standard didn't have in place construction from the start??
		return 0;
	}

	///	\brief Helper function used to enumerate modules onto a file
	///	\note This function is called for each loaded module, and twice more at the beginning
	///		I suspect that the first 2 address corresponds to:
	///			- the application address (which is different from the application base address)
	///			- the next free address where modules can be loaded
	///		however I'm not sure, this is not documented, take this information with a grain of salt.
	static int EnumerateModules2File(struct dl_phdr_info* const p_info, size_t const /*p_size*/, void* const p_context)
	{
		//missing BaseOfDll + ModuleSize);
		OUTPUT(*reinterpret_cast<file_write*>(p_context),
			reinterpret_cast<void*>(p_info->dlpi_addr), " \""sv, os_string_view{p_info->dlpi_name}, "\"\n"sv);
		return 0;
	}

	///	\brief This function prints 1 stack addresses and associated function name,
	///		first it pretyfies the address to make it more easy to correlated with the module it has been loaded from,
	///		secondly it tries to get the name of the symbol to which this address belongs too.
	///	\warning Function names may not necessarily be correct. \n
	///		Name is determined based on last know name prior to current address, which is not necessarily the same function as the one the address belongs to.
	///		Use .map files to confirm names.
	static void print_function_addr(void* const p_address, char* const p_name, file_write& p_file)
	{
		Dl_info t_info;
		//this function gets additional information about the address
		//what we are interested in is the base address of the module that the input address corresponds to
		//which makes it easier to make sense of this data, this function is not that well documented
		//see: http://linux.die.net/man/3/dladdr
		if(dladdr(p_address, &t_info) && (t_info.dli_fbase < p_address))
		{
			//prints it in the form of base address + offset ex: 0x0000123400+567
			uintptr_t diff = reinterpret_cast<uintptr_t>(p_address) - reinterpret_cast<uintptr_t>(t_info.dli_fbase);
			OUTPUT(p_file, t_info.dli_fbase, '+', toPrint_hex{diff});
		}
		else
		{
			OUTPUT(p_file, p_address);
		}
		
		//do not forget to print the symbol name, if there is one
		if(p_name)
		{
			OUTPUT(p_file, ' ', std::string_view{p_name});
		}
		OUTPUT(p_file, '\n');
	}

	///	\brief	This function performs all tasks related to printing the stack trace information.
	///	\details
	///			If the stack trace service is registered, the system will call this function when
	///			SIGILL, SIGABRT, SIGFPE, or SIGSEGV occurs, allowing the opportunity to this function to print:
	///			- Date and time of event
	///			- Signal number, signal information code, and critical address
	///			- Name and addresses of loaded modules
	///			- Stack trace with relative addresses and associated names (not guaranteed to be correct)
	///			- Process ID and thread ID
	///			- Application directory and working directory
	///			- Machine name
	///			- Environment variables
	///	\note Linux version
	static void Linux_exception_handler(const int sig, siginfo_t* const siginfo, void* const context)
	{
		static bool b_has_sig = false;
		//because of the way the signal mechanism works, for example signals can be raised while processing signals
		//it is just better to ensure that this can only get called once, after all the program should crash after this
		if(!b_has_sig)
		{
			b_has_sig = true;
			//resets the signals to their default behaviors, i.e. exit the application and cause a core dump
			//if this is not done there is not only a risk of getting stuck in trying to perpetual handle this signals
			//also we wouldn't be able to cause a core dump at the end
			signal(SIGILL,	SIG_DFL);
			signal(SIGABRT,	SIG_DFL);
			signal(SIGFPE,	SIG_DFL);
			signal(SIGSEGV,	SIG_DFL);

			if(!g_straceOpt.m_output_file.empty())
			{
				file_write		o_file;						// file to be output
				date_time		t_time;
				date_time_local(t_time);	//current date

				{
					std::error_code ec;
					std::filesystem::create_directories(g_straceOpt.m_output_file.parent_path(), ec);
					o_file.open(g_straceOpt.m_output_file, file_write::open_mode::create);
				}

				if(o_file.is_open())
				{
					constexpr std::array UTF8_BOM = {char8_t{0xEF}, char8_t{0xBB}, char8_t{0xBF}};
					o_file.write_unlocked(UTF8_BOM.data(), UTF8_BOM.size());

					std::string_view section_seperator = "-------- -------- -------- --------\n";

					OUTPUT(o_file, section_seperator);
					OUTPUT(o_file
						, t_time.date.year, '/', toPrint_fix_2{t_time.date.month}, '/', toPrint_fix_2{t_time.date.day}
						, ' ', toPrint_fix_2{t_time.time.hour}, ':', toPrint_fix_2{t_time.time.minute}, ':'
						, toPrint_fix_2{t_time.time.second}, '.', toPrint_fix_3{t_time.time.msecond}, '\n');

					ucontext_t* t_context = (ucontext_t*) context;
					void* t_criticalAddr;	// the address were the crash occurred

					if(context)
					{
#if defined(__i386__) // gcc specific
						t_criticalAddr = (void*) reinterpret_cast<struct sigcontext*>(&(t_context->uc_mcontext))->eip;
#elif defined(__x86_64__) // gcc specific
						t_criticalAddr = (void*) reinterpret_cast<struct sigcontext*>(&(t_context->uc_mcontext))->rip;
#elif defined(__ppc__) || defined(__powerpc__)
						t_criticalAddr = (void*) reinterpret_cast<struct sigcontext*>(&(t_context->uc_mcontext))->nip; //probably wrong
#else
						t_criticalAddr = nullptr;
#endif
					}
					else
					{
						t_criticalAddr = nullptr;
					}

					OUTPUT(o_file,
						"Sig:     0x"sv, toPrint_hex_fix{static_cast<uint32_t>(sig)}, //the signal that caused the crash
						"\nCode:    0x"sv, toPrint_hex_fix{static_cast<uint32_t>(siginfo->si_code)},	//the additional code associated with the signal
						'\n');
																					//for example the type of floating point error
					//see: http://linux.die.net/man/2/sigaction

					//print the address where the problem occurred, if there is one
					if(t_criticalAddr)
					{
						OUTPUT(o_file, "Address: "sv, t_criticalAddr, '\n');
					}

					//commits current known data to file
					//prevents from getting nothing if anything further causes a fatal terminate
					o_file.flush_unlocked();

					//---- Moduie list
					OUTPUT(o_file, section_seperator, "Modules:\n"sv);
					
					//lists all loaded modules, calls EnumerateModules2File per loaded module, and then some
					//see: http://linux.die.net/man/3/dl_iterate_phdr
					dl_iterate_phdr(EnumerateModules2File, &o_file);

					OUTPUT(o_file, section_seperator, "Stack:\n"sv);


					void*	trace[64];	// holds the stack trace before it is processed for output
					//======== IMPORTANT ========
					//get the stack addresses and puts it in the "trace" buffer
					//see: http://linux.die.net/man/3/backtrace
					int trace_size = backtrace(trace, 64);
					//======== END ========
					if(trace_size)
					{
						//======== IMPORTANT ========
						//gets the symbols (function names) associated with this addresses
						//see: http://linux.die.net/man/3/backtrace_symbols
						char** t_symb = backtrace_symbols(trace, trace_size);
						//======== END ========
						int i = 0;
						//note that this trace will spit out addresses seen at the point where "backtrace" was called
						//including this function itself, and the handler function that called it
						//so the code that follows is to skip this addresses at the top, so that we can get to the address
						//where the problem actually occurred and get a stack trace from there
						if(t_criticalAddr)
						{
							for(; i < trace_size; ++i)
							{
								if(trace[i] == t_criticalAddr) break;
							}
							if(i >= trace_size)
							{
								//if we failed to get to the address where the problem occurred
								//well, just print everything and let it to the engineer to sort it out
								i = 0;
							}
						}

						//prints all addresses on the stack with respective symbol names
						if(t_symb)
						{
							for(; i < trace_size; ++i)
							{
								if(trace[i])
								{
									print_function_addr(trace[i], t_symb[i], o_file);
								}
							}
							free(t_symb);	//releases the memory created by backtrace_symbols
						}
						else
						{
							for(; i < trace_size; ++i)
							{
								if(trace[i])
								{
									print_function_addr(trace[i], nullptr, o_file);
								}
							}
						}
					}

					//commits current known data to file
					//prevents from getting nothing if anything further causes a fatal terminate
					o_file.flush_unlocked();
					
					uint32_t	proc_ID		= getpid();
					pthread_t	thread_ID	= pthread_self();

					//---- Extra information
					OUTPUT(o_file, section_seperator);
					PrintCPUinfo(o_file);

					//Note: for some reason when handling the signal caused by an access violation, creating a path causes an access violation
					//so we store the path in a global
					OUTPUT(o_file, section_seperator,
						"Proc:    "sv, proc_ID,
						"\nThread:  "sv, thread_ID,
						"\nProcDir: \""sv, g_app_path, '\n');

					{
						std::error_code ec;
						OUTPUT(o_file, "WorkDir: \""sv, std::filesystem::current_path(ec), "\"\n"sv);
					}

					PrintCMD(o_file);	//Prints the commands that were passed to the application
					PrintOS(o_file);
					{
						const std::optional<core::os_string>& res = machine_name();
						if(res.has_value())
						{
							OUTPUT(o_file, "Machine: \""sv, res.value(), "\"\n"sv);
						}
					}

					/*
						TODO: put file descriptor count here
					*/

					OUTPUT(o_file, section_seperator, "Env:\n"sv);

					PrintEnv(o_file);	//Prints the environment variables at this moment

					OUTPUT(o_file, section_seperator);
					o_file.close();
				}
			}
			//Aborts the program, so that it quits while causing a core dump
			//It is important that abort signal was restored to its original signal handler
			raise(SIGABRT);
		}
		//_exit(EXIT_FAILURE);
		_Exit(EXIT_FAILURE); //just in case this handler is called multiple times
	}
} //namespace

bool register_crash_trace(const std::filesystem::path& p_output_file)
{
	if(p_output_file.is_absolute())
	{
		g_straceOpt.m_output_file = p_output_file.lexically_normal();
	}
	else
	{
		g_straceOpt.m_output_file = (application_path().parent_path() / p_output_file).lexically_normal();
	}

	//======== IMPORTANT ========
	//this sets up an alternative stack to run on when we are doing our stack trace
	//see article:https://spin.atomicobject.com/2013/01/13/exceptions-stack-traces-c/
	stack_t ss;
	ss.ss_sp	= reinterpret_cast<void*>(Except_stack.data());
	ss.ss_size	= SIGSTKSZ;
	ss.ss_flags	= 0;
	if (sigaltstack(&ss, nullptr)) return false;

	//initializes the parameters that we are going to use to hijack the signals
	struct sigaction sig_action;
	sig_action.sa_sigaction = Linux_exception_handler;
	sigemptyset(&sig_action.sa_mask);
	sig_action.sa_flags = SA_SIGINFO | SA_ONSTACK;

	if(	sigaction(SIGILL,  &sig_action, nullptr) ||
		sigaction(SIGABRT, &sig_action, nullptr) ||
		sigaction(SIGFPE,  &sig_action, nullptr) ||
		sigaction(SIGSEGV, &sig_action, nullptr) )
	{
		return false;
	}

	return true;
}

uint8_t list_modules(std::list<ModuleAddr>& p_list)
{
	p_list.clear();
	dl_iterate_phdr(EnumerateModules2List, &p_list);
	return p_list.empty() ? 1 : 0;
}

uint8_t stack_trace(StackTrace_FullInfo& p_trace)
{
	uint8_t ret;
	p_trace.m_modules	.clear();
	p_trace.m_stack		.clear();

	dl_iterate_phdr(EnumerateModules2List, &p_trace.m_modules);
	ret = p_trace.m_modules.empty() ? 1 : 0;

	void* trace[64];
	int trace_size = backtrace(trace, 64);

	if(trace_size)
	{
		char** t_symb = backtrace_symbols(trace, trace_size);

		if(t_symb)
		{
			for(int i = 0; i < trace_size; ++i)
			{
				if(trace[i])
				{
					StackBaseInfo t_info;
					t_info.m_addr = reinterpret_cast<uintptr_t>(trace[i]);
					t_info.m_name = reinterpret_cast<const char8_t*>(t_symb[i]);
					Dl_info t_dlinfo;
					if(dladdr(trace[i], &t_dlinfo) && (t_dlinfo.dli_fbase < trace[i]))
					{
						t_info.m_modBase = (uint64_t) t_dlinfo.dli_fbase;
					}
					else
					{
						t_info.m_modBase = 0;
					}
					p_trace.m_stack.push_back(t_info);
				}
			}
			free(t_symb);
			return ret;
		}
		else
		{
			for(int i = 0; i < trace_size; ++i)
			{
				if(trace[i])
				{
					StackBaseInfo t_info;
					t_info.m_addr = reinterpret_cast<uintptr_t>(trace[i]);
					Dl_info t_dlinfo;
					if(dladdr(trace[i], &t_dlinfo) && (t_dlinfo.dli_fbase < trace[i]))
					{
						t_info.m_modBase = reinterpret_cast<uintptr_t>(t_dlinfo.dli_fbase);
					}
					else
					{
						t_info.m_modBase = 0;
					}
					p_trace.m_stack.push_back(t_info);
				}
			}
		}
		return ret | 4;
	}
	return ret | 2;
}

uint8_t stack_trace(std::list<StackBaseInfo>& p_trace)
{
	p_trace.clear();

	void* trace[64];
	int trace_size = backtrace(trace, 64);

	if(trace_size)
	{
		char** t_symb = backtrace_symbols(trace, trace_size);

		if(t_symb)
		{
			for(int i = 0; i < trace_size; ++i)
			{
				if(trace[i])
				{
					StackBaseInfo t_info;
					t_info.m_addr = reinterpret_cast<uintptr_t>(trace[i]);
					t_info.m_name = reinterpret_cast<const char8_t*>(t_symb[i]);
					Dl_info t_dlinfo;
					if(dladdr(trace[i], &t_dlinfo) && (t_dlinfo.dli_fbase < trace[i]))
					{
						t_info.m_modBase = reinterpret_cast<uintptr_t>(t_dlinfo.dli_fbase);
					}
					else
					{
						t_info.m_modBase = 0;
					}
					p_trace.push_back(t_info);
				}
			}
			free(t_symb);
			return 0;
		}
		else
		{
			for(int i = 0; i < trace_size; ++i)
			{
				if(trace[i])
				{
					StackBaseInfo t_info;
					t_info.m_addr = reinterpret_cast<uintptr_t>(trace[i]);
					Dl_info t_dlinfo;
					if(dladdr(trace[i], &t_dlinfo) && (t_dlinfo.dli_fbase < trace[i]))
					{
						t_info.m_modBase = reinterpret_cast<uintptr_t>(t_dlinfo.dli_fbase);
					}
					else
					{
						t_info.m_modBase = 0;
					}
					p_trace.push_back(t_info);
				}
			}
		}
		return 4;
	}
	return 2;
}

uint8_t stack_trace(std::list<StackInfo>& p_trace)
{
	p_trace.clear();

	void* trace[64];
	int trace_size = backtrace(trace, 64);

	if(trace_size)
	{
		char** t_symb = backtrace_symbols(trace, trace_size);

		if(t_symb)
		{
			for(int i = 0; i < trace_size; ++i)
			{
				if(trace[i])
				{
					StackInfo t_info;
					t_info.m_addr = reinterpret_cast<uintptr_t>(trace[i]);
					t_info.m_name = reinterpret_cast<const char8_t*>(t_symb[i]);
					p_trace.push_back(t_info);
				}
			}
			free(t_symb);
			return 0;
		}
		else
		{
			for(int i = 0; i < trace_size; ++i)
			{
				if(trace[i])
				{
					StackInfo t_info;
					t_info.m_addr = reinterpret_cast<uintptr_t>(trace[i]);
					p_trace.push_back(t_info);
				}
			}
		}
		return 4;
	}
	return 2;
}

uint8_t stack_trace(std::list<uintptr_t>& p_trace)
{
	p_trace.clear();

	void* trace[64];
	int trace_size = backtrace(trace, 64);

	if(trace_size)
	{
		for(int i = 0; i < trace_size; ++i)
		{
			if(trace[i])
			{
				p_trace.push_back(reinterpret_cast<uintptr_t>(trace[i]));
			}
		}
		return 0;
	}
	return 2;
}

uint8_t generate_coredump()
{
	pid_t t_pid = fork();

	if(t_pid == 0)
	{
		signal(SIGABRT, SIG_DFL);
		abort();
	}

	if(t_pid < 0) return 1;

	return 0;
}
} //namespace core
#endif

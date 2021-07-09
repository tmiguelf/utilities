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

#include <CoreLib/Core_StackTrace.hpp>

#include <fstream>
#include <iomanip>
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
#endif

#include <CoreLib/Core_OS.hpp>
#include <CoreLib/Core_Time.hpp>

#include "CoreLib/string/core_string_encoding.hpp"
#include "CoreLib/string/core_string_numeric.hpp"

namespace core
{

namespace
{
template <typename T>
static inline void push_hex_fix(std::ofstream& p_out, T p_num)
{
	std::array<char, to_chars_hex_max_digits_v<T>> buff;
	uintptr_t size = to_chars_hex(p_num, buff);
	p_out.write(buff.data(), size);
}

template <typename T>
static inline void push_dec(std::ofstream& p_out, T p_num)
{
	std::array<char, to_chars_dec_max_digits_v<T>> buff;
	uintptr_t size = to_chars(p_num, buff);
	p_out.write(buff.data(), size);
}

static inline void push_time(std::ofstream& p_out, const DateTime& p_time)
{
	char8_t buff[16];
	uintptr_t reSize;

	reSize = to_chars(p_time.date.year, std::span<char8_t, 5>(buff, 5));
	p_out.write(reinterpret_cast<const char*>(buff), reSize);
	p_out.put('/');

	reSize = to_chars(p_time.date.month, std::span<char8_t, 3>(buff, 3));
	if(reSize < 2) p_out.put('0');
	p_out.write(reinterpret_cast<const char*>(buff), reSize);
	p_out.put('/');

	reSize = to_chars(p_time.date.day, std::span<char8_t, 3>(buff, 3));
	if(reSize < 2) p_out.put('0');
	p_out.write(reinterpret_cast<const char*>(buff), reSize);
	p_out.put(' ');

	reSize = to_chars(p_time.time.hour, std::span<char8_t, 3>(buff, 3));
	if(reSize < 2) p_out.put('0');
	p_out.write(reinterpret_cast<const char*>(buff), reSize);
	p_out.put(':');

	reSize = to_chars(p_time.time.minute, std::span<char8_t, 3>(buff, 3));
	if(reSize < 2) p_out.put('0');
	p_out.write(reinterpret_cast<const char*>(buff), reSize);
	p_out.put(':');

	reSize = to_chars(p_time.time.second, std::span<char8_t, 3>(buff, 3));
	if(reSize < 2) p_out.put('0');
	p_out.write(reinterpret_cast<const char*>(buff), reSize);
	p_out.put('.');

	reSize = to_chars(p_time.time.msecond, std::span<char8_t, 5>(buff, 5));
	if(reSize < 3) p_out.put('0');
	if(reSize < 2) p_out.put('0');
	p_out.write(reinterpret_cast<const char*>(buff), reSize);
	p_out.put('\n');
}

static inline void push_data(std::ofstream& p_out, os_string_view p_string)
{
#ifdef _WIN32
	const std::u8string& res = UTF16_to_UTF8_faulty(std::u16string_view{reinterpret_cast<const char16_t*>(p_string.data()), p_string.size()}, u'?');
	p_out.write(reinterpret_cast<const char*>(res.data()), res.size());
#else
	p_out.write(p_string.data(), p_string.size());
#endif
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
		std::ofstream*	m_file;	//!< Output file
		HANDLE			m_proc;	//!< Process handle
	};

	///	\brief Used to pass the process handle and the list we are outputting to to the module enumeration call back.
	struct enumerate2list_context
	{
		std::list<ModuleAddr>*	m_list;	//!< output list
		HANDLE					m_proc;	//!< Process handle
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

	/// \brief Auxiliary function used to print environment variables
	static void PrintEnv(std::ofstream& p_file)
	{
		wchar_t* const t_base = GetEnvironmentStringsW();
		const wchar_t* t_pivot = t_base;
		if(t_base)
		{
			while(*t_pivot)
			{
				core::os_string_view data{t_pivot};
				push_data(p_file, data);
				p_file.put('\n');
				t_pivot += data.size() + 1;
			}
			FreeEnvironmentStringsW(t_base);
		}
	}

	/// \brief Auxiliary function used to enumerate modules and print onto file
	static BOOL CALLBACK EnumerateModules2File(PCWSTR ModuleName, DWORD64 BaseOfDll, ULONG ModuleSize, PVOID UserContext)
	{
		enumerate2file_context* t_context = reinterpret_cast<enumerate2file_context*>(UserContext);
		std::ofstream& tfile = *(t_context->m_file);

		push_hex_fix(tfile, BaseOfDll);
		tfile.put(' ');
		push_hex_fix(tfile, BaseOfDll + ModuleSize);
		tfile.write(" \"", 2);
		push_data(tfile, os_string_view{ModuleName});
		tfile.write("\"\n", 2);
		return TRUE;
	}

	/// \brief Auxiliary function used to enumerate modules onto list
	//
	static BOOL CALLBACK EnumerateModules2List(PCWSTR ModuleName, DWORD64 BaseOfDll, ULONG ModuleSize, PVOID UserContext)
	{
		ModuleAddr t_entry;
		enumerate2list_context* t_context = reinterpret_cast<enumerate2list_context*>(UserContext);
		
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
	static void print_function_addr(HANDLE p_proc, DWORD64 p_address, std::ofstream& p_file)
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
			push_hex_fix(p_file, static_cast<uintptr_t>(base));
			p_file.put('+');
			push_hex_fix(p_file, static_cast<uintptr_t>(p_address - base));
		}
		else
		{	//in theory this should never be the case, this is just in case SymGetModuleBase64 fails, or it produces
			//data that makes no sense, you may not be able to accurately correlated it to the module, but at least you have something
			push_hex_fix(p_file, static_cast<uintptr_t>(p_address));
		}

		//======== IMPORTANT ======== 
		//this gets the name of the symbol (function name) the input address belongs too
		//see SymGetSymFromAddr64 on msdn: https://msdn.microsoft.com/en-us/library/windows/desktop/ms681335(v=vs.85).aspx
		//Note: It is common for most of the symbols names not to be found on release builds, the .pdb file helps with that
		if(SymGetSymFromAddr64(p_proc, p_address, &displacement, reinterpret_cast<IMAGEHLP_SYMBOL64*>(&t_symb)) == TRUE)
		{
			if(t_symb.Name[0] !='\0')
			{
				p_file << ' ' << t_symb.Name;
			}
		}
		//======== END ======== 
		
		p_file << '\n';
	}

	///	\brief This function pushes onto a list 1 stack addresses and associated function name, plus the base address of the corresponding module.
	///	\warning Function names may not necessarily be correct. \n
	///		Name is determined based on last know name prior to current address, which is not necessarily the same function as the one the address belongs to.
	///		Use .map files to confirm names.
	static void AppendStackAddr2List(HANDLE p_proc, uint64_t p_addr, std::list<StackBaseInfo>& p_list)
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
	static void AppendStackAddr2List(HANDLE p_proc, uint64_t p_addr, std::list<StackInfo>& p_list)
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
	static LONG WINAPI WIN_exception_handler(EXCEPTION_POINTERS * ExceptionInfo)
	{
		if(!g_straceOpt.m_output_file.empty())
		{
			std::ofstream o_file;				// file to be output
			DateTime t_time = date_time_local();	//current date

			{
				std::error_code ec;
				create_directories(g_straceOpt.m_output_file.parent_path(), ec);
				o_file.open(g_straceOpt.m_output_file);
			}
			if(o_file.is_open())
			{
				std::string_view section_seperator = "-------- -------- -------- --------\n";
				o_file	<< section_seperator;
				push_time(o_file, t_time);

				//---- The reason of the crash see: https://msdn.microsoft.com/en-us/library/cc704588.aspx
				o_file << "Exception:\t0x";
				push_hex_fix(o_file, static_cast<uint32_t>(ExceptionInfo->ExceptionRecord->ExceptionCode));
				o_file << "\nAddress:\t0x";
				push_hex_fix(o_file, reinterpret_cast<uint64_t>(ExceptionInfo->ExceptionRecord->ExceptionAddress));
				o_file.put('\n');

				//commits current known data to file
				//prevents from getting nothing if anything further causes a fatal terminate
				o_file << std::flush;

				//---- Moduie list
				o_file	<< section_seperator
						<< "Modules:\n";

				//ids are different from handles, handles can be used to collect data inside the application, but they are meaningless externally
				//ids don't have much use inside the application, but can be used externally to correlate data
				HANDLE t_proc	= GetCurrentProcess		();	// access to process information
				HANDLE t_thread	= GetCurrentThread		();	// access to current thread information
				DWORD proc_ID	= GetCurrentProcessId	();	// id of process
				DWORD thread_ID	= GetCurrentThreadId	();	// id of current thread

				enumerate2file_context	t_enum_context;	// used to tell the callback function what file to write too, and also give them the process handle
				t_enum_context.m_file = &o_file;
				t_enum_context.m_proc = t_proc;
				
				//This function will call the function (Core_EnumerateModules2File) for each module (dll + application) that is loaded
				//this will allow us to list all modules, and also help us make sense of all those address
				//see EnumerateLoadedModules64 on msdn: https://msdn.microsoft.com/en-us/library/windows/desktop/ms679316(v=vs.85).aspx
				//Note: This is useful to correlate the addresses 
				EnumerateLoadedModulesW64(t_proc, EnumerateModules2File, &t_enum_context);
				
				o_file	<< section_seperator
						<< "Stack:\n";

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
				o_file << std::flush;
				
				//---- Extra information
				o_file << section_seperator;
				o_file << "Proc:\t\t";
				push_dec(o_file, static_cast<uint32_t>(proc_ID));
				o_file << "\nThread:\t\t";
				push_dec(o_file, static_cast<uint32_t>(thread_ID));
				o_file << "\nProcDir:\t\"";;
				push_data(o_file, application_path().native());
				
				{
					std::error_code ec;
					o_file << "\nWorkDir:\t\"";
					push_data(o_file, std::filesystem::current_path(ec).native());
					o_file.write("\"\n", 2);
				}

				{
					const std::optional<core::os_string>& res = machine_name();
					if(res.has_value())
					{
						o_file	<< "Machine:\t\"";
						push_data(o_file, res.value());
						o_file.write("\"\n", 2);
					}
				}

				{
					LPWSTR temp_str = GetCommandLineW();
					if(temp_str)
					{
						o_file << "CommandLine: ";
						push_data(o_file, os_string_view{temp_str});
						o_file.put('\n');
					}
				}

				{
					DWORD hcount;
					if(GetProcessHandleCount(GetCurrentProcess(), &hcount) == TRUE)
					{
						//Number of open handles (can be open files, network ports, etc.)
						o_file << "Handles:\t";
						push_dec(o_file, static_cast<uint32_t>(hcount));
						o_file.put('\n');
					}
				}

				o_file	<< section_seperator;
				o_file	<< "Env:\n";
				PrintEnv(o_file);			//Prints the environment variables at this moment

				o_file	<< section_seperator;
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
	enumerate2list_context t_enum_context;
	HANDLE t_proc = GetCurrentProcess();

	p_list.clear();
	t_enum_context.m_proc = t_proc;
	t_enum_context.m_list = &p_list;
	
	return EnumerateLoadedModulesW64(t_proc, EnumerateModules2List, &t_enum_context) ? 0 : 1;
}

uint8_t stack_trace(StackTrace_FullInfo& p_trace)
{
	uint8_t ret;
	HANDLE t_proc = GetCurrentProcess();
	
	enumerate2list_context t_enum_context;

	p_trace.m_modules	.clear();
	p_trace.m_stack		.clear();

	t_enum_context.m_proc = t_proc;
	t_enum_context.m_list = &p_trace.m_modules;

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

uint8_t stack_trace(std::list<StackInfo>& p_trace)
{
	void* t_trace[64];
	p_trace.clear();
	USHORT t_numReturned = RtlCaptureStackBackTrace(0, 64, t_trace, nullptr);
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
	USHORT t_numReturned = RtlCaptureStackBackTrace(0, 64, t_trace, nullptr);
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
	HANDLE	t_proc	= GetCurrentProcess		();
	DWORD	t_pId	= GetCurrentProcessId	();
	HANDLE	t_file	= CreateFileW(p_file.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
						 CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	if(t_file == INVALID_HANDLE_VALUE) return 1;

	BOOL ret = MiniDumpWriteDump(t_proc, t_pId, t_file, GenerateMinidumpFlags(), nullptr, nullptr, nullptr);

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


	static uint8_t Except_stack[SIGSTKSZ];	//!< Used a stack for the exception handling

	/// \brief Helper function to print environment variables
	static void PrintEnv(std::ofstream& p_file)
	{
		char** current;
		for(current = environ; *current; ++current)
		{
			p_file << *current << '\n';
		}
	}

	/// \brief Helper function to print the command line arguments
	static void PrintCMD(std::ofstream& p_file)
	{
		std::ifstream p_in;
		char c;
		bool b_first = true;
		uint32_t count = 0;
		p_in.open("/proc/self/cmdline", std::ifstream::in | std::ifstream::binary);

		if(p_in.is_open())
		{
			p_file << "Arguments:" << '\n';
			c = static_cast<char>(p_in.get());

			while(p_in.good())
			{
				if(c == '\0')
				{
					p_file << '\n';
					b_first = true;
				}
				else
				{
					if(b_first)
					{
						b_first = false;
						p_file << '\t' << count++ << ": ";
					}
					p_file << c;
				}
				c = static_cast<char>(p_in.get());
			}
			if(!b_first)
			{
				p_file << '\n';
			}
			p_in.close();
		}
	}

	///	\brief Helper function used to enumerate modules onto a list
	///	\note This function is called for each loaded module, and twice more at the beginning
	///		I suspect that the first 2 address corresponds to:
	///			- the application address (which is different from the application base address)
	///			- the next free address where modules can be loaded
	///		however I'm not sure, this is not documented, take this information with a grain of salt.
	static int EnumerateModules2List(struct dl_phdr_info *p_info, size_t /*p_size*/, void *p_context)
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
	static int EnumerateModules2File(struct dl_phdr_info *p_info, size_t /*p_size*/, void *p_context)
	{
		std::ofstream& tfile = *reinterpret_cast<std::ofstream*>(p_context);
		push_hex_fix(tfile, reinterpret_cast<uintptr_t>(p_info->dlpi_addr));
		//tfile.put(' ');
		//push_hex_fix(tfile, BaseOfDll + ModuleSize);
		tfile.write(" \"", 2);
		push_data(tfile, os_string_view{p_info->dlpi_name});
		tfile.write("\"\n", 2);
		return 0;
	}

	///	\brief This function prints 1 stack addresses and associated function name,
	///		first it pretyfies the address to make it more easy to correlated with the module it has been loaded from,
	///		secondly it tries to get the name of the symbol to which this address belongs too.
	///	\warning Function names may not necessarily be correct. \n
	///		Name is determined based on last know name prior to current address, which is not necessarily the same function as the one the address belongs to.
	///		Use .map files to confirm names.
	static void print_function_addr(void* p_address, char* p_name, std::ofstream& p_file)
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
			push_hex_fix(p_file, reinterpret_cast<uintptr_t>(t_info.dli_fbase));
			p_file.put('+');
			push_hex_fix(p_file, diff);
		}
		else
		{
			push_hex_fix(p_file, reinterpret_cast<uintptr_t>(p_address));
		}
		
		//do not forget to print the symbol name, if there is one
		if(p_name)
		{
			p_file << ' ' << p_name;
		}
		p_file << '\n';
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
	static void Linux_exception_handler(int sig, siginfo_t *siginfo, void *context)
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
				std::ofstream	o_file;						// file to be output
				DateTime		t_time = date_time_local();	//current date

				{
					std::error_code ec;
					create_directories(g_straceOpt.m_output_file.parent_path(), ec);
					o_file.open(g_straceOpt.m_output_file);
				}

				if(o_file.is_open())
				{
					std::string_view section_seperator = "-------- -------- -------- --------\n";

					o_file << section_seperator;
					push_time(o_file, t_time);

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

					o_file	<< "Sig:\t\t0x";
					push_hex_fix(o_file, static_cast<uint32_t>(sig)); //the signal that caused the crash
					o_file	<< "\nCode:\t\t0x";
					push_hex_fix(o_file, static_cast<uint32_t>(siginfo->si_code));	//the additional code associated with the signal
					o_file.put('\n');
																					//for example the type of floating point error
					//see: http://linux.die.net/man/2/sigaction

					//print the address where the problem occurred, if there is one
					if(t_criticalAddr)
					{
						o_file	<< "Address:\t0x";
						push_hex_fix(o_file, reinterpret_cast<uintptr_t>(t_criticalAddr));
						o_file.put('\n');
					}

					//commits current known data to file
					//prevents from getting nothing if anything further causes a fatal terminate
					o_file << std::flush;

					o_file	<< section_seperator
							<< "Modules:" << '\n';
					
					//lists all loaded modules, calls Core_EnumerateModules2File per loaded module, and then some
					//see: http://linux.die.net/man/3/dl_iterate_phdr
					dl_iterate_phdr(EnumerateModules2File, &o_file);

					o_file	<< section_seperator
							<< "Stack:" << '\n';


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
					o_file << std::flush;

					uint32_t	proc_ID		= getpid();
					pthread_t	thread_ID	= pthread_self();

					o_file << section_seperator;
					o_file << "Proc:\t\t";
					push_dec(o_file, static_cast<uint32_t>(proc_ID));
					o_file << "\nThread:\t\t";
					push_dec(o_file, static_cast<uint32_t>(thread_ID));


					{
						std::array<char, 1024> buff;
						ssize_t ret_size = readlink("/proc/self/exe", buff.data(), buff.size());
						if(ret_size > 0)
						{
							o_file << "\nProcDir:\t\"" << std::string_view{buff.data(), static_cast<size_t>(ret_size)} << '\"';
						}
						//else too large
					}

					{
						std::error_code ec;
						o_file << "\nWorkDir:\t\"";
						push_data(o_file, std::filesystem::current_path(ec).native());
						o_file.write("\"\n", 2);
					}

					{
						const std::optional<core::os_string>& res = machine_name();
						if(res.has_value())
						{
							o_file	<< "Machine:\t\"";
							push_data(o_file, res.value());
							o_file.write("\"\n", 2);
						}
					}

					/*
						TODO: put file descriptor count here
					*/

					PrintCMD(o_file);	//Prints the commands that were passed to the application

					o_file	<< section_seperator;
					o_file	<< "Env:" << '\n';

					PrintEnv(o_file);	//Prints the environment variables at this moment

					o_file	<< section_seperator;
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
		g_straceOpt.m_output_file = (core::application_path().parent_path() / p_output_file).lexically_normal();
	}

	//======== IMPORTANT ========
	//this sets up an alternative stack to run on when we are doing our stack trace
	//see article:https://spin.atomicobject.com/2013/01/13/exceptions-stack-traces-c/
	stack_t ss;
	ss.ss_sp	= (void*) Except_stack;
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

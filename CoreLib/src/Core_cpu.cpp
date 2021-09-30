#include <CoreLib/Core_cpu.hpp>

#if defined(_M_AMD64) || defined(__amd64__)
#	if defined(_WIN64)
#		include <intrin.h>
#	else
#		include <cpuid.h>
#	endif
#endif



namespace core
{
#if defined(_M_AMD64) || defined(__amd64__)

	struct CPU_Data
	{

	};








	namespace
	{
		struct EX_Reg
		{
			uint32_t eax;
			uint32_t ebx;
			uint32_t ecx;
			uint32_t edx;
		};

#if !defined(_WIN32)
		static inline void direct_cpu_id(
			uint32_t p_leaf,
			uint32_t& p_eax,
			uint32_t& p_ebx,
			uint32_t& p_ecx,
			uint32_t& p_edx)
		{
			__cpuid (p_leaf, p_eax, p_ebx, p_ecx, p_edx);
		}
#endif


		static void get_cpu_id(EX_Reg& p_registers, uint32_t p_leaf)
		{
#if defined(_WIN32)
			__cpuid(reinterpret_cast<int*>(&p_registers.eax), reinterpret_cast<int&>(p_leaf));
#else
			direct_cpu_id(
				p_leaf,
				p_registers.eax,
				p_registers.ebx,
				p_registers.ecx,
				p_registers.edx);
#endif
		}

		static void get_cpu_id_ex(EX_Reg& p_registers, uint32_t p_leaf, uint32_t p_subleaf)
		{
			__cpuidex(reinterpret_cast<int*>(&p_registers.eax), reinterpret_cast<int&>(p_leaf), reinterpret_cast<int&>(p_subleaf));
		}


		static CPU_Data get_cpu_features()
		{
			CPU_Data outp;



			return outp;
		}

		static CPU_Data g_cpu_data;

	} //namespace
#else

#endif

} //namespace core

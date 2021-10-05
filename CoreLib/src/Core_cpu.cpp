#include <CoreLib/Core_cpu.hpp>

#include <array>
#include <bitset>
#include <cstdint>

#include <CoreLib/Core_Type.hpp>


#if defined(_M_AMD64) || defined(__amd64__)
#	if defined(_WIN64)
#		include <intrin.h>
#	else
#		include <cpuid.h>
#	endif
#endif



namespace core
{
	using literals::operator ""_ui32;

#if defined(_M_AMD64) || defined(__amd64__)

	struct CPU_Data
	{
		alignas(alignof(uint32_t)) std::array<char8_t, 16> vendor{0};

		uint8_t cpu_count	= 0;

		bool m_SSE3			= false;
		bool m_PCLMULQDQ	= false;
		bool m_MONITOR		= false;
		bool m_SSSE3		= false;
		bool m_FMA			= false;
		bool m_CMPXCHG16B	= false;
		bool m_PCID			= false;
		bool m_SSE41		= false;
		bool m_SSE42		= false;
		bool m_X2APIC		= false;
		bool m_MOVBE		= false;
		bool m_POPCNT		= false;
		bool m_AES			= false;
		bool m_XSAVE		= false;
		bool m_OSXSAVE		= false;
		bool m_AVX			= false;
		bool m_F16C			= false;
		bool m_RDRAND		= false;

		bool m_FPU			= false;
		bool m_VME			= false;
		bool m_DE			= false;
		bool m_PSE			= false;
		bool m_TSC			= false;
		bool m_MSR			= false;
		bool m_PAE			= false;
		bool m_MCE			= false;
		bool m_CMPXCHG8B	= false;
		bool m_APIC			= false;
		bool m_SysESysE		= false;
		bool m_MTRR			= false;
		bool m_PGE			= false;
		bool m_MCA			= false;
		bool m_CMOV			= false;
		bool m_PAT			= false;
		bool m_PSE36		= false;
		bool m_CLFSH		= false;
		bool m_MMX			= false;
		bool m_FXSR			= false;
		bool m_SSE			= false;
		bool m_SSE2			= false;
		bool m_HTT			= false;

		bool m_FSGSBASE		= false;
		bool m_BMI1			= false;
		bool m_HLE			= false;
		bool m_AVX2			= false;
		bool m_SMEP			= false;
		bool m_BMI2			= false;
		bool m_ERMS			= false;
		bool m_INVPCID		= false;
		bool m_RTM			= false;
		bool m_PQM			= false;
		bool m_PQE			= false;
		bool m_AVX512F		= false;
		bool m_RDSEED		= false;
		bool m_ADX			= false;
		bool m_SMAP			= false;
		bool m_AVX512PF		= false;
		bool m_AVX512ER		= false;
		bool m_AVX512CD		= false;
		bool m_SHA			= false;

		bool m_PREFETCHWT1	= false;
		bool m_PKU			= false;
		bool m_CET_SS		= false;
		bool m_VAES			= false;
		bool m_VPCLMULQDQ	= false;
		bool m_RDPID		= false;

		bool m_FSRM			= false;
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
			EX_Reg reg;
			// ---- Basic data ----
			{
				get_cpu_id(reg, 0);
				const uint32_t maxId = reg.eax;

				*reinterpret_cast<uint32_t*>(outp.vendor.data()) = reg.ebx;
				*reinterpret_cast<uint32_t*>(outp.vendor.data() + sizeof(uint32_t)) = reg.edx;
				*reinterpret_cast<uint32_t*>(outp.vendor.data() + sizeof(uint32_t) * 2) = reg.ecx;

				if(maxId > 0)
				{
					get_cpu_id(reg, 0x01);
					outp.cpu_count = static_cast<uint8_t>(reg.ebx >> 16);
					{
						const std::bitset<32> data = reg.ecx;
						outp.m_SSE3			= data[ 0];
						outp.m_PCLMULQDQ	= data[ 1];
						outp.m_MONITOR		= data[ 3];
						outp.m_SSSE3		= data[ 9];
						outp.m_FMA			= data[12];
						outp.m_CMPXCHG16B	= data[13];
						outp.m_PCID			= data[17];
						outp.m_SSE41		= data[19];
						outp.m_SSE42		= data[20];
						outp.m_X2APIC		= data[21];
						outp.m_MOVBE		= data[22];
						outp.m_POPCNT		= data[23];
						outp.m_AES			= data[25];
						outp.m_XSAVE		= data[26];
						outp.m_OSXSAVE		= data[27];
						outp.m_AVX			= data[28];
						outp.m_F16C			= data[29];
						outp.m_RDRAND		= data[30];
					}
					{
						const std::bitset<32> data = reg.edx;
						outp.m_FPU			= data[ 0];
						outp.m_VME			= data[ 1];
						outp.m_DE			= data[ 2];
						outp.m_PSE			= data[ 3];
						outp.m_TSC			= data[ 4];
						outp.m_MSR			= data[ 5];
						outp.m_PAE			= data[ 6];
						outp.m_MCE			= data[ 7];
						outp.m_CMPXCHG8B	= data[ 8];
						outp.m_APIC			= data[ 9];
						outp.m_SysESysE		= data[11];
						outp.m_MTRR			= data[12];
						outp.m_PGE			= data[13];
						outp.m_MCA			= data[14];
						outp.m_CMOV			= data[15];
						outp.m_PAT			= data[16];
						outp.m_PSE36		= data[17];
						outp.m_CLFSH		= data[19];
						outp.m_MMX			= data[23];
						outp.m_FXSR			= data[24];
						outp.m_SSE			= data[25];
						outp.m_SSE2			= data[26];
						outp.m_HTT			= data[28];
					}

					if(maxId > 6)
					{
						get_cpu_id_ex(reg, 0x07, 0x00);
						{
							const std::bitset<32> data = reg.ebx;
							outp.m_FSGSBASE		= data[ 0];
							outp.m_BMI1			= data[ 3];
							outp.m_HLE			= data[ 4];
							outp.m_AVX2			= data[ 5];
							outp.m_SMEP			= data[ 7];
							outp.m_BMI2			= data[ 8];
							outp.m_ERMS			= data[ 9];
							outp.m_INVPCID		= data[10];
							outp.m_RTM			= data[11];
							outp.m_PQM			= data[12];
							outp.m_PQE			= data[15];
							outp.m_AVX512F		= data[16];
							outp.m_RDSEED		= data[18];
							outp.m_ADX			= data[19];
							outp.m_SMAP			= data[20];
							outp.m_AVX512PF		= data[26];
							outp.m_AVX512ER		= data[27];
							outp.m_AVX512CD		= data[28];
							outp.m_SHA			= data[29];
						}
						{
							const std::bitset<32> data = reg.ecx;
							outp.m_PREFETCHWT1	= data[ 0];
							outp.m_PKU			= data[ 3];
							outp.m_CET_SS		= data[ 7];
							outp.m_VAES			= data[ 9];
							outp.m_VPCLMULQDQ	= data[10];
							outp.m_RDPID		= data[22];
						}
						{
							const std::bitset<32> data = reg.edx;
							outp.m_FSRM			= data[ 4];
						}
					}
				}
			}

			// ---- Extra data ----
			//{
			//	get_cpu_id(reg, 0x80000000_ui32);
			//	const uint32_t maxId = reg.eax;
			//
			//	if(maxId > 0x80000000_ui32)
			//	{
			//
			//		if(maxId > 0x80000003_ui32)
			//		{
			//		}
			//	}
			//}
			return outp;
		}

		static CPU_Data g_cpu_data = get_cpu_features();

	} //namespace
#else

#endif

} //namespace core

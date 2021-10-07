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

	namespace amd64
	{
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

		void cpu_id(EX_Reg& p_registers, uint32_t p_leaf)
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

		void cpu_id_ex(EX_Reg& p_registers, uint32_t p_leaf, uint32_t p_subleaf)
		{
			__cpuidex(reinterpret_cast<int*>(&p_registers.eax), reinterpret_cast<int&>(p_leaf), reinterpret_cast<int&>(p_subleaf));
		}


	namespace
	{
		struct CPU_Data
		{
			EX_Reg m_Fn0 {0_ui32, 0_ui32, 0_ui32, 0_ui32};
			EX_Reg m_Fn1 {0_ui32, 0_ui32, 0_ui32, 0_ui32};
			EX_Reg m_Fn7 {0_ui32, 0_ui32, 0_ui32, 0_ui32};

			alignas(alignof(uint32_t)) std::array<char8_t, 16> vendor{0};

			uint8_t cpu_count	= 0;

			bool m_SSE3				= false;
			bool m_PCLMULQDQ		= false;
			bool m_MONITOR			= false;
			bool m_VMX				= false;
			bool m_SMX				= false;
			bool m_SSSE3			= false;
			bool m_FMA				= false;
			bool m_CMPXCHG16B		= false;
			bool m_PCID				= false;
			bool m_SSE41			= false;
			bool m_SSE42			= false;
			bool m_X2APIC			= false;
			bool m_MOVBE			= false;
			bool m_POPCNT			= false;
			bool m_AES				= false;
			bool m_XSAVE			= false;
			bool m_OSXSAVE			= false;
			bool m_AVX				= false;
			bool m_F16C				= false;
			bool m_RDRAND			= false;

			bool m_FPU				= false;
			bool m_VME				= false;
			bool m_DE				= false;
			bool m_PSE				= false;
			bool m_TSC				= false;
			bool m_MSR				= false;
			bool m_PAE				= false;
			bool m_MCE				= false;
			bool m_CMPXCHG8B		= false;
			bool m_APIC				= false;
			bool m_SysESysE			= false;
			bool m_MTRR				= false;
			bool m_PGE				= false;
			bool m_MCA				= false;
			bool m_CMOV				= false;
			bool m_PAT				= false;
			bool m_PSE36			= false;
			bool m_PSN				= false;
			bool m_CLFSH			= false;
			bool m_MMX				= false;
			bool m_FXSR				= false;
			bool m_SSE				= false;
			bool m_SSE2				= false;
			bool m_HTT				= false;

			bool m_FSGSBASE			= false;
			bool m_SGX				= false;
			bool m_BMI1				= false;
			bool m_HLE				= false;
			bool m_AVX2				= false;
			bool m_SMEP				= false;
			bool m_BMI2				= false;
			bool m_ERMS				= false;
			bool m_INVPCID			= false;
			bool m_RTM				= false;
			bool m_AVX512F			= false;
			bool m_AVX512DQ			= false;
			bool m_RDSEED			= false;
			bool m_ADX				= false;
			bool m_SMAP				= false;
			bool m_AVX512_IFMA		= false;
			bool m_AVX512PF			= false;
			bool m_AVX512ER			= false;
			bool m_AVX512CD			= false;
			bool m_SHA				= false;
			bool m_AVX512BW			= false;
			bool m_AVX512VL			= false;

			bool m_PREFETCHWT1		= false;
			bool m_AVX512_VBMI		= false;
			bool m_UMIP				= false;
			bool m_PKU				= false;
			bool m_WAITPKG			= false;
			bool m_AVX512_VBMI2		= false;
			bool m_CET_SS			= false;
			bool m_GFNI				= false;
			bool m_VAES				= false;
			bool m_VPCLMULQDQ		= false;
			bool m_AVX512_VNNI		= false;
			bool m_AVX512_BITLAG	= false;
			bool m_AVX512_VPOPCNTDQ	= false;
			bool m_RDPID			= false;
			bool m_KL				= false;
			bool m_MOVDIRI			= false;
			bool m_MOVDIR64B		= false;
			bool m_ENQCMD			= false;
			bool m_SGX_LC			= false;
			bool m_PKS				= false;

			bool m_FSRM					= false;
			bool m_UINTR				= false;
			bool m_AVX512_VP2INTERSECT	= false;
			bool m_AMX_BF16				= false;
			bool m_AVX512_FP16			= false;
			bool m_AMX_TILE				= false;
			bool m_AMX_INT8				= false;
		};

		static CPU_Data get_cpu_features()
		{
			CPU_Data outp;
			// ---- Basic data ----
			{
				cpu_id(outp.m_Fn0, 0);
				const uint32_t maxId = outp.m_Fn0.eax;

				*reinterpret_cast<uint32_t*>(outp.vendor.data()) = outp.m_Fn0.ebx;
				*reinterpret_cast<uint32_t*>(outp.vendor.data() + sizeof(uint32_t)) = outp.m_Fn0.edx;
				*reinterpret_cast<uint32_t*>(outp.vendor.data() + sizeof(uint32_t) * 2) = outp.m_Fn0.ecx;

				if(maxId > 0)
				{
					cpu_id(outp.m_Fn1, 0x01);
					outp.cpu_count = static_cast<uint8_t>(outp.m_Fn1.ebx >> 16);
					{
						const std::bitset<32> data = outp.m_Fn1.ecx;
						outp.m_SSE3			= data[ 0];
						outp.m_PCLMULQDQ	= data[ 1];
						outp.m_MONITOR		= data[ 3];
						outp.m_VMX			= data[ 5];
						outp.m_SMX			= data[ 6];
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
						const std::bitset<32> data = outp.m_Fn1.edx;
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
						outp.m_PSN			= data[18];
						outp.m_CLFSH		= data[19];
						outp.m_MMX			= data[23];
						outp.m_FXSR			= data[24];
						outp.m_SSE			= data[25];
						outp.m_SSE2			= data[26];
						outp.m_HTT			= data[28];
					}

					if(maxId > 6)
					{
						cpu_id_ex(outp.m_Fn7, 0x07, 0x00);
						{
							const std::bitset<32> data = outp.m_Fn7.ebx;
							outp.m_FSGSBASE		= data[ 0];
							outp.m_SGX			= data[ 2];
							outp.m_BMI1			= data[ 3];
							outp.m_HLE			= data[ 4];
							outp.m_AVX2			= data[ 5];
							outp.m_SMEP			= data[ 7];
							outp.m_BMI2			= data[ 8];
							outp.m_ERMS			= data[ 9];
							outp.m_INVPCID		= data[10];
							outp.m_RTM			= data[11];
							outp.m_AVX512F		= data[16];
							outp.m_AVX512DQ		= data[17];
							outp.m_RDSEED		= data[18];
							outp.m_ADX			= data[19];
							outp.m_SMAP			= data[20];
							outp.m_AVX512_IFMA	= data[21];
							outp.m_AVX512PF		= data[26];
							outp.m_AVX512ER		= data[27];
							outp.m_AVX512CD		= data[28];
							outp.m_SHA			= data[29];
							outp.m_AVX512BW		= data[30];
							outp.m_AVX512VL		= data[31];
						}
						{
							const std::bitset<32> data = outp.m_Fn7.ecx;
							outp.m_PREFETCHWT1		= data[ 0];
							outp.m_AVX512_VBMI		= data[ 1];
							outp.m_UMIP				= data[ 2];
							outp.m_PKU				= data[ 3];
							outp.m_WAITPKG			= data[ 5];
							outp.m_AVX512_VBMI2		= data[ 6];
							outp.m_CET_SS			= data[ 7];
							outp.m_GFNI				= data[ 8];
							outp.m_VAES				= data[ 9];
							outp.m_VPCLMULQDQ		= data[10];
							outp.m_AVX512_VNNI		= data[11];
							outp.m_AVX512_BITLAG	= data[12];
							outp.m_AVX512_VPOPCNTDQ	= data[14];
							outp.m_RDPID			= data[22];
							outp.m_KL				= data[23];
							outp.m_MOVDIRI			= data[27];
							outp.m_MOVDIR64B		= data[28];
							outp.m_ENQCMD			= data[29];
							outp.m_SGX_LC			= data[30];
							outp.m_PKS				= data[31];
						}
						{
							const std::bitset<32> data = outp.m_Fn7.edx;
							outp.m_FSRM					= data[ 4];
							outp.m_UINTR				= data[ 5];
							outp.m_AVX512_VP2INTERSECT	= data[ 8];
							outp.m_AMX_BF16				= data[22];
							outp.m_AVX512_FP16			= data[23];
							outp.m_AMX_TILE				= data[24];
							outp.m_AMX_INT8				= data[25];
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

	std::u8string_view CPU_feature_g::vendor() { return std::u8string_view{g_cpu_data.vendor.data()}; }
	uint8_t CPU_feature_g::cpu_count() { return g_cpu_data.cpu_count	; }
	bool CPU_feature_g::SSE3		() { return g_cpu_data.m_SSE3		; }
	bool CPU_feature_g::PCLMULQDQ	() { return g_cpu_data.m_PCLMULQDQ	; }
	bool CPU_feature_g::MONITOR		() { return g_cpu_data.m_MONITOR	; }
	bool CPU_feature_g::VMX			() { return g_cpu_data.m_VMX		; }
	bool CPU_feature_g::SMX			() { return g_cpu_data.m_SMX		; }
	bool CPU_feature_g::SSSE3		() { return g_cpu_data.m_SSSE3		; }
	bool CPU_feature_g::FMA			() { return g_cpu_data.m_FMA		; }
	bool CPU_feature_g::CMPXCHG16B	() { return g_cpu_data.m_CMPXCHG16B	; }
	bool CPU_feature_g::PCID		() { return g_cpu_data.m_PCID		; }
	bool CPU_feature_g::SSE41		() { return g_cpu_data.m_SSE41		; }
	bool CPU_feature_g::SSE42		() { return g_cpu_data.m_SSE42		; }
	bool CPU_feature_g::X2APIC		() { return g_cpu_data.m_X2APIC		; }
	bool CPU_feature_g::MOVBE		() { return g_cpu_data.m_MOVBE		; }
	bool CPU_feature_g::POPCNT		() { return g_cpu_data.m_POPCNT		; }
	bool CPU_feature_g::AES			() { return g_cpu_data.m_AES		; }
	bool CPU_feature_g::XSAVE		() { return g_cpu_data.m_XSAVE		; }
	bool CPU_feature_g::OSXSAVE		() { return g_cpu_data.m_OSXSAVE	; }
	bool CPU_feature_g::AVX			() { return g_cpu_data.m_AVX		; }
	bool CPU_feature_g::F16C		() { return g_cpu_data.m_F16C		; }
	bool CPU_feature_g::RDRAND		() { return g_cpu_data.m_RDRAND		; }

	bool CPU_feature_g::FPU			() { return g_cpu_data.m_FPU		; }
	bool CPU_feature_g::VME			() { return g_cpu_data.m_VME		; }
	bool CPU_feature_g::DE			() { return g_cpu_data.m_DE			; }
	bool CPU_feature_g::PSE			() { return g_cpu_data.m_PSE		; }
	bool CPU_feature_g::TSC			() { return g_cpu_data.m_TSC		; }
	bool CPU_feature_g::MSR			() { return g_cpu_data.m_MSR		; }
	bool CPU_feature_g::PAE			() { return g_cpu_data.m_PAE		; }
	bool CPU_feature_g::MCE			() { return g_cpu_data.m_MCE		; }
	bool CPU_feature_g::CMPXCHG8B	() { return g_cpu_data.m_CMPXCHG8B	; }
	bool CPU_feature_g::APIC		() { return g_cpu_data.m_APIC		; }
	bool CPU_feature_g::SysESysE	() { return g_cpu_data.m_SysESysE	; }
	bool CPU_feature_g::MTRR		() { return g_cpu_data.m_MTRR		; }
	bool CPU_feature_g::PGE			() { return g_cpu_data.m_PGE		; }
	bool CPU_feature_g::MCA			() { return g_cpu_data.m_MCA		; }
	bool CPU_feature_g::CMOV		() { return g_cpu_data.m_CMOV		; }
	bool CPU_feature_g::PAT			() { return g_cpu_data.m_PAT		; }
	bool CPU_feature_g::PSE36		() { return g_cpu_data.m_PSE36		; }
	bool CPU_feature_g::PSN			() { return g_cpu_data.m_PSN		; }
	bool CPU_feature_g::CLFSH		() { return g_cpu_data.m_CLFSH		; }
	bool CPU_feature_g::MMX			() { return g_cpu_data.m_MMX		; }
	bool CPU_feature_g::FXSR		() { return g_cpu_data.m_FXSR		; }
	bool CPU_feature_g::SSE			() { return g_cpu_data.m_SSE		; }
	bool CPU_feature_g::SSE2		() { return g_cpu_data.m_SSE2		; }
	bool CPU_feature_g::HTT			() { return g_cpu_data.m_HTT		; }

	bool CPU_feature_g::FSGSBASE	() { return g_cpu_data.m_FSGSBASE	; }
	bool CPU_feature_g::SGX			() { return g_cpu_data.m_SGX		; }
	bool CPU_feature_g::BMI1		() { return g_cpu_data.m_BMI1		; }
	bool CPU_feature_g::HLE			() { return g_cpu_data.m_HLE		; }
	bool CPU_feature_g::AVX2		() { return g_cpu_data.m_AVX2		; }
	bool CPU_feature_g::SMEP		() { return g_cpu_data.m_SMEP		; }
	bool CPU_feature_g::BMI2		() { return g_cpu_data.m_BMI2		; }
	bool CPU_feature_g::ERMS		() { return g_cpu_data.m_ERMS		; }
	bool CPU_feature_g::INVPCID		() { return g_cpu_data.m_INVPCID	; }
	bool CPU_feature_g::RTM			() { return g_cpu_data.m_RTM		; }
	bool CPU_feature_g::AVX512F		() { return g_cpu_data.m_AVX512F	; }
	bool CPU_feature_g::AVX512DQ	() { return g_cpu_data.m_AVX512DQ	; }
	bool CPU_feature_g::RDSEED		() { return g_cpu_data.m_RDSEED		; }
	bool CPU_feature_g::ADX			() { return g_cpu_data.m_ADX		; }
	bool CPU_feature_g::SMAP		() { return g_cpu_data.m_SMAP		; }
	bool CPU_feature_g::AVX512_IFMA	() { return g_cpu_data.m_AVX512_IFMA; }
	bool CPU_feature_g::AVX512PF	() { return g_cpu_data.m_AVX512PF	; }
	bool CPU_feature_g::AVX512ER	() { return g_cpu_data.m_AVX512ER	; }
	bool CPU_feature_g::AVX512CD	() { return g_cpu_data.m_AVX512CD	; }
	bool CPU_feature_g::SHA			() { return g_cpu_data.m_SHA		; }
	bool CPU_feature_g::AVX512BW	() { return g_cpu_data.m_AVX512BW	; }
	bool CPU_feature_g::AVX512VL	() { return g_cpu_data.m_AVX512VL	; }

	bool CPU_feature_g::PREFETCHWT1		() { return g_cpu_data.m_PREFETCHWT1		; }
	bool CPU_feature_g::AVX512_VBMI		() { return g_cpu_data.m_AVX512_VBMI		; }
	bool CPU_feature_g::UMIP			() { return g_cpu_data.m_UMIP				; }
	bool CPU_feature_g::PKU				() { return g_cpu_data.m_PKU				; }
	bool CPU_feature_g::WAITPKG			() { return g_cpu_data.m_WAITPKG			; }
	bool CPU_feature_g::AVX512_VBMI2	() { return g_cpu_data.m_AVX512_VBMI2		; }
	bool CPU_feature_g::CET_SS			() { return g_cpu_data.m_CET_SS				; }
	bool CPU_feature_g::GFNI			() { return g_cpu_data.m_GFNI				; }
	bool CPU_feature_g::VAES			() { return g_cpu_data.m_VAES				; }
	bool CPU_feature_g::VPCLMULQDQ		() { return g_cpu_data.m_VPCLMULQDQ			; }
	bool CPU_feature_g::AVX512_VNNI		() { return g_cpu_data.m_AVX512_VNNI		; }
	bool CPU_feature_g::AVX512_BITLAG	() { return g_cpu_data.m_AVX512_BITLAG		; }
	bool CPU_feature_g::AVX512_VPOPCNTDQ() { return g_cpu_data.m_AVX512_VPOPCNTDQ	; }
	bool CPU_feature_g::RDPID			() { return g_cpu_data.m_RDPID				; }
	bool CPU_feature_g::KL				() { return g_cpu_data.m_KL					; }
	bool CPU_feature_g::MOVDIRI			() { return g_cpu_data.m_MOVDIRI			; }
	bool CPU_feature_g::MOVDIR64B		() { return g_cpu_data.m_MOVDIR64B			; }
	bool CPU_feature_g::ENQCMD			() { return g_cpu_data.m_ENQCMD				; }
	bool CPU_feature_g::SGX_LC			() { return g_cpu_data.m_SGX_LC				; }
	bool CPU_feature_g::PKS				() { return g_cpu_data.m_PKS				; }

	bool CPU_feature_g::FSRM				() { return g_cpu_data.m_FSRM				; }
	bool CPU_feature_g::UINTR				() { return g_cpu_data.m_UINTR				; }
	bool CPU_feature_g::AVX512_VP2INTERSECT	() { return g_cpu_data.m_AVX512_VP2INTERSECT; }
	bool CPU_feature_g::AMX_BF16			() { return g_cpu_data.m_AMX_BF16			; }
	bool CPU_feature_g::AVX512_FP16			() { return g_cpu_data.m_AVX512_FP16		; }
	bool CPU_feature_g::AMX_TILE			() { return g_cpu_data.m_AMX_TILE			; }
	bool CPU_feature_g::AMX_INT8			() { return g_cpu_data.m_AMX_INT8			; }


	EX_Reg CPU_feature_g::Fn0		() { return g_cpu_data.m_Fn0; }
	EX_Reg CPU_feature_g::Fn1		() { return g_cpu_data.m_Fn1; }
	EX_Reg CPU_feature_g::Fn7		() { return g_cpu_data.m_Fn7; }





	template<uint32_t Fn, uint8_t Register, uint8_t Offset> requires ((Fn == 1 || Fn == 7) && (Register < 4) && (Offset < 32))
	static inline bool help_fecth_single_cpu_id_bit()
	{
		EX_Reg reg;
		cpu_id(reg, 0);

		if constexpr (Fn == 1)
		{
			if(reg.eax < 1)
			{
				return false;
			}
			cpu_id(reg, 0x01);
		}
		else if constexpr (Fn == 7)
		{
			if(reg.eax < 7)
			{
				return false;
			}
			cpu_id_ex(reg, 0x07, 0x00);
		}

		if constexpr (Register == 0)
		{
			return ((reg.eax >> Offset) & 1) ? true : false;
		}
		else if constexpr (Register == 1)
		{
			return ((reg.ebx >> Offset) & 1) ? true : false;
		}
		else if constexpr (Register == 2)
		{
			return ((reg.ecx >> Offset) & 1) ? true : false;
		}
		else if constexpr (Register == 3)
		{
			return ((reg.edx >> Offset) & 1) ? true : false;
		}
	}

	std::array<char8_t, 16> CPU_feature_su::vendor()
	{
		alignas(alignof(uint32_t)) std::array<char8_t, 16> outp{0};
		EX_Reg reg;
		cpu_id(reg, 0);

		*reinterpret_cast<uint32_t*>(outp.data()) = reg.ebx;
		*reinterpret_cast<uint32_t*>(outp.data() + sizeof(uint32_t)) = reg.edx;
		*reinterpret_cast<uint32_t*>(outp.data() + sizeof(uint32_t) * 2) = reg.ecx;

		return outp;
	}

	uint8_t CPU_feature_su::cpu_count()
	{
		EX_Reg reg;
		cpu_id(reg, 0);
		if(reg.eax > 0)
		{
			cpu_id(reg, 0x01);
			return static_cast<uint8_t>(reg.ebx >> 16);
		}
		return 0;
	}

	bool CPU_feature_su::SSE3			() { return help_fecth_single_cpu_id_bit<1, 1,  0>(); }
	bool CPU_feature_su::PCLMULQDQ		() { return help_fecth_single_cpu_id_bit<1, 1,  1>(); }
	bool CPU_feature_su::MONITOR		() { return help_fecth_single_cpu_id_bit<1, 1,  3>(); }
	bool CPU_feature_su::VMX			() { return help_fecth_single_cpu_id_bit<1, 1,  5>(); }
	bool CPU_feature_su::SMX			() { return help_fecth_single_cpu_id_bit<1, 1,  6>(); }
	bool CPU_feature_su::SSSE3			() { return help_fecth_single_cpu_id_bit<1, 1,  9>(); }
	bool CPU_feature_su::FMA			() { return help_fecth_single_cpu_id_bit<1, 1, 12>(); }
	bool CPU_feature_su::CMPXCHG16B		() { return help_fecth_single_cpu_id_bit<1, 1, 13>(); }
	bool CPU_feature_su::PCID			() { return help_fecth_single_cpu_id_bit<1, 1, 17>(); }
	bool CPU_feature_su::SSE41			() { return help_fecth_single_cpu_id_bit<1, 1, 19>(); }
	bool CPU_feature_su::SSE42			() { return help_fecth_single_cpu_id_bit<1, 1, 20>(); }
	bool CPU_feature_su::X2APIC			() { return help_fecth_single_cpu_id_bit<1, 1, 21>(); }
	bool CPU_feature_su::MOVBE			() { return help_fecth_single_cpu_id_bit<1, 1, 22>(); }
	bool CPU_feature_su::POPCNT			() { return help_fecth_single_cpu_id_bit<1, 1, 23>(); }
	bool CPU_feature_su::AES			() { return help_fecth_single_cpu_id_bit<1, 1, 25>(); }
	bool CPU_feature_su::XSAVE			() { return help_fecth_single_cpu_id_bit<1, 1, 26>(); }
	bool CPU_feature_su::OSXSAVE		() { return help_fecth_single_cpu_id_bit<1, 1, 27>(); }
	bool CPU_feature_su::AVX			() { return help_fecth_single_cpu_id_bit<1, 1, 28>(); }
	bool CPU_feature_su::F16C			() { return help_fecth_single_cpu_id_bit<1, 1, 29>(); }
	bool CPU_feature_su::RDRAND			() { return help_fecth_single_cpu_id_bit<1, 1, 30>(); }

	bool CPU_feature_su::FPU			() { return help_fecth_single_cpu_id_bit<1, 3,  0>(); }
	bool CPU_feature_su::VME			() { return help_fecth_single_cpu_id_bit<1, 3,  1>(); }
	bool CPU_feature_su::DE				() { return help_fecth_single_cpu_id_bit<1, 3,  2>(); }
	bool CPU_feature_su::PSE			() { return help_fecth_single_cpu_id_bit<1, 3,  3>(); }
	bool CPU_feature_su::TSC			() { return help_fecth_single_cpu_id_bit<1, 3,  4>(); }
	bool CPU_feature_su::MSR			() { return help_fecth_single_cpu_id_bit<1, 3,  5>(); }
	bool CPU_feature_su::PAE			() { return help_fecth_single_cpu_id_bit<1, 3,  6>(); }
	bool CPU_feature_su::MCE			() { return help_fecth_single_cpu_id_bit<1, 3,  7>(); }
	bool CPU_feature_su::CMPXCHG8B		() { return help_fecth_single_cpu_id_bit<1, 3,  8>(); }
	bool CPU_feature_su::APIC			() { return help_fecth_single_cpu_id_bit<1, 3,  9>(); }
	bool CPU_feature_su::SysESysE		() { return help_fecth_single_cpu_id_bit<1, 3, 11>(); }
	bool CPU_feature_su::MTRR			() { return help_fecth_single_cpu_id_bit<1, 3, 12>(); }
	bool CPU_feature_su::PGE			() { return help_fecth_single_cpu_id_bit<1, 3, 13>(); }
	bool CPU_feature_su::MCA			() { return help_fecth_single_cpu_id_bit<1, 3, 14>(); }
	bool CPU_feature_su::CMOV			() { return help_fecth_single_cpu_id_bit<1, 3, 15>(); }
	bool CPU_feature_su::PAT			() { return help_fecth_single_cpu_id_bit<1, 3, 16>(); }
	bool CPU_feature_su::PSE36			() { return help_fecth_single_cpu_id_bit<1, 3, 17>(); }
	bool CPU_feature_su::PSN			() { return help_fecth_single_cpu_id_bit<1, 3, 18>(); }
	bool CPU_feature_su::CLFSH			() { return help_fecth_single_cpu_id_bit<1, 3, 19>(); }
	bool CPU_feature_su::MMX			() { return help_fecth_single_cpu_id_bit<1, 3, 23>(); }
	bool CPU_feature_su::FXSR			() { return help_fecth_single_cpu_id_bit<1, 3, 24>(); }
	bool CPU_feature_su::SSE			() { return help_fecth_single_cpu_id_bit<1, 3, 25>(); }
	bool CPU_feature_su::SSE2			() { return help_fecth_single_cpu_id_bit<1, 3, 26>(); }
	bool CPU_feature_su::HTT			() { return help_fecth_single_cpu_id_bit<1, 3, 28>(); }

	bool CPU_feature_su::FSGSBASE		() { return help_fecth_single_cpu_id_bit<7, 1,  0>(); }
	bool CPU_feature_su::SGX			() { return help_fecth_single_cpu_id_bit<7, 1,  2>(); }
	bool CPU_feature_su::BMI1			() { return help_fecth_single_cpu_id_bit<7, 1,  3>(); }
	bool CPU_feature_su::HLE			() { return help_fecth_single_cpu_id_bit<7, 1,  4>(); }
	bool CPU_feature_su::AVX2			() { return help_fecth_single_cpu_id_bit<7, 1,  5>(); }
	bool CPU_feature_su::SMEP			() { return help_fecth_single_cpu_id_bit<7, 1,  7>(); }
	bool CPU_feature_su::BMI2			() { return help_fecth_single_cpu_id_bit<7, 1,  8>(); }
	bool CPU_feature_su::ERMS			() { return help_fecth_single_cpu_id_bit<7, 1,  9>(); }
	bool CPU_feature_su::INVPCID		() { return help_fecth_single_cpu_id_bit<7, 1, 10>(); }
	bool CPU_feature_su::RTM			() { return help_fecth_single_cpu_id_bit<7, 1, 11>(); }
	bool CPU_feature_su::PQM			() { return help_fecth_single_cpu_id_bit<7, 1, 12>(); }
	bool CPU_feature_su::PQE			() { return help_fecth_single_cpu_id_bit<7, 1, 15>(); }
	bool CPU_feature_su::AVX512F		() { return help_fecth_single_cpu_id_bit<7, 1, 16>(); }
	bool CPU_feature_su::AVX512DQ		() { return help_fecth_single_cpu_id_bit<7, 1, 17>(); }
	bool CPU_feature_su::RDSEED			() { return help_fecth_single_cpu_id_bit<7, 1, 18>(); }
	bool CPU_feature_su::ADX			() { return help_fecth_single_cpu_id_bit<7, 1, 19>(); }
	bool CPU_feature_su::SMAP			() { return help_fecth_single_cpu_id_bit<7, 1, 20>(); }
	bool CPU_feature_su::AVX512_IFMA	() { return help_fecth_single_cpu_id_bit<7, 1, 21>(); }
	bool CPU_feature_su::AVX512PF		() { return help_fecth_single_cpu_id_bit<7, 1, 26>(); }
	bool CPU_feature_su::AVX512ER		() { return help_fecth_single_cpu_id_bit<7, 1, 27>(); }
	bool CPU_feature_su::AVX512CD		() { return help_fecth_single_cpu_id_bit<7, 1, 28>(); }
	bool CPU_feature_su::SHA			() { return help_fecth_single_cpu_id_bit<7, 1, 29>(); }
	bool CPU_feature_su::AVX512BW		() { return help_fecth_single_cpu_id_bit<7, 1, 30>(); }
	bool CPU_feature_su::AVX512VL		() { return help_fecth_single_cpu_id_bit<7, 1, 31>(); }

	bool CPU_feature_su::PREFETCHWT1		() { return help_fecth_single_cpu_id_bit<7, 2,  0>(); }
	bool CPU_feature_su::AVX512_VBMI		() { return help_fecth_single_cpu_id_bit<7, 2,  1>(); }
	bool CPU_feature_su::UMIP				() { return help_fecth_single_cpu_id_bit<7, 2,  2>(); }
	bool CPU_feature_su::PKU				() { return help_fecth_single_cpu_id_bit<7, 2,  3>(); }
	bool CPU_feature_su::WAITPKG			() { return help_fecth_single_cpu_id_bit<7, 2,  5>(); }
	bool CPU_feature_su::AVX512_VBMI2		() { return help_fecth_single_cpu_id_bit<7, 2,  6>(); }
	bool CPU_feature_su::CET_SS				() { return help_fecth_single_cpu_id_bit<7, 2,  7>(); }
	bool CPU_feature_su::GFNI				() { return help_fecth_single_cpu_id_bit<7, 2,  8>(); }
	bool CPU_feature_su::VAES				() { return help_fecth_single_cpu_id_bit<7, 2,  9>(); }
	bool CPU_feature_su::VPCLMULQDQ			() { return help_fecth_single_cpu_id_bit<7, 2, 10>(); }
	bool CPU_feature_su::AVX512_VNNI		() { return help_fecth_single_cpu_id_bit<7, 2, 11>(); }
	bool CPU_feature_su::AVX512_BITLAG		() { return help_fecth_single_cpu_id_bit<7, 2, 12>(); }
	bool CPU_feature_su::AVX512_VPOPCNTDQ	() { return help_fecth_single_cpu_id_bit<7, 2, 14>(); }
	bool CPU_feature_su::RDPID				() { return help_fecth_single_cpu_id_bit<7, 2, 22>(); }
	bool CPU_feature_su::KL					() { return help_fecth_single_cpu_id_bit<7, 2, 23>(); }
	bool CPU_feature_su::MOVDIRI			() { return help_fecth_single_cpu_id_bit<7, 2, 27>(); }
	bool CPU_feature_su::MOVDIR64B			() { return help_fecth_single_cpu_id_bit<7, 2, 28>(); }
	bool CPU_feature_su::ENQCMD				() { return help_fecth_single_cpu_id_bit<7, 2, 29>(); }
	bool CPU_feature_su::SGX_LC				() { return help_fecth_single_cpu_id_bit<7, 2, 30>(); }
	bool CPU_feature_su::PKS				() { return help_fecth_single_cpu_id_bit<7, 2, 31>(); }

	bool CPU_feature_su::FSRM				() { return help_fecth_single_cpu_id_bit<7, 3,  4>(); }
	bool CPU_feature_su::UINTR				() { return help_fecth_single_cpu_id_bit<7, 3,  5>(); }
	bool CPU_feature_su::AVX512_VP2INTERSECT() { return help_fecth_single_cpu_id_bit<7, 3,  8>(); }
	bool CPU_feature_su::AMX_BF16			() { return help_fecth_single_cpu_id_bit<7, 3, 22>(); }
	bool CPU_feature_su::AVX512_FP16		() { return help_fecth_single_cpu_id_bit<7, 3, 23>(); }
	bool CPU_feature_su::AMX_TILE			() { return help_fecth_single_cpu_id_bit<7, 3, 24>(); }
	bool CPU_feature_su::AMX_INT8			() { return help_fecth_single_cpu_id_bit<7, 3, 25>(); }

	EX_Reg CPU_feature_su::Fn0()
	{
		EX_Reg reg;
		cpu_id(reg, 0);
		return reg;
	}

	EX_Reg CPU_feature_su::Fn1()
	{
		EX_Reg reg;
		cpu_id(reg, 0);

		if(reg.eax < 1)
		{
			return EX_Reg{0, 0, 0, 0};
		}
		cpu_id(reg, 0x01);
		return reg;
	}

	EX_Reg CPU_feature_su::Fn7()
	{
		EX_Reg reg;
		cpu_id(reg, 0);

		if(reg.eax < 7)
		{
			return EX_Reg{0, 0, 0, 0};
		}
		cpu_id_ex(reg, 0x07, 0x00);
		return reg;
	}

} //namespace amd64

#endif

} //namespace core

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

#include <array>
#include <cstdint>
#include <string_view>

namespace core
{
#if defined(_M_AMD64) || defined(__amd64__)

	namespace amd64
	{
		struct EX_Reg
		{
			uint32_t eax;
			uint32_t ebx;
			uint32_t ecx;
			uint32_t edx;
		};

		void cpu_id   (EX_Reg& p_registers, uint32_t p_leaf);
		void cpu_id_ex(EX_Reg& p_registers, uint32_t p_leaf, uint32_t p_subleaf);

		// \brief use this for collecting a single use datapoint
		// no caching involved, cpu_id is always called
		// slower but safe to use in global initialization
		struct CPU_feature_su
		{
			static std::array<char8_t, 16> vendor();
			static uint8_t cpu_count();
			static bool SSE3		();
			static bool PCLMULQDQ	();
			static bool MONITOR		();
			static bool VMX			();
			static bool SMX			();
			static bool SSSE3		();
			static bool FMA			();
			static bool CMPXCHG16B	();
			static bool PCID		();
			static bool SSE41		();
			static bool SSE42		();
			static bool X2APIC		();
			static bool MOVBE		();
			static bool POPCNT		();
			static bool AES			();
			static bool XSAVE		();
			static bool OSXSAVE		();
			static bool AVX			();
			static bool F16C		();
			static bool RDRAND		();

			static bool FPU			();
			static bool VME			();
			static bool DE			();
			static bool PSE			();
			static bool TSC			();
			static bool MSR			();
			static bool PAE			();
			static bool MCE			();
			static bool CMPXCHG8B	();
			static bool APIC		();
			static bool SysESysE	();
			static bool MTRR		();
			static bool PGE			();
			static bool MCA			();
			static bool CMOV		();
			static bool PAT			();
			static bool PSE36		();
			static bool PSN			();
			static bool CLFSH		();
			static bool MMX			();
			static bool FXSR		();
			static bool SSE			();
			static bool SSE2		();
			static bool HTT			();

			static bool FSGSBASE	();
			static bool SGX			();
			static bool BMI1		();
			static bool HLE			();
			static bool AVX2		();
			static bool SMEP		();
			static bool BMI2		();
			static bool ERMS		();
			static bool INVPCID		();
			static bool RTM			();
			static bool PQM			();
			static bool PQE			();
			static bool AVX512F		();
			static bool AVX512DQ	();
			static bool RDSEED		();
			static bool ADX			();
			static bool SMAP		();
			static bool AVX512_IFMA	();
			static bool AVX512PF	();
			static bool AVX512ER	();
			static bool AVX512CD	();
			static bool SHA			();
			static bool AVX512BW	();
			static bool AVX512VL	();

			static bool PREFETCHWT1		();
			static bool AVX512_VBMI		();
			static bool UMIP			();
			static bool PKU				();
			static bool WAITPKG			();
			static bool AVX512_VBMI2	();
			static bool CET_SS			();
			static bool GFNI			();
			static bool VAES			();
			static bool VPCLMULQDQ		();
			static bool AVX512_VNNI		();
			static bool AVX512_BITLAG	();
			static bool AVX512_VPOPCNTDQ();
			static bool RDPID			();
			static bool KL				();
			static bool MOVDIRI			();
			static bool MOVDIR64B		();
			static bool ENQCMD			();
			static bool SGX_LC			();
			static bool PKS				();

			static bool FSRM				();
			static bool UINTR				();
			static bool AVX512_VP2INTERSECT	();
			static bool AMX_BF16			();
			static bool AVX512_FP16			();
			static bool AMX_TILE			();
			static bool AMX_INT8			();

			static EX_Reg Fn0();
			static EX_Reg Fn1();
			static EX_Reg Fn7();
		};

		// \brief consults a cached value
		// fast, but undefined behaviour may occur if used in global variable initialization
		struct CPU_feature_g
		{
			static std::u8string_view vendor();
			static uint8_t cpu_count();
			static bool SSE3		();
			static bool PCLMULQDQ	();
			static bool MONITOR		();
			static bool VMX			();
			static bool SMX			();
			static bool SSSE3		();
			static bool FMA			();
			static bool CMPXCHG16B	();
			static bool PCID		();
			static bool SSE41		();
			static bool SSE42		();
			static bool X2APIC		();
			static bool MOVBE		();
			static bool POPCNT		();
			static bool AES			();
			static bool XSAVE		();
			static bool OSXSAVE		();
			static bool AVX			();
			static bool F16C		();
			static bool RDRAND		();

			static bool FPU			();
			static bool VME			();
			static bool DE			();
			static bool PSE			();
			static bool TSC			();
			static bool MSR			();
			static bool PAE			();
			static bool MCE			();
			static bool CMPXCHG8B	();
			static bool APIC		();
			static bool SysESysE	();
			static bool MTRR		();
			static bool PGE			();
			static bool MCA			();
			static bool CMOV		();
			static bool PAT			();
			static bool PSE36		();
			static bool PSN			();
			static bool CLFSH		();
			static bool MMX			();
			static bool FXSR		();
			static bool SSE			();
			static bool SSE2		();
			static bool HTT			();

			static bool FSGSBASE	();
			static bool SGX			();
			static bool BMI1		();
			static bool HLE			();
			static bool AVX2		();
			static bool SMEP		();
			static bool BMI2		();
			static bool ERMS		();
			static bool INVPCID		();
			static bool RTM			();
			static bool AVX512F		();
			static bool AVX512DQ	();
			static bool RDSEED		();
			static bool ADX			();
			static bool SMAP		();
			static bool AVX512_IFMA	();
			static bool AVX512PF	();
			static bool AVX512ER	();
			static bool AVX512CD	();
			static bool SHA			();
			static bool AVX512BW	();
			static bool AVX512VL	();

			static bool PREFETCHWT1		();
			static bool AVX512_VBMI		();
			static bool UMIP			();
			static bool PKU				();
			static bool WAITPKG			();
			static bool AVX512_VBMI2	();
			static bool CET_SS			();
			static bool GFNI			();
			static bool VAES			();
			static bool VPCLMULQDQ		();
			static bool AVX512_VNNI		();
			static bool AVX512_BITLAG	();
			static bool AVX512_VPOPCNTDQ();
			static bool RDPID			();
			static bool KL				();
			static bool MOVDIRI			();
			static bool MOVDIR64B		();
			static bool ENQCMD			();
			static bool SGX_LC			();
			static bool PKS				();

			static bool FSRM				();
			static bool UINTR				();
			static bool AVX512_VP2INTERSECT	();
			static bool AMX_BF16			();
			static bool AVX512_FP16			();
			static bool AMX_TILE			();
			static bool AMX_INT8			();

			static EX_Reg Fn0();
			static EX_Reg Fn1();
			static EX_Reg Fn7();
		};

	}

#endif

} //namespace core

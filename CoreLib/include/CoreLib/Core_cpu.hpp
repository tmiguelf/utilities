#pragma once

#include <array>
#include <string_view>

namespace core
{
#if defined(_M_AMD64) || defined(__amd64__)
	struct CPU_features
	{
		static std::u8string_view vendor();
		static std::u8string_view brand	();

		static bool SSE3		();
		static bool PCLMULQDQ	();
		static bool MONITOR		();
		static bool SSSE3		();
		static bool FMA			();
		static bool CMPXCHG16B	();
		static bool SSE41		();
		static bool SSE42		();
		static bool MOVBE		();
		static bool POPCNT		();
		static bool AES			();
		static bool XSAVE		();
		static bool OSXSAVE		();
		static bool AVX			();
		static bool F16C		();
		static bool RDRAND		();

		static bool MSR			();
		static bool CX8			();
		static bool SEP			();
		static bool CMOV		();
		static bool CLFSH		();
		static bool MMX			();
		static bool FXSR		();
		static bool SSE			();
		static bool SSE2		();

		static bool FSGSBASE	();
		static bool BMI1		();
		static bool HLE			();
		static bool AVX2		();
		static bool BMI2		();
		static bool ERMS		();
		static bool INVPCID		();
		static bool RTM			();
		static bool AVX512F		();
		static bool RDSEED		();
		static bool ADX			();
		static bool AVX512PF	();
		static bool AVX512ER	();
		static bool AVX512CD	();
		static bool SHA			();

		static bool PREFETCHWT1	();

#if 0
		static bool LAHF		();
		static bool LZCNT		();
		static bool ABM			();
		static bool SSE4a		();
		static bool XOP			();
		static bool TBM			();

		static bool SYSCALL		();
		static bool MMXEXT		();
		static bool RDTSCP		();
		static bool 3DNOWEXT	();
		static bool 3DNOW		();
#endif
	};


#else
	class CPU_features
	{

	};
#endif

} //namespace core

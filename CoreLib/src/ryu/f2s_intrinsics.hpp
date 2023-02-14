// Copyright 2018 Ulf Adams
//
// The contents of this file may be used under the terms of the Apache License,
// Version 2.0.
//
//    (See accompanying file LICENSE-Apache or copy at
//     http://www.apache.org/licenses/LICENSE-2.0)
//
// Alternatively, the contents of this file may be used under the terms of
// the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE-Boost or copy at
//     https://www.boost.org/LICENSE_1_0.txt)
//
// Unless required by applicable law or agreed to in writing, this software
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.
//
// Modified by: Tiago Freire

#pragma once

#if defined(RYU_FLOAT_FULL_TABLE)
#	include "f2s_full_table.hpp"
#else

#	if defined(RYU_OPTIMIZE_SIZE)
#		include "d2s_small_table.hpp"
#	else
#		include "d2s_full_table.hpp"
#	endif
constexpr uint16_t FLOAT_POW5_INV_BITCOUNT = (DOUBLE_POW5_INV_BITCOUNT - 64);
constexpr uint16_t FLOAT_POW5_BITCOUNT     = (DOUBLE_POW5_BITCOUNT - 64);

#endif

static inline uint32_t pow5factor_32(uint32_t value)
{
	uint32_t count = 0;
	for(;;)
	{
		assert(value != 0);
		const uint32_t q = value / 5;
		const uint32_t r = value % 5;
		if(r != 0)
		{
			break;
		}
		value = q;
		++count;
	}
	return count;
}

// Returns true if value is divisible by 5^p.
static inline bool multipleOfPowerOf5_32(const uint32_t value, const uint32_t p)
{
	return pow5factor_32(value) >= p;
}

// Returns true if value is divisible by 2^p.
static inline bool multipleOfPowerOf2_32(const uint32_t value, const uint32_t p)
{
	// __builtin_ctz doesn't appear to be faster here.
	return (value & ((1u << p) - 1)) == 0;
}

// It seems to be slightly faster to avoid uint128_t here, although the
// generated code for uint128_t looks slightly nicer.
static inline uint32_t mulShift32(const uint32_t m, const uint64_t factor, const uint8_t shift)
{
	assert(shift > 32);

	// The casts here help MSVC to avoid calls to the __allmul library
	// function.
	const uint32_t factorLo = (uint32_t) (factor);
	const uint32_t factorHi = (uint32_t) (factor >> 32);
	const uint64_t bits0    = (uint64_t) m * factorLo;
	const uint64_t bits1    = (uint64_t) m * factorHi;

	const uint64_t sum		  = (bits0 >> 32) + bits1;
	const uint64_t shiftedSum = sum >> (shift - 32);
	assert(shiftedSum <= UINT32_MAX);
	return (uint32_t) shiftedSum;
}

static inline uint32_t mulPow5InvDivPow2(const uint32_t m, const uint16_t q, const uint8_t j)
{
#if defined(RYU_FLOAT_FULL_TABLE)
	return mulShift32(m, FLOAT_POW5_INV_SPLIT[q], j);
#elif defined(RYU_OPTIMIZE_SIZE)
	// The inverse multipliers are defined as [2^x / 5^y] + 1; the upper 64 bits from the double lookup
	// table are the correct bits for [2^x / 5^y], so we have to add 1 here. Note that we rely on the
	// fact that the added 1 that's already stored in the table never overflows into the upper 64 bits.
	uint64_t pow5[2];
	double_computeInvPow5(q, pow5);
	return mulShift32(m, pow5[1] + 1, j);
#else
	return mulShift32(m, DOUBLE_POW5_INV_SPLIT[q][1] + 1, j);
#endif
}

static inline uint32_t mulPow5divPow2(const uint32_t m, const uint16_t i, const uint8_t j)
{
#if defined(RYU_FLOAT_FULL_TABLE)
	return mulShift32(m, FLOAT_POW5_SPLIT[i], j);
#elif defined(RYU_OPTIMIZE_SIZE)
	uint64_t pow5[2];
	double_computePow5(i, pow5);
	return mulShift32(m, pow5[1], j);
#else
	return mulShift32(m, DOUBLE_POW5_SPLIT[i][1], j);
#endif
}

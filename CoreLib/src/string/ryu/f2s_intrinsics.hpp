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
#include <cassert>
#include <limits>

#include "d2s_full_table.hpp"

constexpr uint16_t FLOAT_POW5_INV_BITCOUNT = (DOUBLE_POW5_INV_BITCOUNT - 64);
constexpr uint16_t FLOAT_POW5_BITCOUNT     = (DOUBLE_POW5_BITCOUNT - 64);

[[nodiscard]] static inline constexpr uint32_t pow5factor_32(uint32_t value)
{
	uint32_t count = 0;
	for(;;)
	{
		assert(value != 0);
		uint32_t const q = value / 5;
		uint32_t const r = value % 5;
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
[[nodiscard]] static inline constexpr bool multipleOfPowerOf5_32(uint32_t const value, uint32_t const p)
{
	return pow5factor_32(value) >= p;
}

// Returns true if value is divisible by 2^p.
[[nodiscard]] static inline constexpr bool multipleOfPowerOf2_32(uint32_t const value, uint32_t const p)
{
	// __builtin_ctz doesn't appear to be faster here.
	return (value & ((1u << p) - 1)) == 0;
}

// It seems to be slightly faster to avoid uint128_t here, although the
// generated code for uint128_t looks slightly nicer.
[[nodiscard]] static inline constexpr uint32_t mulShift32(uint32_t const m, uint64_t const factor, uint8_t const shift)
{
	assert(shift > 32);

	// The casts here help MSVC to avoid calls to the __allmul library
	// function.
	uint32_t const factorLo = static_cast<uint32_t>(factor);
	uint32_t const factorHi = static_cast<uint32_t>(factor >> 32);
	uint64_t const bits0    = static_cast<uint64_t>(m) * static_cast<uint64_t>(factorLo);
	uint64_t const bits1    = static_cast<uint64_t>(m) * static_cast<uint64_t>(factorHi);

	uint64_t const sum		  = (bits0 >> 32) + bits1;
	uint64_t const shiftedSum = sum >> (shift - 32);
	assert(shiftedSum <= std::numeric_limits<uint32_t>::max());
	return static_cast<uint32_t>(shiftedSum);
}



[[nodiscard]] static inline constexpr uint32_t mulPow5InvDivPow2(uint32_t const m, uint16_t const q, uint8_t const j)
{
	return mulShift32(m, DOUBLE_POW5_INV_SPLIT[q][1] + 1, j);
}

[[nodiscard]] static inline constexpr uint32_t mulPow5divPow2(uint32_t const m, uint16_t const i, uint8_t const j)
{
	return mulShift32(m, DOUBLE_POW5_SPLIT[i][1], j);
}

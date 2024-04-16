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
#include <cstdint>

#include <CoreLib/cpu/x64.hpp>

#ifdef _WIN32
#include <intrin.h>
#endif // _WIN32



// Returns the lower 64 bits of (hi*2^64 + lo) >> dist, with 0 < dist < 64.
[[nodiscard]] static inline uint64_t shiftright128(uint64_t const lo, uint64_t const hi, uint8_t dist)
{

	// For the __shiftright128 intrinsic, the shift value is always
	// modulo 64.
	// In the current implementation of the double-precision version
	// of Ryu, the shift value is always < 64. (In the case
	// RYU_OPTIMIZE_SIZE == 0, the shift value is in the range [49, 58].
	// Otherwise in the range [2, 59].)
	// However, this function is now also called by s2d, which requires supporting
	// the larger shift range (TODO: what is the actual range?).
	// Check this here in case a future change requires larger shift
	// values. In this case this function needs to be adjusted.
	assert(dist < 64);
#ifdef _WIN32
	return __shiftright128(lo, hi, dist);
#else
	dist %= 64;
	uint8_t const rem = 64 - dist;
	return (lo >> dist) | (hi << rem);

#endif
}

[[nodiscard]] static inline constexpr uint32_t pow5Factor(uint64_t value)
{
	uint64_t const m_inv_5 = 14757395258967641293u; // 5 * m_inv_5 = 1 (mod 2^64)
	uint64_t const n_div_5 = 3689348814741910323u;	// #{ n | n = 0 (mod 2^64) } = 2^64 / 5
	uint32_t	   count   = 0;
	for(;;)
	{
		assert(value != 0);
		value *= m_inv_5;
		if(value > n_div_5) break;
		++count;
	}
	return count;
}

// Returns true if value is divisible by 5^p.
[[nodiscard]] static inline constexpr bool multipleOfPowerOf5(uint64_t const value, uint32_t const p)
{
	// I tried a case distinction on p, but there was no performance difference.
	return pow5Factor(value) >= p;
}

// Returns true if value is divisible by 2^p.
[[nodiscard]] static inline constexpr bool multipleOfPowerOf2(uint64_t const value, uint8_t const p)
{
	assert(value != 0);
	assert(p < 64);
	// __builtin_ctzll doesn't appear to be faster here.
	return (value & ((1ull << p) - 1)) == 0;
}

// We need a 64x128-bit multiplication and a subsequent 128-bit shift.
// Multiplication:
//   The 64-bit factor is variable and passed in, the 128-bit factor comes
//   from a lookup table. We know that the 64-bit factor only has 55
//   significant bits (i.e., the 9 topmost bits are zeros). The 128-bit
//   factor only has 124 significant bits (i.e., the 4 topmost bits are
//   zeros).
// Shift:
//   In principle, the multiplication result requires 55 + 124 = 179 bits to
//   represent. However, we then shift this value to the right by j, which is
//   at least j >= 115, so the result is guaranteed to fit into 179 - 115 = 64
//   bits. This means that we only need the topmost 64 significant bits of
//   the 64x128-bit multiplication.
//
// There are several ways to do this:
// 1. Best case: the compiler exposes a 128-bit type.
//    We perform two 64x64-bit multiplications, add the higher 64 bits of the
//    lower result to the higher result, and shift by j - 64 bits.
//
//    We explicitly cast from 64-bit to 128-bit, so the compiler can tell
//    that these are only 64-bit inputs, and can map these to the best
//    possible sequence of assembly instructions.
//    x64 machines happen to have matching assembly instructions for
//    64x64-bit multiplications and 128-bit shifts.
//
// 2. Second best case: the compiler exposes intrinsics for the x64 assembly
//    instructions mentioned in 1.
//
// 3. We only have 64x64 bit instructions that return the lower 64 bits of
//    the result, i.e., we have to use plain C.
//    Our inputs are less than the full width, so we have three options:
//    a. Ignore this fact and just implement the intrinsics manually.
//    b. Split both into 31-bit pieces, which guarantees no internal overflow,
//       but requires extra work upfront (unless we change the lookup table).
//    c. Split only the first factor into 31-bit pieces, which also guarantees
//       no internal overflow, but requires extra work since the intermediate
//       results are not perfectly aligned.

[[nodiscard]] static inline uint64_t mulShift64(uint64_t const m, std::array<uint64_t, 2> const mul, uint8_t const j)
{
	// m is maximum 55 bits
	uint64_t	   high1;								// 128
	uint64_t const low1 = core::umul(m, mul[1], high1); // 64
	uint64_t	   high0;								// 64
	core::umul(m, mul[0], high0);						// 0
	uint64_t const sum = high0 + low1;
	if(sum < high0)
	{
		++high1; // overflow into high1
	}
	return shiftright128(sum, high1, static_cast<uint8_t>(j - 64));
}

[[nodiscard]] static inline uint64_t mulShiftAll64(
	uint64_t const			m,
	std::array<uint64_t, 2> const	mul,
	uint8_t const			j,
	uint64_t&				vp,
	uint64_t&				vm,
	uint8_t const			mmShift)
{
	vp = mulShift64(4 * m + 2, mul, j);
	vm = mulShift64(4 * m - 1 - mmShift, mul, j);
	return mulShift64(4 * m, mul, j);
}

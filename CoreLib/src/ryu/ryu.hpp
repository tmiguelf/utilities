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

#include <cstdint>

uintptr_t d2s_buffered_n(double f, char* result);
uintptr_t f2s_buffered_n(float f, char* result);
uintptr_t d2fixed_buffered_n(double d, uint32_t precision, char* result);
uintptr_t d2exp_buffered_n(double d, uint32_t precision, char* result);

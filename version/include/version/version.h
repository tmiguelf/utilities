//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///		Used to prepare and format version information
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

#include <winver.h>

#include "versionSpecific.h"

// Macros to make a string of argument
#define STRINGER(x)		#x
#define STRINGER2(a)	STRINGER(a)


#ifdef VERSION_REV_MIN
#	define VSTR_STR		STRINGER2(VERSION_MAJOR) "." STRINGER2(VERSION_MINOR) "." STRINGER2(VERSION_REV) "." STRINGER2(VERSION_REV_MIN)
#	define VERSION_STR	VSTR_STR
#else
#	define VSTR_STR		STRINGER2(VERSION_MAJOR) "." STRINGER2(VERSION_MINOR) "." STRINGER2(VERSION_REV)
#	define VERSION_STR	VSTR_STR
#endif

#ifdef _DEBUG
#	define VERSION_LONG_STR		VERSION_STR " (Debug)"
#	define VERSION_FILE_FLGAS	VS_FF_DEBUG
#else
#	define VERSION_LONG_STR		VERSION_STR
#	define VERSION_FILE_FLGAS	0
#endif

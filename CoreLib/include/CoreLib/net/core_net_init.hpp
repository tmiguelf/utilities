//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///
///	\copyright
///		Copyright (c) 2020 Tiago Miguel Oliveira Freire
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

/// \n
namespace core
{
#ifdef _WIN32
///	\brief Initializes the network subsystem for windows. On linux this has no effect.
///		see https://docs.microsoft.com/en-us/windows/desktop/api/winsock/nf-winsock-wsastartup
///
///	\return true if the subsystem was initialized successfully, false on error. On linux always returns true.
/// \sa Net_End
bool Net_Init();

///	\brief Releases the network subsystem on windows. On linux it has no effect.
///		see https://docs.microsoft.com/en-us/windows/desktop/api/winsock/nf-winsock-wsacleanup
/// \sa Net_Init
void Net_End();
#else

///	\brief Initializes the network subsystem for windows. On linux this has no effect.
///		see https://docs.microsoft.com/en-us/windows/desktop/api/winsock/nf-winsock-wsastartup
///
///	\return true if the subsystem was initialized successfully, false on error. On linux always returns true.
/// \sa Net_End
inline constexpr bool Net_Init() { return true; }

///	\brief Releases the network subsystem on windows. On linux it has no effect.
///		see https://docs.microsoft.com/en-us/windows/desktop/api/winsock/nf-winsock-wsacleanup
/// \sa Net_Init
inline constexpr void Net_End	() {}
#endif
} //namespace core
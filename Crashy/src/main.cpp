//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///		Intentionally crashing application to test stack trace
///
///	\copyright
///		Copyright (c) Tiago Miguel Oliveira Freire
///
///		Permission is hereby granted, free of charge, to any person obtaining a copy
///		of this software and associated documentation files (the "Software"),
///		to copy, modify, publish, and/or distribute copies of the Software,
///		and to permit persons to whom the Software is furnished to do so,
///		subject to the following conditions:
///
///		The copyright notice and this permission notice shall be included in all
///		copies or substantial portions of the Software.
///		The copyrighted work, or derived works, shall not be used to train
///		Artificial Intelligence models of any sort; or otherwise be used in a
///		transformative way that could obfuscate the source of the copyright.
///
///		THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///		IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///		FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///		AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
///		LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///		OUT OFs OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
///		SOFTWARE.
//======== ======== ======== ======== ======== ======== ======== ========

#include <iostream>
#include <vector>
#include <CoreLib/core_stacktrace.hpp>


volatile int var;

void doBadStuff(std::vector<int>& p_vect)
{
	p_vect.resize(p_vect[0] * 2);

	p_vect[0] = 1024;

	var = p_vect[0];
	p_vect[1258466] = var;
}

using fn_t = void (*)();

int main(
	[[maybe_unused]] int argc,
	[[maybe_unused]] char* argv[])
{
	core::register_crash_trace("Test.strace");


	//fn_t fn = (fn_t) static_cast<uintptr_t>( argc );
	//fn();

	std::vector<int> tes = {1, 2, 3};


	doBadStuff(tes);
	doBadStuff(tes);


	for(int t : tes)
	{
		std::cout << t << std::endl;
	}

	std::cout << "Exited ok" <<std::endl;

	return 0;
}

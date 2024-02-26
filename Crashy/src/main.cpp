//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///		Intentionally crashing application to test stack trace
///
///	\copyright
///		
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

int main(
	[[maybe_unused]] int argc,
	[[maybe_unused]] char* argv[])
{
	std::vector<int> tes = {1, 2, 3};


	core::register_crash_trace("Test.strace");

	doBadStuff(tes);
	doBadStuff(tes);


	for(int t : tes)
	{
		std::cout << t << std::endl;
	}

	std::cout << "Exited ok" <<std::endl;

	return 0;
}

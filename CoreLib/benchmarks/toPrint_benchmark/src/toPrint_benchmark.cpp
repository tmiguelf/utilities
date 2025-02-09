//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///
///	\copyright
///		
//======== ======== ======== ======== ======== ======== ======== ========

#include <benchmark/benchmark.h>

#include <CoreLib/toPrint/toPrint.hpp>
#include <CoreLib/toPrint/toPrint_sink.hpp>

static char8_t const volatile* g_dump;
static uintptr_t volatile g_dump2;


class dumpSink: public core::sink_toPrint_base
{
public:
	void write(std::u8string_view p_out) const
	{
		g_dump = p_out.data();
		g_dump2 = p_out.size();
	}
};


std::string_view const test_string = "The quick brown fox jumps over the lazy dog";
int32_t const test_signed_int = -34;
uint64_t const test_unsigned_int = 12345;
double const test_fp = -5.67;
char const test_char = 'a';

static void no_op(benchmark::State& state)
{
	bool const volatile ok = false;
	while(state.KeepRunning())
	{
		benchmark::DoNotOptimize(ok);
	}
}

static void toPrint_s(benchmark::State& state)
{
	dumpSink tsink;
	while(state.KeepRunning())
	{
		core::print<char8_t>(tsink, test_string, test_signed_int);
	}
}

static void toPrint2_s(benchmark::State& state)
{
	dumpSink tsink;
	while(state.KeepRunning())
	{
		core::print2<char8_t>(tsink, test_string, test_signed_int);
	}
}

static void toPrint_l(benchmark::State& state)
{
	dumpSink tsink;
	while(state.KeepRunning())
	{
		core::print<char8_t>(tsink, test_string, test_signed_int, test_unsigned_int, test_fp, test_char);
	}
}

static void toPrint2_l(benchmark::State& state)
{
	dumpSink tsink;
	while(state.KeepRunning())
	{
		core::print2<char8_t>(tsink, test_string, test_signed_int, test_unsigned_int, test_fp, test_char);
	}
}

BENCHMARK(no_op);
BENCHMARK(toPrint_s);
BENCHMARK(toPrint2_s);
BENCHMARK(toPrint_l);
BENCHMARK(toPrint2_l);

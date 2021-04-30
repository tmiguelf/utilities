//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///
///	\copyright
///		
//======== ======== ======== ======== ======== ======== ======== ========

#include <string_view>
#include <benchmark/benchmark.h>
#include <CoreLib/string/core_string_encoding.hpp>

#include <bit>

std::u32string_view SampleText_UCS4			= U"The quick brown fox jumps over the lazy dog, and eats all the chickens. 敏捷的棕色狐狸跳过了懒狗，吃掉了所有的鸡";
std::u16string_view SampleText_UCS2			= u"The quick brown fox jumps over the lazy dog, and eats all the chickens.";
std::u16string_view SampleText_UCS2_UTF8	= u"The quick brown fox jumps over the lazy dog, and eats all the chickens. 敏捷的棕色狐狸跳过了懒狗，吃掉了所有的鸡";

std::u8string		SampleText_utf8			= core::UCS4_to_UTF8(U"The quick brown fox jumps over the lazy dog, and eats all the chickens.");
std::u8string		SampleText_utf8_ext		= core::UCS4_to_UTF8(SampleText_UCS4);

#if 0
static void old_UTF16_2_UTF8(benchmark::State& state)
{
	for (auto _ : state)
	{
		auto result = core::UTF16_to_UTF8(SampleText_UCS2_UTF8);
		benchmark::DoNotOptimize(result);
	}
}
BENCHMARK(old_UTF16_2_UTF8);

static void new_UTF16_2_UTF8(benchmark::State& state)
{
	for (auto _ : state)
	{
		auto res1 = core::_p::UTF16_to_UTF8_estimate(SampleText_UCS2_UTF8);
		if(res1.has_value())
		{
			std::u8string dump;
			dump.resize(res1.value());
			core::_p::UTF16_to_UTF8_unsafe(SampleText_UCS2_UTF8, dump.data());
			benchmark::DoNotOptimize(dump);
		}
	}
}
BENCHMARK(new_UTF16_2_UTF8);

static void new_UTF16_2_UTF8_stack(benchmark::State& state)
{
	for (auto _ : state)
	{
		auto res1 = core::_p::UTF16_to_UTF8_estimate(SampleText_UCS2_UTF8);
		if(res1.has_value())
		{
			char8_t dump[1024];
			core::_p::UTF16_to_UTF8_unsafe(SampleText_UCS2_UTF8, dump);
			benchmark::DoNotOptimize(dump);
		}
	}
}
BENCHMARK(new_UTF16_2_UTF8_stack);


static void old_UTF8_2_UTF16(benchmark::State& state)
{
	for (auto _ : state)
	{
		auto result = core::UTF8_to_UTF16(SampleText_utf8_ext);
		if(result.has_value())
		{
			benchmark::DoNotOptimize(result.value());
		}
	}
}
BENCHMARK(old_UTF8_2_UTF16);

static void new_UTF8_2_UTF16(benchmark::State& state)
{
	for (auto _ : state)
	{
		auto res1 = core::_p::UTF8_to_UTF16_estimate(SampleText_utf8_ext);
		if(res1.has_value())
		{
			std::u16string dump;
			dump.resize(res1.value());
			core::_p::UTF8_to_UTF16_unsafe(SampleText_utf8_ext, dump.data());
			benchmark::DoNotOptimize(dump);
		}
	}
}
BENCHMARK(new_UTF8_2_UTF16);

static void new_UTF8_2_UTF16_stack(benchmark::State& state)
{
	for (auto _ : state)
	{
		auto res1 = core::_p::UTF8_to_UTF16_estimate(SampleText_utf8_ext);
		if(res1.has_value())
		{
			char16_t dump[1024];
			core::_p::UTF8_to_UTF16_unsafe(SampleText_utf8_ext, dump);
			benchmark::DoNotOptimize(dump);
		}
	}
}
BENCHMARK(new_UTF8_2_UTF16_stack);


static void old_UTF16_to_UTF8(benchmark::State& state)
{
	for (auto _ : state)
	{
		auto result = core::UTF16_to_UTF8_faulty(SampleText_UCS2_UTF8, '?');
		benchmark::DoNotOptimize(result);
	}
}
BENCHMARK(old_UTF16_to_UTF8);

static void new_UTF16_to_UTF8(benchmark::State& state)
{
	for (auto _ : state)
	{
		uintptr_t res1 = core::_p::UTF16_to_UTF8_faulty_estimate(SampleText_UCS2_UTF8, '?');
		std::u8string dump;
		dump.resize(res1);
		core::_p::UTF16_to_UTF8_faulty_unsafe(SampleText_UCS2_UTF8, '?', dump.data());
		benchmark::DoNotOptimize(dump);
	}
}
BENCHMARK(new_UTF16_to_UTF8);

static void new_UTF16_to_UTF8_stack(benchmark::State& state)
{
	for (auto _ : state)
	{
		uintptr_t res1 = core::_p::UTF16_to_UTF8_faulty_estimate(SampleText_UCS2_UTF8, '?');
		char8_t dump[1024];
		core::_p::UTF16_to_UTF8_faulty_unsafe(SampleText_UCS2_UTF8, '?', dump);
		benchmark::DoNotOptimize(res1);
		benchmark::DoNotOptimize(dump);
	}
}
BENCHMARK(new_UTF16_to_UTF8_stack);

#endif

static inline bool __fmove_1(const char8_t*& p_input, const char8_t* const p_end)
{
	const char8_t testp = *p_input;
	if(testp & 0x80) //!level 0
	{
		if((testp & 0xE0) == 0xC0) //level 1
		{
			if( p_end - p_input < 2		||	//validate size
				testp < 0xE2			||	//validate range
				(*++p_input & 0xC0) != 0x80) //validate encoding
			{
				return false;
			}
		}
		else
		{
			const char8_t* tlocal = p_input;
			if((testp & 0xF0) == 0xE0) //level 2
			{
				if(p_end - tlocal < 3			||	//validate size
					(!(*(tlocal++) & 0x0F) && !(*tlocal & 0x20)) || //validate range
					(*tlocal & 0xC0) != 0x80	||	//validate encoding
					(*++tlocal & 0xC0) != 0x80)
				{
					return false;
				}
				p_input += 2;
			}
			else if((testp & 0xF8) == 0xF0) //level 3
			{
				if(	p_end - tlocal < 4			||
					(!(*(tlocal++) & 0x07) && (*tlocal < 0x90)) || //validate range
					(*tlocal & 0xC0) != 0x80	||	//validate encoding
					(*++tlocal & 0xC0) != 0x80	||
					(*++tlocal & 0xC0) != 0x80	)
				{
					return false;
				}
				p_input += 3;
			}
			else if((testp & 0xFC) == 0xF8) //level 4
			{
				if(	p_end - tlocal < 5			||
					(!(*(tlocal++) & 0x03) && (*tlocal < 0x88)) || //validate range
					(*tlocal & 0xC0) != 0x80	||	//validate encoding
					(*++tlocal & 0xC0) != 0x80	||
					(*++tlocal & 0xC0) != 0x80	||
					(*++tlocal & 0xC0) != 0x80	)
				{
					return false;
				}
				p_input += 4;
			}
			else if((testp & 0xFE) == 0xFC) //level 5
			{
				if(	p_end - tlocal < 6			||
					(!(*(tlocal++) & 0x01) && (*tlocal < 0x84)) || //validate range
					(*tlocal & 0xC0) != 0x80	||	//validate encoding
					(*++tlocal & 0xC0) != 0x80	||
					(*++tlocal & 0xC0) != 0x80	||
					(*++tlocal & 0xC0) != 0x80	||
					(*++tlocal & 0xC0) != 0x80	)
				{
					return false;
				}
				p_input += 5;
			}
			else if((testp & 0xFF) == 0xFE) //level 6
			{
				if(	p_end - tlocal < 7			||
					(*++tlocal != 0x82 && *tlocal != 0x83) || //validate range and encoding
					(*++tlocal & 0xC0) != 0x80	||	//validate encoding
					(*++tlocal & 0xC0) != 0x80	||
					(*++tlocal & 0xC0) != 0x80	||
					(*++tlocal & 0xC0) != 0x80	||
					(*++tlocal & 0xC0) != 0x80	)
				{
					return false;
				}
				p_input += 6;
			}
			else
			{
				return false;
			}
		}
	}
	return true;
}

static inline uintptr_t __fmove_2(const char8_t* p_input, const char8_t* const p_end)
{
	const char8_t testp = *p_input;
	if(!(testp & 0x80)) //level 0
	{
		return 1;
	}
	
	if((testp & 0xE0) == 0xC0) //level 1
	{
		if( p_end - p_input < 2		||	//validate size
			testp < 0xE2			||	//validate range
			(*++p_input & 0xC0) != 0x80) //validate encoding
		{
			return 0;
		}
		return 2;
	}

	if((testp & 0xF0) == 0xE0) //level 2
	{
		if(p_end - p_input < 3			||	//validate size
			(!(*(p_input++) & 0x0F) && !(*p_input & 0x20)) || //validate range
			(*p_input & 0xC0) != 0x80	||	//validate encoding
			(*++p_input & 0xC0) != 0x80)
		{
			return 0;
		}
		return 3;
	}
	if((testp & 0xF8) == 0xF0) //level 3
	{
		if(	p_end - p_input < 4			||
			(!(*(p_input++) & 0x07) && (*p_input < 0x90)) || //validate range
			(*p_input & 0xC0) != 0x80	||	//validate encoding
			(*++p_input & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	)
		{
			return 0;
		}
		return 4;
	}
	if((testp & 0xFC) == 0xF8) //level 4
	{
		if(	p_end - p_input < 5			||
			(!(*(p_input++) & 0x03) && (*p_input < 0x88)) || //validate range
			(*p_input & 0xC0) != 0x80	||	//validate encoding
			(*++p_input & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	)
		{
			return 0;
		}
		return 5;
	}
	if((testp & 0xFE) == 0xFC) //level 5
	{
		if(	p_end - p_input < 6			||
			(!(*(p_input++) & 0x01) && (*p_input < 0x84)) || //validate range
			(*p_input & 0xC0) != 0x80	||	//validate encoding
			(*++p_input & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	)
		{
			return 0;
		}
		return 6;
	}
	if((testp & 0xFF) == 0xFE) //level 6
	{
		if(	p_end - p_input < 7			||
			(*++p_input != 0x82 && *p_input != 0x83) || //validate range and encoding
			(*++p_input & 0xC0) != 0x80	||	//validate encoding
			(*++p_input & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	)
		{
			return 0;
		}
		return 7;
	}

	return 0;
}

static inline const char8_t* __fmove_3(const char8_t* p_input, const char8_t* const p_end)
{
	const char8_t testp = *p_input;
	if(!(testp & 0x80)) //level 0
	{
		return ++p_input;
	}

	if((testp & 0xE0) == 0xC0) //level 1
	{
		if( p_end - p_input < 2		||	//validate size
			testp < 0xE2			||	//validate range
			(*++p_input & 0xC0) != 0x80) //validate encoding
		{
			return nullptr;
		}
		return ++p_input;
	}

	if((testp & 0xF0) == 0xE0) //level 2
	{
		if(p_end - p_input < 3			||	//validate size
			(!(*(p_input++) & 0x0F) && !(*p_input & 0x20)) || //validate range
			(*p_input & 0xC0) != 0x80	||	//validate encoding
			(*++p_input & 0xC0) != 0x80)
		{
			return nullptr;
		}
		return ++p_input;
	}
	if((testp & 0xF8) == 0xF0) //level 3
	{
		if(	p_end - p_input < 4			||
			(!(*(p_input++) & 0x07) && (*p_input < 0x90)) || //validate range
			(*p_input & 0xC0) != 0x80	||	//validate encoding
			(*++p_input & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	)
		{
			return nullptr;
		}
		return ++p_input;
	}
	if((testp & 0xFC) == 0xF8) //level 4
	{
		if(	p_end - p_input < 5			||
			(!(*(p_input++) & 0x03) && (*p_input < 0x88)) || //validate range
			(*p_input & 0xC0) != 0x80	||	//validate encoding
			(*++p_input & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	)
		{
			return nullptr;
		}
		return ++p_input;
	}
	if((testp & 0xFE) == 0xFC) //level 5
	{
		if(	p_end - p_input < 6			||
			(!(*(p_input++) & 0x01) && (*p_input < 0x84)) || //validate range
			(*p_input & 0xC0) != 0x80	||	//validate encoding
			(*++p_input & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	)
		{
			return nullptr;
		}
		return ++p_input;
	}
	if((testp & 0xFF) == 0xFE) //level 6
	{
		if(	p_end - p_input < 7			||
			(*++p_input != 0x82 && *p_input != 0x83) || //validate range and encoding
			(*++p_input & 0xC0) != 0x80	||	//validate encoding
			(*++p_input & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	||
			(*++p_input & 0xC0) != 0x80	)
		{
			return nullptr;
		}
		return ++p_input;
	}

	return nullptr;
}


[[nodiscard]] std::optional<uintptr_t> estimate_1(std::u8string_view p_input)
{
	uintptr_t count = 0;
	const char8_t* pos = p_input.data();
	const char8_t* const end = pos + p_input.size();
	for(; pos < end; ++pos, ++count)
	{
		if(!__fmove_1(pos, end))
		{
			return {};
		}
	}
	return count;
}


[[nodiscard]] std::optional<uintptr_t> estimate_2(std::u8string_view p_input)
{
	uintptr_t count = 0;
	const char8_t* pos = p_input.data();
	const char8_t* const end = pos + p_input.size();
	for(; pos < end; ++count)
	{
		const uintptr_t res = __fmove_2(pos, end);
		if(res)
		{
			pos += res;
		}
		else
		{
			return {};
		}
	}
	return count;
}

[[nodiscard]] std::optional<uintptr_t> estimate_3(std::u8string_view p_input)
{
	uintptr_t count = 0;
	const char8_t* pos = p_input.data();
	const char8_t* const end = pos + p_input.size();
	for(; pos < end; ++count)
	{
		pos = __fmove_3(pos, end);
		if(!pos)
		{
			return {};
		}
	}
	return count;
}

static void test_check1(benchmark::State& state)
{
	for (auto _ : state)
	{
		auto res = estimate_1(SampleText_utf8_ext);
		benchmark::DoNotOptimize(res);
		benchmark::DoNotOptimize(res.has_value());
	}
}
BENCHMARK(test_check1);

static void test_check2(benchmark::State& state)
{
	for (auto _ : state)
	{
		auto res = estimate_2(SampleText_utf8_ext);
		benchmark::DoNotOptimize(res);
		benchmark::DoNotOptimize(res.has_value());
	}
}
BENCHMARK(test_check2);

static void test_check3(benchmark::State& state)
{
	for (auto _ : state)
	{
		auto res = estimate_3(SampleText_utf8_ext);
		benchmark::DoNotOptimize(res);
		benchmark::DoNotOptimize(res.has_value());
	}
}
BENCHMARK(test_check3);

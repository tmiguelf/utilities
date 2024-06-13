//======== ======== ======== ======== ======== ======== ======== ========
///	\file
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
///		OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
///		SOFTWARE.
//======== ======== ======== ======== ======== ======== ======== ========

#include <CoreLib/core_time.hpp>

#include <CoreLib/core_type.hpp>

#include <array>

#ifdef _WIN32
#include <sys/timeb.h>
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include <time.h>


namespace core
{

	using ::core::literals::operator ""_ui8;
	using ::core::literals::operator ""_ui16;
	using ::core::literals::operator ""_ui64;

namespace
{
#ifdef _WIN32
/// \brief Used to get and store the performance frequency
///	as recomended in https://msdn.microsoft.com/en-us/library/windows/desktop/ms644905(v=vs.85).aspx
//
class os_clock_frequency
{
private:
	uint64_t m_frequency;
public:
	os_clock_frequency()
	{
		LARGE_INTEGER t_Frequency;
		QueryPerformanceFrequency(&t_Frequency);
		m_frequency = t_Frequency.QuadPart ? t_Frequency.QuadPart : 1;
	}

	inline uint64_t frequency() const { return m_frequency; }
};

static os_clock_frequency const g_WinFreq;
#endif

static constexpr uint64_t g_sec2nsec = 1000000000_ui64;

}	//namespace


void chrono::set()
{
#ifdef _WIN32
	LARGE_INTEGER Time;
	QueryPerformanceCounter(&Time);
	m_ref = Time.QuadPart;
#else
	struct timespec time;
	clock_gettime(CLOCK_BOOTTIME, &time);
	m_ref = (uint64_t)time.tv_sec * g_sec2nsec + time.tv_nsec;
#endif
}

uint64_t chrono::elapsed() const //nanosecond resolution
{
#ifdef _WIN32
	uint64_t const freq = g_WinFreq.frequency();

	LARGE_INTEGER Time;
	QueryPerformanceCounter(&Time);
	uint64_t t_val = Time.QuadPart - m_ref;

	return ((t_val / freq) * g_sec2nsec) + ((t_val % freq) * g_sec2nsec) / freq;
#else
	struct timespec time;
	clock_gettime(CLOCK_BOOTTIME, &time);
	return static_cast<uint64_t>(time.tv_sec) * g_sec2nsec + static_cast<uint64_t>(time.tv_nsec) - m_ref;
#endif
}

void track_chrono::pause()
{
	if(!m_isPaused)
	{
#ifdef _WIN32
		LARGE_INTEGER Time;
		QueryPerformanceCounter(&Time);
		m_acumulated += Time.QuadPart - m_ref;
#else
		struct timespec time;
		clock_gettime(CLOCK_BOOTTIME, &time);
		m_acumulated += static_cast<uint64_t>(time.tv_sec) * g_sec2nsec + static_cast<uint64_t>(time.tv_nsec) - m_ref;
#endif
		m_isPaused = true;
	}
}

void track_chrono::resume()
{
	if(m_isPaused)
	{
#ifdef _WIN32
		LARGE_INTEGER Time;
		QueryPerformanceCounter(&Time);
		m_ref = Time.QuadPart;
#else
		struct timespec time;
		clock_gettime(CLOCK_BOOTTIME, &time);
		m_ref = static_cast<uint64_t>(time.tv_sec) * g_sec2nsec + static_cast<uint64_t>(time.tv_nsec);
#endif
		m_isPaused = false;
	}
}

void track_chrono::restart()
{
#ifdef _WIN32
	LARGE_INTEGER Time;
	QueryPerformanceCounter(&Time);
	m_ref = Time.QuadPart;
#else
	struct timespec time;
	clock_gettime(CLOCK_BOOTTIME, &time);
	m_ref = static_cast<uint64_t>(time.tv_sec) * g_sec2nsec + static_cast<uint64_t>(time.tv_nsec);
#endif

	m_acumulated	= 0;
	m_isPaused		= false;
}

uint64_t track_chrono::read() const
{

#ifdef _WIN32
	uint64_t const freq = g_WinFreq.frequency();
	if(m_isPaused)
	{
		return (m_acumulated / freq) * g_sec2nsec + ((m_acumulated % freq) * g_sec2nsec) / freq;
	}

	LARGE_INTEGER Time;
	QueryPerformanceCounter(&Time);
	uint64_t const val = m_acumulated + Time.QuadPart - m_ref;
	return (val / freq) * g_sec2nsec + ((val % freq) * g_sec2nsec) / freq;
#else
	if(m_isPaused)
	{
		return m_acumulated;
	}

	struct timespec time;
	clock_gettime(CLOCK_BOOTTIME, &time);
	uint64_t val = m_acumulated + static_cast<uint64_t>(time.tv_sec) * g_sec2nsec + static_cast<uint64_t>(time.tv_nsec) - m_ref;
	return val;
#endif
}

void track_chrono::set(uint64_t p_value)
{
#ifdef _WIN32
	uint64_t const freq = g_WinFreq.frequency();
	m_acumulated = (p_value / g_sec2nsec) * freq + ((p_value % g_sec2nsec) * freq) / g_sec2nsec;
	if(!m_isPaused)
	{
		LARGE_INTEGER Time;
		QueryPerformanceCounter(&Time);
		m_ref = Time.QuadPart;
	}
#else
	m_acumulated = p_value;
	if(!m_isPaused)
	{
		struct timespec time;
		clock_gettime(CLOCK_BOOTTIME, &time);
		m_ref = static_cast<uint64_t>(time.tv_sec) * g_sec2nsec + static_cast<uint64_t>(time.tv_nsec);
	}
#endif
}


uint64_t clock_stamp() //1 nanosecond resolution
{
#ifdef _WIN32
	LARGE_INTEGER Time;
	QueryPerformanceCounter(&Time);
	uint64_t const t_val = Time.QuadPart;
	uint64_t const freq = g_WinFreq.frequency();
	return (t_val / freq) * g_sec2nsec + ((t_val % freq) * g_sec2nsec) / freq;
#else
	struct timespec time;
	clock_gettime(CLOCK_BOOTTIME, &time);
	return static_cast<uint64_t>(time.tv_sec) * g_sec2nsec + static_cast<uint64_t>(time.tv_nsec);
#endif
}






namespace
{
	constexpr uint64_t sec_nsec = 1'000'000'000_ui64;
	constexpr uint64_t min_sec  = 60_ui64;
	constexpr uint64_t hour_min = 60_ui64;
	constexpr uint64_t day_hour = 24_ui64;

	constexpr uint64_t minute_nsec = min_sec * sec_nsec;
	constexpr uint64_t hour_nsec   = hour_min * min_sec * sec_nsec;
	constexpr uint64_t day_nsec    = day_hour * hour_min * min_sec * sec_nsec;

	constexpr uint64_t days_per_1year   = 365;
	constexpr uint64_t days_per_4year   = days_per_1year   *  4 + 1;
	constexpr uint64_t days_per_100year = days_per_4year   * 25 - 1;
	constexpr uint64_t days_per_400year = days_per_100year *  4 + 1;

	constexpr uint64_t time_in_1year   = days_per_1year   * day_nsec;
	constexpr uint64_t time_in_4year   = days_per_4year   * day_nsec;
	constexpr uint64_t time_in_100year = days_per_100year * day_nsec;
	constexpr uint64_t time_in_400year = days_per_400year * day_nsec;

#ifdef _WIN32
	//1601 - 2001
	constexpr uint64_t epoch_offset = 0 - time_in_400year;
#else
	//1970 - 2001
	constexpr uint64_t epoch_offset = time_in_100year * 3 + time_in_4year * 17 + time_in_1year - time_in_400year;
#endif // _WIN32
	constexpr uint16_t reference_year = 2001;

	consteval uint8_t _h_days_in_month(uint8_t const p_month, bool const p_leapYear)
	{
		switch (p_month)
		{
		case 1: //February
			return p_leapYear ? 29 : 28;
		case 3:  //April
		case 5:  //June
		case 8:  //September
		case 10: //November
			return 30;
		default: //All others
			break;
		}
		return 31;
	}

	consteval uint64_t _h_time_till_month(uint8_t const p_month, bool const p_leapYear)
	{
		if (p_month == 0)
			return 0;
		return _h_time_till_month(p_month - 1, p_leapYear) + _h_days_in_month(p_month - 1, p_leapYear) * day_nsec;
	}



	/// \brief Lookup for time until start of the month (regular year)
	//
	static std::array<uint64_t, 12> const g_nsTillMonth =
	{
		_h_time_till_month( 0, false),
		_h_time_till_month( 1, false),
		_h_time_till_month( 2, false),
		_h_time_till_month( 3, false),
		_h_time_till_month( 4, false),
		_h_time_till_month( 5, false),
		_h_time_till_month( 6, false),
		_h_time_till_month( 7, false),
		_h_time_till_month( 8, false),
		_h_time_till_month( 9, false),
		_h_time_till_month(10, false),
		_h_time_till_month(11, false)
	};

	/// \brief Lookup for time until start of the month (leap year)
	//
	static std::array<uint64_t, 12> const g_nsTillMonthLeap =
	{
		_h_time_till_month( 0, true),
		_h_time_till_month( 1, true),
		_h_time_till_month( 2, true),
		_h_time_till_month( 3, true),
		_h_time_till_month( 4, true),
		_h_time_till_month( 5, true),
		_h_time_till_month( 6, true),
		_h_time_till_month( 7, true),
		_h_time_till_month( 8, true),
		_h_time_till_month( 9, true),
		_h_time_till_month(10, true),
		_h_time_till_month(11, true)
	};

	/// \brief Lookup for days in a given month (regular year)
	//
	static std::array<uint8_t, 12> const g_daysInMonth =
	{
		_h_days_in_month( 0, false),
		_h_days_in_month( 1, false),
		_h_days_in_month( 2, false),
		_h_days_in_month( 3, false),
		_h_days_in_month( 4, false),
		_h_days_in_month( 5, false),
		_h_days_in_month( 6, false),
		_h_days_in_month( 7, false),
		_h_days_in_month( 8, false),
		_h_days_in_month( 9, false),
		_h_days_in_month(10, false),
		_h_days_in_month(11, false)
	};

	/// \brief Lookup for days in a given month (leap year)
	//
	static std::array<uint8_t, 12> const g_daysInMonthLeap =
	{
		_h_days_in_month( 0, true),
		_h_days_in_month( 1, true),
		_h_days_in_month( 2, true),
		_h_days_in_month( 3, true),
		_h_days_in_month( 4, true),
		_h_days_in_month( 5, true),
		_h_days_in_month( 6, true),
		_h_days_in_month( 7, true),
		_h_days_in_month( 8, true),
		_h_days_in_month( 9, true),
		_h_days_in_month(10, true),
		_h_days_in_month(11, true)
	};

	constexpr bool is_leap_year(uint16_t const p_year)
	{
		return ((p_year % 4 == 0) && (p_year % 100 != 0)) || (p_year % 400 == 0);
	}


	uint16_t time_to_year(uint64_t p_100ns, uint64_t& p_rem)
	{
		uint16_t t_year = reference_year + static_cast<uint16_t>((p_100ns / time_in_400year) * 400);
		p_100ns %= time_in_400year;

		t_year += static_cast<uint16_t>((p_100ns / time_in_100year) * 100);
		p_100ns %= time_in_100year;

		t_year += static_cast<uint16_t>((p_100ns / time_in_4year) * 4);
		p_100ns %= time_in_4year;

		p_rem = p_100ns % time_in_1year;
		return t_year + static_cast<uint16_t>(p_100ns / time_in_1year);
	}

	uint64_t year_to_time(uint16_t p_year)
	{
		p_year -= reference_year;
		uint64_t t_100ns = (p_year / 400) * time_in_400year;
		uint16_t t_rem = p_year % 400;

		t_100ns += (t_rem / 100) * time_in_100year;
		t_rem %= 100;
		t_100ns += (t_rem / 4) * time_in_4year;
		return t_100ns + (t_rem % 4) * time_in_1year;
	}

	static uint8_t find_month_from_ns(uint64_t const p_100ns, std::array<uint64_t, 12> const& p_100ns_till_month)
	{
		//Used to estimate what is the month it currently must be in so it doesn't have to search allot
		//I know perfectly well that February can sometimes only have 28 days
		//but it follows January which has 31 days, by the time 1 of March comes around 59 days have passed,
		//which leads to an average of 29.5 days, this is as low as the "average days in a month since the start of the year" can go
		//since max days in a month is 31, and there are only 12 months in a year, the real month can not be more then 1 off
		constexpr uint64_t lowerMonthMs = 29 * day_nsec;

		uint8_t pivot = static_cast<uint8_t>(p_100ns / lowerMonthMs /*+ 1*/);

		if(pivot > 11) pivot = 11;
		if(p_100ns < p_100ns_till_month[pivot])
		{
			--pivot;
		}
		return pivot;
	}

	uint8_t time_2_Month(uint64_t const p_100ns, uint64_t& p_rem, bool const p_leapYear)
	{
		if(p_leapYear)
		{
			uint8_t const criticalIndex = find_month_from_ns(p_100ns, g_nsTillMonthLeap);
			p_rem = (p_100ns - g_nsTillMonthLeap[criticalIndex]);
			return criticalIndex;
		}
		else
		{
			uint8_t const criticalIndex = find_month_from_ns(p_100ns, g_nsTillMonth);
			p_rem = (p_100ns - g_nsTillMonth[criticalIndex]);
			return criticalIndex;
		}
	}

	uint64_t time_till_month(uint8_t const p_month, bool const p_leapYear)
	{
		if (p_leapYear)
			return g_nsTillMonthLeap[p_month];
		return g_nsTillMonth[p_month];
	}

#if 0
	uint8_t days_in_month(uint8_t const p_month, bool const p_leapYear)
	{
		if(p_leapYear)
		{
			return g_daysInMonthLeap[p_month];
		}
		return g_daysInMonth[p_month];
	}
#endif
} //namespace


void date_time_local(date_time_t& p_out)
{
#ifdef _WIN32
	SYSTEMTIME timeinfo;
	GetLocalTime(&timeinfo);
	p_out.date.year		= static_cast<uint16_t>(timeinfo.wYear);
	p_out.date.month	= static_cast<uint8_t> (timeinfo.wMonth);
	p_out.date.day		= static_cast<uint8_t> (timeinfo.wDay);
	p_out.time.hour		= static_cast<uint8_t> (timeinfo.wHour);
	p_out.time.minute	= static_cast<uint8_t> (timeinfo.wMinute);
	p_out.time.second	= static_cast<uint8_t> (timeinfo.wSecond);
	p_out.time.nsecond	= static_cast<uint32_t>(timeinfo.wMilliseconds) * 1000000;
#else
	struct tm timeinfo;

	timeval curTime;
	gettimeofday(&curTime, nullptr);
	localtime_r(&curTime.tv_sec, &timeinfo);
	p_out.time.nsecond	= static_cast<uint16_t>(curTime.tv_usec * 1000);
	p_out.date.year		= static_cast<uint16_t>(static_cast<uint16_t>(timeinfo.tm_year)	+ 1900_ui16);
	p_out.date.month	= static_cast<uint8_t> (static_cast<uint8_t> (timeinfo.tm_mon)	+ 1_ui8);
	p_out.date.day		= static_cast<uint8_t> (timeinfo.tm_mday);
	p_out.time.hour		= static_cast<uint8_t> (timeinfo.tm_hour);
	p_out.time.minute	= static_cast<uint8_t> (timeinfo.tm_min);
	p_out.time.second	= static_cast<uint8_t> (timeinfo.tm_sec);
#endif
}

void date_time_local(date_time_t& p_out, date_time_extra& p_extra)
{
	struct tm timeinfo;

#ifdef _WIN32
	struct __timeb64 timeptr;
	_ftime64_s(&timeptr);
	_localtime64_s(&timeinfo, &timeptr.time);
	p_out.time.nsecond	= static_cast<uint32_t>(timeptr .millitm) * 1000000;
	p_out.date.year		= static_cast<uint16_t>(timeinfo.tm_year) + 1900_ui16;
	p_out.date.month	= static_cast<uint8_t> (timeinfo.tm_mon ) + 1_ui8;
	p_extra.dst			= timeptr.dstflag > 0 ? true : false;
#else
	timeval curTime;
	gettimeofday(&curTime, nullptr);
	localtime_r(&curTime.tv_sec, &timeinfo);
	p_out.time.nsecond	= static_cast<uint16_t>(curTime.tv_usec * 1000);
	p_out.date.year		= static_cast<uint16_t>(static_cast<uint16_t>(timeinfo.tm_year)	+ 1900_ui16);
	p_out.date.month	= static_cast<uint8_t> (static_cast<uint8_t> (timeinfo.tm_mon)	+ 1_ui8);
	p_extra.dst			= timeinfo.tm_isdst > 0 ? true : false;
#endif

	p_out.date.day		= static_cast<uint8_t> (timeinfo.tm_mday);
	p_out.time.hour		= static_cast<uint8_t> (timeinfo.tm_hour);
	p_out.time.minute	= static_cast<uint8_t> (timeinfo.tm_min);
	p_out.time.second	= static_cast<uint8_t> (timeinfo.tm_sec);
	p_extra.week_day	= static_cast<uint8_t> (timeinfo.tm_wday);
}


#ifdef _WIN32

[[nodiscard]] time_point_t system_time_fast()
{
	FILETIME temp;
	GetSystemTimeAsFileTime(&temp);
	return time_point_t
		{
			((static_cast<uint64_t>(temp.dwHighDateTime) << 32) | static_cast<uint64_t>(temp.dwLowDateTime)) * 100
			+ epoch_offset
		};
}

[[nodiscard]] time_point_t system_time_precise()
{
	FILETIME temp;
	GetSystemTimePreciseAsFileTime(&temp);
	return time_point_t
		{
			((static_cast<uint64_t>(temp.dwHighDateTime) << 32) | static_cast<uint64_t>(temp.dwLowDateTime))
			* 100 + epoch_offset
		};
}
#else
[[nodiscard]] time_point_t system_time_fast()
{
	timeval curTime;
	gettimeofday(&curTime, nullptr);
	return time_point_t{static_cast<uint64_t>(curTime.tv_sec) * sec_nsec + curTime.tv_usec * 1000 + epoch_offset};
}

[[nodiscard]] time_point_t system_time_precise()
{
	struct timespec temp{};
	timespec_get(&temp, TIME_UTC);
	//clock_gettime(CLOCK_REALTIME, &temp);
	return time_point_t{static_cast<uint64_t>(temp.tv_sec) * sec_nsec + temp.tv_nsec + epoch_offset};
}
#endif

time_point_t date_to_system_time(date_time_t const& value)
{
	uint8_t month = value.date.month -1;
	if(month > 11) month = 11;

	return time_point_t{year_to_time(value.date.year)
		+ time_till_month(month, is_leap_year(value.date.year))
		+ (value.date.day -1) * day_nsec
		+ value.time.hour   * hour_nsec
		+ value.time.minute * minute_nsec
		+ value.time.second * sec_nsec
		+ value.time.nsecond};
}

date_time_t system_time_to_date(time_point_t value)
{
	date_time_t out;
	uint64_t t_rem;
	{
		uint16_t const t_year = time_to_year(value.raw(), t_rem);
		out.date.year = t_year;
		out.date.month = time_2_Month(t_rem, t_rem, is_leap_year(t_year)) + 1_ui8;
	}

	out.date.day = static_cast<uint8_t>(t_rem / day_nsec + 1_ui8);
	t_rem       %= day_nsec;

	out.time.hour = static_cast<uint8_t>(t_rem / hour_nsec);
	t_rem        %= hour_nsec;

	out.time.minute = static_cast<uint8_t>(t_rem / minute_nsec);
	t_rem         %= minute_nsec;

	out.time.second  = static_cast<uint8_t>(t_rem / sec_nsec);
	out.time.nsecond = static_cast<uint32_t>(t_rem % sec_nsec);

	return out;
}

core::date_time_t to_date(std::chrono::system_clock::time_point const p_time)
{
	std::chrono::time_point const floor{std::chrono::floor<std::chrono::days>(p_time)};
	std::chrono::year_month_day const ymd{floor};
	std::chrono::hh_mm_ss const ttime{p_time - floor};

	using tdate_t = decltype(core::date_time_t::date);
	using ttime_t = decltype(core::date_time_t::time);

	return core::date_time_t
	{
		.date=tdate_t
		{
			.year  = static_cast<uint16_t>(static_cast<int16_t>(static_cast<int>(ymd.year()))),
			.month = static_cast<uint8_t>(static_cast<unsigned>(ymd.month())),
			.day   = static_cast<uint8_t>(static_cast<unsigned>(ymd.day()))
		},
		.time=ttime_t
		{
			.hour    = static_cast<uint8_t>(ttime.hours().count()),
			.minute  = static_cast<uint8_t>(ttime.minutes().count()),
			.second  = static_cast<uint8_t>(ttime.seconds().count()),
			.nsecond = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(ttime.subseconds()).count())
		}
	};
}

} //namespace core

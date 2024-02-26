//======== ======== ======== ======== ======== ======== ======== ========
///	\file
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

#include <CoreLib/core_time.hpp>

#include <CoreLib/core_type.hpp>

#ifdef _WIN32
#include <sys/timeb.h>
#include <windows.h>
#else
#include <sys/time.h>
#endif
#include <time.h>


namespace core
{

namespace core_p
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

static const os_clock_frequency g_WinFreq;
#endif

static constexpr uint64_t g_sec2nsec = uint64_t{1000000000};

}	//namespace core_p


void chrono::set()
{
#ifdef _WIN32
	LARGE_INTEGER Time;
	QueryPerformanceCounter(&Time);
	m_ref = Time.QuadPart;
#else
	struct timespec time;
	clock_gettime(CLOCK_BOOTTIME, &time);
	m_ref = (uint64_t)time.tv_sec * core_p::g_sec2nsec + time.tv_nsec;
#endif
}

uint64_t chrono::elapsed() const //nanosecond resolution
{
#ifdef _WIN32
	const uint64_t freq = core_p::g_WinFreq.frequency();

	LARGE_INTEGER Time;
	QueryPerformanceCounter(&Time);
	uint64_t t_val = Time.QuadPart - m_ref;

	return ((t_val / freq) * core_p::g_sec2nsec) + ((t_val % freq) * core_p::g_sec2nsec) / freq;
#else
	struct timespec time;
	clock_gettime(CLOCK_BOOTTIME, &time);
	return static_cast<uint64_t>(time.tv_sec) * core_p::g_sec2nsec + static_cast<uint64_t>(time.tv_nsec) - m_ref;
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
		m_acumulated += static_cast<uint64_t>(time.tv_sec) * core_p::g_sec2nsec + static_cast<uint64_t>(time.tv_nsec) - m_ref;
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
		m_ref = static_cast<uint64_t>(time.tv_sec) * core_p::g_sec2nsec + static_cast<uint64_t>(time.tv_nsec);
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
	m_ref = static_cast<uint64_t>(time.tv_sec) * core_p::g_sec2nsec + static_cast<uint64_t>(time.tv_nsec);
#endif

	m_acumulated	= 0;
	m_isPaused		= false;
}

uint64_t track_chrono::read() const
{

#ifdef _WIN32
	const uint64_t freq = core_p::g_WinFreq.frequency();
	if(m_isPaused)
	{
		return (m_acumulated / freq) * core_p::g_sec2nsec + ((m_acumulated % freq) * core_p::g_sec2nsec) / freq;
	}

	LARGE_INTEGER Time;
	QueryPerformanceCounter(&Time);
	const uint64_t val = m_acumulated + Time.QuadPart - m_ref;
	return (val / freq) * core_p::g_sec2nsec + ((val % freq) * core_p::g_sec2nsec) / freq;
#else
	if(m_isPaused)
	{
		return m_acumulated;
	}

	struct timespec time;
	clock_gettime(CLOCK_BOOTTIME, &time);
	uint64_t val = m_acumulated + static_cast<uint64_t>(time.tv_sec) * core_p::g_sec2nsec + static_cast<uint64_t>(time.tv_nsec) - m_ref;
	return val;
#endif
}

void track_chrono::set(uint64_t p_value)
{
#ifdef _WIN32
	const uint64_t freq = core_p::g_WinFreq.frequency();
	m_acumulated = (p_value / core_p::g_sec2nsec) * freq + ((p_value % core_p::g_sec2nsec) * freq) / core_p::g_sec2nsec;
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
		m_ref = static_cast<uint64_t>(time.tv_sec) * core_p::g_sec2nsec + static_cast<uint64_t>(time.tv_nsec);
	}
#endif
}


uint64_t clock_stamp() //1 nanosecond resolution
{
#ifdef _WIN32
	LARGE_INTEGER Time;
	QueryPerformanceCounter(&Time);
	const uint64_t t_val = Time.QuadPart;
	const uint64_t freq = core_p::g_WinFreq.frequency();
	return (t_val / freq) * core_p::g_sec2nsec + ((t_val % freq) * core_p::g_sec2nsec) / freq;
#else
	struct timespec time;
	clock_gettime(CLOCK_BOOTTIME, &time);
	return static_cast<uint64_t>(time.tv_sec) * core_p::g_sec2nsec + static_cast<uint64_t>(time.tv_nsec);
#endif
}

void date_time_local(date_time& p_out)
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
	p_out.time.msecond	= static_cast<uint16_t>(timeinfo.wMilliseconds);
#else
	struct tm timeinfo;

	timeval curTime;
	gettimeofday(&curTime, nullptr);
	localtime_r(&curTime.tv_sec, &timeinfo);
	p_out.time.msecond	= static_cast<uint16_t>(curTime.tv_usec / 1000);
	p_out.date.year		= static_cast<uint16_t>(static_cast<uint16_t>(timeinfo.tm_year)	+ uint16_t{1900});
	p_out.date.month	= static_cast<uint8_t> (static_cast<uint8_t> (timeinfo.tm_mon)	+ uint8_t{1});
	p_out.date.day		= static_cast<uint8_t> (timeinfo.tm_mday);
	p_out.time.hour		= static_cast<uint8_t> (timeinfo.tm_hour);
	p_out.time.minute	= static_cast<uint8_t> (timeinfo.tm_min);
	p_out.time.second	= static_cast<uint8_t> (timeinfo.tm_sec);
#endif
}

void date_time_local(date_time& p_out, date_time_extra& p_extra)
{
	struct tm timeinfo;

#ifdef _WIN32
	struct __timeb64 timeptr;
	_ftime64_s(&timeptr);
	_localtime64_s(&timeinfo, &timeptr.time);
	p_out.time.msecond	= (uint16_t) timeptr.millitm;
	p_out.date.year		= static_cast<uint16_t>(timeinfo.tm_year)	+ uint16_t{1900};
	p_out.date.month	= static_cast<uint8_t> (timeinfo.tm_mon)	+ uint8_t{1};
	p_extra.dst			= timeptr.dstflag > 0 ? true : false;
#else
	timeval curTime;
	gettimeofday(&curTime, nullptr);
	localtime_r(&curTime.tv_sec, &timeinfo);
	p_out.time.msecond	= static_cast<uint16_t>(curTime.tv_usec / 1000);
	p_out.date.year		= static_cast<uint16_t>(static_cast<uint16_t>(timeinfo.tm_year)	+ uint16_t{1900});
	p_out.date.month	= static_cast<uint8_t> (static_cast<uint8_t> (timeinfo.tm_mon)	+ uint8_t{1});
	p_extra.dst			= timeinfo.tm_isdst > 0 ? true : false;
#endif

	p_out.date.day		= static_cast<uint8_t> (timeinfo.tm_mday);
	p_out.time.hour		= static_cast<uint8_t> (timeinfo.tm_hour);
	p_out.time.minute	= static_cast<uint8_t> (timeinfo.tm_min);
	p_out.time.second	= static_cast<uint8_t> (timeinfo.tm_sec);
	p_extra.week_day	= static_cast<uint8_t> (timeinfo.tm_wday);
}

/// \todo Accuracy has not been extensively tested
void date_time_UTC(date_time& p_out)
{
#ifdef _WIN32
	SYSTEMTIME timeinfo;
	GetSystemTime(&timeinfo);
	p_out.date.year		= static_cast<uint16_t>(timeinfo.wYear);
	p_out.date.month	= static_cast<uint8_t> (timeinfo.wMonth);
	p_out.date.day		= static_cast<uint8_t> (timeinfo.wDay);
	p_out.time.hour		= static_cast<uint8_t> (timeinfo.wHour);
	p_out.time.minute	= static_cast<uint8_t> (timeinfo.wMinute);
	p_out.time.second	= static_cast<uint8_t> (timeinfo.wSecond);
	p_out.time.msecond	= static_cast<uint16_t>(timeinfo.wMilliseconds);
#else
	struct tm timeinfo;
	struct timezone tz;
	timeval curTime;
	gettimeofday(&curTime, &tz);
	curTime.tv_sec -= tz.tz_minuteswest * 60; //check
	gmtime_r(&curTime.tv_sec, &timeinfo);

	p_out.date.year		= static_cast<uint16_t>(static_cast<uint16_t>(timeinfo.tm_year)	+ uint16_t{1900});
	p_out.date.month	= static_cast<uint8_t> (static_cast<uint8_t> (timeinfo.tm_mon)	+ uint8_t{1});
	p_out.date.day		= static_cast<uint8_t> (timeinfo.tm_mday);
	p_out.time.hour		= static_cast<uint8_t> (timeinfo.tm_hour);
	p_out.time.minute	= static_cast<uint8_t> (timeinfo.tm_min);
	p_out.time.second	= static_cast<uint8_t> (timeinfo.tm_sec);
	p_out.time.msecond	= static_cast<uint16_t>(curTime.tv_usec / 1000);
#endif
}

/// \todo Accuracy has not been extensively tested
void date_time_UTC(date_time& p_out, uint8_t& p_weekDay)
{
#ifdef _WIN32
	SYSTEMTIME timeinfo;
	GetSystemTime(&timeinfo);
	p_out.date.year		= static_cast<uint16_t>(timeinfo.wYear);
	p_out.date.month	= static_cast<uint8_t> (timeinfo.wMonth);
	p_out.date.day		= static_cast<uint8_t> (timeinfo.wDay);
	p_out.time.hour		= static_cast<uint8_t> (timeinfo.wHour);
	p_out.time.minute	= static_cast<uint8_t> (timeinfo.wMinute);
	p_out.time.second	= static_cast<uint8_t> (timeinfo.wSecond);
	p_out.time.msecond	= static_cast<uint16_t>(timeinfo.wMilliseconds);
	p_weekDay			= static_cast<uint8_t> (timeinfo.wDayOfWeek);
#else
	struct tm timeinfo;
	struct timezone tz;
	timeval curTime;
	gettimeofday(&curTime, &tz);
	curTime.tv_sec -= tz.tz_minuteswest * 60; //check
	gmtime_r(&curTime.tv_sec, &timeinfo);

	p_out.date.year		= static_cast<uint16_t>(static_cast<uint16_t>(timeinfo.tm_year)	+ uint16_t{1900});
	p_out.date.month	= static_cast<uint8_t> (static_cast<uint8_t> (timeinfo.tm_mon)	+ uint8_t{1});
	p_out.date.day		= static_cast<uint8_t> (timeinfo.tm_mday);
	p_out.time.hour		= static_cast<uint8_t> (timeinfo.tm_hour);
	p_out.time.minute	= static_cast<uint8_t> (timeinfo.tm_min);
	p_out.time.second	= static_cast<uint8_t> (timeinfo.tm_sec);
	p_out.time.msecond	= static_cast<uint16_t>(curTime.tv_usec / 1000);
	p_weekDay			= static_cast<uint8_t> (timeinfo.tm_wday);
#endif
}


} //namespace core

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

#include <CoreLib/Core_Time.hpp>

#include <CoreLib/Core_Type.hpp>

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
class OS_ClockFrequency
{
private:
	uint64_t m_frequency;
public:
	OS_ClockFrequency()
	{
		LARGE_INTEGER t_Frequency;
		QueryPerformanceFrequency(&t_Frequency);
		m_frequency = t_Frequency.QuadPart ? t_Frequency.QuadPart : 1;
	}

	inline uint64_t frequency() const { return m_frequency; }
};

static const OS_ClockFrequency g_WinFreq;
#endif

static constexpr uint64_t g_sec2nsec = uint64_t{1000000000};

}	//namespace core_p


void Chrono::set()
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

uint64_t Chrono::elapsed() const //nanosecond resolution
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

void TrackChrono::pause()
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

void TrackChrono::resume()
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

void TrackChrono::restart()
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

uint64_t TrackChrono::read() const
{

#ifdef _WIN32
	const uint64_t freq = core_p::g_WinFreq.frequency();
	if(m_isPaused)
	{
		return (m_acumulated / freq) * core_p::g_sec2nsec + ((m_acumulated % freq) * core_p::g_sec2nsec) / freq;
	}

	LARGE_INTEGER Time;
	QueryPerformanceCounter(&Time);
	uint64_t val = m_acumulated + Time.QuadPart - m_ref;
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

void TrackChrono::set(uint64_t p_value)
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
	uint64_t t_val = Time.QuadPart;
	const uint64_t freq = core_p::g_WinFreq.frequency();
	return (t_val / freq) * core_p::g_sec2nsec + ((t_val % freq) * core_p::g_sec2nsec) / freq;
#else
	struct timespec time;
	clock_gettime(CLOCK_BOOTTIME, &time);
	return static_cast<uint64_t>(time.tv_sec) * core_p::g_sec2nsec + static_cast<uint64_t>(time.tv_nsec);
#endif
}

DateTime date_time_local()
{
	DateTime output;
	struct tm timeinfo;

#ifdef _WIN32
	struct __timeb64 timeptr;
	_ftime64_s(&timeptr);
	_localtime64_s(&timeinfo, &timeptr.time);
	output.time.msecond	= (uint16_t) timeptr.millitm;
	output.date.year	= static_cast<uint16_t>(timeinfo.tm_year)	+ uint16_t{1900};
	output.date.month	= static_cast<uint8_t> (timeinfo.tm_mon)	+ uint8_t{1};
#else
	timeval curTime;
	gettimeofday(&curTime, nullptr);
	localtime_r(&curTime.tv_sec, &timeinfo);
	output.time.msecond		= static_cast<uint16_t>(curTime.tv_usec / 1000);
	output.date.year		= static_cast<uint16_t>(static_cast<uint16_t>(timeinfo.tm_year)	+ uint16_t{1900});
	output.date.month		= static_cast<uint8_t> (static_cast<uint8_t> (timeinfo.tm_mon)	+ uint8_t{1});
#endif

	output.date.day			= static_cast<uint8_t> (timeinfo.tm_mday);
	output.time.hour		= static_cast<uint8_t> (timeinfo.tm_hour);
	output.time.minute		= static_cast<uint8_t> (timeinfo.tm_min);
	output.time.second		= static_cast<uint8_t> (timeinfo.tm_sec);
	return output;
}

DateTime date_time_local(DateTime_extra& p_extra)
{
	DateTime output;
	struct tm timeinfo;

#ifdef _WIN32
	struct __timeb64 timeptr;
	_ftime64_s(&timeptr);
	_localtime64_s(&timeinfo, &timeptr.time);
	output.time.msecond	= (uint16_t) timeptr.millitm;
	output.date.year	= static_cast<uint16_t>(timeinfo.tm_year)	+ uint16_t{1900};
	output.date.month	= static_cast<uint8_t> (timeinfo.tm_mon)	+ uint8_t{1};
	p_extra.dst	= timeptr.dstflag > 0 ? true : false;
#else
	timeval curTime;
	gettimeofday(&curTime, nullptr);
	localtime_r(&curTime.tv_sec, &timeinfo);
	output.time.msecond		= static_cast<uint16_t>(curTime.tv_usec / 1000);
	output.date.year		= static_cast<uint16_t>(static_cast<uint16_t>(timeinfo.tm_year)	+ uint16_t{1900});
	output.date.month		= static_cast<uint8_t> (static_cast<uint8_t> (timeinfo.tm_mon)	+ uint8_t{1});
	p_extra.dst		= timeinfo.tm_isdst > 0 ? true : false;
#endif

	output.date.day			= static_cast<uint8_t> (timeinfo.tm_mday);
	output.time.hour		= static_cast<uint8_t> (timeinfo.tm_hour);
	output.time.minute		= static_cast<uint8_t> (timeinfo.tm_min);
	output.time.second		= static_cast<uint8_t> (timeinfo.tm_sec);
	p_extra.week_day	= static_cast<uint8_t> (timeinfo.tm_wday);
	return output;
}


/// \todo Accuracy has not been extensively tested
DateTime date_time_UTC()
{
	DateTime output;
#ifdef _WIN32
	SYSTEMTIME timeinfo;
	GetSystemTime(&timeinfo);
	output.date.year		= static_cast<uint16_t>(timeinfo.wYear);
	output.date.month		= static_cast<uint8_t> (timeinfo.wMonth);
	output.date.day			= static_cast<uint8_t> (timeinfo.wDay);
	output.time.hour		= static_cast<uint8_t> (timeinfo.wHour);
	output.time.minute		= static_cast<uint8_t> (timeinfo.wMinute);
	output.time.second		= static_cast<uint8_t> (timeinfo.wSecond);
	output.time.msecond		= static_cast<uint16_t>(timeinfo.wMilliseconds);
#else
	struct tm timeinfo;
	struct timezone tz;
	timeval curTime;
	gettimeofday(&curTime, &tz);
	curTime.tv_sec -= tz.tz_minuteswest * 60; //check
	gmtime_r(&curTime.tv_sec, &timeinfo);

	output.date.year		= static_cast<uint16_t>(static_cast<uint16_t>(timeinfo.tm_year)	+ uint16_t{1900});
	output.date.month		= static_cast<uint8_t> (static_cast<uint8_t> (timeinfo.tm_mon)	+ uint8_t{1});
	output.date.day			= static_cast<uint8_t> (timeinfo.tm_mday);
	output.time.hour		= static_cast<uint8_t> (timeinfo.tm_hour);
	output.time.minute		= static_cast<uint8_t> (timeinfo.tm_min);
	output.time.second		= static_cast<uint8_t> (timeinfo.tm_sec);
	output.time.msecond		= static_cast<uint16_t>(curTime.tv_usec / 1000);
#endif
	return output;
}

/// \todo Accuracy has not been extensively tested
DateTime date_time_UTC(uint8_t& p_weekDay)
{
	DateTime output;
#ifdef _WIN32
	SYSTEMTIME timeinfo;
	GetSystemTime(&timeinfo);
	output.date.year		= static_cast<uint16_t>(timeinfo.wYear);
	output.date.month		= static_cast<uint8_t> (timeinfo.wMonth);
	output.date.day			= static_cast<uint8_t> (timeinfo.wDay);
	output.time.hour		= static_cast<uint8_t> (timeinfo.wHour);
	output.time.minute		= static_cast<uint8_t> (timeinfo.wMinute);
	output.time.second		= static_cast<uint8_t> (timeinfo.wSecond);
	output.time.msecond		= static_cast<uint16_t>(timeinfo.wMilliseconds);
	p_weekDay				= static_cast<uint8_t> (timeinfo.wDayOfWeek);
#else
	struct tm timeinfo;
	struct timezone tz;
	timeval curTime;
	gettimeofday(&curTime, &tz);
	curTime.tv_sec -= tz.tz_minuteswest * 60; //check
	gmtime_r(&curTime.tv_sec, &timeinfo);

	output.date.year		= static_cast<uint16_t>(static_cast<uint16_t>(timeinfo.tm_year)	+ uint16_t{1900});
	output.date.month		= static_cast<uint8_t> (static_cast<uint8_t> (timeinfo.tm_mon)	+ uint8_t{1});
	output.date.day			= static_cast<uint8_t> (timeinfo.tm_mday);
	output.time.hour		= static_cast<uint8_t> (timeinfo.tm_hour);
	output.time.minute		= static_cast<uint8_t> (timeinfo.tm_min);
	output.time.second		= static_cast<uint8_t> (timeinfo.tm_sec);
	output.time.msecond		= static_cast<uint16_t>(curTime.tv_usec / 1000);
	p_weekDay				= static_cast<uint8_t> (timeinfo.tm_wday);
#endif
	return output;
}


} //namespace core

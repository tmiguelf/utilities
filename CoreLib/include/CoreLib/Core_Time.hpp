//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///		Provides timing capabilities
///
///	\author Tiago Freire
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

#include <cstdint>
#include <cstring>

namespace core
{

/// \brief holds date and time data
struct DateTime
{
	struct
	{
		uint16_t	year;		//!< 0 = Year 0
		uint8_t		month;		//!< 1 = January
		uint8_t		day;		//!< 1 = Day 1
	} date;

	struct
	{
		uint8_t		hour;
		uint8_t		minute;
		uint8_t		second;
		uint16_t	msecond;	//!< milliseconds
	} time;

	struct
	{
		uint8_t		week_day;	//!< 0 = Sunday
		bool		dst;		//!< True if Daylight savings is active
	} extra;
};

/// \brief	The Chrono is an high precision real time chronometer.
///			Its function is similar to TrackChrono, but with a couple of major
///			differences, mainly that Chrono is much simpler, it cannot be paused
///			or resumed, it can only count elapsed time from set().
///			These simplifications reduces overhead, making it less impactful and thus
///			the best implementation for applications where Ultra high time keeping
///			performance is imperative.
///
///	\note	Although the time readout provides a nanosecond resolution, the granularity
///			is still constrained to the what the system can provide, which may mean
///			that the smallest non 0 value can be in the order of 100 nanoseconds or higher
///
///	\warning	Value may overflow before reaching uint64_t limit.
class Chrono
{
private:
	uint64_t m_ref;

public:

	/// \brief sets 0
	void		set();

	/// \brief gets currently elapsed time, 1ns resolution, granularity depends on system
	[[nodiscard]] uint64_t elapsed();
};

/// \brief	The Core_TrackChrono is an high precision real time chronometer.
///			Its function is similar to Chrono with the additional complication
///			of being able to pause and resume the chronometer. This complication
///			adds a minor overhead in relation to Core_Chrono that the user may
///			not want to pay for. If the ability to pause the counter is not required,
///			you should use Core_Chrono instead as this provides less overhead
///			and a better performance.
///
///	\note	Although the time readout provides a nanosecond resolution, the granularity
///			is still constrained to the what the system can provide, which may mean
///			that the smallest non 0 value can be in the order of 100 nanoseconds or higher
///
///	\warning	Internal counter may overflow before reaching uint64_t limit,
///				when this occurs value will be invalid until reset.
class TrackChrono
{
private:
	uint64_t	m_ref;
	uint64_t	m_acumulated = 0;
	bool		m_isPaused = true;

public:

	/// \brief Checks if chronometer is in the paused state
	[[nodiscard]] inline bool is_paused() const { return m_isPaused; }
	
	/// \brief Sets counter to 0 and sets counter state to paused
	inline void clear	()
	{
		m_isPaused		= true;
		m_acumulated	= 0;
	}

	/// \brief Pauses the counter
	void		pause	();

	/// \brief Continues counting from current value
	void		resume	();

	/// \brief Sets counter to 0 and sets counter state to running
	void		restart	();

	/// \brief Reads current value on the chronometer, 1ns resolution, granularity depends on system
	[[nodiscard]] uint64_t read() const;

	/// \brief Sets the current readout to the input value (1ns resolution)
	void		set		(uint64_t p_value);
};

///\brief Current time-stamp, 1ns resolution, granularity depends on system
///	\warning	Value may overflow before reaching uint64_t limit.
[[nodiscard]] uint64_t clockStamp();

/// \brief	gets the current local date and time based on internal clock
[[nodiscard]] DateTime dateTimeLocal();

/// \brief	gets the current UTC date and time based on internal clock
[[nodiscard]] DateTime dateTimeUTC();

} //namespace core

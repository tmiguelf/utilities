//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///		Provides timing capabilities
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

#pragma once

#include <cstdint>
#include <cstring>
#include <chrono>

namespace core
{

struct date_time_t
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
		uint32_t	nsecond;	//!< nanoseconds
	} time;
};


struct date_time_extra
{
	uint8_t week_day;	//!< 0 = Sunday
	bool    dst;		//!< True if Daylight savings is active
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
class chrono
{
private:
	uint64_t m_ref;

public:

	/// \brief sets 0
	void		set();

	/// \brief gets currently elapsed time, 1ns resolution, granularity depends on system
	[[nodiscard]] uint64_t elapsed() const;
};

/// \brief	The track_chrono is an high precision real time chronometer.
///			Its function is similar to Chrono with the additional complication
///			of being able to pause and resume the chronometer. This complication
///			adds a minor overhead in relation to chrono that the user may
///			not want to pay for. If the ability to pause the counter is not required,
///			you should use chrono instead as this provides less overhead
///			and a better performance.
///
///	\note	Although the time readout provides a nanosecond resolution, the granularity
///			is still constrained to the what the system can provide, which may mean
///			that the smallest non 0 value can be in the order of 100 nanoseconds or higher
///
///	\warning	Internal counter may overflow before reaching uint64_t limit,
///				when this occurs value will be invalid until reset.
class track_chrono
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
[[nodiscard]] uint64_t clock_stamp();

/// \brief	gets the current local date and time based on internal clock
void date_time_local(date_time_t& p_out);

/// \brief	gets the current local date and time based on internal clock
void date_time_local(date_time_t& p_out, date_time_extra& p_extra);



class time_delta_t;

class time_point_t
{
public:
	constexpr time_point_t() = default;
	constexpr time_point_t(time_point_t&&) = default;
	constexpr time_point_t(time_point_t const&) = default;

	explicit constexpr time_point_t(uint64_t const p_raw) : raw_data{p_raw} {}

	time_point_t& operator = (time_point_t const &) = default;
	time_point_t& operator = (time_point_t &&) = default;

	constexpr bool operator == (time_point_t const p_other) const { return raw_data == p_other.raw_data; }
	constexpr bool operator != (time_point_t const p_other) const { return raw_data != p_other.raw_data; }
	constexpr bool operator <  (time_point_t const p_other) const { return raw_data <  p_other.raw_data; }
	constexpr bool operator >  (time_point_t const p_other) const { return raw_data >  p_other.raw_data; }
	constexpr bool operator <= (time_point_t const p_other) const { return raw_data <= p_other.raw_data; }
	constexpr bool operator >= (time_point_t const p_other) const { return raw_data >= p_other.raw_data; }

	constexpr time_delta_t operator -  (time_point_t const p_other) const;
	time_point_t operator + (time_delta_t const p_other);
	time_point_t operator - (time_delta_t const p_other);

	time_point_t& operator += (time_delta_t const p_other);
	time_point_t& operator -= (time_delta_t const p_other);

	constexpr uint64_t raw() const { return raw_data; }

private:
	uint64_t raw_data = 0;
};

class time_delta_t
{
public:
	constexpr time_delta_t() = default;
	constexpr time_delta_t(time_delta_t&&) = default;
	constexpr time_delta_t(time_delta_t const&) = default;

	explicit constexpr time_delta_t(int64_t const p_raw) : raw_data{p_raw} {}

	time_delta_t& operator = (time_delta_t const &) = default;
	time_delta_t& operator = (time_delta_t &&) = default;

	constexpr bool operator == (time_delta_t const p_other) const { return raw_data == p_other.raw_data; }
	constexpr bool operator != (time_delta_t const p_other) const { return raw_data != p_other.raw_data; }
	constexpr bool operator <  (time_delta_t const p_other) const { return raw_data <  p_other.raw_data; }
	constexpr bool operator >  (time_delta_t const p_other) const { return raw_data >  p_other.raw_data; }
	constexpr bool operator <= (time_delta_t const p_other) const { return raw_data <= p_other.raw_data; }
	constexpr bool operator >= (time_delta_t const p_other) const { return raw_data >= p_other.raw_data; }


	constexpr time_delta_t  operator - () const
	{
		return time_delta_t{-raw_data};
	}

	constexpr time_delta_t  operator - (time_delta_t const p_other) const
	{
		return time_delta_t{raw_data - p_other.raw_data};
	}

	time_delta_t& operator += (time_delta_t const & p_other)
	{
		raw_data += p_other.raw_data;
		return *this;
	}

	time_delta_t& operator -= (time_delta_t const & p_other)
	{
		raw_data -= p_other.raw_data;
		return *this;
	}

	constexpr int64_t raw() const { return raw_data; }

private:
	int64_t raw_data = 0;
};

inline constexpr time_delta_t time_point_t::operator - (time_point_t const p_other) const
{
	return time_delta_t{static_cast<int64_t>(raw_data - p_other.raw_data)};
}

inline time_point_t time_point_t::operator + (time_delta_t const p_other)
{
	return time_point_t{raw_data + static_cast<uint64_t>(p_other.raw())};
}

inline time_point_t time_point_t::operator - (time_delta_t const p_other)
{
	return time_point_t{raw_data - static_cast<uint64_t>(p_other.raw())};
}

inline time_point_t& time_point_t::operator += (time_delta_t const p_other)
{
	raw_data += static_cast<uint64_t>(p_other.raw());
	return *this;
}

inline time_point_t& time_point_t::operator -= (time_delta_t const p_other)
{
	raw_data -= static_cast<uint64_t>(p_other.raw());
	return *this;
}

[[nodiscard]] time_point_t system_time_fast();
[[nodiscard]] time_point_t system_time_precise();

time_point_t date_to_system_time(date_time_t const& value);
date_time_t system_time_to_date(time_point_t value);

core::date_time_t to_date(std::chrono::system_clock::time_point p_time);


} //namespace core

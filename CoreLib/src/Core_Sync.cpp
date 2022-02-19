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

#include <CoreLib/Core_Sync.hpp>

#ifdef _WIN32
#	include <Windows.h>
#else
#	include <time.h>
#endif

namespace core
{


#ifdef _WIN32
Mutex::Mutex()
{
	m_mutex = nullptr;
}

Mutex::~Mutex()
{
	destroy();
}

SYNC_Error Mutex::create()
{
	destroy();
	m_mutex = CreateMutexA(nullptr, false, "");
	if(m_mutex == nullptr) return SYNC_Error::Fail;
	return SYNC_Error::NoErr;
}

SYNC_Error Mutex::destroy()
{
	if(m_mutex != nullptr)
	{
		if(CloseHandle(m_mutex) == TRUE)
		{
			m_mutex = nullptr;
			return SYNC_Error::NoErr;
		}
		return SYNC_Error::Fail;
	}
	return SYNC_Error::NoErr;
}

SYNC_Error Mutex::lock()
{
	switch(WaitForSingleObject(m_mutex, INFINITE))
	{
		case WAIT_ABANDONED:
		case WAIT_OBJECT_0:
			return SYNC_Error::NoErr;
		default:
			break;
	}
#ifdef __CORE_EXTENDED_ERROR__
	if(m_mutex == nullptr)
	{
		return SYNC_Error::Does_Not_Exist;
	}
#endif
	return SYNC_Error::Fail;
}

SYNC_Error Mutex::try_lock()
{
	switch(WaitForSingleObject(m_mutex, 0))
	{
		case WAIT_ABANDONED:
		case WAIT_OBJECT_0:
			return SYNC_Error::NoErr;
		case WAIT_TIMEOUT:
			return SYNC_Error::WouldBlock;
		default:
			break;
	}
#ifdef __CORE_EXTENDED_ERROR__
	if(m_mutex == nullptr)
	{
		return SYNC_Error::Does_Not_Exist;
	}
#endif
	return SYNC_Error::Fail;
}

SYNC_Error Mutex::unlock()
{
	if(ReleaseMutex(m_mutex) == TRUE) return SYNC_Error::NoErr;
#ifdef __CORE_EXTENDED_ERROR__
	if(m_mutex == nullptr)
	{
		return SYNC_Error::Does_Not_Exist;
	}
#endif
	return SYNC_Error::Fail;
}


Semaphore::Semaphore()
{
	m_semaphore = nullptr;
}

Semaphore::~Semaphore()
{
	destroy();
}

SYNC_Error Semaphore::create(std::u8string& p_name, const uint32_t p_range)
{
	destroy();
#ifdef __CORE_EXTENDED_ERROR__
	if(p_range > 0x7FFFFFFF || p_name.size() > MAX_PATH) return SYNC_Error::Invalid_Argument;
#endif
	m_semaphore = CreateSemaphoreA(nullptr, p_range, p_range, reinterpret_cast<const char*>(p_name.c_str()));
	if(m_semaphore == nullptr) return SYNC_Error::Fail;
	return SYNC_Error::NoErr;
}

SYNC_Error Semaphore::create(std::u16string& p_name, const uint32_t p_range)
{
	destroy();
#ifdef __CORE_EXTENDED_ERROR__
	if(p_range > 0x7FFFFFFF || p_name.size() > MAX_PATH) return SYNC_Error::Invalid_Argument;
#endif
	m_semaphore = CreateSemaphoreW(nullptr, p_range, p_range, reinterpret_cast<const wchar_t*>(p_name.c_str()));
	if(m_semaphore == nullptr) return SYNC_Error::Fail;
	return SYNC_Error::NoErr;
}

SYNC_Error Semaphore::create(const uint32_t p_range)
{
	destroy();
#ifdef __CORE_EXTENDED_ERROR__
	if(p_range > 0x7FFFFFFF) return SYNC_Error::Invalid_Argument;
#endif
	m_semaphore = CreateSemaphoreA(nullptr, p_range, p_range, "");
	if(m_semaphore == nullptr) return SYNC_Error::Fail;
	return SYNC_Error::NoErr;
}

SYNC_Error Semaphore::destroy()
{
	if(m_semaphore != nullptr)
	{
		if(CloseHandle(m_semaphore) == TRUE)
		{
			m_semaphore = nullptr;
			return SYNC_Error::NoErr;
		}
		return SYNC_Error::Fail;
	}
	return SYNC_Error::NoErr;
}

SYNC_Error Semaphore::wait()
{
	switch(WaitForSingleObject(m_semaphore, INFINITE))
	{
		case WAIT_ABANDONED:
		case WAIT_OBJECT_0:
			return SYNC_Error::NoErr;
		default:
			break;
	}
#ifdef __CORE_EXTENDED_ERROR__
	if(m_semaphore == nullptr)
	{
		return SYNC_Error::Does_Not_Exist;
	}
#endif
	return SYNC_Error::Fail;
}

SYNC_Error Semaphore::try_wait()
{
	switch(WaitForSingleObject(m_semaphore, 0))
	{
		case WAIT_ABANDONED:
		case WAIT_OBJECT_0:
			return SYNC_Error::NoErr;
		case WAIT_TIMEOUT:
			return SYNC_Error::WouldBlock;
		default:
			break;
	}
#ifdef __CORE_EXTENDED_ERROR__
	if(m_semaphore == nullptr)
	{
		return SYNC_Error::Does_Not_Exist;
	}
#endif
	return SYNC_Error::Fail;
}

SYNC_Error Semaphore::post()
{
	if(ReleaseSemaphore(m_semaphore, 1, nullptr) == TRUE) return SYNC_Error::NoErr;
#ifdef __CORE_EXTENDED_ERROR__
	if(m_semaphore == nullptr)
	{
		return SYNC_Error::Does_Not_Exist;
	}
#endif
	return SYNC_Error::Fail;
}

EventTrap::EventTrap()
{
	m_event = nullptr;
}

EventTrap::~EventTrap()
{
	destroy();
}

SYNC_Error EventTrap::create()
{
	destroy();
	if(((m_event = CreateEventA(nullptr, TRUE, FALSE, nullptr))) != 0)
	{
		return SYNC_Error::NoErr;
	}
	return SYNC_Error::Fail;
}

void EventTrap::destroy()
{
	if(m_event)
	{
		CloseHandle(m_event);
		m_event = nullptr;
	}
}

SYNC_Error EventTrap::signal()
{
	if(SetEvent(m_event))
	{
		return SYNC_Error::NoErr;
	}
#ifdef __CORE_EXTENDED_ERROR__
	if(m_event == nullptr) return SYNC_Error::Does_Not_Exist;
#endif
	return SYNC_Error::Fail;
}

SYNC_Error EventTrap::reset()
{
	if(ResetEvent(m_event))
	{
		return SYNC_Error::NoErr;
	}
#ifdef __CORE_EXTENDED_ERROR__
	if(m_event == nullptr) return SYNC_Error::Does_Not_Exist;
#endif
	return SYNC_Error::Fail;
}

SYNC_Error EventTrap::wait()
{
	switch(WaitForSingleObjectEx(m_event, INFINITE, TRUE))
	{
		case WAIT_OBJECT_0:
			return SYNC_Error::NoErr;
		case WAIT_TIMEOUT:
		case WAIT_IO_COMPLETION:
			return SYNC_Error::PreEmptive;
		default:
			break;
	}
#ifdef __CORE_EXTENDED_ERROR__
	if(m_event == nullptr) return SYNC_Error::Does_Not_Exist;
#endif
	return SYNC_Error::Fail;
}

SYNC_Error EventTrap::timed_wait(const uint32_t p_miliseconds)
{
	switch(WaitForSingleObjectEx(m_event, p_miliseconds, TRUE))
	{
		case WAIT_OBJECT_0:
			return SYNC_Error::NoErr;
		case WAIT_TIMEOUT:
			return SYNC_Error::TimeOut;
		case WAIT_IO_COMPLETION:
			return SYNC_Error::PreEmptive;
		default:
			break;
	}
#ifdef __CORE_EXTENDED_ERROR__
	if(m_event == nullptr) return SYNC_Error::Does_Not_Exist;
#endif
	return SYNC_Error::Fail;
}

#else //OS

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

Mutex::Mutex()
{
	m_init = false;
}

Mutex::~Mutex()
{
	destroy();
}

SYNC_Error Mutex::create()
{
	destroy();
	if(pthread_mutex_init(&m_mutex, nullptr)) return SYNC_Error::Fail;
	m_init = true;
	return SYNC_Error::NoErr;
}

SYNC_Error Mutex::destroy()
{
	if(m_init)
	{
		if(pthread_mutex_destroy(&m_mutex)) return SYNC_Error::Fail;
		m_init = false;
		return SYNC_Error::NoErr;
	}
	return SYNC_Error::Does_Not_Exist;
}

SYNC_Error Mutex::lock()
{
	if(pthread_mutex_lock(&m_mutex) == 0) return SYNC_Error::NoErr;

#ifdef __CORE_EXTENDED_ERROR__
	if(!m_init) return SYNC_Error::Does_Not_Exist;
#endif

	return SYNC_Error::Fail;
}

SYNC_Error Mutex::try_lock()
{
	switch(pthread_mutex_trylock(&m_mutex))
	{
		case 0:
			return SYNC_Error::NoErr;
		case EBUSY:
			return SYNC_Error::WouldBlock;
		default:
			break;
	}

#ifdef __CORE_EXTENDED_ERROR__
	if(!m_init) return SYNC_Error::Does_Not_Exist;
#endif

	return SYNC_Error::Fail;
}

SYNC_Error Mutex::unlock()
{
	if(pthread_mutex_unlock(&m_mutex) == 0) return SYNC_Error::NoErr;

#ifdef __CORE_EXTENDED_ERROR__
	if(!m_init) return SYNC_Error::Does_Not_Exist;
#endif

	return SYNC_Error::Fail;
}


Semaphore::Semaphore()
{
	m_semaphore = nullptr;
}

Semaphore::~Semaphore()
{
	destroy();
}

SYNC_Error Semaphore::create(std::u8string& p_name, const uint32_t p_range)
{
	destroy();
#ifdef __CORE_EXTENDED_ERROR__
	if(p_name.size() > PATH_MAX) return SYNC_Invalid_Argument;
#endif
	is_named = true;

	m_semaphore = sem_open(reinterpret_cast<const char*>(p_name.c_str()), O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH , p_range);
	if(m_semaphore != nullptr) return SYNC_Error::NoErr;
	return SYNC_Error::Fail;
}

SYNC_Error Semaphore::create(const uint32_t p_range)
{
	destroy();
	is_named = false;
	m_semaphore = &m_unSem;
	if(sem_init(m_semaphore, 0, p_range)) return SYNC_Error::Fail;
	return SYNC_Error::NoErr;
}

SYNC_Error Semaphore::destroy()
{
	if(m_semaphore != nullptr)
	{
		if(is_named)
		{
			if(sem_close(m_semaphore)) return SYNC_Error::Fail;
			m_semaphore = nullptr;
			return SYNC_Error::NoErr;
		}
		else
		{
			if(sem_destroy(m_semaphore)) return SYNC_Error::Fail;
			m_semaphore = nullptr;
			return SYNC_Error::NoErr;
		}
	}
	return SYNC_Error::NoErr;
}

SYNC_Error Semaphore::wait()
{
	if(sem_wait(m_semaphore) == 0) return SYNC_Error::NoErr;
#ifdef __CORE_EXTENDED_ERROR__
	if(m_semaphore == nullptr)
	{
		return SYNC_Error::Does_Not_Exist;
	}
#endif
	return SYNC_Error::Fail;
}

SYNC_Error Semaphore::try_wait()
{
	switch(sem_trywait(m_semaphore))
	{
		case 0:
			return SYNC_Error::NoErr;
		case EAGAIN:
			return SYNC_Error::WouldBlock;
		default:
			break;
	}
#ifdef __CORE_EXTENDED_ERROR__
	if(m_semaphore == nullptr)
	{
		return SYNC_Error::Does_Not_Exist;
	}
#endif
	return SYNC_Error::Fail;
}

SYNC_Error Semaphore::post()
{
	if(sem_post(m_semaphore) == 0) return SYNC_Error::NoErr;
#ifdef __CORE_EXTENDED_ERROR__
	if(m_semaphore == nullptr)
	{
		return SYNC_Error::Does_Not_Exist;
	}
#endif
	return SYNC_Error::Fail;
}

EventTrap::EventTrap()
{
	m_init	= false;
}

EventTrap::~EventTrap()
{
	destroy();
}

SYNC_Error EventTrap::create()
{
	pthread_condattr_t attr;
	destroy();
	m_cond = false;
	pthread_condattr_init(&attr);
	pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
	if(pthread_cond_init(&m_condition, &attr))
	{
		return SYNC_Error::Fail;
	}

	if(pthread_mutex_init(&m_mutex, nullptr))
	{
		pthread_cond_destroy(&m_condition);
		return SYNC_Error::Fail;
	}
	m_init = true;
	return SYNC_Error::NoErr;
}

void EventTrap::destroy()
{
	if(m_init)
	{
		pthread_cond_destroy(&m_condition);
		pthread_mutex_destroy(&m_mutex);
		m_init = false;
	}
}

SYNC_Error EventTrap::reset()
{
	pthread_mutex_lock(&m_mutex);
	m_cond = false;
	pthread_mutex_unlock(&m_mutex);
	return SYNC_Error::NoErr;
}

SYNC_Error EventTrap::signal()
{
	int ret;
	pthread_mutex_lock(&m_mutex);
	m_cond = true;
	ret = pthread_cond_signal(&m_condition);
	pthread_mutex_unlock(&m_mutex);

	if(ret == 0) return SYNC_Error::NoErr;

#ifdef __CORE_EXTENDED_ERROR__
	if(!m_init) return SYNC_Error::Does_Not_Exist;
#endif

	return SYNC_Error::Fail;
}

/*SYNC_Error EventTrap::BroadCast()
{
	int ret;
	pthread_mutex_lock(&m_mutex);
	m_cond = true;
	ret = pthread_cond_broadcast(&m_condition);
	pthread_mutex_unlock(&m_mutex);

	if(ret == 0) return SYNC_Error::NoErr;

#ifdef __CORE_EXTENDED_ERROR__
	if(!m_init) return SYNC_Error::Does_Not_Exist;
#endif

	return SYNC_Error::Fail;
}*/

SYNC_Error EventTrap::wait()
{
	bool condition;
	pthread_mutex_lock(&m_mutex);
	if(m_cond)
	{
		pthread_mutex_unlock(&m_mutex);
		return SYNC_Error::NoErr;
	}
	if(pthread_cond_wait(&m_condition, &m_mutex))
	{
		pthread_mutex_unlock(&m_mutex);
#ifdef __CORE_EXTENDED_ERROR__
		if(!m_init) return SYNC_Error::Does_Not_Exist;
#endif
		return SYNC_Error::Fail;
	}
	condition = m_cond;
	pthread_mutex_unlock(&m_mutex);
	if(condition)
	{
		return SYNC_Error::NoErr;
	}

	return SYNC_Error::PreEmptive;
}

SYNC_Error EventTrap::timed_wait(const uint32_t p_miliseconds)
{
	struct timespec end;
	uint64_t counter;
	bool condition;
	int ret;
	clock_gettime(CLOCK_MONOTONIC, &end);

	counter = (end.tv_sec + 5) * 1000000000 + p_miliseconds * 1000000 + end.tv_nsec;

	end.tv_sec	= counter / 1000000000;
	end.tv_nsec	= counter % 1000000000;

	pthread_mutex_lock(&m_mutex);
	if(m_cond)
	{
		pthread_mutex_unlock(&m_mutex);
		return SYNC_Error::NoErr;
	}
	ret = pthread_cond_timedwait(&m_condition, &m_mutex, &end);
	if(ret)
	{
		pthread_mutex_unlock(&m_mutex);
		if(ret == ETIMEDOUT) return SYNC_Error::TimeOut;
#ifdef __CORE_EXTENDED_ERROR__
		if(!m_init) return SYNC_Error::Does_Not_Exist;
#endif
		return SYNC_Error::Fail;
	}
	condition = m_cond;
	pthread_mutex_unlock(&m_mutex);
	if(condition)
	{
		return SYNC_Error::NoErr;
	}

	return SYNC_Error::PreEmptive;
}

/*SYNC_Error EventTrap::Peek()
{
	if(m_cond)
	{
		return SYNC_Error::WouldBlock;
	}
	return SYNC_Error::NoErr;
}*/

#endif

} //namespace core

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

#include <CoreLib/core_sync.hpp>

#ifdef _WIN32
#	include <limits>
#	include <Windows.h>
#else
#	include <time.h>
#endif

namespace core
{


#ifdef _WIN32
mutex::mutex()
	: m_mutex(CreateMutexW(nullptr, false, nullptr))
{
}

mutex::~mutex()
{
	if(m_mutex != nullptr)
	{
		if(!CloseHandle(m_mutex))
		{
			if(SetHandleInformation(m_mutex, HANDLE_FLAG_PROTECT_FROM_CLOSE, 0))
			{
				CloseHandle(m_mutex);
			}
		}
	}
}

SYNC_Error mutex::lock()
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

SYNC_Error mutex::try_lock()
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

SYNC_Error mutex::unlock()
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


semaphore::semaphore(uint32_t p_range)
{
	if(p_range > static_cast<uint32_t>(std::numeric_limits<LONG>::max())) return;
	m_semaphore = CreateSemaphoreW(nullptr, p_range, p_range, nullptr);
}

semaphore::semaphore(std::u8string& p_name, uint32_t p_range)
{
	if(p_range > static_cast<uint32_t>(std::numeric_limits<LONG>::max())) return;
	m_semaphore = CreateSemaphoreA(nullptr, p_range, p_range, reinterpret_cast<const char*>(p_name.c_str()));
}

semaphore::semaphore(std::u16string& p_name, uint32_t p_range)
{
	if(p_range > static_cast<uint32_t>(std::numeric_limits<LONG>::max())) return;
	m_semaphore = CreateSemaphoreW(nullptr, p_range, p_range, reinterpret_cast<const wchar_t*>(p_name.c_str()));
}

semaphore::~semaphore()
{
	if(m_semaphore)
	{
		if(!CloseHandle(m_semaphore))
		{
			if(SetHandleInformation(m_semaphore, HANDLE_FLAG_PROTECT_FROM_CLOSE, 0))
			{
				CloseHandle(m_semaphore);
			}
		}
	}
}

SYNC_Error semaphore::wait()
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

SYNC_Error semaphore::try_wait()
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

SYNC_Error semaphore::post()
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

event_trap::event_trap()
	: m_event(CreateEventW(nullptr, TRUE, FALSE, nullptr))
{
}

event_trap::~event_trap()
{
	if(m_event)
	{
		if(!CloseHandle(m_event))
		{
			if(SetHandleInformation(m_event, HANDLE_FLAG_PROTECT_FROM_CLOSE, 0))
			{
				CloseHandle(m_event);
			}
		}
	}
}

SYNC_Error event_trap::signal()
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

SYNC_Error event_trap::reset()
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

SYNC_Error event_trap::wait()
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

SYNC_Error event_trap::timed_wait(const uint32_t p_miliseconds)
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

mutex::mutex()
{
	m_init = (pthread_mutex_init(&m_mutex, nullptr) == 0);
}

mutex::~mutex()
{
	if(m_init)
	{
		int const ret = pthread_mutex_destroy(&m_mutex);
		if(ret)
		{
			//todo
			if(ret == EBUSY)
			{
				lock();
				pthread_mutex_destroy(&m_mutex);
			}
		}
	}
}

SYNC_Error mutex::lock()
{
	if(pthread_mutex_lock(&m_mutex) == 0) return SYNC_Error::NoErr;

#ifdef __CORE_EXTENDED_ERROR__
	if(!m_init) return SYNC_Error::Does_Not_Exist;
#endif

	return SYNC_Error::Fail;
}

SYNC_Error mutex::try_lock()
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

SYNC_Error mutex::unlock()
{
	if(pthread_mutex_unlock(&m_mutex) == 0) return SYNC_Error::NoErr;

#ifdef __CORE_EXTENDED_ERROR__
	if(!m_init) return SYNC_Error::Does_Not_Exist;
#endif

	return SYNC_Error::Fail;
}


semaphore::semaphore(uint32_t p_range)
{
	is_named = false;
	if(!sem_init(m_semaphore, 0, p_range))
	{
		m_semaphore = &m_unSem;
	}
}

semaphore::semaphore(std::u8string& p_name, uint32_t p_range)
{
	is_named = true;
	m_semaphore = sem_open(reinterpret_cast<const char*>(p_name.c_str()), O_CREAT | O_CLOEXEC, DEFFILEMODE, p_range);
}

semaphore::~semaphore()
{
	if(m_semaphore)
	{
		if(is_named)
		{
			sem_close(m_semaphore);
		}
		else
		{
			sem_destroy(m_semaphore);
		}
	}
}

SYNC_Error semaphore::wait()
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

SYNC_Error semaphore::try_wait()
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

SYNC_Error semaphore::post()
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

event_trap::event_trap()
{
	m_init = false;
	m_cond = false;

	pthread_condattr_t attr;
	pthread_condattr_init(&attr);
	pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);

	if(pthread_cond_init(&m_condition, &attr))
	{
		return;
	}

	if(pthread_mutex_init(&m_mutex, nullptr))
	{
		pthread_cond_destroy(&m_condition);
		return;
	}
	m_init = true;
}

event_trap::~event_trap()
{
	if(m_init)
	{
		pthread_cond_destroy(&m_condition);
		pthread_mutex_destroy(&m_mutex);
	}
}

SYNC_Error event_trap::reset()
{
	pthread_mutex_lock(&m_mutex);
	m_cond = false;
	pthread_mutex_unlock(&m_mutex);
	return SYNC_Error::NoErr;
}

SYNC_Error event_trap::signal()
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

/*SYNC_Error event_trap::BroadCast()
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

SYNC_Error event_trap::wait()
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

SYNC_Error event_trap::timed_wait(const uint32_t p_miliseconds)
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

/*SYNC_Error event_trap::Peek()
{
	if(m_cond)
	{
		return SYNC_Error::WouldBlock;
	}
	return SYNC_Error::NoErr;
}*/

#endif

} //namespace core

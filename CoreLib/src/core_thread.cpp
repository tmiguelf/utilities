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

//======== ======== ======== ======== Header ======== ======== ======== ========
//======== ======== ======== Include ======== ======== ========
//---- User Libraries ----
#include <CoreLib/core_thread.hpp>

#include <algorithm>

#ifdef _WIN32
#	include <windows.h>
#else
#	include <signal.h>
#	include <unistd.h>
#	include <sys/syscall.h>
#	include <sys/types.h>
#	include <time.h>
#	include <errno.h>
#endif
//======== ======== ======== Include END ======== ======== ========
//======== ======== ======== ======== Header END ======== ======== ======== ========

namespace core
{

namespace _p
{

_thread_obj_redir::~_thread_obj_redir() = default;

///	\internal
///	\brief Helper struct to pass information about which function to call and its parameter.
//
struct FP_Func
{
	void	(*m_func)(void *);	//!< Function to call
	void*	m_param;			//!< Parameter to pass
};

void _thread_call_object_assist(void* const p_arg)
{
	static_cast<_thread_obj_redir*>(p_arg)->call();
}

}	//namespace _p


#ifdef _WIN32

namespace _p
{
/// \brief Helper function used to standardize thread launches
//
static DWORD WINAPI launch_thread(void* const p_param)
{
	FP_Func* const t_param = reinterpret_cast<FP_Func*const >(p_param);
	void (*t_function)(void *) = t_param->m_func;
	void* t_arg = t_param->m_param;

	delete t_param;

	t_function(t_arg);

	return (DWORD) 0;
}

}	//namespace _p



thread_id_t current_thread_id ()
{
	return GetCurrentThreadId();
}

void thread_yield()
{
	SwitchToThread();
}

void milliSleep(uint16_t p_time)
{
	Sleep(p_time);
}

thread::thread() = default;

thread::thread(thread&& p_other)
{
	swap(p_other);
}

thread::~thread()
{
	if(m_handle)
	{
		CloseHandle(m_handle);
	}
}

void thread::swap(thread& p_other)
{
	std::swap(m_handle, p_other.m_handle);
	std::swap(m_id, p_other.m_id);
}

thread::Error thread::create(void (* const p_function)(void *), void* const p_param)
{
	if(m_handle) return Error::AlreadyInUse;

	_p::FP_Func* data = new _p::FP_Func;
	data->m_func	= p_function;
	data->m_param	= p_param;

	DWORD lID;
	m_handle = ::CreateThread(nullptr, 0, _p::launch_thread, data, 0, &lID);
	m_id = lID;

	if(m_handle)
	{
		return Error::None;
	}

	delete data;
	return Error::Fail;
}


thread::Error thread::join(uint32_t p_time)
{
	if(m_handle)
	{
		DWORD result = WaitForSingleObject(m_handle, (DWORD) p_time);
		if(WAIT_OBJECT_0 != result)
		{
			if(result == WAIT_TIMEOUT) return Error::Timeout;
			return Error::Fail;
		}

		CloseHandle(m_handle);
		m_handle = nullptr;
	}
	return Error::None;
}

void thread::detach()
{
	if(m_handle)
	{
		CloseHandle(m_handle);
		m_handle = nullptr;
	}
}

thread::Error thread::set_affinity_mask(uint64_t const p_affinity)
{
	DWORD_PTR t_affinity;
#ifndef _WIN64
	if(p_affinity > 0xFFFFFFFF) return Error::Fail;
#endif
	t_affinity = (DWORD_PTR) p_affinity;

	if(m_handle)
	{
		return SetThreadAffinityMask(m_handle, t_affinity) ? Error::None : Error::Fail;
	}

	return Error::Unavailable;
}

thread::Error thread::_setPreferedProcessor(uint8_t const p_num)
{
	if(p_num > MAXIMUM_PROCESSORS) return Error::Fail;
	if(m_handle)
	{
		return (SetThreadIdealProcessor(m_handle, (DWORD) p_num) == DWORD(-1)) ? Error::Fail : Error::None;
	}
	return Error::Unavailable;
}

void milli_sleep(uint16_t p_time)
{
	Sleep(p_time);
}

#else

namespace _p
{
	/// \brief Helper function used to standardize thread launches
	//
	static void* launch_thread(void* const p_param)
	{
		FP_Func* t_param = reinterpret_cast<FP_Func*const >(p_param);
		void (*const t_function)(void *) = t_param->m_func;
		void* t_arg = t_param->m_param;
		delete t_param;
		
		t_function(t_arg);
		
		return (void*) 0;
	}

}	//namespace core_p


thread::thread() = default;

thread::thread(thread&& p_other)
{
	swap(p_other);
}

thread::~thread()
{
	if(m_hasThread) pthread_detach(m_handle);
}

void thread::swap(thread& p_other)
{
	std::swap(m_handle, p_other.m_handle);
	std::swap(m_hasThread, p_other.m_hasThread);
}

thread::Error thread::create(void (*const p_function)(void *), void* const p_param)
{
	if(m_hasThread) return Error::AlreadyInUse;

	_p::FP_Func* data = new _p::FP_Func;

	data->m_func	= p_function;
	data->m_param	= p_param;

	if(pthread_create(&m_handle, nullptr, _p::launch_thread, (void*) data) == 0)
	{
		m_hasThread = true;
		return Error::None;
	}
	else
	{
		delete data;
		return Error::Fail;
	}
}

thread::Error thread::join(uint32_t const p_time)
{
	if(!m_hasThread) return Error::None;
	
	void* result;
	if(p_time == 0)
	{
		int ret = pthread_tryjoin_np(m_handle, &result);
		if(ret)
		{
			if(ret == EBUSY) return Error::Timeout;
			return Error::Fail;
		}
	}
	else if(p_time != Infinite)
	{
		timespec tw;
		if (clock_gettime(CLOCK_REALTIME, &tw) == -1)
		{
			return Error::Fail;
		}
		tw.tv_sec	+= p_time / 1000;
		tw.tv_nsec	+= p_time % 1000;

		int ret = pthread_timedjoin_np(m_handle, &result, &tw);
		if(ret)
		{
			if(ret == ETIMEDOUT) return Error::Timeout;
			return Error::Fail;
		}
	}
	else
	{
		if(pthread_join(m_handle, &result))
		{
			return Error::Fail;
		}
	}
	m_hasThread = false;
	return Error::None;
}

void thread::detach()
{
	if(m_hasThread)
	{
		pthread_detach(m_handle);
		m_hasThread = false;
	}
}

thread::Error thread::set_affinity_mask(uint64_t const p_affinity)
{
	if(m_hasThread)
	{
		uint64_t	t_bias;
		cpu_set_t*	t_set	= CPU_ALLOC(64);
		uintptr_t	t_size	= CPU_ALLOC_SIZE(64);
		if(t_set == nullptr) return Error::Fail;

		CPU_ZERO_S(t_size, t_set);

		for(uint8_t i = 0; i < 64; ++i)
		{
			t_bias = p_affinity >> i;
			if(t_bias & 0x01)
			{
				CPU_SET(i, t_set);
			}
			if(t_bias == 0) break;
		}

		int ret = pthread_setaffinity_np(m_handle, t_size, t_set);

		CPU_FREE(t_set);
		return ret ? Error::Fail : Error::None;
	}
	return Error::Unavailable;
}


thread_id_t current_thread_id()
{
	return static_cast<thread_id_t>(syscall(SYS_gettid));
}

void milli_sleep(uint16_t p_time)
{
	usleep(p_time * 1000);
}

#endif

} //namespace core

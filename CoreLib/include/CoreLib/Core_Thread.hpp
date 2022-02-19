//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///		Provides thread management functions
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

#pragma once

//======== ======== ======== Include ======== ======== ========
#include <cstdint>

//---- Default Libraries ----
#ifndef _WIN32
#	include <pthread.h>
#endif
//======== ======== ======== Include END ======== ======== ========

/// \n
namespace core
{

#ifdef _WIN32
using thread_id_t = uint32_t;
#else
using thread_id_t = pthread_t;
#endif // 



/// \internal \n
namespace _p
{

class _thread_obj_redir
{
public:
	virtual ~_thread_obj_redir();
	virtual void call() = 0;
};

template <class T>
class thread_obj_redir: public _thread_obj_redir
{
public:
	using T_method_p = void	(T::*)(void*);

private:
	T* const			m_object;
	T_method_p const	m_method;
	void* const			m_arg;

public:
	thread_obj_redir(T* const p_obj, T_method_p const p_method, void* const p_arg):
		m_object(p_obj),
		m_method(p_method),
		m_arg	(p_arg)
	{}

	void call() override
	{
		T* const			t_obj		= m_object;
		T_method_p const	t_method	= m_method;
		void* const			t_arg		= m_arg;

		//Object needs to be destroyed before calling the user method because user might call thread_exit
		delete this;
		(t_obj->*t_method)(t_arg);
	}
};

void _thread_call_object_assist(void*);

}	//namespace _p

///	\brief
///		Used to create threads or to manage thread related actions (such as joining, detatching, etc...)
class Thread
{
public:
	enum class Error: uint8_t
	{
		None			= 0x00,	//!< The operation succeeded
		AlreadyInUse	= 0x01,	//!< The thread handle is already in use, please join the thread or detatch before trying to re-use this object for a new thread
		Detatched		= 0x02,	//!< The operation could not complete because the thread is already detached
		Unavailable		= 0x03,	//!< The action could not be completed because there is no active thread handle. Please spawn a thread first.
		Fail			= 0xFE,	//!< Generic failure
		Timeout			= 0xFF	//!< A timed operation was requested on the thread, but the time-out was reached before the action could be completed
	};

	static constexpr uint32_t Infinite = 0xFFFFFFFF;
private:
	Thread& operator = (const Thread&)	= delete;
	Thread& operator = (Thread&&)		= delete;
	Thread(const Thread&)				= delete;

#ifdef _WIN32
	void*		m_handle = nullptr;
	thread_id_t	m_id = 0;
#else
	thread_id_t	m_handle;
	bool		m_hasThread = false;
#endif

public:
	Thread();

	Thread(Thread&& p_other);

	///	\note
	///		If a thread has been launched, but it has neither been \ref join or \ref detach
	///		the destructor will detach the thread.
	~Thread();

	void swap(Thread& p_other);

	///	\brief Spawns a thread and gives it to user control via a function pointer
	///
	///	\param[in] p_function	- The function to call
	///	\param[in] p_param		- Additional argument to be passed to the user function.
	///
	///	\return		\ref Error_None if the thread was launched successfully, other \ref Error code on failure.
	///
	///	\remarks
	///			When a thread is successfully created, the thread objects goes into an "occupied" state.
	///			The user he user must either use \ref join or \ref detach
	///			the thread in order to avoid resource leaks and return this object to a "reusable" state.
	Error create(void (*p_function)(void *), void* p_param);

	///	\brief Tries to join a thread
	///
	///	\param[in] p_function	- The function to call
	///	\param[in] p_param		- Additional argument to be passed to the user function.
	///
	///	\return		\ref Error::None if the thread has been sucessfully joined or the object was in the "free" state.
	///				\ref Error::Timeout if the specified timeout period is reached before the thread was successfully joined.
	///				Other \ref Error code on failure.
	Error join(uint32_t p_time = Infinite);

	/// \brief Detaches the thread.
	///			After this call, the user no longer needs to keep track of the thread handle.
	///			The resources will be cleaned automatically when the thread exists.
	void detach();

	///	\brief Checks if this object is safe to be re-used
	///	\return true if no thread is associated to this object or the thread is detached.
	[[nodiscard]] bool joinable() const;

	///	\brief Request the operating system to schedule the thread only on specific core encoded in the bit mask
	///
	///	\param[in]	A bit mask with each bit representing one logical core. Maximum 64 cores supported.
	///
	///	\return \ref Error code
	///
	///	\warning
	///		This system is not NUMA aware
	Error set_affinity_mask(uint64_t p_affinity);

	///	\return Operating system given number for this thread
	[[nodiscard]] inline thread_id_t id() const;

#ifdef _WIN32
	//this method is mostly sugestive, it is not enforceable
	Error _setPreferedProcessor(uint8_t p_num);
#endif

	///	\brief Spawns a thread and gives it to user control via an object method
	///
	///	\param[in] p_object	- Object to use
	///	\param[in] p_method	- Method to call
	///	\param[in] p_param	- Additional argument to be passed to the user method.
	///
	///	\return		\ref Error_None if the thread was launched successfully, other \ref Errror code on failure.
	///
	///	\remarks
	///			When a thread is successfully created, the thread objects goes into an "occupied" state.
	///			The user he user must either use \ref join or \ref detach
	///			the thread in order to avoid resource leaks and return this object to a "reusable" state.
	template <class T>
	Error create(T* const p_object, void (T::*const p_method)(void *), void* const p_param)
	{
		if(m_handle) return Error::AlreadyInUse;

		_p::thread_obj_redir<T>* t_obj = new _p::thread_obj_redir<T>(p_object, p_method, p_param);

		Error ret = create(_p::_thread_call_object_assist, static_cast<_p::_thread_obj_redir*>(t_obj));
		if(ret != Error::None)
		{
			delete t_obj;
		}
		return ret;
	}
};

#ifdef _WIN32
inline thread_id_t	Thread::id			() const { return m_id;					}
inline bool			Thread::joinable	() const { return m_handle != nullptr;	}

///	\brief Gets the current thread ID as seen by the OS
[[nodiscard]] thread_id_t current_thread_id();

///	\brief Yields the currently alloted time slot for the current thread
void thread_yield();

///	\brief Suspends the thread execution by a number of milliseconds
///
///	`\param[in] p_time - The number of milliseconds to sleep
///
///	\warning
///			The time the thread actually sleeps for is not accurate.
///			The thread may wake up later than the requested time depending on the operating system scheduling
///			The thread may wake up earlier in case an alertable interrupt occurs
void milli_sleep(uint16_t p_time);

#else

inline thread_id_t	Thread::id			() const { return m_handle;		}
inline bool			Thread::joinable	() const { return m_hasThread;	}


///	\brief Gets the current thread ID as seen by the OS
[[nodiscard]] thread_id_t current_thread_id();

///	\brief Yields the currently alloted time slot for the current thread
inline void thread_yield() { pthread_yield(); }

///	\brief Suspends the thread execution by a number of milliseconds
///
///	`\param[in] p_time - The number of milliseconds to sleep
///
///	\warning
///			The time the thread actually sleeps for is not accurate.
///			The thread may wake up later than the requested time depending on the operating system scheduling
///			The thread may wake up earlier in case an alertable interrupt occurs
void milli_sleep(uint16_t p_time);

#endif

} //namespace core

//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///		Provides thread synchronization utilities
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
#include <string>
#include <atomic>


#ifndef _WIN32
#	include <pthread.h>
#	include <semaphore.h>
#endif

namespace core
{

/// \brief Synchronization error codes
enum class SYNC_Error: uint8_t
{
	NoErr				= 0x00,
	Fail				= 0x01,
	Does_Not_Exist		= 0x02,
	//Invalid_Argument	= 0x03,
	PreEmptive			= 0x04,
	Unknown				= 0xF0,
	TimeOut				= 0xFF,
	WouldBlock			= 0xFF
};


///	\brief	Encapsulates a mutex
///			Mutex needs to be created before use, instantiating a class does not create the mutex
///			mutex is destroyed if it goes out of scope
class mutex
{
private:
#ifdef _WIN32
	void* m_mutex = nullptr;
#else
	pthread_mutex_t	m_mutex{};
	bool			m_init = false;
#endif

public:
	mutex();
	~mutex();
	
	core::SYNC_Error lock();

	///	\brief		Attempts to acquire the mutex in a non locking form
	///	\return		0 is returned if mutex is acquired, if mutex is already locked CORE_ERROR::SYNC_WouldBlock is returned
	///				on error a code from \ref CORE_ERROR::SYNC is returned
	[[nodiscard]]
	core::SYNC_Error try_lock();

	///	\brief		Unlocks the mutex
	///	\return		0 on success, or an error code from \ref CORE_ERROR::SYNC
	core::SYNC_Error unlock();

	[[nodiscard]]
	inline bool initialized() const
	{
#ifdef _WIN32
		return m_mutex != nullptr;
#else
		return m_init;
#endif
	}

public:
	///	\brief	Class to use scope locking technics with a mutex
	///			Works similar to a smart pointer, but for mutexes
	///			When the object is constructed it takes the mutex object
	///			as an argument and locks it.
	///			When this class goes out of scope (and the object is destroyed)
	///			the lock is released.
	class scope_locker
	{
		mutex& m_mux;
	public:
		inline scope_locker(mutex& p_mux): m_mux(p_mux) { p_mux.lock(); }
		inline ~scope_locker() { m_mux.unlock(); }
	};

};


///	\brief	Encapsulates a Semaphore
///			Semaphore needs to be created before use, instantiating a class does not create the semaphore
///			Semaphore is destroyed if it goes out of scope
class semaphore
{
private:
#ifdef _WIN32
	void*	m_semaphore;
#else
	sem_t	m_unSem;
	sem_t*	m_semaphore = nullptr;
	bool	is_named;
#endif

public:
	semaphore(uint32_t p_range);
	semaphore(std::u8string const& p_name, uint32_t p_range);
#ifdef _WIN32
	semaphore(std::u16string const& p_name, uint32_t p_range);
#endif

	~semaphore();


	///	\brief	tries to acquire the semaphore in a blocking way
	///	\return	0 on success, or an error code from \ref SYNC_Error
	core::SYNC_Error wait();

	///	\brief		Attempts to acquire the semaphore in a non locking form
	///	\return		0 is returned if semaphore is acquired, if semaphore is already maxed SYNC_Error::WouldBlock is returned
	///				on error a code from \ref SYNC_Error is returned
	[[nodiscard]]
	core::SYNC_Error try_wait();

	///	\brief	releases the semaphore
	///	\return	0 on success, or an error code from \ref SYNC_Error
	core::SYNC_Error post();

	[[nodiscard]]
	inline bool initialized() const
	{
		return m_semaphore != nullptr;
	}
};

///	\brief	Encapsulates an event trap i.e. on call thread blocks execution until a separate thread signals it.
///			EventTrap needs to be created before use, instantiating a class does not create the EventTrap.
///			EventTrap is destroyed if it goes out of scope.
class event_trap
{
private:
#ifdef _WIN32
	void*			m_event;
#else
	pthread_cond_t	m_condition;
	pthread_mutex_t	m_mutex;
	bool			m_init = false;
	bool			m_cond;
#endif

/*	enum
	{
		Infinite = 0xFFFFFFFF
	};*/

public:
	event_trap();
	~event_trap();

	///	\brief		Clears the unlock flag, subsequent calls to \ref wait and \ref timed_wait will block
	core::SYNC_Error reset();

	///	\brief		Signals the trap to be unlocked
	core::SYNC_Error signal();
	//byte_t	BroadCast();

	///	\brief		Blocks the execution of the thread until the trap is unlocked
	core::SYNC_Error wait();
	
	///	\brief		Blocks the execution of the thread until the trap is unlocked or a timeout value is reached
	///	\param		p_miliseconds	time in milliseconds to wait before returning prematurely
	///	\return		0 if unlocked via a signal, \ref SYNC_Error::TimeOut if unlocked via timer, or an error code from \ref SYNC_Error.
	core::SYNC_Error timed_wait(uint32_t p_miliseconds);
	
	//core::SYNC_Error	Peek(); //todo

	[[nodiscard]]
	inline bool initialized() const
	{
#ifdef _WIN32
		return m_event != nullptr;
#else
		return m_init;
#endif
	}
};

///	\brief	Uses atomic flag to implement simple spinlock
class atomic_spinlock
{
private:
	std::atomic_flag m_lock = ATOMIC_FLAG_INIT;

public:
	
	/// \brief	SpinLocks, returns only when lock is acquired.
	/// \note	Each call to lock() must have a subsequent call to unlock()
	/// \warning
	///		There are no deadlock safety checks, including calls on the same thread.
	inline void lock   () { while(m_lock.test_and_set(std::memory_order::acquire)); }

	/// \brief		Releases the lock.
	/// \warning	This happens regardless of either or not the current thread has acquired the lock.
	inline void unlock () { m_lock.clear(std::memory_order::release); }

	/// \brief	Attempts to acquire the lock
	/// \note
	///			If lock is acquired there should be a subsequent call to unlock(),
	///			otherwise it should not.
	/// \return	true if lock was acquired sucessfully, false if otherwise
	inline bool try_lock() { return !m_lock.test_and_set(std::memory_order::acquire); }

public:

	///	\brief	Class to use scope locking technics with a AtomicSpinLock
	///			Works similar to a smart pointer, but for AtomicSpinLock
	///			When the object is constructed it takes the AtomicSpinLock object
	///			as an argument and locks it.
	///			When this class goes out of scope (and the object is destroyed)
	///			the lock is released.
	class scope_locker
	{
		atomic_spinlock& m_lock;
	public:
		inline scope_locker(atomic_spinlock& p_mux): m_lock(p_mux) { p_mux.lock(); }
		inline ~scope_locker() { m_lock.unlock(); }
	};
};

}	//namespace core

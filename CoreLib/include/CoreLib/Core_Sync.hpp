//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///		Provides thread synchronization utilities
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
	Invalid_Argument	= 0x03,
	PreEmptive			= 0x04,
	Unknown				= 0xF0,
	TimeOut				= 0xFF,
	WouldBlock			= 0xFF
};


///	\brief	Encapsulates a mutex
///			Mutex needs to be created before use, instantiating a class does not create the mutex
///			mutex is destroyed if it goes out of scope
class Mutex
{
#ifdef _WIN32
	void* m_mutex;
#else
	pthread_mutex_t	m_mutex;
	bool			m_init;
#endif

public:
	Mutex();
	~Mutex();

	///	\brief		Creates the mutex
	///	\return		0 on success, or an error code from \ref CORE_ERROR::SYNC
	core::SYNC_Error createMutex();

	///	\brief		Destroys the mutex
	///	\return		0 on success, or an error code from \ref CORE_ERROR::SYNC
	core::SYNC_Error destroyMutex();
	
	///	\brief	Locks the mutex
	///	\return		0 on success, or an error code from \ref CORE_ERROR::SYNC
	core::SYNC_Error lock();

	///	\brief		Attempts to acquire the mutex in a non locking form
	///	\return		0 is returned if mutex is acquired, if mutex is already locked CORE_ERROR::SYNC_WouldBlock is returned
	///				on error a code from \ref CORE_ERROR::SYNC is returned
	[[nodiscard]]
	core::SYNC_Error tryLock();

	///	\brief		Unlocks the mutex
	///	\return		0 on success, or an error code from \ref CORE_ERROR::SYNC
	core::SYNC_Error unlock();


public:
	///	\brief	Class to use scope locking technics with a mutex
	///			Works similar to a smart pointer, but for mutexes
	///			When the object is constructed it takes the mutex object
	///			as an argument and locks it.
	///			When this class goes out of scope (and the object is destroyed)
	///			the lock is released.
	class ScopeLocker
	{
		Mutex& m_mux;
	public:
		inline ScopeLocker(Mutex& p_mux): m_mux(p_mux) { p_mux.lock(); }
		inline ~ScopeLocker() { m_mux.unlock(); }
	};

};


///	\brief	Encapsulates a Semaphore
///			Semaphore needs to be created before use, instantiating a class does not create the semaphore
///			Semaphore is destroyed if it goes out of scope
class Semaphore
{
#ifdef _WIN32
	void*	m_semaphore;
#else
	sem_t	m_unSem;
	sem_t*	m_semaphore;
	bool	is_named;
#endif

public:
	Semaphore();
	~Semaphore();

	///	\brief		Creates a named semaphore
	///	\return		0 on success, or an error code from \ref SYNC_Error
	core::SYNC_Error createSemaphore(std::u8string& p_name, uint32_t p_range);
	
	#ifdef _WIN32
	///	\brief		Creates a named semaphore
	///	\return		0 on success, or an error code from \ref SYNC_Error
	///	\note		Windows only
	core::SYNC_Error createSemaphore(std::u16string& p_name, uint32_t p_range);
	#endif

	///	\brief		Creates an unnamed semaphore
	///	\return		0 on success, or an error code from \ref SYNC_Error
	core::SYNC_Error createSemaphore(uint32_t p_range);

	///	\brief		Destroys the semaphore
	///	\return		0 on success, or an error code from \ref SYNC_Error
	core::SYNC_Error destroySemaphore();

	///	\brief	tries to acquire the semaphore in a blocking way
	///	\return	0 on success, or an error code from \ref SYNC_Error
	core::SYNC_Error wait();

	///	\brief		Attempts to acquire the semaphore in a non locking form
	///	\return		0 is returned if semaphore is acquired, if semaphore is already maxed SYNC_Error::WouldBlock is returned
	///				on error a code from \ref SYNC_Error is returned
	[[nodiscard]]
	core::SYNC_Error tryWait();

	///	\brief	releases the semaphore
	///	\return	0 on success, or an error code from \ref SYNC_Error
	core::SYNC_Error post();
};

///	\brief	Encapsulates an event trap i.e. on call thread blocks execution until a separate thread signals it.
///			EventTrap needs to be created before use, instantiating a class does not create the EventTrap.
///			EventTrap is destroyed if it goes out of scope.
class EventTrap
{
private:
#ifdef _WIN32
	void*			m_event;
#else
	pthread_cond_t	m_condition;
	pthread_mutex_t	m_mutex;
	bool			m_init;
	bool			m_cond;
#endif

/*	enum
	{
		Infinite = 0xFFFFFFFF
	};*/

public:
	EventTrap();
	~EventTrap();

	///	\brief		Creates the trap
	///	\return		0 on success, or an error code from \ref SYNC_Error
	core::SYNC_Error createTrap();
	
	///	\brief		Destroys the trap
	void destroyTrap();

	///	\brief		Clears the unlock flag, subsequent calls to \ref Wait and \ref TimedWait will block
	///	\return		0 on success, or an error code from \ref SYNC_Error
	core::SYNC_Error reset();

	///	\brief		Signals the trap to be unlocked
	///	\return		0 on success, or an error code from \ref SYNC_Error
	core::SYNC_Error signal();
	//byte_t	BroadCast();

	///	\brief		Blocks the execution of the thread until the trap is unlocked
	///	\return		0 on success, or an error code from \ref SYNC_Error
	core::SYNC_Error wait();
	
	///	\brief		Blocks the execution of the thread until the trap is unlocked or a timeout value is reached
	///	\param		p_miliseconds	time in milliseconds to wait before returning prematurely
	///	\return		0 if unlocked via a signal, \ref SYNC_Error::TimeOut if unlocked via timer, or an error code from \ref SYNC_Error.
	core::SYNC_Error timedWait(uint32_t p_miliseconds);
	//byte_t	Peek();
};

///	\brief	Uses atomic bool to implement simple spinlock
class AtomicSpinLock
{
private:
	std::atomic_flag m_lock = ATOMIC_FLAG_INIT;

public:
	
	/// \brief	SpinLocks, returns only when lock is acquired.
	/// \note	Each call to lock() must have a subsequent call to unlock()
	/// \warning
	///		There are no deadlock safety checks, including calls on the same thread.
	inline void lock   () { while(m_lock.test_and_set(std::memory_order_acquire)); }

	/// \brief		Releases the lock.
	/// \warning	This happens regardless of either or not the current thread has acquired the lock.
	inline void unlock () { m_lock.clear(std::memory_order_release); }

	/// \brief	Attempts to acquire the lock
	/// \note
	///			If lock is acquired there should be a subsequent call to unlock(),
	///			otherwise it should not.
	/// \return	true if lock was acquired sucessfully, false if otherwise
	inline bool tryLock() { return !m_lock.test_and_set(std::memory_order_acquire); }

public:

	///	\brief	Class to use scope locking technics with a mutex
	///			Works similar to a smart pointer, but for mutexes
	///			When the object is constructed it takes the mutex object
	///			as an argument and locks it.
	///			When this class goes out of scope (and the object is destroyed)
	///			the lock is released.
	class ScopeLocker
	{
		AtomicSpinLock& m_lock;
	public:
		inline ScopeLocker(AtomicSpinLock& p_mux): m_lock(p_mux) { p_mux.lock(); }
		inline ~ScopeLocker() { m_lock.unlock(); }
	};
};

}	//namespace core

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

#pragma once

#include <cstdint>
#include <filesystem>
#include <system_error>

namespace core
{
	namespace _p
	{
		class file_base
		{
		public:
			enum class open_mode: uint8_t
			{
				create = 0,
				crete_if_new,
				open_or_create,
				open_existing,
			};

		public:
			file_base() = default;
			~file_base();

			void close();
			[[nodiscard]] inline bool is_open() const { return m_handle ? true : false; }
			
			[[nodiscard]] int64_t pos() const;

			std::errc seek(int64_t p_pos);
			std::errc seek_current(int64_t p_pos);
			std::errc seek_end(int64_t p_pos);

			[[nodiscard]] bool eof  () const;
			[[nodiscard]] bool error() const;
			[[nodiscard]] bool good () const;

			void clear_error();

			[[nodiscard]] int64_t size() const;

			void lock();
			void unlock();
#ifdef _WIN32
			void close_unlocked();
			[[nodiscard]] int64_t pos_unlocked() const;

			[[nodiscard]] std::errc seek_unlocked(int64_t p_pos);
			[[nodiscard]] std::errc seek_current_unlocked(int64_t p_pos);
			[[nodiscard]] std::errc seek_end_unlocked(int64_t p_pos);
#else
			[[nodiscard]] bool eof_unlocked  () const;
			[[nodiscard]] bool error_unlocked() const;
			[[nodiscard]] bool good_unlocked () const;
			void clear_error_unlocked();

			[[nodiscard]] bool try_lock();
#endif // _WIN32

			inline void*& handle() { return m_handle; }

		protected:
			void* m_handle = nullptr;

		private:
			file_base(file_base const&) = delete;
			file_base(file_base&&) = delete;
			file_base& operator = (file_base const&) = delete;
			file_base& operator = (file_base&&) = delete;
		};

	} //namespace _p

	class file_read: public _p::file_base
	{
	public:
		std::errc open(std::filesystem::path const& p_path);
		uintptr_t read(void* p_buff, uintptr_t p_size);
		uintptr_t read_unlocked(void* p_buff, uintptr_t p_size);

#ifndef _WIN32
		uintptr_t read_offset(void* p_buff, uintptr_t p_size, int64_t p_offset);
#endif
	};

	class file_write: public _p::file_base
	{
	public:
		std::errc open(std::filesystem::path const& p_path, open_mode p_mode, bool p_create_directories = true);
		uintptr_t write(void const* p_buff, uintptr_t p_size);
		void flush();
		std::errc resize(int64_t p_size);

		uintptr_t write_unlocked(void const* p_buff, uintptr_t p_size);
		void flush_unlocked();

#ifndef _WIN32
		uintptr_t write_offset(void const* p_buff, uintptr_t p_size, int64_t p_offset);
#endif
	};

	class file_duplex: public _p::file_base
	{
	public:
		std::errc open(std::filesystem::path const& p_path, open_mode p_mode, bool p_create_directories = true);

		uintptr_t read (void* p_buff, uintptr_t p_size);
		uintptr_t write(void const* p_buff, uintptr_t p_size);
		void flush();
		std::errc resize(int64_t p_size);

		uintptr_t read_unlocked(void* p_buff, uintptr_t p_size);
		uintptr_t write_unlocked(void const* p_buff, uintptr_t p_size);
		void flush_unlocked();

#ifndef _WIN32
		uintptr_t read_offset(void* p_buff, uintptr_t p_size, int64_t p_offset);
		uintptr_t write_offset(void const* p_buff, uintptr_t p_size, int64_t p_offset);
#endif
	};

} //namespace core

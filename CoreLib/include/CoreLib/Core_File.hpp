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

#pragma once

#include <cstdint>
#include <filesystem>

namespace core
{
	namespace _p
	{
		class file_base
		{
		public:
			enum open_mode: uint8_t
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
			inline bool is_open() const { return m_handle ? true : false; }
			
			int64_t pos() const;

			bool seek(int64_t p_pos);
			bool seek_current(int64_t p_pos);
			bool seek_end(int64_t p_pos);

			bool eof  () const;
			bool error() const;
			bool good () const;

			void clear_error();

			int64_t size() const;

			void lock();
			void unlock();
#ifdef _WIN32
			void close_unlocked();
			int64_t pos_unlocked() const;

			bool seek_unlocked(int64_t p_pos);
			bool seek_current_unlocked(int64_t p_pos);
			bool seek_end_unlocked(int64_t p_pos);
#else
			bool try_lock();
#endif // _WIN32


		protected:
			void* m_handle = nullptr;

		private:
			file_base(const file_base&) = delete;
			file_base(file_base&&) = delete;
			file_base& operator = (const file_base&) = delete;
			file_base& operator = (file_base&&) = delete;
		};
	} //namespace _p

	class file_read: public _p::file_base
	{
	public:
		bool open(std::filesystem::path& p_path);
		uintptr_t read(void* p_buff, uintptr_t p_size);
		uintptr_t read_unlocked(void* p_buff, uintptr_t p_size);

#ifndef _WIN32
		uintptr_t read_offset(void* p_buff, uintptr_t p_size, int64_t p_offset);
#endif
	};

	class file_write: public _p::file_base
	{
	public:
		bool open(std::filesystem::path& p_path, open_mode p_mode);
		uintptr_t write(void* p_buff, uintptr_t p_size);
		void flush();
		bool resize(int64_t p_size);

		uintptr_t write_unlocked(void* p_buff, uintptr_t p_size);
		void flush_unlocked();

#ifndef _WIN32
		uintptr_t write_offset(void* p_buff, uintptr_t p_size, int64_t p_offset);
#endif
	};

	class file_duplex: public _p::file_base
	{
	public:
		bool open(std::filesystem::path& p_path, open_mode p_mode);

		uintptr_t read (void* p_buff, uintptr_t p_size);
		uintptr_t write(void* p_buff, uintptr_t p_size);
		void flush();
		bool resize(int64_t p_size);

		uintptr_t read_unlocked(void* p_buff, uintptr_t p_size);
		uintptr_t write_unlocked(void* p_buff, uintptr_t p_size);
		void flush_unlocked();

#ifndef _WIN32
		uintptr_t read_offset(void* p_buff, uintptr_t p_size, int64_t p_offset);
		uintptr_t write_offset(void* p_buff, uintptr_t p_size, int64_t p_offset);
#endif
	};


} //namespace core

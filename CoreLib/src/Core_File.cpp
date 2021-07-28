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

#ifdef _WIN32
#	include <io.h>
#else
//#	define _LARGEFILE64_SOURCE
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <CoreLib/Core_File.hpp>
#include <cstdio>
#include <fcntl.h>

namespace core
{
	namespace
	{
#ifdef _WIN32
		static inline void os_lock_file(void* const p_handle)
		{
			_lock_file(reinterpret_cast<FILE*>(p_handle));
		}
	
		static inline void os_unlock_file(void* const p_handle)
		{
			_unlock_file(reinterpret_cast<FILE*>(p_handle));
		}
#else
		static inline void os_lock_file(void* const p_handle)
		{
			flockfile(reinterpret_cast<FILE*>(p_handle));
		}
	
		static inline void os_unlock_file(void* const p_handle)
		{
			funlockfile(reinterpret_cast<FILE*>(p_handle));
		}
		
		static inline bool os_try_lock_file(void* const p_handle)
		{
			return ftrylockfile(reinterpret_cast<FILE*>(p_handle)) ? false : true;
		}
#endif

#ifdef _WIN32
		constexpr int fixed_flags = _O_BINARY | _O_NOINHERIT;

		static inline void* os_open_read(const std::filesystem::path& p_path)
		{
			int fd0 = _wopen(p_path.native().c_str(), _O_RDONLY | fixed_flags);
			if(fd0 >= 0)
			{
				FILE* fd = _wfdopen(fd0, L"rb");
				if(fd)
				{
					return fd;
				}
				close(fd0);
			}

			return nullptr;
		}

		static inline void* os_open_write(const std::filesystem::path& p_path, _p::file_base::open_mode p_mode)
		{
			int fd0;
			switch(p_mode)
			{
			case _p::file_base::open_mode::create:
				fd0 = _wopen(p_path.native().c_str(), _O_WRONLY | _O_CREAT | _O_TRUNC | fixed_flags);
				break;
			case _p::file_base::open_mode::crete_if_new:
				fd0 = _wopen(p_path.native().c_str(), _O_WRONLY | _O_CREAT | _O_EXCL | fixed_flags);
				break;
			case _p::file_base::open_mode::open_or_create:
				fd0 = _wopen(p_path.native().c_str(), _O_WRONLY | _O_CREAT | fixed_flags);
				break;
			case _p::file_base::open_mode::open_existing:
				fd0 = _wopen(p_path.native().c_str(), _O_WRONLY | fixed_flags);
				break;
			default:
				return nullptr;
			}

			if(fd0 >= 0)
			{
				FILE* fd = _wfdopen(fd0, L"wb");
				if(fd)
				{
					return fd;
				}
				close(fd0);
			}
			return nullptr;
		}

		static inline void* os_open_duplex(const std::filesystem::path& p_path, _p::file_base::open_mode p_mode)
		{
			int fd0;
			switch(p_mode)
			{
			case _p::file_base::open_mode::create:
				fd0 = _wopen(p_path.native().c_str(), _O_RDWR | _O_CREAT | _O_TRUNC | fixed_flags);
				break;
			case _p::file_base::open_mode::crete_if_new:
				fd0 = _wopen(p_path.native().c_str(), _O_RDWR | _O_CREAT | _O_EXCL | fixed_flags);
				break;
			case _p::file_base::open_mode::open_or_create:
				fd0 = _wopen(p_path.native().c_str(), _O_RDWR | _O_CREAT | fixed_flags);
				break;
			case _p::file_base::open_mode::open_existing:
				fd0 = _wopen(p_path.native().c_str(), _O_RDWR | fixed_flags);
				break;
			default:
				return nullptr;
			}

			if(fd0 >= 0)
			{
				FILE* fd = _wfdopen(fd0, L"r+b");
				if(fd)
				{
					return fd;
				}
				close(fd0);
			}
			return nullptr;
		}

		static inline void os_close(void* const p_handle)
		{
			fclose(reinterpret_cast<FILE*>(p_handle));
		}

		static inline void os_close_unlocked(void* const p_handle)
		{
			_fclose_nolock(reinterpret_cast<FILE*>(p_handle));
		}

		static inline int64_t os_tell(void* const p_handle)
		{
			return _ftelli64(reinterpret_cast<FILE*>(p_handle));
		}

		static inline int64_t os_tell_unlocked(void* const p_handle)
		{
			return _ftelli64_nolock(reinterpret_cast<FILE*>(p_handle));
		}

		static inline std::errc os_seek(void* const p_handle, int64_t p_pos, int p_mode)
		{
			return std::errc{_fseeki64(reinterpret_cast<FILE*>(p_handle), p_pos, p_mode) ? errno : 0};
		}

		static inline std::errc os_seek_unlocked(void* const p_handle, int64_t p_pos, int p_mode)
		{
			return std::errc{_fseeki64_nolock(reinterpret_cast<FILE*>(p_handle), p_pos, p_mode) ? errno : 0};
		}

		static inline bool os_eof(void* const p_handle)
		{
			return feof(reinterpret_cast<FILE*>(p_handle)) ? true : false;
		}

		static inline bool os_error(void* const p_handle)
		{
			return ferror(reinterpret_cast<FILE*>(p_handle)) ? true : false;
		}

		static inline void os_clear_error(void* const p_handle)
		{
			clearerr_s(reinterpret_cast<FILE*>(p_handle));
		}

		static inline int64_t os_file_lenght(void* const p_handle)
		{
			return _filelengthi64(_fileno(reinterpret_cast<FILE*>(p_handle)));
		}

		static inline std::errc os_file_resize(void* const p_handle, int64_t p_size)
		{
			return std::errc{_chsize_s(_fileno(reinterpret_cast<FILE*>(p_handle)), p_size) ? errno : 0};
		}

		static inline uintptr_t os_read(void* const p_handle, void* p_buff, uintptr_t p_size)
		{
			return fread(p_buff, 1, p_size, reinterpret_cast<FILE*>(p_handle));
		}

		static inline uintptr_t os_read_unlocked(void* const p_handle, void* p_buff, uintptr_t p_size)
		{
			return _fread_nolock(p_buff, 1, p_size, reinterpret_cast<FILE*>(p_handle));
		}

		static inline uintptr_t os_write(void* const p_handle, void* p_buff, uintptr_t p_size)
		{
			return fwrite(p_buff, 1, p_size, reinterpret_cast<FILE*>(p_handle));
		}

		static inline uintptr_t os_write_unlocked(void* const p_handle, void* p_buff, uintptr_t p_size)
		{
			return _fwrite_nolock(p_buff, 1, p_size, reinterpret_cast<FILE*>(p_handle));
		}

		static inline void os_flush(void* const p_handle)
		{
			fflush(reinterpret_cast<FILE*>(p_handle));
		}

		static inline void os_flush_unlocked(void* const p_handle)
		{
			_fflush_nolock(reinterpret_cast<FILE*>(p_handle));
		}
#else
		constexpr int fixed_flags = O_CLOEXEC;
		constexpr mode_t fixed_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
		static inline void* os_open_read(const std::filesystem::path& p_path)
		{
			int fd0 = open64(p_path.native().c_str(), O_RDONLY | fixed_flags);
			if(fd0 >= 0)
			{
				FILE* fd = fdopen(fd0, "rb");
				if(fd)
				{
					return fd;
				}
				close(fd0);
			}

			return nullptr;
		}

		static inline void* os_open_write(const std::filesystem::path& p_path, _p::file_base::open_mode p_mode)
		{
			int fd0;
			switch(p_mode)
			{
			case _p::file_base::open_mode::create:
				fd0 = open64(p_path.native().c_str(), O_WRONLY | O_CREAT | O_TRUNC | fixed_flags, fixed_mode);
				break;
			case _p::file_base::open_mode::crete_if_new:
				fd0 = open64(p_path.native().c_str(), O_WRONLY | O_CREAT | O_EXCL | fixed_flags, fixed_mode);
				break;
			case _p::file_base::open_mode::open_or_create:
				fd0 = open64(p_path.native().c_str(), O_WRONLY | O_CREAT | fixed_flags, fixed_mode);
				break;
			case _p::file_base::open_mode::open_existing:
				fd0 = open64(p_path.native().c_str(), O_WRONLY | fixed_flags);
				break;
			default:
				return nullptr;
			}

			if(fd0 >= 0)
			{
				FILE* fd = fdopen(fd0, "wb");
				if(fd)
				{
					return fd;
				}
				close(fd0);
			}
			return nullptr;
		}

		static inline void* os_open_duplex(const std::filesystem::path& p_path, _p::file_base::open_mode p_mode)
		{
			int fd0;
			switch(p_mode)
			{
			case _p::file_base::open_mode::create:
				fd0 = open64(p_path.native().c_str(), O_RDWR | O_CREAT | O_TRUNC | fixed_flags, fixed_mode);
				break;
			case _p::file_base::open_mode::crete_if_new:
				fd0 = open64(p_path.native().c_str(), O_RDWR | O_CREAT | O_EXCL | fixed_flags, fixed_mode);
				break;
			case _p::file_base::open_mode::open_or_create:
				fd0 = open64(p_path.native().c_str(), O_RDWR | O_CREAT | fixed_flags, fixed_mode);
				break;
			case _p::file_base::open_mode::open_existing:
				fd0 = open64(p_path.native().c_str(), O_RDWR | fixed_flags);
				break;
			default:
				return nullptr;
			}

			if(fd0 >= 0)
			{
				FILE* fd = fdopen(fd0, "r+b");
				if(fd)
				{
					return fd;
				}
				close(fd0);
			}
			return nullptr;
		}

		static inline void os_close(void* const p_handle)
		{
			fclose(reinterpret_cast<FILE*>(p_handle));
		}

		static inline int64_t os_tell(void* const p_handle)
		{
			return ftello64(reinterpret_cast<FILE*>(p_handle));
		}

		static inline std::errc os_seek(void* const p_handle, int64_t p_pos, int p_mode)
		{
			return std::errc{fseeko64(reinterpret_cast<FILE*>(p_handle), p_pos, p_mode) ? errno : 0};
		}

		static inline bool os_eof(void* const p_handle)
		{
			return feof(reinterpret_cast<FILE*>(p_handle)) ? true : false;
		}

		static inline bool os_error(void* const p_handle)
		{
			return ferror(reinterpret_cast<FILE*>(p_handle)) ? true : false;
		}

		static inline void os_clear_error(void* const p_handle)
		{
			clearerr(reinterpret_cast<FILE*>(p_handle));
		}

		static inline int64_t os_file_lenght(void* const p_handle)
		{
			struct stat64 stat_value;
			if(fstat64(fileno(reinterpret_cast<FILE*>(p_handle)), &stat_value))
			{
				return -1;
			}
			return stat_value.st_size;
		}

		static inline std::errc os_file_resize(void* const p_handle, int64_t p_size)
		{
			return std::errc{ftruncate64(fileno(reinterpret_cast<FILE*>(p_handle)), p_size) ? errno : 0};
		}

		static inline uintptr_t os_read(void* const p_handle, void* p_buff, uintptr_t p_size)
		{
			return fread(p_buff, 1, p_size, reinterpret_cast<FILE*>(p_handle));
		}

		static inline uintptr_t os_read_unlocked(void* const p_handle, void* p_buff, uintptr_t p_size)
		{
			return fread_unlocked(p_buff, 1, p_size, reinterpret_cast<FILE*>(p_handle));
		}

		static inline uintptr_t os_read_offset(void* const p_handle, void* p_buff, uintptr_t p_size, int64_t p_offset)
		{
			ssize_t res = pread64(fileno(reinterpret_cast<FILE*>(p_handle)), p_buff, p_size, p_offset);
			return (res == -1) ? 0 : static_cast<uintptr_t>(res);
		}

		static inline uintptr_t os_write(void* const p_handle, void* p_buff, uintptr_t p_size)
		{
			return fwrite(p_buff, 1, p_size, reinterpret_cast<FILE*>(p_handle));
		}

		static inline uintptr_t os_write_unlocked(void* const p_handle, void* p_buff, uintptr_t p_size)
		{
			return fwrite_unlocked(p_buff, 1, p_size, reinterpret_cast<FILE*>(p_handle));
		}

		static inline uintptr_t os_write_offset(void* const p_handle, void* p_buff, uintptr_t p_size, int64_t p_offset)
		{
			ssize_t res = pwrite64(fileno(reinterpret_cast<FILE*>(p_handle)), p_buff, p_size, p_offset);
			return (res == -1) ? 0 : static_cast<uintptr_t>(res);
		}

		static inline void os_flush(void* const p_handle)
		{
			fflush(reinterpret_cast<FILE*>(p_handle));
		}

		static inline void os_flush_unlocked(void* const p_handle)
		{
			fflush_unlocked(reinterpret_cast<FILE*>(p_handle));
		}
#endif

	} //namespace

	namespace _p
	{

		file_base::~file_base()
		{
			if(m_handle)
			{
				os_close(m_handle);
			}
		}

		void file_base::close()
		{
			if(m_handle)
			{
				os_close(m_handle);
				m_handle = nullptr;
			}
		}

		int64_t file_base::pos() const
		{
			return os_tell(m_handle);
		}

		std::errc file_base::seek(int64_t p_pos)
		{
			return os_seek(m_handle, p_pos, SEEK_SET);
		}

		std::errc file_base::seek_current(int64_t p_pos)
		{
			return os_seek(m_handle, p_pos, SEEK_CUR);
		}

		std::errc file_base::seek_end(int64_t p_pos)
		{
			return os_seek(m_handle, p_pos, SEEK_END);
		}

		bool file_base::eof() const
		{
			return os_eof(m_handle);
		}

		bool file_base::error() const
		{
			return os_error(m_handle);
		}

		bool file_base::good() const
		{
			return !os_eof(m_handle) && !os_error(m_handle);
		}

		void file_base::clear_error()
		{
			os_clear_error(m_handle);
		}

		int64_t file_base::size() const
		{
			return os_file_lenght(m_handle);
		}

		void file_base::lock()
		{
			os_lock_file(m_handle);
		}

		void file_base::unlock()
		{
			os_unlock_file(m_handle);
		}

#ifdef _WIN32
		void file_base::close_unlocked()
		{
			if(m_handle)
			{
				os_close_unlocked(m_handle);
				m_handle = nullptr;
			}
		}

		int64_t file_base::pos_unlocked() const
		{
			return os_tell_unlocked(m_handle);
		}

		std::errc file_base::seek_unlocked(int64_t p_pos)
		{
			return os_seek_unlocked(m_handle, p_pos, SEEK_SET);
		}

		std::errc file_base::seek_current_unlocked(int64_t p_pos)
		{
			return os_seek_unlocked(m_handle, p_pos, SEEK_CUR);
		}

		std::errc file_base::seek_end_unlocked(int64_t p_pos)
		{
			return os_seek_unlocked(m_handle, p_pos, SEEK_END);
		}
#else
		bool file_base::try_lock()
		{
			return os_try_lock_file(m_handle);
		}
#endif
	} //namespace _p



	//
	std::errc file_read::open(const std::filesystem::path& p_path)
	{
		close();
		void* const handle = os_open_read(p_path);
		m_handle = handle;
		return std::errc{handle ? 0 : errno};
	}

	uintptr_t file_read::read(void* p_buff, uintptr_t p_size)
	{
		return os_read(m_handle, p_buff, p_size);
	}

	uintptr_t file_read::read_unlocked(void* p_buff, uintptr_t p_size)
	{
		return os_read_unlocked(m_handle, p_buff, p_size);
	}

#ifndef _WIN32
	uintptr_t file_read::read_offset(void* p_buff, uintptr_t p_size, int64_t p_offset)
	{
		return os_read_offset(m_handle, p_buff, p_size, p_offset);
	}
#endif


	std::errc file_write::open(const std::filesystem::path& p_path, open_mode p_mode)
	{
		close();
		void* const handle = os_open_write(p_path, p_mode);
		m_handle = handle;
		return std::errc{handle ? 0 : errno};
	}

	uintptr_t file_write::write(void* p_buff, uintptr_t p_size)
	{
		return os_write(m_handle, p_buff, p_size);
	}

	void file_write::flush()
	{
		os_flush(m_handle);
	}

	std::errc file_write::resize(int64_t p_size)
	{
		return os_file_resize(m_handle, p_size);
	}

	uintptr_t file_write::write_unlocked(void* p_buff, uintptr_t p_size)
	{
		return os_write_unlocked(m_handle, p_buff, p_size);
	}

	void file_write::flush_unlocked()
	{
		os_flush_unlocked(m_handle);
	}

#ifndef _WIN32
	uintptr_t file_write::write_offset(void* p_buff, uintptr_t p_size, int64_t p_offset)
	{
		return os_write_offset(m_handle, p_buff, p_size, p_offset);
	}
#endif

	std::errc file_duplex::open(const std::filesystem::path& p_path, open_mode p_mode)
	{
		close();
		void* const handle = os_open_duplex(p_path, p_mode);
		m_handle = handle;
		return std::errc{handle ? 0 : errno};
	}

	uintptr_t file_duplex::read(void* p_buff, uintptr_t p_size)
	{
		return os_read(m_handle, p_buff, p_size);
	}

	uintptr_t file_duplex::write(void* p_buff, uintptr_t p_size)
	{
		return os_write(m_handle, p_buff, p_size);
	}

	void file_duplex::flush()
	{
		os_flush(m_handle);
	}

	std::errc file_duplex::resize(int64_t p_size)
	{
		return os_file_resize(m_handle, p_size);
	}

	uintptr_t file_duplex::read_unlocked(void* p_buff, uintptr_t p_size)
	{
		return os_read_unlocked(m_handle, p_buff, p_size);
	}

	uintptr_t file_duplex::write_unlocked(void* p_buff, uintptr_t p_size)
	{
		return os_write_unlocked(m_handle, p_buff, p_size);
	}

	void file_duplex::flush_unlocked()
	{
		os_flush_unlocked(m_handle);
	}


#ifndef _WIN32
	uintptr_t file_duplex::read_offset(void* p_buff, uintptr_t p_size, int64_t p_offset)
	{
		return os_read_offset(m_handle, p_buff, p_size, p_offset);
	}

	uintptr_t file_duplex::write_offset(void* p_buff, uintptr_t p_size, int64_t p_offset)
	{
		return os_write_offset(m_handle, p_buff, p_size, p_offset);
	}
#endif
} //namespace core

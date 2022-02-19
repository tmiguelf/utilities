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
///
///	\todo	Provide a comprehensive and consistent set of error codes, to give
///			extra information regarding he nature of the failure
//======== ======== ======== ======== ======== ======== ======== ========

#pragma once

#include <type_traits>

#include "toPrint_sink.hpp"

#include <CoreLib/Core_extra_compiler.hpp>
#include <CoreLib/Core_File.hpp>
#include <CoreLib/Core_Alloca.hpp>
#include <CoreLib/Core_Endian.hpp>
#include <CoreLib/string/core_wchar_alias.hpp>
#include <CoreLib/string/core_string_encoding.hpp>

namespace core
{
	namespace _p::file_toPrint
	{
		constexpr uintptr_t alloca_treshold = 0x10000;
	}
	namespace _p
	{
		template<typename file_t> requires (std::is_same_v<file_t, file_write> || std::is_same_v<file_t, file_duplex>)
		class sink_file_locked
		{
		protected:
			inline sink_file_locked(file_t& p_file): m_file(p_file){}

			inline void push_out(const void* const p_data, uintptr_t const p_size) const
			{
				m_file.write(p_data, p_size);
			}

		private:
			file_t& m_file;
		};

		template<typename file_t> requires (std::is_same_v<file_t, file_write> || std::is_same_v<file_t, file_duplex>)
		class sink_file_unlocked
		{
		protected:
			inline sink_file_unlocked(file_t& p_file): m_file(p_file){}

			inline void push_out(const void* const p_data, uintptr_t const p_size) const
			{
				m_file.write_unlocked(p_data, p_size);
			}

		private:
			file_t& m_file;
		};
	} //namespace _p


	//======== ======== ======== ======== Locked ======== ======== ======== ========

	template<typename file_t> requires (std::is_same_v<file_t, file_write> || std::is_same_v<file_t, file_duplex>)
	class sink_file_UTF8: public sink_toPrint_base, private _p::sink_file_locked<file_t>
	{
	private:
		using sink_t = _p::sink_file_locked<file_t>;
	public:
		sink_file_UTF8(file_t& p_file): sink_t(p_file){}

		void write(std::u8string_view const p_out) const
		{
			sink_t::push_out(p_out.data(), p_out.size());
		}

		NO_INLINE void write(std::u16string_view const p_out) const
		{
			const uintptr_t count = core::_p::UTF16_to_UTF8_faulty_estimate(p_out, '?');
			if(count > _p::file_toPrint::alloca_treshold)
			{
				std::vector<char8_t> buff;
				buff.resize(count);
				core::_p::UTF16_to_UTF8_faulty_unsafe(p_out, '?', buff.data());
				sink_t::push_out(buff.data(), count);
			}
			else
			{
				char8_t* const buff = reinterpret_cast<char8_t*>(core_alloca(count));
				core::_p::UTF16_to_UTF8_faulty_unsafe(p_out, '?', buff);
				sink_t::push_out(buff, count);
			}
		}

		NO_INLINE void write(std::u32string_view p_out) const
		{
			const uintptr_t count = core::_p::UCS4_to_UTF8_faulty_estimate(p_out, '?');
			if(count > _p::file_toPrint::alloca_treshold)
			{
				std::vector<char8_t> buff;
				buff.resize(count);
				core::_p::UCS4_to_UTF8_faulty_unsafe(p_out, '?', buff.data());
				sink_t::push_out(buff.data(), count);
			}
			else
			{
				char8_t* const buff = reinterpret_cast<char8_t*>(core_alloca(count));
				core::_p::UCS4_to_UTF8_faulty_unsafe(p_out, '?', buff);
				sink_t::push_out(buff, count);
			}
		}
	};

	template<typename file_t> requires (std::is_same_v<file_t, file_write> || std::is_same_v<file_t, file_duplex>)
	class sink_file_UTF16BE: public sink_toPrint_base, private _p::sink_file_locked<file_t>
	{
	private:
		using sink_t = _p::sink_file_locked<file_t>;

		inline void commit(std::span<char16_t> p_out) const
		{
			if constexpr(std::endian::native == std::endian::little)
			{
				for(char16_t& tchar : p_out)
				{
					tchar = byte_swap(tchar);
				}
			}
			sink_t::push_out(p_out.data(), p_out.size() * sizeof(char16_t));
		}

	public:
		sink_file_UTF16BE(file_t& p_file): sink_t(p_file){}

		NO_INLINE void write(std::u8string_view const p_out) const
		{
			const uintptr_t count = core::_p::UTF8_to_UTF16_faulty_estimate(p_out, '?');

			if(count > _p::file_toPrint::alloca_treshold / sizeof(char16_t))
			{
				std::vector<char16_t> buff;
				buff.resize(count);
				core::_p::UTF8_to_UTF16_faulty_unsafe(p_out, '?', buff.data());
				commit({buff.data(), count});
			}
			else
			{
				char16_t* const buff = reinterpret_cast<char16_t*>(core_alloca(count * sizeof(char16_t)));
				core::_p::UTF8_to_UTF16_faulty_unsafe(p_out, '?', buff);
				commit({buff, count});
			}
		}

		NO_INLINE void write(std::u16string_view const p_out) const
		{
			if constexpr(std::endian::native == std::endian::little)
			{
				const uintptr_t count = p_out.size();
				if(count > _p::file_toPrint::alloca_treshold / sizeof(char16_t))
				{
					std::vector<char16_t> buff;
					buff.resize(count);
					memcpy(buff.data(), p_out.data(), count * sizeof(char16_t));
					commit({buff.data(), count});
				}
				else
				{
					char16_t* const buff = reinterpret_cast<char16_t*>(core_alloca(count * sizeof(char16_t)));
					memcpy(buff, p_out.data(), count * sizeof(char16_t));
					commit({buff, count});
				}
			}
			else
			{
				sink_t::push_out(p_out.data(), p_out.size() * sizeof(char16_t));
			}
		}

		NO_INLINE void write(std::u32string_view const p_out) const
		{
			const uintptr_t count = core::_p::UCS4_to_UTF16_faulty_estimate(p_out, '?');

			if(count > _p::file_toPrint::alloca_treshold / sizeof(char16_t))
			{
				std::vector<char16_t> buff;
				buff.resize(count);
				core::_p::UCS4_to_UTF16_faulty_unsafe(p_out, '?', buff.data());
				commit({buff.data(), count});
			}
			else
			{
				char16_t* const buff = reinterpret_cast<char16_t*>(core_alloca(count * sizeof(char16_t)));
				core::_p::UCS4_to_UTF16_faulty_unsafe(p_out, '?', buff);
				commit({buff, count});
			}
		}
	};

	template<typename file_t> requires (std::is_same_v<file_t, file_write> || std::is_same_v<file_t, file_duplex>)
	class sink_file_UTF16LE: public sink_toPrint_base, private _p::sink_file_locked<file_t>
	{
	private:
		using sink_t = _p::sink_file_locked<file_t>;

		inline void commit(std::span<char16_t> p_out) const
		{
			if constexpr(std::endian::native == std::endian::big)
			{
				for(char16_t& tchar : p_out)
				{
					tchar = byte_swap(tchar);
				}
			}
			sink_t::push_out(p_out.data(), p_out.size() * sizeof(char16_t));
		}

	public:
		sink_file_UTF16LE(file_t& p_file): sink_t(p_file){}

		NO_INLINE void write(std::u8string_view  p_out) const
		{
			const uintptr_t count = core::_p::UTF8_to_UTF16_faulty_estimate(p_out, '?');

			if(count > _p::file_toPrint::alloca_treshold / sizeof(char16_t))
			{
				std::vector<char16_t> buff;
				buff.resize(count);
				core::_p::UTF8_to_UTF16_faulty_unsafe(p_out, '?', buff.data());
				commit({buff.data(), count});
			}
			else
			{
				char16_t* const buff = reinterpret_cast<char16_t*>(core_alloca(count * sizeof(char16_t)));
				core::_p::UTF8_to_UTF16_faulty_unsafe(p_out, '?', buff);
				commit({buff, count});
			}
		}

		NO_INLINE void write(std::u16string_view p_out) const
		{
			if constexpr(std::endian::native == std::endian::big)
			{
				const uintptr_t count = p_out.size();
				if(count > _p::file_toPrint::alloca_treshold / sizeof(char16_t))
				{
					std::vector<char16_t> buff;
					buff.resize(count);
					memcpy(buff.data(), p_out.data(), count * sizeof(char16_t));
					commit({buff.data(), count});
				}
				else
				{
					char16_t* const buff = reinterpret_cast<char16_t*>(core_alloca(count * sizeof(char16_t)));
					memcpy(buff, p_out.data(), count * sizeof(char16_t));
					commit({buff, count});
				}
			}
			else
			{
				sink_t::push_out(p_out.data(), p_out.size() * sizeof(char16_t));
			}
		}

		NO_INLINE void write(std::u32string_view p_out) const
		{
			const uintptr_t count = core::_p::UCS4_to_UTF16_faulty_estimate(p_out, '?');

			if(count > _p::file_toPrint::alloca_treshold / sizeof(char16_t))
			{
				std::vector<char16_t> buff;
				buff.resize(count);
				core::_p::UCS4_to_UTF16_faulty_unsafe(p_out, '?', buff.data());
				commit({buff.data(), count});
			}
			else
			{
				char16_t* const buff = reinterpret_cast<char16_t*>(core_alloca(count * sizeof(char16_t)));
				core::_p::UCS4_to_UTF16_faulty_unsafe(p_out, '?', buff);
				commit({buff, count});
			}
		}
	};

	template<typename file_t> requires (std::is_same_v<file_t, file_write> || std::is_same_v<file_t, file_duplex>)
	class sink_file_UCS4BE: public sink_toPrint_base, private _p::sink_file_locked<file_t>
	{
	private:
		using sink_t = _p::sink_file_locked<file_t>;

		inline void commit(std::span<char32_t> p_out) const
		{
			if constexpr(std::endian::native == std::endian::little)
			{
				for(char32_t& tchar : p_out)
				{
					tchar = byte_swap(tchar);
				}
			}
			sink_t::push_out(p_out.data(), p_out.size() * sizeof(char32_t));
		}

	public:
		sink_file_UCS4BE(file_t& p_file): sink_t(p_file){}

		NO_INLINE void write(std::u8string_view  p_out) const
		{
			const uintptr_t count = core::_p::UTF8_to_UCS4_faulty_estimate(p_out);

			if(count > _p::file_toPrint::alloca_treshold / sizeof(char32_t))
			{
				std::vector<char32_t> buff;
				buff.resize(count);
				core::_p::UTF8_to_UCS4_faulty_unsafe(p_out, '?', buff.data());
				commit({buff.data(), count});
			}
			else
			{
				char32_t* const buff = reinterpret_cast<char32_t*>(core_alloca(count * sizeof(char32_t)));
				core::_p::UTF8_to_UCS4_faulty_unsafe(p_out, '?', buff);
				commit({buff, count});
			}
		}

		NO_INLINE void write(std::u16string_view p_out) const
		{
			const uintptr_t count = core::_p::UTF16_to_UCS4_faulty_estimate(p_out);

			if(count > _p::file_toPrint::alloca_treshold / sizeof(char32_t))
			{
				std::vector<char32_t> buff;
				buff.resize(count);
				core::_p::UTF16_to_UCS4_faulty_unsafe(p_out, '?', buff.data());
				commit({buff.data(), count});
			}
			else
			{
				char32_t* const buff = reinterpret_cast<char32_t*>(core_alloca(count * sizeof(char32_t)));
				core::_p::UTF16_to_UCS4_faulty_unsafe(p_out, '?', buff);
				commit({buff, count});
			}
		}

		NO_INLINE void write(std::u32string_view p_out) const
		{
			if constexpr(std::endian::native == std::endian::little)
			{
				const uintptr_t count = p_out.size();
				if(count > _p::file_toPrint::alloca_treshold / sizeof(char32_t))
				{
					std::vector<char32_t> buff;
					buff.resize(count);
					memcpy(buff.data(), p_out.data(), count * sizeof(char32_t));
					commit({buff.data(), count});
				}
				else
				{
					char32_t* const buff = reinterpret_cast<char16_t*>(core_alloca(count * sizeof(char32_t)));
					memcpy(buff, p_out.data(), count * sizeof(char32_t));
					commit({buff, count});
				}
			}
			else
			{
				sink_t::push_out(p_out.data(), p_out.size() * sizeof(char32_t));
			}
		}
	};

	template<typename file_t> requires (std::is_same_v<file_t, file_write> || std::is_same_v<file_t, file_duplex>)
	class sink_file_UCS4LE: public sink_toPrint_base, private _p::sink_file_locked<file_t>
	{
	private:
		using sink_t = _p::sink_file_locked<file_t>;

		inline void commit(std::span<char32_t> p_out) const
		{
			if constexpr(std::endian::native == std::endian::big)
			{
				for(char32_t& tchar : p_out)
				{
					tchar = byte_swap(tchar);
				}
			}
			sink_t::push_out(p_out.data(), p_out.size() * sizeof(char32_t));
		}

	public:
		sink_file_UCS4LE(file_t& p_file): sink_t(p_file){}

		NO_INLINE void write(std::u8string_view  p_out) const
		{
			const uintptr_t count = core::_p::UTF8_to_UCS4_faulty_estimate(p_out);

			if(count > _p::file_toPrint::alloca_treshold / sizeof(char32_t))
			{
				std::vector<char32_t> buff;
				buff.resize(count);
				core::_p::UTF8_to_UCS4_faulty_unsafe(p_out, '?', buff.data());
				commit({buff.data(), count});
			}
			else
			{
				char32_t* const buff = reinterpret_cast<char32_t*>(core_alloca(count * sizeof(char32_t)));
				core::_p::UTF8_to_UCS4_faulty_unsafe(p_out, '?', buff);
				commit({buff, count});
			}
		}

		NO_INLINE void write(std::u16string_view p_out) const
		{
			const uintptr_t count = core::_p::UTF16_to_UCS4_faulty_estimate(p_out);

			if(count > _p::file_toPrint::alloca_treshold / sizeof(char32_t))
			{
				std::vector<char32_t> buff;
				buff.resize(count);
				core::_p::UTF16_to_UCS4_faulty_unsafe(p_out, '?', buff.data());
				commit({buff.data(), count});
			}
			else
			{
				char32_t* const buff = reinterpret_cast<char32_t*>(core_alloca(count * sizeof(char32_t)));
				core::_p::UTF16_to_UCS4_faulty_unsafe(p_out, '?', buff);
				commit({buff, count});
			}
		}

		NO_INLINE void write(std::u32string_view p_out) const
		{
			if constexpr(std::endian::native == std::endian::big)
			{
				const uintptr_t count = p_out.size();
				if(count > _p::file_toPrint::alloca_treshold / sizeof(char32_t))
				{
					std::vector<char32_t> buff;
					buff.resize(count);
					memcpy(buff.data(), p_out.data(), count * sizeof(char32_t));
					commit({buff.data(), count});
				}
				else
				{
					char32_t* const buff = reinterpret_cast<char16_t*>(core_alloca(count * sizeof(char32_t)));
					memcpy(buff, p_out.data(), count * sizeof(char32_t));
					commit({buff, count});
				}
			}
			else
			{
				sink_t::push_out(p_out.data(), p_out.size() * sizeof(char32_t));
			}
		}
	};


	//======== ======== ======== ======== Unlocked ======== ======== ======== ========

	template<typename file_t> requires (std::is_same_v<file_t, file_write> || std::is_same_v<file_t, file_duplex>)
	class sink_file_UTF8_unlocked: public sink_toPrint_base, private _p::sink_file_unlocked<file_t>
	{
	private:
		using sink_t = _p::sink_file_unlocked<file_t>;

	public:
		sink_file_UTF8_unlocked(file_t& p_file): sink_t(p_file){}

		void write(std::u8string_view  p_out) const
		{
			sink_t::push_out(p_out.data(), p_out.size());
		}

		NO_INLINE void write(std::u16string_view p_out) const
		{
			const uintptr_t count = core::_p::UTF16_to_UTF8_faulty_estimate(p_out, '?');
			if(count > _p::file_toPrint::alloca_treshold)
			{
				std::vector<char8_t> buff;
				buff.resize(count);
				core::_p::UTF16_to_UTF8_faulty_unsafe(p_out, '?', buff.data());
				sink_t::push_out(buff.data(), count);
			}
			else
			{
				char8_t* const buff = reinterpret_cast<char8_t*>(core_alloca(count));
				core::_p::UTF16_to_UTF8_faulty_unsafe(p_out, '?', buff);
				sink_t::push_out(buff, count);
			}
		}

		NO_INLINE void write(std::u32string_view p_out) const
		{
			const uintptr_t count = core::_p::UCS4_to_UTF8_faulty_estimate(p_out, '?');
			if(count > _p::file_toPrint::alloca_treshold)
			{
				std::vector<char8_t> buff;
				buff.resize(count);
				core::_p::UCS4_to_UTF8_faulty_unsafe(p_out, '?', buff.data());
				sink_t::push_out(buff.data(), count);
			}
			else
			{
				char8_t* const buff = reinterpret_cast<char8_t*>(core_alloca(count));
				core::_p::UCS4_to_UTF8_faulty_unsafe(p_out, '?', buff);
				sink_t::push_out(buff, count);
			}
		}
	};

	template<typename file_t> requires (std::is_same_v<file_t, file_write> || std::is_same_v<file_t, file_duplex>)
	class sink_file_UTF16BE_unlocked: public sink_toPrint_base, private _p::sink_file_unlocked<file_t>
	{
	private:
		using sink_t = _p::sink_file_unlocked<file_t>;

		inline void commit(std::span<char16_t> p_out) const
		{
			if constexpr(std::endian::native == std::endian::little)
			{
				for(char16_t& tchar : p_out)
				{
					tchar = byte_swap(tchar);
				}
			}
			sink_t::push_out(p_out.data(), p_out.size() * sizeof(char16_t));
		}

	public:
		sink_file_UTF16BE_unlocked(file_t& p_file): sink_t(p_file){}

		NO_INLINE void write(std::u8string_view  p_out) const
		{
			const uintptr_t count = core::_p::UTF8_to_UTF16_faulty_estimate(p_out, '?');

			if(count > _p::file_toPrint::alloca_treshold / sizeof(char16_t))
			{
				std::vector<char16_t> buff;
				buff.resize(count);
				core::_p::UTF8_to_UTF16_faulty_unsafe(p_out, '?', buff.data());
				commit({buff.data(), count});
			}
			else
			{
				char16_t* const buff = reinterpret_cast<char16_t*>(core_alloca(count * sizeof(char16_t)));
				core::_p::UTF8_to_UTF16_faulty_unsafe(p_out, '?', buff);
				commit({buff, count});
			}
		}

		NO_INLINE void write(std::u16string_view p_out) const
		{
			if constexpr(std::endian::native == std::endian::little)
			{
				const uintptr_t count = p_out.size();
				if(count > _p::file_toPrint::alloca_treshold / sizeof(char16_t))
				{
					std::vector<char16_t> buff;
					buff.resize(count);
					memcpy(buff.data(), p_out.data(), count * sizeof(char16_t));
					commit({buff.data(), count});
				}
				else
				{
					char16_t* const buff = reinterpret_cast<char16_t*>(core_alloca(count * sizeof(char16_t)));
					memcpy(buff, p_out.data(), count * sizeof(char16_t));
					commit({buff, count});
				}
			}
			else
			{
				sink_t::push_out(p_out.data(), p_out.size() * sizeof(char16_t));
			}
		}

		NO_INLINE void write(std::u32string_view p_out) const
		{
			const uintptr_t count = core::_p::UCS4_to_UTF16_faulty_estimate(p_out, '?');

			if(count > _p::file_toPrint::alloca_treshold / sizeof(char16_t))
			{
				std::vector<char16_t> buff;
				buff.resize(count);
				core::_p::UCS4_to_UTF16_faulty_unsafe(p_out, '?', buff.data());
				commit({buff.data(), count});
			}
			else
			{
				char16_t* const buff = reinterpret_cast<char16_t*>(core_alloca(count * sizeof(char16_t)));
				core::_p::UCS4_to_UTF16_faulty_unsafe(p_out, '?', buff);
				commit({buff, count});
			}
		}
	};

	template<typename file_t> requires (std::is_same_v<file_t, file_write> || std::is_same_v<file_t, file_duplex>)
	class sink_file_UTF16LE_unlocked: public sink_toPrint_base, private _p::sink_file_unlocked<file_t>
	{
	private:
		using sink_t = _p::sink_file_unlocked<file_t>;

		inline void commit(std::span<char16_t> p_out) const
		{
			if constexpr(std::endian::native == std::endian::big)
			{
				for(char16_t& tchar : p_out)
				{
					tchar = byte_swap(tchar);
				}
			}
			sink_t::push_out(p_out.data(), p_out.size() * sizeof(char16_t));
		}

	public:
		sink_file_UTF16LE_unlocked(file_t& p_file): sink_t(p_file){}

		NO_INLINE void write(std::u8string_view  p_out) const
		{
			const uintptr_t count = core::_p::UTF8_to_UTF16_faulty_estimate(p_out, '?');

			if(count > _p::file_toPrint::alloca_treshold / sizeof(char16_t))
			{
				std::vector<char16_t> buff;
				buff.resize(count);
				core::_p::UTF8_to_UTF16_faulty_unsafe(p_out, '?', buff.data());
				commit({buff.data(), count});
			}
			else
			{
				char16_t* const buff = reinterpret_cast<char16_t*>(core_alloca(count * sizeof(char16_t)));
				core::_p::UTF8_to_UTF16_faulty_unsafe(p_out, '?', buff);
				commit({buff, count});
			}
		}

		NO_INLINE void write(std::u16string_view p_out) const
		{
			if constexpr(std::endian::native == std::endian::big)
			{
				const uintptr_t count = p_out.size();
				if(count > _p::file_toPrint::alloca_treshold / sizeof(char16_t))
				{
					std::vector<char16_t> buff;
					buff.resize(count);
					memcpy(buff.data(), p_out.data(), count * sizeof(char16_t));
					commit({buff.data(), count});
				}
				else
				{
					char16_t* const buff = reinterpret_cast<char16_t*>(core_alloca(count * sizeof(char16_t)));
					memcpy(buff, p_out.data(), count * sizeof(char16_t));
					commit({buff, count});
				}
			}
			else
			{
				sink_t::push_out(p_out.data(), p_out.size() * sizeof(char16_t));
			}
		}

		NO_INLINE void write(std::u32string_view p_out) const
		{
			const uintptr_t count = core::_p::UCS4_to_UTF16_faulty_estimate(p_out, '?');

			if(count > _p::file_toPrint::alloca_treshold / sizeof(char16_t))
			{
				std::vector<char16_t> buff;
				buff.resize(count);
				core::_p::UCS4_to_UTF16_faulty_unsafe(p_out, '?', buff.data());
				commit({buff.data(), count});
			}
			else
			{
				char16_t* const buff = reinterpret_cast<char16_t*>(core_alloca(count * sizeof(char16_t)));
				core::_p::UCS4_to_UTF16_faulty_unsafe(p_out, '?', buff);
				commit({buff, count});
			}
		}
	};

	template<typename file_t> requires (std::is_same_v<file_t, file_write> || std::is_same_v<file_t, file_duplex>)
	class sink_file_UCS4BE_unlocked: public sink_toPrint_base, private _p::sink_file_unlocked<file_t>
	{
	private:
		using sink_t = _p::sink_file_unlocked<file_t>;

		inline void commit(std::span<char32_t> p_out) const
		{
			if constexpr(std::endian::native == std::endian::little)
			{
				for(char32_t& tchar : p_out)
				{
					tchar = byte_swap(tchar);
				}
			}
			sink_t::push_out(p_out.data(), p_out.size() * sizeof(char32_t));
		}

	public:
		sink_file_UCS4BE_unlocked(file_t& p_file): sink_t(p_file){}

		NO_INLINE void write(std::u8string_view  p_out) const
		{
			const uintptr_t count = core::_p::UTF8_to_UCS4_faulty_estimate(p_out);

			if(count > _p::file_toPrint::alloca_treshold / sizeof(char32_t))
			{
				std::vector<char32_t> buff;
				buff.resize(count);
				core::_p::UTF8_to_UCS4_faulty_unsafe(p_out, '?', buff.data());
				commit({buff.data(), count});
			}
			else
			{
				char32_t* const buff = reinterpret_cast<char32_t*>(core_alloca(count * sizeof(char32_t)));
				core::_p::UTF8_to_UCS4_faulty_unsafe(p_out, '?', buff);
				commit({buff, count});
			}
		}

		NO_INLINE void write(std::u16string_view p_out) const
		{
			const uintptr_t count = core::_p::UTF16_to_UCS4_faulty_estimate(p_out);

			if(count > _p::file_toPrint::alloca_treshold / sizeof(char32_t))
			{
				std::vector<char32_t> buff;
				buff.resize(count);
				core::_p::UTF16_to_UCS4_faulty_unsafe(p_out, '?', buff.data());
				commit({buff.data(), count});
			}
			else
			{
				char32_t* const buff = reinterpret_cast<char32_t*>(core_alloca(count * sizeof(char32_t)));
				core::_p::UTF16_to_UCS4_faulty_unsafe(p_out, '?', buff);
				commit({buff, count});
			}
		}

		NO_INLINE void write(std::u32string_view p_out) const
		{
			if constexpr(std::endian::native == std::endian::little)
			{
				const uintptr_t count = p_out.size();
				if(count > _p::file_toPrint::alloca_treshold / sizeof(char32_t))
				{
					std::vector<char32_t> buff;
					buff.resize(count);
					memcpy(buff.data(), p_out.data(), count * sizeof(char32_t));
					commit({buff.data(), count});
				}
				else
				{
					char32_t* const buff = reinterpret_cast<char16_t*>(core_alloca(count * sizeof(char32_t)));
					memcpy(buff, p_out.data(), count * sizeof(char32_t));
					commit({buff, count});
				}
			}
			else
			{
				sink_t::push_out(p_out.data(), p_out.size() * sizeof(char32_t));
			}
		}
	};

	template<typename file_t> requires (std::is_same_v<file_t, file_write> || std::is_same_v<file_t, file_duplex>)
	class sink_file_UCS4LE_unlocked: public sink_toPrint_base, private _p::sink_file_unlocked<file_t>
	{
	private:
		using sink_t = _p::sink_file_unlocked<file_t>;

		inline void commit(std::span<char32_t> p_out) const
		{
			if constexpr(std::endian::native == std::endian::big)
			{
				for(char32_t& tchar : p_out)
				{
					tchar = byte_swap(tchar);
				}
			}
			sink_t::push_out(p_out.data(), p_out.size() * sizeof(char32_t));
		}

	public:
		sink_file_UCS4LE_unlocked(file_t& p_file): sink_t(p_file){}

		NO_INLINE void write(std::u8string_view  p_out) const
		{
			const uintptr_t count = core::_p::UTF8_to_UCS4_faulty_estimate(p_out);

			if(count > _p::file_toPrint::alloca_treshold / sizeof(char32_t))
			{
				std::vector<char32_t> buff;
				buff.resize(count);
				core::_p::UTF8_to_UCS4_faulty_unsafe(p_out, '?', buff.data());
				commit({buff.data(), count});
			}
			else
			{
				char32_t* const buff = reinterpret_cast<char32_t*>(core_alloca(count * sizeof(char32_t)));
				core::_p::UTF8_to_UCS4_faulty_unsafe(p_out, '?', buff);
				commit({buff, count});
			}
		}

		NO_INLINE void write(std::u16string_view p_out) const
		{
			const uintptr_t count = core::_p::UTF16_to_UCS4_faulty_estimate(p_out);

			if(count > _p::file_toPrint::alloca_treshold / sizeof(char32_t))
			{
				std::vector<char32_t> buff;
				buff.resize(count);
				core::_p::UTF16_to_UCS4_faulty_unsafe(p_out, '?', buff.data());
				commit({buff.data(), count});
			}
			else
			{
				char32_t* const buff = reinterpret_cast<char32_t*>(core_alloca(count * sizeof(char32_t)));
				core::_p::UTF16_to_UCS4_faulty_unsafe(p_out, '?', buff);
				commit({buff, count});
			}
		}

		NO_INLINE void write(std::u32string_view p_out) const
		{
			if constexpr(std::endian::native == std::endian::big)
			{
				const uintptr_t count = p_out.size();
				if(count > _p::file_toPrint::alloca_treshold / sizeof(char32_t))
				{
					std::vector<char32_t> buff;
					buff.resize(count);
					memcpy(buff.data(), p_out.data(), count * sizeof(char32_t));
					commit({buff.data(), count});
				}
				else
				{
					char32_t* const buff = reinterpret_cast<char16_t*>(core_alloca(count * sizeof(char32_t)));
					memcpy(buff, p_out.data(), count * sizeof(char32_t));
					commit({buff, count});
				}
			}
			else
			{
				sink_t::push_out(p_out.data(), p_out.size() * sizeof(char32_t));
			}
		}
	};


	//======== ======== ======== ======== CTAD ======== ======== ======== ========

	template<typename T> sink_file_UTF8   (T&) -> sink_file_UTF8   <std::remove_cvref_t<T>>;
	template<typename T> sink_file_UTF16BE(T&) -> sink_file_UTF16BE<std::remove_cvref_t<T>>;
	template<typename T> sink_file_UTF16LE(T&) -> sink_file_UTF16LE<std::remove_cvref_t<T>>;
	template<typename T> sink_file_UCS4BE (T&) -> sink_file_UCS4BE <std::remove_cvref_t<T>>;
	template<typename T> sink_file_UCS4LE (T&) -> sink_file_UCS4LE <std::remove_cvref_t<T>>;

	template<typename T> sink_file_UTF8_unlocked   (T&) -> sink_file_UTF8_unlocked   <std::remove_cvref_t<T>>;
	template<typename T> sink_file_UTF16BE_unlocked(T&) -> sink_file_UTF16BE_unlocked<std::remove_cvref_t<T>>;
	template<typename T> sink_file_UTF16LE_unlocked(T&) -> sink_file_UTF16LE_unlocked<std::remove_cvref_t<T>>;
	template<typename T> sink_file_UCS4BE_unlocked (T&) -> sink_file_UCS4BE_unlocked <std::remove_cvref_t<T>>;
	template<typename T> sink_file_UCS4LE_unlocked (T&) -> sink_file_UCS4LE_unlocked <std::remove_cvref_t<T>>;
}


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

#include <gtest/gtest.h>

#include <CoreLib/core_file.hpp>

#include <filesystem>
#include <fstream>
#include <string_view>
#include <string>

namespace
{
	class AssistFileCleanup
	{
	public:
		AssistFileCleanup(const std::filesystem::path& p_path): m_path(p_path) {}
		~AssistFileCleanup()
		{
			std::error_code ec;
			std::filesystem::remove(m_path, ec);
		}
		std::filesystem::path m_path;
	};


	void assist_delete_file(const std::filesystem::path& p_path)
	{
		std::error_code ec;
		if(std::filesystem::exists(p_path, ec))
		{
			if(!std::filesystem::remove(p_path, ec) && ec != std::error_code{})
			{
				ASSERT_TRUE(false) << "test setup failed";
			}
		}
	}

	void assist_make_file(const std::filesystem::path& p_path, std::string_view p_content)
	{
		assist_delete_file(p_path);
		std::ofstream aux_file;
		aux_file.open(p_path, std::ios_base::out | std::ios_base::binary);
		if(!aux_file.is_open())
		{
			ASSERT_TRUE(false) << "test setup failed";
		}

		aux_file.write(p_content.data(), p_content.size());
		if(!aux_file.good())
		{
			ASSERT_TRUE(false) << "test setup failed";
		}
	}
}


namespace text_formating
{
	TEST(core_file, safe_op)
	{
		core::file_read file;
		EXPECT_TRUE(file.eof());
		EXPECT_TRUE(file.error());
		EXPECT_FALSE(file.good());
		ASSERT_FALSE(file.is_open());
		file.close();
		ASSERT_FALSE(file.is_open());
	}

	void assist_read_test(core::file_read& file, uintptr_t (core::file_read::* read_method)(void*, uintptr_t),
		std::string_view test_content, std::string_view p_method_name)
	{
		const uintptr_t expected_read_count = test_content.size();
		std::string aux;
		aux.resize(expected_read_count);
		memset(aux.data(), 0, expected_read_count);

		ASSERT_EQ((file.*read_method)(aux.data(), expected_read_count), expected_read_count) << p_method_name;
		ASSERT_EQ(std::string_view{aux}, test_content) << p_method_name;

		ASSERT_TRUE(file.is_open()) << p_method_name;
		ASSERT_EQ(file.size(), static_cast<int64_t>(expected_read_count)) << p_method_name;
		ASSERT_EQ(file.pos(), static_cast<int64_t>(expected_read_count)) << p_method_name;
		ASSERT_FALSE(file.eof()) << p_method_name;
		ASSERT_FALSE(file.error()) << p_method_name;
		ASSERT_TRUE(file.good()) << p_method_name;

		//---- Read past end ----
		ASSERT_EQ((file.*read_method)(aux.data(), 1), uintptr_t{0}) << p_method_name;
		ASSERT_EQ(file.size(), static_cast<int64_t>(expected_read_count)) << p_method_name;
		ASSERT_EQ(file.pos(), static_cast<int64_t>(expected_read_count)) << p_method_name;
		ASSERT_TRUE(file.eof()) << p_method_name;
		ASSERT_FALSE(file.error()) << p_method_name;
		ASSERT_FALSE(file.good()) << p_method_name;

		//---- reposition ----
		const int64_t read_offset = 2;
		ASSERT_EQ(file.seek(read_offset), std::errc{}) << p_method_name;
		ASSERT_EQ(file.size(), static_cast<int64_t>(expected_read_count)) << p_method_name;
		ASSERT_EQ(file.pos(), read_offset) << p_method_name;
		ASSERT_FALSE(file.eof()) << p_method_name;
		ASSERT_FALSE(file.error()) << p_method_name;
		ASSERT_TRUE(file.good()) << p_method_name;

		//---- re-read ----
		const uintptr_t new_read_count = expected_read_count - read_offset;
		aux.resize(new_read_count);
		ASSERT_EQ((file.*read_method)(aux.data(), new_read_count), static_cast<uintptr_t>(new_read_count)) << p_method_name;
		ASSERT_EQ(std::string_view{aux}, test_content.substr(read_offset)) << p_method_name;
		ASSERT_EQ(file.pos(), static_cast<int64_t>(expected_read_count)) << p_method_name;
		ASSERT_FALSE(file.eof()) << p_method_name;
		ASSERT_FALSE(file.error()) << p_method_name;
		ASSERT_TRUE(file.good()) << p_method_name;
	}

	TEST(core_file, open_read)
	{
		const std::filesystem::path fileName   = "open_R_e_test.txt";
		const std::filesystem::path fileName_n = "open_R_n_test.txt";

		constexpr std::string_view test_content = "The quick brown fox jumps over the lazy dog";
		constexpr uintptr_t expected_read_count = test_content.size();

		AssistFileCleanup auto_cleanup  {fileName};
		AssistFileCleanup auto_cleanup_n{fileName_n};
		//setup
		assist_make_file(fileName, test_content);
		assist_delete_file(fileName_n);

		//Test
		//---- Initial state ----
		{
			core::file_read file;
			ASSERT_TRUE(file.open(fileName_n) != std::errc{});
			ASSERT_FALSE(file.is_open());
			ASSERT_TRUE(file.eof());
			ASSERT_TRUE(file.error());
			ASSERT_FALSE(file.good());
		}

		core::file_read file;
		ASSERT_EQ(file.open(fileName), std::errc{});
		ASSERT_TRUE(file.is_open());
		ASSERT_EQ(file.size(), static_cast<int64_t>(expected_read_count));
		ASSERT_EQ(file.pos(), int64_t{0});
		ASSERT_FALSE(file.eof());
		ASSERT_FALSE(file.error());
		ASSERT_TRUE(file.good());
	
		assist_read_test(file, &core::file_read::read, test_content, "read");
		file.seek(0);
		assist_read_test(file, &core::file_read::read_unlocked, test_content, "read_unlocked");
		//---- Read ----

		file.close();
		ASSERT_FALSE(file.is_open());
	}

	TEST(core_file, open_modes_write)
	{
		const std::filesystem::path fileName_create_e			= "open_W_create_e_test.txt";
		const std::filesystem::path fileName_create_n			= "open_W_create_n_test.txt";
		const std::filesystem::path fileName_create_if_new_e	= "open_W_create_if_new_e_test.txt";
		const std::filesystem::path fileName_create_if_new_n	= "open_W_create_if_new_n_test.txt";
		const std::filesystem::path fileName_open_or_create_e	= "open_W_open_or_create_e_test.txt";
		const std::filesystem::path fileName_open_or_create_n	= "open_W_open_or_create_n_test.txt";
		const std::filesystem::path fileName_open_existing_e	= "open_W_open_existing_e_test.txt";
		const std::filesystem::path fileName_open_existing_n	= "open_W_open_existing_n_test.txt";
		
		constexpr std::string_view test_content = "The quick brown fox jumps over the lazy dog";
		constexpr uintptr_t expected_read_count = test_content.size();

		AssistFileCleanup auto_cleanup_create_e			{fileName_create_e};
		AssistFileCleanup auto_cleanup_create_n			{fileName_create_n};
		AssistFileCleanup auto_cleanup_create_if_new_e	{fileName_create_if_new_e};
		AssistFileCleanup auto_cleanup_create_if_new_n	{fileName_create_if_new_n};
		AssistFileCleanup auto_cleanup_open_or_create_e	{fileName_open_or_create_e};
		AssistFileCleanup auto_cleanup_open_or_create_n	{fileName_open_or_create_n};
		AssistFileCleanup auto_cleanup_open_existing_e	{fileName_open_existing_e};
		AssistFileCleanup auto_cleanup_open_existing_n	{fileName_open_existing_n};

		//setup
		assist_make_file(fileName_create_e, test_content);
		assist_make_file(fileName_create_if_new_e, test_content);
		assist_make_file(fileName_open_or_create_e, test_content);
		assist_make_file(fileName_open_existing_e, test_content);
		assist_delete_file(fileName_create_n);
		assist_delete_file(fileName_create_if_new_n);
		assist_delete_file(fileName_open_or_create_n);
		assist_delete_file(fileName_open_existing_n);
		//Test
		//---- Initial state ----
		core::file_write file_create_e			;
		core::file_write file_create_n			;
		core::file_write file_create_if_new_e	;
		core::file_write file_create_if_new_n	;
		core::file_write file_open_or_create_e	;
		core::file_write file_open_or_create_n	;
		core::file_write file_open_existing_e	;
		core::file_write file_open_existing_n	;

		ASSERT_TRUE(file_create_e			.open(fileName_create_e			, core::file_write::open_mode::create) == std::errc{});
		ASSERT_TRUE(file_create_n			.open(fileName_create_n			, core::file_write::open_mode::create) == std::errc{});
		ASSERT_TRUE(file_create_if_new_e	.open(fileName_create_if_new_e	, core::file_write::open_mode::crete_if_new) != std::errc{});
		ASSERT_TRUE(file_create_if_new_n	.open(fileName_create_if_new_n	, core::file_write::open_mode::crete_if_new) == std::errc{});
		ASSERT_TRUE(file_open_or_create_e	.open(fileName_open_or_create_e	, core::file_write::open_mode::open_or_create) == std::errc{});
		ASSERT_TRUE(file_open_or_create_n	.open(fileName_open_or_create_n	, core::file_write::open_mode::open_or_create) == std::errc{});
		ASSERT_TRUE(file_open_existing_e	.open(fileName_open_existing_e	, core::file_write::open_mode::open_existing) == std::errc{});
		ASSERT_TRUE(file_open_existing_n	.open(fileName_open_existing_n	, core::file_write::open_mode::open_existing) != std::errc{});

		ASSERT_TRUE (file_create_e			.is_open());
		ASSERT_TRUE (file_create_n			.is_open());
		ASSERT_FALSE(file_create_if_new_e	.is_open());
		ASSERT_TRUE (file_create_if_new_n	.is_open());
		ASSERT_TRUE (file_open_or_create_e	.is_open());
		ASSERT_TRUE (file_open_or_create_n	.is_open());
		ASSERT_TRUE (file_open_existing_e	.is_open());
		ASSERT_FALSE(file_open_existing_n	.is_open());

		ASSERT_EQ(file_create_e			.size(), int64_t{0});
		ASSERT_EQ(file_create_n			.size(), int64_t{0});
		ASSERT_EQ(file_create_if_new_n	.size(), int64_t{0});
		ASSERT_EQ(file_open_or_create_e	.size(), static_cast<int64_t>(expected_read_count));
		ASSERT_EQ(file_open_or_create_n	.size(), int64_t{0});
		ASSERT_EQ(file_open_existing_e	.size(), static_cast<int64_t>(expected_read_count));

		ASSERT_EQ(file_create_e			.pos(), int64_t{0});
		ASSERT_EQ(file_create_n			.pos(), int64_t{0});
		ASSERT_EQ(file_create_if_new_n	.pos(), int64_t{0});
		ASSERT_EQ(file_open_or_create_e	.pos(), int64_t{0});
		ASSERT_EQ(file_open_or_create_n	.pos(), int64_t{0});
		ASSERT_EQ(file_open_existing_e	.pos(), int64_t{0});

	}

	TEST(core_file, open_modes_read_write)
	{
		const std::filesystem::path fileName_create_e			= "open_RW_create_e_test.txt";
		const std::filesystem::path fileName_create_n			= "open_RW_create_n_test.txt";
		const std::filesystem::path fileName_create_if_new_e	= "open_RW_create_if_new_e_test.txt";
		const std::filesystem::path fileName_create_if_new_n	= "open_RW_create_if_new_n_test.txt";
		const std::filesystem::path fileName_open_or_create_e	= "open_RW_open_or_create_e_test.txt";
		const std::filesystem::path fileName_open_or_create_n	= "open_RW_open_or_create_n_test.txt";
		const std::filesystem::path fileName_open_existing_e	= "open_RW_open_existing_e_test.txt";
		const std::filesystem::path fileName_open_existing_n	= "open_RW_open_existing_n_test.txt";

		constexpr std::string_view test_content = "The quick brown fox jumps over the lazy dog";
		constexpr uintptr_t expected_read_count = test_content.size();

		AssistFileCleanup auto_cleanup_create_e			{fileName_create_e};
		AssistFileCleanup auto_cleanup_create_n			{fileName_create_n};
		AssistFileCleanup auto_cleanup_create_if_new_e	{fileName_create_if_new_e};
		AssistFileCleanup auto_cleanup_create_if_new_n	{fileName_create_if_new_n};
		AssistFileCleanup auto_cleanup_open_or_create_e	{fileName_open_or_create_e};
		AssistFileCleanup auto_cleanup_open_or_create_n	{fileName_open_or_create_n};
		AssistFileCleanup auto_cleanup_open_existing_e	{fileName_open_existing_e};
		AssistFileCleanup auto_cleanup_open_existing_n	{fileName_open_existing_n};

		//setup
		assist_make_file(fileName_create_e, test_content);
		assist_make_file(fileName_create_if_new_e, test_content);
		assist_make_file(fileName_open_or_create_e, test_content);
		assist_make_file(fileName_open_existing_e, test_content);
		assist_delete_file(fileName_create_n);
		assist_delete_file(fileName_create_if_new_n);
		assist_delete_file(fileName_open_or_create_n);
		assist_delete_file(fileName_open_existing_n);
		//Test
		//---- Initial state ----
		core::file_duplex file_create_e			;
		core::file_duplex file_create_n			;
		core::file_duplex file_create_if_new_e	;
		core::file_duplex file_create_if_new_n	;
		core::file_duplex file_open_or_create_e	;
		core::file_duplex file_open_or_create_n	;
		core::file_duplex file_open_existing_e	;
		core::file_duplex file_open_existing_n	;

		ASSERT_TRUE(file_create_e			.open(fileName_create_e			, core::file_duplex::open_mode::create) == std::errc{});
		ASSERT_TRUE(file_create_n			.open(fileName_create_n			, core::file_duplex::open_mode::create) == std::errc{});
		ASSERT_TRUE(file_create_if_new_e	.open(fileName_create_if_new_e	, core::file_duplex::open_mode::crete_if_new) != std::errc{});
		ASSERT_TRUE(file_create_if_new_n	.open(fileName_create_if_new_n	, core::file_duplex::open_mode::crete_if_new) == std::errc{});
		ASSERT_TRUE(file_open_or_create_e	.open(fileName_open_or_create_e	, core::file_duplex::open_mode::open_or_create) == std::errc{});
		ASSERT_TRUE(file_open_or_create_n	.open(fileName_open_or_create_n	, core::file_duplex::open_mode::open_or_create) == std::errc{});
		ASSERT_TRUE(file_open_existing_e	.open(fileName_open_existing_e	, core::file_duplex::open_mode::open_existing) == std::errc{});
		ASSERT_TRUE(file_open_existing_n	.open(fileName_open_existing_n	, core::file_duplex::open_mode::open_existing) != std::errc{});

		ASSERT_TRUE (file_create_e			.is_open());
		ASSERT_TRUE (file_create_n			.is_open());
		ASSERT_FALSE(file_create_if_new_e	.is_open());
		ASSERT_TRUE (file_create_if_new_n	.is_open());
		ASSERT_TRUE (file_open_or_create_e	.is_open());
		ASSERT_TRUE (file_open_or_create_n	.is_open());
		ASSERT_TRUE (file_open_existing_e	.is_open());
		ASSERT_FALSE(file_open_existing_n	.is_open());

		ASSERT_EQ(file_create_e			.size(), int64_t{0});
		ASSERT_EQ(file_create_n			.size(), int64_t{0});
		ASSERT_EQ(file_create_if_new_n	.size(), int64_t{0});
		ASSERT_EQ(file_open_or_create_e	.size(), static_cast<int64_t>(expected_read_count));
		ASSERT_EQ(file_open_or_create_n	.size(), int64_t{0});
		ASSERT_EQ(file_open_existing_e	.size(), static_cast<int64_t>(expected_read_count));

		ASSERT_EQ(file_create_e			.pos(), int64_t{0});
		ASSERT_EQ(file_create_n			.pos(), int64_t{0});
		ASSERT_EQ(file_create_if_new_n	.pos(), int64_t{0});
		ASSERT_EQ(file_open_or_create_e	.pos(), int64_t{0});
		ASSERT_EQ(file_open_or_create_n	.pos(), int64_t{0});
		ASSERT_EQ(file_open_existing_e	.pos(), int64_t{0});


	}


}


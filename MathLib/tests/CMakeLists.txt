cmake_minimum_required(VERSION 4.2.3)
project(MathLib_test)
enable_language(CXX)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS 
	"./hypercomplex/*.cpp"
	"./LinearAlgebra/*.cpp"
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} LINK_PRIVATE MathLib CoreLib)
target_link_libraries(${PROJECT_NAME} LINK_PRIVATE gtest gtest_main)

quick_default_post_config()
if((CMAKE_SYSTEM_NAME STREQUAL "Linux") AND (CMAKE_CXX_COMPILER_ID STREQUAL "Clang") AND (NOT CMAKE_BUILD_TYPE STREQUAL "Debug"))
	target_compile_options(${PROJECT_NAME} PRIVATE "-Wno-missing-braces")
endif()

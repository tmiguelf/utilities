cmake_minimum_required(VERSION 4.2.3)
project(version_lib)

set(HEADER_FILE_ONLY ON)

add_library(${PROJECT_NAME} INTERFACE)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows") 
	enable_language(RC)
	target_include_directories(${PROJECT_NAME} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
endif()

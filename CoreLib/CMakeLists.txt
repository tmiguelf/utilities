cmake_minimum_required(VERSION 4.2.3)
project(CoreLib)
enable_language(CXX)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS 
	"./src/*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	target_link_libraries(${PROJECT_NAME} INTERFACE Ws2_32.lib Dbghelp.lib)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	target_link_libraries(${PROJECT_NAME} INTERFACE pthread dl)
endif()

quick_default_post_config()

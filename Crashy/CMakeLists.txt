cmake_minimum_required(VERSION 4.2.3)
project(Crashy)
enable_language(CXX)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS 
	"./src/*.cpp"
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	enable_language(RC)
	set(RESOURCE_FILES  "./resources/genericIcon.rc" )
else()
	set(RESOURCE_FILES "")
endif()

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE CoreLib version_lib)

quick_default_post_config()



cmake_minimum_required(VERSION 4.2.3)
project(utilities)

set(INFO_CONFIG "${CMAKE_SYSTEM_NAME}_${CMAKE_CXX_COMPILER_ID}_${CMAKE_SYSTEM_PROCESSOR}_${CMAKE_BUILD_TYPE}")
set(CMAKE_PDB_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/DebugData/${INFO_CONFIG}")
set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/Runtime/${INFO_CONFIG}")

include("${CMAKE_CURRENT_SOURCE_DIR}/submodules/quick_cmake/quick.cmake")
quick_add_collection_presets()

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/version")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/CoreLib")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/MathLib")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Crashy")

#Tests
set(BENCHMARK_ENABLE_EXCEPTIONS False)
set(INSTALL_GTEST OFF)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/submodules/googletest/")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/submodules/benchmark")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/CoreLib/tests")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/CoreLib/benchmarks/charconv_benchmark")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/CoreLib/benchmarks/toPrint_benchmark")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/MathLib/tests")

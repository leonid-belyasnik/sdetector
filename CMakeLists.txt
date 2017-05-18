cmake_minimum_required(VERSION 2.8)
 
set(solution_name test_1)
set(app_name sdetector)
set(svr_name scannerd)

add_definitions(-std=c++11)

project(${solution_name})

find_package(Threads)

set(bin_path "${CMAKE_SOURCE_DIR}/bin")
set(EXECUTABLE_OUTPUT_PATH ${bin_path})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${bin_path})

set(src_path "${CMAKE_SOURCE_DIR}/src")
set(src_path_common "${src_path}/common")

option(_USE_TEST "Use Test applications" ON)

add_subdirectory(${src_path}/application)
add_subdirectory(${src_path}/scanner)
add_subdirectory(${src_path}/test)

if (MSVC)
	option(_USE_VLD "Use Visual leak detector" OFF)
endif (MSVC)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${app_name})
configure_file(${src_path_common}/config.h.in ${src_path_common}/config.h)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)

set(src_path_app "${src_path}/application")

file(GLOB ${app_name}_SOURCES ${src_path_app}/*.cpp ${src_path_common}/*.cpp)
file(GLOB ${app_name}_HEADERS ${src_path_app}/*.h ${src_path_common}/*.h)
file(GLOB ${app_name}_SOURCE_LIST ${${app_name}_SOURCES} ${${app_name}_HEADERS}) 
set (COMPILE_APP_SOURCE_LIST ${${app_name}_SOURCE_LIST})

include_directories(${src_path_common})

add_executable(${app_name} ${COMPILE_APP_SOURCE_LIST})
target_link_libraries (${app_name} ${CMAKE_THREAD_LIBS_INIT})

qt5_use_modules(${app_name} Core Widgets)


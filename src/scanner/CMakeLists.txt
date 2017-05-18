set(src_path_svr "${src_path}/scanner")

file(GLOB ${svr_name}_SOURCES ${src_path_svr}/*.cpp ${src_path_common}/*.cpp)
file(GLOB ${svr_name}_HEADERS ${src_path_svr}/*.h ${src_path_common}/*.h)
file(GLOB ${svr_name}_SOURCE_LIST ${${svr_name}_SOURCES} ${${svr_name}_HEADERS}) 
set (COMPILE_SVR_SOURCE_LIST ${${svr_name}_SOURCE_LIST})

include_directories(${src_path_common})

add_executable(${svr_name} ${COMPILE_SVR_SOURCE_LIST})
target_link_libraries(${svr_name} ${CMAKE_THREAD_LIBS_INIT})

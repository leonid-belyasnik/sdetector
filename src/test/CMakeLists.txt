set(src_path_app "${src_path}/application")
set(src_path_svr "${src_path}/scanner")
set(src_path_test "${src_path}/test")

include_directories(${src_path_common})
include_directories(${src_path_app})
include_directories(${src_path_svr})

# if ( _USE_TEST )
add_executable(test_list 
	${src_path_test}/testMalwareList.cpp 
	${src_path_svr}/malware_list.h 
	${src_path_svr}/malware_list.cpp
	${src_path_svr}/seeker.h 
	${src_path_svr}/seeker.cpp 
	${src_path_svr}/mmfile.cpp 
	${src_path_svr}/mmfile.h
)

add_executable(test_seeker 
	${src_path_test}/testBMSeeker.cpp 
	${src_path_svr}/seeker.h 
	${src_path_svr}/seeker.cpp 
	${src_path_svr}/mmfile.cpp 
	${src_path_svr}/mmfile.h
	${src_path_svr}/malware_list.h 
	${src_path_svr}/malware_list.cpp
)

add_executable(test_client 
	${src_path_test}/testClient.cpp 
	${src_path_common}/common.h 
	${src_path_common}/socket.cpp 
	${src_path_common}/socket.h
	${src_path_app}/client.h
	${src_path_app}/client.cpp 
)

# endif ( _USE_TEST )

include_directories(
	${SOLUTION_INCLUDE_DIRS}
)
link_directories(
	${SOLUTION_LINK_DIRS}
    ${src_dir}/lib
)

add_executable(vehicle_reid
	httplib.h
	vehicle_reid.cpp
)
add_executable(single_reid
    httplib.h
    ../edge/lpr_alg.h
    ../edge/lpr_alg.cpp
    single_reid.cpp
)

target_link_libraries(vehicle_reid pthread jsoncpp common boost_thread boost_system thplateid)

target_link_libraries(single_reid pthread jsoncpp common boost_thread boost_system thplateid)

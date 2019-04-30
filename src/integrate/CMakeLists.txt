include_directories(
	${SOLUTION_INCLUDE_DIRS}
    /usr/local/include
)
link_directories(
    /usr/local/lib
)

add_executable(vehicle_reid
    httplib.h
	vehicle_reid.cpp
)

target_link_libraries(vehicle_reid pthread jsoncpp)

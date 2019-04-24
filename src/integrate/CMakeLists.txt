include_directories(
	${SOLUTION_INCLUDE_DIRS}
)
link_directories(
)

add_executable(vehicle_reid
	vehicle_reid.cpp
)

target_link_libraries(vehicle_reid pthread )

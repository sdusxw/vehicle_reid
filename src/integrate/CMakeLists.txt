include_directories(
	${SOLUTION_INCLUDE_DIRS}
    /usr/local/include
)
link_directories(
    /usr/local/lib
)

add_executable(vehicle_reid
    httplib.h
    jpg_codec.h
    jpg_codec.cpp
    main.c
    TH_PlateID.h
    TH_ErrorDef.h
	vehicle_reid.cpp
)

target_link_libraries(vehicle_reid pthread jpeg jsoncpp)

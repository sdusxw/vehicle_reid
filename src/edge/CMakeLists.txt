include_directories(
	${SOLUTION_INCLUDE_DIRS}
)
link_directories(
    ${src_dir}/lib
)

add_executable(lpr_edge
	TH_PlateID.h
	TH_ErrorDef.h
    lpr_alg.h
    lpr_alg.cpp
	lpr_edge.cpp
)

add_executable(lpr_test
    TH_PlateID.h
    TH_ErrorDef.h
    lpr_alg.h
    lpr_alg.cpp
    lpr_test.cpp
)

add_executable(shm_test
    lpr_alg.h
    lpr_alg.cpp
    test_shm.cpp
)

target_link_libraries(lpr_edge pthread common thplateid)

target_link_libraries(lpr_test pthread common thplateid)

target_link_libraries(shm_test pthread common thplateid)

include_directories(
	${SOLUTION_INCLUDE_DIRS}
)
link_directories(
)

add_executable(lpr_edge
	TH_PlateID.h
	TH_ErrorDef.h
	lpr_edge.cpp
)

target_link_libraries(lpr_edge pthread common)

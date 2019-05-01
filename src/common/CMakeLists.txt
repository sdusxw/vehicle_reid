
add_library(common
	common.h
	common.cpp
	concurrent_queue.h
	concurrent_queue.cpp
	jpg_codec.h
	jpg_codec.cpp
)

target_link_libraries(common pthread jpeg boost_system boost_thread)


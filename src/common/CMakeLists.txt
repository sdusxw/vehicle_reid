
add_library(common
	concurrent_queue.h
	concurrent_queue.cpp
	jpg_codec.h
	jpg_codec.cpp
)


target_link_libraries(common pthread jpeg)


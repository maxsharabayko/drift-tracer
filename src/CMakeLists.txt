project(lib-drift-tracer)

FILE(GLOB SOURCES *.cpp *.hpp ./packet/*.cpp ./packet/*.hpp)

add_library(lib-drift-tracer ${SOURCES})

set_target_properties(lib-drift-tracer
	PROPERTIES
	CXX_STANDARD 17
)


project(lib-drift-tracer)

FILE(GLOB SOURCES *.cpp *.hpp ./packet/*.cpp ./packet/*.hpp)

add_library(lib-drift-tracer ${SOURCES})

target_compile_definitions(lib-drift-tracer
	PUBLIC
	#SPDLOG_COMPILED_LIB
	SPDLOG_FMT_EXTERNAL
)

set_target_properties(lib-drift-tracer
	PROPERTIES
	CXX_STANDARD 17
)


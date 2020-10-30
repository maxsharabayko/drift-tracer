project(drift-tracer)

FILE(GLOB SOURCES *.cpp *.hpp)

add_executable(drift-tracer ${SOURCES})

target_include_directories(drift-tracer PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(drift-tracer
	PRIVATE CONAN_PKG::cli11
	PRIVATE CONAN_PKG::spdlog
)

set_target_properties(drift-tracer
	PROPERTIES
	CXX_STANDARD 17
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_DIR}"
)


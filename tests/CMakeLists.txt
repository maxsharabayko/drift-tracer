project(test-drift-tracer)

FILE(GLOB SOURCES *.cpp *.hpp)

enable_testing()

add_executable(test-drift-tracer ${SOURCES})
list(APPEND SOURCES_TO_FORMAT ${SOURCES})

target_include_directories(test-drift-tracer PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CONAN_INCLUDE_DIRS_CATCH2}
	)

target_link_libraries(test-drift-tracer
    PRIVATE CONAN_PKG::catch2
    )

set_target_properties(test-drift-tracer
	PROPERTIES
	CXX_STANDARD 17
)

add_test(drift-tracer test-drift-tracer)

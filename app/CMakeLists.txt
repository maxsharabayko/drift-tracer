project(drift-tracer)

FILE(GLOB SOURCES *.cpp *.hpp)

add_executable(drift-tracer ${SOURCES})

target_include_directories(drift-tracer PRIVATE
	${CMAKE_SOURCE_DIR}/src
)

target_compile_definitions(drift-tracer
	PUBLIC
	SPDLOG_FMT_EXTERNAL
)

target_link_libraries(drift-tracer
	PRIVATE CONAN_PKG::cli11
	PRIVATE CONAN_PKG::spdlog
	PRIVATE CONAN_PKG::fmt
	PRIVATE lib-drift-tracer
)

if (WIN32 AND (NOT MINGW AND NOT CYGWIN))
	target_link_libraries(drift-tracer PRIVATE ws2_32.lib)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(drift-tracer PUBLIC atomic)
endif()

set_target_properties(drift-tracer
	PROPERTIES
	CXX_STANDARD 17
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)


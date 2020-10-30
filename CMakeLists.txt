cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(DRIF_TTRACER VERSION 0.0.1 LANGUAGES C CXX)
set(VERSION_TYPE "dev" CACHE STRING "version type" FORCE)
site_name(VERSION_HOST) # read hostname to VERSION_HOST
set(VERSION_HOST "${VERSION_HOST}" CACHE STRING "host of build" FORCE)

message(STATUS "")
message(STATUS "=== ${PROJECT_NAME} Project Configuration ===")
message(STATUS "")

#------------------------------------------------------------------------------
# General settings

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS NO_OUTPUT_DIRS)

# Be nice to visual studio
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Be nice and export compile commands by default, this is handy for clang-tidy
# and for other tools.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# We can use include() and find_package() for our scripts in there
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Helpful option enable build profiling to identify slowly compiling files
option(MEASURE_ALL "When enabled all commands will be passed through time command" OFF)
if(MEASURE_ALL)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "time")
endif()

#-------------------------------------------------------------------------------
# Set default install location to dist folder in build dir
# we do not want to install to /usr by default
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/dist" CACHE PATH
        "Install path prefix, prepended onto install directories." FORCE )
endif()

#------------------------------------------------------------------------------
# Included CMakeLists.txt

# Testing
enable_testing()
add_subdirectory(tests)

# Library source code
add_subdirectory(src)

#-------------------------------------------------------------------------------
# Wrap up of settings printed on build
message(STATUS "")
message(STATUS "Version:              ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH} ${VERSION_TYPE} @ ${VERSION_HOST}")
message(STATUS "Install prefix:       ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Compiler:             ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_BUILD_TYPE:     ${CMAKE_BUILD_TYPE}")
message(STATUS "Catch2 include:       ${CONAN_INCLUDE_DIRS_CATCH2}")
message(STATUS "")


# A target named "clangformat", use make clangformat to format all specified files
include(ClangFormat)
set(SOURCES_TO_FORMAT "src/buf_view.hpp")
message(STATUS "SOURCES_TO_FORMAT: ${SOURCES_TO_FORMAT}")
clangformat_setup(${SOURCES_TO_FORMAT})


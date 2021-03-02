#pragma once

#include <cstdint>
#include <cstdlib>
#include <cinttypes>

#include <chrono>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <mutex>

#include <sstream>

// Note: std::put_time is supported only in GCC 5 and higher
#if !defined(__GNUC__) || defined(__clang__) || (__GNUC__ >= 5)
#define HAS_PUT_TIME
#else
#pragma message( "ISO 8601 timepoints will not be used. Consider updating GCC to v5 or gigher!")
#endif

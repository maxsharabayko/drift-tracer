#pragma once
#include "stdafx.hpp"

/// Running moving average (RMA).
/// Also known as Smoothed moving average (SMMA), modified moving average (MMA).
template <std::size_t N, typename ValueType>
inline ValueType avg_rma(ValueType old_value, ValueType new_value)
{
    return (old_value * (N - 1) + new_value) / N;
}

/// Weighted running moving average
template <std::size_t N, typename ValueType>
inline ValueType avg_rma_w(ValueType old_value, ValueType new_value, std::size_t new_val_weight)
{
    return (old_value * (N - new_val_weight) + new_value * new_val_weight) / N;
}

inline long long count_microseconds(const std::chrono::steady_clock::duration &t)
{
    return std::chrono::duration_cast<std::chrono::microseconds>(t).count();
}

inline long long count_microseconds(const std::chrono::steady_clock::time_point tp)
{
    return std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch()).count();
}

inline long long count_milliseconds(const std::chrono::steady_clock::duration &t)
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(t).count();
}

inline long long count_seconds(const std::chrono::steady_clock::duration &t)
{
    return std::chrono::duration_cast<std::chrono::seconds>(t).count();
}

inline std::chrono::steady_clock::duration microseconds_from(int64_t t_us)
{
    return std::chrono::microseconds(t_us);
}

inline std::chrono::steady_clock::duration milliseconds_from(int64_t t_ms)
{
    return std::chrono::milliseconds(t_ms);
}

inline std::chrono::steady_clock::duration seconds_from(int64_t t_s)
{
    return std::chrono::seconds(t_s);
}

namespace {
template <int val>
int pow10();

template <>
int pow10<10>()
{
    return 1;
}

template <int val>
int pow10()
{
    return 1 + pow10<val / 10>();
}
}

/// Prints the provided steady clock time in human readable manner
inline std::string format_time_stdy(const std::chrono::steady_clock::time_point& timestamp)
{
    using namespace std;
    if (timestamp == std::chrono::steady_clock::time_point())
    {
        // Use special string for 0
        return "00:00:00.000000";
    }

    const int64_t ticks_per_sec = (std::chrono::steady_clock::period::den / std::chrono::steady_clock::period::num);
    const int     decimals      = pow10<ticks_per_sec>();
    const uint64_t total_sec = count_seconds(timestamp.time_since_epoch());
    const uint64_t days = total_sec / (60 * 60 * 24);
    const uint64_t hours = total_sec / (60 * 60) - days * 24;
    const uint64_t minutes = total_sec / 60 - (days * 24 * 60) - hours * 60;
    const uint64_t seconds = total_sec - (days * 24 * 60 * 60) - hours * 60 * 60 - minutes * 60;
    ostringstream out;
    if (days)
        out << days << "D ";
    out << setfill('0') << setw(2) << hours << ":"
        << setfill('0') << setw(2) << minutes << ":"
        << setfill('0') << setw(2) << seconds << "."
        << setfill('0') << setw(decimals) << (timestamp - seconds_from(total_sec)).time_since_epoch().count();
    return out.str();
}

inline struct tm sys_local_time(time_t tt)
{
    struct tm tms;
    memset(&tms, 0, sizeof tms);
#ifdef _WIN32
	errno_t rr = localtime_s(&tms, &tt);
	if (rr == 0)
		return tms;
#else

    // Ignore the error, state that if something
    // happened, you simply have a pre-cleared tms.
    localtime_r(&tt, &tms);
#endif

    return tms;
}

/// Prints the provided steady clock time mapped onto system time in human readable manner
inline std::string format_time_sys(const std::chrono::steady_clock::time_point& timestamp)
{
    using namespace std;
    using namespace chrono;
    const time_t                   now_s         = ::time(NULL); // get current time in seconds
    const steady_clock::time_point now_timestamp = steady_clock::now();
    const int64_t                  delta_us      = count_microseconds(timestamp - now_timestamp);
    const int64_t                  delta_s =
        floor((static_cast<int64_t>(count_microseconds(now_timestamp.time_since_epoch()) % 1000000) + delta_us) / 1000000.0);
    const time_t tt = now_s + delta_s;
    struct tm    tm = sys_local_time(tt); // in seconds
    char         tmp_buf[512];
    strftime(tmp_buf, 512, "%X.", &tm);

    ostringstream out;
    out << tmp_buf << setfill('0') << setw(6) << (count_microseconds(timestamp.time_since_epoch()) % 1000000);
    return out.str();
}

#ifdef HAS_PUT_TIME
// Follows ISO 8601
inline std::string print_timestamp()
{
    using namespace std;
    using namespace std::chrono;

    const auto   systime_now = system_clock::now();
    const time_t time_now    = system_clock::to_time_t(systime_now);

    std::ostringstream output;

    // SysLocalTime returns zeroed tm_now on failure, which is ok for put_time.
    const tm tm_now = sys_local_time(time_now);
    output << std::put_time(&tm_now, "%FT%T.") << std::setfill('0') << std::setw(6);
    const auto    since_epoch = systime_now.time_since_epoch();
    const seconds s           = duration_cast<seconds>(since_epoch);
    output << duration_cast<microseconds>(since_epoch - s).count();
    output << std::put_time(&tm_now, "%z");
    return output.str();
}
#endif // HAS_PUT_TIME

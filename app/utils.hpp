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

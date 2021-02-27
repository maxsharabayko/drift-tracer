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

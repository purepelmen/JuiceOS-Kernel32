#pragma once

template <typename T>
T min(T a, T b)
{
    return a < b ? a : b;
}

template <typename T>
T max(T a, T b)
{
    return a > b ? a : b;
}

template <typename T>
constexpr T divide_round_up(T a, T b)
{
    return a / b + min<T>(1, a % b);
}

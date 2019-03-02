#pragma once

#include <cstddef>
#include <cstdint>

namespace immutable
{
namespace detail
{

using Bitmap = std::uint64_t;
using Count  = std::uint32_t;
using Shift  = std::uint32_t;
using Hash   = std::size_t;
using Size   = std::size_t;

template <Count B, typename T = Shift>
constexpr T max_shift = ((sizeof(Hash) * 8u + B - 1u) / B) * B;

template <Count B, typename T = Bitmap>
constexpr T mask = (1ul << B) - 1ul;

inline Count popcount(std::uint64_t x)
{
    return __builtin_popcountll(x);
}

} // namespace detail
} // namespace immutable

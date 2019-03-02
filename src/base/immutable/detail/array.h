#pragma once

#include "allocation.h"
#include "bits.h"

#include <cassert>
#include <memory>

namespace immutable
{
namespace detail
{

template <typename T>
inline T* make_array(Count size)
{
    assert(size > 0);
    return allocation::allocate<T>(size);
}

template <typename T>
inline T* make_array(T* first, Count size)
{
    auto last       = first + size;
    auto dest_first = make_array<T>(size);

    std::uninitialized_copy(first, last, dest_first);

    return dest_first;
}

template <typename T, typename... Args>
inline T* make_array_insert(T*      first,
                            Count size,
                            Count position,
                            Args&&... args)
{
    assert((position <= size) && (position >= 0));

    auto placeholder      = first + position;
    auto last             = first + size;
    auto dest_size        = size + 1;
    auto dest_first       = make_array<T>(dest_size);
    auto dest_placeholder = dest_first + position;

    std::uninitialized_copy(first, placeholder, dest_first);

    allocation::construct(dest_placeholder, std::forward<Args>(args)...);

    std::uninitialized_copy(placeholder, last, dest_placeholder + 1);

    return dest_first;
}

template <typename T, typename... Args>
inline T* make_array_replace(T*      first,
                             Count size,
                             Count position,
                             Args&&... args)
{
    assert((position < size) && (position >= 0));

    auto placeholder      = first + position;
    auto last             = first + size;
    auto dest_size        = size;
    auto dest_first       = make_array<T>(dest_size);
    auto dest_placeholder = dest_first + position;

    std::uninitialized_copy(first, placeholder, dest_first);

    allocation::construct(dest_placeholder, std::forward<Args>(args)...);

    std::uninitialized_copy(placeholder + 1, last, dest_placeholder + 1);

    return dest_first;
}

template <typename T, typename... Args>
inline T* make_array_erase(T* first, Count size, Count position)
{
    assert((position < size) && (position >= 0));

    if (size == 1)
    {
        assert(position == 0);
        return nullptr;
    }

    auto placeholder      = first + position;
    auto last             = first + size;
    auto dest_size        = size - 1;
    auto dest_first       = make_array<T>(dest_size);
    auto dest_placeholder = dest_first + position;

    std::uninitialized_copy(first, placeholder, dest_first);
    std::uninitialized_copy(placeholder + 1, last, dest_placeholder);

    return dest_first;
}

} // namespace detail
} // namespace immutable

#pragma once

#include <cstddef>
#include <memory>

namespace immutable
{
namespace detail
{
namespace allocation
{

template <typename T>
static T* allocate(std::size_t size)
{
    return std::allocator<T> {}.allocate(size);
}

template <typename T>
static void deallocate(T* pointer, std::size_t size)
{
    std::allocator<T> {}.deallocate(pointer, size);
}

template <typename T, typename... Args>
static void construct(T* pointer, Args&&... args)
{
    std::allocator<T> {}.construct(pointer, std::forward<Args>(args)...);
}

template <typename T>
static void destroy(T* pointer)
{
    std::allocator<T> {}.destroy(pointer);
}

} // namespace allocation
} // namespace detail
} // namespace immutable

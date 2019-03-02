#pragma once

#include <atomic>
#include <cstdint>

namespace immutable
{
namespace detail
{

struct RefCounted
{
    using RefCount = std::atomic<std::uint64_t>;

    RefCounted()
        : m_ref_count(new RefCount(1))
    {}

    RefCounted(const RefCounted& other)
        : m_ref_count(other.m_ref_count)
    {}

    ~RefCounted()
    {
        if (is_unique())
        {
            delete m_ref_count;
        }
    }

    void inc()
    {
        m_ref_count->fetch_add(1, std::memory_order_relaxed);
    }

    bool dec()
    {
        return m_ref_count->fetch_sub(1, std::memory_order_release) == 1;
    }

    bool is_unique() const
    {
        return m_ref_count->load() == 1u;
    }

    RefCount* m_ref_count;
};

} // namespace detail
} // namespace immutable

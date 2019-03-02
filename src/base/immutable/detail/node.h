#pragma once

#include "allocation.h"
#include "array.h"
#include "bits.h"
#include "refcounted.h"

#include <cassert>
#include <stdexcept>
#include <type_traits>
#include <variant>

namespace immutable
{
namespace detail
{

template <typename T, Count B>
struct Node : public RefCounted
{
    using Data = T;

    struct Inner
    {
        Bitmap datamap;
        Bitmap nodemap;
        Data*  data;
        Node** children;
    };

    struct Collision
    {
        Size  size;
        Data* data;
    };

    using Impl = std::variant<Inner, Collision>;

    struct InnerTag
    {};

    struct CollisionTag
    {};

    Node(InnerTag)
        : m_impl(Inner {0, 0, nullptr, nullptr})
    {}

    Node(CollisionTag)
        : m_impl(Collision {0, nullptr})
    {}

    Node(Node&& other)
        : m_impl(std::move(other.m_impl))
    {}

    ~Node()
    {
        if (std::get_if<Inner>(&m_impl))
        {
            auto n_first = children();
            auto n_size  = children_size();

            dec_children(n_first, n_first + n_size);
            allocation::deallocate(n_first, n_size);

            auto d_first = data();
            auto d_size  = data_size();
            auto d_last  = d_first + d_size;

            for (; d_first < d_last; ++d_first)
            {
                allocation::destroy(d_first);
            }

            allocation::deallocate(data(), d_size);
        }
        else
        {
            auto c_first = collision_data();
            auto c_size  = collision_size();
            auto c_last  = c_first + c_size;

            for (; c_first < c_last; ++c_first)
            {
                allocation::destroy(c_first);
            }

            allocation::deallocate(collision_data(), c_size);
        }
    }

    auto& inner()
    {
        return std::get<Inner>(m_impl);
    }

    const auto& inner() const
    {
        return std::get<Inner>(m_impl);
    }

    auto& collision()
    {
        return std::get<Collision>(m_impl);
    }

    const auto& collision() const
    {
        return std::get<Collision>(m_impl);
    }

    auto datamap() const
    {
        return inner().datamap;
    }

    auto nodemap() const
    {
        return inner().nodemap;
    }

    auto data() const
    {
        return inner().data;
    }

    auto children() const
    {
        return inner().children;
    }

    auto collision_data() const
    {
        return collision().data;
    }

    auto data_size() const
    {
        return popcount(datamap());
    }

    auto children_size() const
    {
        return popcount(nodemap());
    }

    auto collision_size() const
    {
        return collision().size;
    }

    static void inc_children(Node** first, Node** last)
    {
        for (; first < last; ++first)
        {
            (*first)->inc();
        }
    }

    static void dec_children(Node** first, Node** last)
    {
        for (; first < last; ++first)
        {
            if ((*first)->dec())
            {
                allocation::destroy(*first);
                allocation::deallocate(*first, 1);
            }
        }
    }

    static auto make_inner_n()
    {
        Node* dest = allocation::allocate<Node>(1);
        allocation::construct(dest, InnerTag {});

        return dest;
    }

    static auto make_collision_n()
    {
        Node* dest = allocation::allocate<Node>(1);
        allocation::construct(dest, CollisionTag {});

        return dest;
    }

    static Node* make_collision_n(Data&& a, Data&& b)
    {
        Node* dest = make_collision_n();

        dest->collision().size = 2;
        dest->collision().data = make_array<Data>(2);

        allocation::construct(dest->collision().data, std::move(a));
        allocation::construct(dest->collision().data + 1, std::move(b));

        return dest;
    }

    Node* replace_value(Data&& value, Count compact_idx) const
    {
        Node* dest = make_inner_n();

        dest->inner().datamap  = datamap();
        dest->inner().data     = make_array_replace(data(),
                                                data_size(),
                                                compact_idx,
                                                std::move(value));
        dest->inner().nodemap  = nodemap();
        dest->inner().children = children();

        inc_children(children(), children() + children_size());

        return dest;
    }

    Node* insert_value(Data&& value, Bitmap bit) const
    {
        Node* dest = make_inner_n();

        auto compact_idx = popcount(datamap() & (bit - 1));

        dest->inner().datamap  = datamap() | bit;
        dest->inner().data     = make_array_insert(data(),
                                               data_size(),
                                               compact_idx,
                                               std::move(value));
        dest->inner().nodemap  = nodemap();
        dest->inner().children = children();

        inc_children(children(), children() + children_size());

        return dest;
    }

    Node* erase_value(Count d_compact_idx, Bitmap bit) const
    {
        Node* dest = make_inner_n();

        dest->inner().datamap  = datamap() & ~bit;
        dest->inner().data     = make_array_erase(data(),
                                              data_size(),
                                              d_compact_idx);
        dest->inner().nodemap  = nodemap();
        dest->inner().children = children();

        inc_children(children(), children() + children_size());

        return dest;
    }

    Node* replace_child(Node* new_n, Count compact_idx) const
    {
        Node* dest = make_inner_n();

        auto first = children();
        auto size  = children_size();

        dest->inner().datamap  = datamap();
        dest->inner().data     = data();
        dest->inner().nodemap  = nodemap();
        dest->inner().children = make_array_replace(first,
                                                    size,
                                                    compact_idx,
                                                    new_n);

        inc_children(first, first + compact_idx);
        inc_children(first + compact_idx + 1, first + size);

        return dest;
    }

    Node* insert_child_erase_value(Node*  child,
                                   Count  d_compact_idx,
                                   Bitmap bit) const
    {
        Node* dest = make_inner_n();

        auto n_compact_idx = popcount(nodemap() & (bit - 1));
        auto first         = children();
        auto size          = children_size();

        dest->inner().datamap = datamap() & ~bit;
        dest->inner().data    = make_array_erase(data(),
                                              data_size(),
                                              d_compact_idx);

        dest->inner().nodemap  = nodemap() | bit;
        dest->inner().children = make_array_insert(first,
                                                   size,
                                                   n_compact_idx,
                                                   child);

        inc_children(first, first + size);

        return dest;
    }

    Node* insert_value_erase_child(Data&& value,
                                   Count  n_compact_idx,
                                   Bitmap bit) const
    {
        Node* dest = make_inner_n();

        auto d_compact_idx = popcount(datamap() & (bit - 1));
        auto first         = children();
        auto size          = children_size();

        dest->inner().nodemap  = nodemap() & ~bit;
        dest->inner().children = make_array_erase(first, size, n_compact_idx);
        dest->inner().datamap  = datamap() | bit;
        dest->inner().data     = make_array_insert(data(),
                                               data_size(),
                                               d_compact_idx,
                                               std::move(value));

        inc_children(first, first + n_compact_idx);
        inc_children(first + n_compact_idx + 1, first + size);

        return dest;
    }

    Node* replace_collision(Data&& value, Count idx) const
    {
        Node* dest = make_collision_n();

        dest->collision().size = collision_size();
        dest->collision().data = make_array_replace(collision_data(),
                                                    collision_size(),
                                                    idx,
                                                    std::move(value));

        return dest;
    }

    Node* insert_collision(Data&& value) const
    {
        Node* dest = make_collision_n();

        dest->collision().size = collision_size() + 1;
        dest->collision().data = make_array_insert(collision_data(),
                                                   collision_size(),
                                                   collision_size(),
                                                   std::move(value));

        return dest;
    }

    Node* erase_collision(Count idx) const
    {
        assert(collision_size() > 2);

        Node* dest = make_collision_n();

        dest->collision().size = collision_size() - 1;
        dest->collision().data = make_array_erase(collision_data(),
                                                  collision_size(),
                                                  idx);

        return dest;
    }

    Impl m_impl;
};

} // namespace detail
} // namespace immutable

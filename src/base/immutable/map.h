#pragma once

#include "detail/allocation.h"
#include "detail/iterator.h"
#include "detail/node.h"
#include "detail/operations.h"

#include <functional>
#include <stdexcept>
#include <utility>

namespace immutable
{

template <typename K, typename V, typename H = std::hash<K>>
class Map
{
  public:
    static constexpr detail::Count branches = 6u;
    using Key                               = K;
    using Value                             = V;
    using Pair                              = std::pair<const K, const V>;
    using Node                              = detail::Node<Pair, branches>;
    using Iterator                          = detail::Iterator<Pair, branches>;

    Map()
    {
        m_root = detail::allocation::allocate<Node>(1);
        detail::allocation::construct(m_root, typename Node::InnerTag {});
    }

    Map(const Map& other)
        : m_root(other.m_root)
    {
        m_root->inc();
    }

    Map(Map&& other)
        : m_root(nullptr)
    {
        std::swap(other.m_root, m_root);
    }

    Map& operator=(Map other)
    {
        std::swap(other.m_root, m_root);
        return *this;
    }

    const Value& get(K key) const
    {
        return detail::MapGetOp<K, V, H, branches> {}.get(m_root,
                                                          std::move(key));
    }

    Map set(K key, V value) const
    {
        auto res = detail::MapSetOp<K, V, H, branches> {}
                       .set(m_root, std::move(Pair {key, value}));
        if (auto n = std::get_if<Node*>(&res))
        {
            return Map {*n};
        }
        return *this;
    }

    Map erase(K key) const
    {
        auto res = detail::MapEraseOp<K, V, H, branches> {}.erase(m_root,
                                                                  std::move(
                                                                      key));
        if (auto n = std::get_if<Node*>(&res))
        {
            return Map {*n};
        }
        else if (auto v = std::get_if<Pair>(&res))
        {
            assert(false && "Wrong return value.");
        }
        return *this;
    }

    Iterator begin() const
    {
        return Iterator(m_root);
    }

    Iterator end() const
    {
        return Iterator();
    }

    ~Map()
    {
        if (m_root && m_root->dec())
        {
            detail::allocation::destroy(m_root);
            detail::allocation::deallocate(m_root, 1);
        }
    }

  private:
    Map(Node* m_root)
        : m_root(m_root)
    {}

    Node* m_root;
};

} // namespace immutable

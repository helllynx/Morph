#pragma once

#include "array.h"
#include "bits.h"
#include "node.h"

#include <utility>
#include <variant>

namespace immutable
{
namespace detail
{

//
//  Resolve two values collision in the map node.
//
template <typename K, typename V, typename H, Count B>
struct MapMergeNodesOp
{
    using Data   = std::pair<const K, const V>;
    using HashFn = H;

    Node<Data, B>* merge(Data&& a,
                         Hash   a_hash,
                         Data&& b,
                         Hash   b_hash,
                         Shift  shift)
    {
        auto a_sparse_idx = (a_hash >> shift) & (mask<B>);
        auto b_sparse_idx = (b_hash >> shift) & (mask<B>);

        if (a_sparse_idx != b_sparse_idx)
        {
            // TODO: refactor this. (Special node constructor?)
            Node<Data, B>* dest  = Node<Data, B>::make_inner_n();
            auto           a_bit = Bitmap {1u} << a_sparse_idx;
            auto           b_bit = Bitmap {1u} << b_sparse_idx;

            dest->inner().datamap = a_bit | b_bit;
            dest->inner().data    = make_array<Data>(2);

            allocation::construct(dest->inner().data + (a_bit > b_bit),
                                  std::move(a));

            allocation::construct(dest->inner().data + (b_bit > a_bit),
                                  std::move(b));

            return dest;
        }

        Node<Data, B>* dest = Node<Data, B>::make_inner_n();

        auto bit               = Bitmap {1u} << a_sparse_idx;
        dest->inner().nodemap  = bit;
        dest->inner().children = make_array<Node<Data, B>*>(1);

        if (shift + B >= max_shift<B>)
        {
            auto collision_n = Node<Data, B>::make_collision_n(std::move(a),
                                                               std::move(b));
            allocation::construct(dest->inner().children, collision_n);

            return dest;
        }

        auto inner_n = merge(std::move(a),
                             a_hash,
                             std::move(b),
                             b_hash,
                             shift + B);

        allocation::construct(dest->inner().children, inner_n);
        return dest;
    }
};

//
//  Set the value in the map node.
//
template <typename K, typename V, typename H, Count B>
struct MapSetOp
{
    using Data   = std::pair<const K, const V>;
    using HashFn = H;

    // Set yields either a new node or nothing (in case element already exists).
    using SetResult = std::variant<std::monostate, Node<Data, B>*>;

    SetResult set(Node<Data, B>* node, Data&& value) const
    {
        auto hash = HashFn {}(value.first);
        return set(node, std::move(value), hash, 0);
    }

    SetResult set(Node<Data, B>* node,
                  Data&&         value,
                  Hash           hash,
                  Shift          shift) const
    {
        if (shift >= max_shift<B>)
        {
            // We reached maximum tree depth. This is collision node.
            for (Size idx = 0; idx < node->collision_size(); ++idx)
            {
                auto data_ptr = node->collision_data() + idx;
                if (data_ptr->first == value.first)
                {
                    return node->replace_collision(std::move(value), idx);
                }
            }

            return node->insert_collision(std::move(value));
        }

        auto sparse_idx = (hash >> shift) & mask<B>;
        auto bit        = Bitmap {1u} << sparse_idx;

        if (bit & node->datamap())
        {
            auto compact_idx = popcount(node->datamap() & (bit - 1));
            auto prev_value  = node->data() + compact_idx;

            if (prev_value->first == value.first)
            {
                if (prev_value->second == value.second)
                {
                    // We found the same element in the map. Nothing to be done.
                    return {};
                }

                return node->replace_value(std::move(value), compact_idx);
            }

            auto old_value = *prev_value;

            // Don't calculate hash if it's not needed.
            if (shift + B >= max_shift<B>)
            {
                auto child = Node<Data, B>::make_collision_n(std::move(
                                                                 old_value),
                                                             std::move(value));

                return node->insert_child_erase_value(child, compact_idx, bit);
            }

            auto old_hash = HashFn {}(prev_value->first);

            auto merge_op = MapMergeNodesOp<K, V, H, B> {};

            auto child = merge_op.merge(std::move(old_value),
                                        old_hash,
                                        std::move(value),
                                        hash,
                                        shift + B);

            return node->insert_child_erase_value(child, compact_idx, bit);
        }
        else if (bit & node->nodemap())
        {
            auto compact_idx = popcount(node->nodemap() & (bit - 1));
            auto child       = *(node->children() + compact_idx);

            auto res = set(child, std::move(value), hash, shift + B);

            if (auto n = std::get_if<Node<Data, B>*>(&res))
            {
                return node->replace_child(*n, compact_idx);
            }

            return {};
        }

        return node->insert_value(std::move(value), bit);
    }
};

template <typename K, typename V, typename H, Count B>
struct MapGetOp
{
    using Key    = K;
    using Value  = V;
    using Data   = std::pair<const K, const V>;
    using HashFn = H;

    const Value& get(Node<Data, B>* node, Key&& key) const
    {
        auto hash = HashFn {}(key);
        return get(node, std::move(key), hash, 0);
    }

    const Value& get(Node<Data, B>* node,
                     Key&&          key,
                     Hash           hash,
                     Shift          shift) const
    {
        if (shift >= max_shift<B>)
        {
            auto first = node->collision_data();
            auto last  = first + node->collision_size();

            for (; first < last; ++first)
            {
                if (first->first == key)
                {
                    return first->second;
                }
            }

            throw std::out_of_range("Key not found.");
        }

        auto sparse_idx = (hash >> shift) & mask<B>;
        auto bit        = Bitmap {1u} << sparse_idx;

        if (bit & node->datamap())
        {
            auto compact_idx = popcount(node->datamap() & (bit - 1));
            auto data_ptr    = node->data() + compact_idx;

            if (data_ptr->first == key)
            {
                return data_ptr->second;
            }
        }
        else if (bit & node->nodemap())
        {
            auto compact_idx = popcount(node->nodemap() & (bit - 1));
            auto child       = *(node->children() + compact_idx);

            return get(child, std::move(key), hash, shift + B);
        }

        throw std::out_of_range("Key not found.");
    }
};

//
//  Erase value from the map node.
//
template <typename K, typename V, typename H, Count B>
struct MapEraseOp
{
    using Key    = K;
    using Data   = std::pair<const K, const V>;
    using HashFn = H;

    using EraseResult = std::variant<std::monostate, Node<Data, B>*, Data>;

    EraseResult erase(Node<Data, B>* node, Key&& key) const
    {
        auto hash = HashFn {}(key);
        return erase(node, std::move(key), hash, 0);
    }

    EraseResult erase(Node<Data, B>* node,
                      Key&&          key,
                      Hash           hash,
                      Shift          shift) const
    {
        if (shift >= max_shift<B>)
        {
            for (size_t idx = 0; idx < node->collision_size(); ++idx)
            {
                auto data_ptr = node->collision_data() + idx;
                if (data_ptr->first == key)
                {
                    if (node->collision_size() > 2)
                    {
                        return node->erase_collision(idx);
                    }

                    return *(node->collision_data() + (idx == 0));
                }
            }

            return {};
        }

        auto sparse_idx = (hash >> shift) & mask<B>;
        auto bit        = Bitmap {1u} << sparse_idx;

        if (bit & node->datamap())
        {
            auto compact_idx = popcount(node->datamap() & (bit - 1));
            auto value       = node->data() + compact_idx;

            if (value->first == key)
            {
                if ((shift == 0) || (node->children_size() > 0)
                    || (node->data_size() > 2))
                {
                    return node->erase_value(compact_idx, bit);
                }

                // We know now that this is not the root node, the node has no
                // children and hence there is exactly 2 values (1 value no
                // children case only allowed in root node).

                return *(node->data() + (compact_idx == 0));
            }

            return {};
        }
        else if (bit & node->nodemap())
        {
            auto compact_idx = popcount(node->nodemap() & (bit - 1));
            auto child       = *(node->children() + compact_idx);

            auto res = erase(child, std::move(key), hash, shift + B);

            if (auto n = std::get_if<Node<Data, B>*>(&res))
            {
                return node->replace_child(*n, compact_idx);
            }
            else if (auto v = std::get_if<Data>(&res))
            {
                if ((shift == 0) || (node->children_size() > 1)
                    || (node->data_size() > 0))
                {
                    return node->insert_value_erase_child(std::move(*v),
                                                          compact_idx,
                                                          bit);
                }

                return std::move(res);
            }

            return {};
        }

        return {};
    }
};

} // namespace detail
} // namespace immutable

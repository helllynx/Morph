#include "base/immutable/map.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstddef>
#include <functional>

template <typename T>
struct collision_hash
{
    std::size_t operator()(const T&) const
    {
        return 0;
    }
};

//
// Immutable detail tests.
//

TEST(ImmutableDetail, make_array)
{
    int  a[7] = {1, 3, 2, 8, 11, 17, 21};
    int* b    = immutable::detail::make_array<int>(a, 7);

    for (auto i = 0; i < 7; ++i)
    {
        ASSERT_EQ(a[i], b[i]);
    }
}

TEST(ImmutableDetail, make_array_insert_begin)
{
    int  a[7] = {1, 3, 2, 8, 11, 17, 21};
    int* b    = immutable::detail::make_array_insert<int>(a, 7, 0, 666);

    ASSERT_EQ(666, b[0]);
    for (auto i = 1; i < 8; ++i)
    {
        ASSERT_EQ(a[i - 1], b[i]);
    }
}

TEST(ImmutableDetail, make_array_insert_middle)
{
    int  a[7] = {1, 3, 2, 8, 11, 17, 21};
    int* b    = immutable::detail::make_array_insert<int>(a, 7, 3, 666);

    for (auto i = 0; i < 3; ++i)
    {
        ASSERT_EQ(a[i], b[i]);
    }

    ASSERT_EQ(666, b[3]);

    for (auto i = 4; i < 8; ++i)
    {
        ASSERT_EQ(a[i - 1], b[i]);
    }
}

TEST(ImmutableDetail, make_array_insert_end)
{
    int  a[7] = {1, 3, 2, 8, 11, 17, 21};
    int* b    = immutable::detail::make_array_insert<int>(a, 7, 7, 666);

    for (auto i = 0; i < 7; ++i)
    {
        ASSERT_EQ(a[i], b[i]);
    }

    ASSERT_EQ(666, b[7]);
}

TEST(ImmutableDetail, make_array_replace_begin)
{
    int  a[7] = {1, 3, 2, 8, 11, 17, 21};
    int* b    = immutable::detail::make_array_replace<int>(a, 7, 0, 666);

    ASSERT_EQ(666, b[0]);
    for (auto i = 1; i < 7; ++i)
    {
        ASSERT_EQ(a[i], b[i]);
    }
}

TEST(ImmutableDetail, make_array_replace_middle)
{
    int  a[7] = {1, 3, 2, 8, 11, 17, 21};
    int* b    = immutable::detail::make_array_replace<int>(a, 7, 3, 666);

    for (auto i = 0; i < 7; ++i)
    {
        if (i == 3)
        {
            ASSERT_EQ(666, b[i]);
            continue;
        }

        ASSERT_EQ(a[i], b[i]);
    }
}

TEST(ImmutableDetail, make_array_replace_end)
{
    int  a[7] = {1, 3, 2, 8, 11, 17, 21};
    int* b    = immutable::detail::make_array_replace<int>(a, 7, 6, 666);

    for (auto i = 0; i < 6; ++i)
    {
        ASSERT_EQ(a[i], b[i]);
    }

    ASSERT_EQ(666, b[6]);
}

TEST(ImmutableDetail, make_array_erase_begin)
{
    int  a[7] = {1, 3, 2, 8, 11, 17, 21};
    int* b    = immutable::detail::make_array_erase<int>(a, 7, 0);

    for (auto i = 0; i < 6; ++i)
    {
        ASSERT_EQ(a[i + 1], b[i]);
    }
}

TEST(ImmutableDetail, make_array_erase_middle)
{
    int  a[7] = {1, 3, 2, 8, 11, 17, 21};
    int* b    = immutable::detail::make_array_erase<int>(a, 7, 3);

    for (auto i = 0; i < 3; ++i)
    {
        ASSERT_EQ(a[i], b[i]);
    }

    for (auto i = 3; i < 6; ++i)
    {
        ASSERT_EQ(a[i + 1], b[i]);
    }
}

TEST(ImmutableDetail, make_array_erase_end)
{
    int  a[7] = {1, 3, 2, 8, 11, 17, 21};
    int* b    = immutable::detail::make_array_erase<int>(a, 7, 6);

    for (auto i = 0; i < 6; ++i)
    {
        ASSERT_EQ(a[i], b[i]);
    }
}

//
// Immutable map tests.
//

TEST(ImmutableMap, basic_set)
{
    using Map = immutable::Map<int, int>;
    Map m;

    std::function<Map(int)> set = [m](int value) {
        return m.set(value, value);
    };

    for (int i = 0; i < 64 * 64; ++i)
    {
        auto new_map = set(i);

        for (int j = 0; j < i; ++j)
        {
            EXPECT_EQ(new_map.get(j), j);
        }

        set = [new_map](int value) { return new_map.set(value, value); };
    }
}

TEST(ImmutableMap, collision_set)
{
    using Map = immutable::Map<int, int, collision_hash<int>>;
    Map m;

    std::function<Map(int)> set = [m](int value) {
        return m.set(value, value);
    };

    for (int i = 0; i < 64; ++i)
    {
        auto new_map = set(i);

        for (int j = 0; j < i; ++j)
        {
            EXPECT_EQ(new_map.get(j), j);
        }

        set = [new_map](int value) { return new_map.set(value, value); };
    }
}

TEST(ImmutableMap, basic_erase)
{
    using Map = immutable::Map<int, int>;
    Map m;

    for (int i = 0; i < 64 * 16; ++i)
    {
        m = m.set(i, i);
    }

    for (int i = 64 * 16 - 1; i >= 0; --i)
    {
        m = m.erase(i);

        for (int j = 0; j < i; ++j)
        {
            EXPECT_EQ(m.get(j), j);
        }

        for (int j = i; j < 64 * 16; ++j)
        {
            EXPECT_THROW(m.get(j), std::out_of_range);
        }
    }
}

TEST(ImmutableMap, collision_erase)
{
    using Map = immutable::Map<int, int, collision_hash<int>>;
    Map m;

    for (int i = 0; i < 64; ++i)
    {
        m = m.set(i, i);
    }

    for (int i = 64 - 1; i >= 0; --i)
    {
        m = m.erase(i);

        for (int j = 0; j < i; ++j)
        {
            EXPECT_EQ(m.get(j), j);
        }

        for (int j = i; j < 64; ++j)
        {
            EXPECT_THROW(m.get(j), std::out_of_range);
        }
    }
}

TEST(ImmutableMap, ranged_for_iteration)
{
    using Map = immutable::Map<int, int>;
    Map m;

    for (int i = 0; i < 64 * 64; ++i)
    {
        m = m.set(i, i);
    }

    int counter = 0;
    
    for (auto p : m)
    {
        static_cast<void>(p);
        ++counter;    
    }

    ASSERT_EQ(counter, 64 * 64);
}

TEST(ImmutableMap, ranged_for_iteration_empty_map)
{
    using Map = immutable::Map<int, int>;
    Map m;

    int counter = 0;
    
    for (auto p : m)
    {
        static_cast<void>(p);
        ++counter;    
    }

    ASSERT_EQ(counter, 0);
}

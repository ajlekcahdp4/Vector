#include <gtest/gtest.h>
#include "vector.hpp"

using namespace Container;

TEST(Vector, Ctors)
{
    Vector<int> vec1 (5);
    EXPECT_EQ(vec1.size(), 0);
    EXPECT_EQ(vec1.capacity(), 5);

    Vector<int> vec2 {1, 2, 3, 4, 5};
    EXPECT_EQ(vec2.size(), 5);
    EXPECT_EQ(vec2.capacity(), 5);

    std::array<int, 5> arr {1, 2, 3, 4, 5};
    Vector<int> vec3 (arr.cbegin(), arr.cend());
    EXPECT_EQ(vec3.size(), 5);
    EXPECT_EQ(vec3.capacity(), 5);
}

TEST(Vector, push_pop_front_back)
{
    Vector<int> vec (5);
    vec.push_back(0);
    EXPECT_EQ(vec.size(), 1);
    vec.push_back(1);
    EXPECT_EQ(vec.size(), 2);

    for (auto i = 2; i < 10; i++)
        vec.push_back(i);
    
    EXPECT_EQ(vec.size(), 10);
    EXPECT_LT(vec.size(), vec.capacity());

    for (auto i = 0; i < vec.size(); i++)
        EXPECT_EQ(vec[i], i);

    EXPECT_EQ(vec.front(), 0);
    EXPECT_EQ(vec.back(), 9);

    for (auto i = 0; i < vec.size(); i++)
    {
        EXPECT_EQ(vec.back(), 9 - i);
        vec.pop_back();
    }
}

TEST(Vector, iterators)
{
    static_assert(std::random_access_iterator<Vector<int>::Iterator>);
    static_assert(std::random_access_iterator<Vector<int>::ConstIterator>);
    
    Vector<int> vec {0, 1, 2, 3, 4, 5};
    auto x = 0;
    for (auto& elem: vec)
        EXPECT_EQ(elem, x++);

    auto itr = vec.begin();

    for (auto i = 0; i < vec.size(); i++)
        EXPECT_EQ(itr[i], i);

    EXPECT_EQ(*(itr + 1), 1);
    EXPECT_EQ(*itr++, 0);
    EXPECT_EQ(*itr, 1);
    EXPECT_EQ(*++itr, 2);
    EXPECT_EQ(*--itr, 1);
    EXPECT_EQ(*itr--, 1);
    EXPECT_EQ(*itr, 0);
    EXPECT_EQ(*(itr += 2), 2);
    EXPECT_EQ(*(itr - 1), 1);
    EXPECT_EQ(*(itr -= 2), 0);
    EXPECT_EQ(std::distance(vec.begin(), vec.end()), 6);

    auto citr = vec.cbegin();

    for (auto i = 0; i < vec.size(); i++)
        EXPECT_EQ(citr[i], i);

    EXPECT_EQ(*(citr + 1), 1);
    EXPECT_EQ(*citr++, 0);
    EXPECT_EQ(*citr, 1);
    EXPECT_EQ(*++citr, 2);
    EXPECT_EQ(*--citr, 1);
    EXPECT_EQ(*citr--, 1);
    EXPECT_EQ(*citr, 0);
    EXPECT_EQ(*(citr += 2), 2);
    EXPECT_EQ(*(citr - 1), 1);
    EXPECT_EQ(*(citr -= 2), 0);
    EXPECT_EQ(std::distance(vec.cbegin(), vec.cend()), 6);
}

TEST(Vector, mem_work)
{
    Vector<int> vec (10);
    for (auto i = 0; i < 4; i++)
        vec.push_back(i);

    vec.shrink_to_fit();
    EXPECT_EQ(vec.capacity(), 4);
    EXPECT_EQ(vec.size(), 4);

    vec.resize(10);
    EXPECT_EQ(vec.size(), 10);
    EXPECT_LE(vec.size(), vec.capacity());
    for (auto i = 0; i < 4; i++)
        EXPECT_EQ(vec[i], i);
    for (auto i = 4; i < vec.size(); i++)
        EXPECT_EQ(vec[4], 0);

    vec.resize(2);
    EXPECT_EQ(vec.size(), 2);
    EXPECT_LE(vec.size(), vec.capacity());
    for (auto i = 0; i < vec.size(); i++)
        EXPECT_EQ(vec[i], i);

    vec.resize(15, 3);
    EXPECT_EQ(vec.size(), 15);
    EXPECT_LE(vec.size(), vec.capacity());
    for (auto i = 2; i < vec.size(); i++)
        EXPECT_EQ(vec[i], 3);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

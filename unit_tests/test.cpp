#include <gtest/gtest.h>
#include <vector>
#include "vector.hpp"

template<typename T>
bool vec_cmp(const Container::Vector<T>& myvec, const std::vector<T>& stdvec)
{
    return !std::lexicographical_compare(myvec.begin(), myvec.end(), stdvec.begin(), stdvec.end())
    && !12   std::lexicographical_compare(stdvec.begin(), stdvec.end(), myvec.begin(), myvec.end());
}

struct Throwable
{
    static inline int a;
    Throwable()
    {
        a++;
        if (a % 50 == 0)
            throw std::exception{};
    }   

    Throwable(const Throwable&): Throwable() {}
    Throwable(Throwable&&): Throwable() {}

    Throwable& operator=(const Throwable&) = default;
    Throwable& operator=(Throwable&&) = default;
    ~Throwable() = default;
};

TEST(Vector, Ctors)
{
    Container::Vector<int> vec0 {};
    EXPECT_EQ(vec0.size(), 0);
    EXPECT_EQ(vec0.capacity(), 0);

    Container::Vector<int> vec1 (42);
    EXPECT_EQ(vec1.size(), 42);
    EXPECT_EQ(vec1.capacity(), 42);

    Container::Vector<int> vec2 (42, 5);
    EXPECT_EQ(vec2.size(), 42);
    EXPECT_EQ(vec2.capacity(), 42);
    EXPECT_TRUE(vec_cmp(vec2, std::vector<int>(42, 5)));

    Container::Vector<std::unique_ptr<int>> vec3 (42);
    EXPECT_EQ(vec3.size(), 42);
    EXPECT_EQ(vec3.capacity(), 42);

    std::vector<int> example {1, 2, 3, 4, 5, 6, 7, 8, 9};
    Container::Vector<int> vec4 (example.begin(), example.end());
    EXPECT_TRUE(vec_cmp(vec4, example));

    Container::Vector<int> vec5 {1, 2, 3, 4, 5, 6, 7, 8, 9};
    EXPECT_TRUE(vec_cmp(vec5, example));

    Throwable::a = 0;

    EXPECT_NO_THROW(Container::Vector<Throwable> vec6 (49););
    Throwable::a = 0;
    EXPECT_ANY_THROW(Container::Vector<Throwable> vec7 (51););
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
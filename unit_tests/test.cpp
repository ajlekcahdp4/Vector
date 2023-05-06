#include <gtest/gtest.h>
#include <vector>
#include "vector.hpp"

template<typename T>
bool vec_cmp(const Container::Vector<T>& myvec, const std::vector<T>& stdvec)
{
    return std::lexicographical_compare_three_way(myvec.begin(), myvec.end(), stdvec.begin(), stdvec.end()) == 0;
}

template<typename T>
bool vec_cmp(const Container::Vector<T>& vec1, const Container::Vector<T>& vec2)
{
    return std::lexicographical_compare_three_way(vec1.begin(), vec1.end(), vec2.begin(), vec2.end()) == 0;
}


struct Throwable
{
    static inline bool throw_on;
    static inline int a;
    std::vector<int> vec {};

    Throwable()
    {
        if (a > 0 && a % 50 == 0 && throw_on)
            throw std::exception{};
        a++;
        vec = std::vector<int>(7, 16);
    }  

    Throwable(const Throwable&): Throwable() {}
    Throwable(Throwable&&): Throwable() {}

    Throwable& operator=(const Throwable&) = default;
    Throwable& operator=(Throwable&&) = default;
    ~Throwable() {a--;}
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
    Throwable::throw_on = true;

    EXPECT_NO_THROW(Container::Vector<Throwable> vec6 (20););
    EXPECT_ANY_THROW(Container::Vector<Throwable> vec7 (51););
}

TEST(Vector, BigFive)
{
    Container::Vector<int> example {1, 2, 3, 4, 5, 6, 7, 8};
    
    Container::Vector<int> vec1 (example);
    EXPECT_TRUE(vec_cmp(vec1, example));

    Container::Vector<int> vec2 {1, 2, 3, 4};
    vec2 = example;
    EXPECT_TRUE(vec_cmp(vec2, example));

    Container::Vector<int> vec3 (std::move(vec1));
    EXPECT_TRUE(vec_cmp(vec3, example));

    Container::Vector<int> vec4 (16, 42);
    vec4 = std::move(vec2);
    EXPECT_TRUE(vec_cmp(vec4, example));
}

TEST(Vector, BigFiveExceptions)
{
    Throwable::a = 0;
    
    if (true){
    Throwable::throw_on = false;

    Container::Vector<Throwable> example1(69);
    Container::Vector<Throwable> example2(69);
    Container::Vector<Throwable> example3(69);
    Container::Vector<Throwable> example4(69);

    Throwable::throw_on = true;

    auto act_a = Throwable::a;
    EXPECT_ANY_THROW(Container::Vector vec1 (example1););
    EXPECT_EQ(Throwable::a, act_a);

    Throwable::throw_on = false;
    Container::Vector<Throwable> vec2 (42);
    Throwable::throw_on = true;

    act_a = Throwable::a;
    EXPECT_ANY_THROW(vec2 = example2;);
    EXPECT_EQ(Throwable::a, act_a);
    EXPECT_EQ(vec2.size(), 42);

    EXPECT_NO_THROW(Container::Vector vec3 (std::move(example3)););

    Throwable::throw_on = false;
    Container::Vector<Throwable> vec4 (42);
    Throwable::throw_on = true;

    EXPECT_NO_THROW(vec4 = std::move(example4););
    EXPECT_EQ(vec4.size(), 69);
    }
    
    EXPECT_EQ(Throwable::a, 0);
}

TEST(Vector, reserve)
{
    Container::Vector<int> vec {};
    EXPECT_EQ(vec.size(), 0);
    EXPECT_EQ(vec.capacity(), 0);

    vec.reserve(10);
    EXPECT_EQ(vec.size(), 0);
    EXPECT_EQ(vec.capacity(), 10);

    vec.reserve(5);
    EXPECT_EQ(vec.size(), 0);
    EXPECT_EQ(vec.capacity(), 10);

    for (auto i = 0; i < 10; i++)
        vec.push_back(i);

    EXPECT_EQ(vec.size(), 10);
    EXPECT_EQ(vec.capacity(), 10);

    vec.reserve(20);
    for (auto i = 0; i < 10; i++)
        EXPECT_EQ(vec[i], i);
    EXPECT_EQ(vec.size(), 10);
    EXPECT_EQ(vec.capacity(), 20);

    Container::Vector<std::unique_ptr<int>> vec_ptr {};

    vec_ptr.reserve(10);
    EXPECT_EQ(vec_ptr.size(), 0);
    EXPECT_EQ(vec_ptr.capacity(), 10);

    for (auto i = 0; i < 10; i++)
        vec_ptr.push_back(std::make_unique<int>(i));
    EXPECT_EQ(vec_ptr.size(), 10);
    EXPECT_EQ(vec_ptr.capacity(), 10);

    vec_ptr.reserve(20);
    EXPECT_EQ(vec_ptr.size(), 10);
    EXPECT_EQ(vec_ptr.capacity(), 20);
    for (auto i = 0; i < 10; i++)
        EXPECT_EQ(*vec_ptr[i], i);
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
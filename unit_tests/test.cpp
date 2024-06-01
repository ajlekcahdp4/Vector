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
    Throwable(Throwable&& rhs): Throwable() {rhs.vec.clear();}

    Throwable& operator=(const Throwable&) = default;
    Throwable& operator=(Throwable&&) = default;
    ~Throwable() {a--;}
};

TEST(Vector, constructors)
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

    Container::Vector<int> vec3 {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    EXPECT_EQ(vec3.size(), 10);
    EXPECT_EQ(vec3.capacity(), 10);
    for (int i = 0; i < 10; i++)
        EXPECT_EQ(vec3[i], i);

    std::vector<int> svec {1, 2, 3, 4, 5, 6, 7, 8, 9};
    Container::Vector<int> vec4 (svec.begin(), svec.end());
    EXPECT_TRUE(vec_cmp(vec4, svec));
}

TEST(Vector, constructorsUnique)
{
    Container::Vector<std::unique_ptr<int>> vec0 {};
    EXPECT_EQ(vec0.size(), 0);
    EXPECT_EQ(vec0.capacity(), 0);

    Container::Vector<std::unique_ptr<int>> vec1 (42);
    EXPECT_EQ(vec1.size(), 42);
    EXPECT_EQ(vec1.capacity(), 42);
}

TEST(Vector, constructorsExceptions)
{
    Throwable::a = 0;
    if (true) {
    Throwable::throw_on = true;
    EXPECT_NO_THROW(Container::Vector<Throwable> vec {};);
    EXPECT_ANY_THROW(Container::Vector<Throwable> vec (1000););
    Throwable::throw_on = false;
    std::vector<Throwable> svec (1000);
    Throwable::throw_on = true;
    EXPECT_ANY_THROW(Container::Vector<Throwable> vec (svec.begin(), svec.end()););
    }
    EXPECT_EQ(Throwable::a, 0);
}

TEST(Vector, push_back)
{
    Container::Vector<int> vec {1, 2, 3, 4};
    vec.push_back(5);
    EXPECT_EQ(vec.back(), 5);
    EXPECT_EQ(vec[4], 5);
    auto cap_before = vec.capacity();
    for (int i = 6; i < 100; i++)
        vec.push_back(i);

    EXPECT_GE(vec.capacity(), vec.size());
    EXPECT_GE(vec.capacity(), cap_before);

    vec.reserve(1000);
    for (int i = 100; i < 1000; i++)
        EXPECT_EQ(1000, (vec.push_back(i), vec.capacity()));
}

TEST(Vector, push_backUnique)
{
    Container::Vector<std::unique_ptr<int>> vec {};
    vec.push_back(std::make_unique<int>(5));
    EXPECT_EQ(*vec.back(), 5);
    EXPECT_EQ(vec.size(), 1);

    for (int i = 1; i < 100; i++)
        vec.push_back(std::make_unique<int>(i));
    EXPECT_EQ(vec.size(), 100);
    EXPECT_LT(vec.size(), vec.capacity());
}

TEST(Vector, bigFive)
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

TEST(Vector, bigFiveUnique)
{
    Container::Vector<std::unique_ptr<int>> vec {};
    for (int i = 0; i < 42; i++)
        vec.push_back(std::make_unique<int>(i));

    Container::Vector vec1 (std::move(vec));
    for (int i = 0; i < 42; i++)
        EXPECT_EQ(*vec1[i], i);

    Container::Vector<std::unique_ptr<int>> vec2 {};
    vec2.push_back(std::make_unique<int>(79));
    vec2 = std::move(vec1);
    for (int i = 0; i < 42; i++)
        EXPECT_EQ(*vec2[i], i);
}

TEST(Vector, bigFiveExceptions)
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
}

TEST(Vector, reserveUnique)
{
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

TEST(Vector, reserveExceptions)
{
    Throwable::a = 0;
    if (true) {
    Throwable::throw_on = false;
    Container::Vector<Throwable> vec (69);
    Throwable::throw_on = true;
    
    vec.reserve(20);
    EXPECT_EQ(vec.size(), 69);
    EXPECT_EQ(vec.capacity(), 69);

    vec.reserve(69);
    EXPECT_EQ(vec.size(), 69);
    EXPECT_EQ(vec.capacity(), 69);

    EXPECT_ANY_THROW(vec.reserve(80););
    EXPECT_EQ(vec.size(), 69);
    EXPECT_EQ(vec.capacity(), 69);
    }
    EXPECT_EQ(Throwable::a, 0);
}

TEST(Vector, resize)
{
    Container::Vector<int> vec (42, 42);
    
    auto data_before = vec.data();
    vec.resize(10);
    EXPECT_EQ(data_before, vec.data());
    EXPECT_EQ(vec.size(), 10);
    EXPECT_EQ(vec.capacity(), 42);
    for (auto i = 0; i < 10; i++)
        EXPECT_EQ(vec[i], 42);

    data_before = vec.data();
    vec.resize(42);
    EXPECT_EQ(data_before, vec.data());
    EXPECT_EQ(vec.size(), 42);
    EXPECT_EQ(vec.capacity(), 42);
    for (auto i = 0; i < 10; i++)
        EXPECT_EQ(vec[i], 42);
    for (auto i = 10; i < 42; i++)
        EXPECT_EQ(vec[i], 0);

    vec.resize(10);
    data_before = vec.data();
    vec.resize(42, 42);
    EXPECT_EQ(data_before, vec.data());
    EXPECT_EQ(vec.size(), 42);
    EXPECT_EQ(vec.capacity(), 42);
    for (auto i = 0; i < 42; i++)
        EXPECT_EQ(vec[i], 42);

    vec.resize(80);
    EXPECT_EQ(vec.size(), 80);
    EXPECT_EQ(vec.capacity(), 80);
    for (auto i = 0; i < 42; i++)
        EXPECT_EQ(vec[i], 42);
    for (auto i = 42; i < 80; i++)
        EXPECT_EQ(vec[i], 0);

    vec.resize(100, 42);
    EXPECT_EQ(vec.size(), 100);
    EXPECT_EQ(vec.capacity(), 100);
    for (auto i = 0; i < 42; i++)
        EXPECT_EQ(vec[i], 42);
    for (auto i = 42; i < 80; i++)
        EXPECT_EQ(vec[i], 0);
    for (auto i = 80; i < 100; i++)
        EXPECT_EQ(vec[i], 42);
}

TEST(Vector, resizeUnique)
{
    Container::Vector<std::unique_ptr<int>> vec_ptr {};
    for (auto i = 0; i < 42; i++)
        vec_ptr.push_back(std::make_unique<int>(42));

    vec_ptr.resize(420);
    EXPECT_EQ(vec_ptr.size(), 420);
    for (auto i = 0; i < 42; i++)
        EXPECT_EQ(*vec_ptr[i], 42);
    for (auto i = 42; i < 420; i++)
        EXPECT_EQ(vec_ptr[i].get(), nullptr);
}

struct ThrowCopyable
{
    static inline int  a;
    static inline bool throw_on;
    std::vector<int> vec;
    
    ThrowCopyable()
    {
        if (a > 0 && a % 50 == 0 && throw_on)
            throw std::exception{};
        a++;
        vec = std::vector<int>(7, 16);
    }  

    ThrowCopyable(const ThrowCopyable&): ThrowCopyable() {}
    ThrowCopyable(ThrowCopyable&& rhs) noexcept 
    {
        a++;
        vec = std::vector<int>(7, 16);
        rhs.vec.clear();
    }

    ThrowCopyable& operator=(const ThrowCopyable&) = default;
    ThrowCopyable& operator=(ThrowCopyable&&) noexcept = default;
    ~ThrowCopyable() {a--;}
};

TEST(Vector, resizeExceptions)
{
    Throwable::a = 0;
    if (true) {
    Throwable::throw_on = false;
    Container::Vector<Throwable> vec (100);
    Throwable::throw_on = true;

    auto data_before = vec.data();
    EXPECT_NO_THROW(vec.resize(42););
    EXPECT_EQ(vec.data(), data_before);
    EXPECT_EQ(vec.capacity(), 100);
    EXPECT_EQ(vec.size(), 42);

    data_before = vec.data();
    EXPECT_ANY_THROW(vec.resize(99););
    EXPECT_EQ(vec.data(), data_before);
    EXPECT_EQ(vec.size(), 42);
    EXPECT_EQ(vec.capacity(), 100);

    data_before = vec.data();
    EXPECT_ANY_THROW(vec.resize(1000););
    EXPECT_EQ(vec.data(), data_before);
    EXPECT_EQ(vec.size(), 42);
    EXPECT_EQ(vec.capacity(), 100);
    }
    EXPECT_EQ(Throwable::a, 0);

    Throwable::a = 0;
    if (true) {
        Throwable::throw_on = true;
        Container::Vector<Throwable> vec (20);
        auto data_before = vec.data();
        EXPECT_ANY_THROW(vec.resize(40););
        EXPECT_EQ(vec.data(), data_before);
    }
    EXPECT_EQ(Throwable::a, 0);

    ThrowCopyable::a = 0;
    if (true) {
    ThrowCopyable::throw_on = false;
    Container::Vector<ThrowCopyable> vec (100);
    ThrowCopyable::throw_on = true;

    EXPECT_NO_THROW(vec.resize(42););
    EXPECT_EQ(vec.capacity(), 100);
    EXPECT_EQ(vec.size(), 42);

    EXPECT_ANY_THROW(vec.resize(99););
    EXPECT_EQ(vec.size(), 42);
    EXPECT_EQ(vec.capacity(), 100);

    auto data_before = vec.data();
    EXPECT_ANY_THROW(vec.resize(1000););
    EXPECT_EQ(vec.data(), data_before);
    EXPECT_EQ(vec.size(), 42);
    EXPECT_EQ(vec.capacity(), 100);
    }
    EXPECT_EQ(ThrowCopyable::a, 0);
}

TEST(Vector, shrink_to_fit)
{
    Container::Vector<int> vec (42, 42);

    vec.reserve(322);
    vec.shrink_to_fit();
    EXPECT_EQ(vec.size(), 42);
    EXPECT_EQ(vec.capacity(), 42);

    vec.resize(74);
    vec.shrink_to_fit();
    EXPECT_EQ(vec.size(), 74);
    EXPECT_EQ(vec.capacity(), 74);
    
    vec.pop_back();
    EXPECT_EQ(vec.size(), 73);
    EXPECT_EQ(vec.capacity(), 74);

    vec.shrink_to_fit();
    EXPECT_EQ(vec.size(), 73);
    EXPECT_EQ(vec.capacity(), 73);
}

TEST(Vector, shrink_to_fitUnique)
{
    Container::Vector<std::unique_ptr<int>> vec {};
    vec.reserve(420);
    vec.shrink_to_fit();
    EXPECT_EQ(vec.size(), 0);
    EXPECT_EQ(vec.capacity(), 0);

    for (auto i = 0; i < 10; i++)
        vec.push_back(std::make_unique<int>(i));
    EXPECT_LE(vec.size(), vec.capacity());
    vec.shrink_to_fit();
    EXPECT_EQ(vec.size(), 10);
    EXPECT_EQ(vec.capacity(), 10);
}

TEST(Vector, shrink_to_fitExceptions)
{
    Throwable::a = 0;
    if (true) {
    Throwable::throw_on = false;
    Container::Vector<Throwable> vec (100);
    Throwable::throw_on = true;

    vec.resize(60);
    EXPECT_ANY_THROW(vec.shrink_to_fit(););
    EXPECT_EQ(vec.size(), 60);
    EXPECT_EQ(vec.capacity(), 100);
    for (int i = 0; i < 60; i++)
    {
        EXPECT_EQ(vec[i].vec.size(), 7);
        EXPECT_EQ(vec[i].vec[4], 16);
    }

    Throwable::throw_on = false;
    vec.resize(420);
    vec.resize(200);
    Throwable::throw_on = true;

    EXPECT_ANY_THROW(vec.shrink_to_fit());
    EXPECT_EQ(vec.size(), 200);
    EXPECT_EQ(vec.capacity(), 420);
    for (int i = 0; i < 200; i++)
    {
        EXPECT_EQ(vec[i].vec.size(), 7);
        EXPECT_EQ(vec[i].vec[4], 16);
    }
    }
    EXPECT_EQ(Throwable::a, 0);
}

TEST(Vector, iterators)
{
    Container::Vector<int> vec {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    const Container::Vector<int> cvec {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    auto itr = vec.begin();
    EXPECT_EQ(*itr, 0);
    EXPECT_EQ(*(++itr), 1);
    EXPECT_EQ(*itr, 1);
    EXPECT_EQ(*(itr++), 1);
    EXPECT_EQ(*itr, 2);
    EXPECT_EQ(itr[-1], 1);
    EXPECT_EQ(itr[0], 2);
    EXPECT_EQ(itr[1], 3);
    EXPECT_EQ(*(itr += 1), 3);
    EXPECT_EQ(*(itr -= 1), 2);
    EXPECT_EQ(*(itr - 1), 1);
    EXPECT_EQ(*(itr + 1), 3);
    EXPECT_EQ(std::distance(vec.begin(), vec.end()), vec.size());

    auto citr = vec.cbegin();
    EXPECT_EQ(*citr, 0);
    EXPECT_EQ(*(++citr), 1);
    EXPECT_EQ(*citr, 1);
    EXPECT_EQ(*(citr++), 1);
    EXPECT_EQ(*citr, 2);
    EXPECT_EQ(citr[-1], 1);
    EXPECT_EQ(citr[0], 2);
    EXPECT_EQ(citr[1], 3);
    EXPECT_EQ(*(citr += 1), 3);
    EXPECT_EQ(*(citr -= 1), 2);
    EXPECT_EQ(*(citr - 1), 1);
    EXPECT_EQ(*(citr + 1), 3);
    EXPECT_EQ(std::distance(vec.cbegin(), vec.cend()), vec.size());

    auto ccitr = cvec.begin();
    EXPECT_EQ(*ccitr, 0);
    EXPECT_EQ(*(++ccitr), 1);
    EXPECT_EQ(*ccitr, 1);
    EXPECT_EQ(*(ccitr++), 1);
    EXPECT_EQ(*ccitr, 2);
    EXPECT_EQ(ccitr[-1], 1);
    EXPECT_EQ(ccitr[0], 2);
    EXPECT_EQ(ccitr[1], 3);
    EXPECT_EQ(*(ccitr += 1), 3);
    EXPECT_EQ(*(ccitr -= 1), 2);
    EXPECT_EQ(*(ccitr - 1), 1);
    EXPECT_EQ(*(ccitr + 1), 3);
    EXPECT_EQ(std::distance(cvec.begin(), cvec.end()), cvec.size());
}

TEST(Vector, reverse_iterators)
{
    Container::Vector<int> vec {10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    const Container::Vector<int> cvec {10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};

    auto itr = vec.rbegin();
    EXPECT_EQ(*itr, 0);
    EXPECT_EQ(*(++itr), 1);
    EXPECT_EQ(*itr, 1);
    EXPECT_EQ(*(itr++), 1);
    EXPECT_EQ(*itr, 2);
    EXPECT_EQ(itr[-1], 1);
    EXPECT_EQ(itr[0], 2);
    EXPECT_EQ(itr[1], 3);
    EXPECT_EQ(*(itr += 1), 3);
    EXPECT_EQ(*(itr -= 1), 2);
    EXPECT_EQ(*(itr - 1), 1);
    EXPECT_EQ(*(itr + 1), 3);
    EXPECT_EQ(std::distance(vec.rbegin(), vec.rend()), vec.size());

    auto citr = vec.crbegin();
    EXPECT_EQ(*citr, 0);
    EXPECT_EQ(*(++citr), 1);
    EXPECT_EQ(*citr, 1);
    EXPECT_EQ(*(citr++), 1);
    EXPECT_EQ(*citr, 2);
    EXPECT_EQ(citr[-1], 1);
    EXPECT_EQ(citr[0], 2);
    EXPECT_EQ(citr[1], 3);
    EXPECT_EQ(*(citr += 1), 3);
    EXPECT_EQ(*(citr -= 1), 2);
    EXPECT_EQ(*(citr - 1), 1);
    EXPECT_EQ(*(citr + 1), 3);
    EXPECT_EQ(std::distance(vec.crbegin(), vec.crend()), vec.size());

    auto ccitr = cvec.rbegin();
    EXPECT_EQ(*ccitr, 0);
    EXPECT_EQ(*(++ccitr), 1);
    EXPECT_EQ(*ccitr, 1);
    EXPECT_EQ(*(ccitr++), 1);
    EXPECT_EQ(*ccitr, 2);
    EXPECT_EQ(ccitr[-1], 1);
    EXPECT_EQ(ccitr[0], 2);
    EXPECT_EQ(ccitr[1], 3);
    EXPECT_EQ(*(ccitr += 1), 3);
    EXPECT_EQ(*(ccitr -= 1), 2);
    EXPECT_EQ(*(ccitr - 1), 1);
    EXPECT_EQ(*(ccitr + 1), 3);
    EXPECT_EQ(std::distance(cvec.rbegin(), cvec.rend()), cvec.size());
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

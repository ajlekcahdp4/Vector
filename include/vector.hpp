#pragma once
#include "contiguous_iterator.hpp"
#include <initializer_list>

namespace Container
{

namespace detail
{

template<typename T>
void construct(T* p, const T& rhs) {new (p) T{rhs};}

template<typename T>
void construct(T* p, T&& rhs) {new (p) T{std::move(rhs)};}

template<class T>
void destroy(T* p) {p->~T();}

template<std::forward_iterator It>
void destroy(It first, It last)
{
    while (first != last)
        destroy(&*first++);
}

template<typename T>
class VectorBuf
{
    using value_type = T;
    using pointer    = T*;
    using size_type  = typename std::size_t;
protected:
    size_type size_, used_ = 0;
    pointer data_ = nullptr;
protected:
    VectorBuf(size_type size = 0)
    :size_ {size},  
     data_ {(size_ == 0) ? nullptr : static_cast<pointer>(::operator new(sizeof(value_type) * size_))}
    {}

    VectorBuf(const VectorBuf&)            = delete;
    VectorBuf& operator=(const VectorBuf&) = delete;

    void swap(VectorBuf& rhs) noexcept
    {
        std::swap(size_, rhs.size_);
        std::swap(used_, rhs.used_);
        std::swap(data_, rhs.data_);
    }

    VectorBuf(VectorBuf&& rhs) noexcept
    {
        swap(rhs);
    }

    VectorBuf& operator=(VectorBuf&& rhs) noexcept
    {
        swap(rhs);
        return *this;
    }

    virtual ~VectorBuf()
    {
        ::operator delete(data_);
    }
};

} // namespace detail

template<typename T>
class Vector final: private detail::VectorBuf<T>
{
    using value_type      = T;
    using pointer         = T*;
    using reference       = T&;
    using const_reference = const T&;
    using size_type       = typename std::size_t;
    using base            = detail::VectorBuf<T>;

    using base::size_;
    using base::used_;
    using base::data_;
public:
    Vector(size_type size = 0): base::VectorBuf(size) {}

    template<std::input_iterator InpIt>
    Vector(InpIt first, InpIt last): base::VectorBuf(std::distance(first, last))
    {
        while (first != last)
            detail::construct(data_ + used_++, *first++);   
    }

    Vector(std::initializer_list<T> initlist)
    :Vector(initlist.begin(), initlist.end())
    {}

public:
    Vector(Vector&&)            = default;
    Vector& operator=(Vector&&) = default;

    Vector(const Vector& rhs): base::VectorBuf(rhs.used_)
    {
        for (size_type i = 0; i < used_; i++)
            detail::construct(data_ + i, rhs.data[i]);
    }

    Vector& operator=(const Vector& rhs)
    {
        auto cpy {rhs};
        std::swap(*this, cpy);
        return *this;
    }

    ~Vector()
    {
        detail::destroy(data_, data_ + used_);
    }

public:
    size_type size() const {return used_;}
    size_type capacity() const {return size_;}
    bool empty() const {return used_ == 0;}

    reference       operator[](size_type index) &      noexcept {return data_[index];}
    const_reference operator[](size_type index) const& noexcept {return data_[index];}
    value_type      operator[](size_type index) &&     noexcept {return data_[index];}

    reference at(size_type index) &  
    {
        if(index >= used_)
            throw std::out_of_range{"try to get acces to element out of array"};
        return data_[index];
    }
    const_reference at(size_type index) const&
    {
        if(index >= used_)
            throw std::out_of_range{"try to get acces to element out of array"};
        return data_[index];
    }
    value_type at(size_type index) &&          
    {
        if(index >= used_)
            throw std::out_of_range{"try to get acces to element out of array"};
        return data_[index];
    }

public:
    void push_back(const value_type& val)
    {
        auto cpy {val};
        push_back(std::move(cpy));
    }

    void push_back(value_type&& val)
    {
        if (need_resize_up())
            resize_up(2 * size_ + 1);
        
        detail::construct(data_ + used_++, std::move(val));
    }
private:
    bool need_resize_up() const {return (used_ == size_);}

    void resize_up(size_type newsz)
    {
        Vector tmp (newsz);
        while (tmp.used_ < used_)
            tmp.push_back(std::move(data_[tmp.used_]));
        std::swap(*this, tmp);
    }
public:
    value_type back() const
    {
        if (empty())
            throw std::underflow_error{"try to get back from empty vector"};
        return data_[used_ - 1];
    }

    void pop_back()
    {
        if(empty())
            throw std::underflow_error{"try to pop element from empty vector"};
        used_--;
        detail::destroy(data_ + used_);
    }

    value_type front()
    {
        if (empty())
            throw std::underflow_error{"try to get back from empty vector"};
        return data_[0];
    }

    void shrink_to_fit()
    {
        assert(used_ <= size_);

        if (used_ == size_)
            return;

        Vector tmp (used_);
        while (tmp.used_ < used_)
            tmp.push_back(std::move(data_[tmp.used_]));
        std::swap(*this, tmp);
    }

    void clear()
    {
        detail::destroy(data_, data_ + used_);
        used_ = 0;
    }

    void resize(size_type newsz)
    {
        if (newsz < used_)
            while (used_ != newsz)
                pop_back();
        else
            while (used_ != newsz)
                push_back(value_type{});
    }

    void resize(size_type newsz, const value_type& value)
    {
        if (newsz < used_)
            while (used_ != newsz)
                pop_back();
        else
            while (used_ != newsz)
                push_back(value);
    }

    using Iterator      = typename detail::ContiguousIterator<value_type>;
    using ConstIterator = typename detail::ContiguousIterator<const value_type>;
    
    Iterator begin() {return Iterator{data_};}
    Iterator end()   {return Iterator{data_ + used_};}

    ConstIterator begin() const {return ConstIterator{data_};}
    ConstIterator end()   const {return ConstIterator{data_ + used_};}

    ConstIterator cbegin() const {return ConstIterator{data_};}
    ConstIterator cend()   const {return ConstIterator{data_ + used_};}
}; // class Vector

} // namespace Container
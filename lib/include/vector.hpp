#pragma once
#include <initializer_list>
#include "my_ranges.hpp"
#include <iterator>

namespace Container
{

namespace detail
{

template<typename P>
struct iterator
{
    using iterator_category = std::random_access_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = std::remove_pointer_t<P>;
    using reference         = value_type&;
    using pointer           = P;

private:
    pointer ptr_;

public:
    iterator(pointer ptr = nullptr): ptr_ {ptr} {}

    reference operator*() const {return *ptr_;}
    pointer operator->() const {return ptr_;}

    iterator& operator++()
    {
        ptr_++;
        return *this;
    }
    iterator operator++(int)
    {
        iterator tmp (*this);
        ++(*this);
        return tmp;
    }
    iterator& operator--()
    {
        ptr_--;
        return *this;
    }
    iterator operator--(int)
    {
        iterator tmp (*this);
        --(*this);
        return tmp;
    }

    iterator& operator+=(const difference_type& diff)
    {
        ptr_ += diff;
        return *this;
    }
    iterator& operator-=(const difference_type& diff)
    {
        ptr_ -= diff;
        return *this;
    }

    difference_type operator-(const iterator& itr) const
    {
        return ptr_ - itr.ptr_;
    }

    reference operator[](const difference_type& diff) const
    {
        return ptr_[diff];
    }

    auto operator<=>(const iterator& other) const = default;
};

template<typename P>
iterator<P> operator+(const iterator<P>& itr, const typename iterator<P>::difference_type& diff)
{
    iterator itr_cpy (itr);
    itr_cpy += diff;
    return itr_cpy;
}

template<typename P>
iterator<P> operator+(const typename iterator<P>::difference_type& diff, const iterator<P>& itr)
{
    return itr + diff;
}

template<typename P>
iterator<P> operator-(const iterator<P>& itr, const typename iterator<P>::difference_type& diff)
{
    iterator itr_cpy (itr);
    itr_cpy -= diff;
    return itr_cpy;
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

    ~VectorBuf()
    {
        std::destroy(data_, data_ + used_);
        ::operator delete(data_);
    }
};
} // namespace detail

template<typename T>
class Vector final: private detail::VectorBuf<T>
{
public:
    using value_type      = T;
    using pointer         = T*;
    using const_pointer   = const T*;
    using reference       = T&;
    using const_reference = const T&;
    using size_type       = std::size_t;
    using base            = detail::VectorBuf<T>;

    using iterator       = detail::iterator<pointer>;
    using const_iterator = detail::iterator<const_pointer>;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
private:   
    using base::size_;
    using base::used_;
    using base::data_;
public:
    Vector() = default;

    explicit Vector(size_type size): base(size)
    {
        Ranges::uninitialized_default_construct(data_, data_ + size_);
        used_ = size_;
    }

    Vector(size_type size, const_reference val): base(size)
    {
        std::uninitialized_fill(data_, data_ + size_, val);
        used_ = size_;
    }

    template<std::input_iterator InpIt>
    Vector(InpIt first, InpIt last): base(std::distance(first, last))
    {
        std::uninitialized_copy(first, last, data_);
        used_ = size_;
    }

    Vector(std::initializer_list<T> initlist)
    :Vector(initlist.begin(), initlist.end())
    {}

public:
    Vector(Vector&&)            = default;
    Vector& operator=(Vector&&) = default;

    Vector(const Vector& rhs): base(rhs.used_)
    {
        std::uninitialized_copy(rhs.data_, rhs.data_ + rhs.used_, data_);
        used_ = rhs.used_;
    }

    Vector& operator=(const Vector& rhs)
    {
        auto cpy (rhs);
        std::swap(*this, cpy);
        return *this;
    }

    ~Vector() = default;

public:
    size_type size() const {return used_;}
    size_type capacity() const {return size_;}
    bool empty() const {return used_ == 0;}
    pointer data() {return data_;}
    const_pointer data() const {return data_;}

    reference       operator[](size_type index)       noexcept {return data_[index];}
    const_reference operator[](size_type index) const noexcept {return data_[index];}
    
    reference at(size_type index)  
    {
        if (index >= used_)
            throw std::out_of_range{"try to get acces to element out of array"};
        return data_[index];
    }
    const_reference at(size_type index) const
    {
        if (index >= used_)
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
        if (need_reserve_up())
            reserve(2 * size_ + 1);
        
        std::construct_at(data_ + used_++, std::move(val));
    }

private:
    bool need_reserve_up() const {return (used_ == size_);}

public:
    const_reference back() const
    {
        if (empty())
            throw std::underflow_error{"try to get back from empty vector"};
        return data_[used_ - 1];
    }

    reference back()
    {
        if (empty())
            throw std::underflow_error{"try to get back from empty vector"};
        return data_[used_ - 1];
    }

    const_reference front() const
    {
        if (empty())
            throw std::underflow_error{"try to get back from empty vector"};
        return data_[0];
    }

    reference front()
    {
        if (empty())
            throw std::underflow_error{"try to get back from empty vector"};
        return data_[0];
    }

    void pop_back()
    {
        if(empty())
            throw std::underflow_error{"try to pop element from empty vector"};
        used_--;
        std::destroy_at(data_ + used_);
    }

private:
    struct raw_deleter {void operator()(void* ptr) const {::operator delete(ptr);}};
    using scoped_raw_ptr = std::unique_ptr<value_type, raw_deleter>;

public:
    void reserve(size_type newsz)
    {
        if (size_ >= newsz)
            return;
        
        scoped_raw_ptr new_data_scoped {static_cast<pointer>(::operator new(sizeof(value_type) * newsz))};
        auto new_data = new_data_scoped.get();
        Ranges::strong_guarantee_uninitialized_move_or_copy(data_, data_ + used_, new_data);

        std::destroy(data_, data_ + used_);
        ::operator delete(data_);
        data_ = new_data_scoped.release();
        size_ = newsz;
    }

private:
    template<class Initializer>
    void resize(size_type newsz, Initializer initializer)
    {
        if (newsz <= used_)
            std::destroy(data_ + newsz, data_ + used_);
        else if (newsz > used_ && newsz <= size_)
            initializer(data_ + used_, data_ + newsz);
        else
        {
            scoped_raw_ptr new_data_scoped {static_cast<pointer>(::operator new(sizeof(value_type) * newsz))};
            auto new_data = new_data_scoped.get();
            Ranges::strong_guarantee_uninitialized_move_or_copy(data_, data_ + used_, new_data);
            try 
            {
                initializer(new_data + used_, new_data + newsz);
            }
            catch (...)
            {
                if constexpr ((!std::is_copy_constructible<value_type>::value || 
                std::is_nothrow_move_constructible<value_type>::value) && std::is_nothrow_move_assignable<value_type>::value)
                    std::move(new_data, new_data + used_, data_);
                std::destroy(new_data, new_data + used_);
                throw;
            }
            std::destroy(data_, data_ + used_);
            ::operator delete(data_);
            data_ = new_data_scoped.release();
            size_ = newsz;
        } 
        used_ = newsz;
    }

public:
    void resize(size_type newsz)
    {
        resize(newsz, [](pointer first, pointer last){Ranges::uninitialized_default_construct(first, last);});
    }

    void resize(size_type newsz, const_reference value)
    {
        resize(newsz, [&value](pointer first, pointer last){std::uninitialized_fill(first, last, value);});
    }

    void shrink_to_fit()
    {
        scoped_raw_ptr new_data_scoped {static_cast<pointer>(::operator new(sizeof(value_type) * used_))};
        auto new_data = new_data_scoped.get();
        Ranges::strong_guarantee_uninitialized_move_or_copy(data_, data_ + used_, new_data);

        std::destroy(data_, data_ + used_);
        ::operator delete(data_);
        data_ = new_data_scoped.release();
        size_ = used_;
    }

    void clear()
    {
        std::destroy(data_, data_ + used_);
        used_ = 0;
    }

    iterator begin() {return iterator{data_};}
    iterator end()   {return iterator{data_ + used_};}

    const_iterator begin() const {return const_iterator{data_};}
    const_iterator end()   const {return const_iterator{data_ + used_};}

    const_iterator cbegin() const {return const_iterator{data_};}
    const_iterator cend()   const {return const_iterator{data_ + used_};}

    reverse_iterator rbegin() {return reverse_iterator{data_ + used_};}
    reverse_iterator rend()   {return reverse_iterator{data_};}

    const_reverse_iterator rbegin() const {return const_reverse_iterator{data_ + used_};}
    const_reverse_iterator rend()   const {return const_reverse_iterator{data_};}

    const_reverse_iterator crbegin() const {return const_reverse_iterator{data_ + used_};}
    const_reverse_iterator crend()   const {return const_reverse_iterator{data_};}

}; // class Vector

} // namespace Container
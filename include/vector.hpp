#pragma once
#include <initializer_list>
#include <memory>
#include <iterator>

namespace Container
{

namespace detail
{
//give a strong guarantee if value_type is copy constructible 
template<typename InpIt, typename OutIt>
constexpr OutIt strong_guarantee_uninitialized_move_or_copy(InpIt first, InpIt last, OutIt d_first)
{
    using value_type = typename std::iterator_traits<InpIt>::value_type;
    if constexpr (!std::is_copy_constructible<value_type>::value || std::is_nothrow_move_constructible<value_type>::value)
        return std::uninitialized_move(first, last, d_first);
    else
        return std::uninitialized_copy(first, last, d_first);   
}

template<std::forward_iterator FwdIt>
constexpr void uninitialized_default_construct(FwdIt first, FwdIt last)
{
    using value_type = typename std::iterator_traits<FwdIt>::value_type;
    if constexpr (std::is_copy_constructible<value_type>::value)
        std::uninitialized_fill(first, last, value_type{});
    else
        std::uninitialized_default_construct(first, last);
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
    using size_type       = typename std::size_t;
    using base            = detail::VectorBuf<T>;

    using Iterator      = pointer;
    using ConstIterator = const_pointer;
    using ReverseIterator      = std::reverse_iterator<Iterator>;
    using ConstReverseIterator = std::reverse_iterator<ConstIterator>;
private:   
    using base::size_;
    using base::used_;
    using base::data_;
public:
    Vector() = default;

    explicit Vector(size_type size): base(size)
    {
        detail::uninitialized_default_construct(data_, data_ + size_);
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

    reference       operator[](size_type index) &      noexcept {return data_[index];}
    const_reference operator[](size_type index) const& noexcept {return data_[index];}
    
    reference at(size_type index) &  
    {
        if (index >= used_)
            throw std::out_of_range{"try to get acces to element out of array"};
        return data_[index];
    }
    const_reference at(size_type index) const&
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

    reference front()
    {
        if (empty())
            throw std::underflow_error{"try to get back from empty vector"};
        return data_[0];
    }

    const_reference front() const
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
        detail::strong_guarantee_uninitialized_move_or_copy(data_, data_ + used_, new_data);

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
            detail::strong_guarantee_uninitialized_move_or_copy(data_, data_ + used_, new_data);
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
        resize(newsz, [](pointer first, pointer last){detail::uninitialized_default_construct(first, last);});
    }

    void resize(size_type newsz, const_reference value)
    {
        resize(newsz, [&value](pointer first, pointer last){std::uninitialized_fill(first, last, value);});
    }

    void shrink_to_fit()
    {
        scoped_raw_ptr new_data_scoped {static_cast<pointer>(::operator new(sizeof(value_type) * used_))};
        auto new_data = new_data_scoped.get();
        detail::strong_guarantee_uninitialized_move_or_copy(data_, data_ + used_, new_data);

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

    Iterator begin() {return Iterator{data_};}
    Iterator end()   {return Iterator{data_ + used_};}

    ConstIterator begin() const {return ConstIterator{data_};}
    ConstIterator end()   const {return ConstIterator{data_ + used_};}

    ConstIterator cbegin() const {return ConstIterator{data_};}
    ConstIterator cend()   const {return ConstIterator{data_ + used_};}

    ReverseIterator rbegin() {return ReverseIterator{data_ + used_};}
    ReverseIterator rend()   {return ReverseIterator{data_};}

    ConstReverseIterator rbegin() const {return ConstReverseIterator{data_ + used_};}
    ConstReverseIterator rend()   const {return ConstReverseIterator{data_};}

    ConstReverseIterator crbegin() const {return ConstReverseIterator{data_ + used_};}
    ConstReverseIterator crend()   const {return ConstReverseIterator{data_};}

}; // class Vector

} // namespace Container
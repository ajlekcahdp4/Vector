#pragma once
#include <initializer_list>
#include <memory>

namespace Container
{

namespace detail
{

template<typename T>
void construct(T* p, T&& rhs) {new (p) T{std::forward<T>(rhs)};}

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

    ~VectorBuf()
    {
        detail::destroy(data_, data_ + used_);
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

    using Iterator      = std::iterator_traits<pointer>;
    using ConstIterator = std::iterator_traits<const_pointer>;
    using ReverseIterator      = std::reverse_iterator<Iterator>;
    using ConstReverseIterator = std::reverse_iterator<ConstIterator>;
    

    using base::size_;
    using base::used_;
    using base::data_;
public:
    Vector() = default;

    Vector(size_type size): base(size)
    {
        std::uninitialized_fill(data_, data_ + size_, value_type{});
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
        while (first != last)
            detail::construct(data_ + used_++, *first++);   
    }

    Vector(std::initializer_list<T> initlist)
    :Vector(initlist.begin(), initlist.end())
    {}

public:
    Vector(Vector&&)            = default;
    Vector& operator=(Vector&&) = default;

    Vector(const Vector& rhs): base(rhs.used_)
    {
        for (size_type i = 0; i < used_; i++)
            detail::construct(data_ + i, rhs.data_[i]);
    }

    Vector& operator=(const Vector& rhs)
    {
        auto cpy {rhs};
        std::swap(*this, cpy);
        return *this;
    }

    ~Vector() = default;

public:
    size_type size() const {return used_;}
    size_type capacity() const {return size_;}
    bool empty() const {return used_ == 0;}

    reference       operator[](size_type index) &      noexcept {return data_[index];}
    const_reference operator[](size_type index) const& noexcept {return data_[index];}
    
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
        
        detail::construct(data_ + used_++, std::move(val));
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
        detail::destroy(data_ + used_);
    }

    void clear()
    {
        detail::destroy(data_, data_ + used_);
        used_ = 0;
    }

    void reserve(size_type newsz)
    {
        if (size_ >= newsz)
            return;
        
        auto new_data = static_cast<pointer>(::operator new(sizeof(value_type) * newsz));

        std::uninitialized_move(data_, data_ + used_, new_data);
        ::operator delete(data_);
        data_ = new_data;
        size_ = newsz;
    }

    void resize(size_type newsz)
    {
        if (newsz <= used_)
            detail::destroy(data + newsz, data_ + used_);
        else if (newsz > used_ && newsz <= size_)
            std::uninitialized_fill(data_ + used_, data_ + newsz, value_type{});
        else
        {
            auto new_data = static_cast<pointer>(::operator new(sizeof(value_type) * newsz));
            std::uninitialized_move(data_, data_ + used_, new_data);
            try 
            {
                std::uninitialized_fill(new_data + used_, new_data + newsz, value_type{});
            }
            catch(...)
            {
                std::move(new_data, new_data + used_, data_);
                ::operator delete(new_data);
                throw;
            }
            ::operator delete(data_);
            data_ = new_data;
            size_ = newsz;
        } 
        used_ = newsz;
    }

    void resize(size_type newsz, const_reference value)
    {
        if (newsz <= used_)
            detail::destroy(data + newsz, data_ + used_);
        else if (newsz > used_ && newsz <= size_)
            std::uninitialized_fill(data_ + used_, data_ + newsz, value);
        else
        {
            auto new_data = static_cast<pointer>(::operator new(sizeof(value_type) * newsz));
            std::uninitialized_move(data_, data_ + used_, new_data);
            try 
            {
                std::uninitialized_fill(new_data + used_, new_data + newsz, value);
            }
            catch(...)
            {
                std::move(new_data, new_data + used_, data_);
                ::operator delete(new_data);
                throw;
            }
            ::operator delete(data_);
            data_ = new_data;
            size_ = newsz;
        } 
        used_ = newsz;
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
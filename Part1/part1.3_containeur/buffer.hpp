#pragma once

#include<memory>

template<class T>
class temporary_buffer
{
public:
    using value_type = T;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer  = const value_type*;
    using iterator = value_type*;
    using const_iterator = const value_type*;
    using difference_type = ptrdiff_t;
    using size_type = size_t;

private:
    std::unique_ptr<value_type[]> m_memory;
    size_type m_size;
public:
    temporary_buffer():
        temporary_buffer<value_type>(0)
        {}
    explicit temporary_buffer(
        size_type initial_size): 
        m_size(initial_size),
        m_memory(std::make_unique_for_overwrite<value_type[]>(initial_size))
    {}

    temporary_buffer(const temporary_buffer<value_type>& another_buffer):
        m_size(another_buffer.m_size),
        m_memory(std::make_unique_for_overwrite<value_type[]>(m_size))
    {
        std::copy_n(another_buffer.data(), m_size, m_memory.get());
    }
    temporary_buffer(temporary_buffer<value_type>&& another_buffer) noexcept:
        m_size(another_buffer.m_size),
        m_memory(another_buffer.m_memory)
    {
        another_buffer.m_size = 0;        
    }
    ~temporary_buffer() = default;
    constexpr temporary_buffer& operator = (const temporary_buffer<value_type>& another_buffer)
    {
        if(&another_buffer != this)
        {
            m_size = another_buffer.m_size;
            m_memory(std::make_unique_for_overwrite<value_type[]>(another_buffer.m_size)),
            std::copy_n(another_buffer.data(), m_size, m_memory.get());
        }
        return *this;
    }
    temporary_buffer& operator = (temporary_buffer<value_type>&& another_buffer) noexcept
    {
        if(&another_buffer != this)
        {
            m_size = another_buffer.m_size;
            m_memory = another_buffer.m_memory;
            another_buffer.m_size = 0;
        }
        return *this;
    }

    constexpr iterator begin() noexcept { return m_memory.get(); }
    constexpr const_iterator begin() const noexcept { return m_memory.get(); }

    constexpr const_iterator cbegin() const noexcept { return m_memory.get(); }
    constexpr const_iterator cend() const noexcept { return m_memory.get()  + m_size; }

    constexpr iterator end() { return m_memory.get() + m_size; }
    constexpr const_iterator end() const noexcept { return m_memory.get()  + m_size; }
    
    constexpr bool empty() const noexcept { return m_size == 0; }

    void increase_by(size_type number_of_elements)
    {
        size_type new_size = m_size + number_of_elements;
        auto new_memory = std::make_unique_for_overwrite<value_type[]>(new_size);
        std::copy_n(m_memory.get(), m_size, new_memory.get());
        m_size = new_size;
        std::swap(m_memory, new_memory);
    }

    constexpr pointer data() noexcept { return m_memory.get(); }
    constexpr const_pointer data() const noexcept { return m_memory.get(); }

    void decrease_by(size_type number_of_elements)
    {
        if(number_of_elements > m_size)
        {
            m_size = 0;
            m_memory = std::make_unique_for_overwrite<value_type[]>(m_size);
        }
        else
        {
            size_type new_size = m_size - number_of_elements;
            auto new_memory = std::make_unique_for_overwrite<value_type[]>(new_size);
            std::copy_n(m_memory.get(), new_size, new_memory.get());
            m_size = new_size;
            std::swap(m_memory, new_memory);
        }
    }

    constexpr size_type max_size() const noexcept { return std::numeric_limits<unsigned>::max(); }
    constexpr size_type size() const noexcept { return m_size; }

    void resize(size_type number_of_elements)
    {
        if(number_of_elements < m_size)
            decrease_by(m_size - number_of_elements)
        else
            increase_by(number_of_elements - m_size);
    }

    constexpr void swap(temporary_buffer<value_type>& another_buffer) noexcept
    {
        std::swap(m_memory, another_buffer.m_memory);
        std::swap(m_size, another_buffer.m_size);
    }

    constexpr bool operator == (const temporary_buffer<value_type>& another_buffer) const noexcept
    {
        return m_memory == another_buffer.m_memory;
    }
};

namespace std
{

template<class T>
constexpr void swap(temporary_buffer<T>& first_buffer, temporary_buffer<T>& second_buffer)
{
    first_buffer.swap(second_buffer);
}

}
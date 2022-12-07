#pragma once

#include<memory>

template<class T>
class temporary_buffer
{
public:
    using value_type = T;
    using pointer = value_type*;
    using iterator = value_type*;
    using size_type = size_t;

private:
    std::unique_ptr<value_type[]> m_memory;
    size_type m_size;
public:
    explicit temporary_buffer(
        size_type initial_size): 
        m_size(initial_size),
        m_memory(std::make_unique_for_overwrite<value_type[]>(initial_size))
    {}

    ~temporary_buffer() = default;

    constexpr iterator begin() { return m_memory.get(); }
    constexpr iterator end() { return m_memory.get() + m_size; }

    constexpr pointer data() { return m_memory.get(); }
    
    void increase_by(size_type number_of_elements)
    {
        size_type new_size = m_size + number_of_elements;
        auto new_memory = std::make_unique_for_overwrite<value_type[]>(new_size);
        std::copy_n(m_memory.get(), m_size, new_memory.get());
        m_size = new_size;
        std::swap(m_memory, new_memory);
    }

    constexpr size_type size() const noexcept { return m_size; }
};

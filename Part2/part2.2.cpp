#include<exception>
#include<iostream>
#include<memory>

class invalid_iterator: std::exception
{
public:
    invalid_iterator() {}
    invalid_iterator(const char* const &aMessage):
        exception(aMessage) {}
};

template<typename T>
class List
{
private:
    struct Node
    {
    private:
        T m_value;
        std::shared_ptr<Node> m_next_node;

    public:
        Node(T aValue): 
            m_value(aValue), m_next_node()
            {}
        Node(T aValue, std::shared_ptr<Node>& theNextNode):
            m_value(aValue), m_next_node(theNextNode)
            {}
        
        void insert_after(T aValue)
        {
            m_next_node = std::make_shared<Node>(aValue, m_next_node);
        }
        std::shared_ptr<Node>& next() { return m_next_node; }
        T& value() { return m_value; }
        T value() const { return m_value; }
    };

    using version_type = unsigned;

    std::shared_ptr<Node> m_front;
    std::shared_ptr<Node> m_back;
    version_type m_version;

public:
    using value_type = T;
    using pointer = value_type*;
    using reference = T&;

    class iterator: 
        std::iterator<std::forward_iterator_tag, T>
    {
    private:
        std::shared_ptr<Node> m_current;
        const List<T>& m_list;
        typename List<T>::version_type m_version;
        void check_if_is_valid()
        {
            if(m_version != m_list.m_version)
                throw invalid_iterator();
        }

    public:        
        using typename std::iterator<std::forward_iterator_tag, T>::pointer;   
        using typename std::iterator<std::forward_iterator_tag, T>::reference;   

        iterator(const List<T>& theList):
             m_list(theList), m_current(), 
             m_version(theList.m_version) {}
        iterator(const List<T>& theList, 
            std::shared_ptr<Node>& node):
            m_list(theList), m_current(node), 
            m_version(theList.m_version) {}
        iterator& operator++()
        {
            check_if_is_valid();
            if(m_current != NULL)
                m_current = m_current->next();
            return *this;
        }
        iterator operator++(int)
        {
            check_if_is_valid();
            auto result = iterator(*this);
            if(m_current != NULL)
                m_current = m_current->next();
            return result;
        }
        reference operator *()
        {
            check_if_is_valid();
            return m_current->value();
        }
        pointer operator ->()
        {
            check_if_is_valid();
            return &(m_current->value());
        }
        bool operator == (const iterator& another) 
        { 
            return &m_list == &another.m_list 
                && m_version == another.m_version 
                && m_current == another.m_current; 
        }
        bool operator != (const iterator& another) 
        { 
            return &m_list == &another.m_list 
                && m_version == another.m_version 
                && m_current != another.m_current; 
        }
    };

    List(): m_front(), m_back(), m_version(0) {}
    iterator begin() { return iterator(*this, m_front); }
    iterator end() { return iterator(*this); }
    void push_front(T value)
    {
        if(m_front == NULL)
        {
            m_front = std::make_shared<Node>(value);
            m_back = m_front;
        }
        else
            m_front = std::make_shared<Node>(value, m_front);
        m_version ++;
    }
    void push_back(T value)
    {
        if(m_back == NULL)
        {
            m_front = std::make_shared<Node>(value);
            m_back = m_front;
        }
        else
        {
            m_back->insert_after(value);
            m_back = m_back->next();
        }
        m_version ++;
    }
};


int main()
{
    List<int> list;
    list.push_back(0);
    for(int i = 1; i<5; i++)
    {
        list.push_back(i);
        list.push_front(i);
    }
    for(auto it = list.begin(); it != list.end(); it++)
        std::cout << *it << "\n";
}

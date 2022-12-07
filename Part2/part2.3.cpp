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

    class iterator
    {
    private:
        std::weak_ptr<Node> m_current;
        const List<T>& m_list;
        typename List<T>::version_type m_version;
        void check_if_is_valid()
        {
            if(m_current.expired() || m_version != m_list.m_version)
                throw invalid_iterator();
        }

    public:     
        using difference_type = typename std::iterator_traits<T*>::difference_type;
        using value_type = typename std::iterator_traits<T*>::value_type;
        using pointer = typename std::iterator_traits<T*>::pointer;
        using reference = typename std::iterator_traits<T*>::reference;
        using iterator_category = typename std::forward_iterator_tag;
        using iterator_concept = typename std::forward_iterator_tag;

        iterator(const List<T>& theList):
             m_list(theList), m_current() {}
        iterator(const List<T>& theList, 
            std::shared_ptr<Node>& node):
            m_list(theList), m_current(node)
        {}
        iterator& operator++()
        {
            check_if_is_valid();
            auto current_ptr = m_current.lock();
            if(current_ptr != NULL)
                m_current = current_ptr->next();
            return *this;
        }
        iterator operator++(int)
        {
            check_if_is_valid();
            auto result = iterator(*this);
            auto current_ptr = m_current.lock();
            if(current_ptr != NULL)
                m_current = current_ptr->next();
            return result;
        }
        reference operator *()
        {
            check_if_is_valid();
            return m_current.lock()->value();
        }
        pointer operator ->()
        {
            check_if_is_valid();
            return &(m_current.lock()->value());
        }
        bool operator == (const iterator& another) 
        { 
            return &m_list == &another.m_list 
                && m_version == another.m_version 
                && m_current == another.m_current; 
        }
        bool operator != (const iterator& another) 
        { 
            return &m_list != &another.m_list 
                && m_version != another.m_version 
                && m_current.lock() != another.m_current.lock(); 
        }
    };
public:
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

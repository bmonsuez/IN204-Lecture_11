#include<exception>
#include<iostream>
#include<memory>


template<typename T>
class List
{
private:
    struct Node
    {
    private:
        T m_value;
        Node* m_next_node;

    public:
        Node(T aValue): Node(aValue, NULL) 
            {}
        Node(T aValue, Node* theNextNode):
            m_value(aValue), m_next_node(theNextNode)
            {}
        
        void insert_after(T aValue)
        {
            m_next_node = new Node(aValue, m_next_node);
        }
        Node* next() { return m_next_node; }
        T& value() { return m_value; }
        T value() const { return m_value; }
    };

    Node* m_front;
    Node* m_back;

    class iterator: 
        std::iterator<std::forward_iterator_tag, T>
    {
    private:
        Node* m_current;
    public:        
        iterator(Node* node): m_current(node)
        {}
        iterator& operator++()
        {
            if(m_current != NULL)
                m_current = m_current .next();
            return *this;
        }
        iterator operator++(int)
        {
            auto result = iterator(*this);
            if(m_current != NULL)
                m_current = m_current->next();
            return result;
        }
        reference operator *()
        {
            return m_current->value();
        }
        pointer operator ->()
        {
            return &m_current.value();
        }
        bool operator == (const iterator& another) { return m_current == another.m_current; }
        bool operator != (const iterator& another) { return m_current != another.m_current; }
    };
public:
    List(): m_front(NULL), m_back(NULL) {}
    ~List()
    {
        for(auto m_current = m_front; m_current != NULL; )
        {
            auto m_next = m_current->next();
            delete m_current;
            m_current = m_next;
        }
    }
    iterator begin() { return iterator(m_front); }
    iterator end() { return iterator(NULL); }
    void push_front(T value)
    {
        if(m_front == NULL)
        {
            m_front = new Node(value);
            m_back = m_front;
        }
        else
            m_front = new Node(value, m_front);
    }
    void push_back(T value)
    {
        if(m_back == NULL)
        {
            m_front = new Node(value);
            m_back = m_front;
        }
        else
        {
            m_back->insert_after(value);
            m_back = m_back->next();
        }
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

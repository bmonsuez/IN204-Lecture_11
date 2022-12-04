## Partie 2 : `std::shared_ptr` & liste simplement chaînée

Nous nous intéressons à un containeur que nous implantons comme une liste simplement chaînée.

```cpp
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

public:
    List(): m_front(NULL), m_back(NULL) {}
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

```

### Question n°1 :

Proposez un destructeur qui détruit bien tous les éléments stockés dans la liste avant de détruire la liste.

<details><summary>Correction</summary>

Nous devons parcourir l'ensemble des nœuds qui ont été créés et les détruire un par un.

```cpp
    ~List()
    {
        for(auto m_current = m_front; m_current != NULL; )
        {
            auto m_next = m_current->next();
            delete m_current;
            m_current = m_next;
        }
    }
```
</details>

### Question n°2 :

Nous aimerions ajouter des itérateurs. Pour ce faire, nous proposons la classe suivante d'itérateurs qui fait référence au nœud de la liste.

```cpp
class List
{
    public:
    ...
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
...
}
```

Ajouter les méthodes `begin` et `end` à la liste.

<details><summary>Correction</summary>

Le code est relativement simple et se réduit à :

```cpp
    iterator begin() { return iterator(m_front); }
    iterator end() { return iterator(NULL); }
```
</details>

### Question n°3 :

Est-il possible d'utiliser des `std::unique_ptr` en lieu et place des pointeurs natifs de C++, expliquer pourquoi ?

<details><summary>Correction</summary>

Il n'est pas possible d'utiliser les `std::unique_ptr` pointeurs puisque nous avons deux références sur le dernier élément de la liste, la référence venant du nœud précédent le dernier et la référence stockée dans le champ `m_back`.

De même, les itérateurs font références à des nœuds de la liste, ce qui veut dire que pour un nœud de la liste, nous pouvons avoir plusieurs références sur ce nœud.

</details>

### Question n°4 :

Est-il possible d'utiliser des `std::shared_ptr` en lieu et place des pointeurs natifs de C++, expliquer pourquoi ?

<details><summary>Correction</summary>

Les `std::shared_ptr` supportent plusieurs références à un élément. En conséquence, il est possible d'utiliser des `std::shared_ptr` au lieu et place de pointeurs natifs de C++. Ces pointeurs autorisent un accès aussi rapide que les pointeurs standards de C++ et garantissent que l'élément référencé ne sera détruit uniquement au moment de la destruction de la dernière référence sur celui-ci.

</details>

### Question n°5 :

Réécrivez la classe avec des `std::shared_ptr`. Avons-nous toujours besoin d'un destructeur ?

<details><summary>Correction</summary>

Il suffit de remplacer dans le code précédent chacune des références à un pointeur sur un élément de `T` par un `std::shared_ptr<T>`.

De plus, il n'est plus besoin de destructeur sachant que la suppression de toutes les références à un nœud va entrainer automatiquement la destruction de ce nœud.

Il est cependant à remarquer que cette stratégie de gestion de la mémoire peut induire un changement de comportement du logiciel. Si un itérateur faisant référence à un élément de la liste continue à exister après la destruction de la liste, l'élément de la liste référencée par cet itérateur continuera à être présent en mémoire tant que l'itérateur ne sera pas détruit.

La suppression du destructeur ainsi que le remplacement des pointeurs natifs `T*` par des `std::shared_ptr<T>` conduit au code suivant :

```cpp
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

    std::shared_ptr<Node> m_front;
    std::shared_ptr<Node> m_back;

    class iterator: 
        std::iterator<std::forward_iterator_tag, T>
    {
    private:
        std::shared_ptr<Node> m_current;
    public:        
        iterator(): m_current() {}
        iterator(std::shared_ptr<Node>& node): m_current(node)
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
            return &(m_current->value());
        }
        bool operator == (const iterator& another) { return m_current == another.m_current; }
        bool operator != (const iterator& another) { return m_current != another.m_current; }
    };
public:
    List(): m_front(NULL), m_back(NULL) {}
    iterator begin() { return iterator(m_front); }
    iterator end() { return iterator(); }
    void push_front(T value)
    {
        if(m_front == NULL)
        {
            m_front = std::make_shared<Node>(value);
            m_back = m_front;
        }
        else
            m_front = std::make_shared<Node>(value, m_front);
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
    }
};
```
</details>

### Question n°6 :

Nous souhaitons désormais imposer une sémantique qui est assez habituelle lorsque nous manipulons des itérateurs. Un itérateur est invalidé quand la collection sous-jacente est modifiée. La technique la plus classique pour déterminer si un containeur a été modifiée, consister à ajouter un numéro de version au containeur, souvent représenté par un nombre entier non-signé, qui est incrémenté chaque fois que le containeur est modifié. Lors que l'itérateur cherche à accéder à un élément, il vérifie que le numéro de version correspond au numéro de version qu'il a obtenu lors de sa création, si ce n'est pas le cas, l'itérateur déclenche une exception.

### Question n°6.1 :

Modifier le code du containeur `List` pour intégrer les fonctionnalités suivantes :

* `List` doit définir un champ qui contient le numéro de version du containeur. Le champ est supporté être à zéro lors de l'initialisation du containeur.
* Chacune des opérations qui modifient le containeur doit incrémenter le numéro de version.
* Chaque fois qu'un itérateur essaye d'accéder à un élément de la collection, il doit vérifier que le numéro de version est bien défini, sinon, il doit générer une exception que nous avons défini comme étant de type `invalid_iterator` et dérivant de `std::exception`.

<details><summary>Correction</summary>

Nous définissons dans un premier temps une nouvelle classe `invalid_iterator` :

```cpp
class invalid_iterator: std::exception
{
public:
    invalid_iterator() {}
    invalid_iterator(const char* const &aMessage):
        exception(aMessage) {}
};
```

Ensuite, nous ajoutons à la classe `List`, le numéro de version :
```cpp
template<typename T>
class List:
{
private:
    ...
    using version_type = unsigned;
    ...
    version_type m_version = 0ul;

public:

};
```

Les méthodes modifiant la liste `List` doivent désormais incrémentée le numéro de version `m_version`. 

```cpp
template<typename T>
class List:
{
    ...
public:
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
```
Une fois ces modifications faites, il faut modifier la classe itérateur `List<T>::iterator`. Désormais cette classe doit stocker une référence à la liste ainsi que doit initialiser un champ `m_version` avec le numéro de version définie par la liste au moment de la création de l'itérateur.

Il faut en conséquence ajouter les deux champs et modifier les constructeurs comme suit :

```cpp
    class iterator: 
        std::iterator<std::forward_iterator_tag, T>
    {
    private:
        std::shared_ptr<Node> m_current;
        const List<T>& m_list;
        typename List<T>::version_type m_version;
        ...
    public:
        iterator(const List<T>& theList):
             m_list(theList), m_current(), 
             m_version(theList.m_version) {}
        iterator(const List<T>& theList, 
            std::shared_ptr<Node>& node):
            m_list(theList), m_current(node), 
            m_version(theList.m_version) {}
        {}
        ...
    };
```

Ensuite, chaque méthode de l'itérateur doit vérifier que l'itérateur est valide. Pour factoriser le code, nous proposons de définir une méthode `check_if_is_valid` qui vérifie que l'itérateur est bien valide et si ce n'est le cas lève l'exception `invalid_iterator`.

```cpp
    class iterator: 
        std::iterator<std::forward_iterator_tag, T>
    {
    private:
        ...
        void check_if_is_valid()
        {
            if(m_version != m_list.m_version)
                throw invalid_iterator();
        }
        ...
    public:
        ...
    };
```

Enfin pour chacun que méthodes qui manipulent l'itérateur, comme les opérateurs d'accès à l'élément, les opérateurs d'incrémentation, il est nécessaire d'appeler la méthode privée `check_if_is_valid` pour s'assurer que l'itérateur est valide.

```cpp
    class iterator: 
        std::iterator<std::forward_iterator_tag, T>
    {
        ...
        iterator& operator++()
        {
            check_if_is_valid();
            if(m_current != NULL)
                m_current = m_current->next();
            return *this;
        }
        ...
        reference operator *()
        {
            check_if_is_valid();
            return m_current->value();
        }
        ...
    };
```

Enfin, nous devons modifier les appels aux constructeurs dans les méthodes `begin` et `end` de la liste `List`.

```cpp
template<typename T>
class List:
{
    ...
public:
    ...
    iterator begin() { return iterator(*this, m_front); }
    iterator end() { return iterator(*this); }
    ...
};
```

Ce qui termine l'ensemble des modifications nécessaires pour implanter ce nouveau comportement.

Ceci nous donne en conséquence le code consolidé suivant :

```cpp
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
```

</details>

### Question n°6.2 :

Cependant, le code précédent n'est pas optimal. En effet, quand le containeur est détruit et qu'un itérateur faisant référence au containuer existe encore, l'itérateur va pour pouvoir d'abord vérifier le numéro de version. Cependant si la liste a été supprimé, le champ correspondant au numéro de version n'est plus accessible et nous aurons une erreur d'accès qui va se produire à ce moment. 

Pour ce faire, nous souhaitons pouvoir garantir deux comportements :

* détruire l'ensemble des éléments stockés dans la liste (ou le containeur) quand le containeur est détruit. 
* déterminer si la liste (ou le containeur) a été détruit.

C++ introduit une classe dénommée [`std::weak_ptr`](https://en.cppreference.com/w/cpp/memory/weak_ptr). À la différence de `std::unique_ptr` ou `std::shared_ptr`, un `std::weak_ptr` ne garantit pas que l'élément auquel il fait référence est accessible. Pour pouvoir accéder à un objet, il faut verrouiller l'accès en utilisant la méthode [`lock`](https://en.cppreference.com/w/cpp/memory/weak_ptr/lock)/ Cette méthode retourne un `std::shared_ptr` qui soit fait référence à l'élément si cette référence est encore actuelle ou fait référence à `NULL` si la référence est devenue obsolète. La méthode [`expired`](https://en.cppreference.com/w/cpp/memory/weak_ptr/expired) indique si la référence à l'élément est devenue obsolète.

Désormais, si nous souhaitons implanter un itérateur qui détermine si la collection n'a pas été modifiée, cela impose :

1. Tester si la référence est devenue obsolète. Si la référence est devenue obsolète, cela signifie que l'élément référencé a été retiré de la collection ou a été modifié. Dans ce cas, l'itérateur n'est plus valide.
2. Tester si le numéro de version de la liste est différent de celui obtenu lors de la création de la liste. Si tel est le cas, l'itérateur n'est plus valide.

<details><summary>Correction</summary>

La solution consiste à utiliser dans la classe `iterator` un `std::weak_ptr` en lieu et place d'un `std::shared_ptr` pour faire référence au nœud de la liste. 

```cpp
    class iterator: 
        std::iterator<std::forward_iterator_tag, T>
    {
    private:
        std::weak_ptr<Node> m_current;
        const List<T>& m_list;
        typename List<T>::version_type m_version;
    ...
    };
```
Nous modifions ensuite le code de la méthode `check_if_is_valid` pour tester en premier si la référence au nœud est toujours valide. Si elle n'est plus valide, cela signifie soit que le nœud a été supprimé, soit que la collection a été détruite.

    class iterator: 
        std::iterator<std::forward_iterator_tag, T>
    {
    private:
        ...
        void check_if_is_valid()
        {
            if(m_current.expired() || m_version != m_list.m_version)
                throw invalid_iterator();
        }
        ...
    };

Il faut ensuite modifier les méthodes qui accèdent au nœud pour obtenir un `std::shared_ptr` le temps nécessaire pour effectuer l'opération. Ce `std::shared_ptr` sera automatiquement détruit au plus tard à la fin de la méthode.

Ceci donne par exemple les méthodes suivantes :

```cpp
    class iterator: 
        std::iterator<std::forward_iterator_tag, T>
    {
        ...
    public:
        ...
        iterator operator++(int)
        {
            check_if_is_valid();
            auto result = iterator(*this);
            auto current_ptr = m_current.lock();
            if(current_ptr != NULL)
                m_current = current_ptr->next();
            return result;
        }
        ...
        reference operator *()
        {
            check_if_is_valid();
            return m_current.lock()->value();
        }
        ...
    };
```

Et cela termine l'ensemble des modifications à réaliser pour implanter le nouveau comportement. 

Ceci donne le code consolidé suivant :
```cpp
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

    class iterator: 
        std::iterator<std::forward_iterator_tag, T>
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
```

</details>



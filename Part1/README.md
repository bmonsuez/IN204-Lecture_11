# Partie 1 : allocation d'un buffer

Nous considérons une opération de recherche d'une expression régulière dans un fichier. Pour ce faire, il est nécessaire de pouvoir charger en mémoire une certain nombre de caractères du fichier. Et nous allons chercher dans ce buffer, l'expression régulière concernée. Nous continuons à charger le buffer suivant et nous continuons la recherche
jusqu'à ce que l'on arrive à la fin du fichier.

L'expression régulière suivante recherche une définition d'une variable 

```cpp
std::regex match_variables(
    "^([A-Za-z_][A-Za-z_0-9]*)\\s*=\\s*(.*)$", 
    std::regex_constants::ECMAScript);
```

qui est définie selon la syntaxe : 

```text
...
VAR_NAME=VAR_CONTENT
NEXT_VAR_NAME=VAR_CONTENT
...
```

Le code suivant crée un buffer d'une taille adéquate et recherche dans ce buffer l'expression régulière.

```cpp

std::regex match_variables(
    "^([A-Za-z_][A-Za-z_0-9]*)\\s*=\\s*(.*)$", 
    std::regex_constants::ECMAScript);

std::map<std::string, std::string> find_all_variables(std::string filename)
{
    size_t buffer_size = 1024;
    using iterator = char*;
    char* buffer = new char[1024];
    std::map<std::string, std::string> variables;
    try
    {
        std::ifstream stream(filename);
        ptrdiff_t start = 0;

        while(!stream.eof() && !stream.fail())
        {
            // Load the buffer
            stream.read(buffer, buffer_size);            
            size_t number_of_available_chars = 
                stream.eof() ? (size_t)stream.gcount() : buffer_size;
            
            // Look inside the buffer for all patterns that 
            // matches a variable declaration.
            iterator current_iterator = buffer;
            iterator end_iterator = buffer + number_of_available_chars;
            std::match_results<iterator> match;
            while(current_iterator != end_iterator &&
                std::regex_search(
                    current_iterator, end_iterator, 
                    match, match_variables))
            {
                variables[match[1].str()] = match[2].str();
                current_iterator = match[0].second;
            }
        }
        delete[] buffer;
    }
    catch(...)
    {
        delete[] buffer;
        throw;
    }
    return variables;
}
```

## Question n°1.1

Tester le code précédent ? Expliquer pourquoi le bloc `try {} catch(...) {}` a été introduit dans le code ?

<details><summary>Correction</summary>
Le bloc `try {} catch(...) {}` est défini afin de capturer la survenance d'une exception. Un exception interrompt l'exécution du programme au moment où elle est déclenchée, ceci signifie que les lignes de programmes restantes ne sont pas exécutée. 

Si nous considérons le code précédent, cela signifie que l'instruction `delete [] buffer` ne sera pas exécutée. Ceci signifie que la levée de l'exception si aucune action n'est prise aura pour conséquence de créer une fuite de mémoire, la mémoire allouée pour le buffer `buffer` ne sera jamais libérée. Pour éviter cette situation, le `try {} catch(...) {}` capture toute exception qui aurait pu se produire et procéder à la libération de la mémoire allouée pour le buffer `buffer`. Une fois cette opération effectuée, l'exception est de nouveau propagée.

</details>

## Question n°1.2

Nous souhaitons ne pas utiliser directement des pointeurs. Proposez une écriture permettant d'éviter d'allouer de la mémoire dans le tas, mais dans la pile.

<details><summary>Correction</summary>

La plupart des compilateurs C offre la possibilité d'allouer dynamiquement de la mémoire de la pile. Il suffit pour cela d'allouer la mémoire en utilisant la fonction `_alloca` ou `alloca` en fonction du compilateur. 

Ainsi, il suffirait d'écrire (avec GCC par exemple) :

```cpp
    size_t buffer_size = 1024;
    using iterator = char*;
    char* buffer = (char*)alloca(buffer_size);
    std::ifstream stream(filename);

    while(!stream.eof() && !stream.fail())
    {
        // Load the buffer
        stream.read(buffer, buffer_size);            
        size_t number_of_available_chars = 
            stream.eof() ? (size_t)stream.gcount() : buffer_size;
        
        // Look inside the buffer for all the
        // variable definitions.
        iterator current_iterator = buffer;
        iterator end_iterator = buffer + buffer_size;
        std::match_results<iterator> match;
        while(current_iterator != end_iterator &&
            std::regex_search(
                current_iterator, end_iterator, 
                match, match_variables))
        {
            variables[match[1].str()] = match[2].str();
            current_iterator = match[0].second;
        }
    }
    return variables;

```
Cela effectuerait le travail. Cependant, cette technique n'est pas conseillée et elle correspond plus à un style de programmation C que C++. En plus, la taille de la pile n'est pas toujours extensible à souhait, cela dépend des processeurs. Ce type de code peut parfois conduire à la génération d'erreurs liées à un dépassement de la taille maximale de la pile, notamment sur des architectures embarquées où la taille maximale de la pile peut-être assez restreinte.

Comme la taille du buffer est une constante, il est possible d'allouer un tableau d'entier de taille statique [`std::array`](https://en.cppreference.com/w/cpp/container/array). Un tableau d'entier de taille statique est a priori alloué sur la pile, cependant, si le tableau est trop grand, il pourra être alloué dans le tas. C'est l'implantation associée à la plateforme et au compilateur qui déterminera où la mémoire sera effectivement allouée.

Dans ce cas, la déclaration de `buffer` se réécrit en :

```cpp
    const size_t buffer_size = 1024;
    using iterator = std::array<char, buffer_size>::iterator;

    std::array<char, buffer_size> buffer;
```

Désormais, nous ne manipulons plus un pointeur, mais un containeur. Il fait pour ce faire modifier l'appel à la fonction `read` qui lit les données à partir du flux :

```cpp
        stream.read(buffer.data(), buffer.size());            
```

De même, nous définissons les itérateurs en appelant directement les fonctions `begin` et `end` du containeur `buffer`.

```cpp
        iterator current_iterator = buffer.begin();
        iterator end_iterator = buffer.end();
        std::match_results<iterator> match;
        while(current_iterator != end_iterator &&
            std::regex_search(
                current_iterator, end_iterator, 
                match, match_variables))
        {
            variables[match[0].str()] = match.length();
            std::advance(current_iterator, match.length());
        }            
```

Ce qui nous donne le code consolidé suivant :

```cpp
std::map<std::string, std::string> find_all_variables(std::string filename)
{
    const size_t buffer_size = 1024;
    using iterator = std::array<char, buffer_size>::iterator;

    std::array<char, buffer_size> buffer;
    std::map<std::string, std::string> variables;

    std::ifstream stream(filename);
    while(!stream.eof() && !stream.fail())
    {
        // Load the buffer
        stream.read(buffer.data(), buffer.size());            
        size_t number_of_available_chars = 
            stream.eof() ? (size_t)stream.gcount() : buffer_size;
        
        // Look inside the buffer for all patterns that 
        // matches a variable declaration.
        iterator current_iterator = buffer.begin();
        iterator end_iterator = buffer.end();
        std::match_results<iterator> match;
        while(current_iterator != end_iterator &&
            std::regex_search(
                current_iterator, end_iterator, 
                match, match_variables))
        {
            variables[match[1].str()] = match[2].str();
            current_iterator = match[0].second;
        }            
    }
    return variables;
}
```

</details>

## Question n°2

En fait, le code précédent n'est pas correct parce que nous lisons le flux en chargeant des blocs de taille fixe. Nous ne devrions pas lire des blocs de taille fixe, mais des blocs de taille variable correspondant à une ligne (se terminant par un caractère de fin de ligne).

Les flux offrent la possibilité de lire une ligne (s'arrêtant au prochain retour à la ligne ou à la fin d'une ligne).

Nous souhaitons modifier le code précédent pour :

* Allouer un bloc correspondant à une taille prédéterminée (par exemple 80 caractères).
* Si jamais le bloc est trop petit, augmenter la taille du bloc mémoire.

Nous nous proposons d'utiliser les smart pointers et plus précisément les [`std::unique_ptr`](https://en.cppreference.com/w/cpp/memory/unique_ptr). Ces pointeurs définissent une référence unique sur un objet ou un tableau d'objets alloués dans le tas. Si le pointeur est détruit, puisqu'il s'agit de la seule référence à ce tableau d'objet, la zone mémoire associée à ce pointeur est automatique libérée.

Pour créer un tableau de caractères, il suffit d'écrire :

```cpp
size_t buffer_size = 80;
auto memory = std::make_unique<char[]>(buffer_size);
```

Si nous souhaitons effectuer un redimensionnement d'un vecteur, nous pouvons imaginer le code suivant :

```cpp
{
    size_t new_buffer_size = buffer_size + 80; 
    auto new_memory = std::make_unique<char[]>(new_buffer_size);
    std::copy(memory, memory + buffer_size, new_memory);
    buffer_size = new_buffer_size;
    std::swap(memory, new_memory);
}
```

## Question n°2.1

Expliquez le fonctionnement du code précédent. Pourquoi empêche-t-il d'avoir une fuite mémoire ?

<details><summary>Correction</summary>

Le code précédent crée un nouveau pointeur `new_memory` qui désigne une référence exclusive sur un tableau de caractères contenant `buffer_size + 80` éléments. Les `buffer_size` premiers éléments sont recopiés du tableau référencé par le pointeur `memory` dans le tableau référencé par le pointeur `new_memory`. Ensuite, nous permutons les deux vecteurs, `memory` fait désormais référencé à la zone mémoire nouvellement crée, `new_memory` fait référence à la zone mémoire antérieurement crée. Quand nous quittons le bloc, le pointeur `new_memory` est détruit, comme ce pointeur est une référence exclusive sur la mémoire, la mémoire associée est libérée. Il s'agit de la mémoire qui avait été allouée avant le redimensionnement du tableau auquel le pointeur `memory` faisait référence.

</details>

## Question n°2.2

À partir du code précédent, modifier pour ne pas lire des buffers de taille fixe, mais des buffers pouvant contenir toute la ligne courante. On lira en utilisant la méthode [`getline`](https://en.cppreference.com/w/cpp/io/basic_istream/getline) de la classe [`std::basic_istream`](https://en.cppreference.com/w/cpp/io/basic_istream).


<details><summary>Correction</summary>

Il est nécessaire de redéfinir l'opération de lecture à partir du flux. Nous souhaitons utiliser la méthode `getline()`. Cette méthode peut renvoyer trois états :

* le buffer `buffer` est suffisamment grand pour contenir l'ensemble des caractères constituant la ligne. Dans ce cas, aucune erreur est remontée, un appel à la méthode `fail()` retournera la valeur `false`. Si jamais nous avons atteint la fin du fichier, la méthode `eof()` indiquera que nous avons atteint la fin du fichier. 

* le buffer `buffer` n'est suffisamment grand pour contenir l'ensemble des caractères constituant la ligne, dans ce cas, un appel à la méthode `fail()` retournera la valeur `true` et `buffer_size - 1` caractères auront été lu dans le buffer. Il est possible d'avoir le nombre de caractères lus en appelant la méthode `gcount()`.

* une erreur, dans ce cas la méthode `gcount()` retourne moins de `buffer_size - 1` caractères et la méthode `fail()` retourne `false`.

Nous réécrivons l'opération de lecture comme suit :

1. Dans un premier temps, la ligne est lue à partir du flux :

```cpp
        stream.getline(buffer.get(), buffer_size);
        size_t number_of_available_chars = (size_t)stream.gcount();
```

2. Si nous sommes dans un état d'erreur, nous répétons le processus suivant :
 
   * Nous augmentons la taille du buffer
 
   * Nous lisons les caractères suivant en appelant de nouveau la méthode `getline`

   Et ce tant que nous n'avons atteint ni la fin du flux ni la fin de la ligne.


```cpp
        while(stream.fail() && !stream.eof() 
            && number_of_available_chars == buffer_size - 1)
        {
            // Increase the size of the buffer.
            auto new_buffer_size = buffer_size  + 40;
            auto new_buffer = std::make_unique<char[]>(new_buffer_size);
            std::copy(buffer.get(), 
                buffer.get() + buffer_size, new_buffer.get());

            // Load the upper part of the buffer.
            stream.clear();
            stream.getline(
                new_buffer.get() + number_of_available_chars, 
                new_buffer_size - number_of_available_chars);
            number_of_available_chars += (size_t)stream.gcount();

            // Swap both buffers
            buffer.swap(new_buffer);
            buffer_size = new_buffer_size;
        }
```

Ce qui nous donne le code consolidé suivant :

```cpp
std::map<std::string, std::string> find_all_variables(std::string filename)
{
    size_t buffer_size = 80;
    using iterator = char*;

    auto buffer = std::make_unique<char[]>(buffer_size);
    std::map<std::string, std::string> variables;
    std::ifstream stream(filename);

    while(!stream.eof() && !stream.fail())
    {
        // Load the buffer
        stream.getline(buffer.get(), buffer_size);
        size_t number_of_available_chars = (size_t)stream.gcount();
        while(stream.fail() && !stream.eof() 
            && number_of_available_chars == buffer_size - 1)
        {
            // Increase the size of the buffer.
            auto new_buffer_size = buffer_size  + 40;
            auto new_buffer = std::make_unique<char[]>(new_buffer_size);
            std::copy(buffer.get(), 
                buffer.get() + buffer_size, new_buffer.get());

            // Load the upper part of the buffer.
            stream.clear();
            stream.getline(
                new_buffer.get() + number_of_available_chars, 
                new_buffer_size - number_of_available_chars);
            number_of_available_chars += (size_t)stream.gcount();

            // Swap both buffers
            buffer.swap(new_buffer);
            buffer_size = new_buffer_size;
        }
                        
        // Test if the line that has been loaded denotes
        // a variable definition.
        std::match_results<iterator> match;
        if(std::regex_match(buffer.get(), buffer.get() + buffer_size, 
                match, match_variables))
        {
            variables[match[1].str()] = match[2].str();
        }
    }
    return variables;
}
```
</details>

## Question n°3 

En fait, dans le code précédent, nous constatons un mélange entre le code qui implante et manipule le `buffer` et le code qui lit les informations du flux `stream` et qui analyse le contenu de la ligne. Il serait bien de séparer ces codes, de manière à rendre la lecture plus simple. 

Pour simplifier le code, nous aimerions pouvoir non pas directement manipuler le pointeur sur la mémoire, mais éventuellement un objet qui encapsulerait la mémoire, un peu comme `std::array` encapsule la mémoire allouée dans un tableau de taille fixe.

En effet, si imaginons que `buffer` n'est plus un `std::unique_ptr`, mais un objet plus riche qui offre une méthode permettant de redimensionner la classe `temporary_buffer`, nous pourrions réécrire le code comme suit :

```cpp
    while(!stream.eof() && !stream.fail())
    {
        // Try to load the full line into the buffer.
        stream.getline(buffer.data(), buffer.size());
        size_t number_of_available_chars = (size_t)stream.gcount();
        while(stream.fail() && !stream.eof() 
            && number_of_available_chars == buffer.size() - 1)
        {
            // Increase the buffer as long as it is required.
            buffer.increase_by(increment);
            stream.clear();
            stream.getline(
                buffer.data() + number_of_available_chars, 
                buffer.size() - number_of_available_chars);
            number_of_available_chars += (size_t)stream.gcount();
        }
                        
        // Test if the line matches the regular expressions and
        // retrieve the name of the variable and the associated value.
        std::match_results<iterator> match;
        if(std::regex_match(buffer.begin(), buffer.end(), 
            match, match_variables))
        {
            variables[match[1].str()] = match[2].str();
        }
    }
```

Nous nous proposons donc de générer une classe `temporary_buffer` qui serait à même de fournir les fonctionnalités.

### Question n°3.1

Déterminer l'ensemble des informations dont la classe `temporary_buffer` a besoin. À partir de ces informations, déterminer l'ensemble des champs nécessaires ainsi que l'ensemble des méthodes dont vous avez impérativement besoin pour que le code précédent fonctionne.

<details><summary>Correction</summary>

La seule information nécessaire est la taille courante du buffer.

Les champs devant être stockés dans le tableau sont :

* la taille courante du buffer, 
* un pointeur sur la mémoire allouée par le buffer.

Les opérations nécessaires pour la génération du code précédent sont :

* la création du buffer
* la destruction du buffer
* l'augmentation de la taille du buffer
* l'accès en écriture à la zone mémoire interne du buffer, celle servant à recevoir les données du flux,
* l'accès en lecture à l'aide d'itérateurs,
* la taille du buffer.

Ceci permet de définir la première interface suivante pour la définition de cette nouvelle classe `temporary_buffer` :

```cpp
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
    explicit temporary_buffer(size_type initial_size);
    ~temporary_buffer();

    constexpr iterator begin();
    constexpr iterator end();
    
    void increase_by(size_type number_of_elements);

    constexpr pointer data() { return m_memory.get(); }

    constexpr size_type size() const noexcept { return m_size; }
};
```
</details>

### Question n°3.2

Proposez une implantation pour l'ensemble des champs et méthodes que vous avez identifiées dans la question précédente.

<details><summary>Correction</summary>

L'implantation est assez simple, en fait, seule la méthode de redimensionnement n'est pas immédiate, mais elle correspond au code précédemment fourni pour effectuer le redimensionnement du buffer.

```cpp
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
```
</details>

### Question n°3.3

Réécrivez le code de la fonction de lecture en utilisant désormais la classe `temporary_buffer` et non plus un pointeur à accès exclusif sur une zone mémoire.

<details><summary>Correction</summary>

En fait, le code se réécrit de manière immédiate en remplaçant :

```cpp
std::unique_ptr buffer = std::make_unique<char[]>(buffer_size)
``` 

par :

```cpp
    buffer_type buffer(buffer_size) ;
``` 

Il faut de plus redéfinir le type itérateur : 

```cpp
    using buffer_type = temporary_buffer<char>;
    using iterator = typename buffer_type::iterator;
```

De même les appels à la méthode `getline` sont modifiés en appelant non plus la méthode `get()` de `std::unique_ptr`, mais la méthode `data()` de `temporary_buffer`. Il en va de même pour la taille du buffer qui désormais est stockée dans l'objet `buffer`et non plus dans la variable `buffer_size`. 

Pour exemple :

```cpp
        stream.getline(buffer.get(), buffer_size);
```

est réécrit en :

```cpp
        stream.getline(buffer.data(), buffer.size());
```

Enfin, le code correspond à redimensionnement du buffer doit être supprimé et est remplacé par le seul appel à la méthode `increase_by` de la classe `temporary_buffer` :

```cpp
            buffer.increase_by(increment);
```

Toutes ces modifications nous fournissent le code suivant :

```cpp
std::map<std::string, std::string> find_all_variables(std::string filename)
{
    using buffer_type = temporary_buffer<char>;
    using iterator = typename buffer_type::iterator;

    const size_t buffer_size = 80;
    const size_t increment = 40;

    buffer_type buffer(buffer_size) ;
    std::map<std::string, std::string> variables;
    std::ifstream stream(filename);

    while(!stream.eof() && !stream.fail())
    {
        // Try to load the full line into the buffer.
        stream.getline(buffer.data(), buffer.size());
        size_t number_of_available_chars = (size_t)stream.gcount();
        while(stream.fail() && !stream.eof() 
            && number_of_available_chars == buffer.size() - 1)
        {
            // Increase the buffer as long as it is required.
            buffer.increase_by(increment);
            stream.clear();
            stream.getline(
                buffer.data() + number_of_available_chars, 
                buffer.size() - number_of_available_chars);
            number_of_available_chars += (size_t)stream.gcount();
        }
                        
        // Test if the line matches the regular expressions and
        // retrieve the name of the variable and the associated value.
        std::match_results<iterator> match;
        if(std::regex_match(buffer.begin(), buffer.end(), 
            match, match_variables))
        {
            variables[match[1].str()] = match[2].str();
        }
    }
    return variables;
}
```
</details>

### Question n°3.4

Nous souhaitons transformer la classe `temporary_buffer` en classe générique. Typiquement, la classe `temporary_buffer` est une classe qui correspond à une containeur. Nous souhaitons donc qu'elle puisse être manipulée comme un containeur. Pour ce faire, il est nécessaire de s'assure que cette classe respecte les contraintes associées au containeurs tel que définis par la norme [C++ named requirements: Container](https://en.cppreference.com/w/cpp/named_req/Container).

### Question n°3.4.1

Établissez l'ensemble des méthodes que vous devez ajouter à votre classe.

<details><summary>Correction</summary>

Il suffit de faire la liste des types ainsi que des méthodes qui doivent être implantés. Cette liste permet de définir l'interface de notre classe générique `temporary_buffer` nécessaire pour être conforme à la définition d'un containeur :

```cpp
template<class T>
class temporary_buffer
{
private:
    std::unique_ptr<value_type[]> m_memory;
    size_type m_size;

public:
    using value_type = T;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer  = const value_type*;
    using iterator = value_type*;
    using const_iterator = const T*;
    using difference_type = ptrdiff_t;
    using size_type = size_t;

    temporary_buffer() noexcept;

    temporary_buffer(const temporary_buffer& another_buffer);
    temporary_buffer(temporary_buffer&& another_buffer);
    ~temporary_buffer() = default;
    constexpr temporary_buffer& operator = (const temporary_buffer& another_buffer);
    temporary_buffer& operator = (temporary_buffer&& another_buffer) noexcept;

    constexpr iterator begin() noexcept;
    constexpr const_iterator begin();

    constexpr const_iterator cbegin();
    constexpr const_iterator cend();

    constexpr iterator end() noexcept;
    constexpr const_iterator end();
    
    constexpr bool empty() const noexcept;
    void increase_by(size_type number_of_elements);

    constexpr size_type max_size() const noexcept;
    constexpr size_type size() const noexcept;
    constexpr void swap(temporary_buffer& another_buffer);

    constexpr bool operator == (const temporary_buffer& another_buffer) const noexcept;

};
```
</details>

### Question n°3.4.1

Terminez l'implantation de l'ensemble des définitions de types et méthodes que vous venez d'ajouter à votre classe.

<details><summary>Correction</summary>
La plupart des méthodes sont relativement simples à implanter et n'appelle pas de commentaires particuliers. Ainsi les méthodes qui retournent les itérateurs :

```cpp
    constexpr iterator begin() noexcept { return m_memory.get(); }
    constexpr const_iterator begin() const noexcept { return m_memory.get(); }

    constexpr const_iterator cbegin() const noexcept { return m_memory.get(); }
    constexpr const_iterator cend() const noexcept { return m_memory.get()  + m_size; }

    constexpr iterator end() { return m_memory.get() + m_size; }
    constexpr const_iterator end() const noexcept { return m_memory.get()  + m_size; }
```

Il en va de même pour les méthodes accédant aux nombres d'éléments et à la structure interne :

```cpp
    constexpr bool empty() const noexcept { return m_size == 0; }


    constexpr pointer data() noexcept { return m_memory.get(); }
    constexpr const_pointer data() const noexcept { return m_memory.get(); }

    constexpr size_type max_size() const noexcept { return std::numeric_limits<unsigned>::max(); }
    constexpr size_type size() const noexcept { return m_size; }
```

Cependant, il est intéressant de se pencher sur les constructeurs de recopie ainsi que les opérateurs d'affectation. Nous avons deux constructeurs de recopie et deux opérateurs d'affectation. 

```cpp
    temporary_buffer(const temporary_buffer& another_buffer);
    temporary_buffer(temporary_buffer&& another_buffer);

    constexpr temporary_buffer& operator = (const temporary_buffer& another_buffer);
    temporary_buffer& operator = (temporary_buffer&& another_buffer) noexcept;
```

Qu'est ce qui distingue l'opérateur :
```cpp
    constexpr temporary_buffer& operator = (const temporary_buffer& another_buffer);
```

de l'opérateur :

```cpp
    temporary_buffer& operator = (temporary_buffer&& another_buffer) noexcept;
```

En fait, si nous considérons le code suivant :

```cpp
    temporary_buffer create_buffer() { return temporary_buffer(10) }
    
    auto buffer = create_buffer();
    auto copy_of_buffer = buffer;
```

la fonction `create_buffer` va créer un buffer, ce buffer va être dupliqué dans le résultat de la fonction et il sera juste détruit après. De même le résultat de la fonction va être recopié dans la variable `buffer` et il va être ensuite détruit juste après l'affectation. 

Par contre, la dernière ligne indique que `copy_of_buffer` est créé comme étant une copie de `buffer`, mais que `buffer` continuera à être utilisé ensuite. 

Il est possible de définir différentes sémantiques de recopie. En C++, il en existe deux. La première sémantique est la sémantique habituelle, aussi apellée ***copy value semantics***, le contenu de l'objet destination reçoit une copie des informations définies dans l'objet source, le contenu de l'objet source est préservé. La seconde sémantique appelée ***move semantics*** par contre déplace le contenu de l'objet source vers le contenu de l'objet destination. Le contenu de l'objet source est supposé avoir été déplacé vers l'objet destination, il est soit invalidé, détruit ou réinitialisé.

Typiquement dans notre cas, lorsque nous dupliquons le buffer qui vient d'être crée dans la variable `buffer`, nous devrions a priori créer une nouvelle zone mémoire et ensuite recopier les données de la zone mémoire initiale vers la zone mémoire nouvellement créée. Ceci est inefficace, sachant que l'objet `temporary_buffer(10)` ne sera jamais plus utilisé. Il serait bien plus opportun de déplacement le contenu de l'objet qui a été créé par l'appel du constructeur `temporary_buffer(10)` (et qui sera détruit au moment de sortir de la fonction `create_buffer`) vers l'objet qui est créé et qui sera stocké dans la variable `buffer`. Pour que le compilateur puisse optimiser le code, il faut expliquer comment procéder à la création ou à l'affectation d'un objet en effectuant non pas une recopie mais un transfert de recopie.

Le constructeur ou l'opérateur d'affectation suivant :

```cpp
    temporary_buffer(temporary_buffer<value_type>&& another_buffer);
    temporary_buffer& operator = (temporary_buffer<value_type>&& another_buffer);
```

définissent le constructeur et l'opérateur d'affectation qui vont transférer le contenu du buffer `another_buffer` vers l'objet en cours de création ou l'objet destinataire de l'affectation. Comme le contenu de l'objet passé en paramètre `temporary_buffer<value_type>` est a priori invalidé, il n'y a pas de spécificateurs `const` qui qualifie le type.

Dans le cas des `temporary_buffer`, la sémantique serait de transférer la référence à la mémoire ainsi que de recopier le champ contenant la taille. Une fois l'opération faite, il faut remette la taille de la source à zéro. Ainsi le code du constructeur et de l'opération d'affectation se résume à :

```cpp
    temporary_buffer(temporary_buffer&& another_buffer) noexcept:
        m_size(another_buffer.m_size),
        m_memory(another_buffer.m_memory)
    {
        another_buffer.m_size = 0;        
    }
    temporary_buffer& operator = (temporary_buffer&& another_buffer) noexcept
    {
        if(&another_buffer != this)
        {
            m_size = another_buffer.m_size;
            m_memory = another_buffer.m_memory;
            another_buffer.m_size = 0;
        }
        return *this;
    }
```

En effet, la classe `std::unique_ptr` garantissant l'exclusivité ne supporte que la sémantique de transfert du contenu, l'instruction : 

```cpp
            m_memory = another_buffer.m_memory;
```

transfère la référence de l'objet pointeur `another_buffer.m_memory` et remet le contenu de ce pointeur à `std::null_ptr` pour indiquer que le pointeur ne pointe pas sur une zone définie.

Cependant, si nous souhaitons dupliquer le contenu, il faut fournir un constructeur de copie ainsi qu'un opérateur d'affectation qui effectue non pas un transfert, mais une copie du contenu de l'objet source vers l'objet destination. Il s'agit des constructeurs et opérateurs de copie habituels de C++ :

```cpp
    temporary_buffer(const temporary_buffer<value_type>& another_buffer);
    temporary_buffer& operator = (const temporary_buffer<value_type>& another_buffer);
```

Dans ce cas, il est nécessaire de créer une nouvelle zone mémoire qui va stocker les informations en provenance la zone mémoire à laquelle le buffer source fait référence et ensuite de copier le contenu de la zone mémoire initiale vers la zone mémoire destination.

```cpp
    temporary_buffer(const temporary_buffer<value_type>& another_buffer):
        m_size(another_buffer.m_size),
        m_memory(std::make_unique_for_overwrite<value_type[]>(m_size))
    {
        std::copy_n(another_buffer.data(), m_size, m_memory.get());
    }
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
```

Enfin, il ne reste plus qu'à définir l'opérateur de comparaison. Celui-ci se limite à comparer si les zones mémoires référencées sont égales :

```cpp
    constexpr bool operator == (const temporary_buffer<value_type>& another_buffer) const noexcept
    {
        return m_memory == another_buffer.m_memory;
    }
```

ainsi que la méthode `swap()` qui va permuter le contenu de chacun des objets buffers. Après cette opération, le buffer courant fera référence à la zone mémoire initialement référencée par le second buffer et le second buffer fera référence à la zone mémoire initialement référencée par le premier buffer.

```cpp
    constexpr void swap(temporary_buffer<value_type>& another_buffer) noexcept
    {
        std::swap(m_memory, another_buffer.m_memory);
        std::swap(m_size, another_buffer.m_size);
    }
```

Pour être complètement conforme, il faut surcharger la fonction `std::swap` de la bibliothèque standard qui prend deux buffers en argument et en permute les contenus.

```cpp
template<class T>
constexpr void swap(temporary_buffer<T>& first_buffer, temporary_buffer<T>& second_buffer)
{
    first_buffer.swap(second_buffer);
}
```

</details>
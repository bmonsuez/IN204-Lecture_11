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
    std::map<std::string, std::string> variables;
    std::vector<double> values;
    size_t buffer_size = 1024;
    size_t maximal_size_of_double = 16;
    char* buffer = new char[1024];
    using iterator = char*;
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
            
            // Look inside the buffer for all pattern that 
            // denotes a decimal value.
            iterator current_iterator = buffer;
            iterator end_iterator = buffer + buffer_size;
            std::match_results<iterator> match;
            while(current_iterator != end_iterator &&
                std::regex_search(
                    current_iterator, end_iterator, 
                    match, match_variables))
            {
                variables[match[0].str()] = match.length();
                std::advance(current_iterator, match.length());
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
    std::map<std::string, std::string> variables;
    std::vector<double> values;
    size_t buffer_size = 1024;
    size_t maximal_size_of_double = 16;
    char* buffer = (char*)alloca(1024);
    using iterator = char*;
    std::ifstream stream(filename);
    ptrdiff_t start = 0;

    while(!stream.eof() && !stream.fail())
    {
        // Load the buffer
        stream.read(buffer, buffer_size);            
        size_t number_of_available_chars = 
            stream.eof() ? (size_t)stream.gcount() : buffer_size;
        
        // Look inside the buffer for all pattern that 
        // denotes a decimal value.
        iterator current_iterator = buffer;
        iterator end_iterator = buffer + buffer_size;
        std::match_results<iterator> match;
        while(current_iterator != end_iterator &&
            std::regex_search(
                current_iterator, end_iterator, 
                match, match_variables))
        {
            variables[match[0].str()] = match.length();
            std::advance(current_iterator, match.length());
        }            
    }
    return variables;
}
```
Cela effectuerait le travail. Cependant, cette technique n'est pas conseillée et n'est pas non plus très C++. En plus, la taille de la pile n'est pas toujours extensible à souhait, cela dépend des processeurs. Ce type de code peut parfois conduire à la génération d'erreurs liées à un dépassement de la taille maximale de la pile.

Comme la taille du buffer étant une constante, il est possible d'allouer un tableau d'entier de taille statique [std::array](https://en.cppreference.com/w/cpp/container/array). Un tableau d'entier de taille statique est a priori alloué sur la pile, cependant, si le tableau est trop grand, il pourra être alloué dans le tas. C'est l'implantation associée à la plateforme et au compilateur qui déterminera où la mémoire sera effectivement allouée.

Dans ce cas, la déclaration de `buffer` se réécrit en :

```cpp
std::array<char, 1024> buffer;
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
    std::map<std::string, std::string> variables;
    std::vector<double> values;
    const size_t buffer_size = 1024;
    std::array<char, buffer_size> buffer;
    using iterator = std::array<char, buffer_size>::iterator;
    std::ifstream stream(filename);
    ptrdiff_t start = 0;

    while(!stream.eof() && !stream.fail())
    {
        // Load the buffer
        stream.read(buffer.data(), buffer.size());            
        size_t number_of_available_chars = 
            stream.eof() ? (size_t)stream.gcount() : buffer_size;
        
        // Look inside the buffer for all pattern that 
        // denotes a decimal value.
        iterator current_iterator = buffer.begin();
        iterator end_iterator = buffer.end();
        std::match_results<iterator> match;
        while(current_iterator != end_iterator &&
            std::regex_search(
                current_iterator, end_iterator, 
                match, match_variables))
        {
            variables[match[1].str()] = match[2].str();
            std::advance(current_iterator, match.length());
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
            // Create a larger buffer
            auto new_buffer_size = buffer_size  + 40;
            auto new_buffer = std::make_unique<char[]>(new_buffer_size);
            std::copy(buffer.get(), 
                buffer.get() + buffer_size, new_buffer.get());

            // Load the upper part of the buffer/
            stream.clear(stream.rdstate() & ~std::ios_base::failbit);
            stream.getline(
                new_buffer.get() + number_of_available_chars, 
                new_buffer_size - buffer_size);
            number_of_available_chars += (size_t)stream.gcount();

            buffer.swap(new_buffer);
            buffer_size = new_buffer_size;
        }
```

Ce qui nous donne le code consolidé suivant :

```cpp
std::map<std::string, std::string> find_all_variables(std::string filename)
{
    std::map<std::string, std::string> variables;
    std::vector<double> values;
    size_t buffer_size = 80;
    auto buffer = std::make_unique<char[]>(buffer_size);
    using iterator = char*;
    std::ifstream stream(filename);
    ptrdiff_t start = 0;

    while(!stream.eof() && !stream.fail())
    {
        // Load the buffer

        stream.getline(buffer.get(), buffer_size);
        size_t number_of_available_chars = (size_t)stream.gcount();
        while(stream.fail() && !stream.eof() 
            && number_of_available_chars == buffer_size - 1)
        {
            // Create a larger buffer
            auto new_buffer_size = buffer_size  + 40;
            auto new_buffer = std::make_unique<char[]>(new_buffer_size);
            std::copy(buffer.get(), 
                buffer.get() + buffer_size, new_buffer.get());

            // Load the upper part of the buffer/
            stream.clear(stream.rdstate() & ~std::ios_base::failbit);
            stream.getline(
                new_buffer.get() + number_of_available_chars, 
                new_buffer_size - buffer_size);
            number_of_available_chars += (size_t)stream.gcount();

            // Swap both buffers
            buffer.swap(new_buffer);
            buffer_size = new_buffer_size;
        }
                        
        // Look inside the buffer for all pattern that 
        // denotes a decimal value.
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



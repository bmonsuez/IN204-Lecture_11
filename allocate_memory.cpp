#include<array>
#include<complex>
#include<iostream>
#include<vector>

int main()
{
    int number_of_characters = 10;
    char* memory = new char[number_of_characters];
    for(char* start = memory; number_of_characters -- > 0; start ++)
        *start = 'a';
    delete [] memory;

    auto vec = new std::vector<int>();
    delete vec;

    std::vector<int*> v;
    for(int number_of_integers = 10; number_of_integers > 0; number_of_integers --)
        v.push_back(new int(number_of_integers));

    std::array<int, 3> a = {1, 2, 3};

    auto array_of_strings = new std::string[10];

    auto c = new std::complex(0, -1);
    std::cout << *c << std::endl;
    delete c;
}

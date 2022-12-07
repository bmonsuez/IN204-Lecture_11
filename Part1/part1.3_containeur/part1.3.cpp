#include<algorithm>
#include<fstream>
#include<iostream>
#include<iterator>
#include<map>
#include<regex>

#include"buffer.hpp"

std::regex match_variables(
    "^([A-Za-z_][A-Za-z_0-9()]*)\\s*=\\s*(.*)$", 
    std::regex_constants::ECMAScript);

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


int main()
{
    auto variables = find_all_variables("C:\\Temp\\variables");
    std::cout << "Number of variables: " << variables.size() << "\n";
}

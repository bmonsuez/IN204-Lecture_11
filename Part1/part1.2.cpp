#include<algorithm>
#include<fstream>
#include<iostream>
#include<iterator>
#include<map>
#include<regex>

std::regex match_variables(
    "^([A-Za-z_][A-Za-z_0-9()]*)\\s*=\\s*(.*)$", 
    std::regex_constants::ECMAScript);

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


int main()
{
    auto variables = find_all_variables("C:\\Temp\\variables");
    std::cout << "Number of variables: " << variables.size() << "\n";
}

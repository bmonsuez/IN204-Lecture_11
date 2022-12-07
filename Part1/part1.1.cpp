#include<array>
#include<fstream>
#include<iostream>
#include<iterator>
#include<map>
#include<regex>

std::regex match_variables(
    "^([A-Za-z_][A-Za-z_0-9]*)\\s*=\\s*(.*)$", 
    std::regex_constants::ECMAScript);

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

int main()
{
    auto variables = find_all_variables("C:\\Temp\\variables");
    std::cout << "Number of variables: " << variables.size() << "\n";
}

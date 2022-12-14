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


int main()
{
    auto variables = find_all_variables("C:\\Temp\\variables");
    std::cout << "Number of variables: " << variables.size() << "\n";
}

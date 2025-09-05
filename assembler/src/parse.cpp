#include <iostream>
#include <fstream>

#include "parse.hpp"

std::vector<Token> parse_tokens_from_file(std::string filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open()) return {};

    std::vector<char> file_buffer;

    file.seekg(0, std::ios::end);
    size_t length = file.tellg();
    file.seekg(0, std::ios::beg);

    file_buffer.resize(length);
    file.read((char*)file_buffer.data(), length);

    std::vector<Token> tokens;

    std::string token_buffer;
    bool parsing_comment = false;
    bool parsing_string = false;
    bool parsing_string_escape = false;
    int line = 1;

    for (int i = 0; i < file_buffer.size(); i++)
    {
        char c = file_buffer[i];

        if (parsing_comment)
        {
            if (c == '\n') parsing_comment = false;
            continue;
        }
        
        if (parsing_string)
        {
            if (parsing_string_escape)
            {
                if (c == 'n') token_buffer += '\n';
                if (c == 'r') token_buffer += '\r';
                parsing_string_escape = false;
                continue;
            }

            if (!parsing_string_escape && c == '"')
            {
                Token token;
                token.type = TokenType::StringLiteral;
                token.text = token_buffer;
                token.line = line;
                tokens.push_back(token);

                std::cout << "STRING LITERAL TOKEN: " << token_buffer << "\n";
                
                token_buffer.clear();
                parsing_string = false;
                continue;
            }

            parsing_string_escape = false;
        }
        else if (c == ' ' || c == ',' || c == '\n' || c == ';')
        {
            if (token_buffer.size() > 0)
            {
                tokens.push_back(create_token(token_buffer, line));
            }

            token_buffer.clear();

            if (c == ';')
            {
                parsing_comment = true;
            }

            if (c == '\n') line++;

            continue;
        }

        if (c >= '!' && c <= '~' || (c == ' ' && parsing_string))
        {
            if (token_buffer.empty() && c == '"')
            {
                parsing_string = true;
                continue;
            }

            if (parsing_string)
            {
                if (c == '\\')
                {
                    parsing_string_escape = true;
                    continue;
                }
                token_buffer += c;
            }
            else
            {
                token_buffer += to_lower(c);
            }
        }
    }

    if (!parsing_string && token_buffer.size() > 0)
    {
        tokens.push_back(create_token(token_buffer, line));
    }

    return tokens;
}

std::string parse_file_path_out_file_name(std::string filepath)
{
    int dir_idx = -1;
    int extension_idx = -1;
    for (int i = 0; i < filepath.length(); i++)
    {
        if (filepath[i] == '\\' || filepath[i] == '/')
        {
            dir_idx = i + 1;
        }
        else if (filepath[i] == '.')
        {
            extension_idx = i;
        }
    }

    if (dir_idx < 0) dir_idx = 0;
    if (extension_idx <= dir_idx) extension_idx = filepath.length() - 1;

    return filepath.substr(dir_idx, extension_idx - dir_idx) + ".vmex";
}
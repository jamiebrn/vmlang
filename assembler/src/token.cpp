#include <iostream>

#include "token.hpp"
#include "isa_map.hpp"

char to_lower(char c)
{
    if (c >= 'A' && c <= 'Z')
    {
        return c + ('a' - 'A');
    }

    return c;
}

bool is_token_register(const std::string& token, uint8_t& reg_id)
{
    if (auto iter = reg_name_map.find(token); iter != reg_name_map.end())
    {
        reg_id = iter->second;
        return true;
    }

    return false;
}

bool is_token_instruction(const std::string& token, uint8_t& instruction)
{
    if (auto iter = instruction_name_map.find(token); iter != instruction_name_map.end())
    {
        instruction = iter->second;
        return true;
    }

    return false;
}

bool is_token_label(const std::string& token)
{
    return token[0] == '.';
}

bool is_token_hex_literal(const std::string& token, uint32_t& value)
{
    if (token.length() < 3) return false;
    if (token[0] != '0') return false;
    if (to_lower(token[1]) != 'x') return false;

    value = 0;
    for (int i = 2; i < token.length(); i++)
    {
        char c = to_lower(token[i]);

        if (c >= '0' && c <= '9')
        {
            value = (value << 4) + (c - '0');
        }
        else if (c >= 'a' && c <= 'f')
        {
            value = (value << 4) + (c - 'a' + 10);
        }
        else
        {
            return false;
        }
    }

    return true;
}

bool is_token_int_literal(const std::string& token, uint32_t& value)
{
    if (token.length() < 1) return false;

    bool negative = false;
    value = 0;
    for (int i = 0; i < token.length(); i++)
    {
        char c = token[i];

        if (i == 0 && c == '-')
        {
            negative = true;
            continue;
        }

        if (c >= '0' && c <= '9')
        {
            value = value * 10 + (c - '0');
        }
        else
        {
            return false;
        }
    }

    if (negative)
    {
        value = -value;
    }

    return true;
}

bool is_token_float_literal(const std::string& token, float& value)
{
    if (token.length() < 1) return false;

    bool negative = false;
    bool found_decimal;
    float decimal_value = 0;
    value = 0;
    for (int i = 0; i < token.length(); i++)
    {
        char c = token[i];

        if (i == 0 && c == '-')
        {
            negative = true;
            continue;
        }

        if (c == '.')
        {
            if (!found_decimal)
            {
                found_decimal = true;
                continue;
            }

            return false;
        }

        if (c >= '0' && c <= '9')
        {
            if (found_decimal)
            {
                decimal_value = decimal_value / 10.0f + (c - '0') / 10.0f;
            }
            else
            {
                value = value * 10 + (c - '0');
            }
        }
        else if (i != token.length() - 1 || c != 'f')
        {
            return false;
        }
    }

    value += decimal_value;

    if (negative)
    {
        value = -value;
    }

    return true;
}

bool is_token_data_directive(const std::string& token)
{
    return token == "[data]";
}

bool is_token_program_directive(const std::string& token)
{
    return token == "[program]";
}

Token create_token(const std::string& text, int line)
{
    Token token;
    token.line = line;

    if (is_token_label(text))
    {
        token.type = TokenType::Label;
        token.text = text.substr(1);
        std::cout << "LABEL TOKEN: " << text << "\n";
        return token;
    }

    if (is_token_register(text, token.reg_id))
    {
        token.type = TokenType::Register;
        std::cout << "REGISTER (" << static_cast<int>(token.reg_id) << ") TOKEN: " << text << "\n";
        return token;
    }

    if (is_token_instruction(text, token.instruction))
    {
        token.type = TokenType::Instruction;
        std::cout << "INSTRUCTION (" << static_cast<int>(token.instruction) << ") TOKEN: " << text << "\n";
        return token;
    }

    if (is_token_int_literal(text, token.value))
    {
        token.type = TokenType::IntLiteral;
        std::cout << "INT LITERAL (" << token.value << ") TOKEN: " << text << "\n";
        return token;
    }
    
    if (is_token_float_literal(text, token.fvalue))
    {
        token.type = TokenType::FloatLiteral;
        std::cout << "FLOAT LITERAL (" << token.fvalue << ") TOKEN: " << text << "\n";
        return token;
    }

    if (is_token_hex_literal(text, token.value))
    {
        token.type = TokenType::HexLiteral;
        std::cout << "HEX LITERAL (" << token.value << ") TOKEN: " << text << "\n";
        return token;
    }
    
    if (is_token_data_directive(text))
    {
        token.type = TokenType::DataDirective;
        std::cout << "DATA DIRECTIVE TOKEN\n";
        return token;
    }

    if (is_token_program_directive(text))
    {
        token.type = TokenType::ProgramDirective;
        std::cout << "PROGRAM DIRECTIVE TOKEN\n";
        return token;
    }

    token.type = TokenType::Unknown;
    token.text = text;
    std::cout << "UNKNOWN TYPE TOKEN: " << text << "\n";

    return token;
}
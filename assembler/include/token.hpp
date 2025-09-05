#pragma once

#include <stdint.h>
#include <string>

enum class TokenType
{
    Unknown,

    DataDirective,
    ProgramDirective,

    Label,
    Instruction,
    Register,
    IntLiteral,
    FloatLiteral,
    HexLiteral,
    StringLiteral
};

struct Token
{
    TokenType type;

    union
    {
        uint8_t instruction;
        uint8_t reg_id;
        uint32_t value;
        float fvalue;
    };

    std::string text;

    int line;
};

char to_lower(char c);

bool is_token_register(const std::string& token, uint8_t& reg_id);

bool is_token_instruction(const std::string& token, uint8_t& instruction);

bool is_token_label(const std::string& token);

bool is_token_hex_literal(const std::string& token, uint32_t& value);

bool is_token_int_literal(const std::string& token, uint32_t& value);

bool is_token_float_literal(const std::string& token, float& value);

bool is_token_data_directive(const std::string& token);

bool is_token_program_directive(const std::string& token);

Token create_token(const std::string& text, int line);
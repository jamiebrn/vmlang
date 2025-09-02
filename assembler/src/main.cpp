#include <fstream>
#include <iostream>
#include <vector>
#include <stdint.h>
#include <unordered_map>
#include <cmath>

#include "ISA.hpp"

static const std::unordered_map<std::string, uint8_t> reg_name_map = {
    {"ax", 0}, {"bx", 1}, {"cx", 2}, {"dx", 3}, {"fax", 4}, {"fbx", 5}, {"fcx", 6}
};

static const std::unordered_map<std::string, uint8_t> instruction_name_map = {
    {INSTR_LOAD_STR, INSTR_LOAD}, {INSTR_LOADS_STR, INSTR_LOADS}, {INSTR_LOADC_STR, INSTR_LOADC},
    {INSTR_STORE_STR, INSTR_STORE}, {INSTR_STORES_STR, INSTR_STORES},
    {INSTR_COPY_STR, INSTR_COPY},
    {INSTR_ADD_STR, INSTR_ADD}, {INSTR_SUB_STR, INSTR_SUB}, {INSTR_MUL_STR, INSTR_MUL}, {INSTR_DIV_STR, INSTR_DIV},
    {INSTR_IDIV_STR, INSTR_IDIV}, {INSTR_SHL_STR, INSTR_SHL}, {INSTR_SHR_STR, INSTR_SHR},
    {INSTR_AND_STR, INSTR_AND}, {INSTR_OR_STR, INSTR_OR}, {INSTR_XOR_STR, INSTR_XOR}, {INSTR_NOT_STR, INSTR_NOT},
    {INSTR_FADD_STR, INSTR_FADD}, {INSTR_FSUB_STR, INSTR_FSUB}, {INSTR_FMUL_STR, INSTR_FMUL}, {INSTR_FDIV_STR, INSTR_FDIV},
    {INSTR_CMP_STR, INSTR_CMP}, {INSTR_CMPI_STR, INSTR_CMPI}, {INSTR_CMPF_STR, INSTR_CMPF},
    {INSTR_PUSH_STR, INSTR_PUSH}, {INSTR_POP_STR, INSTR_POP}, {INSTR_CALL_STR, INSTR_CALL}, {INSTR_RET_STR, INSTR_RET},
    {INSTR_SYSCALL_STR, INSTR_SYSCALL},
    {INSTR_STOP_STR, INSTR_STOP},
    {INSTR_JMP_STR, INSTR_JMP}, {INSTR_JMPZ_STR, INSTR_JMPZ}, {INSTR_JMPS_STR, INSTR_JMPS}, {INSTR_JMPC_STR, INSTR_JMPC}
};

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

bool is_token_data_directive(const std::string& token)
{
    return token == "[data]";
}

bool is_token_program_directive(const std::string& token)
{
    return token == "[program]";
}

enum class TokenType
{
    Unknown,

    DataDirective,
    ProgramDirective,

    Label,
    Instruction,
    Register,
    IntLiteral,
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
    };

    std::string text;

    int line;
};

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

int main(int argv, char** argc)
{
    if (argv < 2) return 1;

    std::vector<Token> tokens = parse_tokens_from_file(argc[1]);

    std::vector<uint8_t> bytecode(tokens.size() * 4, 0);

    // 8 bytes for program start addr (4) and data size (4)
    uint32_t bytecode_top_ptr = 8;

    std::unordered_map<std::string, uint32_t> data_ptrs;

    // Find any data blocks and store in bytecode
    bool data_mode = false;
    for (size_t token_idx = 0; token_idx < tokens.size(); token_idx++)
    {
        const Token& token = tokens[token_idx];

        if (!data_mode)
        {
            if (token.type == TokenType::DataDirective)
            {
                data_mode = true;
            }

            continue;
        }

        switch (token.type)
        {
            case TokenType::ProgramDirective:
            {
                data_mode = false;
                break;
            }
            case TokenType::Unknown:
            {
                if (data_ptrs.contains(token.text))
                {
                    std::cout << "ERROR: Found duplicate data label \"" << token.text << "\"\n";
                    break;
                }

                // Program will be loaded in from 0 memory in VM, subtract header
                data_ptrs[token.text] = bytecode_top_ptr - 8;
                break;
            }
            case TokenType::StringLiteral:
            {
                memcpy(&bytecode[bytecode_top_ptr], token.text.c_str(), token.text.length() + 1);
                bytecode_top_ptr += token.text.length() + 1;
                break;
            }
            case TokenType::IntLiteral:
            case TokenType::HexLiteral: // fallthrough
            {
                memcpy(&bytecode[bytecode_top_ptr], &token.value, 4);
                bytecode_top_ptr += token.text.length() + 4;
                break;
            }
        }
    }

    size_t data_size = bytecode_top_ptr - 8;

    std::unordered_map<std::string, uint32_t> label_ptrs;
    std::unordered_map<std::string, std::vector<uint32_t>> label_ref_ptrs;

    // Find existing labels
    for (size_t token_idx = 0; token_idx < tokens.size(); token_idx++)
    {
        const Token& token = tokens[token_idx];

        if (token.type == TokenType::Label)
        {
            if (label_ptrs.contains(token.text))
            {
                std::cout << "ERROR: Found duplicate label \"" << token.text << "\" on line " << token.line << "\n";
                return 1;
            }

            label_ptrs[token.text] = 0;
        }
    }

    if (!label_ptrs.contains("main"))
    {
        std::cout << "ERROR: Program does not contain main (.main label) entry point\n";
        return 1;
    }

    data_mode = false;
    for (size_t token_idx = 0; token_idx < tokens.size();)
    {
        const Token& token = tokens[token_idx];

        if (data_mode)
        {
            if (token.type == TokenType::ProgramDirective)
            {
                data_mode = false;
            }

            token_idx++;
            continue;
        }

        switch (token.type)
        {
            case TokenType::Label:
            {
                label_ptrs[token.text] = bytecode_top_ptr;

                token_idx++;
                break;
            }
            case TokenType::Instruction:
            {
                bytecode[bytecode_top_ptr] = token.instruction;

                bytecode_top_ptr++;
                token_idx++;
                break;
            }
            case TokenType::Register:
            {
                bytecode[bytecode_top_ptr] = token.reg_id;

                bytecode_top_ptr++;
                token_idx++;
                break;
            }
            case TokenType::IntLiteral:
            case TokenType::HexLiteral: // fallthrough
            {
                memcpy(&bytecode[bytecode_top_ptr], &token.value, 4);
                
                bytecode_top_ptr += 4;
                token_idx++;
                break;
            }
            case TokenType::Unknown:
            {
                // Test if label
                if (label_ptrs.contains(token.text))
                {
                    label_ref_ptrs[token.text].push_back(bytecode_top_ptr);
                    bytecode_top_ptr += 4;
                    token_idx++;
                    break;
                }
                
                // Test if data ptr
                if (data_ptrs.contains(token.text))
                {
                    memcpy(&bytecode[bytecode_top_ptr], &data_ptrs.at(token.text), 4);
                    bytecode_top_ptr += 4;
                    token_idx++;
                    break;
                }
                
                std::cout << "\nERROR: Could not assemble program (UNKNOWN TOKEN : " << token.text << ")\n";
                return 1;
            }
            case TokenType::DataDirective:
            {
                data_mode = true;
                break;
            }
        }
    }

    // Add labels
    for (auto iter = label_ref_ptrs.begin(); iter != label_ref_ptrs.end(); iter++)
    {
        const std::string& label = iter->first;
        const std::vector<uint32_t> refs = iter->second;

        uint32_t label_ptr = label_ptrs.at(label);

        for (uint32_t ref : refs)
        {
            memcpy(&bytecode[ref], &label_ptr, 4);
        }
    }

    // Store entry point
    memcpy(&bytecode[0], &label_ptrs.at("main"), 4);
    memcpy(&bytecode[4], &data_size, 4);

    std::ofstream out_file("out.vmex", std::ios::binary);
    for (uint32_t i = 0; i < bytecode_top_ptr; i++)
    {
        out_file << bytecode[i];
    }

    return 0;
}
#include "pattern.hpp"

const std::unordered_map<uint8_t, std::vector<std::vector<TokenType>>> instruction_token_patterns = {
    {INSTR_LOAD, {{TokenType::Register, TokenType::Register}}},
    {INSTR_LOADS, {{TokenType::Register, TokenType::IntLiteral}, {TokenType::Register, TokenType::HexLiteral}}},
    {INSTR_LOADC, {{TokenType::Register, TokenType::IntLiteral}, {TokenType::Register, TokenType::HexLiteral}, {TokenType::Register, TokenType::Unknown}}},
    {INSTR_STORE, {{TokenType::Register, TokenType::Register}}},
    {INSTR_STORES, {{TokenType::Register, TokenType::IntLiteral}, {TokenType::Register, TokenType::HexLiteral}}},
    {INSTR_COPY, {{TokenType::Register, TokenType::Register}}},
    {INSTR_ADD, {{TokenType::Register, TokenType::Register}}},
    {INSTR_SUB, {{TokenType::Register, TokenType::Register}}},
    {INSTR_MUL, {{TokenType::Register, TokenType::Register}}},
    {INSTR_DIV, {{TokenType::Register, TokenType::Register}}},
    {INSTR_IDIV, {{TokenType::Register, TokenType::Register}}},
    {INSTR_SHL, {{TokenType::Register, TokenType::Register}}},
    {INSTR_SHR, {{TokenType::Register, TokenType::Register}}},
    {INSTR_AND, {{TokenType::Register, TokenType::Register}}},
    {INSTR_OR, {{TokenType::Register, TokenType::Register}}},
    {INSTR_XOR, {{TokenType::Register, TokenType::Register}}},
    {INSTR_NOT, {{TokenType::Register}}},
    {INSTR_FADD, {{TokenType::Register, TokenType::Register}}},
    {INSTR_FSUB, {{TokenType::Register, TokenType::Register}}},
    {INSTR_FMUL, {{TokenType::Register, TokenType::Register}}},
    {INSTR_FDIV, {{TokenType::Register, TokenType::Register}}},
    {INSTR_CMP, {{TokenType::Register, TokenType::Register}}},
    {INSTR_CMPI, {{TokenType::Register, TokenType::Register}}},
    {INSTR_CMPF, {{TokenType::Register, TokenType::Register}}},
    {INSTR_PUSH, {{TokenType::Register}}},
    {INSTR_POP, {{TokenType::Register}}},
    {INSTR_CALL, {{TokenType::Unknown}}},
    {INSTR_RET, {}},
    {INSTR_SYSCALL, {{TokenType::IntLiteral}, {TokenType::HexLiteral}}},
    {INSTR_STOP, {}},
    {INSTR_JMP, {{TokenType::Unknown}}},
    {INSTR_JMPZ, {{TokenType::Unknown}}},
    {INSTR_JMPS, {{TokenType::Unknown}}},
    {INSTR_JMPC, {{TokenType::Unknown}}},
};

bool instruction_token_has_valid_pattern(const std::vector<Token>& tokens, int index)
{
    const Token& token = tokens.at(index);

    const std::vector<std::vector<TokenType>>& patterns = instruction_token_patterns.at(token.instruction);
    if (patterns.size() <= 0) return true;

    for (const std::vector<TokenType>& pattern : patterns)
    {
        if (index + pattern.size() >= tokens.size()) continue;
        
        bool valid = true;
        for (int i = 0; i < pattern.size(); i++)
        {
            if (tokens.at(index + i + 1).type != pattern.at(i))
            {
                valid = false;
                break;
            }
        }

        if (valid) return true;
    }

    return false;
}
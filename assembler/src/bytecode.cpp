#include <vector>
#include <unordered_map>
#include <iostream>
#include <fstream>

#include "bytecode.hpp"
#include "token.hpp"
#include "parse.hpp"
#include "pattern.hpp"

#include "ISA.hpp"
#include "syscall.hpp"
#include "bytes.hpp"

const uint32_t BYTECODE_HEADER_SIZE = 16;

bool _token_data_label_pass(const std::vector<Token>& tokens, std::vector<uint8_t>& bytecode, uint32_t& bytecode_top_ptr,
    std::unordered_map<std::string, uint32_t>& data_ptrs_out, std::unordered_map<std::string, uint32_t>& label_ptrs_out)
{
    // Find any data blocks and store in bytecode
    // Scan for labels
    bool data_mode = false;
    for (size_t token_idx = 0; token_idx < tokens.size(); token_idx++)
    {
        const Token& token = tokens[token_idx];

        if (token.type == TokenType::Label)
        {
            if (label_ptrs_out.contains(token.text))
            {
                std::cout << "ERROR: Found duplicate label \"" << token.text << "\" on line " << token.line << "\n";
                return false;
            }

            label_ptrs_out[token.text] = 0;
        }

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
                if (data_ptrs_out.contains(token.text))
                {
                    std::cout << "ERROR: Found duplicate data label \"" << token.text << "\"\n";
                    break;
                }

                // Program will be loaded in from 0 memory in VM, subtract header
                data_ptrs_out[token.text] = bytecode_top_ptr - BYTECODE_HEADER_SIZE;
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
                write_int(&bytecode[bytecode_top_ptr], token.value);
                bytecode_top_ptr += 4;
                break;
            }
            case TokenType::FloatLiteral:
            {
                uint32_t value = *(uint32_t*)&token.fvalue;
                write_int(&bytecode[bytecode_top_ptr], value);
                bytecode_top_ptr += 4;
                break;
            }
        }
    }

    return true;
}

bool _token_instruction_pass(const std::vector<Token>& tokens, std::vector<uint8_t>& bytecode, uint32_t& bytecode_top_ptr,
    const std::unordered_map<std::string, uint32_t>& data_ptrs, std::unordered_map<std::string, uint32_t>& label_ptrs,
    std::unordered_map<std::string, std::vector<uint32_t>>& label_ref_ptrs_out)
{
    bool data_mode = false;
    for (size_t token_idx = 0; token_idx < tokens.size(); token_idx++)
    {
        const Token& token = tokens[token_idx];

        if (data_mode)
        {
            if (token.type == TokenType::ProgramDirective)
            {
                data_mode = false;
            }
            continue;
        }

        switch (token.type)
        {
            case TokenType::Label:
            {
                label_ptrs[token.text] = bytecode_top_ptr;

                break;
            }
            case TokenType::Instruction:
            {
                if (!instruction_token_has_valid_pattern(tokens, token_idx))
                {
                    std::cout << "\nERROR: Could not assemble program - invalid instruction (" << static_cast<int>(token.instruction) <<
                        ") pattern on line " << token.line << "\n";
                    return 1;
                }

                bytecode[bytecode_top_ptr] = token.instruction;

                bytecode_top_ptr++;
                break;
            }
            case TokenType::Register:
            {
                bytecode[bytecode_top_ptr] = token.reg_id;

                bytecode_top_ptr++;
                break;
            }
            case TokenType::IntLiteral:
            case TokenType::HexLiteral: // fallthrough
            {
                write_int(&bytecode[bytecode_top_ptr], token.value);
                
                bytecode_top_ptr += 4;
                break;
            }
            case TokenType::Unknown:
            {
                // Test if label
                if (label_ptrs.contains(token.text))
                {
                    label_ref_ptrs_out[token.text].push_back(bytecode_top_ptr);
                    bytecode_top_ptr += 4;

                    break;
                }
                
                // Test if data ptr
                if (data_ptrs.contains(token.text))
                {
                    write_int(&bytecode[bytecode_top_ptr], data_ptrs.at(token.text));
                    bytecode_top_ptr += 4;

                    break;
                }
                
                std::cout << "\nERROR: Could not assemble program (UNKNOWN TOKEN : " << token.text << ")\n";
                return false;
            }
            case TokenType::DataDirective:
            {
                data_mode = true;
                break;
            }
        }
    }

    return true;
}

void _write_label_refs(std::vector<uint8_t>& bytecode, const std::unordered_map<std::string, uint32_t>& label_ptrs,
    const std::unordered_map<std::string, std::vector<uint32_t>>& label_ref_ptrs)
{
    for (auto iter = label_ref_ptrs.begin(); iter != label_ref_ptrs.end(); iter++)
    {
        const std::string& label = iter->first;
        const std::vector<uint32_t> refs = iter->second;

        uint32_t label_ptr = label_ptrs.at(label);

        for (uint32_t ref : refs)
        {
            write_int(&bytecode[ref], label_ptr);
        }
    }
}

void _write_header(std::vector<uint8_t>& bytecode, const std::unordered_map<std::string, uint32_t>& label_ptrs, uint32_t data_size)
{
    // Store ISA and syscall versions
    uint32_t isa_ver = ISA_version;
    uint32_t syscall_ver = SYSCALL_version;
    write_int(&bytecode[0], isa_ver);
    write_int(&bytecode[4], syscall_ver);

    // Store entry point and data size
    write_int(&bytecode[8], label_ptrs.at("main"));
    write_int(&bytecode[12], data_size);
}

bool assemble_file(std::string filepath)
{
    std::vector<Token> tokens = parse_tokens_from_file(filepath);

    std::vector<uint8_t> bytecode(tokens.size() * 4, 0);

    uint32_t bytecode_top_ptr = BYTECODE_HEADER_SIZE;
    
    std::unordered_map<std::string, uint32_t> data_ptrs;
    std::unordered_map<std::string, uint32_t> label_ptrs;
    
    if (!_token_data_label_pass(tokens, bytecode, bytecode_top_ptr, data_ptrs, label_ptrs))
    {
        std::cout << "ERROR: Data-label pass failed\n";
        return false;
    }

    uint32_t data_size = bytecode_top_ptr - BYTECODE_HEADER_SIZE;

    if (!label_ptrs.contains("main"))
    {
        std::cout << "ERROR: Program does not contain main (.main label) entry point\n";
        return false;
    }
    
    std::unordered_map<std::string, std::vector<uint32_t>> label_ref_ptrs;

    if (!_token_instruction_pass(tokens, bytecode, bytecode_top_ptr, data_ptrs, label_ptrs, label_ref_ptrs))
    {
        std::cout << "ERROR: Instruction pass failed\n";
        return false;
    }

    _write_label_refs(bytecode, label_ptrs, label_ref_ptrs);

    _write_header(bytecode, label_ptrs, data_size);

    // Get input file name
    std::string out_filepath = parse_file_path_out_file_name(filepath);

    std::cout << "Assembled file \"" << out_filepath << "\"\n";

    std::ofstream out_file(out_filepath, std::ios::binary);
    for (uint32_t i = 0; i < bytecode_top_ptr; i++)
    {
        out_file << bytecode[i];
    }

    return true;
}
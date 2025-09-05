#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "token.hpp"

bool assemble_file(std::string filepath);

bool _token_data_label_pass(const std::vector<Token>& tokens, std::vector<uint8_t>& bytecode, uint32_t& bytecode_top_ptr,
    std::unordered_map<std::string, uint32_t>& data_ptrs_out, std::unordered_map<std::string, uint32_t>& label_ptrs_out);

bool _token_instruction_pass(const std::vector<Token>& tokens, std::vector<uint8_t>& bytecode, uint32_t& bytecode_top_ptr,
    const std::unordered_map<std::string, uint32_t>& data_ptrs, std::unordered_map<std::string, uint32_t>& label_ptrs,
    std::unordered_map<std::string, std::vector<uint32_t>>& label_ref_ptrs_out);

void _write_label_refs(std::vector<uint8_t>& bytecode, const std::unordered_map<std::string, uint32_t>& label_ptrs,
    const std::unordered_map<std::string, std::vector<uint32_t>>& label_ref_ptrs);

void _write_header(std::vector<uint8_t>& bytecode, const std::unordered_map<std::string, uint32_t>& label_ptrs, uint32_t data_size);
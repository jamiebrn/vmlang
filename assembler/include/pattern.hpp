#pragma once

#include <unordered_map>
#include <vector>

#include "ISA.hpp"
#include "token.hpp"

extern const std::unordered_map<uint8_t, std::vector<std::vector<TokenType>>> instruction_token_patterns;

bool instruction_token_has_valid_pattern(const std::vector<Token>& tokens, int index);
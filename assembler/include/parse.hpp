#pragma once

#include <string>
#include <vector>

#include "token.hpp"

std::vector<Token> parse_tokens_from_file(std::string filepath);

std::string parse_file_path_out_file_name(std::string filepath);
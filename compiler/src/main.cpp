#include <unordered_map>
#include <string>
#include <iostream>
#include <vector>
#include <stdint.h>

enum TokenType
{
    Value,
    Operator
};

struct Token
{
    TokenType type;
    std::string value;
};

static const std::unordered_map<std::string, uint8_t> operator_precedence = {{"+", 1}, {"-", 1}, {"*", 2}, {"/", 2}};

struct ExpressionNode
{
    bool is_operator = false;
    std::string value;

    ExpressionNode* left = nullptr;
    ExpressionNode* right = nullptr;
};

ExpressionNode* parse_expression(const std::vector<Token>& tokens, size_t& idx, uint8_t precedence_level)
{
    ExpressionNode* left = new ExpressionNode;
    left->value = tokens[idx].value;
    idx++;

    if (left->value == "(")
    {
        delete left;
        left = parse_expression(tokens, idx, 0);
    }

    while (idx < tokens.size())
    {
        const Token& op = tokens[idx];
        if (op.value == ")")
        {
            idx++;
            break;
        }

        uint8_t prec = operator_precedence.at(op.value);

        if (prec <= precedence_level) break;

        idx++;

        ExpressionNode* right = parse_expression(tokens, idx, prec);

        ExpressionNode* new_node = new ExpressionNode;
        new_node->is_operator = true;
        new_node->value = op.value;
        new_node->left = left;
        new_node->right = right;
        left = new_node;
    }

    return left;
}

void print_expression(ExpressionNode* node, int depth = 0)
{
    if (depth > 0)
    {
        if (depth > 1)
        {
            std::cout << '|';
            std::cout << std::string((depth - 1) * 3 - 1, ' ');
        }
        std::cout << "\'- ";
    }
    std::cout << node->value << "\n";

    if (node->is_operator)
    {
        print_expression(node->left, depth + 1);
        print_expression(node->right, depth + 1);
    }
}

int evaluate_expression(ExpressionNode* node)
{
    if (!node->is_operator)
    {
        return std::atoi(node->value.c_str());
    }

    if (node->value == "+")
    {
        return evaluate_expression(node->left) + evaluate_expression(node->right);
    }
    if (node->value == "-")
    {
        return evaluate_expression(node->left) - evaluate_expression(node->right);
    }
    if (node->value == "*")
    {
        return evaluate_expression(node->left) * evaluate_expression(node->right);
    }
    if (node->value == "/")
    {
        return evaluate_expression(node->left) / evaluate_expression(node->right);
    }
    return 0;
}

bool is_num(char c)
{
    return c >= '0' && c <= '9';
}

bool is_operator(char c)
{
    return (c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')');
}

Token create_token(std::string token_buffer)
{
    Token token;
    if (token_buffer.size() == 1 && is_operator(token_buffer[0]))
    {
        token.type = TokenType::Operator;
        token.value = token_buffer;
        return token;
    }

    token.type = TokenType::Value;
    token.value = token_buffer;
    return token;
}


std::vector<Token> parse_tokens(const std::string& expression)
{
    int i = 0;
    std::vector<Token> tokens;
    while (i < expression.size())
    {
        char c = expression[i];
        if (is_operator(c))
        {
            tokens.push_back(create_token(std::string(1, c)));
            i++;
            continue;
        }
        else if (is_num(c))
        {
            std::string token_buffer;
            while (is_num(expression[i]))
            {
                token_buffer += expression[i];
                i++;
            }
            tokens.push_back(create_token(token_buffer));
            continue;
        }

        while (!is_operator(expression[i]) && !is_num(expression[i])) i++;
    }

    return tokens;
}

int main()
{
    while (true)
    {
        printf(" > ");
        std::string input;
        std::getline(std::cin, input);

        if (input.empty()) break;

        std::vector<Token> tokens = parse_tokens(input);
        // for (const Token& token : tokens)
        // {
        //     std::cout << token.value << "\n";
        // }
        // std::cout << "\n";

        size_t idx = 0;
        ExpressionNode* expression = parse_expression(tokens, idx, 0);
        print_expression(expression);
        printf("= %d\n", evaluate_expression(expression));
    }
}
#include "bytecode.hpp"

int main(int argv, char** argc)
{
    if (argv < 2) return 1;

    if (!assemble_file(argc[1])) return 1;

    return 0;
}
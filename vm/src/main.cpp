#include <SDL.h>

#include "VirtualMachine.hpp"

int main(int argc, char** argv)
{
    if (argc < 2) return 1;

    if (SDL_Init(SDL_INIT_VIDEO)) return 1;

    VirtualMachine virtual_machine;
    virtual_machine.load_program(argv[1]);

    virtual_machine.run();

    SDL_Quit();

    return 0;
}
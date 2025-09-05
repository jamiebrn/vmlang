#pragma once

#include <string>
#include <vector>

#include <SDL.h>

#define MACHINE_STACK_SIZE 2 * 1024 * 1024

struct VirtualWindow
{
    SDL_Window* window;
    SDL_Renderer* renderer;
};

class VirtualMachine
{
public:
    VirtualMachine();

    bool load_program(const std::string& filepath);

    void run();

private:
    void reset_flags();

    void* get_register(uint8_t id);

    void dispatch_syscall(uint8_t id);

    void process_instruction();

    uint32_t reg_a = 0;
    uint32_t reg_b = 0;
    uint32_t reg_c = 0;
    uint32_t reg_d = 0;
    uint32_t reg_stack_ptr = 0;
    uint32_t reg_base_ptr = 0;
    uint32_t reg_instruction_ptr = 0;

    float reg_fa = 0;
    float reg_fb = 0;
    float reg_fc = 0;

    bool flag_zero = 0;
    bool flag_sign = 0;
    bool flag_carry = 0;

    std::vector<uint8_t> memory;
    uint32_t heap_ptr;

    std::vector<uint8_t> program;

    std::vector<VirtualWindow> windows;

};
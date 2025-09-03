#include <iostream>
#include <vector>
#include <fstream>
#include <stdint.h>
#include <thread>

#include <SDL.h>

#include "ISA.hpp"
#include "syscall.hpp"
#include "bytes.hpp"

#define MACHINE_STACK_SIZE 2 * 1024 * 1024

#define PRINT_DEBUG 0

struct VirtualWindow
{
    SDL_Window* window;
    SDL_Renderer* renderer;
};

class VirtualMachine
{
public:
    VirtualMachine()
    {
        memory.resize(MACHINE_STACK_SIZE * 10, 0);
        // heap_ptr = MACHINE_STACK_SIZE;
    }

    bool load_program(const std::string& filepath)
    {
        std::ifstream file(filepath, std::ios::binary);
        
        if (!file.is_open())
        {
            return false;
        }

        file.seekg(0, std::ios::end);
        size_t length = file.tellg();
        file.seekg(0, std::ios::beg);

        program.resize(length);
        file.read((char*)program.data(), length);

        std::cout << "Loaded program of " << length << " bytes\n";
        return true;
    }

    void run()
    {
        uint32_t binary_isa_ver = load_int(&program[0]);
        uint32_t binary_syscall_ver = load_int(&program[4]);

        if (binary_isa_ver != ISA_version)
        {
            std::cout << "ERROR: Executable has different ISA version to runtime\n Executable ISA: " << binary_isa_ver <<
                "\n Runtime ISA: " << ISA_version << "\n";
            return;
        }

        if (binary_syscall_ver != SYSCALL_version)
        {
            std::cout << "WARNING: Executable has different syscall version to runtime\n Executable SYSCALL: " <<
                binary_syscall_ver << "\n Runtime SYSCALL: " << SYSCALL_version << "\n";
        }

        reg_instruction_ptr = load_int(&program[8]);
        std::fill(memory.begin(), memory.end(), 0);
        
        uint32_t program_data_size = load_int(&program[12]);
        
        // Load program data
        memcpy(&memory[0], &program[16], program_data_size);
        
        reg_base_ptr = program_data_size;
        reg_stack_ptr = program_data_size;

        std::cout << "Data size: " << program_data_size << "   IP: " << reg_instruction_ptr << "\n";

        while (reg_instruction_ptr < program.size())
        {
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_WINDOWEVENT)
                {
                    if (event.window.event == SDL_WINDOWEVENT_CLOSE)
                    {
                        // Find closed window
                        for (int i = 0; i < windows.size(); i++)
                        {
                            if (SDL_GetWindowID(windows[i].window) == event.window.windowID)
                            {
                                SDL_DestroyRenderer(windows[i].renderer);
                                SDL_DestroyWindow(windows[i].window);
                                windows[i].renderer = nullptr;
                                windows[i].window = nullptr;
                                break;
                            }
                        }
                    }
                }
            }

            process_instruction();
        }
    }

private:
    void reset_flags()
    {
        flag_zero = 0;
        flag_sign = 0;
        flag_carry = 0;
    }

    void* get_register(uint8_t id)
    {
        switch (id)
        {
            case 0: return &reg_a;
            case 1: return &reg_b;
            case 2: return &reg_c;
            case 3: return &reg_d;
            case 4: return &reg_fa;
            case 5: return &reg_fb;
            case 6: return &reg_fc;
        }

        return nullptr;
    }

    void dispatch_syscall(uint8_t id)
    {
        switch (id)
        {
            case SYSCALL_ID_WAIT:
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(reg_b));
                break;
            }
            case SYSCALL_ID_PRINTREG:
            {
                if (reg_b <= 3)
                {
                    printf("%d\n", *(uint32_t*)get_register(reg_b));
                }
                else
                {
                    printf("%f\n", *(float*)get_register(reg_b));
                }
                break;
            }
            case SYSCALL_ID_PRINTF:
            {
                printf((char*)&memory[reg_b]);
                break;
            }
            case SYSCALL_ID_WINDOW_CREATE:
            {
                VirtualWindow window;
                window.window = SDL_CreateWindow((char*)&memory[reg_d], SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                    reg_b, reg_c, 0);
                
                window.renderer = SDL_CreateRenderer(window.window, -1, 0);
                
                reg_a = windows.size();
                windows.push_back(window);
                break;
            }
            case SYSCALL_ID_WINDOW_CLOSE:
            {
                SDL_DestroyRenderer(windows[reg_b].renderer);
                SDL_DestroyWindow(windows[reg_b].window);
                windows[reg_b].renderer = nullptr;
                windows[reg_b].window = nullptr;
                break;
            }
            case SYSCALL_ID_WINDOW_IS_VALID:
            {
                reg_a = (windows.size() > reg_b && windows[reg_b].window) ? 1 : 0;
                break;
            }
            case SYSCALL_ID_WINDOW_SET_PIXEL:
            {
                SDL_SetRenderDrawColor(windows[reg_b].renderer, memory[reg_stack_ptr - 12],
                    memory[reg_stack_ptr - 8], memory[reg_stack_ptr - 4], 255);
                SDL_RenderDrawPoint(windows[reg_b].renderer, reg_c, reg_d);
                reg_stack_ptr -= 12;
                break;
            }
            case SYSCALL_ID_WINDOW_CLEAR:
            {
                SDL_SetRenderDrawColor(windows[reg_b].renderer, reg_c, reg_d,
                    memory[reg_stack_ptr - 4], 255);
                SDL_RenderClear(windows[reg_b].renderer);
                reg_stack_ptr -= 4;
                break;
            }
            case SYSCALL_ID_WINDOW_UPDATE:
            {
                SDL_RenderPresent(windows[reg_b].renderer);
                break;
            }
            case SYSCALL_ID_WINDOW_GET_MOUSE_X:
            {
                int mouse_x;
                SDL_GetMouseState(&mouse_x, NULL);
                reg_a = mouse_x;
                break;
            }
            case SYSCALL_ID_WINDOW_GET_MOUSE_Y:
            {
                int mouse_y;
                SDL_GetMouseState(NULL, &mouse_y);
                reg_a = mouse_y;
                break;
            }
            case SYSCALL_ID_WINDOW_GET_KEY_STATE:
            {
                reg_a = SDL_GetKeyboardState(NULL)[reg_b];
                break;
            }
        }
    }

    void process_instruction()
    {
        uint8_t instruction = program[reg_instruction_ptr];
        switch (instruction)
        {
            case INSTR_LOAD:
            {
                uint8_t reg_id = program[reg_instruction_ptr + 1];
                uint8_t addr_reg_id = program[reg_instruction_ptr + 2];
                *(uint32_t*)get_register(reg_id) = load_int(&memory[*(uint32_t*)get_register(addr_reg_id)]);
                reg_instruction_ptr += 3;

                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: LOAD " << *(uint32_t*)get_register(reg_id) << " into reg " << (int)reg_id << "\n";
                #endif

                break;
            }
            case INSTR_LOADS:
            {
                uint8_t reg_id = program[reg_instruction_ptr + 1];
                int32_t offset = *(int32_t*)&program[reg_instruction_ptr + 2];
                *(uint32_t*)get_register(reg_id) = load_int(&memory[reg_base_ptr + offset]);
                reg_instruction_ptr += 6;

                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: LOADS " << *(uint32_t*)get_register(reg_id) << " into reg " << (int)reg_id << "\n";
                #endif

                break;
            }
            case INSTR_LOADC:
            {
                uint8_t reg_id = program[reg_instruction_ptr + 1];
                uint32_t value = load_int(&program[reg_instruction_ptr + 2]);
                *(uint32_t*)get_register(reg_id) = value;
                reg_instruction_ptr += 6;

                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: LOADC " << value << " into reg " << (int)reg_id << "\n";
                #endif

                break;
            }
            case INSTR_STORE:
            {
                uint8_t reg_id = program[reg_instruction_ptr + 1];
                uint8_t addr_reg_id = program[reg_instruction_ptr + 2];
                uint32_t addr = *(uint32_t*)get_register(addr_reg_id);
                write_int(&memory[addr], *(uint32_t*)get_register(reg_id));
                reg_instruction_ptr += 3;

                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: STORE " << *(uint32_t*)get_register(reg_id) << " in addr " << addr << "\n";
                #endif

                break;
            }
            case INSTR_STORES:
            {
                uint8_t reg_id = program[reg_instruction_ptr + 1];
                uint32_t addr = reg_base_ptr + load_int(&program[reg_instruction_ptr + 2]);
                write_int(&memory[addr], *(uint32_t*)get_register(reg_id));
                reg_instruction_ptr += 6;

                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: STORES " << *(uint32_t*)get_register(reg_id) << " in stack addr " << addr << "\n";
                #endif

                break;
            }
            case INSTR_COPY:
            {
                uint8_t reg_src_id = program[reg_instruction_ptr + 1];
                uint8_t reg_dest_id = program[reg_instruction_ptr + 2];

                bool casted = false;
                if (reg_src_id <= 3)
                {
                    if (reg_dest_id > 3)
                    {
                        float value = (float)*(uint32_t*)get_register(reg_src_id);
                        memcpy(get_register(reg_dest_id), &value, 4);
                        casted = true;
                    }
                }
                else
                {
                    if (reg_dest_id <= 3)
                    {
                        uint32_t value = (uint32_t)*(float*)get_register(reg_src_id);
                        memcpy(get_register(reg_dest_id), &value, 4);
                        casted = true;
                    }
                }

                if (!casted)
                {
                    memcpy(get_register(reg_dest_id), get_register(reg_src_id), 4);
                }

                reg_instruction_ptr += 3;

                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: COPY reg " << (int)reg_src_id << " to reg " << (int)reg_dest_id << "\n";
                #endif

                break;
            }
            case INSTR_ADD:
            {
                uint8_t reg_a_id = program[reg_instruction_ptr + 1];
                uint8_t reg_b_id = program[reg_instruction_ptr + 2];
                uint32_t result = *(uint32_t*)get_register(reg_a_id) + *(uint32_t*)get_register(reg_b_id);
                reg_a = result;

                reg_instruction_ptr += 3;

                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: ADD reg " << (int)reg_a_id << " and reg " << (int)reg_b_id << " (" << result << ")\n";
                #endif

                break;
            }
            case INSTR_SUB:
            {
                uint8_t reg_a_id = program[reg_instruction_ptr + 1];
                uint8_t reg_b_id = program[reg_instruction_ptr + 2];
                uint32_t result = *(uint32_t*)get_register(reg_a_id) - *(uint32_t*)get_register(reg_b_id);
                reg_a = result;

                reg_instruction_ptr += 3;

                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: SUB reg " << (int)reg_b_id << " from reg " << (int)reg_a_id << " (" << result << ")\n";
                #endif

                break;
            }
            case INSTR_MUL:
            {
                uint8_t reg_a_id = program[reg_instruction_ptr + 1];
                uint8_t reg_b_id = program[reg_instruction_ptr + 2];
                uint32_t result = *(uint32_t*)get_register(reg_a_id) * *(uint32_t*)get_register(reg_b_id);
                reg_a = result;

                reg_instruction_ptr += 3;

                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: MUL reg " << (int)reg_a_id << " and reg " << (int)reg_b_id << " (" << result << ")\n";
                #endif

                break;
            }
            case INSTR_DIV:
            {
                uint8_t reg_a_id = program[reg_instruction_ptr + 1];
                uint8_t reg_b_id = program[reg_instruction_ptr + 2];
                uint32_t result = *(uint32_t*)get_register(reg_a_id) / *(uint32_t*)get_register(reg_b_id);
                reg_a = result;

                reg_instruction_ptr += 3;

                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: DIV reg " << (int)reg_a_id << " by reg " << (int)reg_b_id << " (" << result << ")\n";
                #endif

                break;
            }
            case INSTR_IDIV:
            {
                uint8_t reg_a_id = program[reg_instruction_ptr + 1];
                uint8_t reg_b_id = program[reg_instruction_ptr + 2];
                int32_t result = *(int32_t*)get_register(reg_a_id) / *(int32_t*)get_register(reg_b_id);
                memcpy(&reg_a, &result, 4);

                reg_instruction_ptr += 3;

                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: IDIV reg " << (int)reg_a_id << " by reg " << (int)reg_b_id << " (" << result << ")\n";
                #endif

                break;
            }
            case INSTR_SHL:
            {
                uint8_t reg_a_id = program[reg_instruction_ptr + 1];
                uint8_t reg_b_id = program[reg_instruction_ptr + 2];
                uint32_t result = *(uint32_t*)get_register(reg_a_id) << *(uint32_t*)get_register(reg_b_id);
                reg_a = result;

                reg_instruction_ptr += 3;
                
                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: SHL\n";
                #endif

                break;
            }
            case INSTR_SHR:
            {
                uint8_t reg_a_id = program[reg_instruction_ptr + 1];
                uint8_t reg_b_id = program[reg_instruction_ptr + 2];
                uint32_t result = *(uint32_t*)get_register(reg_a_id) >> *(uint32_t*)get_register(reg_b_id);
                reg_a = result;

                reg_instruction_ptr += 3;

                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: SHR\n";
                #endif

                break;
            }
            case INSTR_AND:
            {
                uint8_t reg_a_id = program[reg_instruction_ptr + 1];
                uint8_t reg_b_id = program[reg_instruction_ptr + 2];
                uint32_t result = *(uint32_t*)get_register(reg_a_id) & *(uint32_t*)get_register(reg_b_id);
                reg_a = result;

                reg_instruction_ptr += 3;

                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: AND reg " << (int)reg_a_id << " and reg " << (int)reg_b_id << " (" << result << ")\n";
                #endif

                break;
            }
            case INSTR_OR:
            {
                uint8_t reg_a_id = program[reg_instruction_ptr + 1];
                uint8_t reg_b_id = program[reg_instruction_ptr + 2];
                uint32_t result = *(uint32_t*)get_register(reg_a_id) | *(uint32_t*)get_register(reg_b_id);
                reg_a = result;

                reg_instruction_ptr += 3;

                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: OR reg " << (int)reg_b_id << " from reg " << (int)reg_a_id << " (" << result << ")\n";
                #endif

                break;
            }
            case INSTR_XOR:
            {
                uint8_t reg_a_id = program[reg_instruction_ptr + 1];
                uint8_t reg_b_id = program[reg_instruction_ptr + 2];
                uint32_t result = *(uint32_t*)get_register(reg_a_id) ^ *(uint32_t*)get_register(reg_b_id);
                reg_a = result;

                reg_instruction_ptr += 3;

                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: XOR reg " << (int)reg_a_id << " and reg " << (int)reg_b_id << " (" << result << ")\n";
                #endif

                break;
            }
            case INSTR_NOT:
            {
                uint8_t reg_id = program[reg_instruction_ptr + 1];
                reg_a = *(uint32_t*)get_register(reg_id);

                reg_instruction_ptr += 2;

                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: NOT reg " << (int)reg_id << " (" << reg_a << ")\n";
                #endif

                break;
            }
            case INSTR_FADD:
            {
                uint8_t reg_a_id = program[reg_instruction_ptr + 1];
                uint8_t reg_b_id = program[reg_instruction_ptr + 2];
                float result = *(float*)get_register(reg_a_id) + *(float*)get_register(reg_b_id);
                reg_fa = result;

                reg_instruction_ptr += 3;

                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: FADD\n";
                #endif

                break;
            }
            case INSTR_FSUB:
            {
                uint8_t reg_a_id = program[reg_instruction_ptr + 1];
                uint8_t reg_b_id = program[reg_instruction_ptr + 2];
                float result = *(float*)get_register(reg_a_id) - *(float*)get_register(reg_b_id);
                reg_fa = result;

                reg_instruction_ptr += 3;

                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: FSUB\n";
                #endif

                break;
            }
            case INSTR_FMUL:
            {
                uint8_t reg_a_id = program[reg_instruction_ptr + 1];
                uint8_t reg_b_id = program[reg_instruction_ptr + 2];
                float result = *(float*)get_register(reg_a_id) * *(float*)get_register(reg_b_id);
                reg_fa = result;

                reg_instruction_ptr += 3;
                
                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: FMUL\n";
                #endif

                break;
            }
            case INSTR_FDIV:
            {
                uint8_t reg_a_id = program[reg_instruction_ptr + 1];
                uint8_t reg_b_id = program[reg_instruction_ptr + 2];
                float result = *(float*)get_register(reg_a_id) / *(float*)get_register(reg_b_id);
                reg_fa = result;

                reg_instruction_ptr += 3;

                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: FDIV\n";
                #endif

                break;
            }
            case INSTR_CMP:
            {
                reset_flags();

                uint32_t reg_a_value = *(uint32_t*)get_register(program[reg_instruction_ptr + 1]);
                uint32_t reg_b_value = *(uint32_t*)get_register(program[reg_instruction_ptr + 2]);

                if (reg_a_value == reg_b_value)
                {
                    flag_zero = 1;
                }
                else
                {
                    flag_sign = reg_a_value > reg_b_value ? 0 : 1;
                }

                reg_instruction_ptr += 3;
                
                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: CMP " << reg_a_value << " to " << reg_b_value << " (" <<
                    flag_zero << " " << flag_sign << ")\n";
                #endif

                break;
            }
            case INSTR_CMPI:
            {
                reset_flags();

                int32_t reg_a_value = *(int32_t*)get_register(program[reg_instruction_ptr + 1]);
                int32_t reg_b_value = *(int32_t*)get_register(program[reg_instruction_ptr + 2]);

                if (reg_a_value == reg_b_value)
                {
                    flag_zero = 1;
                }
                else
                {
                    flag_sign = reg_a_value > reg_b_value ? 0 : 1;
                }

                reg_instruction_ptr += 3;
                
                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: CMPI\n";
                #endif

                break;
            }
            case INSTR_CMPF:
            {
                reset_flags();

                float reg_a_value = *(float*)get_register(program[reg_instruction_ptr + 1]);
                float reg_b_value = *(float*)get_register(program[reg_instruction_ptr + 2]);

                if (reg_a_value == reg_b_value)
                {
                    flag_zero = 1;
                }
                else
                {
                    flag_sign = reg_a_value > reg_b_value ? 0 : 1;
                }

                reg_instruction_ptr += 3;
                
                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: CMPF\n";
                #endif

                break;
            }
            case INSTR_PUSH:
            {
                uint8_t reg_id = program[reg_instruction_ptr + 1];
                write_int(&memory[reg_stack_ptr], *(uint32_t*)get_register(reg_id));
                reg_stack_ptr += 4;

                reg_instruction_ptr += 2;

                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: PUSH\n";
                #endif

                break;
            }
            case INSTR_POP:
            {
                uint8_t reg_id = program[reg_instruction_ptr + 1];
                reg_stack_ptr -= 4;
                *(uint32_t*)get_register(reg_id) = load_int(&memory[reg_stack_ptr]);

                reg_instruction_ptr += 2;

                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: POP\n";
                #endif

                break;
            }
            case INSTR_CALL:
            {
                uint32_t instr = load_int(&program[reg_instruction_ptr + 1]);
                uint32_t reg_instruction_ptr_next = reg_instruction_ptr + 5;

                write_int(&memory[reg_stack_ptr], reg_instruction_ptr_next);
                reg_stack_ptr += 4;

                write_int(&memory[reg_stack_ptr], reg_base_ptr);
                reg_stack_ptr += 4;

                reg_base_ptr = reg_stack_ptr - 8;

                reg_instruction_ptr = instr;

                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: CALL\n";
                #endif

                break;
            }
            case INSTR_RET:
            {
                reg_stack_ptr = reg_base_ptr;
                reg_instruction_ptr = load_int(&memory[reg_stack_ptr]);
                reg_base_ptr = load_int(&memory[reg_stack_ptr + 4]);

                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: RET\n";
                #endif

                break;
            }
            case INSTR_SYSCALL:
            {
                uint8_t syscall_id = program[reg_instruction_ptr + 1];

                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: SYSCALL id " << (int)syscall_id << "\n";
                #endif
                
                dispatch_syscall(syscall_id);

                reg_instruction_ptr += 5;

                break;
            }
            case INSTR_STOP:
            {
                // End of program
                reg_instruction_ptr = program.size();

                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: STOP\n";
                #endif

                break;
            }
            case INSTR_JMP:
            {
                uint32_t addr = load_int(&program[reg_instruction_ptr + 1]);
                reg_instruction_ptr = addr;

                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: JMP to addr " << addr << "\n";
                #endif

                break;
            }
            case INSTR_JMPZ:
            {
                if (!flag_zero)
                {
                    reg_instruction_ptr += 5;
                    break;
                }

                uint32_t addr = load_int(&program[reg_instruction_ptr + 1]);
                reg_instruction_ptr = addr;

                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: JMPZ to addr " << addr << "\n";
                #endif

                break;
            }
            case INSTR_JMPS:
            {
                if (!flag_sign)
                {
                    reg_instruction_ptr += 5;
                    break;
                }

                uint32_t addr = load_int(&program[reg_instruction_ptr + 1]);
                reg_instruction_ptr = addr;

                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: JMPS to addr " << addr << "\n";
                #endif

                break;
            }
            case INSTR_JMPC:
            {
                if (!flag_carry)
                {
                    reg_instruction_ptr += 5;
                    break;
                }

                uint32_t addr = load_int(&program[reg_instruction_ptr + 1]);
                reg_instruction_ptr = addr;

                #if PRINT_DEBUG
                std::cout << "INSTRUCTION: JMPS to addr " << addr << "\n";
                #endif

                break;
            }
        }
    }

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
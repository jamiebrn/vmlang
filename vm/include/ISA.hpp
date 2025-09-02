#pragma once

#define ISA_version 1

#define INSTR_LOAD 0x00
#define INSTR_LOAD_STR "load"

#define INSTR_LOADS 0x01
#define INSTR_LOADS_STR "loads"

#define INSTR_LOADC 0x02
#define INSTR_LOADC_STR "loadc"

#define INSTR_STORE 0x10
#define INSTR_STORE_STR "store"

#define INSTR_STORES 0x11
#define INSTR_STORES_STR "stores"

#define INSTR_COPY 0x20
#define INSTR_COPY_STR "copy"

#define INSTR_ADD 0x30
#define INSTR_ADD_STR "add"

#define INSTR_SUB 0x31
#define INSTR_SUB_STR "sub"

#define INSTR_MUL 0x32
#define INSTR_MUL_STR "mul"

#define INSTR_DIV 0x33
#define INSTR_DIV_STR "div"

#define INSTR_IDIV 0x37
#define INSTR_IDIV_STR "idiv"

#define INSTR_SHL 0x38
#define INSTR_SHL_STR "shl"

#define INSTR_SHR 0x39
#define INSTR_SHR_STR "shr"

#define INSTR_AND 0x3A
#define INSTR_AND_STR "and"

#define INSTR_OR 0x3B
#define INSTR_OR_STR "or"

#define INSTR_XOR 0x3C
#define INSTR_XOR_STR "xor"

#define INSTR_NOT 0x3D
#define INSTR_NOT_STR "not"

#define INSTR_FADD 0x40
#define INSTR_FADD_STR "fadd"

#define INSTR_FSUB 0x41
#define INSTR_FSUB_STR "fsub"

#define INSTR_FMUL 0x42
#define INSTR_FMUL_STR "fmul"

#define INSTR_FDIV 0x43
#define INSTR_FDIV_STR "fdiv"

#define INSTR_CMP 0x50
#define INSTR_CMP_STR "cmp"

#define INSTR_CMPI 0x51
#define INSTR_CMPI_STR "cmpi"

#define INSTR_CMPF 0x52
#define INSTR_CMPF_STR "cmpf"

#define INSTR_PUSH 0x60
#define INSTR_PUSH_STR "push"

#define INSTR_POP 0x61
#define INSTR_POP_STR "pop"

#define INSTR_CALL 0x62
#define INSTR_CALL_STR "call"

#define INSTR_RET 0x63
#define INSTR_RET_STR "ret"

#define INSTR_SYSCALL 0x64
#define INSTR_SYSCALL_STR "syscall"

#define INSTR_STOP 0xFF
#define INSTR_STOP_STR "stop"

#define INSTR_JMP 0x70
#define INSTR_JMP_STR "jmp"

#define INSTR_JMPZ 0x71
#define INSTR_JMPZ_STR "jmpz"

#define INSTR_JMPS 0x72
#define INSTR_JMPS_STR "jmps"

#define INSTR_JMPC 0x73
#define INSTR_JMPC_STR "jmpc"
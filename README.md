# vmlang
A basic 32 bit virtual machine, read [the specs](specs.txt) for the ISA.

Has a simple assembler, check [sample programs](example) for reference.

Will probably write a compiler for a custom high level language for this machine (eventually)

### Writing programs
Each program starts with the `.main` label (a program will not compile without this).

Program data can be stored using the `[data]` directive, and instructions with the `[program]` directive.
These can be used throughout the file to switch between data storage and program modes.

Data/label order does not matter - the assembler first passes through the file and gets data offsets etc.

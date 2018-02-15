#include "c8.h"

void c8::initialize() {
    /* remember the first 512 bytes (upto 0x200) are for chip8 interpreter */
    pc = 0x200;
    opcode = 0;
    I = 0;
    sp = 0;
}

void c8::emulateCycle() {
    /* First we fetch the opcode which is in memory */
    opcode = (memory[pc] << 8) | (memory[pc+1]);


    pc += 2;


}
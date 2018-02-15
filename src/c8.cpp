#include <time.h>
#include <random>
#include <iostream>
#include "c8.h"

unsigned char chip8_fontset[80] =
        {
                0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
                0x20, 0x60, 0x20, 0x20, 0x70, // 1
                0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
                0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
                0x90, 0x90, 0xF0, 0x10, 0x10, // 4
                0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
                0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
                0xF0, 0x10, 0x20, 0x40, 0x40, // 7
                0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
                0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
                0xF0, 0x90, 0xF0, 0x90, 0x90, // A
                0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
                0xF0, 0x80, 0x80, 0x80, 0xF0, // C
                0xE0, 0x90, 0x90, 0x90, 0xE0, // D
                0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
                0xF0, 0x80, 0xF0, 0x80, 0x80  // F
        };

c8::c8() {
    /* empty for now */
}

c8::~c8() {
    /* empty for now */
}


void c8::initialize() {
    /* remember the first 512 bytes (upto 0x200) are for chip8 interpreter */
    pc = 0x200; /* reset program counter */
    opcode = 0; /* reset opcode */
    I = 0; /* reset address register */
    sp = 0; /* reset stack pointer */

    for(int i=0; i<2048; i++) {
        gfx[i] = 0;     /* reset display */
    }

    for(int i=0; i<16; i++) {
        stack[i] = 0; /* reset stack */
        V[i] = 0;     /* reset registers */
    }

    for(int i=0; i<4096; i++) {
        memory[i] = 0;  /* reset all memory */
    }

    for(int i=0; i<80; i++) {
        memory[i] = chip8_fontset[i]; /* set address 0-79 the chip-8 font */
    }

    /* reset timers */
    delayTimer = 0;
    soundTimer = 0;

    srand(time(nullptr));   /* initialize seed for random */
}

bool c8::load(const char *filepath) {
    printf("Resetting memories...\n");
    initialize();

    printf("Loading ROM: %s...\n", filepath);
    FILE *file = fopen(filepath, "rb"); /* open ROM binary file */
    if(file == nullptr) {
        std::cerr << "There is no such file..." << std::endl;
        return false;
    }

    fseek(file, 0, SEEK_END); /* set indicator to end of file */
    int numBytes = ftell(file); /* get number of bytes from beginning of file to indicator (which is at the end) */
    rewind(file); /* set indicator back to beginning of file */
    printf("Filesize: %d bytes \n", numBytes);

    char *rom = (char *) malloc(sizeof(char) * numBytes);
    if(rom == nullptr) {
        std::cerr << "There is not enough memory to allocate ROM..." << std::endl;
        return false;
    }

    size_t bytesRead = fread((void *) rom, sizeof(char), numBytes, file);
    if(bytesRead != numBytes * sizeof(char)) {
        std::cerr << "Was unable to read all the binary from ROM..." << std::endl;
        return false;
    }


    /* 4096 memory size, 512 chip-8 interpreter, 256 display refresh, 96 for call stack */
    if((4096 - 512 - 256 - 96 ) >= bytesRead) {
        for (int i = 0; i < numBytes; i++) {
            memory[i + 512] = rom[i];   /* set memory starting from 512 (incrementing by 1) to ROM binary
 *                                     note that we're copying 1 byte at a time */
        }
    } else {
        std::cerr << "The size of ROM is too big to fit into 4MB memory..." << std::endl;
        return false;
    }

    free(rom); /* free binary rom allocated */
    fclose(file); /* close file */

    return true;
}

void c8::emulateCycle() {
    /* First we fetch the opcode which is in memory */
    opcode = (memory[pc] << 8) | (memory[pc+1]);


    pc += 2;


}
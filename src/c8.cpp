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

    /* We decode the opcode here
       We first look at first 4 bits */
    switch(opcode & 0xF000) {

        case 0x0000: {      /* if the first 4 bits is 0 */
            switch (opcode & 0x000F) {   /* look at the last 4 bits */
                case 0x0000:        /* 0X00E0: clears the screen */
                    for (int i = 0; i < 2048; i++) {
                        gfx[i] = 0;
                    }
                    drawFlag = true;
                    pc += 2;
                    break;

                case 0x000E:        /* 0X00EE: return from subroutine */
                    pc = stack[--sp];
                    pc += 2;
                    break;

                default:
                    /* print opcode in hexadecimal */
                    printf("Unknown opcode 0x%X\n", opcode);
            }
            break;
        }

        case 0x1000: {   /* if the first 4 bits is 1, 0x1NNN: Jumps to address NNN */
            pc = opcode & 0x0FFF;
            break;
        }


        case 0x2000: {    /* if the first 4 bits is 2, 0x2NNN: Calls subroutine at NNN */
            stack[sp++] = pc;
            pc = opcode & 0X0FFF;
            break;
        }

        case 0x3000: {    /* if the first 4 bits is 3, 0x3XNN: skips next instr if VX = NN */
            if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
                pc += 4;    /* if true skip next instr */
            } else {
                pc += 2;    /* otherwise, go to next instr */
            }
            break;
        }

        case 0x4000: {    /* if the first 4 bits is 4, 0x4XNN: skips next instr if VX != NN */
            if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
                pc += 4;    /* if true skip next instr */
            } else {
                pc += 2;    /* otherwise, go to next instr */
            }
            break;
        }

        case 0x5000: {   /* if the first 4 bits is 5, 0x5XY0: skips next instr if VX == VY */
            if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4]) {
                pc += 4;    /* if true skip next instr */
            } else {
                pc += 2;    /* otherwise, go to next instr */
            }
            break;
        }

        case 0x6000: {   /* if the first 4 bits is 6, 0x6XNN: sets VX to NN */
            V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
            pc += 2;
            break;
        }

        case 0x7000: {    /* if the first 4 bits is 7, 0x7XNN: Adds NN to VX */
            V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
            pc += 2;
            break;
        }

        case 0x8000: {    /* if the first 4 bits is 8, we have to see the last 4 bits */
            switch (opcode & 0x000F) {
                case 0x0000: {    /* 0x8XY0: sets VX to the value of VY */
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                }

                case 0x0001: {    /* 0x8XY1: sets VX to (VX or VY) */
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] | V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                }

                case 0x0002: {    /* 0x8XY2: sets VX to (VX and VY) */
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] & V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                }

                case 0x0003: {    /* 0x8XY3: sets VX to (VX xor VY) */
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] ^ V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                }

                case 0x0004: {    /* 0x8XY4: VX = VX + VY. VF set to 1 when there's a carry. 0 Otherwise */
                    if (V[(opcode & 0x0F00) >> 8] > (0xFF - V[(opcode & 0x00F0) >> 4])) {     /* VX > 255 - VY */
                        V[0xF] = 1; /* carry flag, aka: the 9th bit */
                    } else {
                        V[0xF] = 0;
                    }
                    V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                }

                case 0x0005: {    /* 0x8XY5: VX = VX - VY. VF set to 0 when there's a borrow. 1 Otherwise */
                    if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8]) {        /* VY > VX */
                        V[0xF] = 0;
                    } else {
                        V[0xF] = 1;
                    }
                    V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                }

                case 0x0006: {    /* 0x8XY6: Set VX to VX shift right by 1.
 *                                 VF set to least sig bit of VX before the shift */
                    V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;   /* this is 0 or 1 */
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] >> 1;
                    pc += 2;
                    break;
                }

                case 0x0007: {    /* 0x8XY7: VX = VY - VX. VF set to 0 when there's a borrow. 1 otherwise. */
                    if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4]) {        /* VY > VX */
                        V[0xF] = 0;
                    } else {
                        V[0xF] = 1;
                    }
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;
                }

                case 0x000E: {    /* 0x8XYE: Set VX to VX shift left by 1.
 *                                         VF is set to the most sig bit of VX before the shift */
                    V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] << 1;
                    pc += 2;
                    break;
                }

                default:
                    /* print opcode in hexadecimal */
                    printf("Unknown opcode: 0x%X\n", opcode);
            }
            break;
        }

        case 0x9000: {    /* if the first 4 bits is 9, 9XY0: skip next instr if VX != VY. */
            if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;
        }

        case 0xA000: {    /* if the first 4 bits is A, 0xANNN: set I=NNN */
            I = opcode & 0x0FFF;    /* the bit operation results in 12 bits */
            pc += 2;
            break;
        }

        case 0xB000: {    /* if the first 4 bits is B, 0xBNNN: set PC = V0 + NNN */
            pc = V[0] + (opcode & 0x0FFF);  /* 8 bits addition with 12 bits fit into 16 bits */
            break;
        }

        case 0xC000: {    /* if the first 4 bits is C, 0xCXNN: set VX = rand() & NN. rand() should be 0-255 (8bits). */
            V[(opcode & 0x0F00) >> 8] = (rand() % 256) & (opcode & 0x00FF); /* 0-255 is 8 bits AND with 8 bits */
            pc += 2;
            break;
        }

        case 0xD000: {    /* if the first 4 bits is D, 0xDXYN: draw a sprite at (VX,VY).
                           Should be 8 pixels wide and N pixels high.
                           I should be pointing to the base sprite address we want to draw */
            unsigned short x = V[(opcode & 0x0F00) >> 8];   /* TODO: maybe try using unsigned char for all? */
            unsigned short y = V[(opcode & 0x00F0) >> 4];
            unsigned short height = opcode & 0x000F;
            unsigned short pixel;

            V[0xF] = 0;
            for (int i = 0; i < height; i++) {
                pixel = memory[I + i];
                for (int j = 0; j < 8; j++) {
                    if ((pixel & (0x80 >> j)) != 0) {    /* if sprite pixel bit we want to draw is 1 */
                        if (gfx[x + j + ((y + i) * 64)] == 1) {   /* AND the pixel bit at the screen is 1 */
                            V[0xF] = 1;
                        }
                        gfx[x + j + ((y + i) * 64)] = gfx[x + j + ((y + i) * 64)] ^ 1;
                    }
                }
            }
            drawFlag = true;
            pc += 2;
            break;
        }

        case 0xE000: {   /* if the first 4 bits is E, need to check last 4 or 8 bits */
            switch (opcode & 0x00FF) {
                case 0x009E: {    /* if the last 8 bits is 9E, 0xEX9E: skip next instr if key in VX is pressed */
                    if (key[V[(opcode & 0x0F00) >> 8]] != 0) {
                        pc += 4;
                    } else {
                        pc += 2;
                    }
                    break;
                }

                case 0x00A1: {    /* if the last 8 bits is A1, 0xEXA1: skip next instr if key in VX isn't pressed */
                    if (key[V[(opcode & 0x0F00) >> 8]] == 0) {
                        pc += 4;
                    } else {
                        pc += 2;
                    }
                    break;
                }

                default:
                    printf("Unknown opcode: 0x%X\n", opcode);
            }
            break;
        }

        case 0xF000: {  /* if the first 4 bits is F look at the last 8 bits */
            switch(opcode & 0x00FF) {
                case 0x0007: {  /* 0xFX07: sets VX to the value of delay timer */
                    V[(opcode & 0x0F00) >> 8] = delayTimer;
                    pc += 2;
                    break;
                }

                case 0x000A: { /* 0xFX0A: A key press is awaited, then stored in VX */
                    bool keyPress = false;
                    for(int i=0; i<16; i++) {
                        if(key[i] != 0) {
                            V[(opcode & 0x0F00) >> 8] = i;
                            keyPress = true;
                        }
                    }

                    if(!keyPress) {
                        return;
                    }
                    pc += 2;
                    break;
                }

                case 0x0015: { /* 0xFX15: Sets the delay timer to VX */
                    delayTimer = V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;
                }

                case 0x0018: { /* 0xFX18: Sets the sound timer to VX */
                    soundTimer = V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;
                }

                case 0x001E: { /* 0xFX1E: Adds VX to I */
                    if(I + (V[opcode & 0x0F00] >> 8) > 0xFFFF) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    I += V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;
                }

                case 0x0029: {  /* 0xFX29: Sets I to the location
                                   of the sprite for the character in VX.
                                   Characters 0-F (in hexadecimal) are represented by a 4x5 font */
                    I = 5 * V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;
                }

                case 0x0033: {  /* 0xFX33 */
                    memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
                    memory[I+1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
                    memory[I+2] = V[(opcode & 0x0F00) >> 8] % 10;
                    pc += 2;
                    break;
                }

                case 0x0055: { /* 0xFX55: reg_dump to mem */
                    for(int i=0; i<= ((opcode & 0x0F00) >> 8); i++) {
                        memory[I + i] = V[i];
                    }
                    I += ((opcode & 0x0F00) >> 8) + 1;
                    pc += 2;
                    break;
                }

                case 0x0065: { /* 0xFX65: reg_load from mem */
                    for (int i = 0; i <= ((opcode & 0x0F00) >> 8); i++) {
                        V[i] = memory[I + i];
                    }
                    I += ((opcode & 0x0F00) >> 8) + 1;
                    pc += 2;
                    break;
                }
            }
        }



        default:
            /* print opcode in hexadecimal */
            printf("Unknown opcode: 0x%X\n", opcode);
    }



}
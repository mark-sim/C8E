#ifndef C8E_C8_H
#define C8E_C8_H


class c8 {
    private:

    unsigned short opcode;  /* This is 16 bits (2 bytes) opcode */
    unsigned char memory[4096]; /* This is the amount of memory for chip8 */
    unsigned char V[16];  /* 16 registers each 8 bits (1byte). V0 - VE and VF(carry flag) */

    /* There are two special registers, PC and I which are both 16 bits (2 bytes) */
    unsigned short pc; /* keeps track of address of memory */
    unsigned short I; /* address register */

    unsigned char gfx[64 * 32]; /* each value keeps state of display (pixels) 0 or 1 */

    /* Two timer registers that count down at 60Hz */
    unsigned char delayTimer;
    unsigned char soundTimer;

    /* We also need a stack (which is 16 levels in our case and only keeps the return address which is 16 bits)
       we also need a stack pointer to know at which level we're in */
    unsigned short stack[16];
    unsigned short sp;

    /* We also need a hex keyboard input */

    unsigned char key[16];

    public:

    void initialize(); /* initialize registers and memory */
    void emulateCycle(); /* emulates fetch-execute cycle of chips-8 */

};


#endif //C8E_C8_H

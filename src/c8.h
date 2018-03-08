#ifndef C8E_C8_H
#define C8E_C8_H

class c8 {
    private:
    unsigned short opcode;  /* This is 16 bits (2 bytes) opcode */
    unsigned char memory[4096]; /* This is the amount of memory for chip-8 */
    unsigned char V[16];  /* 16 registers each 8 bits (1byte). V0 - VE and VF(carry flag) */

    /* There are two special registers, PC and I which are both 16 bits (2 bytes) */
    unsigned short pc; /* keeps track of address of memory */
    unsigned short I; /* address register */

    /* Two timer registers that count down at 60Hz */
    unsigned char delayTimer;
    unsigned char soundTimer;

    /* We also need a stack (which is 16 levels in our case and only keeps the return address which is 16 bits)
       we also need a stack pointer to know at which level we're in */
    unsigned short stack[16];
    unsigned short sp;

    /* We also need a hex keyboard input */
    /* Keyboard will look the following:
     * 1    2   3   4
     * Q    W   E   R
     * A    S   D   F
     * Z    X   C   V                   */


    public:

    bool drawFlag;          /* drawFlag is On/True when we need an update for display. Off/False otherwise */
    unsigned char gfx[64 * 32]; /* each value keeps state of display (pixels) 0 or 1 */
    unsigned char key[16];

    c8();   /* constructor for chip-8 */
    ~c8();  /* destructor for chip-8 */

    void initialize(); /* initialize registers and memory */
    void emulateCycle(); /* emulates fetch-execute cycle of chip-8 */
    bool load(const char *filepath); /* loads the ROM binary to memory */

};


#endif //C8E_C8_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define MEMORY_MAX (1 << 16) // 65536 words (LC-3 memory size)
uint16_t memory[MEMORY_MAX]; // Simulated LC-3 memory

// Swap endianness (LC-3 uses big-endian, most PCs use little-endian)
uint16_t swap16(uint16_t x) {
    return (x << 8) | (x >> 8);
}

enum
{
    OP_BR = 0, /* branch */
    OP_ADD,    /* add  */
    OP_LD,     /* load */
    OP_ST,     /* store */
    OP_JSR,    /* jump register */
    OP_AND,    /* bitwise and */
    OP_LDR,    /* load register */
    OP_STR,    /* store register */
    OP_RTI,    /* unused */
    OP_NOT,    /* bitwise not */
    OP_LDI,    /* load indirect */
    OP_STI,    /* store indirect */
    OP_JMP,    /* jump */
    OP_RES,    /* reserved (unused) */
    OP_LEA,    /* load effective address */
    OP_TRAP,   /* execute trap */
	// New Op Codes
    OP_NOP = 0xE8,      /* no operation */
    OP_CLR = 0xE9,      /* clear a register */
    OP_INC = 0xEA,      /* increment a register */
    OP_DEC = 0xEB,      /* decrement a register */
};
enum
{
    MR_KBSR = 0xFE00, /* keyboard status */
    MR_KBDR = 0xFE02  /* keyboard data */
};
enum
{
    TRAP_GETC = 0x20,  /* get character from keyboard, not echoed onto the terminal */
    TRAP_OUT = 0x21,   /* output a character */
    TRAP_PUTS = 0x22,  /* output a word string */
    TRAP_IN = 0x23,    /* get character from keyboard, echoed onto the terminal */
    TRAP_PUTSP = 0x24, /* output a byte string */
    TRAP_HALT = 0x25,  /* halt the program */
    // New Trap Codes
    TRAP_PUTHEX = 0x26, /* print a number in hex format */
    TRAP_RND = 0x27,    /* generate a random number */ 
    TRAP_GETSTR = 0x28, /* read entire string*/
    TRAP_SLEEP = 0x29,  /* pause for a specified delay */
};

uint16_t sign_extend(uint16_t x, int bit_count)
{
    if ((x >> (bit_count - 1)) & 1) {
        x |= (0xFFFF << bit_count);
    }
    return x;
}

// Read .obj file into memory
void read_obj_file(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Error: Could not open %s\n", filename);
        exit(1);
    }
    
    // Read the startung address
    uint16_t origin;
    fread(&origin, sizeof(origin), 1, file);
    origin = swap16(origin);

    // Load instructions into memory
    uint16_t *p = memory + origin;
    size_t read = fread(p, sizeof(uint16_t), MEMORY_MAX - origin, file);

    // Swap endianess for each instruction
    for (int i = 0; i < read; i++) {
        p[i] = swap16(p[i]);
    }
    fclose(file);
}

void disassemble_instruction(uint16_t addr, uint16_t instr) {
    uint16_t op = instr >> 12;

    printf("0x%04X: ", addr);

    switch (op)
        {
            case OP_ADD:    
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t r1 = (instr >> 6) & 0x7;
                    uint16_t imm_flag = (instr >> 5) & 0x1;
                    if (imm_flag)
                    {
                        uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                        printf("ADD R%d, R%d, #%d\n", r0, r1, imm5);
                    } else {
                        uint16_t r2 = instr & 0x7;
                        printf("ADD R%d, R%d, R%d\n", r0, r1, r2);
                    }
                }
                break;
            case OP_AND:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t r1 = (instr >> 6) & 0x7;
                    uint16_t imm_flag = (instr >> 5) & 0x1;
                
                    if (imm_flag)
                    {
                        uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                        printf("AND R%d, R%d, #%d\n", r0, r1, imm5);
                    }
                    else
                    {
                        uint16_t r2 = instr & 0x7;
                        printf("AND R%d, R%d, R%d\n", r0, r1, r2);
                    }
                }
                break;
            case OP_NOT:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t r1 = (instr >> 6) & 0x7;
                    printf("NOT R%d, R%d\n", r0, r1);
                }
                break;
            case OP_BR:
                {
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    uint16_t cond_flag = (instr >> 9) & 0x7;
                    uint16_t target_addr = pc_offset + addr + 1;

                    printf("BR");
                    if (cond_flag & 0x4) printf("n");  // Negative
                    if (cond_flag & 0x2) printf("z");  // Zero
                    if (cond_flag & 0x1) printf("p");  // Positive

                    printf(" %#06x\n", target_addr);
                }
                break;
            case OP_JMP:
                {
                    /* Also handles RET */
                    uint16_t r1 = (instr >> 6) & 0x7;
                    if (r1 != 7) {
                        printf("JMP r%d\n", r1);
                    } else {
                        printf ("RET\n");
                    }
                }
                break;
            case OP_JSR:
                {
                    uint16_t long_flag = (instr >> 11) & 1;
                    if (long_flag)
                    {
                        uint16_t pc_offset = sign_extend(instr & 0x7FF, 11); 
                        uint16_t target_addr = addr + 1 + pc_offset;  // Calculate jump target
                        printf("JSR %#06x\n", target_addr);  // Print label
                    } else {
                        uint16_t r1 = (instr >> 6) & 0x7;
                        printf("JSRR R%d\n", r1);
                    }
                }
                break;
            case OP_LD:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    uint16_t target_addr = addr + 1 + pc_offset;
                    printf("LD R%d\n", r0);
                }
                break;
            case OP_LDI:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    uint16_t target_addr = addr + 1 + pc_offset;
                    printf("LDI R%d, %#06x\n", r0, target_addr);
                }
                break;
            case OP_LDR:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t r1 = (instr >> 6) & 0x7;
                    uint16_t offset = sign_extend(instr & 0x3F, 6);
                    printf("LDR R%d, R%d, #%d\n", r0, r1, offset);
                }
                break;
            case OP_LEA:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    uint16_t target_addr = addr + 1 + pc_offset;
                    printf("LEA R%d, %#06x\n", r0, target_addr);
                }
                break;
            case OP_ST:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    uint16_t target_addr = addr + 1 + pc_offset;
                    printf("ST R%d, %#06x\n", r0, target_addr);
                }
                break;
            case OP_STI:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    uint16_t target_addr = addr + 1 + pc_offset;
                    printf("STI R%d, %#06x\n", r0, target_addr);
                }
                break;
            case OP_STR:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t r1 = (instr >> 6) & 0x7;
                    uint16_t offset = sign_extend(instr & 0x3F, 6);
                    printf("STR R%d, R%d, #%d\n", r0, r1, offset);
                }
                break;
            case OP_NOP:
                {
                    printf("NOP\n");
                }
                break;
            case OP_CLR:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    printf("CLR R%d", r0);
                }
                break;
            case OP_INC:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    printf("INC R%d", r0);
                }
                break;
            case OP_DEC:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    printf("DEC R%d", r0);
                }
                break;
            case OP_TRAP:
                {            
                    printf("TRAP 0x%02X\n", OP_TRAP);
                }
                break;
            case OP_RES:
                {
                    printf("RES\n");
                }
                break;
            case OP_RTI:
                {
                    printf("RTI\n");
                }
                break;
        }
    fflush(stdout);
}

void disassemble_program(uint16_t origin, size_t num_instructions) {
    printf("Disassembly Output:\n\n");

    for (size_t i = 0; i < num_instructions; i++) {
        uint16_t addr = origin + i;
        uint16_t instr = memory[addr];

        disassemble_instruction(addr, instr);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <file.obj>\n", argv[0]);
        return 1;
    }

    FILE* file = fopen(argv[1], "rb");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    // Read origin address
    uint16_t origin;
    fread(&origin, sizeof(uint16_t), 1, file);
    origin = swap16(origin);

    // Load instructions into memory from the origin
    uint16_t *p = memory + origin;
    size_t read_count = fread(p, sizeof(uint16_t), MEMORY_MAX - origin, file);
    for (size_t i = 0; i < read_count; i++) {
        p[i] = swap16(p[i]);  // Swap endianness
    }

    fclose(file);

    // Disassemble the program
    disassemble_program(origin, read_count);

    return 0;
}
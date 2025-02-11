#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define MEMORY_MAX (1 << 16) // 65536 words (LC-3 memory size)
uint16_t memory[MEMORY_MAX]; // Simulated LC-3 memory

// Swap endianness (LC-3 uses big-endian, most PCs use little-endian)
uint16_t swap16(uint16_t x) {
    return (x << 8) | (x >> 8);
}

// LC-3 instruction mnemonics
const char *opcodes[] = {
    "BR", "ADD", "LD", "ST", "JSR", "AND", "LDR", "STR",
    "RTI", "NOT", "LDI", "STI", "JMP", "RES", "LEA", "TRAP",
    "NOP", "CLR", "INC", "DEC"
};

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



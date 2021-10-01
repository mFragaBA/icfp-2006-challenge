#include <iostream>
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <memory.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vector.h"

using std::cerr;
using std::cout;
using std::cin;
using std::vector;

typedef unsigned int platter;
typedef unsigned int uint;

struct vm {
    public:
    vm(vec prog) : 
    program(prog), 
    finger(0),
    registers({0, 0, 0, 0, 0, 0, 0, 0})
    { }
    void spin_cycle();

    private:
    enum OP_TYPE {
        CONDITIONAL_MOVE,
        ARRAY_INDEX,
        ARRAY_AMENDMENT,
        ADDITION,
        MULTIPLICATION,
        DIVISION,
        NAND,
        HALT,
        ALLOCATION,
        ABANDONMENT,
        OUTPUT,
        INPUT,
        LOAD_PROGRAM,
        ORTHOGRAPHY
    };

    platter registers[8];
    vec program;
    uint finger;

    void conditional_move(uint, uint, uint);
    void array_index(uint, uint, uint);
    void array_amendment(uint, uint, uint);
    void addition(uint, uint, uint);
    void multiplication(uint, uint, uint);
    void division(uint, uint, uint);
    void nand(uint, uint, uint);
    void halt(uint, uint, uint);
    void allocation(uint, uint, uint);
    void abandonment(uint, uint, uint);
    void output(uint, uint, uint);
    void input(uint, uint, uint);
    void load_program(uint, uint, uint);
    void orthography(uint, uint);

    inline uint _opcode(platter p) {
        return p >> 28;
    }

    inline uint _c(platter p) {
        return p & 7;
    }

    inline uint _b(platter p) {
        return (p >> 3) & 7;
    }

    inline uint _a(platter p) {
        return (p >> 6) & 7;
    }

    inline uint _orth_value(platter p) {
        return (p & 0x01FFFFFF);
    }

    inline uint _orth_a(platter p) {
        return (p & 0x0E000000) >> 25;
    }
};

void vm::spin_cycle() {
    for(;;) {
        auto op = _opcode(program[finger]);
        auto a = _a(op);
        auto b = _b(op);
        auto c = _c(op);

        switch (op) {
            // Standard Operations
            case CONDITIONAL_MOVE: conditional_move(a, b, c); break;                   
            case ARRAY_INDEX: array_index(a, b, c); break;
            case ARRAY_AMENDMENT: array_amendment(a, b, c); break;
            case ADDITION: addition(a, b, c); break;
            case MULTIPLICATION: multiplication(a, b, c); break;
            case DIVISION: division(a, b, c); break;
            case NAND: nand(a, b, c); break;

            // Other Operators
            case HALT: halt(a, b, c); return;
            case ALLOCATION: allocation(a, b, c); break;
            case ABANDONMENT: abandonment(a, b, c); break;
            case OUTPUT: output(a, b, c); break;
            case INPUT: input(a, b, c); break;
            case LOAD_PROGRAM: load_program(a, b, c); break;

            // Special Operators
            case ORTHOGRAPHY: orthography(_orth_a(op), _orth_value(op)); break;
        }

        finger++;
    }
}

/*
*   transforms:
*       |b_4|b_3|b_2|b_1|
*   into:
*       |b_1|b_2|b_3|b_4|
*/
uint reverse_endianness(uint x) {
    return (x & 0xFF000000) >> 24
        | (x & 0xFF0000) >> 8
        | (x & 0xFF00) << 8
        | (x & 0xFF) << 24;
}

vec read_file(char *filename) {
    // get file size
    int filesize;
    struct stat s;
    
    if (stat(filename, &s) == -1) {
        cerr << "error reading input binary file\n";
    }

    filesize = s.st_size;
    vec v = build_vec(filesize / sizeof(uint));

    FILE* file = fopen(filename, "r");
    fread(v.data, v.size, sizeof(uint), file);
    fclose(file);

    // fix endianness
    for (uint i = 0; i < v.size; ++i) {
        v[i] = reverse_endianness(v[i]);
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        cerr << "Usage: ./um PROGRAM_SCROLL_PATH\n";
        return 1;
    }

    vm m(read_file(argv[1]));
    m.spin_cycle();

    return 0;
}
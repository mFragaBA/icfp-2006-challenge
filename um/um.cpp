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

#define DEBUG

#ifdef DEBUG
    #define TRY_DBG(fmt, ...) printf(fmt, ##__VA_ARGS__);
#else
    #define TRY_DBG(fmt, ...) ;
#endif

#define DBG_OP TRY_DBG("op %s(%i, %i, %i)\n", __FUNCTION__, a, b, c)

struct vm {
    public:
    
    // transfers ownership of prog
    vm(vec* prog) : 
    program(prog), 
    finger(0),
    registers{0, 0, 0, 0, 0, 0, 0, 0}
    { }
    ~vm() {
        program->clear(); free(program);
    }

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
    vec* program;
    uint finger;

    inline void conditional_move(uint, uint, uint);
    inline void array_index(uint, uint, uint);
    inline void array_amendment(uint, uint, uint);
    inline void addition(uint, uint, uint);
    inline void multiplication(uint, uint, uint);
    inline void division(uint, uint, uint);
    inline void nand(uint, uint, uint);
    inline void halt(uint, uint, uint);
    inline void allocation(uint, uint, uint);
    inline void abandonment(uint, uint, uint);
    inline void output(uint, uint, uint);
    inline void input(uint, uint, uint);
    void load_program(uint, uint, uint);
    inline void orthography(uint, uint);

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
        return (p & 0x01FFFFFFU);
    }

    inline uint _orth_a(platter p) {
        return 7 & (p >> 25);
    }
};

void vm::spin_cycle() {
    for(;;) {
        auto operand = (*program)[finger]; 
        auto op = _opcode(operand);
        //cerr << "op: " << op << '\n';
        finger++;
        auto a = _a(operand);
        auto b = _b(operand);
        auto c = _c(operand);

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

    }
}

#define arr(m) (m?*(vec*)m:*(vec*)program)

inline void vm::conditional_move(uint a, uint b, uint c) {
    DBG_OP;
    if (registers[c]) registers[a] = registers[b];
}

inline void vm::array_index(uint a, uint b, uint c) {
    DBG_OP;
    registers[a] = (arr(registers[b]))[registers[c]];
}

inline void vm::array_amendment(uint a, uint b, uint c) {
    DBG_OP;
    (arr(registers[a]))[registers[b]] = registers[c];
}

inline void vm::addition(uint a, uint b, uint c) {
    DBG_OP;
    registers[a] = registers[b] + registers[c];
}

inline void vm::multiplication(uint a, uint b, uint c) {
    DBG_OP;
    registers[a] = registers[b] * registers[c];
}

inline void vm::division(uint a, uint b, uint c) {
    DBG_OP;
    if (registers[c]) registers[a] = registers[b] / registers[c];
}

inline void vm::nand(uint a, uint b, uint c) {
    DBG_OP;
    registers[a] = ~(registers[b] & registers[c]);
}

inline void vm::halt(uint a, uint b, uint c) { 
    cerr << "Shutting Down...\n";
}

inline void vm::allocation(uint a, uint b, uint c) {
    DBG_OP;
    registers[b] = (platter)build_vec(registers[c]);
}

inline void vm::abandonment(uint a, uint b, uint c) {
    DBG_OP;
    ((vec*)registers[c])->clear();
    delete((vec*)registers[c]);
}

// Only values between 0 and 255 are allowed
inline void vm::output(uint a, uint b, uint c) {
    DBG_OP;
    putchar(registers[c]);
}

inline void vm::input(uint a, uint b, uint c) {
    DBG_OP;
    registers[c] = getchar();
}

void vm::load_program(uint a, uint b, uint c) {
    DBG_OP;
    if (registers[b]) {
        uint newsize = ((vec*)registers[b])->size;
        program->resize(newsize);
        memcpy(program->data, ((vec*)registers[b])->data, newsize*sizeof(uint));
    }

    finger = registers[c];
}

inline void vm::orthography(uint a, uint value) {
    printf("op orthography(%i, %i)\n", a, value);
    registers[a] = value;
}

/*
*   transforms:
*       |b_4|b_3|b_2|b_1|
*   into:
*       |b_1|b_2|b_3|b_4|
*/
uint reverse_endianness(uint x) {
    // return ((x & 0xFF000000U) >> 24)
    //     | ((x & 0xFF0000U) >> 8)
    //     | ((x & 0xFF00U) << 8)
    //     | ((x & 0xFFU) << 24);

    return ((x >> 24) & 0xFF) <<  0
       | ((x >> 16) & 0xFF) <<  8
       | ((x >>  8) & 0xFF) << 16
       | ((x >>  0) & 0xFF) << 24;
}

vec* read_file(char *filename) {
    // get file size
    int filesize;
    struct stat s;
    
    if (stat(filename, &s) == -1) {
        cerr << "error reading input binary file\n";
        exit(1);
    }

    filesize = s.st_size;
    cerr << "input file size: " << filesize << '\n';
    vec* v = build_vec(filesize / sizeof(uint));

    FILE* file = fopen(filename, "rb");
    size_t readbytes = fread(v->data, sizeof(uint)*v->size, 1, file);
    fclose(file);

    // fix endianness
    for (uint i = 0; i < v->size; ++i) {
        (*v)[i] = reverse_endianness((*v)[i]);
    }

    return v;
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
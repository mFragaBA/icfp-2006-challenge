#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <sys/stat.h>
#include <memory.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "vector.h"

using std::cerr;
using std::cout;
using std::cin;
using std::vector;
using std::list;

typedef unsigned int platter;
typedef unsigned int uint;

// #define DEBUG

#ifdef DEBUG
    #define TRY_DBG(fmt, ...) printf(fmt, ##__VA_ARGS__);print_state();std::flush(cout);
#else
    #define TRY_DBG(fmt, ...) ;
#endif

#define DBG_OP TRY_DBG("op %s(%u, %u, %u)\n", __FUNCTION__, a, b, c)

struct vm {
    public:
    
    // transfers ownership of prog
    vm(vec* prog) : 
    finger(0),
    registers{0, 0, 0, 0, 0, 0, 0, 0}
    {
        segments.push_back(prog);
    }
    ~vm() {
        for (vec* v : segments) {
            v->clear();
            free(v);
        }
    }

    void print_state() {
        printf("{finger:%u, regs:[%u, %u, %u, %u, %u, %u, %u, %u]}\n", finger, registers[0], registers[1], registers[2], registers[3], registers[4], registers[5], registers[6], registers[7]);
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
    uint finger;

    vector<vec*> segments;
    list<uint> free_segments;

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
        return (p) & 0b1111111111111111111111111;
    }

    inline uint _orth_a(platter p) {
        return ((p) >> 25) & 0b111;
    }
};

void vm::spin_cycle() {
    for(;;) {
        // printf("%i %i %i\n", (uint)segments.size(), finger, segments[0]->size);
        // std::flush(cout);

        assert(segments.size() > 0 && "segments array cannot be empty");
        assert(finger < segments[0]->size && "Finger out of bounds!");
        assert(segments[0]->size > 0 && "program array cannot be empty");

    
        auto operand = segments[0]->at(finger); 

        auto op = _opcode(operand);
        //cerr << "op: " << op << '\n';
        auto a = _a(operand);
        auto b = _b(operand);
        auto c = _c(operand);
        auto orth_a = _orth_a(operand);
        auto orth_value = _orth_value(operand);

        finger++;

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
            case ORTHOGRAPHY: orthography(orth_a, orth_value); break;

            default: assert(false && "unknown instruction");
        }

    }
}

inline void vm::conditional_move(uint a, uint b, uint c) {
    DBG_OP;
    if (registers[c]) registers[a] = registers[b];
}

inline void vm::array_index(uint a, uint b, uint c) {
    DBG_OP;
    registers[a] = segments[registers[b]]->at(registers[c]);
}

inline void vm::array_amendment(uint a, uint b, uint c) {
    DBG_OP;
    segments[registers[a]]->at(registers[b]) = registers[c];
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
    assert(registers[c] != 0 && "Division by zero!");
    registers[a] = registers[b] / registers[c];
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
    uint addr;
    if (free_segments.size() == 0) {
        addr = segments.size();
        segments.push_back(new vec(registers[c]));
        assert( addr != 0 && "registers[b] cannot be assigned direction 0" );
    } else {
        addr = free_segments.front();
        free_segments.pop_front();

        segments[addr]->resize(registers[c]);
        memset(segments[addr]->data, 0, segments[addr]->size * sizeof(uint));
    }

    registers[b] = addr;
}

inline void vm::abandonment(uint a, uint b, uint c) {
    DBG_OP;
    assert(registers[c] != 0 && "Free of array zero!");
    free_segments.push_back(registers[c]);
}

// Only values between 0 and 255 are allowed
inline void vm::output(uint a, uint b, uint c) {
    DBG_OP;
    assert(registers[c] <= 255 && "Output of unknown character!");
    putchar(registers[c]);fflush(stdout);
}

inline void vm::input(uint a, uint b, uint c) {
    DBG_OP;
    registers[c] = getchar();
}

void vm::load_program(uint a, uint b, uint c) {
    DBG_OP;
    if (registers[b]) {
        uint newsize = segments[registers[b]]->size;
        segments[0]->resize(newsize);
        memcpy(segments[0]->data, segments[registers[b]]->data, newsize*sizeof(uint));
    }

    finger = registers[c];
}

inline void vm::orthography(uint a, uint value) {
    #ifdef DEBUG
        printf("op orthography(%u, %u)\n", a, value);print_state(); std::flush(cout);
    #endif
    registers[a] = value;
}

/*
*   transforms:
*       |b_4|b_3|b_2|b_1|
*   into:
*       |b_1|b_2|b_3|b_4|
*/
uint reverse_endianness(uint x) {
    return ((x & 0xFF000000U) >> 24)
        | ((x & 0xFF0000U) >> 8)
        | ((x & 0xFF00U) << 8)
        | ((x & 0xFFU) << 24);
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
    vec* v = new vec(filesize / sizeof(uint));

    FILE* file = fopen(filename, "rb");
    fread(v->data, sizeof(uint)*v->size, 1, file);
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
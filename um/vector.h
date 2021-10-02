#include <iostream>
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <memory.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int uint;

struct vec {
    uint size;
    uint* data; 

    // transfers ownership of d
    vec(uint s, uint* d) : size(s), data(d) {};
    ~vec() { clear(); }

    inline uint& operator[](uint i) {
        return data[i];
    }

    void clear();
    void resize(uint);
};

vec* build_vec(uint size) {
    vec* v = new vec(
        size,
        (uint*)calloc(size*sizeof(uint), sizeof(uint)) 
    );

    memset(v->data, 0, size * sizeof(uint));

    return v;
}

void vec::clear() {
    free(data);
    data = NULL;
    size = 0;
}

void vec::resize(uint size) {
    this->data = (uint*)reallocarray(this->data, size, sizeof(uint));
    this->size = size;
}
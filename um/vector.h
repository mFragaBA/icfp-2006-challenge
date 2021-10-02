#include <iostream>
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <memory.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef unsigned int uint;

struct vec {
    uint size;
    uint* data; 

    // transfers ownership of d
    vec(uint s, uint* d) : size(s), data(d) {};
    vec(uint s) : size(s) {
        this->data = (uint*)calloc(s, sizeof(uint));
        assert(data != NULL && "calloc failed");
        memset(data, 0, s * sizeof(uint));
    }
    ~vec() { clear(); }

    inline uint& at(uint i) {
        assert(i < size && "Index out of bounds!");
        return data[i];
    }

    inline uint& operator[](uint i) {
        return data[i];
    }

    void clear();
    void resize(uint);
};

void vec::clear() {
    free(data);
    data = NULL;
    size = 0;
}

void vec::resize(uint size) {
    this->data = (uint*)reallocarray(this->data, size, sizeof(uint));
    this->size = size;
}
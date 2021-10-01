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

    inline uint& operator[](uint i) {
        return data[i];
    }

    void clear();
    void resize(uint);
};

vec build_vec(uint size) {
    return {
        size,
        (uint*)calloc(size*sizeof(uint), sizeof(uint)) 
    };
}

void vec::clear() {
    free(data);
    size = 0;
}

void vec::resize(uint size) {
    data = (uint*)reallocarray(data, size, sizeof(uint));
    size = size;
}
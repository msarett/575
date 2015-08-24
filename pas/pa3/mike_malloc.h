/*
 *  Copyright 2015 Mike Reed
 */

#ifndef mike_malloc_DEFINED
#define mike_malloc_DEFINED

#include "GTypes.h"

template <typename T> class AutoTArray {
public:
    AutoTArray() {
        fArray = NULL;
    }
    AutoTArray(int count) {
        fArray = new T[count];
    }
    ~AutoTArray() {
        delete[] fArray;
    }

    T* setCount(int count) {
        delete[] fArray;
        fArray = new T[count];
    }

    T& operator[](int index) { return fArray[index]; }
    operator T*() const { return fArray; }

private:
    T*  fArray;
};

#endif

#include "../include/trychainlib.h"

#include <stdlib.h>

typedef struct {
    int* rootDepths;
    int topIndex;
    int capacity;
} tcl_rootDepthStack;

static int tcl_tryDepth = 0;
static tcl_rootDepthStack rdStack;

int tcl_init(const int maxRootCapacity) {
    rdStack.rootDepths = calloc(maxRootCapacity, sizeof(int));
    if (!rdStack.rootDepths) {
        return -1;
    }
    rdStack.capacity = maxRootCapacity;
    rdStack.topIndex = 0;
    return 0;
}
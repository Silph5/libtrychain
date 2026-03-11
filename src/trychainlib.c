#include "../include/trychainlib.h"

#include <stdlib.h>

typedef struct {
    int* rootDepths;
    int topIndex;
    int capacity;
} ttl_rootDepthStack;

static int ttl_tryDepth = 0;
static ttl_rootDepthStack rdStack;

int ttl_init(const int maxRootCapacity) {
    rdStack.rootDepths = calloc(maxRootCapacity, sizeof(int));
    if (!rdStack.rootDepths) {
        return -1;
    }
    rdStack.capacity = maxRootCapacity;
    rdStack.topIndex = 0;
    return 0;
}
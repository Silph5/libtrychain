#include "../include/trychainlib.h"

#include <stdio.h>
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

void _tcl_onTry() {
    tcl_tryDepth++;
}

void _tcl_onTryFail(const char* errMsg) {
    tcl_tryDepth--;
    fprintf(stderr, "    %s\n", errMsg);
    if (rdStack.rootDepths[rdStack.topIndex] == tcl_tryDepth) {
        fprintf(stderr, "End Of Chain\n");
        rdStack.topIndex--;
    }
}

void _tcl_onTrySuccess() {
    tcl_tryDepth--;
}
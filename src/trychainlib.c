#include "../include/trychainlib.h"

#include <stdio.h>
#include <stdlib.h>

static int tcl_tryDepth = 0;
static int tcl_inFailChain = 0;

void _tcl_onTry() {
    tcl_tryDepth++;
}

void _tcl_onTryFail(const char* errMsg, int line, const char* fileName) {
    if (!tcl_inFailChain) {
        fprintf(stderr, "\nTCL: chain triggered (depth: %i)\n", tcl_tryDepth);
        tcl_inFailChain = 1;
    }
    tcl_tryDepth--;
    fprintf(stderr, "    [%s, %i] %s\n", fileName, line, errMsg);
}

void _tcl_onTryRootFail(const char* errMsg, int line, const char* fileName) {
    tcl_inFailChain = 0;
    tcl_tryDepth--;
    fprintf(stderr, "ROOT[%s, %i] %s\n", fileName, line, errMsg);
    fprintf(stderr, "End of chain\n");

}

void _tcl_onTrySuccess() {
    tcl_tryDepth--;
}
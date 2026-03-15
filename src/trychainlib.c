#include "../include/trychainlib.h"

#include <stdio.h>
#include <stdlib.h>

static int tcl_tryDepth = 0;
static int tcl_inFailChain = 0;
static FILE* tcl_outStream = NULL;

//lib config functions
void tcl_setOutStream(FILE* stream) {
    tcl_outStream = stream;
}

//static util functions
void checkOutStream() {
    if (!tcl_outStream) {
        tcl_outStream = stderr;
        fprintf(stderr, "\nTCL: out stream not set, setting to stderr\n");
    }
}

//functions used in lib macros
void _tcl_onTry() {
    tcl_tryDepth++;
}

void _tcl_onTryFail(const char* errMsg, int line, const char* fileName) {
    checkOutStream();
    if (!tcl_inFailChain) {
        fprintf(stderr, "\nTCL: chain triggered (depth: %i)\n", tcl_tryDepth);
        tcl_inFailChain = 1;
    }
    tcl_tryDepth--;
    fprintf(stderr, "    [%s, %i] %s\n", fileName, line, errMsg);
}

void _tcl_onTryRootFail(const char* errMsg, int line, const char* fileName) {
    checkOutStream();
    tcl_inFailChain = 0;
    tcl_tryDepth--;
    fprintf(stderr, "ROOT[%s, %i] %s\n", fileName, line, errMsg);
    fprintf(stderr, "End of chain\n");

}

void _tcl_onTrySuccess() {
    tcl_tryDepth--;
}
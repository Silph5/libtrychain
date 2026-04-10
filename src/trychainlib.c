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
        fprintf(tcl_outStream, "\nTCL: chain triggered (depth: %i)\n", tcl_tryDepth);
        tcl_inFailChain = 1;
    }
    tcl_tryDepth--;
    fprintf(tcl_outStream, "    [%s, %i] %s\n", fileName, line, errMsg);
}

void _tcl_onTryRootFail(const char* errMsg, int line, const char* fileName) {
    checkOutStream();
    tcl_inFailChain = 0;
    tcl_tryDepth--;
    fprintf(tcl_outStream, "ROOT[%s, %i] %s\n", fileName, line, errMsg);
    fprintf(tcl_outStream, "End of chain\n");

}

void _tcl_onTrySuccess() {
    tcl_tryDepth--;
}

//wrapped c lib functions

tcl_status tcl_malloc(void** outPtr, size_t size) {
    if (outPtr == NULL) {
        return tcl_fail;
    }

    void* ptr = malloc(size);
    if (ptr == NULL) {
        return tcl_fail;
    }
    *outPtr = ptr;
    return tcl_success;
}

tcl_status tcl_calloc(void** outPtr, size_t nItems, size_t itemSize) {
    if (outPtr == NULL) {
        return tcl_fail;
    }

    void* ptr = calloc(nItems, itemSize);
    if (ptr == NULL) {
        return tcl_fail;
    }
    *outPtr = ptr;
    return tcl_success;
}

tcl_status tcl_realloc(void** outPtr, size_t size) {
    if (outPtr == NULL) {
        return tcl_fail;
    }

    void* ptr = realloc(*outPtr, size);
    if (ptr == NULL) {
        return tcl_fail;
    }
    *outPtr = ptr;
    return tcl_success;
}
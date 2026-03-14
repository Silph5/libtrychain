#include "../include/trychainlib.h"

#include <stdio.h>
#include <stdlib.h>

static int tcl_tryDepth = 0;
static int tcl_inFailChain = 0;

void _tcl_onTry() {
    tcl_tryDepth++;
}

void _tcl_onTryFail(const char* errMsg, ) {
    if (!tcl_inFailChain) {
        fprintf(stderr, "TCL: error chain triggered\n");
        tcl_inFailChain = 1;
    }
    tcl_tryDepth--;
    fprintf(stderr, "    %s\n", errMsg);
}

void _tcl_onTrySuccess() {
    tcl_tryDepth--;
}
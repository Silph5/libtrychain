#include "../include/trychainlib.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TCL_BUF_CAP 1024

static _Thread_local int tcl_tryDepth = 0;
static _Thread_local int tcl_inFailChain = 0;
static _Thread_local FILE* tcl_outStream = NULL;

static _Thread_local char tcl_logBuf[TCL_BUF_CAP];
static _Thread_local size_t tcl_logBufLength = 0;

static _Thread_local int argFailSubject = 0;

//lib config functions
void tcl_setOutStream(FILE* stream) {
    tcl_outStream = stream;
}

void tcl_setArgFailSubject(const int argNum) {
    argFailSubject = argNum;
}

//static util functions
void checkOutStream() {
    if (!tcl_outStream) {
        tcl_outStream = stderr;
        fprintf(stderr, "\nTCL: defaulting outstream to stderr\n");
    }
}

const char* fetchEnumErrMsg (tcl_status status) {
    switch (status) {
        case tcl_fail: case tcl_fail_e:
            return "TCL: GENERIC FAIL";
        case _tcl_chain_fail:
            return "(chain)";
        case tcl_fail_no_mem:
            return "TCL: OUT OF MEMORY";
        case tcl_fail_invalid_arg:
            return "TCL: INVALID ARGUMENT";
        case tcl_fail_invalid_state:
            return "TCL: INVALID STATE";
        case tcl_fail_not_found:
            return "TCL: SUBJECT NOT FOUND";
        case tcl_fail_io:
            return "TCL: IO ERROR";
        case tcl_fail_file_open:
            return "TCL: FILE OPEN FAIL";
        case tcl_fail_file_close:
            return "TCL: FILE CLOSE FAIL";
        case tcl_fail_parse:
            return "TCL: PARSE FAIL";
        case tcl_fail_timeout:
            return "TCL: FAIL DUE TO TIMEOUT";
        default:
            return "TCL: UNKNOWN";
    }
}

void fetchLibErrMsg (tcl_status status, int errNum, char* out, size_t outsize) {
    const char* enumMsg = fetchEnumErrMsg(status);

    switch (status) {
        case tcl_fail_e:
        case tcl_fail_io:
        case tcl_fail_file_open:
        case tcl_fail_file_close:
            snprintf(out, outsize, "%s (%s)", enumMsg, strerror(errNum));
            break;
        case tcl_fail_invalid_arg:
            if (argFailSubject != 0) {
                snprintf(out, outsize, "%s (%i)", enumMsg, argFailSubject);
                argFailSubject = 0;
                return;
            }
            snprintf(out, outsize, "%s", enumMsg);
        default:
            snprintf(out, outsize, "%s", enumMsg);
    }
}

static void tcl_appendLog(const char* fmt, ...) {
    if (tcl_logBufLength >= TCL_BUF_CAP - 1) {
        snprintf(tcl_logBuf + TCL_BUF_CAP - 16, 16, "\n...TRUNCATED\n\n");
        return;
    }

    va_list args;
    va_start(args, fmt);

    size_t remaining = TCL_BUF_CAP - tcl_logBufLength;
    int written = vsnprintf(tcl_logBuf + tcl_logBufLength, remaining, fmt, args);

    va_end(args);

    if (written < 0) {
        return;
    }

    if ((size_t)written >= remaining) {
        //truncate
        tcl_logBufLength = TCL_BUF_CAP - 1;
    } else {
        tcl_logBufLength += (size_t)written;
    }
}

//functions used in lib macros
void _tcl_onTry() {
    tcl_tryDepth++;
}

void _tcl_onTryFail(const char* errMsg, int line, const char* fileName, tcl_status status) {
    checkOutStream();
    if (!tcl_inFailChain) {
        tcl_appendLog("\nTCL: chain triggered (depth: %i)\n", tcl_tryDepth);
        tcl_inFailChain = 1;
    }
    tcl_tryDepth--;

    char libErrMsg[256];
    fetchLibErrMsg(status, errno, libErrMsg, sizeof(libErrMsg));

    tcl_appendLog("    [%s, %i] %s\n       -%s\n", fileName, line, libErrMsg, errMsg);
}

void _tcl_onTryRootFail(const char* errMsg, int line, const char* fileName, tcl_status status) {
    checkOutStream();
    tcl_inFailChain = 0;
    tcl_tryDepth--;

    char libErrMsg[256];
    fetchLibErrMsg(status, errno, libErrMsg, sizeof(libErrMsg));

    tcl_appendLog("ROOT[%s, %i] %s\n       -%s\n", fileName, line, libErrMsg, errMsg);

    tcl_appendLog("End of chain\n");
    fwrite(tcl_logBuf, 1, tcl_logBufLength, tcl_outStream);

    tcl_logBufLength = 0;
}

void _tcl_onTrySuccess() {
    tcl_tryDepth--;
}

//wrapped c lib functions

tcl_status tcl_malloc(void** outPtr, size_t size) {
    if (outPtr == NULL) {
        tcl_setArgFailSubject(1);
        return tcl_fail_invalid_arg;
    }

    void* ptr = malloc(size);
    if (ptr == NULL) {
        return tcl_fail_no_mem;
    }
    *outPtr = ptr;
    return tcl_success;
}

tcl_status tcl_calloc(void** outPtr, size_t nItems, size_t itemSize) {
    if (outPtr == NULL) {
        tcl_setArgFailSubject(1);
        return tcl_fail_invalid_arg;
    }

    void* ptr = calloc(nItems, itemSize);
    if (ptr == NULL) {
        return tcl_fail_no_mem;
    }
    *outPtr = ptr;
    return tcl_success;
}

tcl_status tcl_realloc(void** outPtr, size_t size) {
    if (outPtr == NULL) {
        tcl_setArgFailSubject(1);
        return tcl_fail_invalid_arg;
    }

    void* ptr = realloc(*outPtr, size);
    if (ptr == NULL) {
        return tcl_fail_no_mem;
    }
    *outPtr = ptr;
    return tcl_success;
}

tcl_status tcl_fopen(FILE** outFile, const char* path, const char* mode) {
    if (outFile == NULL) {tcl_setArgFailSubject(1); return tcl_fail_invalid_arg;}
    if (path == NULL) {tcl_setArgFailSubject(2); return tcl_fail_invalid_arg;}
    if (mode == NULL) {tcl_setArgFailSubject(3); return tcl_fail_invalid_arg;}

    FILE* f = fopen(path, mode);

    if (f == NULL) {
        return tcl_fail_file_open;
    }

    *outFile = f;
    return tcl_success;
}

tcl_status tcl_fclose(FILE** file) {

    if (file == NULL || *file == NULL) {
        return tcl_fail_invalid_arg;
    }

    FILE* f = *file;
    *file = NULL;

    if (fclose(f) != 0) {
        return tcl_fail_file_close;
    }

    return tcl_success;
}

tcl_status tcl_getenv(const char* name, char** outVal) {
    if (name == NULL) {tcl_setArgFailSubject(1); return tcl_fail_invalid_arg;}
    if (outVal == NULL) {tcl_setArgFailSubject(2); return tcl_fail_invalid_arg;}

    char* val = getenv(name);

    if (val == NULL) {
        return tcl_fail_not_found;
    }

    *outVal = val;

    return tcl_success;
}
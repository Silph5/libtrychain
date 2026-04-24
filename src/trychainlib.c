/*
* MIT License
 *
 * Copyright (c) 2026 Silph5
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to do so, subject to the
 * following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "../include/trychainlib.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LTC_BUF_CAP 1024 //maximum length of a log from a single fail chain. Lib will automatically truncate if max is exceeded.

static _Thread_local int ltc_tryDepth = 0;
static _Thread_local int ltc_inFailChain = 0;
static _Thread_local FILE* ltc_outStream = NULL;

static _Thread_local char ltc_logBuf[LTC_BUF_CAP];
static _Thread_local size_t ltc_logBufLength = 0;

static _Thread_local int argFailSubject = 0;
static _Thread_local int ltc_errno;

//lib config functions
void ltc_setOutStream(FILE* stream) {
    ltc_outStream = stream;
}

void ltc_setArgFailSubject(const int argNum) {
    argFailSubject = argNum;
}

void ltc_captureErrno(const int newErrno) {
    ltc_errno = newErrno;
}

//static util functions
void checkOutStream() {
    if (!ltc_outStream) {
        ltc_outStream = stderr;
        fprintf(stderr, "\nLTC: defaulting outstream to stderr\n");
    }
}

const char* fetchEnumErrMsg (ltc_status status) {
    switch (status) {
        case ltc_fail: case ltc_fail_e:
            return "LTC: GENERIC FAIL";
        case _ltc_chain_fail:
            return "(chain)";
        case ltc_fail_no_mem:
            return "LTC: OUT OF MEMORY";
        case ltc_fail_invalid_arg:
            return "LTC: INVALID ARGUMENT";
        case ltc_fail_invalid_state:
            return "LTC: INVALID STATE";
        case ltc_fail_not_found:
            return "LTC: SUBJECT NOT FOUND";
        case ltc_fail_io:
            return "LTC: IO ERROR";
        case ltc_fail_file_open:
            return "LTC: FILE OPEN FAIL";
        case ltc_fail_file_close:
            return "LTC: FILE CLOSE FAIL";
        case ltc_fail_parse:
            return "LTC: PARSE FAIL";
        case ltc_fail_timeout:
            return "LTC: FAIL DUE TO TIMEOUT";
        default:
            return "LTC: UNKNOWN";
    }
}

void fetchLibErrMsg (ltc_status status, int errNum, char* out, size_t outsize) {
    const char* enumMsg = fetchEnumErrMsg(status);

    switch (status) { //using switch case incase more special behavior is added to individual statuses
        case ltc_fail_invalid_arg:
            if (argFailSubject) {
                snprintf(out, outsize, "%s (arg%i)", enumMsg, argFailSubject);
                argFailSubject = 0;
                return;
            }
            snprintf(out, outsize, "%s", enumMsg);
            return;
        default:
            if (ltc_errno) {
                snprintf(out, outsize, "%s (%s)", enumMsg, strerror(errNum));
                ltc_errno = 0;
                return;
            }
            snprintf(out, outsize, "%s", enumMsg);
    }
}

static void ltc_appendLog(const char* fmt, ...) {
    if (ltc_logBufLength >= LTC_BUF_CAP - 1) {
        snprintf(ltc_logBuf + LTC_BUF_CAP - 16, 16, "\n...TRUNCATED\n\n");
        return;
    }

    va_list args;
    va_start(args, fmt);

    size_t remaining = LTC_BUF_CAP - ltc_logBufLength;
    int written = vsnprintf(ltc_logBuf + ltc_logBufLength, remaining, fmt, args);

    va_end(args);

    if (written < 0) {
        return;
    }

    if ((size_t)written >= remaining) {
        //truncate
        ltc_logBufLength = LTC_BUF_CAP - 1;
    } else {
        ltc_logBufLength += (size_t)written;
    }
}

//functions used in lib macros
void _ltc_onTry() {
    ltc_tryDepth++;
}

void _ltc_onTryFail(const char* errMsg, int line, const char* fileName, ltc_status status) {
    checkOutStream();
    if (!ltc_inFailChain) {
        ltc_appendLog("\nLTC: chain triggered (depth: %i)\n", ltc_tryDepth);
        ltc_inFailChain = 1;
    }
    ltc_tryDepth--;

    char libErrMsg[256];
    fetchLibErrMsg(status, errno, libErrMsg, sizeof(libErrMsg));

    ltc_appendLog("    [%s, %i] %s\n       -%s\n", fileName, line, libErrMsg, errMsg);
}

void _ltc_onTryRootFail(const char* errMsg, int line, const char* fileName, ltc_status status) {
    checkOutStream();
    ltc_inFailChain = 0;
    ltc_tryDepth--;

    char libErrMsg[256];
    fetchLibErrMsg(status, errno, libErrMsg, sizeof(libErrMsg));

    ltc_appendLog("ROOT[%s, %i] %s\n       -%s\n", fileName, line, libErrMsg, errMsg);

    ltc_appendLog("End of chain\n");
    fwrite(ltc_logBuf, 1, ltc_logBufLength, ltc_outStream);

    ltc_logBufLength = 0;
}

void _ltc_onTrySuccess() {
    ltc_tryDepth--;
}

//wrapped c lib functions

ltc_status ltc_malloc(void** outPtr, size_t size) {
    if (outPtr == NULL) {
        ltc_setArgFailSubject(1);
        return ltc_fail_invalid_arg;
    }

    void* ptr = malloc(size);
    if (ptr == NULL) {
        return ltc_fail_no_mem;
    }
    *outPtr = ptr;
    return ltc_success;
}

ltc_status ltc_calloc(void** outPtr, size_t nItems, size_t itemSize) {
    if (outPtr == NULL) {
        ltc_setArgFailSubject(1);
        return ltc_fail_invalid_arg;
    }

    void* ptr = calloc(nItems, itemSize);
    if (ptr == NULL) {
        return ltc_fail_no_mem;
    }
    *outPtr = ptr;
    return ltc_success;
}

ltc_status ltc_realloc(void** outPtr, size_t size) {
    if (outPtr == NULL) {
        ltc_setArgFailSubject(1);
        return ltc_fail_invalid_arg;
    }

    void* ptr = realloc(*outPtr, size);
    if (ptr == NULL) {
        return ltc_fail_no_mem;
    }
    *outPtr = ptr;
    return ltc_success;
}

ltc_status ltc_fopen(FILE** outFile, const char* path, const char* mode) {
    if (outFile == NULL) {ltc_setArgFailSubject(1); return ltc_fail_invalid_arg;}
    if (path == NULL) {ltc_setArgFailSubject(2); return ltc_fail_invalid_arg;}
    if (mode == NULL) {ltc_setArgFailSubject(3); return ltc_fail_invalid_arg;}

    FILE* f = fopen(path, mode);

    if (f == NULL) {
        ltc_captureErrno(errno);
        return ltc_fail_file_open;
    }

    *outFile = f;
    return ltc_success;
}

ltc_status ltc_fclose(FILE** file) {

    if (file == NULL || *file == NULL) {
        return ltc_fail_invalid_arg;
    }

    FILE* f = *file;
    *file = NULL;

    if (fclose(f) != 0) {
        ltc_captureErrno(errno);
        return ltc_fail_file_close;
    }

    return ltc_success;
}

ltc_status ltc_getenv(const char* name, char** outVal) {
    if (name == NULL) {ltc_setArgFailSubject(1); return ltc_fail_invalid_arg;}
    if (outVal == NULL) {ltc_setArgFailSubject(2); return ltc_fail_invalid_arg;}

    char* val = getenv(name);

    if (val == NULL) {
        return ltc_fail_not_found;
    }

    *outVal = val;

    return ltc_success;
}
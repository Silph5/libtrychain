#ifndef TRYTRACELIB_LIBRARY_H
#define TRYTRACELIB_LIBRARY_H
#include <stdio.h>

typedef enum {
    tcl_success,
    tcl_fail,
    tcl_fail_no_mem,
    tcl_fail_invalid_arg,
    tcl_fail_io,
    tcl_fail_file_open,
    tcl_fail_file_close
} tcl_status;

void _tcl_onTry();
void _tcl_onTryFail(const char* errMsg, int line, const char* fileName, tcl_status status);
void _tcl_onTrySuccess();
void _tcl_onTryRootFail(const char* errMsg, int line, const char* fileName, tcl_status status);

#define TCL_TRY(func, errorMsg) do { \
    _tcl_onTry(); \
    tcl_status status = (func); \
    if (status != tcl_success) { \
        _tcl_onTryFail(errorMsg, __LINE__, __FILE_NAME__, status); \
        return tcl_fail; \
    } \
    _tcl_onTrySuccess(); \
} while (0);

#define TCL_TRY_ROOT(func, errorMsg, onError) do { \
    _tcl_onTry(); \
    tcl_status status = (func); \
    if (status != tcl_success) { \
        _tcl_onTryRootFail(errorMsg, __LINE__, __FILE_NAME__, status); \
        onError; \
    } \
    _tcl_onTrySuccess(); \
} while (0);

void tcl_setOutStream(FILE* stream);

tcl_status tcl_malloc(void** outPtr, size_t size);
tcl_status tcl_calloc(void** outPtr, size_t nItems, size_t itemSize);
tcl_status tcl_realloc(void** outPtr, size_t size);
tcl_status tcl_fopen(FILE** outFile, const char* path, const char* mode);

#endif // TRYTRACELIB_LIBRARY_H
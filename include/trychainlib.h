#ifndef TRYTRACELIB_LIBRARY_H
#define TRYTRACELIB_LIBRARY_H
#include <stdio.h>

typedef enum {
    tcl_success = 0,
    _tcl_chain_fail, //for macro only
    tcl_fail,
    tcl_fail_e,
    tcl_fail_no_mem,
    tcl_fail_invalid_arg,
    tcl_fail_invalid_state,
    tcl_fail_not_found,
    tcl_fail_not_supported,
    tcl_fail_io,
    tcl_fail_file_open,
    tcl_fail_file_close,
    tcl_fail_parse,
    tcl_fail_timeout,
} tcl_status;

//The above enums should be returned from any function interacting with the below TCL_TRY and TCL_TRYROOT macros.
//tcl_success should only return on success. If any other status is returned, the library will interpret it as a fail.
//Some statuses have additional functionality or rules as listed below:
//          _tcl_chain_fail         - only to be returned by the TRY macro. Indicates a return from a fail chain
//          tcl_faile               - reports ERRNO alongside generic fail message
//          tcl_fail_invalid_arg    - can also return the number of the arg that failed as set by tcl_setArgFailSubject() described further below
//          tcl_fail_io,
//          tcl_fail_file_open,
//          tcl_fail_file_close     - these all report errno


//Macro funcs only. do not use these.
void _tcl_onTry();
void _tcl_onTryFail(const char* errMsg, int line, const char* fileName, tcl_status status);
void _tcl_onTrySuccess();
void _tcl_onTryRootFail(const char* errMsg, int line, const char* fileName, tcl_status status);

#define TCL_TRY(func, errorMsg) do { \
    _tcl_onTry(); \
    tcl_status status = (func); \
    if (status != tcl_success) { \
        _tcl_onTryFail(errorMsg, __LINE__, __FILE_NAME__, status); \
        return _tcl_chain_fail; \
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

//The TCL_TRY_ROOT macro indicates the "root" of a fail chain, where the log trace will terminate.
//Unlike TCL_TRY, the onError macro parameter here can be used to execute code in response to the fail chain

//view libtests/main.c for a better look at how the above macros are used

#define TCL_CHECK_IO_ERROR(file) do { \
    if (ferror(file)) return tcl_fail_io; \
} while (0);
//^This can be used to check for io fails on a stream.

void tcl_setOutStream(FILE* stream);
//^Sets the stream where try chains and other warnings will be written. If not set, will default to stderr.

void tcl_setArgFailSubject(int argNum);
//^Sets the number of the argument which is reported back as invalid.
//This should only be used directly before a tcl_fail_invalid_arg return.
//if the subject is not set, no specific argument will be reported as invalid.

tcl_status tcl_malloc(void** outPtr, size_t size);
tcl_status tcl_calloc(void** outPtr, size_t nItems, size_t itemSize);
tcl_status tcl_realloc(void** outPtr, size_t size);
tcl_status tcl_fopen(FILE** outFile, const char* path, const char* mode);
tcl_status tcl_fclose(FILE** file);
tcl_status tcl_getenv(const char* name, char** outVal);
//Above are wrappers for some standard C functions. To avoid obfuscation, only specific funcs have been selected.
//Possible errors from other funcs should be handled manually when used in tcl_status-returning functions.

#endif // TRYTRACELIB_LIBRARY_H
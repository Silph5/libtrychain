#ifndef TRYTRACELIB_LIBRARY_H
#define TRYTRACELIB_LIBRARY_H

typedef enum {
    tcl_success,
    tcl_fail,
} tcl_status;

void _tcl_onTry();
void _tcl_onTryFail(const char* errMsg, int line, const char* fileName);
void _tcl_onTrySuccess();
void _tcl_onTryRootFail(const char* errMsg, int line, const char* fileName);

#define TCL_TRY(func, errorMsg) do { \
    _tcl_onTry(); \
    tcl_status status = (func); \
    if (status != tcl_success) { \
        _tcl_onTryFail(errorMsg, __LINE__, __FILE_NAME__); \
        return tcl_fail; \
    } \
    _tcl_onTrySuccess(); \
} while (0);

#define TCL_TRY_ROOT(func, errorMsg, onError) do { \
    _tcl_onTry(); \
    tcl_status status = (func); \
    if (status != tcl_success) { \
        _tcl_onTryRootFail(errorMsg, __LINE__, __FILE_NAME__); \
        onError; \
    } \
    _tcl_onTrySuccess(); \
} while (0);

#endif // TRYTRACELIB_LIBRARY_H
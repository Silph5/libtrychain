#ifndef TRYTRACELIB_LIBRARY_H
#define TRYTRACELIB_LIBRARY_H

typedef enum {
    tcl_success,
    tcl_fail,
} tcl_status;

void _tcl_onTry();
void _tcl_onTryFail(const char* errMsg, int line, const char* fileName);
void _tcl_onTrySuccess();

#define TCL_TRY(func, errorMsg) do { \
    _tcl_onTry(); \
    tcl_status status = (func); \
    if (status != tcl_success) { \
        _tcl_onTryFail(errorMsg, __LINE__, __FILE_NAME__); \
        return tcl_fail; \
    } \
    _tcl_onTrySuccess(); \
} while (0);

#endif // TRYTRACELIB_LIBRARY_H
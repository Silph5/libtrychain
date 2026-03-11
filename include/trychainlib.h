#ifndef TRYTRACELIB_LIBRARY_H
#define TRYTRACELIB_LIBRARY_H

typedef enum {
    tcl_success,
    tcl_fail,
} tcl_status;

int tcl_init(int maxRootCapacity);

#endif // TRYTRACELIB_LIBRARY_H
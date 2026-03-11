#ifndef TRYTRACELIB_LIBRARY_H
#define TRYTRACELIB_LIBRARY_H

typedef enum {
    ttl_success,
    ttl_fail,
} ttl_status;

int ttl_init(int maxRootCapacity);

#endif // TRYTRACELIB_LIBRARY_H
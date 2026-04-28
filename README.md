# libtrychain

libtrychain is a lightweight, threadsafe C error reporting/logging library which uses standardised function return values checked by macros to provide contextual error reporting through error "chains", wihout using a stack.

This was made mainly for educational purposes, and for my own use across my C projects. I have no doubt a better library exists somewhere with the same concept as this, but i'm happy with my own implementation.

Quick example of an output from the libtrychain:

```
LTC: chain triggered (depth: 4)
    [main.c, 10] LTC: FILE OPEN FAIL (No such file or directory)
       -failed to open file
    [main.c, 16] (chain)
       -failed func 3
    [main.c, 21] (chain)
       -failed func 2
ROOT[main.c, 28] (chain)
       -failed func 1
End of chain
```

## Features (and how to use them)

The primary features of libtrychain are an enum `ltc_status`, and the `LTC_TRY` and `LTC_TRY_ROOT` macros.

This library also features some C standard function wrappers which return `ltc_status`, and some additional functions used to configure the library and provide additional context to errors.

### ltc_status

The `ltc_status` enum has two purposes: To check the success of a function which returns it, and to map a function fail to a const string describing the type of error that occurred.

The `LTC_TRY` and `LTC_TRY_ROOT` macros check the value of ltc_status returned from a function, and act accordingly. The full list of statuses can be found in trychainlib.h, but here are some examples, including the error message they are mapped to:

| Enum  | Error String  |
| --- | --- |
| ```ltc_success```       | N/A (indicated a function's success)) |
| ```ltc_fail```          | "LTC: GENERIC FAIL"                     |
| ```ltc_fail_no_mem```   | "LTC: OUT OF MEMORY"                  |  
| ```ltc_fail_invalid_arg```   | "LTC: INVALID ARGUMENT"                   |  
| ```ltc_fail_io```   | "LTC: IO ERROR"                   |  

### LTC_TRY, LTC_TRY_ROOT

LTC_TRY and LTC_TRY_ROOT are the integral macros of this library. As can be seen in the .h file, they are defined as shown below:
```
#define LTC_TRY(func, errorMsg) do { \
    _ltc_onTry(); \
    ltc_status status = (func); \
    if (status != ltc_success) { \
        _ltc_onTryFail(errorMsg, __LINE__, __FILE_NAME__, status); \
        return _ltc_chain_fail; \
    } \
    _ltc_onTrySuccess(); \
} while (0);

#define LTC_TRY_ROOT(func, errorMsg, onError) do { \
    _ltc_onTry(); \
    ltc_status status = (func); \
    if (status != ltc_success) { \
        _ltc_onTryRootFail(errorMsg, __LINE__, __FILE_NAME__, status); \
        onError; \
    } \
    _ltc_onTrySuccess(); \
} while (0);
```

 - `func`: any tcl_status returning function (the one that will be checked for success)
 - `errorMsg`: an error message that will be printed for any failure of the function, acompanying the enum-specific message
 - `onError`: code to run when an error occurs at the root of the chain

`LTC_TRY_ROOT` defines the root function of a chain. When an `LTC_TRY` fails, it will propagate its fail down a chain of `LTC_TRY`-checked functions until we reach `LTC_TRY_ROOT`, at which point the chain will be output into the error stream.

Because of this design, it is highly important that **`LTC_TRY` should never be used in a function which doesn't eventually propagate it's fail to a chain root**. Otherwise, a fail may never actually be logged and a variety of other weird untested things will probably happen.

If that didn't make sense, take a look at libtests/main.c for a (very basic) example of how the macros should be used.

### Additional error context functions

The below functions are used to attach additional diagnostic context to errors.

**```void ltc_setArgFailSubject(const int argNum)```**
This function is used to provide additional context to a `ltc_fail_invalid_arg` error report. It should be used directly before a return of this ltc_status, as shown in the below example from tlc_malloc:
```
ltc_status ltc_malloc(void** outPtr, size_t size) {
    if (outPtr == NULL) {
        ltc_setArgFailSubject(1);
        return ltc_fail_invalid_arg;
    }
    ...
```

**```void ltc_captureErrno(int newErrno);```**
This function captures the current state of errno and adds it to the chain log. Similarly to `ltc_setArgFailSubject`, it should be used directly before an enum return. The library doesn't care which enum is returned. Example:
```
if (f == NULL) {
        ltc_captureErrno(errno);
        return ltc_fail_file_open;
    }
```

### Config functions

Currently libtrychain only has one config function, which sets the stream where error chains are logged:
```void ltc_setOutStream(FILE* stream)```

If an error chain occurs and a stream hasn't been specified, it will default to stderr.

### Wrapped standard functions

Some standard C functions are wrapped to return `ltc_status` instead of raw values. This is to help ensure consistency across a users' codebase and trivialise error checking.
I deliberately picked functions to wrap which would not lose info relating to function success:

```
ltc_status ltc_malloc(void** outPtr, size_t size);
ltc_status ltc_calloc(void** outPtr, size_t nItems, size_t itemSize);
ltc_status ltc_realloc(void** outPtr, size_t size);
ltc_status ltc_fopen(FILE** outFile, const char* path, const char* mode);
ltc_status ltc_fclose(FILE** file);
ltc_status ltc_getenv(const char* name, char** outVal);
```

## Installation

This library *should* be very easy to install and use. Simple download the .h and .a (or .c) files, or clone the repo with:

```git clone https://github.com/Silph5/libtrychain.git```

...and put those files in your own project, linking the library as is appropriate.

Note that the .a file within the repo is compiled on Windows 11, so you may need to re-compile the library yourself.
## Limitations

There a few significant limitations to libtrychain:

 - Fail chain reports are limited to 1024 characters (will be truncated if necessary)
 - Requires functions to follow the enforced pattern (as in, to use this lib you will need to design or refactor many functions to return the ltc_status enum, and to check functions they themselves call)
 - Extensive use of macros may lead to some code obfuscation
 - Threads and function calls come with additional overhead

libtrychain uses the MIT license, as is embedded in the source code and header file.



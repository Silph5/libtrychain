//
// Created by tjada on 17/02/2026.
//

#include <stdio.h>
#include "trychainlib.h"

tcl_status testFunc2() {
    return tcl_fail;
}

tcl_status testFunc() {
    TCL_TRY(testFunc2(), "failed func 2")
    return tcl_success;
}

int main() {
    TCL_TRY(testFunc(), "failed func 1")
    return 0;
}
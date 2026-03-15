//
// Created by tjada on 17/02/2026.
//

#include <stdio.h>
#include "trychainlib.h"

tcl_status testFunc3() {
    return tcl_fail;
}

tcl_status testFunc2() {
    TCL_TRY(testFunc3(), "failed func 3")
    return tcl_success;
}

tcl_status testFunc() {
    TCL_TRY(testFunc2(), "failed func 2")
    return tcl_success;
}

int main() {
    TCL_TRY_ROOT(testFunc(), "failed func 1", break;)
    TCL_TRY_ROOT(testFunc(), "failed func 1 again", break;)
    return 0;
}
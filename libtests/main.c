//
// Created by tjada on 17/02/2026.
//

#include <stdio.h>
#include "trychainlib.h"

ltc_status testFunc3() {
    FILE* file;
    LTC_TRY(ltc_fopen(&file, "../CMaeLists.txt", "r"), "failed to open file")

    return ltc_success;
}

ltc_status testFunc2() {
    LTC_TRY(testFunc3(), "failed func 3")
    return ltc_success;
}

ltc_status testFunc() {
    LTC_TRY(testFunc2(), "failed func 2")
    return ltc_success;
}

int main() {
    //ltc_setOutStream(stderr);

    LTC_TRY_ROOT(testFunc(), "failed func 1", break;)

    LTC_TRY_ROOT(testFunc3(), "failed func 2", break;)

    return 0;
}
#ifndef __TEST_H
#define __TEST_H

/* include some function used in test */

#include <emp-tool/utils/prg.h> // prg
#include <emp-tool/utils/utils.h> // time

bool cmp_str(const uint8_t *a, const uint8_t *b, int len) {
    for (int i = 0; i < len; i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

#endif
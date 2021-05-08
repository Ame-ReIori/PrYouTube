#ifndef __UTILS_H
#define __UTILS_H

/* some tool functions */

#include "./common.h"
#include <cstring>
#include <openssl/sha.h>
#include <emp-tool/utils/prg.h>
#include <algorithm>

void arbitrary_xor(uint8_t *out, const uint8_t *in1, const uint8_t *in2, const int len);

void KDF(uint8_t *out, int outlen, const uint8_t *in, const int inlen);

void random_matrix64u(Matrix64u &m);

void random_matrix32u(Matrix32u &m);

#endif
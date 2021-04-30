#ifndef __UTILS_H
#define __UTILS_H

/* some tool functions */

#include "./common.h"
#include <cstring>
#include <openssl/sha.h>

void arbitrary_xor(uint8_t *out, uint8_t *in1, uint8_t *in2, int len);

void KDF(uint8_t *out, int outlen, uint8_t *in, int inlen);

#endif
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

void random_vector32u(Vector32u &v);

// generate float matrix whose values \in (-1, 1)
// and then convert them into fixed-point number
void random_fixed_matrix32u(Matrix32u &m);

void random_fixed_vector32u(Vector32u &v);

void get_matrix_hl(Matrix32u &h, Matrix32u &l, const Matrix32u &m);

void get_matrix_bit(Matrix32u &b, const Matrix32u &m, const uint32_t num);

void get_msb(Matrix32u &r, const Matrix32u &bottom, emp::NetIO *io, int party);

#endif
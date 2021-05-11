/* 
    impelementation file of utils.h
*/

#include "../include/utils.h"
#include "../include/common.h"
#include <eigen3/Eigen/src/Core/util/Constants.h>
#include <random>


void arbitrary_xor(uint8_t *out, const uint8_t *in1, const uint8_t *in2, const int len) {
    for (int i = 0; i < len; i++) {
        *(out + i) = *(in1 + i) ^ *(in2 + i);
    }
}


void KDF(uint8_t *out, int outlen, const uint8_t *in, const int inlen) {
    /* a KDF based on sha-256 */

    int index = 0; // locate block position
    int ctr = 0x00000001; // counter initial
    int block_number = outlen / SHA256_DIGEST_LENGTH; // operation times
    
    uint8_t digest[SHA256_DIGEST_LENGTH]; // tmp result
    uint8_t msg[inlen + 4]; // msg inputed to hash function

    memcpy(msg, in, inlen);
    memcpy(msg + inlen, &ctr, 4);

    // SHA256_CTX sha256;
    
    for (int i = 0; i < block_number; i++) {
        SHA256(msg, inlen + 4, out + index);
        index += SHA256_DIGEST_LENGTH;
        *(uint32_t *)(msg + inlen) += 1;
    }

    // process remaining data
    if (index < outlen) {
        SHA256(msg, inlen + 4, digest);
        memcpy(out + index, digest, outlen - index);
    }
}

void random_matrix64u(Matrix64u &m) {
    uint64_t *tmp = new uint64_t[m.size()];
    emp::PRG prg;

    prg.random_data(tmp, m.size() * 8);
    std::copy(tmp, tmp + m.size(), m.data());
    
    delete[] tmp;
}

void random_matrix32u(Matrix32u &m) {
    uint32_t *tmp = new uint32_t[m.size()];
    emp::PRG prg;

    prg.random_data(tmp, m.size() * 4);
    std::copy(tmp, tmp + m.size(), m.data());
    
    delete[] tmp;
}

void random_vector32u(Vector32u &m) {
    uint32_t *tmp = new uint32_t[m.size()];
    emp::PRG prg;

    prg.random_data(tmp, m.size() * 4);
    std::copy(tmp, tmp + m.size(), m.data());
    
    delete[] tmp;
}

inline float rand_float() {
    return -1 + 1.0 * (rand() % RAND_MAX) / RAND_MAX * (2-0);
}

inline void set_rand_float_array(float *arr, int n) {
    std::random_device rd{};
    srand(rd());
    for (int i = 0; i < n; i++) {
        arr[i] = rand_float();
    }
}

void random_fixed_matrix32u(Matrix32u &m) {
    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> tmp;
    tmp.resize(m.rows(), m.cols());
    set_rand_float_array(tmp.data(), tmp.size());
    m = (tmp * SCALE_NUM).cast<uint32_t>();
}

void random_fixed_vector32u(Vector32u &v) {
    Eigen::Matrix<float, Eigen::Dynamic, 1> tmp;
    tmp.resize(v.rows());
    set_rand_float_array(tmp.data(), tmp.size());
    v = (tmp * SCALE_NUM).cast<uint32_t>();
}

void get_matrix_hl(Matrix32u &h, Matrix32u &l, const Matrix32u &m) {
    h.resize(m.rows(), m.cols());
    l.resize(m.rows(), m.cols());

    for (int i = 0; i < m.rows(); i++) {
        for (int j = 0; j < m.cols(); j++) {
            h(i, j) = (m(i, j) & 0xffff0000) >> 16;
            l(i, j) = m(i, j) & 0xffff;
        }
    }
}

void get_matrix_bit(Matrix32u &b, const Matrix32u &m, const uint32_t num) {
    // get m's msb
    // if msb = 0 ==> store as 0xffffffff
    // if msb = 1 ==> store as 0x00000000
    b.resize(m.rows(), m.cols());

    for (int i = 0; i < m.rows(); i++) {
        for (int j = 0; j < m.cols(); j++) {
            b(i, j) = (m(i, j) >> (num - 1)) & 1;
        }
    }
}
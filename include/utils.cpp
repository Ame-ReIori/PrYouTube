/* 
    impelementation file of utils.h
*/

#include "./utils.h"

void arbitrary_xor(uint8_t *out, uint8_t *in1, uint8_t *in2, int len) {
    for (int i = 0; i < len; i++) {
        *(out + i) = *(in1 + i) ^ *(in2 + i);
    }
}


void KDF(uint8_t *out, int outlen, uint8_t *in, int inlen) {
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
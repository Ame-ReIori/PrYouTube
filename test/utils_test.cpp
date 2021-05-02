#include "../include/utils.h"
#include "./test.h"


void KDF_test() {
    int inlen = 54;
    int outlen = 65;
    uint8_t in[inlen];
    uint8_t out[outlen];

    int batch = 10000;
    
    emp::PRG prg;
    prg.random_data(in, inlen);

    KDF(out, outlen, in, inlen);

    // output result
    std::cout << "==================== KDF ACCURACY TEST ====================" << std::endl;

    printf("origin buffer: \n");
    for (int i = 0; i < inlen; i++) {
        printf("\\x%02x", *(in + i)); // convenient when copy to python 
    }
    printf("\n");

    printf("KDF buffer: \n");
    for (int i = 0; i < outlen; i++) {
        printf("%02X", *(out + i)); 
    }
    printf("\n");

    std::cout << "==================== KDF PERFORMANCE TEST ====================" << std::endl;

    auto s = emp::clock_start();

    for (int i = 0; i < batch; i++) {
        KDF(out, outlen, in, inlen);
    }

    double e = emp::time_from(s);

    printf("inlen: %d, outlen: %d, time used: %f us/batch\n", inlen, outlen, e/batch);
}

int main() {

    #ifdef KDF_TEST

    KDF_test();

    #endif
}
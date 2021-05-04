#include "./ot_extension.h"
#include <bits/stdint-uintn.h>

OTEX64::OTEX64(emp::NetIO *io) {
    this->io = io;
    // G = new Group();
    base_ot = new OT(io);
}

OTEX64::~OTEX64() {
    if (base_ot != nullptr) {
        delete base_ot;
    }
}

void OTEX64::initial_TU(uint64_t *T, uint64_t *U, const bool *b, int batch) {
    for (int i = 0; i < batch; i++) {
        prg.random_data(T + i, 8);
        *(U + i) = *(T + i) ^ bool_ext64(*(b + i));
    }
}

void OTEX64::send(const uint64_t *data0, const uint64_t *data1, const int batch) {
    // initial sender's chosen bit
    bool s[64];
    prg.random_bool(s, 64);

    uint8_t **Qt = (uint8_t **)malloc(sizeof(uint8_t *) * 64);
    for (int i = 0; i < 64; i++) {
        Qt[i] = (uint8_t *)malloc(sizeof(uint8_t *) * batch / 8);
        base_ot->recv(*(Qt + i), s[i], batch / 8);
    }

    // a stupid test method, need to optimize
    #ifdef STUPID_OT_TEST
        for (int i = 0; i < 64; i++) {
            printf("%d ", s[i]);
            for (int j = 0; j < batch / 8; j++) {
                printf("%02x", *(*(Q + i) + j));
            }
            printf("\n");
        }
    #endif

    // transform Qt to Q
    uint64_t *Q = new uint64_t[batch];
    trans_mat_64_batch(Q, Qt, batch);

    #ifdef Q_TRANS_TEST
        for (int i = 0; i < batch; i++) {
            printf("%016lx %016lx\n", Q[i], Q[i] ^ s_num);
        }
    #endif

    uint64_t x0, x1;
    
    for (int i = 0; i < batch; i++) {
        prepare_otex_sender_msg(&x0, &x1, data0[i], data1[i], Q[i], s);
        io->send_data((uint8_t *)&x0, 8);
        io->send_data((uint8_t *)&x1, 8);
    }

    delete[] Q;
    delete[] Qt;

}   

void OTEX64::recv(uint64_t *r, const bool *b, const int batch) {
    // prepare T and U, T_i xor U_i = b_i \cdot 1^k
    uint64_t *T = new uint64_t[batch];
    uint64_t *U = new uint64_t[batch];
    initial_TU(T, U, b, batch);

    // transform T and U
    // batch * 64(uint64_t) ==> 64 * batch(uint8_t *)
    uint8_t **Tt = (uint8_t **)malloc(sizeof(uint8_t *) * 64);
    uint8_t **Ut = (uint8_t **)malloc(sizeof(uint8_t *) * 64);

    // allocate Tt and Ut
    for (int i = 0; i < 64; i++) {
        Tt[i] = (uint8_t *)malloc(sizeof(uint8_t *) * batch / 8);
        Ut[i] = (uint8_t *)malloc(sizeof(uint8_t *) * batch / 8);
        memset(Tt[i], 0, batch / 8);
        memset(Ut[i], 0, batch / 8);
    }

    // exec transform
    trans_mat_batch_64(Tt, T, batch);
    trans_mat_batch_64(Ut, U, batch);

    #ifdef TRANS_TEST
        // convert b to uint8_t *
        uint8_t b_str[batch / 8];
        bools_to_bytes(b_str, b, batch);

        for (int i = 0; i < 64; i++) {
            for (int j = 0; j < batch / 8; j++) {
                if (*(*(Tt+i)+j) != (*(*(Ut+i)+j) ^ b_str[j])) {
                    printf("TRANS ERR!!\n");
                    return;
                }
            }
        }
    #endif

    // exec 64 base_ot
    for (int i = 0; i < 64; i++) {
        base_ot->send(*(Tt + i), *(Ut + i), batch / 8);
    }
    
    #ifdef STUPID_OT_TEST
        for (int i = 0; i < 64; i++) {
            for (int j = 0; j < batch / 8; j++) {
                printf("%02x", *(*(Tt + i) + j));
            }
            printf(" ");
            for (int j = 0; j < batch / 8; j++) {
                printf("%02x", *(*(Ut + i) + j));
            }
            printf("\n");
        }
    #endif

    #ifdef Q_TRANS_TEST
        for (int i = 0; i < batch; i++) {
            printf("%d %016lx\n", b[i], T[i]);
        }
    #endif

    uint64_t x0, x1, mask;
    for (int i = 0; i < batch; i++) {
        KDF((uint8_t *)&mask, 8, (uint8_t *)&T[i], 8);
        io->recv_data((uint8_t *)&x0, 8);
        io->recv_data((uint8_t *)&x1, 8);

        if (b[i] == 0) {
            *(r + i) = x0 ^ mask;
        } else {
            *(r + i) = x1 ^ mask;
        }
    }

    delete[] T;
    delete[] U;
    delete[] Tt;
    delete[] Ut;
}

uint64_t bool_ext64(bool b) {
    if (b == 0) return 0;
    else return 0xffffffffffffffff;
}

void trans_mat_batch_64(uint8_t **Mt, uint64_t *M, int batch) {
    int byte_len = batch / 8;
    for (int i = 0; i < batch / 8; i++) {
        uint64_t mask = 0x8000000000000000;
        for (int j = 0; j < 64; j++) {
            *(*(Mt + j) + i) ^= ((uint8_t)((*(M + i * 8 + 0) & mask) >> (63 - j))) << 7;
            *(*(Mt + j) + i) ^= ((uint8_t)((*(M + i * 8 + 1) & mask) >> (63 - j))) << 6;
            *(*(Mt + j) + i) ^= ((uint8_t)((*(M + i * 8 + 2) & mask) >> (63 - j))) << 5;
            *(*(Mt + j) + i) ^= ((uint8_t)((*(M + i * 8 + 3) & mask) >> (63 - j))) << 4;
            *(*(Mt + j) + i) ^= ((uint8_t)((*(M + i * 8 + 4) & mask) >> (63 - j))) << 3;
            *(*(Mt + j) + i) ^= ((uint8_t)((*(M + i * 8 + 5) & mask) >> (63 - j))) << 2;
            *(*(Mt + j) + i) ^= ((uint8_t)((*(M + i * 8 + 6) & mask) >> (63 - j))) << 1;
            *(*(Mt + j) + i) ^= ((uint8_t)((*(M + i * 8 + 7) & mask) >> (63 - j))) << 0;
            mask >>= 1;
        }
    }
}

void trans_mat_64_batch(uint64_t *M, uint8_t **Mt, int batch) {
    memset(M, 0, batch * 8);
    for (int i = 0; i < batch / 8; i++) {
        uint8_t mask = 0x80;
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 64; k++) {
                M[8 * i + j] ^= ((uint64_t)((*(*(Mt + k) + i) & mask) >> (7 - j))) << (63 - k);
            }
            mask >>= 1;
        }
    }
}

void bools_to_bytes(uint8_t *out, const bool *b, const int inlen) {
    int index = 0;
    int tmp = 0;
    for (int i = 0; i < inlen; i++) {
        tmp ^= (b[i] << (7 - i % 8));

        if ((i + 1) % 8 == 0) {
            *(out + index) = tmp;
            index++;
            tmp = 0;
        }
    }
}

uint64_t bools_to_uint64(const bool *in) {
    uint64_t res = 0;
    for (int i = 0; i < 63; i++) {
        res ^= in[i];
        res <<= 1;
    }
    res ^= in[63];
    return res;
}

void prepare_otex_sender_msg(uint64_t *x0, uint64_t *x1, 
                                const uint64_t data0, const uint64_t data1, const uint64_t Qi, const bool *s) {
    uint64_t s_num = bools_to_uint64(s);
    uint64_t mask0, mask1, tmp;
    
    tmp = Qi ^ s_num;

    KDF((uint8_t *)&mask0, 8, (uint8_t *)&Qi, 8);
    KDF((uint8_t *)&mask1, 8, (uint8_t *)&tmp, 8);

    *x0 = mask0 ^ data0;
    *x1 = mask1 ^ data1;
}
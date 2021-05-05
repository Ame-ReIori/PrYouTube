#include "./triple.h"
#include "./utils.h"

Triple::Triple(emp::NetIO *io) {
    this->io = io;
    otex = new OTEX64(io);
}

Triple::~Triple() {
    if (otex != nullptr) {
        delete otex;
    }
}

void Triple::gen_batch(uint64_t *a, uint64_t *b, uint64_t *c, int batch, int party, int is_precomputed) {

    // randomly choose ai and bi
    if (!is_precomputed) {
        prg.random_data((uint8_t *)a, 8 * batch);
        prg.random_data((uint8_t *)b, 8 * batch);
    }

    int total_ot_batch = batch * sizeof(*a) * 8; // for one triple, 64 base ot should be executed

    uint64_t *s0 = new uint64_t[total_ot_batch];
    uint64_t *s1 = new uint64_t[total_ot_batch];
    uint64_t *r  = new uint64_t[total_ot_batch];

    bool *bit = new bool[total_ot_batch];
    uint64s_to_bools(bit, b, batch);

    uint64_t *u = new uint64_t[batch]; // u[i] = \sum -s0
    uint64_t *v = new uint64_t[batch]; // v[i] = \sum r

    // initial u and v to 0
    memset(u, 0, sizeof(uint64_t) * batch);
    memset(v, 0, sizeof(uint64_t) * batch);

    #ifdef CHOSEN_BIT_TEST
        for (int i = 0; i < batch; i++) {
            printf("%016lx\n", *(b + i));
        }

        for (int i = 0; i < total_ot_batch; i++) {
            printf("%01x", *(bit + i));
            if ((i + 1) % 64 == 0) printf("\n");
        }
    #endif

    // fill s0 and s1
    prg.random_data((uint8_t *)s0, 8 * total_ot_batch);

    for (int i = 0; i < total_ot_batch; i++) {
        *(s1 + i) = (*(a + i/64) << (i % 64)) + *(s0 + i);
        *(u + i/64) += (-1 * s0[i]);

        #ifdef OT_ARRAY_TEST
            printf("%016lx, %016lx, %016lx, %d\n", s0[i], s1[i],  
                    (*(a + i/64) << (i % 64)), (*(a + i/64) << (i % 64)) == (s0[i] + s1[i]));
        #endif
    }

    #ifdef FIRST_SHARE_TEST
        for (int j = 0; j < batch; j++) {
            uint64_t tmp = 0;
            for (int k = 0; k < 64; k++) {
                tmp += s0[j * 64 + k];
            }
            printf("%d-th triple, u + \\sum s0 = %lu\n", j, tmp + *(u + j));
        }
    #endif

    // execute ot
    if (party == emp::ALICE) {
        otex->send(s0, s1, total_ot_batch);
        otex->recv(r, bit, total_ot_batch);
    } else {
        otex->recv(r, bit, total_ot_batch);
        otex->send(s0, s1, total_ot_batch);
    }

    // check ot arrcuracy with stupid method
    #ifdef OT_TEST
        for (int i = 0; i < total_ot_batch; i++) {
            printf("%016lx, %016lx, %d, %016lx\n", s0[i], s1[i], bit[i], r[i]);
        }
    #endif

    for (int i = 0; i < total_ot_batch; i++) {
        *(v + i/64) += r[i];
    }

    for (int i = 0; i < batch; i++) {
        c[i] = a[i] * b[i] + u[i] + v[i];
    }
    
    delete[] s0;
    delete[] s1;
    delete[] bit;
    delete[] r;
    delete[] u;
    delete[] v;
}

void Triple::gen_matrix(Matrix64u &a, Matrix64u &b, Matrix64u &c, int m, int d, int n, int party) {
    // a is m * d
    // b is d * n
    // c is m * n and c = ab
    a.resize(m, d);
    b.resize(d, n);
    c.resize(m, n);
    c.setZero();

    random_matrix(a);
    random_matrix(b);

    // transpose a to get rows
    a.transposeInPlace();

    
    // ain and bin is used to exec batch triple generation, result in cou
    uint64_t *ain = new uint64_t[m * d * n];
    uint64_t *bin = new uint64_t[m * d * n];
    uint64_t *cou = new uint64_t[m * d * n];
    uint64_t *res = new uint64_t[m * n];

    for (int i = 0; i < n; i++) {
        std::copy(a.data(), a.data() + a.size(), ain + i * a.size());
        for (int j = 0; j < m; j++) {
            std::copy(b.col(i).data(), b.col(i).data() + d, bin + i * d * m + j * d);
        }
    }
    
    gen_batch(ain, bin, cou, m * d * n, party, true);
    std::cout << "1" << std::endl;

    for (int i = 0; i < m * n; i++) {
        uint64_t tmp = 0;
        for (int j = 0; j < d; j++) {
            tmp += cou[i * d + j];
        }
        res[i] = tmp;
    }
    
    std::copy(res, res + m * n, c.data());

    a.transposeInPlace();

    delete[] ain;
    delete[] bin;
    delete[] cou;
    delete[] res;
}

void uint64s_to_bools(bool *out, const uint64_t *in, const int len) {
    for (int i = 0; i < len; i++) {
        uint64_t tmp = *(in + i);
        for (int j = 0; j < 64; j++) {
            *(out + i * 64 + j) = tmp & 1;
            tmp >>= 1;
        }
    }
}
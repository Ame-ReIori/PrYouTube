#include "../include/triple.h"

void batch_test(emp::NetIO *io, int party) {
    Triple *triple = new Triple(io);

    int batch = 1024 * 128;
    uint64_t *a = new uint64_t[batch];
    uint64_t *b = new uint64_t[batch];
    uint64_t *c = new uint64_t[batch];

    std::cout << "==================== BATCH TRIPLE GEN PERFORMANCE TEST ====================" << std::endl;
    auto s = emp::clock_start();
    triple->gen_batch(a, b, c, batch, party, false);
    double e = emp::time_from(s);
    
    printf("generate %d triples in %f ms.\n", batch, e / 1000);

    #ifdef OUTPUT
        for (int i = 0; i < batch; i++) {
            printf("%016lx %016lx %016lx\n", a[i], b[i], c[i]);
        }
    #endif
}

void matrix_test(emp::NetIO *io, int party) {
    Triple *triple = new Triple(io);
    int m = 2;
    int d = 3;
    int n = 1;

    Matrix64u a, b, c;
    auto s= emp::clock_start();
    triple->gen_matrix(a, b, c, m, d, n, party);
    double end = emp::time_from(s);
    
    printf("generate %dx%d and %dx%d triples in %f ms.\n", m, d, d, n, end / 1000);

    std::cout << a << std::endl << std::endl;
    std::cout << b << std::endl << std::endl;
    std::cout << c << std::endl << std::endl;

    Matrix64u va, vb;

    va.resize(m, d);
    vb.resize(d, n);

    random_matrix(va);
    random_matrix(vb);

    std::cout << va << std::endl << std::endl;
    std::cout << vb << std::endl << std::endl;

    Matrix64u e0 = va - a;
    Matrix64u f0 = vb - b;

    std::cout << e0 << std::endl << std::endl;
    std::cout << f0 << std::endl << std::endl;

    Matrix64u e1, f1;
    e1.resize(m, d);
    f1.resize(d, n);

    uint64_t *e0_str = new uint64_t[e0.size()];
    uint64_t *f0_str = new uint64_t[f0.size()];

    uint64_t *e1_str = new uint64_t[e0.size()];
    uint64_t *f1_str = new uint64_t[f0.size()];

    std::copy(e0.data(), e0.data() + e0.size(), e0_str);
    std::copy(f0.data(), f0.data() + f0.size(), f0_str);

    if (party == emp::ALICE) {
        io->send_data(e0_str, e0.size() * 8);
        io->send_data(f0_str, f0.size() * 8);
    } else {
        io->recv_data(e1_str, e0.size() * 8);
        io->recv_data(f1_str, f0.size() * 8);
    }

    if (party == emp::ALICE) {
        io->recv_data(e1_str, e0.size() * 8);
        io->recv_data(f1_str, f0.size() * 8);
    } else {
        io->send_data(e0_str, e0.size() * 8);
        io->send_data(f0_str, f0.size() * 8);
    }

    std::copy(e1_str, e1_str + e0.size(), e1.data());
    std::copy(f1_str, f1_str + f0.size(), f1.data());

    std::cout << e1 << std::endl << std::endl;
    std::cout << f1 << std::endl << std::endl;

    Matrix64u e = e0 + e1;
    Matrix64u f = f0 + f1;

    Matrix64u t = (party - 1) * e * f + a * f + e * b + c;

    std::cout << t << std::endl;

    delete[] e0_str;
    delete[] f0_str;
    delete[] e1_str;
    delete[] f1_str;
}

int main(int argc, char **argv) {
    int port, party;
    emp::parse_party_and_port(argv, &party, &port);
    emp::NetIO *io = new emp::NetIO(party == emp::ALICE ? nullptr : "127.0.0.1", port);


    #ifdef BATCH_TEST
        batch_test(io, party);
    #endif

    #ifdef MATRIX_TEST
        matrix_test(io, party);
    #endif
}
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
    int d = 5;
    int n = 3;

    Matrix64u a, b, c;
    auto s= emp::clock_start();
    triple->gen_matrix(a, b, c, m, d, n, party);
    double e = emp::time_from(s);
    
    printf("generate %dx%d and %dx%d triples in %f ms.\n", m, d, d, n, e / 1000);

    std::cout << a << std::endl << std::endl;
    std::cout << b << std::endl << std::endl;
    std::cout << c << std::endl << std::endl;
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
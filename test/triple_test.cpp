#include "../include/triple.h"

void batch_test(emp::NetIO *io, int party) {
    Triple *triple = new Triple(io);

    int batch = 10;
    uint64_t *a = new uint64_t[batch];
    uint64_t *b = new uint64_t[batch];
    uint64_t *c = new uint64_t[batch];

    std::cout << "==================== BATCH TRIPLE GEN PERFORMANCE TEST ====================" << std::endl;
    auto s = emp::clock_start();
    triple->gen_batch(a, b, c, batch, party);
    double e = emp::time_from(s);
    
    printf("generate %d triples in %f ms.\n", batch, e / 1000);

    #ifdef OUTPUT
        for (int i = 0; i < batch; i++) {
            printf("%016lx %016lx %016lx\n", a[i], b[i], c[i]);
        }
    #endif
}

int main(int argc, char **argv) {
    int port, party;
    emp::parse_party_and_port(argv, &party, &port);
    emp::NetIO *io = new emp::NetIO(party == emp::ALICE ? nullptr : "127.0.0.1", port);


    #ifdef BATCH_TEST
        batch_test(io, party);
    #endif
}
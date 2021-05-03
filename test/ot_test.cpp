#include "./test.h"
#include "../include/ot.h"
#include "../include/ot_extension.h"

void base_ot_test(emp::NetIO *io, int party) {
    OT *ot = new OT(io);
    int len = 8;
    int batch = 100;

    emp::PRG prg(emp::fix_key);
    uint8_t data0[len];
    uint8_t data1[len];
    uint8_t r[len];
    bool b;

    std::cout << "==================== BASE OT ACCURACY TEST ====================" << std::endl;
    for (int i = 0; i < batch; i++) {
        prg.random_data(data0, len);
        prg.random_data(data1, len);
        prg.random_bool(&b, 1);

        if (party == emp::ALICE) {
            // random choose data0, data1
            ot->send(data0, data1, len);
        } else {
            ot->recv(r, b, len);

            if (!cmp_str(r, (b == 0 ? data0: data1), len)) {
                printf("%d UNPASS\n", i);
                return;
            }
        }
    }
    printf("PASS\n");
}

void ot_extension_test(emp::NetIO *io, int party) {
    OTEX64 *otex = new OTEX64(io);
    int batch = 1024 * 128;
    emp::PRG prg(emp::fix_key);

    uint64_t data0[batch], data1[batch];
    uint64_t r[batch];
    bool b[batch];

    prg.random_data((uint8_t *)data0, batch * 8);
    prg.random_data((uint8_t *)data1, batch * 8);
    prg.random_bool(b, batch);

    std::cout << "==================== OTEX ACCURACY AND PERFORMANCE TEST ====================" << std::endl;
    auto s = emp::clock_start();
    if (party == emp::ALICE) {
        otex->send(data0, data1, batch);
    } else {
        otex->recv(r, b, batch);

        for (int i = 0; i < batch; i++) {
            if (r[i] != (b[i] == 0 ? data0[i] : data1[i])) {
                printf("%d UNPASS\n", i);
                return;
            }
        }
    }
    double e = emp::time_from(s);

    printf("PASS\n");
    printf("exec %d ot in %f ms.\n", batch, e / 1000);
}

int main(int argc, char **argv) {
    int port, party;
    emp::parse_party_and_port(argv, &party, &port);
    emp::NetIO *io = new emp::NetIO(party == emp::ALICE ? nullptr : "127.0.0.1", port);

    #ifdef BASE_OT_TEST
        base_ot_test(io, party);
    #endif

    #ifdef OTEX_TEST
        ot_extension_test(io, party);
    #endif
}
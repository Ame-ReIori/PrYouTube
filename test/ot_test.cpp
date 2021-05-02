#include "./test.h"
#include "../include/ot.h"

void base_ot_test(emp::NetIO *io, int party) {
    OT *ot = new OT(io);
    int len = 8;
    int batch = 100;

    emp::PRG prg(emp::fix_key);
    uint8_t data0[len];
    uint8_t data1[len];
    uint8_t r[len];
    bool b;

    
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

int main(int argc, char **argv) {
    int port, party;
    emp::parse_party_and_port(argv, &party, &port);
    emp::NetIO *io = new emp::NetIO(party == emp::ALICE ? nullptr : "127.0.0.1", port);

    #ifdef BASE_OT_TEST
        base_ot_test(io, party);
    #endif
}
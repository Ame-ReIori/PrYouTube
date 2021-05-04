#ifndef __TRIPLE_H
#define __TRIPLE_H

#include "./common.h"
#include "./ot_extension.h"

class Triple {
    public:
        emp::NetIO *io;
        OTEX64 *otex;
        emp::PRG prg;

        Triple(emp::NetIO *io);
        ~Triple();

        void gen_batch(uint64_t *a, uint64_t *b, uint64_t *c, int batch, int party);
        void gen_matrix(uint64_t *a, uint64_t *b, uint64_t *c, int batch, int party);
};

void uint64s_to_bools(bool *out, const uint64_t *in, const int len);

#endif
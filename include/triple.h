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

        void gen_batch(uint64_t *a, uint64_t *b, uint64_t *c, int batch, int party, int is_precomputed);
        void gen_matrix(Matrix64u &a, Matrix64u &b, Matrix64u &c, int m, int d, int n, int party, int is_precomputed);
};

void uint64s_to_bools(bool *out, const uint64_t *in, const int len);

#endif
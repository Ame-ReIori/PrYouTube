#ifndef __OT_EX_H
#define __OT_EX_H

#include "./ot.h"

/* a spectial ot extension, exec 64 ot transfer 64 bits */

class OTEX64 {
    private:
        void initial_TU(uint64_t *T, uint64_t *U, const bool *b, int batch);

    public:
        emp::NetIO *io;
        //Group *G = nullptr;
        OT *base_ot = nullptr;
        emp::PRG prg;

        OTEX64(emp::NetIO *io);
        ~OTEX64();

        void send(const uint64_t *data0, const uint64_t *data1, const int batch);

        void recv(uint64_t *r, const bool *b, const int batch);
};

uint64_t bool_ext64(bool b);

// TODO: using simd to do 128 x 128 matrix trans
void trans_mat_batch_64(uint8_t **Mt, uint64_t *M, int batch); // a function to trans batch x 64 matrix to 64 x batch

void trans_mat_64_batch(uint64_t *M, uint8_t **Mt, int batch); // a function to trans 64 x batch matrix to batch x 64

void bools_to_bytes(uint8_t *out, const bool *in, const int inlen);

uint64_t bools_to_uint64(const bool *in);

void prepare_otex_sender_msg(uint64_t *x0, uint64_t *x1, 
                                const uint64_t data0, const uint64_t data1, const uint64_t Qi, const bool *s);

#endif
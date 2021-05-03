#ifndef __OT_EX_H
#define __OT_EX_H

#include "./ot.h"

/* a spectial ot extension, exec 64 ot */

class OTEX64 {
    public:
        emp::NetIO *io;
        Group *G = nullptr;
        OT *base_ot = nullptr;

        OTEX64(emp::NetIO *io);
        ~OTEX64();

        void send(const uint8_t **data0, const uint8_t **data1, 
                    const int mlen, const int batch);

        void recv(uint8_t **r, const bool *b, const int mlen, const int batch);
};


// TODO: using simd to do 128 x 128 matrix trans
void trans_mat_batch_64(); // a function to trans batch x 64 matrix to 64 x batch

void trans_mat_64_batch(); // a function to trans 64 x batch matrix to batch x 64

#endif
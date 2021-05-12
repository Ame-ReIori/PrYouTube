#ifndef __SOFTMAX_H
#define __SOFTMAX_H

#include "../layer.h"
#include "../utils.h"
#include "../triple.h"
#include <emp-sh2pc/semihonest.h>
#include <emp-tool/circuits/circuit_file.h>

class Softmax : public Layer {
    private:
        const int bottom_rows;
        const int bottom_cols;
        uint32_t exec_circ(uint32_t a, uint32_t c, uint32_t r, uint32_t b);
        Matrix64u A, B, C;

        void init();

    public:
        Matrix32u relu_val;
        RowVector32u sum; // (\sum_{ReLU(x)}) ^ -1

        Softmax(emp::NetIO *io, int party,
                    const int bottom_rows, const int bottom_cols) : 
            bottom_rows(bottom_rows), bottom_cols(bottom_cols) {
            this->io = io;
            this->party = party;
            init();
        }
        ~Softmax();

        void setup();
        void forward(const Matrix32u & bottom);
        void backward(const Matrix32u & bottom, const Matrix32u & grad_top);

        RowVector32u get_sum() { return sum; }
};

#endif
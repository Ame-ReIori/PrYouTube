#ifndef __RELU_H
#define __RELU_H

#include "../layer.h"
#include "../utils.h"
#include <emp-sh2pc/semihonest.h>
#include <emp-tool/circuits/circuit_file.h>

class ReLU : public Layer {
    private:
        void get_msb(Matrix32u &r, const Matrix32u &bottom);
        uint32_t exec_circ(uint32_t a, uint32_t c, uint32_t r, uint32_t b);

    public:
        ReLU(emp::NetIO *io, int party) {
            this->io = io;
            this->party = party;
        }

        ~ReLU();

        void setup();
        void forward(const Matrix32u &bottom);
        void backward(const Matrix32u &bottom, const Matrix32u &grad_top);
};

#endif
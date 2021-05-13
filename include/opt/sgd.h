#ifndef __SGD_H
#define __SGD_H

#include "../optimizer.h"

class SGD : public Optimizer {
    public:
        explicit SGD(uint32_t lr = 83886) : Optimizer(lr) {};

        void update(Matrix32u &w, Matrix32u &dw);
};

#endif
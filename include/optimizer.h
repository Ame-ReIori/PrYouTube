#ifndef __OPTIMIZER_H
#define __OPTIMIZER_H

#include "./common.h"

class Optimizer {
    protected:
        uint32_t lr;

    public:
        explicit Optimizer(uint32_t lr = 83886) : lr(lr) {}
        virtual ~Optimizer() {}

        virtual void update(Matrix32u &w, Matrix32u &dw) = 0;
};

#endif
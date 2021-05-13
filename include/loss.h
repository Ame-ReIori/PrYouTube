#ifndef __LOSS_H
#define __LOSS_H

#include "./common.h"

class Loss {
    protected:
        // we cannot calculate loss without communication
        Matrix32u grad;

    public:
        virtual ~Loss() {}

        virtual void evaluate(const Matrix32u& pred, const Matrix32u& target) = 0;
        // virtual double output() { return loss; }
        virtual const Matrix32u& back_gradient() { return grad; }
};

#endif
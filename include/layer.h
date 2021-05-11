#ifndef __LAYER_H
#define __LAYER_H

#include "./common.h"
#include "./optimizer.h"

class Layer {
    protected:
        Matrix32u top; // output
        Matrix32u grad; // gradient

    public:
        emp::NetIO *io;
        int party;

        virtual ~Layer() {}

        virtual void setup() = 0;

        virtual void forward(const Matrix32u& bottom) = 0; // forward computation, it is very simple

        virtual void backward(const Matrix32u& bottom, const Matrix32u& grad_top) = 0;
        // bottom is layer's input, grad_top is the gradient transferred from top nodes

        virtual void update(Optimizer& opt) {} // only full connected layer need this function
        
        // output layer's result
        virtual const Matrix32u& output() { return top; }
        
        // output layer's back gradient
        virtual const Matrix32u& back_gradient() { return grad; }
};

#endif
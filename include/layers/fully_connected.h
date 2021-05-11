#ifndef __FULLY_CONNECTED_H
#define __FULLY_CONNECTED_H

#include "../layer.h"
#include "../triple.h"
#include "../utils.h"

class FullyConnected : public Layer {
    private:
        const int sample_num;
        const int feature_num;
        const int neuron_num;
        
        Matrix32u weight;
        Vector32u bias;

        Matrix32u grad_weight;
        Vector32u grad_bias;

        // used for calculating matrix multiplication
        // A * B = C and A * B_ = C_ 
        // (A, B, C)   is used to calculate forward
        // (A, B_, C_) is used to calculate grad_weight
        // (T, U, V)   is used to calculate grad
        Matrix64u A, B, C, A_, B_, C_;
        Matrix64u T, U, V;

        void init();

    public:
        FullyConnected(emp::NetIO *io, int party, 
                            const int sample_num, const int feature_num, const int neuron_num) :
            sample_num(sample_num), feature_num(feature_num),neuron_num(neuron_num) {
                this->io = io;
                this->party = party;
                init();
            }

        void setup();

        // input size: (feature_num, sample_num)
        // weight size: (feature_num, neuron_num)
        // bias size: (neuron_num, 1)
        // forward() calculate: weight ^ t * input + bias ==> result size: (neuron_num, sample_num)
        void forward(const Matrix32u& bottom);
        void backward(const Matrix32u& bottom, const Matrix32u& grad_top);
        void update(Optimizer& opt);

        // param method
        Matrix32u get_weight() { return weight; }
        Vector32u get_bias() { return bias; }

        // some complex computation
        void calculate_grad_weight_bias(const Matrix32u &bottom, const Matrix32u &grad_top);
        void calculate_grad(const Matrix32u &bottom, const Matrix32u &grad_top);
};

#endif
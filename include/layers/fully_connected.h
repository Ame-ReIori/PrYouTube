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
        Matrix64u A;
        Matrix64u B;
        Matrix64u C;

        void init();

    public:
        FullyConnected(emp::NetIO *io, int party, 
                            const int sample_num, const int feature_num, const int neuron_num) :
            sample_num(sample_num), feature_num(feature_num),neuron_num(neuron_num) {
                this->io = io;
                this->party = party;
                init();
            }

        // input size: (feature_num, sample_num)
        // weight size: (feature_num, neuron_num)
        // bias size: (neuron_num, 1)
        // forward() calculate: weight ^ t * input + bias ==> result size: (neuron_num, sample_num)
        void forward(const Matrix32u& bottom);
        void backward(const Matrix32u& bottom, const Matrix32u& grad_top);
        void update(Optimizer& opt);
};

#endif
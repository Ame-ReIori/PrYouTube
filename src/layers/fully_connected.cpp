#include "../../include/layers/fully_connected.h"

void FullyConnected::init() {
    // resize matrix
    weight.resize(feature_num, neuron_num);
    bias.resize(neuron_num);
    grad_weight.resize(feature_num, neuron_num);
    grad_bias.resize(neuron_num);
    top.resize(neuron_num, sample_num);

    random_normal_matrix32u(weight);
    random_normal_vector32u(bias);

    std::cout << weight << std::endl << std::endl;
    std::cout << bias << std::endl << std::endl;

    // A has the same size with weight ^ T
    // B has the same size with input
    // A * B == C
    Triple *triple = new Triple(io);
    triple->gen_matrix(A, B, C, neuron_num, feature_num, sample_num, party);
    delete triple;
}

void FullyConnected::forward(const Matrix32u &bottom) {
    Matrix64u E, E_, F, F_, W, X, tmp;
    E.resize(neuron_num, feature_num);
    E_.resize(neuron_num, feature_num);

    F.resize(feature_num, sample_num);
    F_.resize(feature_num, sample_num);

    W = weight.transpose().cast<int32_t>().cast<uint64_t>();
    X = bottom.cast<int32_t>().cast<uint64_t>();

    E = W - A;
    F = X - B;

    if (party == emp::ALICE) {
        io->send_data(E.data(), E.size() * 8);
        io->send_data(F.data(), F.size() * 8);
        io->recv_data(E_.data(), E_.size() * 8);
        io->recv_data(F_.data(), F_.size() * 8);
    } else {
        io->recv_data(E_.data(), E_.size() * 8);
        io->recv_data(F_.data(), F_.size() * 8);
        io->send_data(E.data(), E.size() * 8);
        io->send_data(F.data(), F.size() * 8);
    }

    E += E_;
    F += F_;

    tmp = (party - 1) * E * F + A * F + E * B + C;

    top = (tmp / SCALE_NUM).cast<uint32_t>();
    top.colwise() += bias;
}

void FullyConnected::backward(const Matrix32u &bottom, const Matrix32u &grad_top) {}
void FullyConnected::update(Optimizer &opt) {}
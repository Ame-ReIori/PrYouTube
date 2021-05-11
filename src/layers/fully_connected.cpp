#include "../../include/layers/fully_connected.h"

void FullyConnected::init() {
    // resize matrix
    weight.resize(feature_num, neuron_num);
    bias.resize(neuron_num);
    grad_weight.resize(feature_num, neuron_num);
    grad_bias.resize(neuron_num);
    grad.resize(feature_num, sample_num);
    top.resize(neuron_num, sample_num);

    random_fixed_matrix32u(weight);
    random_fixed_vector32u(bias);
}

void FullyConnected::setup() {
    Triple *triple = new Triple(io);
    triple->gen_matrix(A, B, C, neuron_num, feature_num, sample_num, party, 0b00);
    triple->gen_matrix(A_, B_, C_, feature_num, sample_num, neuron_num, party, 0b00);
    triple->gen_matrix(T, U, V, feature_num, neuron_num, sample_num, party, 0b00);
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
        io->send_data(E.data(),  E.size()  * 8);
        io->send_data(F.data(),  F.size()  * 8);
        io->recv_data(E_.data(), E_.size() * 8);
        io->recv_data(F_.data(), F_.size() * 8);
    } else {
        io->recv_data(E_.data(), E_.size() * 8);
        io->recv_data(F_.data(), F_.size() * 8);
        io->send_data(E.data(),  E.size()  * 8);
        io->send_data(F.data(),  F.size()  * 8);
    }

    E += E_;
    F += F_;

    tmp = (party - 1) * E * F + A * F + E * B + C;

    top = (tmp / SCALE_NUM).cast<uint32_t>();
    top.colwise() += bias;
}

void FullyConnected::backward(const Matrix32u &bottom, const Matrix32u &grad_top) {
    // bottom size: (feature_num, sample_num)
    // grad_top size: (neuron_num, sample_num)
    // grad_weight = bottom * grad_top ^ T, size: (feature_num, neuron_num)
    // grad = weight * grad_top, size: (feature_num, sample_num)

    calculate_grad_weight_bias(bottom, grad_top);
    calculate_grad(bottom, grad_top);
}

void FullyConnected::update(Optimizer &opt) {}

void FullyConnected::calculate_grad_weight_bias(const Matrix32u &bottom, const Matrix32u &grad_top) {
    Matrix64u E, E_, F, F_, X, G, tmp;

    // calculate grad_weight
    E.resize(feature_num, sample_num);
    E_.resize(feature_num, sample_num);

    F.resize(sample_num, neuron_num);
    F_.resize(sample_num, neuron_num);

    X = bottom.cast<int32_t>().cast<uint64_t>();
    G = grad_top.transpose().cast<int32_t>().cast<uint64_t>();

    E = X - A_;
    F = G - B_;

    if (party == emp::ALICE) {
        io->send_data(E.data(),  E.size()  * 8);
        io->send_data(F.data(),  F.size()  * 8);
        io->recv_data(E_.data(), E_.size() * 8);
        io->recv_data(F_.data(), F_.size() * 8);
    } else {
        io->recv_data(E_.data(), E_.size() * 8);
        io->recv_data(F_.data(), F_.size() * 8);
        io->send_data(E.data(),  E.size()  * 8);
        io->send_data(F.data(),  F.size()  * 8);
    }

    E += E_;
    F += F_;

    tmp = (party - 1) * E * F + A_ * F + E * B_ + C_;
    grad_weight = (tmp / SCALE_NUM).cast<uint32_t>();
    grad_bias = grad_top.rowwise().sum();
}

void FullyConnected::calculate_grad(const Matrix32u &bottom, const Matrix32u &grad_top) {
    Matrix64u E, E_, F, F_, G, W, tmp;
    // calculate grad
    E.resize(feature_num, neuron_num);
    E_.resize(feature_num, neuron_num);

    F.resize(neuron_num, sample_num);
    F_.resize(neuron_num, sample_num);

    W = weight.cast<int32_t>().cast<uint64_t>();
    G = grad_top.cast<int32_t>().cast<uint64_t>();

    E = W - T;
    F = G - U;

    if (party == emp::ALICE) {
        io->send_data(E.data(),  E.size()  * 8);
        io->send_data(F.data(),  F.size()  * 8);
        io->recv_data(E_.data(), E_.size() * 8);
        io->recv_data(F_.data(), F_.size() * 8);
    } else {
        io->recv_data(E_.data(), E_.size() * 8);
        io->recv_data(F_.data(), F_.size() * 8);
        io->send_data(E.data(),  E.size()  * 8);
        io->send_data(F.data(),  F.size()  * 8);
    }

    E += E_;
    F += F_;

    tmp = (party - 1) * E * F + T * F + E * U + V;
    grad = (tmp / SCALE_NUM).cast<uint32_t>();
}
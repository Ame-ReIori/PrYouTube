#include "../include/common.h"
#include "../include/layer.h"
#include "../include/layers/fully_connected.h"

void forward_test(emp::NetIO *io, int party) {
    int sample_num = 2;
    int feature_num = 5;
    int neuron_num = 3;

    FullyConnected *fc = new FullyConnected(io, party, sample_num, feature_num, neuron_num);
    Matrix32u input, input_, weight, weight_, output, output_;
    Vector32u bias, bias_;
    Eigen::MatrixXf I, W, O;
    Eigen::VectorXf B;

    input.resize(feature_num, sample_num);
    output.resize(neuron_num, sample_num);
    weight.resize(feature_num, neuron_num);
    bias.resize(neuron_num);

    input_.resize(feature_num, sample_num);
    output_.resize(neuron_num, sample_num);
    weight_.resize(feature_num, neuron_num);
    bias_.resize(neuron_num);

    random_fixed_matrix32u(input);
    fc->setup();
    fc->forward(input);

    output = fc->output();
    weight = fc->get_weight();
    bias = fc->get_bias();

    if (party == emp::ALICE) {
        io->send_data(output.data(), output.size() * 4);
        io->send_data(weight.data(), weight.size() * 4);
        io->send_data(bias.data(), bias.size() * 4);
        io->send_data(input.data(), input.size() * 4);
    } else {
        io->recv_data(output_.data(), output_.size() * 4);
        io->recv_data(weight_.data(), weight_.size() * 4);
        io->recv_data(bias_.data(), bias_.size() * 4);
        io->recv_data(input_.data(), input_.size() * 4);
    }

    if (party == emp::ALICE) {
        std::cout << output << std::endl << std::endl;
        std::cout << weight << std::endl << std::endl;
        std::cout << bias << std::endl << std::endl;
        std::cout << input << std::endl << std::endl;
    } else {
        std::cout << output_ << std::endl << std::endl;
        std::cout << weight_ << std::endl << std::endl;
        std::cout << bias_ << std::endl << std::endl;
        std::cout << input_ << std::endl << std::endl;

        I = (input + input_).cast<int32_t>().cast<float>() / SCALE_NUM;
        W = (weight + weight_).cast<int32_t>().cast<float>() / SCALE_NUM;
        O = (output + output_).cast<int32_t>().cast<float>() / SCALE_NUM;
        B = (bias + bias_).cast<int32_t>().cast<float>() / SCALE_NUM;

        std::cout << (W.transpose() * I).colwise() + B << std::endl << std::endl;
        std::cout << O << std::endl << std::endl;
    }
}

/*
void backward_test(emp::NetIO *io, int party) {
    int sample_num = 2;
    int feature_num = 5;
    int neuron_num = 3;

    FullyConnected *fc = new FullyConnected(io, party, sample_num, feature_num, neuron_num);
    fc->setup();

    Matrix32u input, grad_top;
    input.resize(feature_num, sample_num);
    grad_top.resize(neuron_num, sample_num);

    random_fixed_matrix32u(input);
    random_fixed_matrix32u(grad_top);

    std::cout << 1 << std::endl;
    fc->backward(input, grad_top);


    Matrix32u X, X_, GT, GT_, GW, GW_, G, G_, W, W_;
    Vector32u GB, GB_;

    X.resize(feature_num, sample_num);
    GT.resize(neuron_num, sample_num);
    GW.resize(feature_num, neuron_num);
    G.resize(feature_num, sample_num);
    W.resize(feature_num, neuron_num);
    GB.resize(neuron_num);

    X_.resize(feature_num, sample_num);
    GT_.resize(neuron_num, sample_num);
    GW_.resize(feature_num, neuron_num);
    G_.resize(feature_num, sample_num);
    W_.resize(feature_num, neuron_num);
    GB_.resize(neuron_num);

    X = input;
    GT = grad_top;
    GW = fc->grad_weight;
    G = fc->grad;
    GB = fc->grad_bias;
    W = fc->get_weight();

    if (party == emp::ALICE) {
        io->send_data(X.data(), X.size() * 4);
        io->send_data(GT.data(), GT.size() * 4);
        io->send_data(GW.data(), GW.size() * 4);
        io->send_data(G.data(), G.size() * 4);
        io->send_data(GB.data(), GB.size() * 4);
        io->send_data(W.data(), W.size() * 4);

        std::cout << X << std::endl << std::endl;
        std::cout << GT << std::endl << std::endl;
        std::cout << GW << std::endl << std::endl;
        std::cout << G << std::endl << std::endl;
        std::cout << GB << std::endl << std::endl;
        std::cout << W << std::endl << std::endl;
    } else {
        io->recv_data(X_.data(), X_.size() * 4);
        io->recv_data(GT_.data(), GT_.size() * 4);
        io->recv_data(GW_.data(), GW_.size() * 4);
        io->recv_data(G_.data(), G_.size() * 4);
        io->recv_data(GB_.data(), GB_.size() * 4);
        io->recv_data(W_.data(), W_.size() * 4);

        std::cout << X_ << std::endl << std::endl;
        std::cout << GT_ << std::endl << std::endl;
        std::cout << GW_ << std::endl << std::endl;
        std::cout << G_ << std::endl << std::endl;
        std::cout << GB_ << std::endl << std::endl;
        std::cout << W_ << std::endl << std::endl;

        X += X_;
        GT += GT_;
        GW += GW_;
        G += G_;
        GB += GB_;
        W += W_;

        Eigen::MatrixXf Xf, GTf, GWf, Gf, Wf;
        Eigen::VectorXf GBf;
        Xf = X.cast<int32_t>().cast<float>() / SCALE_NUM;
        GTf = GT.cast<int32_t>().cast<float>() / SCALE_NUM;
        GWf = GW.cast<int32_t>().cast<float>() / SCALE_NUM;
        Gf = G.cast<int32_t>().cast<float>() / SCALE_NUM;
        GBf = GB.cast<int32_t>().cast<float>() / SCALE_NUM;
        Wf = W.cast<int32_t>().cast<float>() / SCALE_NUM;

        std::cout << Xf * GTf.transpose() << std::endl << std::endl;
        std::cout << GTf.rowwise().sum() << std::endl << std::endl;
        std::cout << Wf * GTf << std::endl << std::endl;

        std::cout << GWf << std::endl << std::endl;
        std::cout << GBf << std::endl << std::endl;
        std::cout << Gf << std::endl << std::endl;
    }
}
*/

int main(int argc, char **argv) {
    int port, party;
    emp::parse_party_and_port(argv, &party, &port);
    emp::NetIO *io = new emp::NetIO(party == emp::ALICE ? nullptr : "127.0.0.1", port);

    #ifdef FORWARD_TEST
        forward_test(io, party);
    #endif

    #ifdef BACKWARD_TEST
        backward_test(io, party);
    #endif
}
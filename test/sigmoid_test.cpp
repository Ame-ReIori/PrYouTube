#include "../include/layers/sigmoid.h"

void forward_test(emp::NetIO *io, int party) {
    Layer *sigmoid = new Sigmoid(io, party);  
    int row_num = 3;
    int col_num = 6;

    Matrix32u bottom, bottom_, top, top_;
    bottom.resize(row_num, col_num);
    bottom_.resize(row_num, col_num);
    top.resize(row_num, col_num);
    top_.resize(row_num, col_num);

    random_fixed_matrix32u(bottom);

    sigmoid->setup();
    sigmoid->forward(bottom);

    top = sigmoid->output();
      
    if (party == emp::ALICE) {
        io->send_data(bottom.data(), bottom.size() * 4);
        io->send_data(top.data(), top.size() * 4);
    } else {
        io->recv_data(bottom_.data(), bottom_.size() * 4);
        io->recv_data(top_.data(), top_.size() * 4);
        bottom += bottom_;
        top += top_;
        std::cout << bottom << std::endl << std::endl;
        std::cout << bottom.cast<int32_t>().cast<float>() / SCALE_NUM << std::endl << std::endl;
        std::cout << top << std::endl << std::endl;
        std::cout << top.cast<int32_t>().cast<float>() / SCALE_NUM << std::endl << std::endl;
    }

    delete sigmoid;
}

void backward_test(emp::NetIO *io, int party) {
    Layer *sigmoid = new Sigmoid(io, party);  
    int row_num = 3;
    int col_num = 6;

    Matrix32u bottom, bottom_, top, top_, grad_top, grad_top_, grad, grad_;
    bottom.resize(row_num, col_num);
    bottom_.resize(row_num, col_num);
    top.resize(row_num, col_num);
    top_.resize(row_num, col_num);
    grad_top.resize(row_num, col_num);
    grad_top_.resize(row_num, col_num);
    grad.resize(row_num, col_num);
    grad_.resize(row_num, col_num);

    random_fixed_matrix32u(bottom);
    random_fixed_matrix32u(grad_top);

    sigmoid->setup();
    sigmoid->forward(bottom);
    sigmoid->backward(bottom, grad_top);

    top = sigmoid->output();
    grad = sigmoid->back_gradient();

    if (party == emp::ALICE) {
        io->send_data(bottom.data(), bottom.size() * 4);
        io->send_data(top.data(), top.size() * 4);
        io->send_data(grad_top.data(), grad_top.size() * 4);
        io->send_data(grad.data(), grad.size() * 4);
    } else {
        io->recv_data(bottom_.data(), bottom_.size() * 4);
        io->recv_data(top_.data(), top_.size() * 4);
        io->recv_data(grad_top_.data(), grad_top_.size() * 4);
        io->recv_data(grad_.data(), grad_.size() * 4);
        bottom += bottom_;
        top += top_;
        grad_top += grad_top_;
        grad += grad_;
        std::cout << bottom << std::endl << std::endl;
        std::cout << grad_top << std::endl << std::endl;
        std::cout << grad << std::endl << std::endl;
        std::cout << bottom.cast<int32_t>().cast<float>() / SCALE_NUM << std::endl << std::endl;
        std::cout << grad_top.cast<int32_t>().cast<float>() / SCALE_NUM << std::endl << std::endl;
        std::cout << grad.cast<int32_t>().cast<float>() / SCALE_NUM << std::endl << std::endl;
    }

    delete sigmoid;
}

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


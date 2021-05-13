#include "../include/layers/softmax.h"

void forward_test(emp::NetIO *io, int party) {
    int row_num = 3;
    int col_num = 6;
    Softmax *softmax = new Softmax(io, party, row_num, col_num);  

    Matrix32u bottom, bottom_, top, top_;
    bottom.resize(row_num, col_num);
    bottom_.resize(row_num, col_num);
    top.resize(row_num, col_num);
    top_.resize(row_num, col_num);

    random_fixed_matrix32u(bottom);

    softmax->setup();
    softmax->forward(bottom);

    top = softmax->output();
      
    if (party == emp::ALICE) {
        io->send_data(bottom.data(), bottom.size() * 4);
        io->send_data(top.data(), top.size() * 4);
    } else {
        io->recv_data(bottom_.data(), bottom_.size() * 4);
        io->recv_data(top_.data(), top_.size() * 4);
        bottom += bottom_;
        top += top_;
        std::cout << bottom.cast<int32_t>().cast<float>() / SCALE_NUM << std::endl << std::endl;
        std::cout << softmax->get_sum().cast<int32_t>().cast<float>() / SCALE_NUM << std::endl << std::endl;
        std::cout << top.cast<int32_t>().cast<float>() / SCALE_NUM << std::endl << std::endl;
    }

    delete softmax;
}

void backward_test(emp::NetIO *io, int party) {
    int row_num = 3;
    int col_num = 6;
    Softmax *softmax = new Softmax(io, party, row_num, col_num);  

    Matrix32u bottom, bottom_, top, top_, grad_top, grad_top_, grad, grad_, r;
    RowVector32u sum;
    Eigen::MatrixXf bottomf, topf, grad_topf, gradf;
    Eigen::Array<float, 1, Eigen::Dynamic> sumf;
    bottom.resize(row_num, col_num);
    bottom_.resize(row_num, col_num);
    top.resize(row_num, col_num);
    top_.resize(row_num, col_num);
    grad_top.resize(row_num, col_num);
    grad_top_.resize(row_num, col_num);
    grad.resize(row_num, col_num);
    grad_.resize(row_num, col_num);
    sum.resize(row_num);
    r.resize(row_num, col_num);

    bottomf.resize(row_num, col_num);
    topf.resize(row_num, col_num);
    grad_topf.resize(row_num, col_num);
    gradf.resize(row_num, col_num);
    sumf.resize(row_num);

    random_fixed_matrix32u(bottom);
    random_fixed_matrix32u(grad_top);

    softmax->setup();
    softmax->forward(bottom);
    softmax->backward(bottom, grad_top);

    top = softmax->output();
    grad = softmax->back_gradient();
    
    sum = softmax->get_sum();

    get_msb(r, bottom, io, party);

    for (int i = 0; i < bottom.rows(); i++) {
        for (int j = 0; j < bottom.cols(); j++) {
            r(i, j) = 1 - r(i, j);
        }
    }

    if (party == emp::ALICE) {
        io->send_data(bottom.data(), bottom.size() * 4);
        io->send_data(top.data(), top.size() * 4);
        io->send_data(grad_top.data(), grad_top.size() * 4);
        io->send_data(grad.data(), grad.size() * 4);
        io->send_data(r.data(), r.size() * 4);
    } else {
        io->recv_data(bottom_.data(), bottom_.size() * 4);
        io->recv_data(top_.data(), top_.size() * 4);
        io->recv_data(grad_top_.data(), grad_top_.size() * 4);
        io->recv_data(grad_.data(), grad_.size() * 4);
        io->recv_data(r.data(), r.size() * 4);
        bottom += bottom_;
        top += top_;
        grad_top += grad_top_;
        grad += grad_;

        bottomf = bottom.cast<int32_t>().cast<float> () / SCALE_NUM;
        topf = top.cast<int32_t>().cast<float> () / SCALE_NUM;
        grad_topf = grad_top.cast<int32_t>().cast<float> () / SCALE_NUM;
        gradf = grad.cast<int32_t>().cast<float> () / SCALE_NUM;
        sumf = sum.cast<int32_t>().cast<float> () / SCALE_NUM;

        Eigen::Array<float, 1, Eigen::Dynamic> tmp_sum = grad_topf.cwiseProduct(topf).colwise().sum();
        Eigen::MatrixXf tmp = grad_topf.array().rowwise() - tmp_sum;
        Eigen::MatrixXf tmp2 = tmp.array().rowwise() * sumf;

        std::cout << tmp << std::endl << std::endl;
        std::cout << sumf << std::endl << std::endl;

        std::cout << bottomf << std::endl << std::endl;
        std::cout << tmp2 << std::endl << std::endl;
        std::cout << grad.cast<int32_t>().cast<float>() / SCALE_NUM << std::endl << std::endl;
    }

    delete softmax;
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


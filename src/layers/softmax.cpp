#include "../../include/layers/softmax.h"
#include <eigen3/Eigen/src/Core/Matrix.h>
#include <emp-sh2pc/semihonest.h>

void Softmax::init() {
    A.resize(bottom_rows, bottom_cols);
    B.resize(bottom_rows, bottom_cols);
    C.resize(bottom_rows, bottom_cols);
    grad.resize(bottom_rows, bottom_cols);
}

Softmax::~Softmax() {
    emp::finalize_semi_honest();
}

void Softmax::setup() {
    emp::setup_semi_honest(io, party);
    Triple *triple = new Triple(io);
    triple->gen_batch(A.data(), B.data(), C.data(), bottom_rows * bottom_cols, party, 0b00);
    delete triple;
}

void Softmax::forward(const Matrix32u &bottom) {
    Matrix32u cn, rn;
    RowVector32u res_colwise_sum, res_colwise_sum_;
    Eigen::Array<float, 1, Eigen::Dynamic> float_res_colwise_sum;
    
    // because bottom is value after secret share
    // so we do not need to reshare it

    // get msb of bottom
    get_msb(cn, bottom, io, party);

    // initial rn
    rn.resize(bottom.rows(), bottom.cols());
    random_matrix32u(rn);

    // initial res
    relu_val.resize(bottom.rows(), bottom.cols());
    res_colwise_sum.resize(bottom.cols());
    res_colwise_sum_.resize(bottom.cols());
    
    uint32_t a, c, r, b;
    for (int i = 0; i < bottom.rows(); i++) {
        for (int j = 0; j < bottom.cols(); j++) {
            if (party == emp::ALICE) {
                a = bottom(i, j);
                c = cn(i, j) + 0xffffffff;
                r = rn(i, j);
            } else {
                b = bottom(i, j);
            }
            uint32_t tmp = exec_circ(a, c, r, b);
            
            if (party == emp::ALICE) {
                relu_val(i, j) = r;
            } else {
                relu_val(i, j) = tmp;
            }
        }
    }

    res_colwise_sum = relu_val.colwise().sum();

    if (party == emp::ALICE) {
        io->send_data(res_colwise_sum.data(),  res_colwise_sum.size() * 4);
        io->recv_data(res_colwise_sum_.data(), res_colwise_sum_.size() * 4);
    } else {
        io->recv_data(res_colwise_sum_.data(), res_colwise_sum_.size() * 4);
        io->send_data(res_colwise_sum.data(),  res_colwise_sum.size() * 4);
    }

    res_colwise_sum += res_colwise_sum_;
    
    float_res_colwise_sum = res_colwise_sum.cast<int32_t>().cast<float>()
                          / SCALE_NUM;
    
    float_res_colwise_sum = float_res_colwise_sum.cwiseInverse();
    sum = (float_res_colwise_sum * SCALE_NUM).cast<uint32_t>();
    
    // covert res and fixed_res_colwise_sum to uint64_t to avoid overflow
    // multiplication
    // truncation
    // recovert the result to uint32_t
    top = ((relu_val.cast<int32_t>().cast<uint64_t>().array().rowwise() 
            * sum.cast<int32_t>().cast<uint64_t>()) / SCALE_NUM)
          .cast<uint32_t>();
            
}

void Softmax::backward(const Matrix32u &bottom, const Matrix32u &grad_top) {
    // calculate (x_i > 0) * (top_i - \sum_{grad_top_j * top_j}) * sum^{-1}
    Matrix64u grad_top64, top64, tmp, E, E_, F, F_;
    RowVector32u tmp_sum;
    Matrix32u middle_ex, ex_with_sum;
    Matrix32u cn(top.rows(), top.cols());
    Matrix32u rn(top.rows(), top.cols());

    E.resize(bottom_rows, bottom_cols);
    F.resize(bottom_rows, bottom_cols);
    E_.resize(bottom_rows, bottom_cols);
    F_.resize(bottom_rows, bottom_cols);

    grad_top64 = grad_top.cast<int32_t>().cast<uint64_t>();
    top64 = top.cast<int32_t>().cast<uint64_t>();

    E = grad_top64 - A;
    F = top64 - B;

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

    tmp = (party - 1) * E.cwiseProduct(F) + A.cwiseProduct(F) + B.cwiseProduct(E) + C;
    tmp_sum = (tmp / SCALE_NUM).cast<uint32_t>().colwise().sum();


    middle_ex = grad_top.array().rowwise() - tmp_sum;
    ex_with_sum = ((middle_ex.cast<int32_t>().cast<uint64_t>().array().rowwise() 
            * sum.cast<int32_t>().cast<uint64_t>()) / SCALE_NUM)
          .cast<uint32_t>();
    
    uint32_t a, c, r, b;
    get_msb(cn, bottom, io, party);
    random_matrix32u(rn);

    for (int i = 0; i < bottom_rows; i++) {
        for (int j = 0; j < bottom_cols; j++) {
            if (party == emp::ALICE) {
                a = ex_with_sum(i, j);
                c = cn(i, j) + 0xffffffff;
                r = rn(i, j);
            } else {
                b = ex_with_sum(i, j);
            }
            uint32_t ret = exec_circ(a, c, r, b);
            
            if (party == emp::ALICE) {
                grad(i, j) = r;
            } else {
                grad(i, j) = ret;
            }
        }
    }
}

uint32_t Softmax::exec_circ(uint32_t a, uint32_t c, uint32_t r, uint32_t b) {
    string circ = "/home/ame/GraduationProject/Code/demo/PrYouTube/circuit/PRrelu_simple.txt";
    BristolFormat cf(circ.c_str());

    emp::Integer A(32, a, emp::ALICE);
    emp::Integer C(32, c, emp::ALICE);
    emp::Integer R(32, r, emp::ALICE);
    emp::Integer B(32, b, emp::BOB);
    emp::Integer res(32, 0, emp::PUBLIC);

    uint32_t ret;

    emp::Integer Ain(R);
    Ain.resize(96, false);
    Ain = Ain << 64;
    C.resize(96, false);
    Ain = Ain ^ (C << 32);
    A.resize(96, false);
    Ain = Ain ^ A;
    cf.compute((emp::block *)res.bits.data(), (emp::block *)Ain.bits.data(), (emp::block *)B.bits.data());

    /*
    if (party == emp::ALICE) {
        ret = r;
        std::cout << 1 << std::endl;
    } else {
        std::cout << res.reveal<uint32_t>() << std::endl;
    }
    */
    ret = res.reveal<uint32_t>(emp::BOB);
    return ret;
}
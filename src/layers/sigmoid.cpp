#include "../../include/layers/sigmoid.h"
#include <emp-sh2pc/semihonest.h>

Sigmoid::~Sigmoid() {
    emp::finalize_semi_honest();
}

void Sigmoid::setup() {
    emp::setup_semi_honest(io, party);
}

void Sigmoid::forward(const Matrix32u &bottom) {
    // calculate f(x)
    // f(x) = 1 if x > 0.5
    // f(x) = x + 0.5 if -0.5 < x < 0.5
    // f(x) = 0 if x < -0.5
    Matrix32u tmp_less = bottom;
    Matrix32u tmp_great = bottom;
    Matrix32u less, great, rn;
    top.resize(bottom.rows(), bottom.cols());
    rn.resize(bottom.rows(), bottom.cols());
    
    for (int i = 0; i < bottom.rows(); i++) {
        for (int j = 0; j < bottom.cols(); j++) {
            tmp_less(i, j) += FIXED_QUARTER;
            tmp_great(i, j) += FIXED_MINUS_QUARTER;
        }
    }

    get_msb(less, tmp_less, io, party);
    get_msb(great, tmp_great, io, party);

    uint32_t a, c, r, b;
    for (int i = 0; i < bottom.rows(); i++) {
        for (int j = 0; j < bottom.cols(); j++) {
            // do share
            if (party == emp::ALICE) {
                a = bottom(i, j) + FIXED_HALF;
                if (great(i, j) == 0 || less(i, j) == 1) {
                    // in this case, bottom < -0.5 or bottom > 0.5
                    c = 0x00000000;
                } else {
                    c = 0xffffffff;
                }
                r = rn(i, j);
            } else {
                b = bottom(i, j);
            }

            uint32_t tmp = exec_circ(a, c, r, b);
            if (party == emp::ALICE) {
                top(i, j) = r;
                if (great(i, j) == 0) {
                    top(i, j) += FIXED_ONE;
                }
            } else {
                top(i, j) = tmp;
            }
        }
    }
}

void Sigmoid::backward(const Matrix32u &bottom, const Matrix32u &grad_top) {
    Matrix32u tmp_less = bottom;
    Matrix32u tmp_great = bottom;
    Matrix32u less, great, rn;
    grad.resize(bottom.rows(), bottom.cols());
    rn.resize(bottom.rows(), bottom.cols());
    
    for (int i = 0; i < bottom.rows(); i++) {
        for (int j = 0; j < bottom.cols(); j++) {
            tmp_less(i, j) += FIXED_QUARTER;
            tmp_great(i, j) += FIXED_MINUS_QUARTER;
        }
    }

    get_msb(less, tmp_less, io, party);
    get_msb(great, tmp_great, io, party);

    uint32_t a, c, r, b;
    for (int i = 0; i < bottom.rows(); i++) {
        for (int j = 0; j < bottom.cols(); j++) {
            // do share
            if (party == emp::ALICE) {
                a = grad_top(i, j);
                if (great(i, j) == 0 || less(i, j) == 1) {
                    // in this case, bottom < -0.5 or bottom > 0.5
                    c = 0x00000000;
                } else {
                    c = 0xffffffff;
                }
                r = rn(i, j);
            } else {
                b = grad_top(i, j);
            }

            uint32_t tmp = exec_circ(a, c, r, b);
            if (party == emp::ALICE) {
                grad(i, j) = r;
            } else {
                grad(i, j) = tmp;
            }
        }
    }
}

uint32_t Sigmoid::exec_circ(uint32_t a, uint32_t c, uint32_t r, uint32_t b) {
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
#include "../../include/layers/relu.h"

ReLU::~ReLU() {
    // if (cf != nullptr) delete cf;
    emp::finalize_semi_honest();
}

void ReLU::setup() {
    emp::setup_semi_honest(io, party);
}

void ReLU::forward(const Matrix32u & bottom) {
    Matrix32u cn, rn;
    
    // because bottom is value after secret share
    // so we do not need to reshare it

    // get msb of bottom
    get_msb(cn, bottom, io, party);

    // initial rn
    rn.resize(bottom.rows(), bottom.cols());
    random_matrix32u(rn);

    // initial res
    top.resize(bottom.rows(), bottom.cols());
    
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
                top(i, j) = r;
            } else {
                top(i, j) = tmp;
            }
        }
    }
}

void ReLU::backward(const Matrix32u &bottom, const Matrix32u &grad_top) {
    // calculate (bottom > 0) * grad_top

    Matrix32u cn, rn;

    // get msb of bottom
    get_msb(cn, bottom, io, party);

    // initial rn
    rn.resize(bottom.rows(), bottom.cols());
    random_matrix32u(rn);

    // initial res
    grad.resize(bottom.rows(), bottom.cols());
    
    uint32_t a, c, r, b;
    for (int i = 0; i < bottom.rows(); i++) {
        for (int j = 0; j < bottom.cols(); j++) {
            if (party == emp::ALICE) {
                a = grad_top(i, j);
                c = cn(i, j) + 0xffffffff;
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

uint32_t ReLU::exec_circ(uint32_t a, uint32_t c, uint32_t r, uint32_t b) {
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
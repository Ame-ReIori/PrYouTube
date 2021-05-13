#include "../../include/loss/cross_entropy.h"

void CrossEntropy::evaluate(const Matrix32u &pred, const Matrix32u &target) {
    Matrix64u rn;
    uint32_t a0, b0, a1, b1;
    uint64_t r;

    rn.resize(pred.rows(), pred.cols());
    grad.resize(pred.rows(), pred.cols());

    random_matrix64u(rn);

    for (int i = 0; i < pred.rows(); i++) {
        for (int j = 0; j < pred.cols(); j++) {
            if (party == emp::ALICE) {
                a0 = target(i, j);
                b0 = pred(i, j);
                r = rn(i, j);
            } else {
                a1 = target(i, j);
                b1 = pred(i, j);
            }
            uint64_t tmp = exec_circ(a0, b0, r, a1, b1);

            if (party == emp::ALICE) {
                grad(i, j) = (uint32_t)(r / 2 + r % 2);
            } else {
                grad(i, j) = (uint32_t)(tmp / 2 + tmp % 2);
            }
        }
    }
}

uint64_t CrossEntropy::exec_circ(uint32_t an0, uint32_t bn0, uint64_t rn, uint32_t an1, uint32_t bn1) {
    string circ = "/home/ame/GraduationProject/Code/demo/PrYouTube/circuit/div.txt";
    BristolFormat cf(circ.c_str());

    emp::Integer a0(32, an0, emp::ALICE);
    emp::Integer b0(32, bn0, emp::ALICE);
    emp::Integer r(64, rn, emp::ALICE);
    emp::Integer a1(32, an1, emp::BOB);
    emp::Integer b1(32, bn1, emp::BOB);
    emp::Integer res(64, 0, emp::PUBLIC);

    uint64_t ret;

    emp::Integer Ain(r);
    Ain.resize(128, false);
    Ain = Ain << 64;
    b0.resize(128, false);
    Ain = Ain ^ (b0 << 32);
    a0.resize(128, false);
    Ain = Ain ^ a0;

    emp::Integer Bin(b1);
    Bin.resize(64, false);
    Bin = Bin << 32;
    a1.resize(64, false);
    Bin = Bin ^ a1;

    cf.compute((emp::block *)res.bits.data(), (emp::block *)Ain.bits.data(), (emp::block *)Bin.bits.data());

    ret = res.reveal<uint64_t>(emp::BOB);
    return ret;
}
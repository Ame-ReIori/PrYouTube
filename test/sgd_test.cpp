#include "../include/utils.h"
#include "../include/opt/sgd.h"

void sgd_test(emp::NetIO *io, int party) {
    int row = 3;
    int col = 5;
    Matrix32u w, dw, w_, dw_, ww;
    w.resize(row, col);
    dw.resize(row, col);
    w_.resize(row, col);
    dw_.resize(row, col);
    ww.resize(row, col);

    random_fixed_matrix32u(w);
    random_fixed_matrix32u(dw);

    SGD *sgd = new SGD();

    if (party == emp::ALICE) {
        io->send_data(w.data(), w.size() * 4);
    } else {
        io->recv_data(w_.data(), w_.size() * 4);
        ww = w + w_;
    }

    sgd->update(w, dw);

    if (party == emp::ALICE) {
        io->send_data(w.data(), w.size() * 4);
        io->send_data(dw.data(), dw.size() * 4);
    } else {
        io->recv_data(w_.data(), w_.size() * 4);
        io->recv_data(dw_.data(), dw_.size() * 4);
        w += w_;
        dw += dw_;
    }

    std::cout << ww.cast<int32_t>().cast<float>() / SCALE_NUM << std::endl << std::endl;
    std::cout << dw.cast<int32_t>().cast<float>() / SCALE_NUM << std::endl << std::endl;
    std::cout << ww.cast<int32_t>().cast<float>() / SCALE_NUM - 0.01 * dw.cast<int32_t>().cast<float>() / SCALE_NUM << std::endl << std::endl;
    std::cout << w.cast<int32_t>().cast<float>() / SCALE_NUM << std::endl << std::endl;

}

int main(int argc, char **argv) {
    int port, party;
    emp::parse_party_and_port(argv, &party, &port);
    emp::NetIO *io = new emp::NetIO(party == emp::ALICE ? nullptr : "127.0.0.1", port);

    #ifdef SGD_TEST
        sgd_test(io, party);
    #endif
}
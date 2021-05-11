#include "../include/layers/relu.h"

void msb_test(emp::NetIO *io, int party) {
    Layer *relu = new ReLU(io, party);  
    int row_num = 128;
    int col_num = 64;

    Matrix32u bottom, bottom_, top, top_;
    bottom.resize(row_num, col_num);
    bottom_.resize(row_num, col_num);
    top.resize(row_num, col_num);
    top_.resize(row_num, col_num);

    random_fixed_matrix32u(bottom);

    relu->setup();
    relu->forward(bottom);

    top = relu->output();
      
    if (party == emp::ALICE) {
        io->send_data(bottom.data(), bottom.size() * 4);
        io->send_data(top.data(), top.size() * 4);
    } else {
        io->recv_data(bottom_.data(), bottom_.size() * 4);
        io->recv_data(top_.data(), top_.size() * 4);
        bottom += bottom_;
        top += top_;
        std::cout << (bottom == top) << std::endl << std::endl;
    }
}

int main(int argc, char **argv) {
    int port, party;
    emp::parse_party_and_port(argv, &party, &port);
    emp::NetIO *io = new emp::NetIO(party == emp::ALICE ? nullptr : "127.0.0.1", port);

    #ifdef MSB_TEST
        msb_test(io, party);
    #endif
}


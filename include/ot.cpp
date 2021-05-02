#include "./ot.h"
#include "utils.h"

OT::OT(emp::NetIO *io) {
    this->io = io;
    G = new Group();
}

OT::~OT() {
    if (G != nullptr) {
        delete G;
    }
}

void OT::send(const uint8_t *data0, const uint8_t *data1, const int len) {
    BigInteger r;
    Point gr;

    // initial sender's sk and pk
    G->get_rand_bn(r);
    gr = G->mul_gen(r);

    // receive msg from receiver
    uint8_t x0[SECP160R1_OCT_UNCOMPRESSED_LENGTH];
    uint8_t x1[SECP160R1_OCT_UNCOMPRESSED_LENGTH];

    io->recv_data(x0, SECP160R1_OCT_UNCOMPRESSED_LENGTH);
    io->recv_data(x1, SECP160R1_OCT_UNCOMPRESSED_LENGTH);

    io->flush();

    #ifdef IO_DEBUG
        printf("round 1: \n");
        for (int i = 0; i < SECP160R1_OCT_UNCOMPRESSED_LENGTH; i++) {
            printf("%02x", x0[i]);
        }
        printf("\n");

        for (int i = 0; i < SECP160R1_OCT_UNCOMPRESSED_LENGTH; i++) {
            printf("%02x", x1[i]);
        }
        printf("\n");
    #endif

    // view <x0, x1> as two random point, and calculate x0^r, x1^r
    // result saved in <h0, h1>
    Point x0_point, x1_point;
    x0_point.from_oct(G, x0, SECP160R1_OCT_UNCOMPRESSED_LENGTH);
    x1_point.from_oct(G, x1, SECP160R1_OCT_UNCOMPRESSED_LENGTH);

    Point h0_point, h1_point;
    h0_point = x0_point.mul(r);
    h1_point = x1_point.mul(r);

    // convert point to bytes and execute KDF to get <mask0, mask1>
    uint8_t mask0[len];
    uint8_t mask1[len];

    get_mask_from_point(mask0, h0_point, SECP160R1_OCT_UNCOMPRESSED_LENGTH, len);
    get_mask_from_point(mask1, h1_point, SECP160R1_OCT_UNCOMPRESSED_LENGTH, len);

    // xor mask and data
    // saved in <s0. s1>
    uint8_t s0[len], s1[len];
    arbitrary_xor(s0, mask0, data0, len);
    arbitrary_xor(s1, mask1, data1, len);

    // sender send <gr, s0, s1>
    // first encode gr
    uint8_t gr_str[SECP160R1_OCT_UNCOMPRESSED_LENGTH];
    gr.to_oct(gr_str, SECP160R1_OCT_UNCOMPRESSED_LENGTH);
    
    io->send_data(gr_str, SECP160R1_OCT_UNCOMPRESSED_LENGTH);
    io->send_data(s0, len);
    io->send_data(s1, len);

    #ifdef IO_DEBUG
        printf("round 2: \n");
        for (int i = 0; i < SECP160R1_OCT_UNCOMPRESSED_LENGTH; i++) {
            printf("%02x", gr_str[i]);
        }
        printf("\n");

        for (int i = 0; i < len; i++) {
            printf("%02x", s0[i]);
        }
        printf("\n");

        for (int i = 0; i < len; i++) {
            printf("%02x", s1[i]);
        }
        printf("\n");
    #endif
}

void OT::recv(uint8_t *r, const bool b, const int len) {

    BigInteger sk; // as sk, and recevier generate pk using this sk
    Point pk, h; // <sk, pk> is a pair, and receiver cannot get h'sk

    uint8_t x0[SECP160R1_OCT_UNCOMPRESSED_LENGTH];
    uint8_t x1[SECP160R1_OCT_UNCOMPRESSED_LENGTH];

    //initial keys
    G->get_rand_bn(sk);
    pk = G->mul_gen(sk);

    G->get_rand_point(h);

    // prepare msg
    prepare_2_tuple_msg(x0, x1, pk, h, b);

    io->send_data(x0, SECP160R1_OCT_UNCOMPRESSED_LENGTH);
    io->send_data(x1, SECP160R1_OCT_UNCOMPRESSED_LENGTH);

    #ifdef IO_DEBUG
        printf("round 1: \n");
        for (int i = 0; i < SECP160R1_OCT_UNCOMPRESSED_LENGTH; i++) {
            printf("%02x", x0[i]);
        }
        printf("\n");

        for (int i = 0; i < SECP160R1_OCT_UNCOMPRESSED_LENGTH; i++) {
            printf("%02x", x1[i]);
        }
        printf("\n");
    #endif

    uint8_t gr_str[SECP160R1_OCT_UNCOMPRESSED_LENGTH];
    uint8_t s0[len];
    uint8_t s1[len];

    io->recv_data(gr_str, SECP160R1_OCT_UNCOMPRESSED_LENGTH);
    io->recv_data(s0, len);
    io->recv_data(s1, len);

    #ifdef IO_DEBUG
        printf("round 2: \n");
        for (int i = 0; i < SECP160R1_OCT_UNCOMPRESSED_LENGTH; i++) {
            printf("%02x", gr_str[i]);
        }
        printf("\n");

        for (int i = 0; i < len; i++) {
            printf("%02x", s0[i]);
        }
        printf("\n");

        for (int i = 0; i < len; i++) {
            printf("%02x", s1[i]);
        }
        printf("\n");
    #endif

    // get x_{b}, stored in r
    Point gr, hb_point;
    uint8_t hb[SECP160R1_OCT_UNCOMPRESSED_LENGTH];
    uint8_t mask[len];

    gr.from_oct(G, gr_str, SECP160R1_OCT_UNCOMPRESSED_LENGTH);
    hb_point = gr.mul(sk);
    
    get_mask_from_point(mask, hb_point, SECP160R1_OCT_UNCOMPRESSED_LENGTH, len);

    if (b == 0) { // r = mask ^ s0
        arbitrary_xor(r, mask, s0, len);
    } else { // r = mask ^ s1
        arbitrary_xor(r, mask, s1, len);
    }

}

void prepare_2_tuple_msg(uint8_t *x0, uint8_t *x1, Point pk, Point h, const bool b) {
    if (b == 0) { // send <pk, h>
        pk.to_oct(x0, SECP160R1_OCT_UNCOMPRESSED_LENGTH);
        h.to_oct(x1, SECP160R1_OCT_UNCOMPRESSED_LENGTH);
    } else { // send <h, pk>
        h.to_oct(x0, SECP160R1_OCT_UNCOMPRESSED_LENGTH);
        pk.to_oct(x1, SECP160R1_OCT_UNCOMPRESSED_LENGTH);
    }
}

void get_mask_from_point(uint8_t *mask, Point p, const int point_len, const int len) {
    uint8_t p_str[point_len];

    p.to_oct(p_str, point_len);
    
    KDF(mask, len, p_str, point_len);
}
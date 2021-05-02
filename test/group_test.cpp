#include "../include/group.h"
#include "./test.h"
#include <openssl/bn.h>
#include <openssl/ec.h>

void BN_test() {
    int len = 256;
    uint8_t a_bin[len], b_bin[len], n_bin[len];
    BigInteger a, b, n, res;
    BN_CTX *ctx = BN_CTX_new();

    emp::PRG prg;
    prg.random_data(a_bin, len);
    prg.random_data(b_bin, len);
    prg.random_data(n_bin, len);

    a.from_bin(a_bin, len);
    b.from_bin(b_bin, len);
    n.from_bin(n_bin, len);

    // check initial function
    std::cout << "==================== INITIAL ACCURACY TEST ====================" << std::endl;

    std::cout << "a: ";
    BN_print_fp(stdout, a.n);
    std::cout << std::endl;

    std::cout << "b: ";
    BN_print_fp(stdout, b.n);
    std::cout << std::endl;
    
    std::cout << "n: ";
    BN_print_fp(stdout, n.n);
    std::cout << std::endl;

    // check operation
    std::cout << "==================== OP ACCURACY TEST ====================" << std::endl;
    
    res = a.add(b);
    std::cout << "add: ";
    BN_print_fp(stdout, res.n);
    std::cout << std::endl;

    res = a.mul(b, ctx);
    std::cout << "mul: ";
    BN_print_fp(stdout, res.n);
    std::cout << std::endl;

    res = a.mod(n, ctx);
    std::cout << "mod: ";
    BN_print_fp(stdout, res.n);
    std::cout << std::endl;

    res = a.add_mod(b, n, ctx);
    std::cout << "add_mod: ";
    BN_print_fp(stdout, res.n);
    std::cout << std::endl;

    res = a.mul_mod(b, n, ctx);
    std::cout << "mul_mod: ";
    BN_print_fp(stdout, res.n);
    std::cout << std::endl;

    std::cout << "==================== MEMORY ACCURACY TEST ====================" << std::endl;

    for (int i = 0; i < 100000; i++) {
        res = a.add(b);
        res = a.mul(b, ctx);
        res = a.mod(n, ctx);
        res = a.add_mod(b, n, ctx);
        res = a.mul_mod(b, n, ctx);
    }

    std::cout << "PASS" << std::endl;
    
    BN_CTX_free(ctx);
}

void Point_Group_test() {
    Group G;
    Point pk, g;

    BigInteger tmp_sk;
    Point tmp_pk;

    BIGNUM *x, *y;
    x = BN_new();
    y = BN_new();

    int batch = 1000;

    // check random point accuracy
    std::cout << "==================== RANDOM POINT ACCURACY TEST ====================" << std::endl;

    G.get_rand_point(pk);
    EC_POINT_get_affine_coordinates_GFp(G.ec_group, pk.point, x, y, G.ctx);

    BN_print_fp(stdout, x);
    std::cout << std::endl;
    BN_print_fp(stdout, y);
    std::cout << std::endl;

    printf("Is the point on curve?  %d\n", EC_POINT_is_on_curve(G.ec_group, pk.point, G.ctx));

    std::cout << "==================== PRESSION TEST ====================" << std::endl;
    for (int i = 0; i < batch; i++) {
        G.get_rand_bn(tmp_sk);
        tmp_pk = G.mul_gen(tmp_sk);

        G.get_rand_point(pk);
        g = G.get_generator();
    }
    std::cout << "PASS" << std::endl;
}

int main() {

    // BigInteger test
    #ifdef BN_TEST
        BN_test();
    #endif

    // point and group test
    #ifdef PG_TEST
        Point_Group_test();
    #endif
}
#include "../include/triple.h"

void batch_test(emp::NetIO *io, int party) {
    Triple *triple = new Triple(io);

    int batch = 1024 * 128;
    uint64_t *a = new uint64_t[batch];
    uint64_t *b = new uint64_t[batch];
    uint64_t *c = new uint64_t[batch];

    std::cout << "==================== BATCH TRIPLE GEN PERFORMANCE TEST ====================" << std::endl;
    auto s = emp::clock_start();
    triple->gen_batch(a, b, c, batch, party, false);
    double e = emp::time_from(s);
    
    printf("generate %d triples in %f ms.\n", batch, e / 1000);

    #ifdef OUTPUT
        for (int i = 0; i < batch; i++) {
            printf("%016lx %016lx %016lx\n", a[i], b[i], c[i]);
        }
    #endif
}

void matrix_test(emp::NetIO *io, int party) {
    Triple *triple = new Triple(io);
    int m = 2;
    int d = 3;
    int n = 1;

    Matrix64u a, b, c;
    auto s= emp::clock_start();
    triple->gen_matrix(a, b, c, m, d, n, party);
    double end = emp::time_from(s);
    
    printf("generate %dx%d and %dx%d triples in %f ms.\n", m, d, d, n, end / 1000);

    std::cout << a << std::endl << std::endl;
    std::cout << b << std::endl << std::endl;
    std::cout << c << std::endl << std::endl;

    Matrix64u va, vb;

    va.resize(m, d);
    vb.resize(d, n);

    random_matrix64u(va);
    random_matrix64u(vb);

    std::cout << va << std::endl << std::endl;
    std::cout << vb << std::endl << std::endl;

    Matrix64u e0 = va - a;
    Matrix64u f0 = vb - b;

    std::cout << e0 << std::endl << std::endl;
    std::cout << f0 << std::endl << std::endl;

    Matrix64u e1, f1;
    e1.resize(m, d);
    f1.resize(d, n);

    if (party == emp::ALICE) {
        io->send_data(e0.data(), e0.size() * 8);
        io->send_data(f0.data(), f0.size() * 8);
    } else {
        io->recv_data(e1.data(), e0.size() * 8);
        io->recv_data(f1.data(), f0.size() * 8);
    }

    if (party == emp::ALICE) {
        io->recv_data(e1.data(), e0.size() * 8);
        io->recv_data(f1.data(), f0.size() * 8);
    } else {
        io->send_data(e0.data(), e0.size() * 8);
        io->send_data(f0.data(), f0.size() * 8);
    }

    std::cout << e1 << std::endl << std::endl;
    std::cout << f1 << std::endl << std::endl;

    Matrix64u e = e0 + e1;
    Matrix64u f = f0 + f1;

    Matrix64u t = (party - 1) * e * f + a * f + e * b + c;

    std::cout << t << std::endl;
}

inline uint32_t FLOAT2FIX(float a, int scale) {
    return (uint32_t)(a * (1 << scale));
}

void fixed_matrix_mul_test(emp::NetIO *io, int party) {
    Triple *triple = new Triple(io);
    Eigen::MatrixXf ta, tb;
    int m = 3, d = 5, n = 4;
    Matrix32u van, vbn, ra, rb, va, vb;
    ta.resize(m, d);
    tb.resize(d, n);
    ra.resize(m, d);
    rb.resize(d, n);
    va.resize(m, d);
    vb.resize(d, n);

    ta << 0.3151515, 0.4386738942, -0.1238910, 0.3583904, -0.10489571,
          -0.1957195, 0.34568935, 0.25906202, 0.269082, -0.2940752,
          -0.19057, 0.2430952, 0.9248502, 0.2048920, -0.2834906;

    tb << -0.4230957, 0.4239607, 0.983476, -0.19325,
          0.20493582, -0.295820, 0.20948, 0.46986,
          0.23940682, -0.239458245, 0.2945820, 0.0294345,
          -0.392582, 0.43298052, 0.9234850, -0.209485092,
          0.3294582, 0.24905820, 0.786564, 0.2490582;

    van = (ta * (1 << 18)).cast<uint32_t>();
    vbn = (tb * (1 << 18)).cast<uint32_t>();

    random_matrix32u(ra);
    random_matrix32u(rb);

    if (party == emp::ALICE) {
        va = ra;
        vb = rb;
    } else {
        va = van - ra;
        vb = vbn - rb;
    }

    Matrix64u va64, vb64, e0, f0, e1, f1, e, f, a, b, c;
    triple->gen_matrix(a, b, c, m, d, n, party);
    
    va64 = va.cast<int32_t>().cast<uint64_t>();
    vb64 = vb.cast<int32_t>().cast<uint64_t>();

    e0.resize(m, d);
    e1.resize(m, d);
    e.resize(m, d);

    f0.resize(d, n);
    f1.resize(d, n);
    f.resize(d, n);
    
    e0 = va64 - a;
    f0 = vb64 - b;

    if (party == emp::ALICE) {
        io->send_data(e0.data(), e0.size() * 8);
        io->send_data(f0.data(), f0.size() * 8);
        io->recv_data(e1.data(), e1.size() * 8);
        io->recv_data(f1.data(), f1.size() * 8);
    } else {
        io->recv_data(e1.data(), e1.size() * 8);
        io->recv_data(f1.data(), f1.size() * 8);
        io->send_data(e0.data(), e0.size() * 8);
        io->send_data(f0.data(), f0.size() * 8);
    }

    e = e0 + e1;
    f = f0 + f1;
    
    Matrix64u vc64 = (party - 1) * e * f + a * f + e * b + c;
    Matrix32u vc = (vc64 / (1 << 18)).cast<uint32_t>();

    std::cout << vc << std::endl << std::endl;
}

int main(int argc, char **argv) {
    int port, party;
    emp::parse_party_and_port(argv, &party, &port);
    emp::NetIO *io = new emp::NetIO(party == emp::ALICE ? nullptr : "127.0.0.1", port);

    #ifdef BATCH_TEST
        batch_test(io, party);
    #endif

    #ifdef MATRIX_TEST
        matrix_test(io, party);
    #endif

    #ifdef FIXED_MATRIX_TEST
        fixed_matrix_mul_test(io, party);
    #endif
}
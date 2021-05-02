#include "./group.h"
#include <emp-tool/utils/utils.h>
#include <openssl/bn.h>
#include <openssl/ec.h>

BigInteger::BigInteger() {
    n = BN_new();
}

BigInteger::BigInteger(const BigInteger &num) {
    n = BN_new();
    BN_copy(n, num.n);
}

BigInteger& BigInteger::operator=(BigInteger num) {
    std::swap(n, num.n);
    return *this;
}

BigInteger::~BigInteger() {
    if (n != nullptr) BN_free(n);
}

int BigInteger::size() {
    return BN_num_bytes(n);
}

/*
void BigInteger::to_hex(uint8_t *out) {
    out = (uint8_t *)BN_bn2hex(n);
}

void BigInteger::from_hex(const uint8_t *in, const int inlen) {
    uint8_t tmp[inlen];
    memcpy(tmp, in, inlen);
    BN_hex2bn(&n, (const char *)tmp);
}
*/

void BigInteger::to_bin(uint8_t *out) {
    BN_bn2bin(n, out);
}

void BigInteger::from_bin(const uint8_t *in, const int inlen) {
    BN_free(n);
    n = BN_bin2bn(in, inlen, nullptr);
}

BigInteger BigInteger::add(const BigInteger &num) {
    BigInteger res;
    BN_add(res.n, n, num.n);
    return res;
}

BigInteger BigInteger::mul(const BigInteger &num, BN_CTX *ctx) {
    BigInteger res;
    BN_mul(res.n, n, num.n, ctx);
    return res;
}

BigInteger BigInteger::mod(const BigInteger &num, BN_CTX *ctx) {
    BigInteger res;
    BN_mod(res.n, n, num.n, ctx);
    return res;
}

BigInteger BigInteger::add_mod(const BigInteger &num, const BigInteger &m, BN_CTX *ctx) {
    BigInteger res;
    BN_mod_add(res.n, n, num.n, m.n, ctx);
    return res;
}

BigInteger BigInteger::mul_mod(const BigInteger &num, const BigInteger &m, BN_CTX *ctx) {
    BigInteger res;
    BN_mod_mul(res.n, n, num.n, m.n, ctx);
    return res;
}

Point::Point(Group *g) {
    if (g == nullptr) return;
    this->group = g;
    point = EC_POINT_new(g->ec_group);
}

Point::Point(const Point &p) {
    if (p.group == nullptr) return;
    this->group = p.group;
    point = EC_POINT_new(group->ec_group);
    if (!EC_POINT_copy(point, p.point)) emp::error("ECC COPY");
}

Point& Point::operator=(Point p) {
    std::swap(point, p.point);
    std::swap(group, p.group);
    return *this;
}

Point::~Point() {
    if (point != nullptr) EC_POINT_free(point);
}

void Point::to_oct(uint8_t *out, int len) {
    if (!EC_POINT_point2oct(group->ec_group, point, POINT_CONVERSION_UNCOMPRESSED, out, len, group->ctx)) 
        emp::error("POINT_TO_OCT ERR");
}

void Point::from_oct(Group *g, const uint8_t *in, const int inlen) {
    if (point == nullptr) {
        group = g;
        point = EC_POINT_new(group->ec_group);
    } 
    if (!EC_POINT_oct2point(group->ec_group, point, in, inlen, group->ctx)) 
        emp::error("OCT_TO_POING ERR");
}

int Point::size() {
    int size = EC_POINT_point2oct(group->ec_group, point, POINT_CONVERSION_UNCOMPRESSED, NULL, 0, group->ctx);
    if (!size) emp::error("SIZE ERR");
    return size;
}

Point Point::add(const Point &p) {
    Point res(group);
    if (!EC_POINT_add(group->ec_group, res.point, point, p.point, group->ctx))
        emp::error("POINT ADD ERR");
    return res;
}

Point Point::mul(const BigInteger &num) {
    Point res(group);
    if (!EC_POINT_mul(group->ec_group, res.point, NULL, point, num.n, group->ctx))
        emp::error("POINT MUL ERR");
    return res;
}

Point Point::inv() {
    Point res(*this);
    if (!EC_POINT_invert(group->ec_group, res.point, group->ctx))
        emp::error("POINT INV ERR");
    return res;
}

Group::Group() {
    ec_group = EC_GROUP_new_by_curve_name(NID_secp160r1);
    ctx = BN_CTX_new();
    EC_GROUP_precompute_mult(ec_group, ctx); // for faster multiplication
    EC_GROUP_get_order(ec_group, order.n, ctx);
}

Group::~Group() {
    if (ec_group != nullptr) EC_GROUP_free(ec_group);
    if (ctx != nullptr) BN_CTX_free(ctx);
}

void Group::get_rand_bn(BigInteger &n) {
    BN_rand_range(n.n, order.n);
}

void Group::get_rand_point(Point &p) {
    BigInteger sk; // as sk, and it cannot be leaked
    get_rand_bn(sk);

    p = mul_gen(sk);
}

Point Group::get_generator() {
    Point res(this);
    if (!EC_POINT_copy(res.point, EC_GROUP_get0_generator(ec_group))) emp::error("GEN ERR");
    return res;
}

Point Group::mul_gen(const BigInteger &num) {
    Point res(this);
    if (!EC_POINT_mul(ec_group, res.point, num.n, NULL, NULL, ctx)) emp::error("MUL_GEN ERR");
    return res;
}
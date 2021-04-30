#ifndef __GROUP_H
#define __GROUP_H

/* curve group */

#include "./common.h"
#include <cstring>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/obj_mac.h>
#include <emp-tool/utils/utils.h>

class BigInteger {
    public:
        BIGNUM *n = nullptr;

        // constructor function
        BigInteger();
        BigInteger(const BigInteger &num);
        BigInteger& operator=(BigInteger num);
        
        ~BigInteger();

        int size();
        // void to_hex(uint8_t *out);
        // void from_hex(const uint8_t *in, const int inlen);
        void to_bin(uint8_t *out);
        void from_bin(const uint8_t *in, const int inlen);

        BigInteger add(const BigInteger &num);
        BigInteger mul(const BigInteger &num, BN_CTX *ctx);
        BigInteger mod(const BigInteger &num, BN_CTX *ctx);
        BigInteger add_mod(const BigInteger &num, const BigInteger &m, BN_CTX *ctx);
        BigInteger mul_mod(const BigInteger &num, const BigInteger &m, BN_CTX *ctx);
};

class Group;
class Point {
    public:
        EC_POINT *point = nullptr;
        Group *group = nullptr;
        
        Point(Group *g = nullptr);
        Point(const Point &p);
        Point& operator=(Point p);

        ~Point();

        void to_oct(uint8_t *out, int len);
        void from_oct(Group *g, const uint8_t *in, const int inlen);

        int size();

        Point add(const Point &p);
        Point mul(const BigInteger &num);
        Point inv();
};

class Group {
    public:
        EC_GROUP *ec_group = nullptr;
        BN_CTX *ctx = nullptr;

        BigInteger order;

        Group();
        ~Group();

        void get_rand_point(Point &p);
        Point get_generator();
        Point mul_gen(const BigInteger &num);
};



#endif
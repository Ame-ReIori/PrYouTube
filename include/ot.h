#ifndef __OT_H
#define __OT_H

#include "./common.h"
#include "./utils.h"
#include "./group.h"
#include <emp-tool/io/net_io_channel.h>


class OT {

    public:
        emp::NetIO *io;
        Group *G = nullptr;

        OT(emp::NetIO *io);
        ~OT();

        void send(const uint8_t *data0, const uint8_t *data1, const int len);
        void recv(uint8_t *r, const bool b, const int len);
};


void prepare_2_tuple_msg(uint8_t *x0, uint8_t *x1, Point pk, Point h, const bool b);

void get_mask_from_point(uint8_t *mask, Point p, const int point_len, const int len);

#endif
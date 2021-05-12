#ifndef __COMMON_H
#define __COMMON_H

/* include some commmon header files */

#include <iostream>
//#include <cstdint>
#include <eigen3/Eigen/Core>
#include <emp-tool/io/net_io_channel.h>

#define SCALE_NUM 8388608
#define FIXED_ONE 8388608
#define FIXED_HALF 4194304
#define FIXED_MINUS_HALF 4290772992
#define FIXED_QUARTER 2097152
#define FIXED_MINUS_QUARTER 4292870144

typedef Eigen::Matrix<uint64_t, Eigen::Dynamic, Eigen::Dynamic> Matrix64u;
typedef Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic> Matrix32u;

typedef Eigen::Matrix<uint64_t, Eigen::Dynamic, 1> Vector64u;
typedef Eigen::Matrix<uint32_t, Eigen::Dynamic, 1> Vector32u;

typedef Eigen::Array<uint64_t, 1, Eigen::Dynamic> RowVector64u;
typedef Eigen::Array<uint32_t, 1, Eigen::Dynamic> RowVector32u;

#endif
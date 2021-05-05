#ifndef __COMMON_H
#define __COMMON_H

/* include some commmon header files */

#include <iostream>
#include <cstdint>
#include <eigen3/Eigen/Core>

typedef Eigen::Matrix<uint64_t, Eigen::Dynamic, Eigen::Dynamic> Matrix64u;
typedef Eigen::Matrix<uint64_t, Eigen::Dynamic, 1> Vector64u;
typedef Eigen::Array<uint64_t, 1, Eigen::Dynamic> RowVector64u;

#endif
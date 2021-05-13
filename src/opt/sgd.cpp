#include "../../include/opt/sgd.h"

void SGD::update(Matrix32u &w, Matrix32u &dw) {
    Matrix32u tmp = ((uint64_t)lr * dw.cast<int32_t>().cast<uint64_t>() / SCALE_NUM).cast<uint32_t>();

    w -= tmp;
}
#include "../include/ss.h"

void Shr(Matrix32u &s0, Matrix32u &s1, Matrix32u &s) {
    random_matrix32u(s0);
    s1 = s - s0;
}

void Rec(Matrix32u &s, Matrix32u &s0, Matrix32u &s1) {
    s = s0 + s1;
}
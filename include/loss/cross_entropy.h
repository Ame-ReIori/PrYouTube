#ifndef __CROSS_ENTROPY_H
#define __CROSS_ENTROPY_H

#include "../loss.h"
#include "../utils.h"
#include <emp-sh2pc/semihonest.h>
#include <emp-tool/circuits/circuit_file.h>

class CrossEntropy : public Loss {
    private:
        emp::NetIO *io;
        int party;
        uint64_t exec_circ(uint32_t an0, uint32_t bn0, uint64_t r, uint32_t an1, uint32_t bn1);

    public:
        CrossEntropy(emp::NetIO *io, int party) {
            this->io = io;
            this->party = party;
            emp::setup_semi_honest(io, party);
        }

        ~CrossEntropy() {
            emp::finalize_semi_honest();
        }

        void evaluate(const Matrix32u & pred, const Matrix32u &target);
};

#endif
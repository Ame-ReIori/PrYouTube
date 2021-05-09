#ifndef __NETWORK_H
#define __NETWORK_H

#include <vector>

#include "./common.h"
#include "./layer.h"
// #include "./loss.h"

class Network {
    public:
        emp::NetIO *io;
        int party;
        std::vector<Layer *> layers;
        // Loss *loss;

        Network(emp::NetIO *io, int party) {
            this->io = io;
            this->party = party;
            // loss = NULL;
        }

        ~Network() {
            // free layers and loss;
            for (int i = 0; i < layers.size(); i++) {
                delete layers[i];
            }
            // if (loss) delete loss;
        }

};


#endif
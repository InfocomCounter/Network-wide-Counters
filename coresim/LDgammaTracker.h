#ifndef SIMULATOR_LDGAMMATRACKER_H
#define SIMULATOR_LDGAMMATRACKER_H

#include <vector>
#include "GammaTracker.h"
#include <array>

class Flow;
class Packet;
class Queue;

class LDgammaTracker {

public:

    LDgammaTracker(Flow* flow);
    void notifyPacketEnque(Packet* p, Queue* queue);
    void notifyPacketDrop(Packet* p, Queue* queue);
    std::pair<bool, std::string> finalizeResults();
    std::string getCounterErrors();

private:
    int num_packets;
    int last_arrived_packet;
    Flow* flow;
    std::vector<std::pair<int, int>> dropped_packets;
    int L;
    std::vector<GammaTracker> gammaTrackers;


    using arr4 = std::array<int, 6>;

    constexpr static arr4 Ns = {5, 5, 6, 6, 7, 7};
    constexpr static arr4 Ts = {2, 3, 2, 3, 2, 3};
};


#endif //SIMULATOR_LDGAMMATRACKER_H

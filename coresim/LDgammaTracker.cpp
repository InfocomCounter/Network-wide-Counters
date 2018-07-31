#include <assert.h>
#include <sstream>
#include <iostream>
#include "LDgammaTracker.h"
#include "packet.h"

constexpr LDgammaTracker::arr4 LDgammaTracker::Ns, LDgammaTracker::Ts;

LDgammaTracker::LDgammaTracker(Flow *flow) {
//    std::cerr<< "start 1" << "\n";

    this->flow = flow;
    this->last_arrived_packet = -1;
    this->num_packets = 0;
    this->L = 0;

    for (int i = 0; i < Ns.size(); i++)
    {
        gammaTrackers.emplace_back(Ns[i], Ts[i]);
    }

//    std::cerr<< "end 1" << "\n";

}


void LDgammaTracker::notifyPacketEnque(Packet *p, Queue *queue) {
//    std::cerr<< "start 2" << "\n";
    if (queue != nullptr)
    {
        if (queue->src->type == HOST)
            p->packet_number_by_first_switch = num_packets++;
        return;
    }

    assert(p->packet_number_by_first_switch != last_arrived_packet);
    if (p->packet_number_by_first_switch > last_arrived_packet)
    {
        last_arrived_packet = p->packet_number_by_first_switch;
    }
    else
    {
//        if ((last_arrived_packet >> 9) != (p->packet_number_by_first_switch >> 9))
//        {
//            std::cerr << "AAAAA:" << " "  << flow->id << "\n";
//            while (true) {};
//        }

        L = std::max(L, last_arrived_packet - p->packet_number_by_first_switch);
    }

    for (auto& tracker : gammaTrackers)
    {
        tracker.update(p->packet_number_by_first_switch);
    }

//    std::cerr<< "end 2" << "\n";

}

void LDgammaTracker::notifyPacketDrop(Packet *p, Queue *queue) {
//    std::cerr<< "start 3" << "\n";
    int insertion_pos = -1;
    for (int i = 0; i < dropped_packets.size(); i++) {
        if (dropped_packets[i].second + 1 == p->packet_number_by_first_switch) {
            dropped_packets[i].second++;
            insertion_pos = i;
            break;
        }
    }
    if (insertion_pos == -1) {
        insertion_pos = dropped_packets.size();
        dropped_packets.emplace_back(p->packet_number_by_first_switch, p->packet_number_by_first_switch);
    }

    for (int i = 0; i < dropped_packets.size(); i++) {
        if (dropped_packets[insertion_pos].second + 1 == dropped_packets[i].first) {
            dropped_packets[insertion_pos].second = dropped_packets[i].second;
            dropped_packets.erase(dropped_packets.begin() + i);
            break;
        }
    }
    p->flow->receive(p);
//    std::cerr<< "end 3" << "\n";
}

std::pair<bool, std::string> LDgammaTracker::finalizeResults() {
//    std::cerr<< "start 4" << "\n";
    std::stringstream ss;
    for (const auto& seg1 : dropped_packets)
    {
        for (const auto& seg2 : dropped_packets)
        {
            if (&seg2 == &seg1)
            {
                continue;
            }
            if (seg2.first - 1 <= seg1.second && seg2.second + 1 >= seg1.first)
            {
                return std::make_pair(true, "Fail");
            }
            if (seg1.first - 1 <= seg2.second && seg1.second + 1 >= seg2.first)
            {
                return std::make_pair(true, "Fail");
            }
        }
    }

    ss << "R = " << L << " L = ";
    int D = 0;
    for (const auto& seg : dropped_packets)
    {
        D = std::max(D, seg.second - seg.first + 1);
    }
    ss << D;

    bool is_intersting = (L+D) > 0;

    for (int i = 0; i < Ns.size(); i++)
    {
        is_intersting |= gammaTrackers[i].getMinGamma(num_packets) > 1;
        ss << " " << gammaTrackers[i].getGammaProfile(num_packets);
    }
//    std::cerr<< "end 4" << "\n";
    return std::make_pair(is_intersting, ss.str());
}

std::string LDgammaTracker::getCounterErrors() {
//    std::cerr<< "start 5" << "\n";
    std::stringstream ss;
    for (int i = 0; i < Ns.size(); i++)
    {
        ss << gammaTrackers[i].getErrorsString(num_packets);
    }
//    std::cerr<< "end 5" << "\n";
    return ss.str();
}

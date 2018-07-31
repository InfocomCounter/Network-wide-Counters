#include <climits>
#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include "assert.h"

#include "queue.h"
#include "packet.h"
#include "event.h"
#include "debug.h"

#include "../run/params.h"

extern double get_current_time(); // TODOm
extern void add_to_event_queue(Event* ev);
extern uint32_t dead_packets;
extern DCExpParams params;

uint32_t Queue::instance_count = 0;

/* Queues */
Queue::Queue(uint32_t id, double rate, uint32_t limit_bytes, int location) {
    this->id = id;
    this->unique_id = Queue::instance_count++;
    this->rate = rate; // in bps
    this->limit_bytes = limit_bytes;
    this->bytes_in_queue = 0;
    this->busy = false;
    this->queue_proc_event = NULL;
    //this->packet_propagation_event = NULL;
    this->location = location;

    if (params.ddc != 0) {
        if (location == 0) {
            this->propagation_delay = 10e-9;
        }
        else if (location == 1 || location == 2) {
            this->propagation_delay = 400e-9;
        }
        else if (location == 3) {
            this->propagation_delay = 210e-9;
        }
        else {
            assert(false);
        }
    }
    else {
        this->propagation_delay = params.propagation_delay;
    }
    this->p_arrivals = 0; this->p_departures = 0;
    this->b_arrivals = 0; this->b_departures = 0;

    this->pkt_drop = 0;
    this->spray_counter=std::rand();
    this->packet_transmitting = NULL;
}

void Queue::set_src_dst(Node *src, Node *dst) {
    this->src = src;
    this->dst = dst;
}

#define BASIC_FIFO 0
#define NOT_ACK 1
#define SRPT_ON_OUT 2

void Queue::enque(Packet *packet) {
    if (dst->type == 0 && dst->id ==7)
    {
     //   std::cerr << " + " << packets.size() << " " <<  packet->flow->id <<"\n";
    }
    p_arrivals += 1;
    b_arrivals += packet->size;
    packets.push_back(packet);
    bytes_in_queue += packet->size;
    if (bytes_in_queue + packet->size > limit_bytes)
    {
        //std::cerr << "well\n";
        pkt_drop++;
        size_t index = packets.size()-1;
        for (int i = packets.size()-1; i >= 0; --i)
        {
            if (params.fifo_settings == BASIC_FIFO && packets[i]->type == NORMAL_PACKET)
            {
                index = i;
                break;
            }

            if (params.fifo_settings == SRPT_ON_OUT && packets[i]->pf_priority > packets[index]->pf_priority)
            {
                index = i;
            }

        }
        Packet* p = packets[index];
        packets.erase(packets.begin() + index);
        bytes_in_queue -= p->size;
        drop(p);
        return;
    }
}

Packet *Queue::deque() {
    if (dst->type == 0 && dst->id ==7)
    {
     //   std::cerr << " - " << packets.size() << "\n";
    }
    if (bytes_in_queue > 0) {
        size_t index = 0;
        if (params.fifo_settings != NOT_ACK)
        {
            for (size_t i = 0; i < packets.size(); i++)
            {
                if (packets[i]->type != NORMAL_PACKET)
                {
                    index = i;
                    break;
                }
            }
        }
        Packet *p = packets[index];
        packets.erase(packets.begin()+index);
        bytes_in_queue -= p->size;
        p_departures += 1;
        b_departures += p->size;
        return p;
    }
    return NULL;
}

void Queue::drop(Packet *packet) {
    //std::cerr << dst->type << " -  " << dst->id << "\n";

    packet->flow->pkt_drop++;
    if(packet->seq_no < packet->flow->size){
        packet->flow->data_pkt_drop++;
    }
    if(packet->type == ACK_PACKET) {
        packet->flow->ack_pkt_drop++;
        Ack* ack = (Ack*) packet;
        delete ack;
    }

    packet->flow->tracker.notifyPacketDrop(packet, this);


    //if (location != 0 && packet->type == NORMAL_PACKET) {
        dead_packets += 1;
    //}

    if(debug_flow(packet->flow->id))
        std::cout << get_current_time() << " pkt drop. flow:" << packet->flow->id
            << " type:" << packet->type << " seq:" << packet->seq_no
            << " at queue id:" << this->id << " loc:" << this->location << "\n";
}

double Queue::get_transmission_delay(uint32_t size) {
    return size * 8.0 / rate;
}

void Queue::preempt_current_transmission() {
    if(params.preemptive_queue && busy){
        this->queue_proc_event->cancelled = true;
        assert(this->packet_transmitting);

        uint delete_index;
        bool found = false;
        for (delete_index = 0; delete_index < packets.size(); delete_index++) {
            if (packets[delete_index] == this->packet_transmitting) {
                found = true;
                break;
            }
        }
        if(found){
            bytes_in_queue -= packet_transmitting->size;
            packets.erase(packets.begin() + delete_index);
        }

        for(uint i = 0; i < busy_events.size(); i++){
            busy_events[i]->cancelled = true;
        }
        busy_events.clear();
        //drop(packet_transmitting);//TODO: should be put back to queue
        enque(packet_transmitting);
        packet_transmitting = NULL;
        queue_proc_event = NULL;
        busy = false;
    }
}

/* Implementation for probabilistically dropping queue */
ProbDropQueue::ProbDropQueue(uint32_t id, double rate, uint32_t limit_bytes,
        double drop_prob, int location)
    : Queue(id, rate, limit_bytes, location) {
        this->drop_prob = drop_prob;
    }

void ProbDropQueue::enque(Packet *packet) {
    p_arrivals += 1;
    b_arrivals += packet->size;

    if (bytes_in_queue + packet->size <= limit_bytes) {
        double r = (1.0 * rand()) / (1.0 * RAND_MAX);
        if (r < drop_prob) {
            return;
        }
        packets.push_back(packet);
        bytes_in_queue += packet->size;
        if (!busy) {
            add_to_event_queue(new QueueProcessingEvent(get_current_time(), this));
            this->busy = true;
            //if(this->id == 7) std::cout << "!!!!!queue.cpp:189\n";
            this->packet_transmitting = packet;
        }
    }
}

HostQueue::HostQueue(uint32_t id, double rate, uint32_t limit_bytes, int location) : Queue(id, rate, limit_bytes, location) {
    flows.clear();
}

void HostQueue::enque(Packet *packet) {
    std::cerr << packet -> seq_no << "\n";
    std::cerr << src ->type << " " <<dst->type;

    std::cerr << "sad";
    while (true) {

    }
}

void HostQueue::drop(Packet *packet) {
    std::cerr << "saad";
    while (true) {

    }
}

void HostQueue::registerFlow(Flow *flow) {
    flows.push_back(flow);
}

Packet *HostQueue::deque() {
    if (flows.size() == 0)
    {
        return nullptr;
    }
    Flow* first = *flows.begin();
    Packet* p = first->send_pending_data();
    if (first->finished)
    {
        flows.pop_front();
    }
    return p;
}

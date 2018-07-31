#include <assert.h>
#include "factory.h"
#include "pfabricqueue.h"

/* Factory method to return appropriate queue */
Queue* Factory::get_queue(
        uint32_t id, 
        double rate,
        uint32_t queue_size, 
        uint32_t type,
        double drop_prob, 
        int location
        ) { // Default drop_prob is 0.0

    switch(type) {
        case DROPTAIL_QUEUE:
            return new Queue(id, rate, queue_size, location);
        case PFABRIC_QUEUE:
            return new PFabricQueue(id, rate, queue_size, location);
        case PROB_DROP_QUEUE:
            return new ProbDropQueue(id, rate, queue_size, drop_prob, location);
    }
    assert(false);
    return NULL;
}

int Factory::flow_counter = 0;

Flow* Factory::get_flow(
        double start_time, 
        uint32_t size,
        Host *src, 
        Host *dst, 
        uint32_t flow_type,
        double rate
        ) {
    return Factory::get_flow(Factory::flow_counter++, start_time, size, src, dst, flow_type, rate);
}

Flow* Factory::get_flow(
        uint32_t id, 
        double start_time, 
        uint32_t size,
        Host *src, 
        Host *dst, 
        uint32_t flow_type,
        double rate
        ) { // Default rate is 1.0
    return new Flow(id, start_time, size, src, dst);
}

Host* Factory::get_host(
        uint32_t id, 
        double rate, 
        uint32_t queue_type, 
        uint32_t host_type
        ) {
    return new Host(id, rate, queue_type, NORMAL_HOST);;
}


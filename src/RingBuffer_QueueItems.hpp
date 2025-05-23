#ifndef RINGBUFFER_QUEUEITEMS_HPP
#define RINGBUFFER_QUEUEITEMS_HPP

#include "global_defines.hpp"
#define MAX_REQUESTS 20

class RingBuffer {
    public:
    RingBuffer() = default;
    ~RingBuffer() {
        for(queue_item* r : _requests) {
            r = nullptr;
        }
    }
    bool producer_push(queue_item* request) {
        size_t head = _head.load(std::memory_order_relaxed);
        size_t tail = _tail.load(std::memory_order_acquire);

        if((head+1) % MAX_REQUESTS == tail) {
            // Buffer is full
            return false;
        }

        _requests[head] = request;
        _head.store((head+1) % MAX_REQUESTS, std::memory_order_release);
        return true;
    }

    bool consumer_pop(queue_item** item) {
        size_t head = _head.load(std::memory_order_acquire);
        size_t tail = _tail.load(std::memory_order_relaxed);

        if(head == tail) {
            // Buffer is empty
            return false;
        }

        *item = _requests[tail];
#ifdef CONTROLLER_DEBUG
        _requests[tail] = nullptr;
#endif        
        _tail.store((tail+1) % MAX_REQUESTS, std::memory_order_release);

        return true;
    }


#ifdef CONTROLLER_DEBUG
    void debug_state() const {
        std::cout << "RingBuffer State:" << std::endl;
        std::cout << "  head: " << _head.load() << std::endl;
        std::cout << "  tail: " << _tail.load() << std::endl;
        std::cout << "  slots: " << MAX_REQUESTS << std::endl;
        for (size_t i = 0; i < MAX_REQUESTS; ++i) {
            if(_requests[i] != nullptr) {
                std::cout << "Operation: " << _requests[i]->op << " address: " << _requests[i]->address << " data: " << _requests[i]->data << " size: " << _requests[i]->size << " slot: " << _requests[i]->slot << std::endl;
            } else {
                std::cout << "[NULL]" << std::endl;
            }
            
        }
        std::cout << std::endl;
    }
#endif
    private:
    queue_item* _requests[MAX_REQUESTS];
    std::atomic<size_t> _head{0};
    std::atomic<size_t> _tail{0};
};

#endif // RINGBUFFER_QUEUEITEMS_HPP
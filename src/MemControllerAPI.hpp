#ifndef MEMORY_CONTROLLER_CORE_HPP
#define MEMORY_CONTROLLER_CORE_HPP
#include "Error_Reg.hpp"
#include "Logger.hpp"
#include "global_defines.hpp"
#include <pthread.h>
#include <atomic>
#include <cstring>
#include <immintrin.h> 
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include "RingBuffer_QueueItems.hpp"
#include <thread>
#ifdef DEBUG
#include "Logger.hpp"
    #ifdef IS_BIG_ENDIAN
    #warning "BIG ENDIAN IS ENABLED"
    #endif
    #ifdef IS_LITTLE_ENDIAN
    #warning "LITTLE ENDIAN IS ENABLED"
    #endif

#endif
// NOTICE:
// This controller is used to manage memory in a generic way
// It is not a real memory controller, but a wrapper around mmap and munmap
// The Controller in the FPGA will be a hardware controller without any boundary checks etc.
// This is only for the Emulator in its VM State

#define MEMORY_FLAG 0xDEADBEEF
#define MEMORY_METAS 6
#define NULL_PTR 0x0
// these defines are used to access the metas via a substract!
// these are not the real offsets in memory
#define MEMORY_MAGIC_POS 6
#define MEMORY_SIZE_POS 5
#define MEMORY_MIN_POS 4
#define MEMORY_MAX_POS 3
#define MEMORY_PREF_POS 2
#define MEMORY_NEXT_POS 1
#define MEMORY_DATA_POS 0


#define ONE_GB 1000000000
#define FOUR_HUNDRED_MB 400000000
// we need 256 slots for the queue
// bits explained:
// 0000 0001 bit for reservation
// 0000 0010 bit for write
// 0000 0100 bit for output ready
#define QUEUE_SLOTS 10 



struct Memory_Controller_Core {
    // this needs to be rewritten in assembly for a ULP Core:
    // IO queues:
    queue_item queue[QUEUE_SLOTS];
    RingBuffer reqs;
    // bitarrays for the queues:
    std::atomic<uint8_t> queue_status_bitarray[QUEUE_SLOTS];

#ifdef CONTROLLER_DEBUG
        std::atomic<int64_t> last_op_index = -1;
        std::atomic<uint64_t> last_write_data = 0;
        std::atomic<uint64_t> last_read_addr = 0;
        std::atomic<uint64_t> last_read_result = 0;
        std::atomic<uint64_t> last_operation = 0;
        
#endif
    void debug_queue_bits();
    void debug_queue_bits(int index);
    // DO NOT CHANGE THE MEM_PTR AT RUNTIME EVER!
    // this pointer is a pointer to the memory allocated by mmap and is used by a seperate thread!
    uint8_t* _mem_ptr;
    uint64_t _max_address;
    uint64_t _min_address;
    uint64_t _size;
    pthread_t thread;
    std::atomic<bool> running = false;
    std::atomic<bool> ready = false;
    void debug_errors();
    void init(uint64_t size);
    void start();
    void set_affinity(int core_id);
    // stops the controller and frees memory
    void stop();
    void loop();
    int add_to_input_queue(queue_item in);
    void add_to_output_queue(uint64_t out, uint64_t index);
    bool get_from_input_queue(queue_item*& in);
    uint64_t get_from_output_queue(uint64_t index);
    void wait_for_controller_to_start();
    // Initialize memory with mmap
    uint8_t* init_mem(uint64_t size);
    void free_mem(uint8_t* mem);
    // make sure to give a correct memory pointer in the function!!!
    uint64_t get_item(uint8_t* _mem, uint64_t in_address, uint16_t size);

    void set_item(uint8_t* _mem, uint64_t in_address, uint64_t data, uint16_t size);


};

// Entry for the thread:
void* thread_entry(void* arg);

#endif
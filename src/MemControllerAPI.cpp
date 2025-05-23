#include "MemControllerAPI.hpp"

void Memory_Controller_Core::debug_queue_bits() {
    std::cout << "Slot Status Bits:" << std::endl;
    for(int i = 0; i < QUEUE_SLOTS; i++)  {
        std::cout << "Slot " << i << ": " << (int)queue_status_bitarray[i].load(std::memory_order_acquire) << std::endl;
    }
}

void Memory_Controller_Core::debug_queue_bits(int index) {
    std::cout << "Statusbit at index: " << index << " bit-state: " << (int)queue_status_bitarray[index].load(std::memory_order_acquire) << std::endl;
}

void Memory_Controller_Core::debug_errors()
{
    CATCH_MEM_ERROR(READ_ERROR)
    {
        LOG_DEBUG("[MEMORY CONTROLLER]: Read error");
    }
    CATCH_MEM_ERROR(BOUNDARY_ERROR)
    {
        LOG_DEBUG("[MEMORY CONTROLLER]: Boundary error");
    }
    CATCH_MEM_ERROR(NULL_PTR_USAGE)
    {
        LOG_DEBUG("[MEMORY CONTROLLER]: Null pointer usage");
    }
    CATCH_MEM_ERROR(WRITE_ERROR)
    {
        LOG_DEBUG("[MEMORY CONTROLLER]: Write error");
    }
    CATCH_MEM_ERROR(MEMORY_ERROR)
    {
        LOG_DEBUG("[MEMORY CONTROLLER]: Generic Memory error");
    }
    CATCH_MEM_ERROR(ALLOC_ERROR)
    {
        LOG_DEBUG("[MEMORY CONTROLLER]: Allocation error");
    }
    CATCH_MEM_ERROR(QUEUE_IS_FULL)
    {
        LOG_DEBUG("[MEMORY CONTROLLER]: Queue was full");
    }
    CATCH_MEM_ERROR(FREE_ERROR)
    {
        LOG_DEBUG("[MEMORY CONTROLLER]: Free error");
    }
}

void Memory_Controller_Core::init(uint64_t size)
{
    _mem_ptr = init_mem(size);
#ifdef DEBUG
            LOG_INFO("[MEMORY CONTROLLER]: Mem_ptr address: "+std::to_string((uint64_t)_mem_ptr));
#endif
    for(int i = 0; i < QUEUE_SLOTS; i++) {
        queue_status_bitarray[i] = 0;
    }
}

void Memory_Controller_Core::start() {
    if(running) {
        return;
    }
    running = true;
#ifdef DEBUG
    LOG_DEBUG("[MEMORY CONTROLLER]: Starting thread");
#endif

    pthread_create(&thread, NULL, thread_entry, this);
}

void Memory_Controller_Core::stop() {
    if(!running) {
        // thread alrdy exited and stopped by its own
        return;
    }
#ifdef DEBUG
    LOG_DEBUG("[MEMORY CONTROLLER]: Stopping thread");
#endif
    running = false;
#ifdef DEBUG
    LOG_DEBUG("[MEMORY CONTROLLER]: freeing memory");
#endif
    free_mem(_mem_ptr);
    _mem_ptr = (uint8_t*)(0);
}

// this functions will run in a separate thread
void Memory_Controller_Core::loop() {
    ready = true;
    queue_item* in;
    while(running) {
        
        if(!get_from_input_queue(in)) {
            continue;
        }
#ifdef CONTROLLER_DEBUG
        std::cerr << "working on item: " << in->address << " data: " << in->data << " operation: " << in->op << " slot: " << in->slot << std::endl;
         LOG_DEBUG("working on item: address: "+std::to_string(in->address)+" data: "+std::to_string(in->data)+" operation: "+std::to_string(in->op)+" slot: "+std::to_string(in->slot));
            last_op_index = in->slot;
            last_write_data = in->data;
            last_read_addr = in->address;
            last_read_result = 0;
            last_operation = in->op;

#endif         
        switch(in->op) {
                case memory_ops::READ:  {
#ifdef CONTROLLER_DEBUG
                    std::cerr << "READ initiated " << std::endl;
#endif
#ifdef DEBUG
            LOG_DEBUG("[MEMORY CONTROLLER]: extraced item from queue slot: "+std::to_string(in->slot)+" -> Read operation");
#endif  
                    uint64_t out = get_item(_mem_ptr, in->address, in->size);
#ifdef CONTROLLER_DEBUG
                    last_read_result = out;
#endif
                    add_to_output_queue(out, in->slot);
                    break;
                }
                case memory_ops::WRITE: {
#ifdef CONTROLLER_DEBUG
                    std::cerr << "WRITE initiated " << std::endl;
#endif
#ifdef DEBUG
            LOG_DEBUG("[MEMORY CONTROLLER]: extraced item from queue slot: "+std::to_string(in->slot)+" -> Write operation");
#endif  
                    set_item(_mem_ptr, in->address, in->data, in->size);
                    // here we dont need to add anything to the output queue
                    // but we need to reset the queue status
                    queue_status_bitarray[in->slot].store(0, std::memory_order_release);
                    break;
                }
        }
        CATCH_ALL_MULTIPLE_ERROR(ALL_CRITICAL_ERRORS|ALL_MEMORY_ERRORS) {
#ifdef DEBUG
            LOG_DEBUG("[MEMORY CONTROLLER]: Stopping Controller");
            LOG_DEBUG("[MEMORY CONTROLLER]: error register state: "+std::to_string(error_reg));
#endif
            return;
        }
    }
}



void* thread_entry(void* arg) {
    Memory_Controller_Core* core = (Memory_Controller_Core*)arg;
#ifdef DEBUG
    LOG_DEBUG("[MEMORY CONTROLLER]: thread started");
#endif
    core->loop();
    CATCH_ALL_MULTIPLE_ERROR(ALL_CRITICAL_ERRORS|ALL_MEMORY_ERRORS) {
#ifdef DEBUG
        core->debug_errors();
#endif
        core->stop();
    }
#ifdef DEBUG
    LOG_DEBUG("[MEMORY CONTROLLER]: thread exited");
#endif
    return NULL;
}

int Memory_Controller_Core::add_to_input_queue(queue_item in) {
    for(uint64_t i = 0; i < QUEUE_SLOTS; i++) {
        uint8_t expected = 0;
        if (queue_status_bitarray[i].compare_exchange_strong(expected, 1)) {
            in.slot = i;
            queue[i] = in;
            queue_status_bitarray[i].store(2, std::memory_order_release);
            if(!reqs.producer_push(&queue[i])) {
                SET_MEM_ERROR(QUEUE_IS_FULL);
                queue_status_bitarray[i].store(0, std::memory_order_release);
                return -1;    
            }
#ifdef CONTROLLER_DEBUG
            std::cerr << "Added item: slot: " << in.slot << std::endl;
#endif
#ifdef DEBUG
            LOG_DEBUG("[PRODUCER]: added to input queue at index: "+std::to_string(i));
#endif      
            return i;
        }
    }
#ifdef DEBUG
    LOG_DEBUG("[MEMORY CONTROLLER]: all slots are full");
#endif
    SET_MEM_ERROR(QUEUE_IS_FULL);
    // BIG F if we get here
    return -1;
}

void Memory_Controller_Core::add_to_output_queue(uint64_t out, uint64_t index) {
#ifdef CONTROLLER_DEBUG
    std::cerr << "Resolving request: slot: " << index << " data: " << out << std::endl;
#endif
    queue[index].data = out;
#ifdef CONTROLLER_DEBUG
    std::cout << "PRE STATE ADD TO OUTPUT: " << std::endl;
    debug_queue_bits(index);
#endif
    queue_status_bitarray[index].store(3, std::memory_order_release);
#ifdef CONTROLLER_DEBUG
    std::cout << "AFTER STATE ADD TO OUTPUT: " << std::endl;
    debug_queue_bits(index);
#endif
}

bool Memory_Controller_Core::get_from_input_queue(queue_item*& in) {
    queue_item* ptr = nullptr;
    if(!reqs.consumer_pop(&ptr)) {
        return false;
    }
    
    /*
    for(uint64_t i = 0; i < QUEUE_SLOTS; i++) {
        // we dont need atomic checking here the controller is allowed to step over since its single threaded
        if(queue_status_bitarray[i].load() == 2) {
            // valid input
            *in = queue[i];
            return i;
        }
    }
    */
   in = ptr;
#ifdef DEBUG
        LOG_DEBUG("Item: address: "+std::to_string(in->address)+" data: "+std::to_string(in->data)+" operation: "+std::to_string(in->op)+" slot: "+std::to_string(in->slot));
#endif
    return true;
}

uint64_t Memory_Controller_Core::get_from_output_queue(uint64_t index) {

        CATCH_ALL_MULTIPLE_ERROR(ALL_CRITICAL_ERRORS|ALL_MEMORY_ERRORS){
#ifdef DEBUG
            LOG_DEBUG("[PRODUCER]: no need to wait memory controller had error");
#endif
            return 0;
        }
    while(queue_status_bitarray[index].load(std::memory_order_acquire) != 3) {
        CATCH_ALL_MULTIPLE_ERROR(ALL_CRITICAL_ERRORS|ALL_MEMORY_ERRORS){
#ifdef DEBUG
            LOG_DEBUG("[PRODUCER]: leaving waitloop -> Error occured");
#endif
            return 0;
        }
    }
    uint64_t out = queue[index].data;
#ifdef CONTROLLER_DEBUG
    std::cout << "PRE STATE GET FROM OUTPUT: " << std::endl;
    debug_queue_bits(index);
#endif
    queue_status_bitarray[index].store(0, std::memory_order_release);
#ifdef CONTROLLER_DEBUG
    std::cout << "AFTER STATE GET FROM OUTPUT: " << std::endl;
    debug_queue_bits(index);
#endif
    return out;
}

void Memory_Controller_Core::wait_for_controller_to_start() {
    while(!ready) {
        usleep(10);
    }
}

// Engine Functions:
// Initialize memory with mmap
uint8_t* Memory_Controller_Core::init_mem(uint64_t size) {
    void* mem = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if(mem == MAP_FAILED) {
        // set the error register
        SET_MULTIPLE_ERROR(ALLOC_ERROR+FAST_EXIT);
        return 0;
    }
    _max_address = (uint64_t)mem+size;
    _min_address = (uint64_t)mem;
    _size = size;
    return (uint8_t*)mem;
}

void Memory_Controller_Core::free_mem(uint8_t* mem) {
    // this part is critical, beacuse the user needs to make sure that the memory is valid
    if(mem == 0) {
        // set the error register
        SET_MULTIPLE_ERROR(FREE_ERROR|FAST_EXIT);
        return;
    }
    // check if the memory is valid
    // unmap the memory
    if(munmap((void*)(mem), _size) == -1) {
        SET_MULTIPLE_ERROR(FREE_ERROR+FAST_EXIT);
        return;
    }
}

// make sure to give a correct memory pointer in the function!!!
uint64_t Memory_Controller_Core::get_item(uint8_t* mem, uint64_t in_address, uint16_t size) {
    // extract the max and min address from the memory
    uint64_t data = 0;
    uint64_t address = in_address+(uint64_t)_min_address; // add the pointer address -> begin of the data area to the in_address so we get the real address
#ifdef DEBUG
    LOG_DEBUG("[MEMORY CONTROLLER]: Reading on address: "+std::to_string(address)+" data: "+std::to_string(data)+" size: "+std::to_string(size));
    LOG_DEBUG("[MEMORY CONTROLLER]: min_address: "+std::to_string(_min_address)+" max adress: "+std::to_string(_max_address));
    LOG_DEBUG("[MEMORY CONTROLLER]: ptr_address: "+std::to_string((uint64_t)_mem_ptr)+" in_address: "+std::to_string(in_address));
#endif
        // address is in range
#ifdef IS_BIG_ENDIAN
        // BIG ENDIAN READ
        // here we will read byte after byte until we reach the given size
        for(int i = 0; i < size; i++) {
            data |= (*(uint8_t*)(address + size - 1 - i) << (i * 8));
        }
        return data;
#endif
#ifdef IS_LITTLE_ENDIAN
        // LITTLE ENDIAN READ
        // here we will read byte after byte until we reach the given size
        for (int i = 0; i < size; i++) {
            data |= ((uint64_t)(*(uint8_t*)(address + i)) << (i * 8));
        }
#ifdef CONTROLLER_DEBUG
        std::cout << "Reading on address:" << address << " data read: " << data << " size:" << size << std::endl;          
#endif
        
        return data;
#endif

    return 0;
}

void Memory_Controller_Core::set_item(uint8_t* mem, uint64_t in_address, uint64_t data, uint16_t size) {
    uint64_t address = in_address+(uint64_t)_mem_ptr; // add the pointer address -> begin of the data area to the in_address so we get the real address
#ifdef DEBUG
    LOG_DEBUG("[MEMORY CONTROLLER]: Writing on address: "+std::to_string(address)+" data: "+std::to_string(data)+" size: "+std::to_string(size));
    LOG_DEBUG("[MEMORY CONTROLLER]: min_address: "+std::to_string(_min_address)+" max adress: "+std::to_string(_max_address));
    LOG_DEBUG("[MEMORY CONTROLLER]: ptr_address: "+std::to_string((uint64_t)_mem_ptr)+" in_address: "+std::to_string(in_address));
#endif
#ifdef IS_BIG_ENDIAN
        // BIG ENDIAN WRITE
        // here we will write byte after byte until we reach the given size
        for(int i = 0; i < size; i++) {
            *(uint8_t*)(address +size -1 - i) = (data >> (i * 8)) & 0xFF;
        }
        return;
#endif
#ifdef IS_LITTLE_ENDIAN

        // LITTLE ENDIAN WRITE
        // here we will write byte after byte until we reach the given size
        for(int i = 0; i < size; i++) {
            *(uint8_t*)(address + i) = (data >> (i * 8)) & 0xFF;
        }
        return;
#endif
}
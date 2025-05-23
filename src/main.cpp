#include "MemoryControllerHandler.hpp"
#include "Logger.hpp"
#include <iostream>
#include <chrono>
int main() {
#ifdef PERFORMANCE_TEST
    // This is the performance example:
    MemoryControllerHandler handler;
    Memory_Controller_Core* mem_controller = new Memory_Controller_Core();
    mem_controller->init(ONE_GB);
    mem_controller->start();
    // the additional controller is not used its only to showcase the usage:
    handler.add_controller(mem_controller);
    mem_controller = new Memory_Controller_Core();
    mem_controller->init(ONE_GB);
    mem_controller->start();
    handler.add_controller(mem_controller);
    
    
   
    const int64_t ops = 100000000; 
    queue_item in;
    uint64_t val = 0;

    auto start = std::chrono::high_resolution_clock::now();

    for (int64_t i = 0; i < ops; ++i) {
        // Write into Memory:
        in.op = memory_ops::WRITE;
        in.address = (i % (ONE_GB - 8));
        in.data = i;
        in.size = 8;
        handler.add_to_queue(in);
        // Read from Memory:
        in.data = 0;
        in.op = memory_ops::READ;
        in.address = (i % (ONE_GB - 8));
        in.size = 8;
        handler.add_to_queue(in);

        CATCH_ALL_MULTIPLE_ERROR(ALL_MEMORY_ERRORS|ALL_CRITICAL_ERRORS) {
            std::cerr << "[MAIN]: errors occured exiting now" << std::endl;
            std::cerr << "Item requested: address: " << in.address << " data: " << in.data << " op: " << in.op << " size: " << in.size << std::endl;
#ifdef CONTROLLER_DEBUG
            handler.debug_controller_state();
#endif
            handler.stop_controllers();
            return 1;
        }
        if (in.data != i) {
            std::cerr << "Data mismatch at op " << i << ": expected " << i << ", got " << in.data << " address: "<< in.address << std::endl;
            break;
        }

        if (i % 10000000 == 0) {
            std::cout << "Progress: " << i << " ops" << std::endl;
        }
        CATCH_ALL_ERROR {
            std::cerr << "Error detected at op " << i << std::endl;
            break;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Ops: " << ops << std::endl;
    std::cout << "Done. Duration: " << std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << "s"  << ", " << duration_ms << " ms"<< std::endl;
    handler.stop_controllers();
#else
    // Example 1:
    // This is the Showcase example:
    SET_LOG_LEVEL(LogLevel::LOG_DEBUG);
    MemoryControllerHandler handler;
    Memory_Controller_Core* mem_controller = new Memory_Controller_Core;
    mem_controller->init(ONE_GB);
    mem_controller->start();
    handler.add_controller(mem_controller);
    CLEAR_ALL_ERROR; // clear the error register
    // Simulate some operations:
    queue_item in1;
    queue_item out1;
    queue_item in2;
    queue_item out2;
    // store some data in memory:
    LOG_INFO("Storing data in memory");
    in1.op = memory_ops::WRITE;
    in1.address = 0x0;
    in1.data = 0xC0FFEE;
    in1.size = 3; // size in bytes
    handler.add_to_queue(in1);
    in2.op = memory_ops::WRITE;
    in2.address += in1.size;
    in2.data = 0xDEADBEEF;
    in2.size = 4; // size in bytes
    handler.add_to_queue(in2);
    CATCH_ALL_ERROR {
        LOG_INFO("Error occured, exit the program now");
        return 1;
    }
    // read some data from memory:
    LOG_INFO("Retrieving data from memory");
    out1.op = memory_ops::READ;
    out1.address = 0x0;
    out1.size = 3; 
    handler.add_to_queue(out1);
    out2.op = memory_ops::READ;
    out2.address += out1.size;
    out2.size = 4; 
    handler.add_to_queue(out2);
    CATCH_ALL_ERROR {
        LOG_INFO("Error occured, exit the program now");
        return 1;
    }
    CATCH_ALL_ERROR {
        LOG_INFO("Error occured, exit the program now");
        return 1;
    }
    std::cout << "Data read from memory: " << std::hex << out1.data << ", " << out2.data << std::endl;
    // stop will cancel all threads exit them out and free memory
    handler.stop_controllers();
    usleep(100); //simply wait for the threads to exit
#endif

    return 0;
}

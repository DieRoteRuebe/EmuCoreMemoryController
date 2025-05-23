#include <vector>
#include "MemControllerAPI.hpp"
#include "Error_Reg.hpp"



class MemoryControllerHandler {
    public:
        MemoryControllerHandler() = default;

        ~MemoryControllerHandler() {
            stop_controllers();
            for(Memory_Controller_Core* con : controllers) {
                if(con != nullptr) {
                    delete con;
                    con = nullptr;
                }
            }
            controllers.clear();
        }

        void add_to_queue(queue_item& in) {
            for(Memory_Controller_Core* con : controllers) {
                if(con->_max_address>in.address && con->_min_address > in.address) {
                    int index = con->add_to_input_queue(in);
                    if(index == -1) {
                        switch(in.op) {
                            case READ: {
                                SET_MEM_ERROR(READ_ERROR);
                                return;
                                break;
                            }
                            case WRITE: {
                                SET_MEM_ERROR(WRITE_ERROR);
                                return;
                                break;
                            }
                        }
                    }

                    if(in.op == READ) {
                        in.data = con->get_from_output_queue(index);
#ifdef CONTROLLER_DEBUG
                        std::cout << "WRITE: slot=" << index << " addr=" << in.address << " data=" << in.data << std::endl;
                        debug_controller_state();
#endif
                    } else {
#ifdef CONTROLLER_DEBUG
                        std::cout << "READ: slot=" << index << " addr=" << in.address << std::endl;
                        debug_controller_state();
#endif                    
                    }
                    return;
                }
            }
            // if we land here we have a boundary error:
            // this might be changed later
            SET_MULTIPLE_ERROR(FAST_EXIT|BOUNDARY_ERROR);
            stop_controllers();

        };

        void stop_controllers() {
            for(Memory_Controller_Core* con : controllers) {
                con->stop();
            }
        }

        bool add_controller(Memory_Controller_Core* controller) {
            if(controller != nullptr) {
                controllers.push_back(controller);
                return true;
            }
            SET_STANDARD_ERROR(UNDEFINED_ERROR);
            return false;
        }
#ifdef CONTROLLER_DEBUG
        void debug_controller_state() {
            int count = 1;
            for(Memory_Controller_Core* con : controllers) {
                std::cerr << "CONTROLLER: " << count << std::endl; 
                std::cerr << "Controller last_slot: " << con->last_op_index << std::endl;
                std::cerr << "Controller last_write_data: " << con->last_write_data << std::endl;
                std::cerr << "Controller last_read_addr: " << con->last_read_addr << std::endl;
                std::cerr << "Controller last_read_result: " << con->last_read_result << std::endl;
                std::cerr << "Controller last_operation: " << con->last_operation << std::endl;
                con->reqs.debug_state();
                con->debug_queue_bits();
                count++;
            }
        }
#endif
    private:

        std::vector<Memory_Controller_Core*> controllers;
};
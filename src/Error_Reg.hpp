#ifndef ERROR_REG_HPP
#define ERROR_REG_HPP
#include <cstdint>
extern uint64_t error_reg;

// Register visualization
// MSB TO LSB
// CRITICAL ERRORS | UNDEFINED | UNDEFINED | UNDEFINED | UNDEFINED | MEMORY ERRORS | EXECUTION ERRORS | STANDARD ERRORS |  

#define ALL_STANDARD_ERRORS 0xFF
#define ALL_EXECUTION_ERRORS 0xFF00
#define ALL_MEMORY_ERRORS 0xFF0000
#define ALL_CRITICAL_ERRORS 0xFF00000000000000

// Makros to extract the error codes from the error register:
#define GET_STANDARD_ERROR(x) (x & 0xFF)
#define GET_EXEC_ERROR(x) (x & 0xFF00)
#define GET_MEM_ERROR(x) (x & 0xFF0000)
#define GET_ALL_ERROR(x) (x & 0xFFFFFFFFFFFFFFFF)
#define GET_CRITICAL_ERROR(x) (x & 0xFF00000000000000)
// Makros to set the error codes in the error register:
#define SET_STANDARD_ERROR(x) (error_reg |= (x))
#define SET_EXEC_ERROR(x) (error_reg |= (x))
#define SET_MEM_ERROR(x) (error_reg |= (x))
#define SET_CRITICAL_ERROR(x) (error_reg |= (x))
#define SET_MULTIPLE_ERROR(x) (error_reg |= (x)) 
// Makros to clear the error codes in the error register:
#define CLEAR_STANDARD_ERROR (error_reg &= 0xFFFFFFFFFFFFFF00)
#define CLEAR_EXEC_ERROR (error_reg &= 0xFFFFFFFFFFFF00FF)
#define CLEAR_MEM_ERROR (error_reg &=  0xFFFFFFFFFF00FFFF)
#define CLEAR_ALL_ERROR (error_reg = 0)
// Makros to for error catch:
#define CATCH_STANDARD_ERROR(type) if(GET_STANDARD_ERROR(error_reg) & (type))
#define CATCH_EXEC_ERROR(type) if(GET_EXEC_ERROR(error_reg) & (type))
#define CATCH_MEM_ERROR(type) if(GET_MEM_ERROR(error_reg) & (type))
#define CATCH_CRITICAL_ERROR(type) if(GET_CRITICAL_ERROR(error_reg) & (type))
#define CATCH_ALL_ERROR if(GET_ALL_ERROR(error_reg))
#define CATCH_ALL_MULTIPLE_ERROR(type) if((GET_ALL_ERROR(error_reg)) & (type))
// goes from 0x0 - 0xFF
enum standard_error : uint64_t{
    NO_ERROR = 0x0,
    UNDEFINED_ERROR = 1ULL << 1,
};

// goes from 0x100 - 0xFF00
enum exec_error : uint64_t {
    EXECUTION_ERROR = 1ULL << 8, // Generic error
    OPERAND_ERROR = 1ULL << 9,
    INSTRUCTION_ERROR = 1ULL << 10,
    
};


// goes from 0x10000 - 0xFF0000
enum error_mem : uint64_t {
    MEMORY_ERROR    = 1ULL << 16, // 0x00010000
    READ_ERROR      = 1ULL << 17, // 0x00020000
    WRITE_ERROR     = 1ULL << 18, // 0x00040000
    ALLOC_ERROR     = 1ULL << 19, // 0x00080000
    FREE_ERROR      = 1ULL << 20, // 0x00100000
    BOUNDARY_ERROR  = 1ULL << 21, // 0x00200000
    NULL_PTR_USAGE  = 1ULL << 22, // 0x00400000
    QUEUE_IS_FULL   = 1ULL << 23, // 0x00800000
};

enum error_critical : uint64_t {
    FAST_EXIT = 1ULL << 56, // Fast exit -> is used when a critical error occurs -> controlled crash
    UNDEFINED_CRITICAL_ERROR = 1ULL << 57,
};


#endif // ERROR_REG_HPP
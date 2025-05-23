#ifndef DEFINES_EMULATOR_HPP
#define DEFINES_EMULATOR_HPP

#define PERFORMANCE_TEST

// Use these defines for controlled debugging:
// overall and simple debugging only used for light debugging
// The controllerthread wont be able to print debug messages when working unter heavy workload
#ifndef PERFORMANCE_TEST
    #define DEBUG
    //#define CONTROLLER_DEBUG
#endif
// Use this define for Thread information and controller states over stderr, stdout
// Can result in mixed messages due to threading but prints atleast all the information you need





// make sure to define the endianness of the system
// this is only for the emulator the compiler does this automatically
// use either: IS_BIG_ENDIAN or IS_LITTLE_ENDIAN
#define IS_LITTLE_ENDIAN
//#define IS_BIG_ENDIAN


enum memory_ops {
    NONE = 0,
    READ = 1,
    WRITE = 2,
};

struct queue_item {
    memory_ops op = memory_ops::NONE;
    uint64_t address = 0;
    uint64_t data = 0;
    uint64_t size = 0;
    uint64_t slot = 0;
};


#endif // DEFINES_EMULATOR_HPP

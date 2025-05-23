# EmuCore Memory Controller

This project implements a high-performance, lock-free memory controller for emulation purposes, designed with future portability to hardware (e.g., FPGA, ULP core) in mind. The controller uses atomic operations, a status-byte-based queue system, and a dedicated RingBuffer for efficient communication between producers and the memory controller thread.

It is recommended to use the `MemoryControllerHandler` for all memory operations, as boundary checks and controller routing are handled there.

---

## Architecture Overview

- **MemControllerAPI**  
  The main class that manages memory and provides a lock-free input/output queue. Each controller runs in its own thread.

- **Queue System**  
  The controller uses a fixed-size queue (`queue_item queue[QUEUE_SLOTS]`) and a status bit array (`std::atomic<uint8_t> queue_status_bitarray[QUEUE_SLOTS]`).  
  Each slot's status byte encodes its state (free, reserved, ready, output ready).

- **RingBuffer**  
  The new `RingBuffer` class (see `RingBuffer_QueueItems.hpp`) is used for lock-free, single-producer/single-consumer communication.  
  Instead of copying queue items, the RingBuffer stores pointers to slots in the main queue, minimizing memory overhead and maximizing throughput.

- **Communication**  
  Producers (e.g., a CPU emulator) and the memory controller communicate via the RingBuffer and status bits. Synchronization is achieved using atomic operations.

---

## Slot Status Byte

Each slot in the queue uses a status byte to represent its state:

- `0`: Slot is free
- `1`: Slot reserved (producer is writing)
- `2`: Slot ready for controller to process
- `3`: Output/result written, ready for producer to read

---

## Typical Workflow

1. **Producer**
    - Searches for a slot with status `0` (free), atomically reserves it (`1`), writes the request, sets status to `2`, and pushes a pointer to the slot into the RingBuffer.
2. **Memory Controller**
    - Pops a pointer from the RingBuffer, processes the request (status `2`), writes the result, and sets status to `3` (for reads) or `0` (for writes).
3. **Producer (reading result)**
    - Waits for status `3`, reads the result, and resets the slot to `0` (free).

---

## Debugging: `CONTROLLER_DEBUG`

Define `CONTROLLER_DEBUG` in `global_defines.hpp` to enable detailed debug output for the controller and the RingBuffer.  
With this define enabled, the controller prints:

- Every operation it processes (address, data, operation, slot)
- The state of the RingBuffer (head, tail, slot contents)
- The status bits of all queue slots

This is extremely useful for diagnosing synchronization issues, slot reuse, and queue overflows.

**Example:**
```cpp
//#define CONTROLLER_DEBUG
```
Uncomment this line in `global_defines.hpp` to enable controller debug output.

---

## Example Usage

```cpp
MemoryControllerHandler handler;

// Create and add controllers
auto* controller1 = new Memory_Controller_Core();
controller1->init(ONE_GB);
controller1->start();
handler.add_controller(controller1);

// Prepare a memory operation
queue_item op;
op.op = memory_ops::WRITE;
op.address = 0x1000;
op.data = 0xDEADBEEF;
op.size = 8;

// Route the operation to the correct controller
handler.add_to_queue(op);

// For READ operations, the result will be in op.data after the call
op.op = memory_ops::READ;
handler.add_to_queue(op);
std::cout << "Read result: " << op.data << std::endl;
```

---

## Performance

In current tests, the memory controller achieves up to **3.2–3.4 million operations per second** with a single-threaded producer and single-threaded consumer.  
The new RingBuffer design minimizes memory copying and synchronization overhead, making the controller suitable for high-throughput emulation and future hardware implementation.
```sh
~/DEV/MemoryController/src/build$ ./mem_controller 
Progress: 0 ops
Progress: 10000000 ops
Progress: 20000000 ops
Progress: 30000000 ops
Progress: 40000000 ops
Progress: 50000000 ops
Progress: 60000000 ops
Progress: 70000000 ops
Progress: 80000000 ops
Progress: 90000000 ops
Ops: 100000000
Done. Duration: 31s, 31149 ms

Progress: 0 ops
Progress: 10000000 ops
Progress: 20000000 ops
Progress: 30000000 ops
Progress: 40000000 ops
Progress: 50000000 ops
Progress: 60000000 ops
Progress: 70000000 ops
Progress: 80000000 ops
Progress: 90000000 ops
Ops: 100000000
Done. Duration: 28s, 28913 ms
```
---

## Building

To build the project, run:

```sh
cd src
mkdir build
cd build
cmake ..
make
```

For performance builds, set `CMAKE_BUILD_TYPE` to `Release` and comment out `#define DEBUG` in `global_defines.hpp`.

---

## Notes

- The controller is designed for high throughput and minimal locking.
- The status byte approach is extensible (e.g., for error flags or additional states).
- The RingBuffer and debug features (`CONTROLLER_DEBUG`) make it easy to analyze and tune performance or debug concurrency issues.
- The code is portable and can be adapted for hardware state machines or ULP cores.
- Tests with valgrind are still missing.
---

If you plan to extend the system with multi-producer/multi-consumer support, advanced scheduling, or priority queues, feel free to do so.  
If you want a concept for advanced scheduling or dependency tracking, just ask! I might have an idea.



## Further thoughts

- With a little bit of work it is possible to use the 4th bit in the statusbits as a spinlock bit
- The other 4 bits of the statusbits can be used to encode the slot in the queue
- The Ringbuffer could be change to uint8_t where the encoded id/slot-index can be safed so no need for pointers
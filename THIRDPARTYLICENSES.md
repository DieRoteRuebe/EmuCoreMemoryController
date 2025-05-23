# Third Party Licenses

This project, **EmuCore Memory Controller**, uses only the following third-party components:

---

## 1. C++ Standard Library

- **Description:**  
  The project uses the C++ Standard Library (STL) for containers, threading, atomics, and utility functions.
- **License:**  
  [ISO/IEC 14882:2017](https://isocpp.org/std/the-standard) (Standard C++), implementation-specific license (e.g., GNU GPL for libstdc++, Apache 2.0 for libc++).

---

## 2. POSIX / System Libraries

- **pthread**  
  Used for threading and synchronization.
  - **License:**  
    [POSIX.1-2008](https://pubs.opengroup.org/onlinepubs/9699919799/) (system library, license depends on your OS, e.g., LGPL for glibc on Linux)

- **sys/mman.h**  
  Used for memory mapping (mmap/munmap).
  - **License:**  
    System library, license depends on your OS (e.g., LGPL for glibc on Linux)

- **unistd.h**  
  Used for sleep/usleep and other POSIX utilities.
  - **License:**  
    System library, license depends on your OS

---

## 3. No External Open Source Libraries

As of this release, this project does **not** use any third-party open source libraries outside of the C++ Standard Library and POSIX system headers.

---

## 4. Logging

The logging system is custom and does **not** use any external logging frameworks.

---

## 5. Future Additions

If you add external dependencies (e.g., Boost, fmt, GoogleTest, etc.), please update this file and include the relevant license texts or links.

---

**If you distribute this project, ensure you comply with the licenses of your compiler and system libraries.**

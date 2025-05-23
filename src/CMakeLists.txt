cmake_minimum_required(VERSION 3.10)

set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -march=native -DNDEBUG")
#set(CMAKE_BUILD_TYPE Release)
# Projektname und Sprache festlegen
project(MemoryController LANGUAGES CXX)

# C++ Standard festlegen
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Alle .cpp Dateien im aktuellen Verzeichnis und Unterverzeichnissen finden
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
)

# Dateien aus dem Build-Verzeichnis ausschließen
list(FILTER SOURCES EXCLUDE REGEX "${CMAKE_BINARY_DIR}/.*")

# Executable für den Compiler erstellen
add_executable(mem_controller ${SOURCES} )

# Include-Verzeichnisse hinzufügen
target_include_directories(mem_controller PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/**
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../**
    ${CMAKE_CURRENT_SOURCE_DIR}/src/**
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
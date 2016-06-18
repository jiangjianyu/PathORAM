cmake_minimum_required(VERSION 3.0)
project(PathORAM)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++11")

set(SOURCE_FILES src/OramCrypto.cpp src/OramBucket.cpp src/OramSocket.cpp src/OramNodeStorage.cpp
        src/OramNode.cpp src/OramStash.cpp src/OramAccessController.cpp src/OramAccessController.h src/PathORAM.h src/main.cpp src/performance.cpp src/performance.h)
add_executable(PathORAM ${SOURCE_FILES})
target_link_libraries(PathORAM sodium)

cmake_minimum_required(VERSION 3.5)
project(PathORAM)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES src/OramCrypto.cpp src/OramBucket.cpp src/OramSocket.cpp src/OramNodeStorage.cpp
        src/OramNode.cpp src/OramStash.cpp src/OramAccessController.cpp src/OramAccessController.h src/PathORAM.h src/main.cpp)
add_executable(PathORAM ${SOURCE_FILES})
target_link_libraries(PathORAM sodium)
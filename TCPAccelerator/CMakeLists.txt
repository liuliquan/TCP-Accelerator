cmake_minimum_required(VERSION 3.8)
project(TCPAccelerator)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

set(SOURCE_FILES main.c headerFiles/constants.h headerFiles/deletedIdBST.h headerFiles/deletedIdMinHeap.h headerFiles/headers.h headerFiles/LanDataQueue.h headerFiles/LanOrderQueue.h headerFiles/ReadAndBuffer.h headerFiles/types.h headerFiles/writeInLogger.h lib/deletedIdBST.c lib/LanDataQueue.c lib/LanOrderQueue.c lib/writeInLogger.c headerFiles/SHAHashing.h lib/SHAHashing.c)
add_executable(TCPAccelerator ${SOURCE_FILES})
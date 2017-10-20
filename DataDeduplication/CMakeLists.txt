cmake_minimum_required(VERSION 3.8)

project(DataDeduplication)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

set(SOURCE_FILES main.c headerFiles/constants.h headerFiles/deletedIdMinHeap.h headerFiles/headers.h headerFiles/LanDataQueue.h headerFiles/LanOrderQueue.h headerFiles/ReadAndBuffer.h headerFiles/writeInLogger.h lib/LanDataQueue.c lib/LanOrderQueue.c lib/writeInLogger.c headerFiles/deletedIdBST.h lib/deletedIdBST.c headerFiles/SHAHashing.h lib/SHAHashing.c headerFiles/types.h)
add_executable(DataDeduplication ${SOURCE_FILES})
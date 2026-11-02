cmake_minimum_required(VERSION 3.28)
project(chatserver-multi-threaded LANGUAGES CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/common.cmake)

add_executable(chatserver-multi-threaded
    src/chatserver.cxx
    src/blocking_queue.hxx
    src/clients.hxx
)
target_link_libraries(chatserver-multi-threaded PRIVATE asio::common)

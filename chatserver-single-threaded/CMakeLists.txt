cmake_minimum_required(VERSION 3.28)
project(chatserver-single-threaded LANGUAGES CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/common.cmake)

add_executable(chatserver-single-threaded src/chatserver.cxx)
target_link_libraries(chatserver-single-threaded PRIVATE asio::common)

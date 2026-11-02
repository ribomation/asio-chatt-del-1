cmake_minimum_required(VERSION 3.28)
project(daytime LANGUAGES CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/common.cmake)

add_executable(daytime-server server/daytime-server.cxx)
target_link_libraries(daytime-server PRIVATE asio::common)

add_executable(daytime-client client/daytime-client.cxx)
target_link_libraries(daytime-client PRIVATE asio::common)

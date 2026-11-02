cmake_minimum_required(VERSION 3.28)
project(asio-chatt-del-1 LANGUAGES CXX)

# Demoprogram till artikeln
#   "Bygg en chattserver med Boost.Asio, del 1: Grunderna och synkron I/O"
# Bygg allt härifrån, eller ett program i taget från dess egen mapp.

include(cmake/common.cmake)

add_subdirectory(echoserver)

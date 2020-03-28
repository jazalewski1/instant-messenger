cmake_minimum_required(VERSION 3.10)

project(Messenger LANGUAGES CXX)

set(CXX_STANDARD 14)

add_subdirectory(server)

add_subdirectory(client)
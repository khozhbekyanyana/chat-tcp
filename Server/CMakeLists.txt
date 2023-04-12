cmake_minimum_required(VERSION 3.0)
project(chat)

add_executable(_server Server.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(_server PRIVATE Threads::Threads)


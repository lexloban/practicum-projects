cmake_minimum_required(VERSION 3.23)
project(untitled1)

set(CMAKE_CXX_STANDARD 20)

add_executable(untitled1 main.cpp log_duration.h document.cpp document.h read_input_functions.cpp read_input_functions.h request_queue.cpp request_queue.h search_server.cpp search_server.h string_processing.cpp string_processing.h paginator.h process_queries.h process_queries.cpp concurrent_map.h)

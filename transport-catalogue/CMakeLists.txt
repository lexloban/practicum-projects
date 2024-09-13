cmake_minimum_required(VERSION 3.23)
project(TransportCatalogue)

set(CMAKE_CXX_STANDARD 20)

add_executable(TransportCatalogue main.cpp geo.h transport_catalogue.h transport_catalogue.cpp domain.h domain.cpp geo.cpp json.h json.cpp json_reader.h json_reader.cpp map_renderer.h map_renderer.cpp request_handler.h request_handler.cpp svg.h svg.cpp json_builder.h json_builder.cpp ranges.h graph.h transport_router.h transport_router.cpp router.h log_duration.h)

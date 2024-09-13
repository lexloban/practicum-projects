cmake_minimum_required(VERSION 3.23)

project(TransportCatalogue)
set(CMAKE_CXX_STANDARD 20)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS proto/transport_catalogue.proto proto/map_renderer.proto proto/svg.proto proto/transport_router.proto proto/graph.proto)

set(CATALOGUE_FILES main.cpp geo.h transport_catalogue.h transport_catalogue.cpp domain.h domain.cpp geo.cpp json.h json.cpp json_reader.h json_reader.cpp map_renderer.h map_renderer.cpp request_handler.h request_handler.cpp svg.h svg.cpp json_builder.h json_builder.cpp ranges.h graph.h transport_router.h transport_router.cpp router.h serialization.h serialization.cpp)

add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} ${CATALOGUE_FILES})
target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)
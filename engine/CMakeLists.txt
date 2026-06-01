# minimum cmake version required to build this project
cmake_minimum_required(VERSION 3.20)

# project name and language
project(logstream CXX)

# use C++20 — required for std::atomic, std::optional, and memory order guarantees
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find required libraries installed via homebrew
find_package(Threads REQUIRED)
find_package(zstd REQUIRED)
find_package(gRPC REQUIRED)
find_package(Protobuf REQUIRED)

# ─────────────────────────────────────────────
# main engine executable
# ─────────────────────────────────────────────

add_executable(logstream_engine
    src/main.cpp
    src/ring_buffer.cpp
    proto/logstream.pb.cc
    proto/logstream.grpc.pb.cc
)

target_include_directories(logstream_engine PRIVATE
    include
    proto
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(logstream_engine PRIVATE
    Threads::Threads
    zstd::libzstd_static
    gRPC::grpc++
    protobuf::libprotobuf
)
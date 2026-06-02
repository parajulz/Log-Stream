# minimum cmake version required to build this project
cmake_minimum_required(VERSION 3.20)

# project name and language
project(logstream CXX)

# use C++20 — required for std::atomic, std::optional, and memory order guarantees
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find required libraries — works on both Mac (Homebrew) and Linux (Ubuntu)
find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)
find_package(PkgConfig REQUIRED)

# use pkg-config to find gRPC — more portable than find_package for gRPC
pkg_check_modules(GRPC REQUIRED grpc++)
pkg_check_modules(ZSTD REQUIRED libzstd)

# ─────────────────────────────────────────────
# main engine executable
# ─────────────────────────────────────────────

add_executable(logstream_engine
    src/main.cpp
    src/ring_buffer.cpp
    src/flusher.cpp
    src/index.cpp
    proto/logstream.pb.cc
    proto/logstream.grpc.pb.cc
)

target_include_directories(logstream_engine PRIVATE
    include
    proto
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GRPC_INCLUDE_DIRS}
    ${ZSTD_INCLUDE_DIRS}
)

target_link_libraries(logstream_engine PRIVATE
    Threads::Threads
    ${GRPC_LINK_LIBRARIES}
    ${ZSTD_LINK_LIBRARIES}
    protobuf::libprotobuf
    -lgrpc++
    -lgrpc
    -lzstd
)

# ─────────────────────────────────────────────
# benchmark executable — only build if benchmark library is available
# ─────────────────────────────────────────────

find_package(benchmark QUIET)

if(benchmark_FOUND)
    add_executable(logstream_bench
        bench/bench_buffer.cpp
        src/ring_buffer.cpp
    )

    target_include_directories(logstream_bench PRIVATE
        include
        ${GRPC_INCLUDE_DIRS}
    )

    target_link_libraries(logstream_bench PRIVATE
        benchmark::benchmark
        Threads::Threads
    )

    target_compile_options(logstream_bench PRIVATE
        -O3
    )
endif()
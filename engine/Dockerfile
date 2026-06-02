# start from ubuntu — has the package manager we need
FROM ubuntu:22.04

# prevents interactive prompts during apt install
ENV DEBIAN_FRONTEND=noninteractive

# install build tools and dependencies
RUN apt-get update && apt-get install -y \
    cmake \
    clang \
    libgrpc++-dev \
    libprotobuf-dev \
    protobuf-compiler \
    protobuf-compiler-grpc \
    libzstd-dev \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

# copy the engine source code into the container
WORKDIR /app
COPY . .

# generate gRPC code from proto file
RUN protoc \
    -I proto \
    --cpp_out=proto \
    --grpc_out=proto \
    --plugin=protoc-gen-grpc=/usr/bin/grpc_cpp_plugin \
    proto/logstream.proto

# build the engine
# CMAKE_PREFIX_PATH=/usr tells cmake to look for gRPC where ubuntu installs it
RUN mkdir -p build && cd build && cmake \
    -DCMAKE_PREFIX_PATH=/usr \
    .. && make

# create data directory for batch files
RUN mkdir -p build/data

# run the engine
CMD ["./build/logstream_engine"]

RUN apt-get update && apt-get install -y \
    cmake \
    clang \
    libgrpc++-dev \
    libprotobuf-dev \
    protobuf-compiler \
    protobuf-compiler-grpc \
    libzstd-dev \
    libbenchmark-dev \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*
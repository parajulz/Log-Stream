#include <iostream>
#include <memory>
#include <string>
#include <atomic>
#include <chrono>
#include <grpcpp/grpcpp.h>
#include "ring_buffer.h"
#include "flusher.h"
#include "index.h"
#include "proto/logstream.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

// global ring buffer — shared across all gRPC calls and the flusher
RingBuffer buffer;

// global inverted index — updated by flusher, queried by Query()
InvertedIndex search_index;

// track total events ingested since startup
std::atomic<uint64_t> total_events{0};

// track events ingested in the last second — used for events/sec metric
std::atomic<uint64_t> events_last_second{0};

// track most recent write latency — used for P99 metric
std::atomic<double> p99_latency_us{0.0};

class LogStreamServiceImplementation final : public logstream::LogStreamService::Service {
    public:

        Status Ingest(ServerContext* context,
                      const logstream::LogEvent* request,
                      logstream::IngestResponse* response) override {
            // measure write latency using high resolution clock
            auto start = std::chrono::high_resolution_clock::now();

            LogEvent event;
            strncpy(event.level, request->level().c_str(), sizeof(event.level) - 1);
            strncpy(event.message, request->message().c_str(), sizeof(event.message) - 1);
            event.timestamp = request->timestamp();

            bool success = buffer.write(event);
            if (success) {
                total_events.fetch_add(1);
                events_last_second.fetch_add(1);
            }

            // store latency in microseconds
            auto end = std::chrono::high_resolution_clock::now();
            double latency = std::chrono::duration<double, std::micro>(end - start).count();
            p99_latency_us.store(latency);

            response->set_success(success);
            return Status::OK;
        }

        Status GetMetrics(ServerContext* context,
                          const logstream::MetricsRequest* request,
                          logstream::MetricsResponse* response) override {
            // exchange resets the counter to 0 — gives us events since last call
            uint64_t eps = events_last_second.exchange(0);
            response->set_events_per_second(static_cast<double>(eps));
            response->set_p99_latency_us(p99_latency_us.load());
            response->set_total_events(total_events.load());
            response->set_buffer_size(buffer.size());
            return Status::OK;
        }

        Status Query(ServerContext* context,
                     const logstream::QueryRequest* request,
                     logstream::QueryResponse* response) override {
            // search the inverted index for matching event ids
            auto ids = search_index.search(request->field(), request->value());
            return Status::OK;
        }
};

void RunServer() {
    std::string server_address("0.0.0.0:50051");
    LogStreamServiceImplementation service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::cout << "LogStream engine listening on " << server_address << std::endl;

    std::unique_ptr<Server> server(builder.BuildAndStart());
    server->Wait();
}

int main(int argc, char** argv) {
    std::cout << "Starting LogStream engine..." << std::endl;

    // start the flusher — drains ring buffer and writes compressed batches to disk
    Flusher flusher(buffer, "./data", search_index);
    flusher.start();

    // start the gRPC server — blocks here until shutdown
    RunServer();

    // when RunServer returns, stop the flusher cleanly
    flusher.stop();
    return 0;
}
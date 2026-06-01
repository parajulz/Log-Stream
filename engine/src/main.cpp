#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "ring_buffer.h"
#include "proto/logstream.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

RingBuffer buffer;

// track total events ingested since startup
std::atomic<uint64_t> total_events{0};

class LogStreamServiceImplementation final : public logstream::LogStreamService::Service {

    public:

        // receives a log event from Python and writes it into the ring buffer
        Status Ingest(ServerContext* context,
                      const logstream::LogEvent* request,
                      logstream::IngestResponse* response) override {
            LogEvent event;
            strncpy(event.level, request->level().c_str(), sizeof(event.level) - 1);
            strncpy(event.message, request->message().c_str(), sizeof(event.message) - 1);
            event.timestamp = request->timestamp();

            bool success = buffer.write(event);
            if (success) total_events.fetch_add(1);
            response->set_success(success);
            return Status::OK;
        }

        // returns current performance metrics to Python
        Status GetMetrics(ServerContext* context,
                          const logstream::MetricsRequest* request,
                          logstream::MetricsResponse* response) override {
            // placeholder values for now — real benchmarking comes later
            response->set_events_per_second(0);
            response->set_p99_latency_us(0);
            response->set_total_events(total_events.load());
            response->set_buffer_size(buffer.size());
            return Status::OK;
        }

        // searches stored logs — placeholder for now
        Status Query(ServerContext* context,
                     const logstream::QueryRequest* request,
                     logstream::QueryResponse* response) override {
            // inverted index search comes later
            // for now returns empty results
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
    RunServer();
    return 0;
}
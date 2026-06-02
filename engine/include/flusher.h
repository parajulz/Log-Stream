// header for background thread that drains the ring buffer and writes events to the disk
// needs reference to the ring buffer - so it can read events out of it
// a directory path -- so it knows where on disk to write the batch files

#pragma once
#include <string>
#include <thread>
#include <atomic>
#include "ring_buffer.h"
#include "index.h"

class Flusher {
public:
    Flusher(RingBuffer& buffer, const std::string& output_dir, InvertedIndex& index);
    ~Flusher();
    void start();
    void stop();

private:
    void run();
    void flush_batch();

    RingBuffer& buffer_;
    std::string output_dir_;
    InvertedIndex& index_;
    std::thread thread_;
    std::atomic<bool> running_;
};
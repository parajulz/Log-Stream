#include "flusher.h"
#include <fstream>
#include <chrono>
#include <iostream>
#include <filesystem>
#include <vector>
#include <zstd.h>
#include <cstring>

// how many events to accumulate before flushing
// if this many events aren't available, flush anyway after FLUSH_INTERVAL_MS
constexpr size_t BATCH_SIZE = 1000;

// how often to flush even if batch isn't full — every 5 seconds
constexpr int FLUSH_INTERVAL_MS = 5000;

Flusher::Flusher(RingBuffer& buffer, const std::string& output_dir, InvertedIndex& index)
    : buffer_(buffer), output_dir_(output_dir), index_(index), running_(false) {
    // create the output directory if it doesn't exist
    std::filesystem::create_directories(output_dir_);
}

Flusher::~Flusher() {
    // make sure the thread is stopped when the flusher is destroyed
    stop();
}

void Flusher::start() {
    running_ = true;
    // launch the run() function in a separate thread
    thread_ = std::thread(&Flusher::run, this);
    std::cout << "Flusher started — writing batches to " << output_dir_ << std::endl;
}

void Flusher::stop() {
    // signal the thread to stop
    running_ = false;

    // wait for the thread to finish its current batch
    if (thread_.joinable()) {
        thread_.join();
    }

    // flush any remaining events before stopping
    flush_batch();
    std::cout << "Flusher stopped" << std::endl;
}

void Flusher::run() {
    // keep running until stop() sets running_ to false
    while (running_) {
        // sleep for FLUSH_INTERVAL_MS between flushes
        std::this_thread::sleep_for(std::chrono::milliseconds(FLUSH_INTERVAL_MS));

        // flush whatever is in the buffer
        flush_batch();
    }
}

void Flusher::flush_batch() {
    // collect up to BATCH_SIZE events from the ring buffer
    std::vector<LogEvent> batch;
    batch.reserve(BATCH_SIZE);

    while (batch.size() < BATCH_SIZE) {
        auto event = buffer_.read();
        if (!event.has_value()) {
            break;  // buffer is empty — flush what we have
        }
        batch.push_back(*event);
    }

    // nothing to flush
    if (batch.empty()) return;

    // add each event to the inverted index before flushing to disk
    static std::atomic<uint64_t> event_counter{0};
    for (const auto& event : batch) {
        index_.add(event, event_counter.fetch_add(1));
    }

    // get current timestamp for the filename
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        now.time_since_epoch()
    ).count();

    // raw batch data — treat the vector as a flat array of bytes
    const void* src = batch.data();
    size_t src_size = batch.size() * sizeof(LogEvent);

    // allocate buffer for compressed output
    size_t compressed_bound = ZSTD_compressBound(src_size);
    std::vector<char> compressed(compressed_bound);

    // compress with zstd — level 3 is a good balance of speed and compression ratio
    size_t compressed_size = ZSTD_compress(
        compressed.data(),   // destination
        compressed_bound,    // destination capacity
        src,                 // source
        src_size,            // source size
        3                    // compression level
    );

    if (ZSTD_isError(compressed_size)) {
        std::cerr << "zstd compression failed: " << ZSTD_getErrorName(compressed_size) << std::endl;
        return;
    }

    // write compressed batch to disk
    std::string filename = output_dir_ + "/batch_" + std::to_string(timestamp) + ".zst";
    std::ofstream file(filename, std::ios::binary);

    if (!file) {
        std::cerr << "failed to open file: " << filename << std::endl;
        return;
    }

    file.write(compressed.data(), compressed_size);
    file.close();

    std::cout << "flushed " << batch.size() << " events → " << filename
              << " (" << src_size << " bytes → " << compressed_size << " bytes)" << std::endl;
}
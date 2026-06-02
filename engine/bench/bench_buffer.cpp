#include <benchmark/benchmark.h>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <memory>
#include "ring_buffer.h"

// mutex ring buffer — heap allocated to avoid stack overflow
class MutexRingBuffer {
public:
    MutexRingBuffer() : buffer_(std::make_unique<std::array<LogEvent, BUFFER_SIZE>>()) {}

    bool write(const LogEvent& event) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (head_ - tail_ >= BUFFER_SIZE) return false;
        (*buffer_)[head_ % BUFFER_SIZE] = event;
        head_++;
        return true;
    }

    std::optional<LogEvent> read() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (head_ == tail_) return std::nullopt;
        LogEvent event = (*buffer_)[tail_ % BUFFER_SIZE];
        tail_++;
        return event;
    }

private:
    std::unique_ptr<std::array<LogEvent, BUFFER_SIZE>> buffer_;
    std::mutex mutex_;
    uint64_t head_{0};
    uint64_t tail_{0};
};

static LogEvent make_event() {
    LogEvent event;
    strncpy(event.level, "info", sizeof(event.level) - 1);
    strncpy(event.message, "benchmark event", sizeof(event.message) - 1);
    event.timestamp = 1717200000;
    return event;
}

static void BM_LockFree(benchmark::State& state) {
    auto buf = std::make_unique<RingBuffer>();
    LogEvent event = make_event();
    int num_threads = state.range(0);
    constexpr int EVENTS_PER_THREAD = 10000;

    for (auto _ : state) {
        std::atomic<bool> done{false};

        std::thread reader([&]() {
            while (!done.load() || buf->size() > 0) {
                buf->read();
            }
        });

        std::vector<std::thread> writers;
        for (int t = 0; t < num_threads; t++) {
            writers.emplace_back([&]() {
                for (int i = 0; i < EVENTS_PER_THREAD; i++) {
                    while (!buf->write(event)) {
                        std::this_thread::yield();
                    }
                }
            });
        }

        for (auto& t : writers) t.join();
        done = true;
        reader.join();
    }

    state.SetItemsProcessed(state.iterations() * num_threads * EVENTS_PER_THREAD);
}

static void BM_Mutex(benchmark::State& state) {
    auto buf = std::make_unique<MutexRingBuffer>();
    LogEvent event = make_event();
    int num_threads = state.range(0);
    constexpr int EVENTS_PER_THREAD = 10000;

    for (auto _ : state) {
        std::atomic<bool> done{false};

        std::thread reader([&]() {
            while (!done.load()) {
                buf->read();
            }
        });

        std::vector<std::thread> writers;
        for (int t = 0; t < num_threads; t++) {
            writers.emplace_back([&]() {
                for (int i = 0; i < EVENTS_PER_THREAD; i++) {
                    while (!buf->write(event)) {
                        std::this_thread::yield();
                    }
                }
            });
        }

        for (auto& t : writers) t.join();
        done = true;
        reader.join();
    }

    state.SetItemsProcessed(state.iterations() * num_threads * EVENTS_PER_THREAD);
}

BENCHMARK(BM_LockFree)->Arg(1)->Arg(2)->Arg(4)->Arg(8)->Arg(16)->Arg(32)->UseRealTime()->Iterations(1);
BENCHMARK(BM_Mutex)->Arg(1)->Arg(2)->Arg(4)->Arg(8)->Arg(16)->Arg(32)->UseRealTime()->Iterations(1);

BENCHMARK_MAIN();
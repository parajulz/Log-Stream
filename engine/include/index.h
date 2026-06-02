#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include "ring_buffer.h"

class InvertedIndex {
public:
    void add(const LogEvent& event, uint64_t event_id);
    std::vector<uint64_t> search(const std::string& field, const std::string& value);

private:
    std::mutex mutex_;
    std::unordered_map<std::string, std::vector<uint64_t>> index_;
    std::vector<LogEvent> events_;
};
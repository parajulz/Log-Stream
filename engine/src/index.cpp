#include "index.h"

void InvertedIndex::add(const LogEvent& event, uint64_t event_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    events_.push_back(event);
    index_["level:" + std::string(event.level)].push_back(event_id);
    index_["message:" + std::string(event.message)].push_back(event_id);
}

std::vector<uint64_t> InvertedIndex::search(const std::string& field, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string key = field + ":" + value;
    auto it = index_.find(key);
    if (it == index_.end()) return {};
    return it->second;
}
#include "ring_buffer.h"
#include <cstring>

bool RingBuffer::write(const LogEvent& event) {
    //1) check if buffer is full. if full, return false to indicate write failed.
    //2) define head and tail 
    uint64_t head = head_.load(std::memory_order_relaxed); // relaxed because we only need to read head for the compare-swap, and we don't need to synchronize with other threads at this point. relaxed allows for maximum performance while still ensuring that we get a consistent view of the head variable.
    uint64_t tail = tail_.load(std::memory_order_acquire); // acquire because we need to see the latest value of tail that was written by other threads. this ensures that we have a consistent view of the buffer and can accurately determine whether the buffer is full or not.

    // if distance between head and tail equals total capacity, buffer is full, every slot is taken. return false to indicate write failed. 
    if (head - tail >= BUFFER_SIZE) {
        return false; // buffer is full
    }

    // else if buffer is available, claim next slot atomically using compare-swap 
    uint64_t current_head = head;
    // load current head from head. in ordet to swap it. 

    // if it doesn't match, return false it means another thread got here first and incremented that slot, so we return false to indicate write failed.
    // else if head still = current_head, change it to head + 1 

    if (!head_.compare_exchange_weak(
            current_head,
            head + 1,
            std::memory_order_release, // success prder. swap succeeds, we claimed the slot. release so readers who load head with acquire will see our written event 
            std::memory_order_relaxed)) { // failure order. we're returning so no gurantees needed. relaxed because we don't need to synchronize with other threads if the compare-swap fails. this allows for maximum performance while still ensuring that we get a consistent view of the head variable.
        return false; // another thread got here first, caller retries
    }

    //buffer only has slots 0 through 65535. You can't use 
    // head directly as the index because at 65536 there is no slot 65536.
    // need to wrap around. convert the big number into a slot index that stays between 0 and 65535.
    uint64_t slot = head & (BUFFER_SIZE - 1);

    //copy log event into the slot we just claimed 
    buffer_[slot] = event;
    return true;
}

std::optional<LogEvent> RingBuffer::read() {
    uint64_t tail = tail_.load(std::memory_order_relaxed); // relaxed bc we only need to read tail for the compare-swap, and we don't need to synchronize with other threads at this point. relaxed allows for maximum performance while still ensuring that we get a consistent view of the tail variable.
    uint64_t head = head_.load(std::memory_order_acquire); // acquire because we need to see the latest value of head that was written by other threads. this ensures that we have a consistent view of the buffer and can accurately determine whether the buffer is empty or not.

    // buffer is empty, return empty optional to indicate no value
    if (tail == head) {
        return std::nullopt; 
    }

    // instead of claiming slot at head --> claim slot at tail. 
    // writing an event in --> it reads an event out and moves tail forward. 
    if (!tail_.compare_exchange_weak(
            tail,
            tail + 1,
            std::memory_order_release, // success order. swap succeeds, we claimed the slot. release so writers who load tail with acquire will see that we've read that slot and can safely overwrite it.
            std::memory_order_relaxed)) { // failure order. we're returning so no guarantees needed. relaxed because we don't need to synchronize with other threads if the compare-swap fails. this allows for maximum performance while still ensuring that we get a consistent view of the tail variable.
        return std::nullopt; // another thread got here first, caller retries
    }

    // reading FROM tail 
    uint64_t slot = tail & (BUFFER_SIZE - 1); 

    LogEvent event = buffer_[slot]; // copy the event out of the slot we just claimed
    return event;
}

size_t RingBuffer::size() const {
    return head_.load(std::memory_order_acquire) - 
           tail_.load(std::memory_order_relaxed);
}

bool RingBuffer::empty() const {
    return size() == 0;
}
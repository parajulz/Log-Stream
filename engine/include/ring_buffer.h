#pragma once 
#include <atomic> // gives us std::atomic, atomic operations (compare and swap. without this lock-free operations are impossible)
#include <array> // a fixed-size array. we use this to store the data in the ring buffer. lives entirely in memory and is not resized at runtime. this allows us to avoid dynamic memory allocation and deallocation, which can be expensive and lead to fragmentation. never allocates on heap 
#include <cstdint> // gives us fixed-width integer types (e.g., uint8_t, uint16_t, etc.). these types are used to ensure that our ring buffer can store data of a specific size, regardless of the platform or architecture. this is important for ensuring that our ring buffer can be used in a wide range of applications and environments.
#include <optional> // gives us std::optional, a wrapper that can contain either a value or no value. we use this to indicate whether a slot in the ring buffer is occupied or not. if the optional contains a value, it means that the slot is occupied and contains valid data. if the optional does not contain a value, it means that the slot is empty and can be used to store new data. this allows us to efficiently manage the state of each slot in the ring buffer without needing additional flags or indicators.


// LogEvent - one unit of data in the buffer
struct LogEvent {
    char level[16]; // the severity level of the log event (e.g., "INFO", "ERROR", etc.). we use a fixed-size character array to store the level, which allows us to avoid dynamic memory allocation and ensures that the size of each log event is consistent.
    // fixed size char array instead of std::string to avoid dynamic memory allocation and ensure that the size of each log event is consistent. this allows us to store log events in a fixed-size array, which is more efficient and easier to manage than using dynamic memory allocation.
    char message[256]; // the log message itself. we use a fixed-size character array to avoid heap allocation 
    int64_t timestamp; // when this event happened. unix timestamp in milliseconds. we use a 64 bit integer bc it can represent a wide range of timestamps, including those far in the past and far in the future. this allows us to accurately record the time of each log event without worrying about overflow or other issues that can arise with smaller integer types.
};

// size of the buffer. we use a power of 2 to allow for efficient wrapping around when the head and tail pointers reach the end of the buffer. this allows us to avoid the need for complex modulo operations, which can be expensive and lead to performance issues. by using a power of 2, we can simply use bitwise operations to wrap around the pointers, which is much more efficient.
// constexpr means evaluated at compile time not runtime, which allows us to avoid the overhead of calculating the buffer size at runtime. this is important for ensuring that our ring buffer can be used in performance-critical applications where every bit of efficiency counts. by defining the buffer size as a compile-time constant, we can ensure that our ring buffer is optimized for performance and can be used in a wide range of applications and environments.
constexpr size_t BUFFER_SIZE = 1024 * 64; 
 
//ringBuffer class 

class RingBuffer {
    public: 
        // write one log event to the buffer
        // returns true if write succeeded
        // returns false if the buffer is full and the event could not be written. this allows us to efficiently manage the state of the buffer and ensure that we do not overwrite existing data when the buffer is full. 
        bool write(const LogEvent& event); 

        // read one event from the buffer and remove it
        // returns std::optional<LogEvent> instead of LogEvent directly 
        // because if the buffer is empty, we need a way to indicate that there is no valid data to return. by using std::optional, we can return an empty optional to indicate that the buffer is empty, and a non-empty optional to indicate that there is valid data to return.
        // optional lets us express "no value" without crashing or returning garbage
        std::optional<LogEvent> read(); 

        //how many events are currently sitting in the buffer
        // const because it does not modify the state of the buffer. 
        // this allows us to safely call this method from multiple threads without worrying about race conditions or other issues that can arise when multiple threads are accessing and modifying shared data. by marking this method as const, we can ensure that it is thread-safe and can be used in a wide range of applications and environments.
        size_t size() const; 

        // returns true if there are zero events in the buffer 
        bool empty() const; 
    
    private:

    // actual storage — 65,536 slots of LogEvent sitting in memory
    // lives on the heap because it's too large for the stack
    // (65,536 * ~280 bytes = ~18MB)
    std::array<LogEvent, BUFFER_SIZE> buffer_;

    // head_ — tracks where the NEXT WRITE goes
    // std::atomic means reads and writes to this variable are thread-safe
    // without atomic, two threads reading head_ simultaneously causes corruption

    // alignas(64) aligns this variable to a 64-byte cache line boundary
    // this prevents FALSE SHARING — the hidden performance killer:
    // if head_ and tail_ share the same cache line, updating head_ on one core
    // invalidates tail_ in every other core's cache, forcing expensive reloads
    // giving each its own cache line eliminates that completely
    alignas(64) std::atomic<uint64_t> head_{0};

    // tail_ — tracks where the NEXT READ comes from
    // same atomic and alignas reasoning as head_
    // head_ and tail_ are separated so they never share a cache line
    alignas(64) std::atomic<uint64_t> tail_{0};
};
    


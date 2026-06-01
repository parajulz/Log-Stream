// defines the shape of data coming from the Python API
// TypeScript uses these to catch errors at compile time

// one log event — matches LogEvent in ring_buffer.h and logstream.proto
export interface LogEvent {
    level: string      // "error", "warning", "info"
    message: string    // the log message
    timestamp: number  // unix timestamp in milliseconds
}

// metrics returned by GET /metrics
export interface Metrics {
    events_per_second: number   // current ingestion rate
    p99_latency_us: number      // P99 write latency in microseconds
    total_events: number        // total events ingested since startup
    buffer_size: number         // events currently in the ring buffer
}

// one data point on the ingestion rate chart
// we build an array of these over time as metrics come in
export interface IngestionDataPoint {
    time: string        // formatted time label for the x axis
    events_per_second: number
}

// one data point on the latency chart
export interface LatencyDataPoint {
    time: string
    p99_latency_us: number
}
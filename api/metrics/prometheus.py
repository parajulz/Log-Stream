# defines Prometheus metrics for the LogStream API
# these are the numbers Prometheus scrapes and stores over time
# so the React dashboard can draw charts of historical performance

from prometheus_client import Counter, Histogram, Gauge

# counts total number of events successfully ingested since startup
# Counter only goes up — never resets
#Counter — a number that only ever goes up. Total events ingested, total errors. Never resets. Good for "how many times did X happen."

events_ingested_total = Counter(
    "logstream_events_ingested_total",  # metric name
    "Total number of events ingested"   # description
)

# counts total number of failed ingestion attempts
# useful for knowing when the buffer is full
events_failed_total = Counter(
    "logstream_events_failed_total",
    "Total number of failed ingestion attempts"
)

# tracks how long each ingest call takes
# Histogram automatically calculates P50, P99, P999 from these measurements
#records individual measurements and automatically groups them into buckets. From those buckets Prometheus calculates P50, P99, P999. This is how you get latency percentiles without storing every single measurement.

write_latency = Histogram(
    "logstream_write_latency_seconds",
    "Time taken to write one event to the C++ engine",
    buckets=[0.0001, 0.0005, 0.001, 0.005, 0.01, 0.05, 0.1]
    # buckets in seconds: 100µs, 500µs, 1ms, 5ms, 10ms, 50ms, 100ms
)

#Gauge — a number that can go up or down. Buffer occupancy goes up when events are written in, down when they're read out.

# tracks current number of events sitting in the ring buffer
# Gauge can go up and down — unlike Counter
buffer_occupancy = Gauge(
    "logstream_buffer_occupancy",
    "Current number of events in the ring buffer"
)
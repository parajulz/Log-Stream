# handles GET /metrics
# returns current performance metrics so the React dashboard can draw charts

from fastapi import APIRouter
import sys
import os

sys.path.append(os.path.join(os.path.dirname(__file__), "../proto"))
import logstream_pb2

router = APIRouter()

from main import stub

@router.get("/metrics")
def metrics():
    # ask the C++ engine for current performance numbers
    request = logstream_pb2.MetricsRequest()
    response = stub.GetMetrics(request)

    # return as plain JSON for the React dashboard
    return {
        "events_per_second": response.events_per_second,  # current ingestion rate
        "p99_latency_us": response.p99_latency_us,        # P99 write latency in microseconds
        "total_events": response.total_events,             # total events since startup
        "buffer_size": response.buffer_size                # events currently in buffer
    }
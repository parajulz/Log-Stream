# handles POST /ingest
# receives a log event over HTTP and forwards it to the C++ engine via gRPC

from metrics.prometheus import events_ingested_total, events_failed_total, write_latency
import time 
from fastapi import APIRouter, HTTPException
from pydantic import BaseModel
import sys
import os

sys.path.append(os.path.join(os.path.dirname(__file__), "../proto"))
import logstream_pb2
import logstream_pb2_grpc

router = APIRouter()

# defines what a log event looks like when it arrives over HTTP
# pydantic validates the incoming JSON automatically
class LogEventRequest(BaseModel):
    level: str      # "error", "warning", "info"
    message: str    # the log message
    timestamp: int  # unix timestamp in milliseconds

# the stub is imported from main.py — shared across all routes
# we import it here to forward events to C++
from main import stub

@router.post("/ingest")
def ingest(event: LogEventRequest):
    # convert the HTTP request body into a protobuf LogEvent
    # that C++ understands
    proto_event = logstream_pb2.LogEvent(
        level=event.level,
        message=event.message,
        timestamp=event.timestamp
    )
    
    # now forward to C++ engine via gRPC

    #wrap the gRPC call with timing to measure latency
    start_time = time.time()
    response = stub.Ingest(proto_event)
    latency = time.time() - start_time #duration in seconds


    # if C++ says buffer is full, return 503 Service Unavailable
    if not response.success:
        events_failed_total.inc()  # increment the failure counter
        raise HTTPException(status_code=503, detail="buffer full")
    
    events_ingested_total.inc()  # increment the success counter
    write_latency.observe(latency)  # record the latency in the histogram

    return {"success": True}
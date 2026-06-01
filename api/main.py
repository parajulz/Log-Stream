#POST/ingest - receives log event from outside world over HTPP and forwards it to the C++ engine via gRPC to be written into the ring buffer
#GET/query - receives a search request like (like level = error) and asls the C++ engine to look it up in th einverted index and return matching events
#GET/metrics - returns Prometheus metrics about the system: current ingestion rate, latency numbers, thread counts, so the REACT dashboard can read them and draw charts

import grpc
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
import sys
import os

# add the proto folder to the path so Python can find the generated gRPC files
sys.path.append(os.path.join(os.path.dirname(__file__), "proto"))

import logstream_pb2
import logstream_pb2_grpc

app = FastAPI(title = "LogStream API")

# enable React to call this API from the browser
# without this, the browser would block the requests from a differnet port 

#CORS - allows React on 5175 to call this API on 8000 without being blocked by the browser's same-origin policy
app.add_middleware(
    CORSMiddleware,
    allow_origins=["http://localhost:5173"],  # Vite's default port
    allow_methods=["*"],
    allow_headers=["*"],
)

#connect to the C++ engine via gRPC
#the channel listens on port 50051 
channel = grpc.insecure_channel('localhost:50051')
stub = logstream_pb2_grpc.LogStreamServiceStub(channel) 

# import the routes -- each route file handles one endpoint 
from routes import ingest, query, metrics 

# register the routes with the app
app.include_router(ingest.router)
app.include_router(query.router)
app.include_router(metrics.router)

# health check — lets you verify the API is running
@app.get("/health")
def health():
    return {"status": "ok"}

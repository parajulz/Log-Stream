# handles GET /query
# receives a search request and asks the C++ engine to search the inverted index

from fastapi import APIRouter, HTTPException
import sys
import os

sys.path.append(os.path.join(os.path.dirname(__file__), "../proto"))
import logstream_pb2

router = APIRouter()

from main import stub

@router.get("/query")
def query(field: str, value: str):
    # field and value come from the URL query string
    # example: GET /query?field=level&value=error

    # build the protobuf query request
    request = logstream_pb2.QueryRequest(
        field=field,
        value=value
    )

    # ask C++ to search the inverted index
    response = stub.Query(request)

    # convert protobuf response back into plain Python dicts
    # so FastAPI can serialize them to JSON for the React frontend
    events = [
        {
            "level": e.level,
            "message": e.message,
            "timestamp": e.timestamp
        }
        for e in response.events
    ]

    return {"events": events}
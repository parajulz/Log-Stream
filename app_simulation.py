# simulates a real application sending log events to LogStream
# run this while the dashboard is open to see live metrics

import requests
import time
import random
import threading
import sys

# how many threads to use — more threads = more events/sec
NUM_THREADS = 8

# API endpoint
URL = "http://localhost:8000/ingest"

# sample data to randomize events
LEVELS = ["info", "info", "info", "warning", "error"]  # info is most common
MESSAGES = [
    "user logged in",
    "user logged out",
    "payment processed",
    "payment failed",
    "request timeout",
    "high memory usage",
    "cache miss",
    "database query slow",
    "API rate limit hit",
    "file upload complete",
]

# track total events sent
total_sent = 0
total_failed = 0
lock = threading.Lock()
running = True

def send_events():
    global total_sent, total_failed
    session = requests.Session()  # reuse connection — much faster than new connection per request

    while running:
        try:
            response = session.post(URL, json={
                "level": random.choice(LEVELS),
                "message": random.choice(MESSAGES),
                "timestamp": int(time.time() * 1000)
            }, timeout=1)

            with lock:
                if response.status_code == 200:
                    total_sent += 1
                else:
                    total_failed += 1

        except Exception:
            with lock:
                total_failed += 1

def print_stats():
    global total_sent, total_failed
    start = time.time()
    last_sent = 0

    while running:
        time.sleep(1)
        elapsed = time.time() - start
        with lock:
            sent = total_sent
            failed = total_failed

        events_per_sec = sent - last_sent
        last_sent = sent

        print(f"[{elapsed:.0f}s] sent: {sent:,} | failed: {failed:,} | rate: {events_per_sec:,}/sec")

if __name__ == "__main__":
    print(f"Starting load test with {NUM_THREADS} threads...")
    print("Watch the dashboard at http://localhost:5173")
    print("Press Ctrl+C to stop\n")

    # start sender threads
    threads = []
    for _ in range(NUM_THREADS):
        t = threading.Thread(target=send_events, daemon=True)
        t.start()
        threads.append(t)

    # start stats printer
    stats_thread = threading.Thread(target=print_stats, daemon=True)
    stats_thread.start()

    try:
        while True:
            time.sleep(0.1)
    except KeyboardInterrupt:
        running = False
        print(f"\nStopped. Total sent: {total_sent:,}")
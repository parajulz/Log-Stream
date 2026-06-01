import { useState, useEffect } from 'react'

// the shape of data coming from GET /metrics
interface Metrics {
    events_per_second: number
    p99_latency_us: number
    total_events: number
    buffer_size: number
}

// one data point on a chart
interface DataPoint {
    time: string
    value: number
}

export function useMetrics() {
    const [metrics, setMetrics] = useState<Metrics | null>(null)
    const [ingestionData, setIngestionData] = useState<DataPoint[]>([])
    const [latencyData, setLatencyData] = useState<DataPoint[]>([])
    const [connected, setConnected] = useState(false)

    useEffect(() => {
        const fetchMetrics = async () => {
            try {
                const response = await fetch('http://localhost:8000/metrics')
                if (!response.ok) {
                    setConnected(false)
                    return
                }

                const data: Metrics = await response.json()
                setConnected(true)
                setMetrics(data)

                const time = new Date().toLocaleTimeString()

                // add new point, keep last 60 seconds
                setIngestionData(prev => [
                    ...prev.slice(-59),
                    { time, value: data.events_per_second }
                ])

                setLatencyData(prev => [
                    ...prev.slice(-59),
                    { time, value: data.p99_latency_us }
                ])

            } catch {
                setConnected(false)
            }
        }

        fetchMetrics()
        const interval = setInterval(fetchMetrics, 1000)
        return () => clearInterval(interval)

    }, [])

    return { metrics, ingestionData, latencyData, connected }
}
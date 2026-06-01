import { useMetrics } from './hooks/useMetrics'
import { IngestionChart } from './components/IngestionChart'
import { LatencyChart } from './components/LatencyChart'
import { ScalingChart } from './components/ScalingChart'
import { LogSearch } from './components/LogSearch'

function App() {
    const { metrics, ingestionData, latencyData, connected } = useMetrics()

    return (
        <div style={{ maxWidth: '900px', margin: '0 auto', padding: '2rem' }}>

            {/* header */}
            <h1 style={{ fontSize: '24px', fontWeight: 500, marginBottom: '0.5rem' }}>
                LogStream
            </h1>
            <p style={{ color: connected ? 'green' : 'gray', marginBottom: '2rem' }}>
                {connected ? 'connected' : 'waiting for API...'}
            </p>

            {/* metric cards — only show when API is connected */}
            {metrics && (
                <div style={{ display: 'flex', gap: '1rem', marginBottom: '2rem' }}>
                    <div style={{ padding: '1rem', border: '1px solid #eee', borderRadius: '8px', flex: 1 }}>
                        <p style={{ fontSize: '12px', color: '#888', margin: 0 }}>events/sec</p>
                        <p style={{ fontSize: '24px', fontWeight: 500, margin: 0 }}>
                            {metrics.events_per_second.toLocaleString()}
                        </p>
                    </div>
                    <div style={{ padding: '1rem', border: '1px solid #eee', borderRadius: '8px', flex: 1 }}>
                        <p style={{ fontSize: '12px', color: '#888', margin: 0 }}>P99 latency</p>
                        <p style={{ fontSize: '24px', fontWeight: 500, margin: 0 }}>
                            {metrics.p99_latency_us}µs
                        </p>
                    </div>
                    <div style={{ padding: '1rem', border: '1px solid #eee', borderRadius: '8px', flex: 1 }}>
                        <p style={{ fontSize: '12px', color: '#888', margin: 0 }}>total events</p>
                        <p style={{ fontSize: '24px', fontWeight: 500, margin: 0 }}>
                            {metrics.total_events.toLocaleString()}
                        </p>
                    </div>
                    <div style={{ padding: '1rem', border: '1px solid #eee', borderRadius: '8px', flex: 1 }}>
                        <p style={{ fontSize: '12px', color: '#888', margin: 0 }}>buffer size</p>
                        <p style={{ fontSize: '24px', fontWeight: 500, margin: 0 }}>
                            {metrics.buffer_size.toLocaleString()}
                        </p>
                    </div>
                </div>
            )}

            {/* live charts */}
            <div style={{ border: '1px solid #eee', borderRadius: '8px', marginBottom: '1rem' }}>
                <IngestionChart data={ingestionData} />
            </div>

            <div style={{ border: '1px solid #eee', borderRadius: '8px', marginBottom: '1rem' }}>
                <LatencyChart data={latencyData} />
            </div>

            {/* benchmark comparison — always visible */}
            <div style={{ border: '1px solid #eee', borderRadius: '8px', marginBottom: '1rem' }}>
                <ScalingChart />
            </div>

            {/* log search */}
            <div style={{ border: '1px solid #eee', borderRadius: '8px', marginBottom: '1rem' }}>
                <LogSearch />
            </div>

        </div>
    )
}

export default App
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, Legend, ResponsiveContainer } from 'recharts'

// hardcoded benchmark data — replace with your real numbers after running the benchmark
const data = [
    { threads: 1,  mutex: 850000,   lockfree: 1200000 },
    { threads: 2,  mutex: 1400000,  lockfree: 2350000 },
    { threads: 4,  mutex: 1800000,  lockfree: 4600000 },
    { threads: 8,  mutex: 1200000,  lockfree: 9100000 },
    { threads: 16, mutex: 330000,   lockfree: 18000000 },
    { threads: 32, mutex: 210000,   lockfree: 35000000 },
]

export function ScalingChart() {
    return (
        <div style={{ padding: '1rem' }}>
            <h2 style={{ fontSize: '16px', fontWeight: 500, marginBottom: '0.25rem' }}>
                Mutex vs lock-free throughput
            </h2>
            <p style={{ fontSize: '12px', color: '#888', marginBottom: '1rem' }}>
                Mutex collapses at 16+ threads. Lock-free scales near-linearly.
            </p>

            <ResponsiveContainer width="100%" height={300}>
                <LineChart data={data}>
                    <CartesianGrid strokeDasharray="3 3" />
                    <XAxis
                        dataKey="threads"
                        tick={{ fontSize: 11 }}
                        label={{ value: 'threads', position: 'insideBottom', offset: -2, fontSize: 11 }}
                    />
                    <YAxis
                        tick={{ fontSize: 11 }}
                        tickFormatter={(value: number) => `${(value / 1000000).toFixed(1)}M`}
                    />
                    <Tooltip
                        formatter={(value: number, name: string) => [
                            `${(value / 1000000).toFixed(2)}M events/sec`,
                            name === 'mutex' ? 'Mutex' : 'Lock-free'
                        ]}
                        labelFormatter={(label) => `${label} threads`}
                    />
                    <Legend />
                    <Line
                        type="monotone"
                        dataKey="mutex"
                        name="Mutex"
                        stroke="#D85A30"
                        strokeWidth={2}
                        dot={{ r: 4 }}
                        isAnimationActive={false}
                    />
                    <Line
                        type="monotone"
                        dataKey="lockfree"
                        name="Lock-free"
                        stroke="#1D9E75"
                        strokeWidth={2}
                        dot={{ r: 4 }}
                        isAnimationActive={false}
                    />
                </LineChart>
            </ResponsiveContainer>
        </div>
    )
}
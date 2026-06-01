import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer } from 'recharts'

interface Props {
    data: { time: string, value: number }[]
}

export function LatencyChart({ data }: Props) {
    return (
        <div style={{ padding: '1rem' }}>
            <h2 style={{ fontSize: '16px', fontWeight: 500, marginBottom: '1rem' }}>
                P99 write latency (µs)
            </h2>

            <ResponsiveContainer width="100%" height={300}>
                <LineChart data={data}>
                    <CartesianGrid strokeDasharray="3 3" />
                    <XAxis dataKey="time" tick={{ fontSize: 11 }} />
                    <YAxis
                        tick={{ fontSize: 11 }}
                        tickFormatter={(value: number) => `${value}µs`}
                    />
                    <Tooltip
                        formatter={(value: number) => [`${value}µs`, 'P99 latency']}
                    />
                    <Line
                        type="monotone"
                        dataKey="value"
                        stroke="#D85A30"
                        strokeWidth={2}
                        dot={false}
                        isAnimationActive={false}
                    />
                </LineChart>
            </ResponsiveContainer>
        </div>
    )
}
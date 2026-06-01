import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer } from 'recharts'

// the data this component receives from App.tsx
interface Props {
    data: { time: string, value: number }[]
}

export function IngestionChart({ data }: Props) {
    return (
        <div style={{ padding: '1rem' }}>
            <h2 style={{ fontSize: '16px', fontWeight: 500, marginBottom: '1rem' }}>
                Live ingestion rate (events/sec)
            </h2>

            <ResponsiveContainer width="100%" height={300}>
                <LineChart data={data}>
                    <CartesianGrid strokeDasharray="3 3" />
                    <XAxis dataKey="time" tick={{ fontSize: 11 }} />
                    <YAxis tick={{ fontSize: 11 }} />
                    <Tooltip formatter={(value: number) => [`${value.toLocaleString()} events/sec`]} />
                    <Line
                        type="monotone"
                        dataKey="value"
                        stroke="#1D9E75"
                        strokeWidth={2}
                        dot={false}
                        isAnimationActive={false}
                    />
                </LineChart>
            </ResponsiveContainer>
        </div>
    )
}
import { useState } from 'react'

interface LogEvent {
    level: string
    message: string
    timestamp: number
}

export function LogSearch() {
    const [field, setField] = useState('level')
    const [value, setValue] = useState('')
    const [results, setResults] = useState<LogEvent[]>([])
    const [loading, setLoading] = useState(false)
    const [error, setError] = useState<string | null>(null)

    const search = async () => {
        if (!value.trim()) return

        setLoading(true)
        setError(null)

        try {
            const response = await fetch(
                `http://localhost:8000/query?field=${field}&value=${value}`
            )

            if (!response.ok) {
                setError('query failed')
                return
            }

            const data = await response.json()
            setResults(data.events)

        } catch {
            setError('could not reach API')
        } finally {
            setLoading(false)
        }
    }

    return (
        <div style={{ padding: '1rem' }}>
            <h2 style={{ fontSize: '16px', fontWeight: 500, marginBottom: '1rem' }}>
                Search logs
            </h2>

            {/* search inputs */}
            <div style={{ display: 'flex', gap: '8px', marginBottom: '1rem' }}>
                <input
                    type="text"
                    value={field}
                    onChange={e => setField(e.target.value)}
                    placeholder="field (e.g. level)"
                    style={{ padding: '6px 10px', borderRadius: '6px', border: '1px solid #ddd', fontSize: '13px' }}
                />
                <input
                    type="text"
                    value={value}
                    onChange={e => setValue(e.target.value)}
                    placeholder="value (e.g. error)"
                    style={{ padding: '6px 10px', borderRadius: '6px', border: '1px solid #ddd', fontSize: '13px' }}
                    onKeyDown={e => e.key === 'Enter' && search()}
                />
                <button
                    onClick={search}
                    disabled={loading}
                    style={{ padding: '6px 14px', borderRadius: '6px', border: '1px solid #ddd', cursor: 'pointer', fontSize: '13px' }}
                >
                    {loading ? 'searching...' : 'search'}
                </button>
            </div>

            {/* error */}
            {error && (
                <p style={{ color: '#D85A30', fontSize: '13px', marginBottom: '1rem' }}>
                    {error}
                </p>
            )}

            {/* results */}
            {results.length === 0 && !loading && (
                <p style={{ color: '#888', fontSize: '13px' }}>no results</p>
            )}

            {results.map((event, index) => (
                <div
                    key={index}
                    style={{
                        padding: '8px 12px',
                        marginBottom: '6px',
                        borderRadius: '6px',
                        border: '1px solid #eee',
                        fontSize: '12px',
                        fontFamily: 'monospace'
                    }}
                >
                    <span style={{
                        color: event.level === 'error' ? '#D85A30' : event.level === 'warning' ? '#854F0B' : '#085041',
                        fontWeight: 500,
                        marginRight: '8px'
                    }}>
                        [{event.level}]
                    </span>
                    <span>{event.message}</span>
                    <span style={{ color: '#888', marginLeft: '8px' }}>
                        {new Date(event.timestamp).toLocaleTimeString()}
                    </span>
                </div>
            ))}
        </div>
    )
}
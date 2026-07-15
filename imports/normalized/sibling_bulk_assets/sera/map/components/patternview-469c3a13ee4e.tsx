"use client";
import React, { useState, useEffect } from 'react';
import { behavioralAPI } from '@/lib/api/behavioral';
import { Card } from '@/components/ui/card';
import { Badge } from '@/components/ui/badge';
import { Alert } from '@/components/ui/alert';

interface Pattern {
    patternType: string;
    confidence: number;
    occurrences: number;
    description: string;
    recommendation: string;
}

export function PatternView() {
    const [individualId, setIndividualId] = useState('');
    const [patterns, setPatterns] = useState<Pattern[]>([]);
    const [loading, setLoading] = useState(false);
    const [error, setError] = useState<string | null>(null);

    const loadPatterns = async () => {
        if (!individualId) {
            setError('Please enter an Individual ID');
            return;
        }

        setLoading(true);
        setError(null);

        try {
            const data = await behavioralAPI.getPatterns(individualId);
            // Assuming the API returns patterns in a format we can use
            if (data.patterns) {
                setPatterns(data.patterns);
            } else {
                setPatterns([]);
            }
        } catch (err: any) {
            setError(err.message || 'Failed to load patterns');
            setPatterns([]);
        } finally {
            setLoading(false);
        }
    };

    const getConfidenceColor = (confidence: number) => {
        if (confidence >= 80) return 'success';
        if (confidence >= 60) return 'warning';
        return 'info';
    };

    return (
        <div className="space-y-6">
            <div>
                <h2 className="text-xl font-semibold text-gray-900">Behavioral Pattern Analysis</h2>
                <p className="text-sm text-gray-600 mt-1">
                    AI-powered pattern recognition to identify behavioral trends
                </p>
            </div>

            <Card className="p-6">
                <div className="flex gap-3 mb-4">
                    <input
                        type="text"
                        placeholder="Enter Individual ID"
                        value={individualId}
                        onChange={(e) => setIndividualId(e.target.value)}
                        className="flex-1 px-3 py-2 border rounded-lg"
                    />
                    <button
                        onClick={loadPatterns}
                        disabled={loading || !individualId}
                        className="px-4 py-2 bg-blue-600 text-white rounded-lg hover:bg-blue-700 disabled:bg-gray-400"
                    >
                        {loading ? 'Analyzing...' : 'Analyze Patterns'}
                    </button>
                </div>

                {error && (
                    <Alert variant="error" title="Error" onClose={() => setError(null)}>
                        {error}
                    </Alert>
                )}

                {loading && (
                    <div className="text-center text-gray-500 py-8">
                        Analyzing behavioral patterns...
                    </div>
                )}

                {!loading && patterns.length === 0 && !error && (
                    <div className="text-center text-gray-500 py-8">
                        Enter an Individual ID and click "Analyze Patterns" to view behavioral patterns
                    </div>
                )}

                {patterns.length > 0 && (
                    <div className="space-y-4">
                        {patterns.map((pattern, idx) => (
                            <Card key={idx} className="p-4 border-l-4 border-l-blue-500">
                                <div className="flex items-start justify-between mb-2">
                                    <h3 className="font-semibold text-gray-900">{pattern.patternType}</h3>
                                    <div className="flex gap-2">
                                        <Badge variant={getConfidenceColor(pattern.confidence) as any}>
                                            {pattern.confidence}% confidence
                                        </Badge>
                                        <Badge variant="info">
                                            {pattern.occurrences} occurrence{pattern.occurrences !== 1 ? 's' : ''}
                                        </Badge>
                                    </div>
                                </div>
                                <p className="text-sm text-gray-700 mb-2">{pattern.description}</p>
                                <div className="bg-blue-50 p-3 rounded border border-blue-200">
                                    <p className="text-sm font-medium text-blue-900 mb-1">Recommendation:</p>
                                    <p className="text-sm text-blue-800">{pattern.recommendation}</p>
                                </div>
                            </Card>
                        ))}
                    </div>
                )}
            </Card>

            <Card className="p-4 bg-gray-50">
                <h3 className="text-sm font-semibold text-gray-900 mb-2">💡 Pattern Analysis Features</h3>
                <ul className="text-sm text-gray-700 space-y-1">
                    <li>• Identifies recurring behavioral patterns</li>
                    <li>• Analyzes antecedents and consequences</li>
                    <li>• Provides evidence-based recommendations</li>
                    <li>• Tracks pattern confidence over time</li>
                    <li>• Helps inform behavior support plans</li>
                </ul>
            </Card>
        </div>
    );
}


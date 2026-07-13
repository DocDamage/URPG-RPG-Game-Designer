import React, { useState } from 'react';

export interface Intervention {
    id: string;
    title: string;
    description: string;
    effectiveness: number;
    steps: string[];
    confidence: number;
}

export interface InterventionSuggestionProps {
    intervention: Intervention;
    onAccept?: (id: string) => void;
    onDismiss?: (id: string) => void;
}

export function InterventionSuggestion({
    intervention,
    onAccept,
    onDismiss
}: InterventionSuggestionProps) {
    const [expanded, setExpanded] = useState(false);

    return (
        <div className="bg-gradient-to-r from-blue-50 to-indigo-50 border border-blue-200 rounded-lg p-4 shadow-sm">
            <div className="flex items-start justify-between">
                <div className="flex-1">
                    <div className="flex items-center space-x-2">
                        <span className="text-2xl">💡</span>
                        <h3 className="font-semibold text-lg text-gray-900">{intervention.title}</h3>
                    </div>
                    <p className="text-sm text-gray-700 mt-2">{intervention.description}</p>

                    <div className="flex items-center space-x-4 mt-3 text-sm">
                        <div className="flex items-center">
                            <span className="text-gray-600">Effectiveness:</span>
                            <span className="ml-2 font-semibold text-green-600">
                                {intervention.effectiveness}%
                            </span>
                        </div>
                        <div className="flex items-center">
                            <span className="text-gray-600">AI Confidence:</span>
                            <span className="ml-2 font-semibold text-blue-600">
                                {intervention.confidence}%
                            </span>
                        </div>
                    </div>

                    {expanded && (
                        <div className="mt-4 p-3 bg-white rounded border border-gray-200">
                            <h4 className="font-medium text-gray-900 mb-2">Steps:</h4>
                            <ol className="list-decimal list-inside space-y-1 text-sm text-gray-700">
                                {intervention.steps.map((step, idx) => (
                                    <li key={idx}>{step}</li>
                                ))}
                            </ol>
                        </div>
                    )}
                </div>
            </div>

            <div className="flex items-center space-x-2 mt-4">
                <button
                    onClick={() => setExpanded(!expanded)}
                    className="px-4 py-2 text-sm bg-white border border-gray-300 rounded hover:bg-gray-50"
                >
                    {expanded ? 'Hide Details' : 'Show Details'}
                </button>
                {onAccept && (
                    <button
                        onClick={() => onAccept(intervention.id)}
                        className="px-4 py-2 text-sm bg-green-600 text-white rounded hover:bg-green-700"
                    >
                        Use This
                    </button>
                )}
                {onDismiss && (
                    <button
                        onClick={() => onDismiss(intervention.id)}
                        className="px-4 py-2 text-sm text-gray-600 hover:text-gray-800"
                    >
                        Dismiss
                    </button>
                )}
            </div>
        </div>
    );
}

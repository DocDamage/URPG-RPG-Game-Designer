import React from 'react';

export interface BehavioralEvent {
    timestamp: string;
    type: string;
    severity: 'low' | 'medium' | 'high';
    description: string;
    intervention?: string;
}

export interface BehavioralTimelineProps {
    events: BehavioralEvent[];
}

export function BehavioralTimeline({ events }: BehavioralTimelineProps) {
    const severityColors = {
        low: 'bg-green-100 border-green-500',
        medium: 'bg-yellow-100 border-yellow-500',
        high: 'bg-red-100 border-red-500'
    };

    return (
        <div className="space-y-4">
            {events.length === 0 ? (
                <p className="text-center text-gray-500 py-8">No behavioral events recorded</p>
            ) : (
                events.map((event, idx) => (
                    <div key={idx} className="flex">
                        <div className="flex flex-col items-center mr-4">
                            <div className={`w-4 h-4 rounded-full ${severityColors[event.severity].split(' ')[0]}`} />
                            {idx < events.length - 1 && (
                                <div className="w-0.5 flex-1 bg-gray-300 mt-1" />
                            )}
                        </div>

                        <div className={`flex-1 p-4 rounded-lg border-l-4 ${severityColors[event.severity]}`}>
                            <div className="flex items-start justify-between">
                                <div>
                                    <span className="text-xs text-gray-500">
                                        {new Date(event.timestamp).toLocaleTimeString()}
                                    </span>
                                    <h4 className="font-semibold text-gray-900 mt-1">{event.type}</h4>
                                    <p className="text-sm text-gray-700 mt-1">{event.description}</p>
                                    {event.intervention && (
                                        <div className="mt-2 p-2 bg-white rounded text-sm">
                                            <span className="font-medium">Intervention:</span> {event.intervention}
                                        </div>
                                    )}
                                </div>
                                <span className={`
                  text-xs px-2 py-1 rounded
                  ${event.severity === 'low' ? 'bg-green-200 text-green-800' : ''}
                  ${event.severity === 'medium' ? 'bg-yellow-200 text-yellow-800' : ''}
                  ${event.severity === 'high' ? 'bg-red-200 text-red-800' : ''}
                `}>
                                    {event.severity}
                                </span>
                            </div>
                        </div>
                    </div>
                ))
            )}
        </div>
    );
}

"use client";
import React from 'react';
import { Card } from '@/components/ui/card';
import { Badge } from '@/components/ui/badge';
import { Alert } from '@/components/ui/alert';

interface OvernightEvent {
    id: string;
    timestamp: string;
    type: 'incident' | 'medication' | 'behavioral' | 'medical' | 'other';
    description: string;
    severity: 'low' | 'medium' | 'high';
    individualId?: string;
}

interface OvernightSummaryProps {
    events: OvernightEvent[];
}

export function OvernightSummary({ events }: OvernightSummaryProps) {
    const formatTime = (timestamp: string) => {
        return new Date(timestamp).toLocaleTimeString('en-US', {
            hour: '2-digit',
            minute: '2-digit'
        });
    };

    const typeIcons = {
        incident: '🚨',
        medication: '💊',
        behavioral: '📊',
        medical: '🏥',
        other: '📝'
    };

    const severityColors = {
        low: 'info' as const,
        medium: 'warning' as const,
        high: 'error' as const
    };

    if (events.length === 0) {
        return (
            <Card className="p-6">
                <h3 className="text-lg font-semibold text-gray-900 mb-4">Overnight Events</h3>
                <div className="text-center text-gray-500 py-4">
                    <p>✓ No overnight events to report</p>
                    <p className="text-sm mt-2">All individuals had a quiet night</p>
                </div>
            </Card>
        );
    }

    return (
        <Card className="p-6">
            <div className="flex items-center justify-between mb-4">
                <h3 className="text-lg font-semibold text-gray-900">Overnight Events</h3>
                <Badge variant="info">{events.length} event{events.length !== 1 ? 's' : ''}</Badge>
            </div>

            <div className="space-y-3">
                {events.map((event) => (
                    <div
                        key={event.id}
                        className={`p-4 rounded-lg border-l-4 ${
                            event.severity === 'high'
                                ? 'bg-red-50 border-red-500'
                                : event.severity === 'medium'
                                ? 'bg-yellow-50 border-yellow-500'
                                : 'bg-blue-50 border-blue-500'
                        }`}
                    >
                        <div className="flex items-start justify-between">
                            <div className="flex-1">
                                <div className="flex items-center gap-2 mb-2">
                                    <span className="text-xl">{typeIcons[event.type]}</span>
                                    <span className="font-medium text-gray-900">
                                        {event.type.charAt(0).toUpperCase() + event.type.slice(1)} Event
                                    </span>
                                    <Badge variant={severityColors[event.severity]}>
                                        {event.severity}
                                    </Badge>
                                </div>
                                <p className="text-sm text-gray-700 mb-1">{event.description}</p>
                                {event.individualId && (
                                    <p className="text-xs text-gray-600">Individual ID: {event.individualId}</p>
                                )}
                            </div>
                            <div className="text-xs text-gray-500 ml-4">
                                {formatTime(event.timestamp)}
                            </div>
                        </div>
                    </div>
                ))}
            </div>

            {events.some(e => e.severity === 'high') && (
                <Alert variant="error" title="High Priority Events" className="mt-4">
                    Please review high-priority overnight events before starting your shift
                </Alert>
            )}
        </Card>
    );
}


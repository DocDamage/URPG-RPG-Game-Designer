import React from 'react';
import { useBriefing } from '../lib/hooks/useBriefing';

export interface BriefingCardProps {
    userId: string;
}

export function BriefingCard({ userId }: BriefingCardProps) {
    const { briefing, loading, acknowledgeBriefing } = useBriefing(userId);

    if (loading || !briefing) {
        return <div className="h-64 bg-gray-100 animate-pulse rounded-lg" />;
    }

    const sections = [
        { title: 'Overnight Events', items: briefing.overnightEvents, icon: '🌙', color: 'blue' },
        { title: 'Risk Alerts', items: briefing.riskAlerts, icon: '⚠️', color: 'red' },
        { title: 'Medication Reminders', items: briefing.medicationReminders, icon: '💊', color: 'green' },
        { title: 'Behavior Plan Highlights', items: briefing.behaviorPlanHighlights, icon: '📋', color: 'yellow' }
    ];

    return (
        <div className="bg-white rounded-lg shadow-lg p-6 space-y-4">
            <div className="flex items-center justify-between">
                <h2 className="text-2xl font-bold">Pre-Shift Briefing</h2>
                <span className="text-sm text-gray-500">
                    {new Date(briefing.date).toLocaleDateString()}
                </span>
            </div>

            <div className="space-y-4">
                {sections.map(section => (
                    section.items.length > 0 && (
                        <div key={section.title} className="border rounded-lg p-4">
                            <h3 className="font-semibold text-lg mb-2 flex items-center">
                                <span className="mr-2">{section.icon}</span>
                                {section.title}
                            </h3>
                            <ul className="space-y-1">
                                {section.items.map((item, idx) => (
                                    <li key={idx} className="text-sm text-gray-700">• {item}</li>
                                ))}
                            </ul>
                        </div>
                    )
                ))}
            </div>

            <button
                onClick={acknowledgeBriefing}
                className="w-full py-3 bg-blue-600 text-white rounded-lg font-medium hover:bg-blue-700"
            >
                Acknowledge Briefing
            </button>
        </div>
    );
}

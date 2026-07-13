"use client";
import React, { useState } from 'react';
import { BehavioralTimeline } from '@/components/BehavioralTimeline';
import { InterventionSuggestion } from '@/components/InterventionSuggestion';
import { Tabs } from '@/components/ui/tabs';
import { Alert } from '@/components/ui/alert';

export default function BehavioralPage() {
    const [selectedIndividual, setSelectedIndividual] = useState('1');

    // Mock data
    const behavioralEvents = [
        {
            timestamp: new Date(Date.now() - 3600000).toISOString(),
            type: 'Agitation',
            severity: 'medium' as const,
            description: 'Individual showed signs of frustration during group activity',
            intervention: 'Provided quiet space and preferred activity'
        },
        {
            timestamp: new Date(Date.now() - 7200000).toISOString(),
            type: 'Positive Engagement',
            severity: 'low' as const,
            description: 'Actively participated in art therapy session'
        },
        {
            timestamp: new Date(Date.now() - 10800000).toISOString(),
            type: 'Verbal Outburst',
            severity: 'high' as const,
            description: 'Raised voice when requested to transition activities',
            intervention: 'Used de-escalation techniques, offered choice'
        }
    ];

    const interventions = [
        {
            id: '1',
            title: 'Sensory Break',
            description: 'Provide access to sensory room for 15 minutes',
            effectiveness: 87,
            confidence: 92,
            steps: [
                'Guide individual to sensory room',
                'Allow 10-15 minutes of quiet time',
                'Offer preferred sensory items (weighted blanket, fidget toys)',
                'Check in after 10 minutes',
                'Gradually transition back to activity'
            ]
        },
        {
            id: '2',
            title: 'Choice Board',
            description: 'Present visual choice board with 3 preferred activities',
            effectiveness: 78,
            confidence: 85,
            steps: [
                'Show choice board with pictures',
                'Allow individual to select preferred activity',
                'Set timer for activity duration',
                'Provide 5-minute warning before transition',
                'Praise for making choice'
            ]
        }
    ];

    const tabs = [
        {
            id: 'timeline',
            label: 'Behavioral Timeline',
            content: (
                <div>
                    <Alert variant="info" className="mb-4">
                        Showing behavioral events from the last 24 hours
                    </Alert>
                    <BehavioralTimeline events={behavioralEvents} />
                </div>
            )
        },
        {
            id: 'interventions',
            label: 'AI Suggestions',
            content: (
                <div className="space-y-4">
                    <Alert variant="info" className="mb-4">
                        AI-powered intervention suggestions based on behavioral patterns
                    </Alert>
                    {interventions.map(intervention => (
                        <InterventionSuggestion
                            key={intervention.id}
                            intervention={intervention}
                            onAccept={(id) => console.log('Accepted:', id)}
                            onDismiss={(id) => console.log('Dismissed:', id)}
                        />
                    ))}
                </div>
            )
        },
        {
            id: 'patterns',
            label: 'Patterns & Trends',
            content: (
                <div className="bg-white p-6 rounded-lg">
                    <h3 className="font-bold text-lg mb-4">Behavioral Patterns (Last 7 Days)</h3>
                    <div className="space-y-4">
                        <div className="flex items-center justify-between p-4 bg-yellow-50 rounded border border-yellow-200">
                            <div>
                                <h4 className="font-semibold">Trigger Identified: Transitions</h4>
                                <p className="text-sm text-gray-600 mt-1">65% of behavioral events occur during activity transitions</p>
                            </div>
                            <span className="text-2xl">⚠️</span>
                        </div>
                        <div className="flex items-center justify-between p-4 bg-green-50 rounded border border-green-200">
                            <div>
                                <h4 className="font-semibold">Effective Intervention: Sensory Breaks</h4>
                                <p className="text-sm text-gray-600 mt-1">92% success rate when provided before transitions</p>
                            </div>
                            <span className="text-2xl">✓</span>
                        </div>
                        <div className="flex items-center justify-between p-4 bg-blue-50 rounded border border-blue-200">
                            <div>
                                <h4 className="font-semibold">Best Time of Day: Morning (8-11 AM)</h4>
                                <p className="text-sm text-gray-600 mt-1">Highest engagement and lowest behavioral incidents</p>
                            </div>
                            <span className="text-2xl">📊</span>
                        </div>
                    </div>
                </div>
            )
        }
    ];

    return (
        <div className="min-h-screen bg-gray-100 p-6">
            <div className="max-w-6xl mx-auto">
                <h1 className="text-3xl font-bold text-gray-900 mb-6">Behavioral Analysis</h1>

                <Tabs tabs={tabs} default Tab="timeline" />
            </div>
        </div>
    );
}

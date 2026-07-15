'use client';

import React from 'react';
import { Button } from '@/components/ui/button';
import { Card } from '@/components/ui/card';

/**
 * ReportTemplates Component
 * 
 * Displays available report templates for quick selection.
 */
interface ReportTemplatesProps {
    onSelectTemplate: (template: string) => void;
}

export function ReportTemplates({ onSelectTemplate }: ReportTemplatesProps) {
    const templates = [
        {
            id: 'compliance_scorecard',
            name: 'Compliance Scorecard',
            description: 'Comprehensive compliance metrics and scores',
            icon: '✅',
        },
        {
            id: 'behavioral_analysis',
            name: 'Behavioral Analysis',
            description: 'Behavioral patterns and intervention effectiveness',
            icon: '📊',
        },
        {
            id: 'medication_administration',
            name: 'Medication Administration',
            description: 'MAR compliance and medication safety metrics',
            icon: '💊',
        },
        {
            id: 'incident_analysis',
            name: 'Incident Analysis',
            description: 'Incident trends, patterns, and root cause analysis',
            icon: '⚠️',
        },
        {
            id: 'staffing_analysis',
            name: 'Staffing Analysis',
            description: 'Staffing levels, coverage, and utilization',
            icon: '👥',
        },
        {
            id: 'quality_metrics',
            name: 'Quality Metrics',
            description: 'Overall quality scores and outcome measures',
            icon: '⭐',
        },
    ];

    return (
        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
            {templates.map((template) => (
                <Card
                    key={template.id}
                    className="p-4 hover:shadow-lg transition-shadow cursor-pointer"
                    onClick={() => onSelectTemplate(template.id)}
                >
                    <div className="flex items-start space-x-3">
                        <div className="text-2xl">{template.icon}</div>
                        <div className="flex-1">
                            <h3 className="font-semibold text-gray-800 mb-1">{template.name}</h3>
                            <p className="text-sm text-gray-600 mb-3">{template.description}</p>
                            <Button variant="outline" size="sm">
                                Use Template →
                            </Button>
                        </div>
                    </div>
                </Card>
            ))}
        </div>
    );
}



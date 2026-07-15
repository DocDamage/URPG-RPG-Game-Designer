"use client";
import React from 'react';
import { Card } from '@/components/ui/card';
import { Badge } from '@/components/ui/badge';
import { Alert } from '@/components/ui/alert';
import { RiskIndicator } from '@/components/RiskIndicator';

interface RiskAlert {
    id: string;
    individualId: string;
    individualName?: string;
    riskLevel: 'low' | 'medium' | 'high' | 'critical';
    riskType: 'behavioral' | 'medical' | 'safety' | 'environmental';
    description: string;
    triggers: string[];
    recommendedActions: string[];
    lastUpdated: string;
}

interface RiskAlertsProps {
    alerts: RiskAlert[];
}

export function RiskAlerts({ alerts }: RiskAlertsProps) {
    const riskColors = {
        low: 'info' as const,
        medium: 'warning' as const,
        high: 'error' as const,
        critical: 'error' as const
    };

    const criticalAlerts = alerts.filter(a => a.riskLevel === 'critical' || a.riskLevel === 'high');
    const otherAlerts = alerts.filter(a => a.riskLevel !== 'critical' && a.riskLevel !== 'high');

    if (alerts.length === 0) {
        return (
            <Card className="p-6">
                <h3 className="text-lg font-semibold text-gray-900 mb-4">Risk Alerts</h3>
                <div className="text-center text-gray-500 py-4">
                    <p>✓ No active risk alerts</p>
                    <p className="text-sm mt-2">All individuals are at baseline risk levels</p>
                </div>
            </Card>
        );
    }

    return (
        <div className="space-y-4">
            {criticalAlerts.length > 0 && (
                <Card className="p-6 border-red-300 bg-red-50">
                    <div className="flex items-center justify-between mb-4">
                        <h3 className="text-lg font-semibold text-red-900">🚨 Critical Risk Alerts</h3>
                        <Badge variant="error">{criticalAlerts.length}</Badge>
                    </div>
                    <div className="space-y-4">
                        {criticalAlerts.map((alert) => (
                            <div key={alert.id} className="bg-white p-4 rounded border border-red-200">
                                <div className="flex items-start justify-between mb-2">
                                    <div className="flex items-center gap-2">
                                        <RiskIndicator level={alert.riskLevel} />
                                        <div>
                                            <p className="font-medium text-gray-900">
                                                {alert.individualName || `Individual ${alert.individualId}`}
                                            </p>
                                            <p className="text-xs text-gray-600">ID: {alert.individualId}</p>
                                        </div>
                                    </div>
                                    <Badge variant={riskColors[alert.riskLevel]}>
                                        {alert.riskLevel}
                                    </Badge>
                                </div>
                                <p className="text-sm text-gray-700 mb-2">{alert.description}</p>
                                {alert.triggers.length > 0 && (
                                    <div className="mb-2">
                                        <p className="text-xs font-medium text-gray-700 mb-1">Triggers:</p>
                                        <ul className="list-disc list-inside text-xs text-gray-600">
                                            {alert.triggers.map((trigger, idx) => (
                                                <li key={idx}>{trigger}</li>
                                            ))}
                                        </ul>
                                    </div>
                                )}
                                {alert.recommendedActions.length > 0 && (
                                    <div className="bg-blue-50 p-2 rounded">
                                        <p className="text-xs font-medium text-blue-900 mb-1">Recommended Actions:</p>
                                        <ul className="list-disc list-inside text-xs text-blue-800">
                                            {alert.recommendedActions.map((action, idx) => (
                                                <li key={idx}>{action}</li>
                                            ))}
                                        </ul>
                                    </div>
                                )}
                            </div>
                        ))}
                    </div>
                </Card>
            )}

            {otherAlerts.length > 0 && (
                <Card className="p-6">
                    <h3 className="text-lg font-semibold text-gray-900 mb-4">Risk Alerts</h3>
                    <div className="space-y-3">
                        {otherAlerts.map((alert) => (
                            <div key={alert.id} className="p-3 bg-gray-50 rounded border">
                                <div className="flex items-center justify-between mb-2">
                                    <div className="flex items-center gap-2">
                                        <RiskIndicator level={alert.riskLevel} />
                                        <span className="font-medium text-gray-900">
                                            {alert.individualName || `Individual ${alert.individualId}`}
                                        </span>
                                    </div>
                                    <Badge variant={riskColors[alert.riskLevel]}>
                                        {alert.riskType}
                                    </Badge>
                                </div>
                                <p className="text-sm text-gray-700">{alert.description}</p>
                            </div>
                        ))}
                    </div>
                </Card>
            )}

            <Alert variant="warning" title="Important">
                Please review all risk alerts before interacting with individuals. Follow recommended actions and behavior support plans.
            </Alert>
        </div>
    );
}


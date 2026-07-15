"use client";
import React, { useState } from 'react';
import { supervisorAPI } from '@/lib/api/supervisor';
import { Card } from '@/components/ui/card';
import { Badge } from '@/components/ui/badge';
import { Button } from '@/components/ui/button';
import { Alert } from '@/components/ui/alert';
import { Textarea } from '@/components/ui/textarea';

interface Alert {
    id: string;
    type: string;
    severity: 'info' | 'low' | 'medium' | 'high' | 'critical' | 'emergency';
    priority: number;
    title: string;
    description: string;
    requiresAction: boolean;
    suggestedActions?: string[];
    status: 'new' | 'acknowledged' | 'in_progress' | 'resolved' | 'dismissed';
    createdAt: string;
    acknowledgedBy?: string;
    resolvedBy?: string;
}

interface RealTimeAlertsProps {
    alerts: Alert[];
}

export function RealTimeAlerts({ alerts: initialAlerts }: RealTimeAlertsProps) {
    const [alerts, setAlerts] = useState<Alert[]>(initialAlerts);
    const [selectedAlert, setSelectedAlert] = useState<Alert | null>(null);
    const [resolution, setResolution] = useState('');
    const [filter, setFilter] = useState<'all' | 'new' | 'acknowledged' | 'in_progress'>('all');

    const filteredAlerts = alerts.filter(alert => {
        if (filter === 'all') return true;
        return alert.status === filter;
    });

    const severityColors = {
        info: 'info' as const,
        low: 'info' as const,
        medium: 'warning' as const,
        high: 'error' as const,
        critical: 'error' as const,
        emergency: 'error' as const
    };

    const handleAcknowledge = async (alertId: string) => {
        try {
            await supervisorAPI.acknowledgeAlert(alertId, 'current_user');
            setAlerts(alerts.map(a => 
                a.id === alertId 
                    ? { ...a, status: 'acknowledged' as const, acknowledgedBy: 'current_user' }
                    : a
            ));
        } catch (error) {
            console.error('Failed to acknowledge alert:', error);
        }
    };

    const handleResolve = async (alertId: string) => {
        if (!resolution.trim()) {
            alert('Please provide a resolution');
            return;
        }

        try {
            await supervisorAPI.resolveAlert(alertId, 'current_user', resolution);
            setAlerts(alerts.map(a => 
                a.id === alertId 
                    ? { ...a, status: 'resolved' as const, resolvedBy: 'current_user' }
                    : a
            ));
            setSelectedAlert(null);
            setResolution('');
        } catch (error) {
            console.error('Failed to resolve alert:', error);
        }
    };

    const criticalAlerts = filteredAlerts.filter(a => a.severity === 'critical' || a.severity === 'emergency');
    const otherAlerts = filteredAlerts.filter(a => a.severity !== 'critical' && a.severity !== 'emergency');

    return (
        <div className="space-y-6">
            <div className="flex items-center justify-between">
                <h2 className="text-xl font-semibold text-gray-900">Real-Time Alerts</h2>
                <select
                    value={filter}
                    onChange={(e) => setFilter(e.target.value as any)}
                    className="px-3 py-2 border rounded-lg text-sm"
                >
                    <option value="all">All Alerts</option>
                    <option value="new">New</option>
                    <option value="acknowledged">Acknowledged</option>
                    <option value="in_progress">In Progress</option>
                </select>
            </div>

            {criticalAlerts.length > 0 && (
                <Card className="p-6 border-red-300 bg-red-50">
                    <h3 className="text-lg font-semibold text-red-900 mb-4">🚨 Critical Alerts</h3>
                    <div className="space-y-3">
                        {criticalAlerts.map((alert) => (
                            <div key={alert.id} className="bg-white p-4 rounded border border-red-200">
                                <div className="flex items-start justify-between mb-2">
                                    <div className="flex-1">
                                        <div className="flex items-center gap-2 mb-1">
                                            <Badge variant={severityColors[alert.severity]}>
                                                {alert.severity}
                                            </Badge>
                                            <span className="font-semibold text-gray-900">{alert.title}</span>
                                        </div>
                                        <p className="text-sm text-gray-700">{alert.description}</p>
                                        {alert.suggestedActions && alert.suggestedActions.length > 0 && (
                                            <div className="mt-2 p-2 bg-blue-50 rounded">
                                                <p className="text-xs font-medium text-blue-900 mb-1">Suggested Actions:</p>
                                                <ul className="list-disc list-inside text-xs text-blue-800">
                                                    {alert.suggestedActions.map((action, idx) => (
                                                        <li key={idx}>{action}</li>
                                                    ))}
                                                </ul>
                                            </div>
                                        )}
                                    </div>
                                    <div className="ml-4 flex gap-2">
                                        {alert.status === 'new' && (
                                            <Button
                                                onClick={() => handleAcknowledge(alert.id)}
                                                className="bg-blue-600 hover:bg-blue-700"
                                            >
                                                Acknowledge
                                            </Button>
                                        )}
                                        <Button
                                            onClick={() => setSelectedAlert(alert)}
                                            variant="error"
                                        >
                                            Resolve
                                        </Button>
                                    </div>
                                </div>
                                <p className="text-xs text-gray-500 mt-2">
                                    Created: {new Date(alert.createdAt).toLocaleString()}
                                </p>
                            </div>
                        ))}
                    </div>
                </Card>
            )}

            {otherAlerts.length > 0 && (
                <Card className="p-6">
                    <h3 className="text-lg font-semibold text-gray-900 mb-4">Alerts</h3>
                    <div className="space-y-3">
                        {otherAlerts.map((alert) => (
                            <div key={alert.id} className="p-4 bg-gray-50 rounded border">
                                <div className="flex items-start justify-between">
                                    <div className="flex-1">
                                        <div className="flex items-center gap-2 mb-1">
                                            <Badge variant={severityColors[alert.severity]}>
                                                {alert.severity}
                                            </Badge>
                                            <span className="font-medium text-gray-900">{alert.title}</span>
                                            <Badge variant={alert.status === 'new' ? 'warning' : 'info'}>
                                                {alert.status}
                                            </Badge>
                                        </div>
                                        <p className="text-sm text-gray-700">{alert.description}</p>
                                    </div>
                                    <div className="ml-4 flex gap-2">
                                        {alert.status === 'new' && (
                                            <Button
                                                onClick={() => handleAcknowledge(alert.id)}
                                                size="sm"
                                            >
                                                Acknowledge
                                            </Button>
                                        )}
                                        {alert.requiresAction && (
                                            <Button
                                                onClick={() => setSelectedAlert(alert)}
                                                size="sm"
                                                variant="error"
                                            >
                                                Resolve
                                            </Button>
                                        )}
                                    </div>
                                </div>
                            </div>
                        ))}
                    </div>
                </Card>
            )}

            {filteredAlerts.length === 0 && (
                <Card className="p-6 text-center text-gray-500">
                    No alerts found for the selected filter
                </Card>
            )}

            {/* Resolution Dialog */}
            {selectedAlert && (
                <Card className="p-6 border-blue-300 bg-blue-50">
                    <h3 className="text-lg font-semibold text-gray-900 mb-4">Resolve Alert: {selectedAlert.title}</h3>
                    <div className="space-y-4">
                        <div>
                            <label className="block text-sm font-medium text-gray-700 mb-2">
                                Resolution Notes
                            </label>
                            <Textarea
                                value={resolution}
                                onChange={(e) => setResolution(e.target.value)}
                                rows={4}
                                placeholder="Describe how this alert was resolved..."
                            />
                        </div>
                        <div className="flex gap-3">
                            <Button
                                onClick={() => handleResolve(selectedAlert.id)}
                                className="flex-1"
                            >
                                Resolve Alert
                            </Button>
                            <Button
                                onClick={() => {
                                    setSelectedAlert(null);
                                    setResolution('');
                                }}
                                variant="secondary"
                            >
                                Cancel
                            </Button>
                        </div>
                    </div>
                </Card>
            )}
        </div>
    );
}


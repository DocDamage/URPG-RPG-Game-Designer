"use client";
import React from 'react';
import { Alert } from '@/components/ui/alert';
import { Badge } from '@/components/ui/badge';
import { Card } from '@/components/ui/card';

interface SafetyWarning {
    type: 'interaction' | 'allergy' | 'dosage' | 'timing' | 'duplicate';
    severity: 'low' | 'medium' | 'high' | 'critical';
    message: string;
    medication?: string;
}

interface SafetyWarningsProps {
    warnings: SafetyWarning[];
    onDismiss?: () => void;
}

export function SafetyWarnings({ warnings, onDismiss }: SafetyWarningsProps) {
    if (!warnings || warnings.length === 0) {
        return null;
    }

    const severityColors = {
        low: 'info' as const,
        medium: 'warning' as const,
        high: 'warning' as const,
        critical: 'error' as const
    };

    const typeIcons = {
        interaction: '⚗️',
        allergy: '⚠️',
        dosage: '📊',
        timing: '⏰',
        duplicate: '🔄'
    };

    const criticalWarnings = warnings.filter(w => w.severity === 'critical');
    const otherWarnings = warnings.filter(w => w.severity !== 'critical');

    return (
        <div className="space-y-4">
            {criticalWarnings.length > 0 && (
                <Card className="border-red-300 bg-red-50">
                    <div className="p-4">
                        <div className="flex items-center justify-between mb-3">
                            <h3 className="text-lg font-semibold text-red-900 flex items-center gap-2">
                                <span>🚨</span>
                                Critical Safety Warnings
                            </h3>
                            {onDismiss && (
                                <button
                                    onClick={onDismiss}
                                    className="text-red-700 hover:text-red-900 text-sm"
                                >
                                    Dismiss
                                </button>
                            )}
                        </div>
                        <div className="space-y-2">
                            {criticalWarnings.map((warning, idx) => (
                                <Alert
                                    key={idx}
                                    variant="error"
                                    title={`${typeIcons[warning.type]} ${warning.type.toUpperCase()}`}
                                >
                                    <div className="space-y-1">
                                        <p className="font-medium">{warning.message}</p>
                                        {warning.medication && (
                                            <p className="text-sm text-gray-600">
                                                Medication: {warning.medication}
                                            </p>
                                        )}
                                    </div>
                                </Alert>
                            ))}
                        </div>
                    </div>
                </Card>
            )}

            {otherWarnings.length > 0 && (
                <Card className="border-yellow-300 bg-yellow-50">
                    <div className="p-4">
                        <div className="flex items-center justify-between mb-3">
                            <h3 className="text-lg font-semibold text-yellow-900 flex items-center gap-2">
                                <span>⚠️</span>
                                Safety Warnings
                            </h3>
                            {onDismiss && (
                                <button
                                    onClick={onDismiss}
                                    className="text-yellow-700 hover:text-yellow-900 text-sm"
                                >
                                    Dismiss
                                </button>
                            )}
                        </div>
                        <div className="space-y-2">
                            {otherWarnings.map((warning, idx) => (
                                <div
                                    key={idx}
                                    className="flex items-start gap-3 p-3 bg-white rounded border border-yellow-200"
                                >
                                    <Badge variant={severityColors[warning.severity]}>
                                        {warning.severity}
                                    </Badge>
                                    <div className="flex-1">
                                        <p className="text-sm font-medium text-gray-900">
                                            {typeIcons[warning.type]} {warning.message}
                                        </p>
                                        {warning.medication && (
                                            <p className="text-xs text-gray-600 mt-1">
                                                Medication: {warning.medication}
                                            </p>
                                        )}
                                    </div>
                                </div>
                            ))}
                        </div>
                    </div>
                </Card>
            )}
        </div>
    );
}


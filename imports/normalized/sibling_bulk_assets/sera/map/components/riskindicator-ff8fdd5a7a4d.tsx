import React from 'react';
import { Badge } from './ui/badge';
import { useRiskState, type RiskLevel } from '../lib/hooks/useRiskState';

export interface RiskIndicatorProps {
    individualId: string;
    showDetails?: boolean;
}

export function RiskIndicator({ individualId, showDetails = false }: RiskIndicatorProps) {
    const { riskState, loading, getRiskColor } = useRiskState(individualId);

    if (loading || !riskState) {
        return <div className="h-6 w-20 bg-gray-200 animate-pulse rounded-full" />;
    }

    const colorMap: Record<RiskLevel, 'default' | 'success' | 'warning' | 'danger'> = {
        low: 'success',
        medium: 'warning',
        high: 'danger',
        critical: 'danger'
    };

    return (
        <div className="flex items-center space-x-2">
            <Badge variant={colorMap[riskState.level]} size="md">
                {riskState.level.toUpperCase()} RISK
            </Badge>
            {showDetails && (
                <span className="text-sm text-gray-600">
                    Score: {riskState.score}/100
                </span>
            )}
            {riskState.level === 'critical' && (
                <span className="ml-2 animate-pulse text-red-600">⚠️</span>
            )}
        </div>
    );
}

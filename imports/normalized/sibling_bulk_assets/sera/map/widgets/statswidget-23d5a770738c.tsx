/**
 * Stats Widget
 * 
 * Displays key metrics with trend indicators.
 */

'use client';

import React, { useMemo } from 'react';
import { clsx } from 'clsx';
import {
  TrendingUp,
  TrendingDown,
  Minus,
  Users,
  UserCheck,
  AlertTriangle,
  Pill,
  Heart,
  Building2,
  ShieldCheck,
  Activity,
  Smile,
  Zap,
  Clock,
  AlertCircle,
} from 'lucide-react';
import type { WidgetProps, StatsWidgetConfig, StatsData } from '../types';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';

// Icon mapping
const iconMap: Record<string, React.ComponentType<{ className?: string }>> = {
  Users,
  UserCheck,
  AlertTriangle,
  Pill,
  Heart,
  Building2,
  ShieldCheck,
  Activity,
  Smile,
  Zap,
  Clock,
  AlertCircle,
  TrendingUp,
  TrendingDown,
  Minus,
};

// Format value based on format type
function formatValue(value: number, format?: string): string {
  switch (format) {
    case 'percentage':
      return `${value.toFixed(1)}%`;
    case 'currency':
      return `$${value.toLocaleString()}`;
    case 'duration':
      return `${Math.floor(value / 60)}h ${value % 60}m`;
    case 'number':
    default:
      return value.toLocaleString();
  }
}

// Get trend icon and color
function getTrendIndicator(change?: number) {
  if (change === undefined || change === 0) {
    return { icon: Minus, color: 'text-gray-500', label: 'No change' };
  }
  if (change > 0) {
    return { icon: TrendingUp, color: 'text-green-500', label: `+${change}%` };
  }
  return { icon: TrendingDown, color: 'text-red-500', label: `${change}%` };
}

// Mock data generator (replace with actual API calls)
function useStatsData(config: StatsWidgetConfig): { data: StatsData[]; loading: boolean } {
  // This would be replaced with actual API calls
  const data = useMemo(() => {
    return config.metrics.map((metric) => ({
      value: Math.floor(Math.random() * 100) + 10,
      previousValue: Math.floor(Math.random() * 100) + 10,
      change: metric.trend ? Math.floor(Math.random() * 20) - 10 : undefined,
      label: metric.label,
      icon: metric.icon,
      color: metric.color,
    }));
  }, [config.metrics]);

  return { data, loading: false };
}

export function StatsWidget({ config, isLoading, className }: WidgetProps) {
  const statsConfig = config as StatsWidgetConfig;
  const { data, loading } = useStatsData(statsConfig);

  if (isLoading || loading) {
    return (
      <Card className={clsx('h-full', className)}>
        <CardHeader className="pb-2">
          <div className="h-6 w-32 bg-gray-200 rounded animate-pulse" />
        </CardHeader>
        <CardContent>
          <div className="grid grid-cols-2 gap-4">
            {[1, 2, 3, 4].map((i) => (
              <div key={i} className="h-20 bg-gray-100 rounded-lg animate-pulse" />
            ))}
          </div>
        </CardContent>
      </Card>
    );
  }

  return (
    <Card className={clsx('h-full', className)}>
      {config.display?.showHeader !== false && (
        <CardHeader className="pb-2">
          <CardTitle className="text-lg font-semibold">
            {config.title || 'Statistics'}
          </CardTitle>
          {config.description && (
            <p className="text-sm text-gray-500">{config.description}</p>
          )}
        </CardHeader>
      )}
      <CardContent className={config.display?.showHeader === false ? 'pt-6' : ''}>
        <div className={clsx(
          'grid gap-4',
          data.length <= 2 ? 'grid-cols-1 sm:grid-cols-2' : 'grid-cols-2'
        )}>
          {data.map((stat, index) => {
            const IconComponent = stat.icon ? iconMap[stat.icon] || Activity : Activity;
            const trend = getTrendIndicator(stat.change);
            const TrendIcon = trend.icon;

            return (
              <div
                key={index}
                className={clsx(
                  'relative overflow-hidden rounded-lg border p-4',
                  'transition-all duration-200 hover:shadow-md',
                  config.display?.compact ? 'p-3' : 'p-4'
                )}
                style={{ borderColor: stat.color ? `${stat.color}30` : undefined }}
              >
                {/* Background color hint */}
                {stat.color && (
                  <div
                    className="absolute inset-0 opacity-5"
                    style={{ backgroundColor: stat.color }}
                  />
                )}

                <div className="relative flex items-start justify-between">
                  <div className="flex-1 min-w-0">
                    <p className={clsx(
                      'text-sm font-medium text-gray-600 truncate',
                      config.display?.compact && 'text-xs'
                    )}>
                      {stat.label}
                    </p>
                    <p className={clsx(
                      'mt-1 font-bold text-gray-900',
                      config.display?.compact ? 'text-xl' : 'text-2xl'
                    )}>
                      {formatValue(stat.value, statsConfig.metrics[index]?.format)}
                    </p>
                    
                    {stat.change !== undefined && (
                      <div className={clsx(
                        'mt-1 flex items-center gap-1 text-xs',
                        trend.color
                      )}>
                        <TrendIcon className="h-3 w-3" />
                        <span>{trend.label}</span>
                      </div>
                    )}
                  </div>

                  {stat.color && (
                    <div
                      className="p-2 rounded-lg"
                      style={{ backgroundColor: `${stat.color}15` }}
                    >
                      <IconComponent
                        className="h-5 w-5"
                        style={{ color: stat.color }}
                      />
                    </div>
                  )}
                </div>
              </div>
            );
          })}
        </div>
      </CardContent>
    </Card>
  );
}

export default StatsWidget;

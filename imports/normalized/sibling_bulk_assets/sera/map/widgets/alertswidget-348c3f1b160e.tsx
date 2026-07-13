/**
 * Alerts Widget
 * 
 * Important alerts and notifications requiring attention.
 */

'use client';

import React, { useState, useCallback, useEffect } from 'react';
import { clsx } from 'clsx';
import { formatDistanceToNow } from 'date-fns';
import {
  Bell,
  AlertTriangle,
  AlertCircle,
  Info,
  X,
  CheckCircle2,
  ChevronRight,
  ExternalLink,
} from 'lucide-react';
import type { WidgetProps, AlertsWidgetConfig, AlertItem } from '../types';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Badge } from '@/components/ui/badge';
import { ScrollArea } from '@/components/ui/scroll-area';
import { cn } from '@/lib/utils';

// Severity configuration
const severityConfig = {
  info: {
    icon: Info,
    color: 'text-blue-500',
    bgColor: 'bg-blue-50',
    borderColor: 'border-blue-200',
    label: 'Info',
  },
  warning: {
    icon: AlertTriangle,
    color: 'text-yellow-500',
    bgColor: 'bg-yellow-50',
    borderColor: 'border-yellow-200',
    label: 'Warning',
  },
  error: {
    icon: AlertCircle,
    color: 'text-red-500',
    bgColor: 'bg-red-50',
    borderColor: 'border-red-200',
    label: 'Error',
  },
  critical: {
    icon: AlertTriangle,
    color: 'text-red-600',
    bgColor: 'bg-red-100',
    borderColor: 'border-red-300',
    label: 'Critical',
  },
};

// Mock alerts
const mockAlerts: AlertItem[] = [
  {
    id: '1',
    title: 'Medication overdue',
    message: 'Morning medication for John Doe is 30 minutes overdue',
    severity: 'critical',
    category: 'medication',
    timestamp: new Date(Date.now() - 1000 * 60 * 30).toISOString(),
    acknowledged: false,
    entityRef: { type: 'individual', id: '1', name: 'John Doe' },
  },
  {
    id: '2',
    title: 'Shift coverage needed',
    message: 'Evening shift on Friday requires additional staff',
    severity: 'warning',
    category: 'scheduling',
    timestamp: new Date(Date.now() - 1000 * 60 * 60).toISOString(),
    acknowledged: false,
  },
  {
    id: '3',
    title: 'PRN request pending',
    message: 'New PRN medication request awaiting nurse approval',
    severity: 'warning',
    category: 'medication',
    timestamp: new Date(Date.now() - 1000 * 60 * 45).toISOString(),
    acknowledged: false,
    entityRef: { type: 'individual', id: '2', name: 'Sarah Wilson' },
  },
  {
    id: '4',
    title: 'Document expiring',
    message: 'Annual care plan review due within 7 days',
    severity: 'info',
    category: 'compliance',
    timestamp: new Date(Date.now() - 1000 * 60 * 60 * 2).toISOString(),
    acknowledged: false,
  },
  {
    id: '5',
    title: 'New training assigned',
    message: 'HIPAA refresher course has been assigned to you',
    severity: 'info',
    category: 'training',
    timestamp: new Date(Date.now() - 1000 * 60 * 60 * 4).toISOString(),
    acknowledged: true,
  },
];

export function AlertsWidget({ config, className }: WidgetProps) {
  const alertsConfig = config as AlertsWidgetConfig;
  const [alerts, setAlerts] = useState<AlertItem[]>(mockAlerts);
  const [dismissingIds, setDismissingIds] = useState<string[]>([]);

  // Filter alerts based on config
  const filteredAlerts = alerts
    .filter((alert) => {
      if (alertsConfig.severity?.length) {
        return alertsConfig.severity.includes(alert.severity);
      }
      return true;
    })
    .filter((alert) => !alert.acknowledged)
    .slice(0, alertsConfig.maxAlerts || 10);

  // Auto-dismiss functionality
  useEffect(() => {
    if (!alertsConfig.autoDismiss) return;

    const delay = (alertsConfig.dismissDelay || 5) * 1000;
    
    filteredAlerts.forEach((alert) => {
      setTimeout(() => {
        acknowledgeAlert(alert.id);
      }, delay);
    });
  }, [filteredAlerts, alertsConfig.autoDismiss, alertsConfig.dismissDelay]);

  const acknowledgeAlert = useCallback((id: string) => {
    setDismissingIds((prev) => [...prev, id]);
    
    setTimeout(() => {
      setAlerts((prev) =>
        prev.map((alert) =>
          alert.id === id ? { ...alert, acknowledged: true } : alert
        )
      );
      setDismissingIds((prev) => prev.filter((i) => i !== id));
    }, 300);
  }, []);

  const dismissAll = useCallback(() => {
    filteredAlerts.forEach((alert) => acknowledgeAlert(alert.id));
  }, [filteredAlerts, acknowledgeAlert]);

  const criticalCount = filteredAlerts.filter((a) => a.severity === 'critical').length;
  const warningCount = filteredAlerts.filter((a) => a.severity === 'warning').length;

  if (filteredAlerts.length === 0) {
    return (
      <Card className={clsx('h-full', className)}>
        <CardHeader className="pb-3">
          <CardTitle className="text-lg font-semibold flex items-center gap-2">
            <Bell className="h-5 w-5" />
            {config.title || 'Alerts'}
          </CardTitle>
        </CardHeader>
        <CardContent>
          <div className="flex flex-col items-center justify-center h-32 text-gray-400">
            <CheckCircle2 className="h-10 w-10 mb-2 text-green-500" />
            <p className="text-sm">All caught up!</p>
            <p className="text-xs">No active alerts</p>
          </div>
        </CardContent>
      </Card>
    );
  }

  return (
    <Card className={clsx('h-full flex flex-col', className)}>
      <CardHeader className="pb-3">
        <div className="flex items-center justify-between">
          <CardTitle className="text-lg font-semibold flex items-center gap-2">
            <Bell className="h-5 w-5" />
            {config.title || 'Alerts'}
            {filteredAlerts.length > 0 && (
              <Badge
                variant={criticalCount > 0 ? 'destructive' : warningCount > 0 ? 'default' : 'secondary'}
                className="text-xs"
              >
                {filteredAlerts.length}
              </Badge>
            )}
          </CardTitle>

          {filteredAlerts.length > 1 && (
            <Button variant="ghost" size="sm" onClick={dismissAll}>
              Dismiss all
            </Button>
          )}
        </div>
      </CardHeader>

      <CardContent className="flex-1 p-0">
        <ScrollArea className="h-full px-4">
          <div className="space-y-2 pb-4">
            {filteredAlerts.map((alert) => {
              const config = severityConfig[alert.severity];
              const Icon = config.icon;
              const isDismissing = dismissingIds.includes(alert.id);

              return (
                <div
                  key={alert.id}
                  className={cn(
                    'relative p-3 rounded-lg border transition-all duration-300',
                    config.bgColor,
                    config.borderColor,
                    isDismissing && 'opacity-0 transform translate-x-full'
                  )}
                >
                  <div className="flex items-start gap-3">
                    <div className={cn('mt-0.5', config.color)}>
                      <Icon className="h-5 w-5" />
                    </div>

                    <div className="flex-1 min-w-0">
                      <div className="flex items-start justify-between gap-2">
                        <div>
                          <p className="text-sm font-medium text-gray-900">
                            {alert.title}
                          </p>
                          <p className="text-xs text-gray-600 mt-0.5">
                            {alert.message}
                          </p>

                          <div className="flex items-center gap-2 mt-2">
                            <Badge
                              variant="ghost"
                              className={cn('text-[10px] px-1.5', config.color)}
                            >
                              {config.label}
                            </Badge>
                            <span className="text-xs text-gray-400">
                              {formatDistanceToNow(new Date(alert.timestamp), {
                                addSuffix: true,
                              })}
                            </span>
                          </div>
                        </div>

                        <div className="flex items-center gap-1">
                          {alert.entityRef?.url && (
                            <Button
                              variant="ghost"
                              size="icon"
                              className="h-7 w-7"
                              asChild
                            >
                              <a href={alert.entityRef.url}>
                                <ExternalLink className="h-3.5 w-3.5" />
                              </a>
                            </Button>
                          )}
                          <Button
                            variant="ghost"
                            size="icon"
                            className="h-7 w-7"
                            onClick={() => acknowledgeAlert(alert.id)}
                          >
                            <X className="h-3.5 w-3.5" />
                          </Button>
                        </div>
                      </div>
                    </div>
                  </div>

                  {/* Pulse animation for critical alerts */}
                  {alert.severity === 'critical' && (
                    <span className="absolute top-2 right-2 h-2 w-2 rounded-full bg-red-500 animate-pulse" />
                  )}
                </div>
              );
            })}
          </div>

          <Button variant="ghost" className="w-full" asChild>
            <a href="/alerts">
              View All Alerts
              <ChevronRight className="h-4 w-4 ml-1" />
            </a>
          </Button>
        </ScrollArea>
      </CardContent>
    </Card>
  );
}

export default AlertsWidget;

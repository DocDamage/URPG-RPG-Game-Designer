/**
 * Incident Widget
 * 
 * Recent incidents summary with trends.
 */

'use client';

import React, { useMemo } from 'react';
import { clsx } from 'clsx';
import { format, parseISO, subDays, isAfter } from 'date-fns';
import {
  AlertTriangle,
  ChevronRight,
  TrendingUp,
  TrendingDown,
  Minus,
  FileText,
  Clock,
  User,
  MapPin,
} from 'lucide-react';
import type { WidgetProps, IncidentWidgetConfig, IncidentSummary } from '../types';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Badge } from '@/components/ui/badge';
import { ScrollArea } from '@/components/ui/scroll-area';

// Mock incidents
const mockIncidents: IncidentSummary[] = [
  {
    id: '1',
    type: 'Fall',
    severity: 'minor',
    status: 'under_review',
    occurredAt: new Date(Date.now() - 1000 * 60 * 60 * 2).toISOString(),
    individualName: 'John Doe',
    location: 'Common Area',
    description: 'Minor slip while walking, no injuries reported',
  },
  {
    id: '2',
    type: 'Medication Error',
    severity: 'moderate',
    status: 'investigation_pending',
    occurredAt: new Date(Date.now() - 1000 * 60 * 60 * 8).toISOString(),
    individualName: 'Sarah Wilson',
    location: 'Medication Room',
    description: 'Incorrect dosage administered, monitoring in progress',
  },
  {
    id: '3',
    type: 'Behavioral',
    severity: 'minor',
    status: 'closed',
    occurredAt: new Date(Date.now() - 1000 * 60 * 60 * 24).toISOString(),
    individualName: 'Mike Johnson',
    location: 'Bedroom 2',
    description: 'Verbal agitation during evening routine',
  },
  {
    id: '4',
    type: 'Property Damage',
    severity: 'minor',
    status: 'under_review',
    occurredAt: new Date(Date.now() - 1000 * 60 * 60 * 48).toISOString(),
    individualName: 'Emma Davis',
    location: 'Kitchen',
    description: 'Broken plate during meal preparation',
  },
  {
    id: '5',
    type: 'Injury',
    severity: 'serious',
    status: 'in_investigation',
    occurredAt: new Date(Date.now() - 1000 * 60 * 60 * 72).toISOString(),
    individualName: 'Tom Brown',
    location: 'Bathroom',
    description: 'Laceration requiring stitches',
  },
];

// Severity configuration
const severityConfig = {
  minor: {
    color: 'text-yellow-600',
    bgColor: 'bg-yellow-50',
    borderColor: 'border-yellow-200',
    label: 'Minor',
  },
  moderate: {
    color: 'text-orange-600',
    bgColor: 'bg-orange-50',
    borderColor: 'border-orange-200',
    label: 'Moderate',
  },
  serious: {
    color: 'text-red-600',
    bgColor: 'bg-red-50',
    borderColor: 'border-red-200',
    label: 'Serious',
  },
  critical: {
    color: 'text-red-700',
    bgColor: 'bg-red-100',
    borderColor: 'border-red-300',
    label: 'Critical',
  },
  catastrophic: {
    color: 'text-purple-700',
    bgColor: 'bg-purple-100',
    borderColor: 'border-purple-300',
    label: 'Catastrophic',
  },
};

// Status configuration
const statusConfig: Record<string, { label: string; color: string }> = {
  draft: { label: 'Draft', color: 'text-gray-500' },
  submitted: { label: 'Submitted', color: 'text-blue-500' },
  under_review: { label: 'Under Review', color: 'text-yellow-500' },
  investigation_pending: { label: 'Investigation Pending', color: 'text-orange-500' },
  in_investigation: { label: 'In Investigation', color: 'text-red-500' },
  pending_closure: { label: 'Pending Closure', color: 'text-purple-500' },
  closed: { label: 'Closed', color: 'text-green-500' },
};

export function IncidentWidget({ config, className }: WidgetProps) {
  const incidentConfig = config as IncidentWidgetConfig;

  // Filter incidents based on time range
  const filteredIncidents = useMemo(() => {
    let filtered = mockIncidents;

    // Filter by time range
    if (incidentConfig.timeRange) {
      const now = new Date();
      let cutoffDate: Date;

      switch (incidentConfig.timeRange) {
        case '24h':
          cutoffDate = subDays(now, 1);
          break;
        case '7d':
          cutoffDate = subDays(now, 7);
          break;
        case '30d':
          cutoffDate = subDays(now, 30);
          break;
        default:
          cutoffDate = subDays(now, 7);
      }

      filtered = filtered.filter((i) =>
        isAfter(parseISO(i.occurredAt), cutoffDate)
      );
    }

    // Filter by status
    if (incidentConfig.status?.length) {
      filtered = filtered.filter((i) => incidentConfig.status?.includes(i.status));
    }

    // Filter by severity
    if (incidentConfig.severity?.length) {
      filtered = filtered.filter((i) => incidentConfig.severity?.includes(i.severity));
    }

    return filtered.slice(0, incidentConfig.limit || 5);
  }, [incidentConfig]);

  // Calculate trends (mock data)
  const trends = useMemo(() => {
    const total = filteredIncidents.length;
    const bySeverity = filteredIncidents.reduce((acc, i) => {
      acc[i.severity] = (acc[i.severity] || 0) + 1;
      return acc;
    }, {} as Record<string, number>);

    return { total, bySeverity };
  }, [filteredIncidents]);

  return (
    <Card className={clsx('h-full flex flex-col', className)}>
      <CardHeader className="pb-3">
        <div className="flex items-center justify-between">
          <CardTitle className="text-lg font-semibold flex items-center gap-2">
            <AlertTriangle className="h-5 w-5" />
            {config.title || 'Recent Incidents'}
          </CardTitle>

          {incidentConfig.showTrends && (
            <Badge variant="secondary" className="text-xs">
              {trends.total} total
            </Badge>
          )}
        </div>

        {/* Severity summary */}
        {incidentConfig.showTrends && (
          <div className="flex flex-wrap gap-1 mt-2">
            {Object.entries(trends.bySeverity).map(([severity, count]) => {
              const config = severityConfig[severity as keyof typeof severityConfig];
              return (
                <Badge
                  key={severity}
                  variant="outline"
                  className={clsx('text-[10px]', config?.color)}
                >
                  {count} {config?.label || severity}
                </Badge>
              );
            })}
          </div>
        )}
      </CardHeader>

      <CardContent className="flex-1 p-0">
        <ScrollArea className="h-full px-4">
          <div className="space-y-2 pb-4">
            {filteredIncidents.length === 0 ? (
              <div className="text-center py-8 text-gray-400">
                <AlertTriangle className="h-8 w-8 mx-auto mb-2 opacity-50" />
                <p className="text-sm">No incidents in selected period</p>
              </div>
            ) : (
              filteredIncidents.map((incident) => {
                const severity = severityConfig[incident.severity as keyof typeof severityConfig];
                const status = statusConfig[incident.status] || { label: incident.status, color: 'text-gray-500' };

                return (
                  <div
                    key={incident.id}
                    className={clsx(
                      'p-3 rounded-lg border transition-all hover:shadow-sm',
                      severity?.bgColor || 'bg-gray-50',
                      severity?.borderColor || 'border-gray-200'
                    )}
                  >
                    <div className="flex items-start justify-between gap-2">
                      <div className="flex-1 min-w-0">
                        <div className="flex items-center gap-2">
                          <span className="font-medium text-sm">
                            {incident.type}
                          </span>
                          <Badge
                            variant="ghost"
                            className={clsx('text-[10px] px-1.5', severity?.color)}
                          >
                            {severity?.label || incident.severity}
                          </Badge>
                        </div>

                        <p className="text-xs text-gray-600 mt-1 line-clamp-2">
                          {incident.description}
                        </p>

                        <div className="flex flex-wrap items-center gap-x-3 gap-y-1 mt-2 text-xs text-gray-500">
                          <span className="flex items-center gap-1">
                            <User className="h-3 w-3" />
                            {incident.individualName}
                          </span>
                          <span className="flex items-center gap-1">
                            <MapPin className="h-3 w-3" />
                            {incident.location}
                          </span>
                          <span className="flex items-center gap-1">
                            <Clock className="h-3 w-3" />
                            {format(parseISO(incident.occurredAt), 'MMM d, h:mm a')}
                          </span>
                        </div>

                        <div className="flex items-center gap-2 mt-2">
                          <Badge variant="outline" className={clsx('text-[10px]', status.color)}>
                            {status.label}
                          </Badge>
                        </div>
                      </div>
                    </div>
                  </div>
                );
              })
            )}
          </div>

          <Button variant="ghost" className="w-full" asChild>
            <a href="/incidents">
              View All Incidents
              <ChevronRight className="h-4 w-4 ml-1" />
            </a>
          </Button>
        </ScrollArea>
      </CardContent>
    </Card>
  );
}

export default IncidentWidget;

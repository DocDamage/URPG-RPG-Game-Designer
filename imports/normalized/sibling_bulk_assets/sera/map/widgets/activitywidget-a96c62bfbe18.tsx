/**
 * Activity Widget
 * 
 * Compact version of the activity feed for dashboard display.
 */

'use client';

import React, { useCallback, useEffect, useState } from 'react';
import Link from 'next/link';
import { clsx } from 'clsx';
import { formatDistanceToNow } from 'date-fns';
import {
  RefreshCw,
  ChevronRight,
  Bell,
  MoreHorizontal,
} from 'lucide-react';
import type { WidgetProps, ActivityWidgetConfig, Activity } from '../types';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Badge } from '@/components/ui/badge';
import { ScrollArea } from '@/components/ui/scroll-area';
import {
  Tooltip,
  TooltipContent,
  TooltipProvider,
  TooltipTrigger,
} from '@/components/ui/tooltip';
import { useActivity } from '@/lib/hooks/useActivity';
import { ActivityIcon } from '@/components/activity/ActivityIcon';
import type { ActivityType, ActivitySeverity } from '@/types/activity';

// Mock activities for when API is not available
const mockActivities: Activity[] = [
  {
    id: '1',
    type: 'medication_administered',
    title: 'Medication administered',
    description: 'John Doe received morning medications',
    severity: 'success',
    actor: { id: '1', name: 'Jane Smith', role: 'dsp' },
    organization_id: '1',
    privacy_level: 'internal',
    contains_phi: true,
    timestamp: new Date(Date.now() - 1000 * 60 * 5).toISOString(),
    created_at: new Date().toISOString(),
    entities: [{ type: 'individual', id: '1', name: 'John Doe' }],
  },
  {
    id: '2',
    type: 'incident_reported',
    title: 'Incident reported',
    description: 'Minor fall in common area - no injuries',
    severity: 'warning',
    actor: { id: '2', name: 'Bob Johnson', role: 'dsp' },
    organization_id: '1',
    privacy_level: 'restricted',
    contains_phi: false,
    timestamp: new Date(Date.now() - 1000 * 60 * 30).toISOString(),
    created_at: new Date().toISOString(),
    entities: [{ type: 'incident', id: '1', name: 'Fall Incident' }],
  },
  {
    id: '3',
    type: 'log_entry_created',
    title: 'Daily log updated',
    description: 'Progress notes added for afternoon activities',
    severity: 'info',
    actor: { id: '3', name: 'Alice Brown', role: 'lead_dsp' },
    organization_id: '1',
    privacy_level: 'internal',
    contains_phi: true,
    timestamp: new Date(Date.now() - 1000 * 60 * 60).toISOString(),
    created_at: new Date().toISOString(),
    entities: [{ type: 'individual', id: '2', name: 'Sarah Wilson' }],
  },
  {
    id: '4',
    type: 'shift_started',
    title: 'Shift started',
    description: 'Evening shift begun by day staff',
    severity: 'info',
    actor: { id: '4', name: 'Carol Davis', role: 'supervisor' },
    organization_id: '1',
    privacy_level: 'public',
    contains_phi: false,
    timestamp: new Date(Date.now() - 1000 * 60 * 60 * 2).toISOString(),
    created_at: new Date().toISOString(),
  },
  {
    id: '5',
    type: 'medication_refused',
    title: 'Medication refused',
    description: 'Patient declined evening medication',
    severity: 'warning',
    actor: { id: '1', name: 'Jane Smith', role: 'dsp' },
    organization_id: '1',
    privacy_level: 'restricted',
    contains_phi: true,
    timestamp: new Date(Date.now() - 1000 * 60 * 60 * 3).toISOString(),
    created_at: new Date().toISOString(),
    entities: [{ type: 'individual', id: '3', name: 'Mike Johnson' }],
  },
];

export function ActivityWidget({ config, className }: WidgetProps) {
  const activityConfig = config as ActivityWidgetConfig;
  const [expandedId, setExpandedId] = useState<string | null>(null);
  const [useMockData, setUseMockData] = useState(false);

  const {
    activities,
    loading,
    error,
    total,
    refresh,
  } = useActivity({
    initialFilter: {
      types: activityConfig.filterTypes as ActivityType[],
      severity: activityConfig.severity as ActivitySeverity[],
    },
    pageSize: activityConfig.limit || 5,
    enableRealtime: activityConfig.enableRealtime !== false,
    groupByDate: activityConfig.groupByDate !== false,
  });

  // Auto-refresh
  useEffect(() => {
    const intervalMinutes = parseInt(config.refreshInterval || '5');
    if (!intervalMinutes) return;

    const interval = setInterval(() => {
      refresh();
    }, intervalMinutes * 60 * 1000);

    return () => clearInterval(interval);
  }, [config.refreshInterval, refresh]);

  // Use mock data if API fails
  useEffect(() => {
    if (error && activities.length === 0) {
      setUseMockData(true);
    }
  }, [error, activities.length]);

  const displayActivities = useMockData
    ? mockActivities.slice(0, activityConfig.limit || 5)
    : activities.slice(0, activityConfig.limit || 5);

  const handleActivityClick = useCallback((activity: Activity) => {
    if (activity.entities?.[0]?.url) {
      window.location.href = activity.entities[0].url;
    }
  }, []);

  if (loading && displayActivities.length === 0) {
    return (
      <Card className={clsx('h-full', className)}>
        <CardHeader className="pb-3">
          <div className="h-6 w-32 bg-gray-200 rounded animate-pulse" />
        </CardHeader>
        <CardContent>
          <div className="space-y-3">
            {[1, 2, 3].map((i) => (
              <div key={i} className="h-16 bg-gray-100 rounded-lg animate-pulse" />
            ))}
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
            {config.title || 'Recent Activity'}
            {total > 0 && (
              <Badge variant="secondary" className="text-xs">
                {useMockData ? mockActivities.length : total}
              </Badge>
            )}
          </CardTitle>

          <TooltipProvider>
            <Tooltip>
              <TooltipTrigger asChild>
                <Button
                  variant="ghost"
                  size="icon"
                  onClick={refresh}
                  disabled={loading}
                  className="h-8 w-8"
                >
                  <RefreshCw className={clsx('h-4 w-4', loading && 'animate-spin')} />
                </Button>
              </TooltipTrigger>
              <TooltipContent>
                <p>Refresh</p>
              </TooltipContent>
            </Tooltip>
          </TooltipProvider>
        </div>
      </CardHeader>

      <CardContent className="flex-1 p-0">
        {displayActivities.length === 0 ? (
          <div className="flex flex-col items-center justify-center h-48 text-gray-400">
            <Bell className="h-8 w-8 mb-2 opacity-50" />
            <p className="text-sm">No recent activity</p>
          </div>
        ) : (
          <ScrollArea className="h-full px-6">
            <div className="space-y-1 pb-4">
              {displayActivities.map((activity, index) => (
                <ActivityItem
                  key={activity.id}
                  activity={activity}
                  isExpanded={expandedId === activity.id}
                  onToggle={() => setExpandedId(expandedId === activity.id ? null : activity.id)}
                  onClick={() => handleActivityClick(activity)}
                  isNew={index === 0 && activityConfig.enableRealtime}
                />
              ))}
            </div>

            {activityConfig.showViewAll !== false && (
              <div className="pt-2 pb-2 border-t">
                <Button variant="ghost" className="w-full" asChild>
                  <Link href="/activity">
                    View All Activity
                    <ChevronRight className="h-4 w-4 ml-1" />
                  </Link>
                </Button>
              </div>
            )}
          </ScrollArea>
        )}
      </CardContent>
    </Card>
  );
}

// Individual activity item
interface ActivityItemProps {
  activity: Activity;
  isExpanded: boolean;
  onToggle: () => void;
  onClick: () => void;
  isNew?: boolean;
}

function ActivityItem({ activity, isExpanded, onToggle, onClick, isNew }: ActivityItemProps) {
  const [isHovered, setIsHovered] = useState(false);

  const relativeTime = formatDistanceToNow(new Date(activity.timestamp), {
    addSuffix: true,
  });

  const primaryEntity = activity.entities?.find((e) => e.type === 'individual') || activity.entities?.[0];

  return (
    <div
      className={clsx(
        'group relative p-3 rounded-lg transition-colors cursor-pointer',
        'hover:bg-gray-50',
        activity.severity === 'critical' && 'bg-red-50/50 hover:bg-red-50',
        activity.severity === 'error' && 'bg-red-50/30 hover:bg-red-50/50',
        isNew && 'bg-blue-50/30'
      )}
      onMouseEnter={() => setIsHovered(true)}
      onMouseLeave={() => setIsHovered(false)}
      onClick={onClick}
    >
      <div className="flex items-start gap-3">
        <ActivityIcon type={activity.type} severity={activity.severity} size="sm" />

        <div className="flex-1 min-w-0">
          <div className="flex items-start justify-between gap-2">
            <div className="flex-1">
              <p className="text-sm font-medium text-gray-900 line-clamp-1">
                {activity.title}
              </p>

              {primaryEntity?.name && (
                <p className="text-xs text-gray-500">{primaryEntity.name}</p>
              )}

              {isExpanded && activity.description && (
                <p className="text-xs text-gray-600 mt-1">{activity.description}</p>
              )}

              <div className="flex items-center gap-2 mt-1">
                <time className="text-xs text-gray-400">{relativeTime}</time>
                {activity.contains_phi && (
                  <span className="text-[10px] px-1 rounded bg-purple-100 text-purple-700">
                    PHI
                  </span>
                )}
              </div>
            </div>

            <div
              className={clsx(
                'flex items-center gap-1 transition-opacity',
                isHovered ? 'opacity-100' : 'opacity-0'
              )}
            >
              {activity.description && (
                <Button
                  variant="ghost"
                  size="icon"
                  className="h-6 w-6"
                  onClick={(e) => {
                    e.stopPropagation();
                    onToggle();
                  }}
                >
                  <MoreHorizontal className="h-3 w-3" />
                </Button>
              )}
              <ChevronRight className="h-4 w-4 text-gray-400" />
            </div>
          </div>
        </div>
      </div>

      {/* New indicator */}
      {isNew && (
        <span className="absolute top-2 right-2 h-2 w-2 rounded-full bg-blue-500 animate-pulse" />
      )}
    </div>
  );
}

export default ActivityWidget;

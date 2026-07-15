'use client';

import React, { useCallback, useEffect, useState } from 'react';
import Link from 'next/link';
import { useRouter } from 'next/navigation';
import { formatDistanceToNow } from 'date-fns';
import {
  Clock,
  ChevronRight,
  RefreshCw,
  Filter,
  MoreHorizontal,
  Bell,
  CheckCheck,
} from 'lucide-react';
import { ActivityIcon } from './ActivityIcon';
import { ActivitySkeleton, ActivityWidgetSkeleton } from './ActivitySkeleton';
import { useActivity } from '@/lib/hooks/useActivity';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Badge } from '@/components/ui/badge';
import { ScrollArea } from '@/components/ui/scroll-area';
import {
  DropdownMenu,
  DropdownMenuContent,
  DropdownMenuItem,
  DropdownMenuSeparator,
  DropdownMenuTrigger,
} from '@/components/ui/dropdown-menu';
import {
  Tooltip,
  TooltipContent,
  TooltipProvider,
  TooltipTrigger,
} from '@/components/ui/tooltip';
import { cn } from '@/lib/utils';
import type { Activity, ActivityType, ActivityEntityRef } from '@/types/activity';
import { ACTIVITY_TYPE_CONFIGS } from '@/lib/api/activity';

interface RecentActivityWidgetProps {
  limit?: number;
  showViewAll?: boolean;
  showFilters?: boolean;
  className?: string;
  filterTypes?: ActivityType[];
  onActivityClick?: (activity: Activity) => void;
  enableRealtime?: boolean;
  refreshInterval?: number; // in milliseconds
}

export function RecentActivityWidget({
  limit = 5,
  showViewAll = true,
  showFilters = true,
  className,
  filterTypes,
  onActivityClick,
  enableRealtime = true,
  refreshInterval = 60000,
}: RecentActivityWidgetProps) {
  const router = useRouter();
  const [selectedTypes, setSelectedTypes] = useState<ActivityType[]>(filterTypes || []);
  const [expandedId, setExpandedId] = useState<string | null>(null);

  const {
    activities,
    loading,
    error,
    total,
    filter,
    setFilter,
    refresh,
  } = useActivity({
    initialFilter: {
      types: filterTypes,
    },
    pageSize: limit,
    enableRealtime,
    groupByDate: false,
  });

  // Auto-refresh
  useEffect(() => {
    if (!refreshInterval) return;

    const interval = setInterval(() => {
      refresh();
    }, refreshInterval);

    return () => clearInterval(interval);
  }, [refresh, refreshInterval]);

  // Handle type filter toggle
  const toggleTypeFilter = useCallback((type: ActivityType) => {
    setSelectedTypes(prev => {
      const newTypes = prev.includes(type)
        ? prev.filter(t => t !== type)
        : [...prev, type];
      
      setFilter({ ...filter, types: newTypes.length > 0 ? newTypes : undefined });
      return newTypes;
    });
  }, [filter, setFilter]);

  // Handle activity click
  const handleActivityClick = useCallback((activity: Activity) => {
    if (onActivityClick) {
      onActivityClick(activity);
    } else if (activity.entities?.[0]?.url) {
      router.push(activity.entities[0].url);
    }
  }, [onActivityClick, router]);

  // Mark all as read (placeholder functionality)
  const markAllAsRead = () => {
    // In a real implementation, this would call an API
    console.log('Mark all as read');
  };

  // Get unique activity types from current data for filter
  const availableTypes = Array.from(new Set(activities.map(a => a.type)));

  if (loading && activities.length === 0) {
    return <ActivityWidgetSkeleton count={limit} />;
  }

  return (
    <Card className={cn('overflow-hidden', className)}>
      <CardHeader className="pb-3">
        <div className="flex items-center justify-between">
          <CardTitle className="flex items-center gap-2 text-lg">
            <Clock className="h-5 w-5" />
            Recent Activity
            {total > 0 && (
              <Badge variant="secondary" className="text-xs">
                {total}
              </Badge>
            )}
          </CardTitle>

          <div className="flex items-center gap-1">
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
                    <RefreshCw className={cn('h-4 w-4', loading && 'animate-spin')} />
                  </Button>
                </TooltipTrigger>
                <TooltipContent>
                  <p>Refresh</p>
                </TooltipContent>
              </Tooltip>
            </TooltipProvider>

            {showFilters && (
              <DropdownMenu>
                <DropdownMenuTrigger asChild>
                  <Button variant="ghost" size="icon" className="h-8 w-8">
                    <Filter className="h-4 w-4" />
                  </Button>
                </DropdownMenuTrigger>
                <DropdownMenuContent align="end" className="w-48">
                  <DropdownMenuItem onClick={markAllAsRead}>
                    <CheckCheck className="h-4 w-4 mr-2" />
                    Mark all as read
                  </DropdownMenuItem>
                  <DropdownMenuSeparator />
                  <DropdownMenuItem disabled className="text-xs text-muted-foreground">
                    Filter by type
                  </DropdownMenuItem>
                  {availableTypes.map((type) => (
                    <DropdownMenuItem
                      key={type}
                      onClick={() => toggleTypeFilter(type)}
                      className="flex items-center gap-2"
                    >
                      <div
                        className={cn(
                          'h-2 w-2 rounded-full',
                          selectedTypes.includes(type) ? 'bg-primary' : 'bg-muted'
                        )}
                      />
                      {ACTIVITY_TYPE_CONFIGS[type].label}
                    </DropdownMenuItem>
                  ))}
                </DropdownMenuContent>
              </DropdownMenu>
            )}
          </div>
        </div>
      </CardHeader>

      <CardContent className="p-0">
        {error ? (
          <div className="p-4 text-center text-red-600">
            <p className="text-sm">Failed to load activities</p>
            <Button variant="outline" size="sm" onClick={refresh} className="mt-2">
              Retry
            </Button>
          </div>
        ) : activities.length === 0 ? (
          <div className="p-8 text-center text-muted-foreground">
            <Bell className="h-8 w-8 mx-auto mb-2 opacity-50" />
            <p className="text-sm">No recent activity</p>
          </div>
        ) : (
          <ScrollArea className="h-[400px]">
            <div className="divide-y">
              {activities.slice(0, limit).map((activity, index) => (
                <ActivityWidgetItem
                  key={activity.id}
                  activity={activity}
                  index={index}
                  isExpanded={expandedId === activity.id}
                  onToggle={() => setExpandedId(expandedId === activity.id ? null : activity.id)}
                  onClick={() => handleActivityClick(activity)}
                />
              ))}
            </div>
          </ScrollArea>
        )}

        {showViewAll && (
          <div className="p-3 border-t">
            <Button variant="ghost" className="w-full" asChild>
              <Link href="/activity">
                View All Activity
                <ChevronRight className="h-4 w-4 ml-1" />
              </Link>
            </Button>
          </div>
        )}
      </CardContent>
    </Card>
  );
}

// Individual activity item for widget
interface ActivityWidgetItemProps {
  activity: Activity;
  index: number;
  isExpanded: boolean;
  onToggle: () => void;
  onClick: () => void;
}

function ActivityWidgetItem({
  activity,
  index,
  isExpanded,
  onToggle,
  onClick,
}: ActivityWidgetItemProps) {
  const [isHovered, setIsHovered] = useState(false);

  const relativeTime = formatDistanceToNow(new Date(activity.timestamp), {
    addSuffix: true,
  });

  // Get primary entity
  const primaryEntity = activity.entities?.find(e => e.type === 'individual') || activity.entities?.[0];

  return (
    <div
      className={cn(
        'group relative p-3 transition-colors cursor-pointer',
        'hover:bg-muted/50',
        activity.severity === 'critical' && 'bg-red-50/50 hover:bg-red-50',
        activity.severity === 'error' && 'bg-red-50/30 hover:bg-red-50/50',
        index === 0 && 'bg-accent/5'
      )}
      onMouseEnter={() => setIsHovered(true)}
      onMouseLeave={() => setIsHovered(false)}
      onClick={onClick}
    >
      <div className="flex items-start gap-3">
        {/* Icon */}
        <ActivityIcon type={activity.type} severity={activity.severity} size="sm" />

        {/* Content */}
        <div className="flex-1 min-w-0">
          <div className="flex items-start justify-between gap-2">
            <div className="flex-1">
              <p className="text-sm font-medium line-clamp-1">{activity.title}</p>
              
              {primaryEntity?.name && (
                <p className="text-xs text-muted-foreground">
                  {primaryEntity.name}
                </p>
              )}

              {isExpanded && activity.description && (
                <p className="text-xs text-muted-foreground mt-1">
                  {activity.description}
                </p>
              )}

              <div className="flex items-center gap-2 mt-1">
                <time className="text-xs text-muted-foreground">
                  {relativeTime}
                </time>
                {activity.contains_phi && (
                  <span className="text-[10px] px-1 rounded bg-purple-100 text-purple-700">
                    PHI
                  </span>
                )}
              </div>
            </div>

            {/* Actions */}
            <div className={cn(
              'flex items-center gap-1 transition-opacity',
              isHovered ? 'opacity-100' : 'opacity-0'
            )}>
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
              <ChevronRight className="h-4 w-4 text-muted-foreground" />
            </div>
          </div>
        </div>
      </div>

      {/* New indicator */}
      {index === 0 && (
        <span className="absolute top-2 right-2 h-2 w-2 rounded-full bg-primary animate-pulse" />
      )}
    </div>
  );
}

// Compact version for small spaces
export function RecentActivityCompact({
  limit = 3,
  className,
  onActivityClick,
}: Omit<RecentActivityWidgetProps, 'showViewAll' | 'showFilters' | 'refreshInterval'>) {
  const router = useRouter();
  const { activities, loading } = useActivity({
    pageSize: limit,
    enableRealtime: true,
    groupByDate: false,
  });

  const handleClick = (activity: Activity) => {
    if (onActivityClick) {
      onActivityClick(activity);
    } else if (activity.entities?.[0]?.url) {
      router.push(activity.entities[0].url);
    }
  };

  if (loading) {
    return <ActivitySkeleton count={limit} compact />;
  }

  return (
    <div className={cn('space-y-2', className)}>
      {activities.slice(0, limit).map((activity) => (
        <button
          key={activity.id}
          onClick={() => handleClick(activity)}
          className="w-full flex items-center gap-3 p-2 rounded-lg hover:bg-muted transition-colors text-left"
        >
          <ActivityIcon type={activity.type} size="sm" />
          <div className="flex-1 min-w-0">
            <p className="text-sm font-medium truncate">{activity.title}</p>
            <p className="text-xs text-muted-foreground">
              {formatDistanceToNow(new Date(activity.timestamp), { addSuffix: true })}
            </p>
          </div>
        </button>
      ))}
    </div>
  );
}

export default RecentActivityWidget;

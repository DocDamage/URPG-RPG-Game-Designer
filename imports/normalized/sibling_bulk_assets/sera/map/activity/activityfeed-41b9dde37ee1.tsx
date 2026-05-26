'use client';

import React, { useCallback, useEffect, useRef, useState } from 'react';
import { useRouter } from 'next/navigation';
import { format } from 'date-fns';
import { Search, Filter, RefreshCw, ChevronDown, X } from 'lucide-react';
import { ActivityItem } from './ActivityItem';
import { ActivitySkeleton, ActivityFeedSkeleton } from './ActivitySkeleton';
import { useActivity, GroupedActivity } from '@/lib/hooks/useActivity';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from '@/components/ui/select';
import {
  DropdownMenu,
  DropdownMenuCheckboxItem,
  DropdownMenuContent,
  DropdownMenuLabel,
  DropdownMenuSeparator,
  DropdownMenuTrigger,
} from '@/components/ui/dropdown-menu';
import { Badge } from '@/components/ui/badge';
import { ScrollArea } from '@/components/ui/scroll-area';
import { cn } from '@/lib/utils';
import type { ActivityType, ActivitySeverity, ActivityFilter, ActivityEntityRef } from '@/types/activity';
import { ACTIVITY_TYPE_CONFIGS } from '@/lib/api/activity';

interface ActivityFeedProps {
  initialFilter?: ActivityFilter;
  showFilters?: boolean;
  showSearch?: boolean;
  className?: string;
  onEntityClick?: (entity: ActivityEntityRef) => void;
  enableRealtime?: boolean;
}

const ACTIVITY_TYPES = Object.keys(ACTIVITY_TYPE_CONFIGS) as ActivityType[];
const SEVERITIES: ActivitySeverity[] = ['info', 'success', 'warning', 'error', 'critical'];

export function ActivityFeed({
  initialFilter = {},
  showFilters = true,
  showSearch = true,
  className,
  onEntityClick,
  enableRealtime = true,
}: ActivityFeedProps) {
  const router = useRouter();
  const [searchQuery, setSearchQuery] = useState('');
  const [selectedTypes, setSelectedTypes] = useState<ActivityType[]>([]);
  const [selectedSeverities, setSelectedSeverities] = useState<ActivitySeverity[]>([]);
  const [dateRange, setDateRange] = useState<string>('all');
  const loadMoreRef = useRef<HTMLDivElement>(null);

  const {
    activities,
    groupedActivities,
    loading,
    loadingMore,
    error,
    hasMore,
    total,
    filter,
    setFilter,
    refresh,
    loadMore,
    search,
    clearSearch,
  } = useActivity({
    initialFilter,
    pageSize: 20,
    enableRealtime,
    groupByDate: true,
  });

  // Handle search
  const handleSearch = useCallback(async (e: React.FormEvent) => {
    e.preventDefault();
    if (searchQuery.trim()) {
      await search(searchQuery);
    }
  }, [searchQuery, search]);

  // Handle filter changes
  const handleTypeToggle = (type: ActivityType) => {
    setSelectedTypes(prev => {
      const newTypes = prev.includes(type)
        ? prev.filter(t => t !== type)
        : [...prev, type];
      
      setFilter({ ...filter, types: newTypes.length > 0 ? newTypes : undefined });
      return newTypes;
    });
  };

  const handleSeverityToggle = (severity: ActivitySeverity) => {
    setSelectedSeverities(prev => {
      const newSeverities = prev.includes(severity)
        ? prev.filter(s => s !== severity)
        : [...prev, severity];
      
      setFilter({ ...filter, severity: newSeverities.length > 0 ? newSeverities : undefined });
      return newSeverities;
    });
  };

  const handleDateRangeChange = (value: string) => {
    setDateRange(value);
    const now = new Date();
    let dateFrom: string | undefined;

    switch (value) {
      case 'today':
        dateFrom = new Date(now.setHours(0, 0, 0, 0)).toISOString();
        break;
      case 'week':
        dateFrom = new Date(now.setDate(now.getDate() - 7)).toISOString();
        break;
      case 'month':
        dateFrom = new Date(now.setMonth(now.getMonth() - 1)).toISOString();
        break;
      default:
        dateFrom = undefined;
    }

    setFilter({ ...filter, date_from: dateFrom });
  };

  // Clear all filters
  const clearFilters = () => {
    setSelectedTypes([]);
    setSelectedSeverities([]);
    setDateRange('all');
    setSearchQuery('');
    setFilter({});
    clearSearch();
  };

  // Infinite scroll
  useEffect(() => {
    const observer = new IntersectionObserver(
      (entries) => {
        if (entries[0].isIntersecting && hasMore && !loadingMore) {
          loadMore();
        }
      },
      { threshold: 0.5 }
    );

    if (loadMoreRef.current) {
      observer.observe(loadMoreRef.current);
    }

    return () => observer.disconnect();
  }, [hasMore, loadingMore, loadMore]);

  // Handle entity click
  const handleEntityClick = useCallback((entity: ActivityEntityRef) => {
    if (onEntityClick) {
      onEntityClick(entity);
    } else if (entity.url) {
      router.push(entity.url);
    }
  }, [onEntityClick, router]);

  const activeFiltersCount = selectedTypes.length + selectedSeverities.length + (dateRange !== 'all' ? 1 : 0);

  if (loading && activities.length === 0) {
    return <ActivityFeedSkeleton />;
  }

  return (
    <div className={cn('space-y-6', className)}>
      {/* Header */}
      <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-4">
        <div>
          <h1 className="text-2xl font-bold">Activity Feed</h1>
          <p className="text-muted-foreground">
            {total.toLocaleString()} activities tracked
          </p>
        </div>

        <div className="flex items-center gap-2">
          <Button
            variant="outline"
            size="icon"
            onClick={refresh}
            disabled={loading}
          >
            <RefreshCw className={cn('h-4 w-4', loading && 'animate-spin')} />
          </Button>
        </div>
      </div>

      {/* Search and Filters */}
      {(showSearch || showFilters) && (
        <div className="flex flex-col sm:flex-row gap-4">
          {showSearch && (
            <form onSubmit={handleSearch} className="flex-1 relative">
              <Search className="absolute left-3 top-1/2 -translate-y-1/2 h-4 w-4 text-muted-foreground" />
              <Input
                placeholder="Search activities..."
                value={searchQuery}
                onChange={(e) => setSearchQuery(e.target.value)}
                className="pl-10"
              />
              {searchQuery && (
                <button
                  type="button"
                  onClick={() => {
                    setSearchQuery('');
                    clearSearch();
                  }}
                  className="absolute right-3 top-1/2 -translate-y-1/2"
                >
                  <X className="h-4 w-4 text-muted-foreground" />
                </button>
              )}
            </form>
          )}

          {showFilters && (
            <>
              {/* Type Filter */}
              <DropdownMenu>
                <DropdownMenuTrigger asChild>
                  <Button variant="outline" className="relative">
                    Type
                    {selectedTypes.length > 0 && (
                      <Badge variant="secondary" className="ml-2 h-5 w-5 p-0 flex items-center justify-center">
                        {selectedTypes.length}
                      </Badge>
                    )}
                    <ChevronDown className="ml-2 h-4 w-4" />
                  </Button>
                </DropdownMenuTrigger>
                <DropdownMenuContent className="w-56" align="end">
                  <DropdownMenuLabel>Activity Types</DropdownMenuLabel>
                  <DropdownMenuSeparator />
                  <ScrollArea className="h-64">
                    {ACTIVITY_TYPES.map((type) => (
                      <DropdownMenuCheckboxItem
                        key={type}
                        checked={selectedTypes.includes(type)}
                        onCheckedChange={() => handleTypeToggle(type)}
                      >
                        {ACTIVITY_TYPE_CONFIGS[type].label}
                      </DropdownMenuCheckboxItem>
                    ))}
                  </ScrollArea>
                </DropdownMenuContent>
              </DropdownMenu>

              {/* Severity Filter */}
              <DropdownMenu>
                <DropdownMenuTrigger asChild>
                  <Button variant="outline" className="relative">
                    Severity
                    {selectedSeverities.length > 0 && (
                      <Badge variant="secondary" className="ml-2 h-5 w-5 p-0 flex items-center justify-center">
                        {selectedSeverities.length}
                      </Badge>
                    )}
                    <ChevronDown className="ml-2 h-4 w-4" />
                  </Button>
                </DropdownMenuTrigger>
                <DropdownMenuContent align="end">
                  <DropdownMenuLabel>Severity Levels</DropdownMenuLabel>
                  <DropdownMenuSeparator />
                  {SEVERITIES.map((severity) => (
                    <DropdownMenuCheckboxItem
                      key={severity}
                      checked={selectedSeverities.includes(severity)}
                      onCheckedChange={() => handleSeverityToggle(severity)}
                    >
                      <span className="capitalize">{severity}</span>
                    </DropdownMenuCheckboxItem>
                  ))}
                </DropdownMenuContent>
              </DropdownMenu>

              {/* Date Range */}
              <Select value={dateRange} onValueChange={handleDateRangeChange}>
                <SelectTrigger className="w-32">
                  <SelectValue placeholder="Date Range" />
                </SelectTrigger>
                <SelectContent>
                  <SelectItem value="all">All Time</SelectItem>
                  <SelectItem value="today">Today</SelectItem>
                  <SelectItem value="week">This Week</SelectItem>
                  <SelectItem value="month">This Month</SelectItem>
                </SelectContent>
              </Select>

              {/* Clear Filters */}
              {activeFiltersCount > 0 && (
                <Button variant="ghost" size="sm" onClick={clearFilters}>
                  <X className="h-4 w-4 mr-1" />
                  Clear ({activeFiltersCount})
                </Button>
              )}
            </>
          )}
        </div>
      )}

      {/* Error State */}
      {error && (
        <div className="p-4 rounded-lg bg-red-50 text-red-700 text-center">
          <p className="font-medium">Failed to load activities</p>
          <p className="text-sm">{error.message}</p>
          <Button variant="outline" size="sm" onClick={refresh} className="mt-2">
            Try Again
          </Button>
        </div>
      )}

      {/* Empty State */}
      {!loading && activities.length === 0 && (
        <div className="text-center py-12">
          <Filter className="h-12 w-12 text-muted-foreground mx-auto mb-4" />
          <h3 className="text-lg font-medium">No activities found</h3>
          <p className="text-muted-foreground mt-1">
            {activeFiltersCount > 0
              ? 'Try adjusting your filters'
              : 'Activities will appear here as they occur'}
          </p>
          {activeFiltersCount > 0 && (
            <Button variant="outline" onClick={clearFilters} className="mt-4">
              Clear Filters
            </Button>
          )}
        </div>
      )}

      {/* Activity Groups */}
      <div className="space-y-6">
        {groupedActivities.map((group) => (
          <div key={group.date} className="space-y-3">
            <h3 className="text-sm font-semibold text-muted-foreground uppercase tracking-wider">
              {group.label}
            </h3>
            <div className="space-y-3">
              {group.activities.map((activity) => (
                <ActivityItem
                  key={activity.id}
                  activity={activity}
                  onEntityClick={handleEntityClick}
                />
              ))}
            </div>
          </div>
        ))}
      </div>

      {/* Load More */}
      {(hasMore || loadingMore) && (
        <div ref={loadMoreRef} className="py-4">
          {loadingMore ? (
            <ActivitySkeleton count={2} />
          ) : (
            <Button variant="outline" onClick={loadMore} className="w-full">
              Load More
            </Button>
          )}
        </div>
      )}
    </div>
  );
}

export default ActivityFeed;

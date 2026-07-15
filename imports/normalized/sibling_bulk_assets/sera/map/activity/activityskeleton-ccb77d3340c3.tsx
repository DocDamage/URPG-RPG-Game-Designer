'use client';

import React from 'react';
import { Skeleton } from '@/components/ui/skeleton';
import { Card, CardContent, CardHeader } from '@/components/ui/card';

interface ActivitySkeletonProps {
  count?: number;
  compact?: boolean;
}

export function ActivitySkeleton({ count = 3, compact = false }: ActivitySkeletonProps) {
  if (compact) {
    return (
      <div className="space-y-2">
        {Array.from({ length: count }).map((_, i) => (
          <div key={i} className="flex items-center gap-3 p-2">
            <Skeleton className="h-8 w-8 rounded-full" />
            <div className="flex-1 space-y-1">
              <Skeleton className="h-4 w-3/4" />
              <Skeleton className="h-3 w-1/2" />
            </div>
          </div>
        ))}
      </div>
    );
  }

  return (
    <div className="space-y-4">
      {Array.from({ length: count }).map((_, i) => (
        <div
          key={i}
          className="flex gap-4 p-4 rounded-xl border bg-card"
        >
          <Skeleton className="h-10 w-10 rounded-full flex-shrink-0" />
          <div className="flex-1 space-y-3">
            <div className="flex items-start justify-between gap-2">
              <div className="flex-1 space-y-2">
                <div className="flex items-center gap-2">
                  <Skeleton className="h-4 w-48" />
                  <Skeleton className="h-5 w-16" />
                </div>
                <Skeleton className="h-3 w-full" />
                <Skeleton className="h-3 w-2/3" />
              </div>
            </div>
            <div className="flex items-center justify-between">
              <div className="flex items-center gap-2">
                <Skeleton className="h-6 w-6 rounded-full" />
                <Skeleton className="h-3 w-24" />
              </div>
              <Skeleton className="h-3 w-20" />
            </div>
          </div>
        </div>
      ))}
    </div>
  );
}

export function ActivityWidgetSkeleton({ count = 5 }: { count?: number }) {
  return (
    <Card>
      <CardHeader className="pb-3">
        <div className="flex items-center justify-between">
          <Skeleton className="h-5 w-32" />
          <Skeleton className="h-8 w-8" />
        </div>
      </CardHeader>
      <CardContent>
        <ActivitySkeleton count={count} compact />
        <Skeleton className="h-9 w-full mt-4" />
      </CardContent>
    </Card>
  );
}

export function ActivityFeedSkeleton() {
  return (
    <div className="space-y-6">
      {/* Header */}
      <div className="flex items-center justify-between">
        <Skeleton className="h-8 w-48" />
        <div className="flex items-center gap-2">
          <Skeleton className="h-9 w-32" />
          <Skeleton className="h-9 w-32" />
          <Skeleton className="h-9 w-32" />
        </div>
      </div>

      {/* Search */}
      <Skeleton className="h-10 w-full" />

      {/* Activity Groups */}
      {Array.from({ length: 3 }).map((_, groupIndex) => (
        <div key={groupIndex} className="space-y-3">
          <Skeleton className="h-6 w-32" />
          <ActivitySkeleton count={3} />
        </div>
      ))}
    </div>
  );
}

export default ActivitySkeleton;

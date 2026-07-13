'use client';

import React, { useState } from 'react';
import Link from 'next/link';
import { formatDistanceToNow, format } from 'date-fns';
import {
  MoreVertical,
  ExternalLink,
  Eye,
  Share2,
  Bookmark,
  BookmarkCheck,
} from 'lucide-react';
import { ActivityIcon, getActivityLabel } from './ActivityIcon';
import { ActivitySeverityBadge } from './ActivitySeverityBadge';
import {
  DropdownMenu,
  DropdownMenuContent,
  DropdownMenuItem,
  DropdownMenuSeparator,
  DropdownMenuTrigger,
} from '@/components/ui/dropdown-menu';
import { Button } from '@/components/ui/button';
import { Avatar, AvatarFallback, AvatarImage } from '@/components/ui/avatar';
import { cn } from '@/lib/utils';
import type { Activity, ActivityEntityRef } from '@/types/activity';

interface ActivityItemProps {
  activity: Activity;
  showActor?: boolean;
  showDate?: boolean;
  compact?: boolean;
  className?: string;
  onEntityClick?: (entity: ActivityEntityRef) => void;
}

export function ActivityItem({
  activity,
  showActor = true,
  showDate = true,
  compact = false,
  className,
  onEntityClick,
}: ActivityItemProps) {
  const [isBookmarked, setIsBookmarked] = useState(false);
  const [isExpanded, setIsExpanded] = useState(false);

  // Get primary entity for linking
  const primaryEntity = activity.entities?.[0];
  
  // Format timestamp
  const relativeTime = formatDistanceToNow(new Date(activity.timestamp), {
    addSuffix: true,
  });
  const absoluteTime = format(new Date(activity.timestamp), 'PPp');

  // Get actor initials for avatar fallback
  const getInitials = (name: string) => {
    return name
      .split(' ')
      .map(n => n[0])
      .join('')
      .toUpperCase()
      .slice(0, 2);
  };

  // Handle entity click
  const handleEntityClick = (entity: ActivityEntityRef, e: React.MouseEvent) => {
    e.preventDefault();
    e.stopPropagation();
    onEntityClick?.(entity);
  };

  // Handle share
  const handleShare = async () => {
    if (navigator.share) {
      try {
        await navigator.share({
          title: activity.title,
          text: activity.description || activity.title,
          url: primaryEntity?.url || window.location.href,
        });
      } catch (err) {
        // User cancelled or share failed
      }
    } else {
      // Fallback: copy to clipboard
      await navigator.clipboard.writeText(
        `${activity.title} - ${activity.description || ''}`
      );
    }
  };

  if (compact) {
    return (
      <div
        className={cn(
          'flex items-center gap-3 p-2 rounded-lg hover:bg-muted/50 transition-colors cursor-pointer',
          className
        )}
        onClick={() => primaryEntity?.url && window.open(primaryEntity.url, '_self')}
      >
        <ActivityIcon type={activity.type} size="sm" />
        <div className="flex-1 min-w-0">
          <p className="text-sm font-medium truncate">{activity.title}</p>
          <p className="text-xs text-muted-foreground">{relativeTime}</p>
        </div>
      </div>
    );
  }

  return (
    <div
      className={cn(
        'group flex gap-4 p-4 rounded-xl border bg-card hover:bg-accent/5 transition-all',
        activity.severity === 'critical' && 'border-red-200 bg-red-50/50',
        activity.severity === 'error' && 'border-red-100',
        activity.severity === 'warning' && 'border-amber-100',
        activity.severity === 'success' && 'border-green-100',
        className
      )}
    >
      {/* Icon */}
      <div className="flex-shrink-0">
        <ActivityIcon type={activity.type} severity={activity.severity} size="md" />
      </div>

      {/* Content */}
      <div className="flex-1 min-w-0">
        {/* Header */}
        <div className="flex items-start justify-between gap-2">
          <div className="flex-1">
            <div className="flex items-center gap-2 flex-wrap">
              <h4 className="font-semibold text-sm">{activity.title}</h4>
              <ActivitySeverityBadge severity={activity.severity} />
              {activity.contains_phi && (
                <span className="text-xs px-1.5 py-0.5 rounded bg-purple-100 text-purple-700">
                  PHI
                </span>
              )}
            </div>
            
            {activity.description && (
              <p
                className={cn(
                  'text-sm text-muted-foreground mt-1',
                  !isExpanded && 'line-clamp-2'
                )}
              >
                {activity.description}
              </p>
            )}
          </div>

          {/* Actions Menu */}
          <DropdownMenu>
            <DropdownMenuTrigger asChild>
              <Button
                variant="ghost"
                size="icon"
                className="h-8 w-8 opacity-0 group-hover:opacity-100 transition-opacity"
              >
                <MoreVertical className="h-4 w-4" />
              </Button>
            </DropdownMenuTrigger>
            <DropdownMenuContent align="end">
              <DropdownMenuItem onClick={() => setIsBookmarked(!isBookmarked)}>
                {isBookmarked ? (
                  <>
                    <BookmarkCheck className="h-4 w-4 mr-2" />
                    Remove Bookmark
                  </>
                ) : (
                  <>
                    <Bookmark className="h-4 w-4 mr-2" />
                    Bookmark
                  </>
                )}
              </DropdownMenuItem>
              <DropdownMenuItem onClick={handleShare}>
                <Share2 className="h-4 w-4 mr-2" />
                Share
              </DropdownMenuItem>
              {primaryEntity?.url && (
                <DropdownMenuItem asChild>
                  <Link href={primaryEntity.url}>
                    <ExternalLink className="h-4 w-4 mr-2" />
                    View Details
                  </Link>
                </DropdownMenuItem>
              )}
              <DropdownMenuSeparator />
              <DropdownMenuItem onClick={() => setIsExpanded(!isExpanded)}>
                <Eye className="h-4 w-4 mr-2" />
                {isExpanded ? 'Show Less' : 'Show More'}
              </DropdownMenuItem>
            </DropdownMenuContent>
          </DropdownMenu>
        </div>

        {/* Entities */}
        {activity.entities && activity.entities.length > 0 && (
          <div className="flex flex-wrap gap-2 mt-2">
            {activity.entities.map((entity, index) => (
              <button
                key={`${entity.type}-${entity.id}-${index}`}
                onClick={(e) => handleEntityClick(entity, e)}
                className="inline-flex items-center gap-1 text-xs px-2 py-1 rounded-full bg-muted hover:bg-muted/80 transition-colors"
              >
                <span className="text-muted-foreground capitalize">{entity.type}:</span>
                <span className="font-medium">{entity.name || entity.id.slice(0, 8)}</span>
                {entity.url && <ExternalLink className="h-3 w-3 ml-1" />}
              </button>
            ))}
          </div>
        )}

        {/* Footer */}
        <div className="flex items-center justify-between mt-3">
          {showActor && (
            <div className="flex items-center gap-2">
              <Avatar className="h-6 w-6">
                <AvatarImage src={activity.actor.avatar_url} />
                <AvatarFallback className="text-xs">
                  {getInitials(activity.actor.name)}
                </AvatarFallback>
              </Avatar>
              <div className="flex items-center gap-1 text-xs text-muted-foreground">
                <span className="font-medium text-foreground">{activity.actor.name}</span>
                {activity.actor.role && (
                  <>
                    <span>•</span>
                    <span className="capitalize">{activity.actor.role}</span>
                  </>
                )}
              </div>
            </div>
          )}

          {showDate && (
            <time
              className="text-xs text-muted-foreground"
              title={absoluteTime}
              dateTime={activity.timestamp}
            >
              {relativeTime}
            </time>
          )}
        </div>

        {/* Expanded Details */}
        {isExpanded && activity.metadata && (
          <div className="mt-3 pt-3 border-t text-xs text-muted-foreground">
            <p className="font-medium mb-1">Additional Details:</p>
            <pre className="bg-muted p-2 rounded overflow-x-auto">
              {JSON.stringify(activity.metadata, null, 2)}
            </pre>
          </div>
        )}
      </div>
    </div>
  );
}

export default ActivityItem;

"use client";

import React from 'react';
import { format, parseISO } from 'date-fns';
import { Clock, User, MapPin, AlertCircle, CheckCircle2, Calendar } from 'lucide-react';
import { Card, CardContent, CardHeader } from '@/components/ui/card';
import { Badge } from '@/components/ui/badge';
import { Button } from '@/components/ui/button';
import { cn } from '@/lib/utils';

export interface Shift {
  id: string;
  patternId?: string;
  patternName?: string;
  homeId: string;
  userId: string;
  userName?: string;
  shiftDate: string;
  startTime: string;
  endTime: string;
  role: string;
  status: 'scheduled' | 'confirmed' | 'in_progress' | 'completed' | 'cancelled' | 'swapped';
  notes?: string;
  colorCode?: string;
}

interface ShiftCardProps {
  shift: Shift;
  onClick?: (shift: Shift) => void;
  onAssign?: (shiftId: string) => void;
  onSwapRequest?: (shiftId: string) => void;
  className?: string;
  compact?: boolean;
}

export function ShiftCard({
  shift,
  onClick,
  onAssign,
  onSwapRequest,
  className,
  compact = false,
}: ShiftCardProps) {
  const getStatusColor = (status: Shift['status']) => {
    switch (status) {
      case 'scheduled':
        return 'bg-blue-100 text-blue-800 border-blue-200';
      case 'confirmed':
        return 'bg-green-100 text-green-800 border-green-200';
      case 'in_progress':
        return 'bg-yellow-100 text-yellow-800 border-yellow-200';
      case 'completed':
        return 'bg-gray-100 text-gray-800 border-gray-200';
      case 'cancelled':
        return 'bg-red-100 text-red-800 border-red-200';
      case 'swapped':
        return 'bg-purple-100 text-purple-800 border-purple-200';
      default:
        return 'bg-gray-100 text-gray-800';
    }
  };

  const getStatusIcon = (status: Shift['status']) => {
    switch (status) {
      case 'confirmed':
        return <CheckCircle2 className="h-3 w-3" />;
      case 'cancelled':
        return <AlertCircle className="h-3 w-3" />;
      default:
        return null;
    }
  };

  const startTime = parseISO(shift.startTime);
  const endTime = parseISO(shift.endTime);
  const duration = (endTime.getTime() - startTime.getTime()) / (1000 * 60 * 60);

  if (compact) {
    return (
      <div
        className={cn(
          "flex items-center gap-3 p-3 rounded-lg border cursor-pointer hover:bg-muted/50 transition-colors",
          getStatusColor(shift.status),
          className
        )}
        style={shift.colorCode ? { backgroundColor: shift.colorCode + '20', borderColor: shift.colorCode } : undefined}
        onClick={() => onClick?.(shift)}
      >
        <div className="flex-1 min-w-0">
          <div className="flex items-center gap-2">
            <span className="font-medium truncate">
              {format(startTime, 'h:mm a')} - {format(endTime, 'h:mm a')}
            </span>
            <Badge variant="outline" className="text-xs">
              {shift.role}
            </Badge>
          </div>
          <div className="text-sm truncate">
            {shift.userName || 'Unassigned'}
          </div>
        </div>
        <Badge variant="secondary" className="text-xs capitalize">
          {getStatusIcon(shift.status)}
          <span className="ml-1">{shift.status.replace('_', ' ')}</span>
        </Badge>
      </div>
    );
  }

  return (
    <Card
      className={cn(
        "cursor-pointer hover:shadow-md transition-shadow",
        shift.status === 'cancelled' && "opacity-60",
        className
      )}
      onClick={() => onClick?.(shift)}
    >
      <CardHeader className="pb-3">
        <div className="flex items-start justify-between">
          <div className="flex items-center gap-2">
            <Calendar className="h-4 w-4 text-muted-foreground" />
            <span className="font-medium">
              {format(parseISO(shift.shiftDate), 'EEEE, MMM d')}
            </span>
          </div>
          <Badge className={cn("capitalize", getStatusColor(shift.status))}>
            {getStatusIcon(shift.status)}
            <span className="ml-1">{shift.status.replace('_', ' ')}</span>
          </Badge>
        </div>
      </CardHeader>
      <CardContent className="space-y-3">
        <div className="flex items-center gap-2 text-sm">
          <Clock className="h-4 w-4 text-muted-foreground" />
          <span>
            {format(startTime, 'h:mm a')} - {format(endTime, 'h:mm a')}
          </span>
          <span className="text-muted-foreground">({duration}h)</span>
        </div>

        <div className="flex items-center gap-2 text-sm">
          <User className="h-4 w-4 text-muted-foreground" />
          <span className={cn(!shift.userName && "text-muted-foreground italic")}>
            {shift.userName || 'Unassigned'}
          </span>
        </div>

        <div className="flex items-center gap-2 text-sm">
          <MapPin className="h-4 w-4 text-muted-foreground" />
          <Badge variant="outline" className="text-xs">
            {shift.role}
          </Badge>
        </div>

        {shift.notes && (
          <p className="text-sm text-muted-foreground line-clamp-2">
            {shift.notes}
          </p>
        )}

        <div className="flex gap-2 pt-2">
          {shift.userId === 'UNASSIGNED' && onAssign && (
            <Button
              size="sm"
              variant="outline"
              className="flex-1"
              onClick={(e) => {
                e.stopPropagation();
                onAssign(shift.id);
              }}
            >
              Assign
            </Button>
          )}
          {shift.status === 'scheduled' && onSwapRequest && (
            <Button
              size="sm"
              variant="outline"
              className="flex-1"
              onClick={(e) => {
                e.stopPropagation();
                onSwapRequest(shift.id);
              }}
            >
              Request Swap
            </Button>
          )}
        </div>
      </CardContent>
    </Card>
  );
}

export default ShiftCard;

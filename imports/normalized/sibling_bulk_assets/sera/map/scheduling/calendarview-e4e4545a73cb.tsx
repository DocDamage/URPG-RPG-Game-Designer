"use client";

import React, { useState, useMemo } from 'react';
import { format, startOfMonth, endOfMonth, eachDayOfInterval, isSameMonth, isToday, addMonths, subMonths, startOfWeek, endOfWeek, isSameDay, parseISO } from 'date-fns';
import { ChevronLeft, ChevronRight, Clock, User, AlertCircle } from 'lucide-react';
import { Button } from '@/components/ui/button';
import { Badge } from '@/components/ui/badge';
import { cn } from '@/lib/utils';

export interface CalendarShift {
  id: string;
  userId: string;
  userName?: string;
  shiftDate: string;
  startTime: string;
  endTime: string;
  role: string;
  status: 'scheduled' | 'confirmed' | 'in_progress' | 'completed' | 'cancelled' | 'swapped';
  colorCode?: string;
  notes?: string;
}

interface CalendarViewProps {
  shifts: CalendarShift[];
  onShiftClick?: (shift: CalendarShift) => void;
  onDateClick?: (date: Date) => void;
  selectedDate?: Date;
  className?: string;
}

export function CalendarView({
  shifts,
  onShiftClick,
  onDateClick,
  selectedDate,
  className,
}: CalendarViewProps) {
  const [currentMonth, setCurrentMonth] = useState(new Date());

  const calendarDays = useMemo(() => {
    const start = startOfWeek(startOfMonth(currentMonth));
    const end = endOfWeek(endOfMonth(currentMonth));
    return eachDayOfInterval({ start, end });
  }, [currentMonth]);

  const shiftsByDay = useMemo(() => {
    const map = new Map<string, CalendarShift[]>();
    shifts.forEach((shift) => {
      const date = shift.shiftDate;
      if (!map.has(date)) {
        map.set(date, []);
      }
      map.get(date)!.push(shift);
    });
    return map;
  }, [shifts]);

  const getShiftsForDay = (day: Date): CalendarShift[] => {
    const dateStr = format(day, 'yyyy-MM-dd');
    return shiftsByDay.get(dateStr) || [];
  };

  const getStatusColor = (status: CalendarShift['status']) => {
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

  const weekDays = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'];

  return (
    <div className={cn("bg-white rounded-lg border shadow-sm", className)}>
      {/* Header */}
      <div className="flex items-center justify-between p-4 border-b">
        <h2 className="text-lg font-semibold">
          {format(currentMonth, 'MMMM yyyy')}
        </h2>
        <div className="flex items-center gap-2">
          <Button
            variant="outline"
            size="icon"
            onClick={() => setCurrentMonth(subMonths(currentMonth, 1))}
          >
            <ChevronLeft className="h-4 w-4" />
          </Button>
          <Button
            variant="outline"
            size="sm"
            onClick={() => setCurrentMonth(new Date())}
          >
            Today
          </Button>
          <Button
            variant="outline"
            size="icon"
            onClick={() => setCurrentMonth(addMonths(currentMonth, 1))}
          >
            <ChevronRight className="h-4 w-4" />
          </Button>
        </div>
      </div>

      {/* Week Day Headers */}
      <div className="grid grid-cols-7 border-b">
        {weekDays.map((day) => (
          <div
            key={day}
            className="py-2 text-center text-sm font-medium text-muted-foreground"
          >
            {day}
          </div>
        ))}
      </div>

      {/* Calendar Grid */}
      <div className="grid grid-cols-7">
        {calendarDays.map((day, index) => {
          const dayShifts = getShiftsForDay(day);
          const isCurrentMonth = isSameMonth(day, currentMonth);
          const isTodayDate = isToday(day);
          const isSelected = selectedDate && isSameDay(day, selectedDate);

          return (
            <div
              key={index}
              className={cn(
                "min-h-[100px] border-b border-r p-2 cursor-pointer transition-colors hover:bg-muted/50",
                !isCurrentMonth && "bg-muted/30 text-muted-foreground",
                isTodayDate && "bg-blue-50/50",
                isSelected && "ring-2 ring-primary ring-inset"
              )}
              onClick={() => onDateClick?.(day)}
            >
              <div className={cn(
                "text-sm font-medium mb-1",
                isTodayDate && "text-primary font-bold"
              )}>
                {format(day, 'd')}
              </div>
              <div className="space-y-1">
                {dayShifts.slice(0, 3).map((shift) => (
                  <div
                    key={shift.id}
                    className={cn(
                      "text-xs px-1.5 py-0.5 rounded border truncate cursor-pointer",
                      getStatusColor(shift.status)
                    )}
                    style={shift.colorCode ? { backgroundColor: shift.colorCode + '20', borderColor: shift.colorCode } : undefined}
                    onClick={(e) => {
                      e.stopPropagation();
                      onShiftClick?.(shift);
                    }}
                    title={`${shift.userName || 'Unassigned'} - ${shift.role}`}
                  >
                    {format(parseISO(shift.startTime), 'h:mm a')} - {shift.userName || 'Unassigned'}
                  </div>
                ))}
                {dayShifts.length > 3 && (
                  <div className="text-xs text-muted-foreground text-center">
                    +{dayShifts.length - 3} more
                  </div>
                )}
              </div>
            </div>
          );
        })}
      </div>
    </div>
  );
}

export default CalendarView;

/**
 * Calendar Widget
 * 
 * Mini calendar with events display.
 */

'use client';

import React, { useState, useMemo } from 'react';
import { clsx } from 'clsx';
import {
  format,
  startOfMonth,
  endOfMonth,
  startOfWeek,
  endOfWeek,
  eachDayOfInterval,
  isSameMonth,
  isSameDay,
  isToday,
  addMonths,
  subMonths,
  parseISO,
} from 'date-fns';
import {
  ChevronLeft,
  ChevronRight,
  Calendar as CalendarIcon,
  Pill,
  Clock,
  User,
  FileText,
} from 'lucide-react';
import type { WidgetProps, CalendarWidgetConfig, CalendarEvent } from '../types';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Badge } from '@/components/ui/badge';
import {
  Tooltip,
  TooltipContent,
  TooltipProvider,
  TooltipTrigger,
} from '@/components/ui/tooltip';

// Mock events
const mockEvents: CalendarEvent[] = [
  {
    id: '1',
    title: 'Morning Meds',
    start: new Date().toISOString(),
    end: new Date(Date.now() + 3600000).toISOString(),
    type: 'medication',
    allDay: false,
  },
  {
    id: '2',
    title: 'Dr. Appointment',
    start: new Date(Date.now() + 86400000).toISOString(),
    end: new Date(Date.now() + 90000000).toISOString(),
    type: 'appointment',
    allDay: false,
  },
  {
    id: '3',
    title: 'Staff Meeting',
    start: new Date(Date.now() + 172800000).toISOString(),
    end: new Date(Date.now() + 180000000).toISOString(),
    type: 'other',
    allDay: false,
  },
  {
    id: '4',
    title: 'Evening Meds',
    start: new Date(Date.now() + 64800000).toISOString(),
    end: new Date(Date.now() + 68400000).toISOString(),
    type: 'medication',
    allDay: false,
  },
];

// Event type configuration
const eventTypeConfig: Record<string, { icon: React.ComponentType<{ className?: string }>; color: string; label: string }> = {
  medication: { icon: Pill, color: 'bg-blue-500', label: 'Medication' },
  appointment: { icon: Clock, color: 'bg-green-500', label: 'Appointment' },
  shift: { icon: User, color: 'bg-purple-500', label: 'Shift' },
  task: { icon: FileText, color: 'bg-orange-500', label: 'Task' },
  other: { icon: CalendarIcon, color: 'bg-gray-500', label: 'Other' },
};

// Get events for a specific date
function getEventsForDate(events: CalendarEvent[], date: Date): CalendarEvent[] {
  return events.filter((event) => {
    const eventDate = parseISO(event.start);
    return isSameDay(eventDate, date);
  });
}

export function CalendarWidget({ config, className }: WidgetProps) {
  const calendarConfig = config as CalendarWidgetConfig;
  const [currentMonth, setCurrentMonth] = useState(new Date());
  const [selectedDate, setSelectedDate] = useState<Date | null>(null);

  // Calendar grid days
  const days = useMemo(() => {
    const monthStart = startOfMonth(currentMonth);
    const monthEnd = endOfMonth(monthStart);
    const startDate = startOfWeek(monthStart);
    const endDate = endOfWeek(monthEnd);

    return eachDayOfInterval({ start: startDate, end: endDate });
  }, [currentMonth]);

  // Get events for selected date
  const selectedDateEvents = selectedDate
    ? getEventsForDate(mockEvents, selectedDate)
    : [];

  // Get upcoming events (next 3)
  const upcomingEvents = useMemo(() => {
    const now = new Date();
    return mockEvents
      .filter((e) => parseISO(e.start) >= now)
      .sort((a, b) => parseISO(a.start).getTime() - parseISO(b.start).getTime())
      .slice(0, 3);
  }, []);

  const weekDays = ['Su', 'Mo', 'Tu', 'We', 'Th', 'Fr', 'Sa'];

  return (
    <Card className={clsx('h-full flex flex-col', className)}>
      <CardHeader className="pb-3">
        <div className="flex items-center justify-between">
          <CardTitle className="text-lg font-semibold">
            {config.title || 'Calendar'}
          </CardTitle>
          <div className="flex items-center gap-1">
            <Button
              variant="ghost"
              size="icon"
              className="h-7 w-7"
              onClick={() => setCurrentMonth(subMonths(currentMonth, 1))}
            >
              <ChevronLeft className="h-4 w-4" />
            </Button>
            <span className="text-sm font-medium min-w-[100px] text-center">
              {format(currentMonth, 'MMMM yyyy')}
            </span>
            <Button
              variant="ghost"
              size="icon"
              className="h-7 w-7"
              onClick={() => setCurrentMonth(addMonths(currentMonth, 1))}
            >
              <ChevronRight className="h-4 w-4" />
            </Button>
          </div>
        </div>
      </CardHeader>

      <CardContent className="flex-1 p-0 flex flex-col">
        {/* Calendar grid */}
        <div className="px-4">
          {/* Week day headers */}
          <div className="grid grid-cols-7 mb-2">
            {weekDays.map((day) => (
              <div
                key={day}
                className="text-center text-xs font-medium text-gray-500 py-1"
              >
                {day}
              </div>
            ))}
          </div>

          {/* Days grid */}
          <div className="grid grid-cols-7 gap-1">
            {days.map((day, index) => {
              const dateEvents = getEventsForDate(mockEvents, day);
              const hasEvents = dateEvents.length > 0;
              const isSelected = selectedDate && isSameDay(day, selectedDate);
              const isCurrentMonth = isSameMonth(day, currentMonth);

              return (
                <TooltipProvider key={day.toISOString()}>
                  <Tooltip>
                    <TooltipTrigger asChild>
                      <button
                        onClick={() => setSelectedDate(day)}
                        className={clsx(
                          'aspect-square flex flex-col items-center justify-center rounded-lg text-sm transition-colors',
                          'hover:bg-gray-100',
                          isToday(day) && 'bg-blue-50 text-blue-600 font-semibold',
                          isSelected && 'bg-blue-500 text-white hover:bg-blue-600',
                          !isCurrentMonth && 'text-gray-300',
                          !isSelected && !isToday(day) && isCurrentMonth && 'text-gray-700'
                        )}
                      >
                        <span>{format(day, 'd')}</span>
                        {hasEvents && (
                          <div className="flex gap-0.5 mt-0.5">
                            {dateEvents.slice(0, 3).map((e, i) => (
                              <span
                                key={i}
                                className={clsx(
                                  'h-1 w-1 rounded-full',
                                  isSelected ? 'bg-white' : eventTypeConfig[e.type]?.color || 'bg-gray-400'
                                )}
                              />
                            ))}
                          </div>
                        )}
                      </button>
                    </TooltipTrigger>
                    {hasEvents && (
                      <TooltipContent>
                        <div className="space-y-1">
                          {dateEvents.map((e) => (
                            <p key={e.id} className="text-xs">
                              {e.title}
                            </p>
                          ))}
                        </div>
                      </TooltipContent>
                    )}
                  </Tooltip>
                </TooltipProvider>
              );
            })}
          </div>
        </div>

        {/* Selected date events or upcoming events */}
        <div className="flex-1 mt-4 px-4 border-t pt-3">
          <h4 className="text-xs font-semibold text-gray-500 uppercase tracking-wider mb-2">
            {selectedDate ? format(selectedDate, 'EEEE, MMM d') : 'Upcoming Events'}
          </h4>

          <div className="space-y-2">
            {(selectedDate ? selectedDateEvents : upcomingEvents).length === 0 ? (
              <p className="text-sm text-gray-400 italic">No events</p>
            ) : (
              (selectedDate ? selectedDateEvents : upcomingEvents).map((event) => {
                const config = eventTypeConfig[event.type] || eventTypeConfig.other;
                const Icon = config.icon;

                return (
                  <div
                    key={event.id}
                    className="flex items-center gap-3 p-2 rounded-lg hover:bg-gray-50 transition-colors"
                  >
                    <div className={clsx('p-2 rounded-lg', config.color, 'bg-opacity-10')}>
                      <Icon className={clsx('h-4 w-4', config.color.replace('bg-', 'text-'))} />
                    </div>
                    <div className="flex-1 min-w-0">
                      <p className="text-sm font-medium truncate">{event.title}</p>
                      <p className="text-xs text-gray-400">
                        {format(parseISO(event.start), 'h:mm a')}
                        {!selectedDate && ` · ${format(parseISO(event.start), 'MMM d')}`}
                      </p>
                    </div>
                  </div>
                );
              })
            )}
          </div>

          {selectedDate && (
            <Button
              variant="ghost"
              size="sm"
              className="w-full mt-2"
              onClick={() => setSelectedDate(null)}
            >
              Show upcoming events
            </Button>
          )}
        </div>
      </CardContent>
    </Card>
  );
}

export default CalendarWidget;

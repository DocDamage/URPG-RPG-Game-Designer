'use client';

import React, { useState, useEffect } from 'react';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Badge } from '@/components/ui/badge';
import { ChevronLeft, ChevronRight, Clock, User, MapPin } from 'lucide-react';
import { getShifts, ShiftAssignment } from '@/lib/api/scheduling';
import { useToast } from '@/components/ui/use-toast';

interface ShiftCalendarProps {
  homeId?: string;
  userId?: string;
}

export function ShiftCalendar({ homeId, userId }: ShiftCalendarProps) {
  const [shifts, setShifts] = useState<ShiftAssignment[]>([]);
  const [currentDate, setCurrentDate] = useState(new Date());
  const [loading, setLoading] = useState(true);
  const { toast } = useToast();

  useEffect(() => {
    loadShifts();
  }, [currentDate, homeId, userId]);

  const loadShifts = async () => {
    try {
      setLoading(true);
      const startOfMonth = new Date(currentDate.getFullYear(), currentDate.getMonth(), 1);
      const endOfMonth = new Date(currentDate.getFullYear(), currentDate.getMonth() + 1, 0);

      const response = await getShifts({
        homeId,
        userId,
        startDate: startOfMonth.toISOString(),
        endDate: endOfMonth.toISOString(),
      });

      if (response.success) {
        setShifts(response.data);
      }
    } catch (error) {
      toast({
        title: 'Error',
        description: 'Failed to load shifts',
        variant: 'destructive',
      });
    } finally {
      setLoading(false);
    }
  };

  const getDaysInMonth = () => {
    const year = currentDate.getFullYear();
    const month = currentDate.getMonth();
    const firstDay = new Date(year, month, 1);
    const lastDay = new Date(year, month + 1, 0);
    const daysInMonth = lastDay.getDate();
    const startingDayOfWeek = firstDay.getDay();

    const days: (Date | null)[] = [];

    // Add empty cells for days before the first day of the month
    for (let i = 0; i < startingDayOfWeek; i++) {
      days.push(null);
    }

    // Add all days of the month
    for (let i = 1; i <= daysInMonth; i++) {
      days.push(new Date(year, month, i));
    }

    return days;
  };

  const getShiftsForDay = (date: Date | null) => {
    if (!date) return [];
    const dateStr = date.toISOString().split('T')[0];
    return shifts.filter((shift) => {
      const shiftDate = new Date(shift.startTime).toISOString().split('T')[0];
      return shiftDate === dateStr;
    });
  };

  const getStatusColor = (status: string) => {
    switch (status) {
      case 'scheduled':
        return 'bg-blue-100 text-blue-800';
      case 'confirmed':
        return 'bg-green-100 text-green-800';
      case 'in_progress':
        return 'bg-yellow-100 text-yellow-800';
      case 'completed':
        return 'bg-gray-100 text-gray-800';
      case 'cancelled':
        return 'bg-red-100 text-red-800';
      default:
        return 'bg-gray-100 text-gray-800';
    }
  };

  const navigateMonth = (direction: number) => {
    setCurrentDate(new Date(currentDate.getFullYear(), currentDate.getMonth() + direction, 1));
  };

  const monthNames = [
    'January', 'February', 'March', 'April', 'May', 'June',
    'July', 'August', 'September', 'October', 'November', 'December'
  ];

  const dayNames = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'];

  if (loading) {
    return (
      <Card>
        <CardContent className="p-6">
          <div className="flex items-center justify-center h-64">
            <div className="animate-spin rounded-full h-8 w-8 border-b-2 border-primary"></div>
          </div>
        </CardContent>
      </Card>
    );
  }

  return (
    <Card>
      <CardHeader className="flex flex-row items-center justify-between">
        <CardTitle>Shift Calendar</CardTitle>
        <div className="flex items-center gap-2">
          <Button variant="outline" size="icon" onClick={() => navigateMonth(-1)}>
            <ChevronLeft className="h-4 w-4" />
          </Button>
          <span className="font-medium min-w-[150px] text-center">
            {monthNames[currentDate.getMonth()]} {currentDate.getFullYear()}
          </span>
          <Button variant="outline" size="icon" onClick={() => navigateMonth(1)}>
            <ChevronRight className="h-4 w-4" />
          </Button>
        </div>
      </CardHeader>
      <CardContent>
        <div className="grid grid-cols-7 gap-1">
          {dayNames.map((day) => (
            <div key={day} className="text-center text-sm font-medium text-muted-foreground py-2">
              {day}
            </div>
          ))}
          {getDaysInMonth().map((date, index) => {
            const dayShifts = getShiftsForDay(date);
            const isToday = date?.toDateString() === new Date().toDateString();

            return (
              <div
                key={index}
                className={`min-h-[100px] border rounded-lg p-2 ${
                  date ? 'bg-card' : 'bg-muted'
                } ${isToday ? 'ring-2 ring-primary' : ''}`}
              >
                {date && (
                  <>
                    <div className={`text-sm font-medium mb-1 ${isToday ? 'text-primary' : ''}`}>
                      {date.getDate()}
                    </div>
                    <div className="space-y-1">
                      {dayShifts.slice(0, 3).map((shift) => (
                        <div
                          key={shift.id}
                          className={`text-xs px-2 py-1 rounded cursor-pointer hover:opacity-80 ${getStatusColor(
                            shift.status
                          )}`}
                        >
                          <div className="flex items-center gap-1">
                            <Clock className="h-3 w-3" />
                            {new Date(shift.startTime).toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })}
                          </div>
                        </div>
                      ))}
                      {dayShifts.length > 3 && (
                        <div className="text-xs text-muted-foreground text-center">
                          +{dayShifts.length - 3} more
                        </div>
                      )}
                    </div>
                  </>
                )}
              </div>
            );
          })}
        </div>
      </CardContent>
    </Card>
  );
}

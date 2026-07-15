"use client";

import React, { useState, useEffect, useCallback } from 'react';
import { format, startOfMonth, endOfMonth, addMonths } from 'date-fns';
import { CalendarDays, List, Plus, Clock, AlertCircle } from 'lucide-react';
import { Button } from '@/components/ui/button';
import { Tabs, TabsContent, TabsList, TabsTrigger } from '@/components/ui/tabs';
import { Alert, AlertDescription } from '@/components/ui/alert';
import { CalendarView, CalendarShift } from '@/components/scheduling/CalendarView';
import { ShiftCard } from '@/components/scheduling/ShiftCard';
import { TimeOffRequestForm } from '@/components/scheduling/TimeOffRequestForm';
import schedulingApi from '@/lib/api/scheduling';

export default function SchedulingPage() {
  const [view, setView] = useState<'calendar' | 'list'>('calendar');
  const [shifts, setShifts] = useState<CalendarShift[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [selectedDate, setSelectedDate] = useState<Date>(new Date());
  const [currentMonth, setCurrentMonth] = useState(new Date());

  // Fetch shifts for the current month view
  const fetchShifts = useCallback(async () => {
    setLoading(true);
    setError(null);
    try {
      const startDate = format(startOfMonth(currentMonth), 'yyyy-MM-dd');
      const endDate = format(endOfMonth(currentMonth), 'yyyy-MM-dd');
      
      // Using a placeholder homeId - in production this would come from context
      const homeId = 'default-home';
      
      const response = await schedulingApi.getShifts({
        homeId,
        startDate,
        endDate,
      });
      
      if (response.data) {
        setShifts(response.data);
      }
    } catch (err) {
      console.error('Failed to fetch shifts:', err);
      setError('Failed to load schedule. Please try again.');
    } finally {
      setLoading(false);
    }
  }, [currentMonth]);

  useEffect(() => {
    fetchShifts();
  }, [fetchShifts]);

  const handleShiftClick = (shift: CalendarShift) => {
    console.log('Shift clicked:', shift);
    // Open shift detail modal
  };

  const handleDateClick = (date: Date) => {
    setSelectedDate(date);
  };

  const selectedDateShifts = shifts.filter(
    (shift) => shift.shiftDate === format(selectedDate, 'yyyy-MM-dd')
  );

  return (
    <div className="container mx-auto py-6 space-y-6">
      {/* Header */}
      <div className="flex flex-col sm:flex-row sm:items-center sm:justify-between gap-4">
        <div>
          <h1 className="text-3xl font-bold tracking-tight">Scheduling</h1>
          <p className="text-muted-foreground">
            Manage shifts, time-off requests, and coverage
          </p>
        </div>
        <div className="flex items-center gap-2">
          <TimeOffRequestForm onSuccess={fetchShifts} />
          <Button>
            <Plus className="mr-2 h-4 w-4" />
            Create Shift
          </Button>
        </div>
      </div>

      {/* Error Alert */}
      {error && (
        <Alert variant="destructive">
          <AlertCircle className="h-4 w-4" />
          <AlertDescription>{error}</AlertDescription>
        </Alert>
      )}

      {/* Main Content */}
      <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
        {/* Calendar / List View */}
        <div className="lg:col-span-2 space-y-4">
          <Tabs value={view} onValueChange={(v) => setView(v as 'calendar' | 'list')}>
            <div className="flex items-center justify-between">
              <TabsList>
                <TabsTrigger value="calendar">
                  <CalendarDays className="mr-2 h-4 w-4" />
                  Calendar
                </TabsTrigger>
                <TabsTrigger value="list">
                  <List className="mr-2 h-4 w-4" />
                  List
                </TabsTrigger>
              </TabsList>
            </div>

            <TabsContent value="calendar" className="mt-4">
              <CalendarView
                shifts={shifts}
                onShiftClick={handleShiftClick}
                onDateClick={handleDateClick}
                selectedDate={selectedDate}
              />
            </TabsContent>

            <TabsContent value="list" className="mt-4">
              <div className="space-y-3">
                {loading ? (
                  <div className="text-center py-8 text-muted-foreground">
                    Loading shifts...
                  </div>
                ) : shifts.length === 0 ? (
                  <div className="text-center py-8 text-muted-foreground">
                    No shifts found for this period
                  </div>
                ) : (
                  shifts
                    .sort((a, b) => new Date(a.startTime).getTime() - new Date(b.startTime).getTime())
                    .map((shift) => (
                      <ShiftCard
                        key={shift.id}
                        shift={shift}
                        onClick={handleShiftClick}
                        compact
                      />
                    ))
                )}
              </div>
            </TabsContent>
          </Tabs>
        </div>

        {/* Sidebar */}
        <div className="space-y-6">
          {/* Selected Date Shifts */}
          <div className="bg-white rounded-lg border shadow-sm p-4">
            <h3 className="font-semibold mb-3">
              {format(selectedDate, 'EEEE, MMMM d')}
            </h3>
            {selectedDateShifts.length === 0 ? (
              <p className="text-sm text-muted-foreground">No shifts scheduled</p>
            ) : (
              <div className="space-y-2">
                {selectedDateShifts.map((shift) => (
                  <ShiftCard
                    key={shift.id}
                    shift={shift}
                    onClick={handleShiftClick}
                    compact
                  />
                ))}
              </div>
            )}
          </div>

          {/* Quick Stats */}
          <div className="bg-white rounded-lg border shadow-sm p-4">
            <h3 className="font-semibold mb-3">This Month</h3>
            <div className="space-y-3">
              <div className="flex items-center justify-between">
                <span className="text-sm text-muted-foreground">Total Shifts</span>
                <span className="font-medium">{shifts.length}</span>
              </div>
              <div className="flex items-center justify-between">
                <span className="text-sm text-muted-foreground">Confirmed</span>
                <span className="font-medium">
                  {shifts.filter((s) => s.status === 'confirmed').length}
                </span>
              </div>
              <div className="flex items-center justify-between">
                <span className="text-sm text-muted-foreground">Unassigned</span>
                <span className="font-medium text-yellow-600">
                  {shifts.filter((s) => s.userId === 'UNASSIGNED').length}
                </span>
              </div>
            </div>
          </div>

          {/* Coverage Alerts */}
          <div className="bg-white rounded-lg border shadow-sm p-4">
            <h3 className="font-semibold mb-3 flex items-center gap-2">
              <Clock className="h-4 w-4" />
              Coverage Alerts
            </h3>
            <div className="space-y-2">
              <Alert variant="default" className="bg-yellow-50 border-yellow-200">
                <AlertCircle className="h-4 w-4 text-yellow-600" />
                <AlertDescription className="text-sm">
                  3 uncovered shifts next week
                </AlertDescription>
              </Alert>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}

/**
 * MAR Widget
 * 
 * Today's medication administration schedule.
 */

'use client';

import React, { useState, useMemo } from 'react';
import { clsx } from 'clsx';
import { format, parseISO, isAfter, isBefore, isSameDay, addHours } from 'date-fns';
import {
  Pill,
  Clock,
  CheckCircle2,
  XCircle,
  AlertCircle,
  ChevronRight,
  User,
  Filter,
} from 'lucide-react';
import type { WidgetProps, MarWidgetConfig, MARScheduleEntry } from '../types';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Badge } from '@/components/ui/badge';
import { ScrollArea } from '@/components/ui/scroll-area';
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from '@/components/ui/select';

// Mock MAR data
const mockMARDATA: MARScheduleEntry[] = [
  {
    id: '1',
    individualId: '1',
    individualName: 'John Doe',
    medication: 'Lisinopril',
    dosage: '10mg',
    scheduledTime: new Date(Date.now() - 3600000).toISOString(),
    status: 'administered',
    administeredBy: 'Jane Smith',
    administeredAt: new Date(Date.now() - 3500000).toISOString(),
    notes: 'Taken with water',
  },
  {
    id: '2',
    individualId: '1',
    individualName: 'John Doe',
    medication: 'Metformin',
    dosage: '500mg',
    scheduledTime: new Date(Date.now() - 1800000).toISOString(),
    status: 'pending',
  },
  {
    id: '3',
    individualId: '2',
    individualName: 'Sarah Wilson',
    medication: 'Atorvastatin',
    dosage: '20mg',
    scheduledTime: new Date(Date.now() + 3600000).toISOString(),
    status: 'pending',
  },
  {
    id: '4',
    individualId: '3',
    individualName: 'Mike Johnson',
    medication: 'Amlodipine',
    dosage: '5mg',
    scheduledTime: new Date(Date.now() - 7200000).toISOString(),
    status: 'refused',
    notes: 'Patient declined - feeling nauseous',
  },
  {
    id: '5',
    individualId: '2',
    individualName: 'Sarah Wilson',
    medication: 'Vitamin D',
    dosage: '1000 IU',
    scheduledTime: new Date(Date.now() + 7200000).toISOString(),
    status: 'pending',
  },
  {
    id: '6',
    individualId: '4',
    individualName: 'Emma Davis',
    medication: 'Gabapentin',
    dosage: '300mg',
    scheduledTime: new Date(Date.now() - 10800000).toISOString(),
    status: 'missed',
  },
];

// Status configuration
const statusConfig = {
  pending: {
    icon: Clock,
    color: 'text-yellow-600',
    bgColor: 'bg-yellow-50',
    borderColor: 'border-yellow-200',
    label: 'Pending',
  },
  administered: {
    icon: CheckCircle2,
    color: 'text-green-600',
    bgColor: 'bg-green-50',
    borderColor: 'border-green-200',
    label: 'Administered',
  },
  refused: {
    icon: XCircle,
    color: 'text-orange-600',
    bgColor: 'bg-orange-50',
    borderColor: 'border-orange-200',
    label: 'Refused',
  },
  missed: {
    icon: AlertCircle,
    color: 'text-red-600',
    bgColor: 'bg-red-50',
    borderColor: 'border-red-200',
    label: 'Missed',
  },
};

export function MarWidget({ config, className }: WidgetProps) {
  const marConfig = config as MarWidgetConfig;
  const [filterStatus, setFilterStatus] = useState<string>('all');

  // Filter medications based on config
  const filteredMedications = useMemo(() => {
    let filtered = mockMARDATA;

    // Filter by status
    if (!marConfig.showAdministered) {
      filtered = filtered.filter((m) => m.status !== 'administered');
    }
    if (!marConfig.showPending) {
      filtered = filtered.filter((m) => m.status !== 'pending');
    }
    if (!marConfig.showRefused) {
      filtered = filtered.filter((m) => m.status !== 'refused');
    }

    // Filter by individual
    if (marConfig.individualId) {
      filtered = filtered.filter((m) => m.individualId === marConfig.individualId);
    }

    // Filter by status dropdown
    if (filterStatus !== 'all') {
      filtered = filtered.filter((m) => m.status === filterStatus);
    }

    // Sort by scheduled time
    return filtered.sort(
      (a, b) => new Date(a.scheduledTime).getTime() - new Date(b.scheduledTime).getTime()
    );
  }, [marConfig, filterStatus]);

  // Group by status for summary
  const summary = useMemo(() => {
    return {
      pending: filteredMedications.filter((m) => m.status === 'pending').length,
      administered: filteredMedications.filter((m) => m.status === 'administered').length,
      refused: filteredMedications.filter((m) => m.status === 'refused').length,
      missed: filteredMedications.filter((m) => m.status === 'missed').length,
    };
  }, [filteredMedications]);

  const overdueCount = filteredMedications.filter(
    (m) => m.status === 'pending' && isBefore(parseISO(m.scheduledTime), new Date())
  ).length;

  return (
    <Card className={clsx('h-full flex flex-col', className)}>
      <CardHeader className="pb-3">
        <div className="flex items-center justify-between">
          <CardTitle className="text-lg font-semibold flex items-center gap-2">
            <Pill className="h-5 w-5" />
            {config.title || "Today's Medications"}
          </CardTitle>

          <Select value={filterStatus} onValueChange={setFilterStatus}>
            <SelectTrigger className="w-28 h-8">
              <Filter className="h-3.5 w-3.5 mr-1" />
              <SelectValue />
            </SelectTrigger>
            <SelectContent>
              <SelectItem value="all">All</SelectItem>
              <SelectItem value="pending">Pending</SelectItem>
              <SelectItem value="administered">Given</SelectItem>
              <SelectItem value="refused">Refused</SelectItem>
              <SelectItem value="missed">Missed</SelectItem>
            </SelectContent>
          </Select>
        </div>

        {/* Summary */}
        <div className="flex gap-2 mt-2">
          {summary.pending > 0 && (
            <Badge variant="secondary" className="text-xs">
              {summary.pending} pending
            </Badge>
          )}
          {summary.administered > 0 && (
            <Badge variant="default" className="text-xs bg-green-100 text-green-800 hover:bg-green-100">
              {summary.administered} given
            </Badge>
          )}
          {overdueCount > 0 && (
            <Badge variant="destructive" className="text-xs">
              {overdueCount} overdue
            </Badge>
          )}
        </div>
      </CardHeader>

      <CardContent className="flex-1 p-0">
        <ScrollArea className="h-full px-4">
          <div className="space-y-2 pb-4">
            {filteredMedications.length === 0 ? (
              <div className="text-center py-8 text-gray-400">
                <Pill className="h-8 w-8 mx-auto mb-2 opacity-50" />
                <p className="text-sm">No medications scheduled</p>
              </div>
            ) : (
              filteredMedications.map((med) => {
                const status = statusConfig[med.status];
                const StatusIcon = status.icon;
                const scheduledTime = parseISO(med.scheduledTime);
                const isOverdue =
                  med.status === 'pending' && isBefore(scheduledTime, new Date());

                return (
                  <div
                    key={med.id}
                    className={clsx(
                      'p-3 rounded-lg border transition-all hover:shadow-sm',
                      status.bgColor,
                      status.borderColor,
                      isOverdue && 'ring-2 ring-red-200'
                    )}
                  >
                    <div className="flex items-start justify-between gap-2">
                      <div className="flex-1 min-w-0">
                        <div className="flex items-center gap-2">
                          <StatusIcon className={clsx('h-4 w-4', status.color)} />
                          <span className="font-medium text-sm truncate">
                            {med.medication}
                          </span>
                          {isOverdue && (
                            <Badge variant="destructive" className="text-[10px]">
                              Overdue
                            </Badge>
                          )}
                        </div>

                        <div className="flex items-center gap-2 mt-1 text-xs text-gray-600">
                          <span>{med.dosage}</span>
                          <span>·</span>
                          <span>{format(scheduledTime, 'h:mm a')}</span>
                        </div>

                        <div className="flex items-center gap-1 mt-1.5 text-xs">
                          <User className="h-3 w-3 text-gray-400" />
                          <span className="text-gray-600 truncate">
                            {med.individualName}
                          </span>
                        </div>

                        {med.notes && (
                          <p className="mt-1.5 text-xs text-gray-500 italic">
                            &ldquo;{med.notes}&rdquo;
                          </p>
                        )}

                        {med.status === 'administered' && med.administeredBy && (
                          <p className="mt-1 text-xs text-gray-400">
                            By {med.administeredBy} at{' '}
                            {med.administeredAt
                              ? format(parseISO(med.administeredAt), 'h:mm a')
                              : ''}
                          </p>
                        )}
                      </div>

                      {med.status === 'pending' && (
                        <Button size="sm" className="shrink-0">
                          Administer
                        </Button>
                      )}
                    </div>
                  </div>
                );
              })
            )}
          </div>

          <Button variant="ghost" className="w-full" asChild>
            <a href="/medications">
              View Full MAR
              <ChevronRight className="h-4 w-4 ml-1" />
            </a>
          </Button>
        </ScrollArea>
      </CardContent>
    </Card>
  );
}

export default MarWidget;

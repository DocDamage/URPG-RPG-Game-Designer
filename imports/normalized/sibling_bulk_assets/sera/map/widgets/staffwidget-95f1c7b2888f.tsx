/**
 * Staff Widget
 * 
 * On-duty staff display.
 */

'use client';

import React, { useMemo } from 'react';
import { clsx } from 'clsx';
import { format, parseISO, isBefore, isAfter } from 'date-fns';
import {
  Users,
  User,
  Clock,
  Phone,
  Mail,
  Circle,
  Coffee,
  LogOut,
  CheckCircle2,
  ChevronRight,
} from 'lucide-react';
import type { WidgetProps, StaffWidgetConfig, OnDutyStaff } from '../types';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Badge } from '@/components/ui/badge';
import { ScrollArea } from '@/components/ui/scroll-area';
import {
  Avatar,
  AvatarFallback,
  AvatarImage,
} from '@/components/ui/avatar';
import {
  Tooltip,
  TooltipContent,
  TooltipProvider,
  TooltipTrigger,
} from '@/components/ui/tooltip';

// Mock staff data
const mockStaff: OnDutyStaff[] = [
  {
    id: '1',
    name: 'Jane Smith',
    role: 'DSP',
    avatar: '',
    shiftStart: new Date(Date.now() - 1000 * 60 * 60 * 4).toISOString(),
    shiftEnd: new Date(Date.now() + 1000 * 60 * 60 * 4).toISOString(),
    status: 'available',
    phone: '555-0101',
    email: 'jane.smith@example.com',
  },
  {
    id: '2',
    name: 'Bob Johnson',
    role: 'Lead DSP',
    avatar: '',
    shiftStart: new Date(Date.now() - 1000 * 60 * 60 * 6).toISOString(),
    shiftEnd: new Date(Date.now() + 1000 * 60 * 60 * 2).toISOString(),
    status: 'busy',
    phone: '555-0102',
    email: 'bob.johnson@example.com',
  },
  {
    id: '3',
    name: 'Alice Brown',
    role: 'Nurse',
    avatar: '',
    shiftStart: new Date(Date.now() - 1000 * 60 * 60 * 2).toISOString(),
    shiftEnd: new Date(Date.now() + 1000 * 60 * 60 * 6).toISOString(),
    status: 'available',
    phone: '555-0103',
    email: 'alice.brown@example.com',
  },
  {
    id: '4',
    name: 'Carol Davis',
    role: 'DSP',
    avatar: '',
    shiftStart: new Date(Date.now() - 1000 * 60 * 60 * 1).toISOString(),
    shiftEnd: new Date(Date.now() + 1000 * 60 * 60 * 7).toISOString(),
    status: 'break',
    phone: '555-0104',
    email: 'carol.davis@example.com',
  },
  {
    id: '5',
    name: 'David Wilson',
    role: 'Supervisor',
    avatar: '',
    shiftStart: new Date(Date.now() - 1000 * 60 * 60 * 8).toISOString(),
    shiftEnd: new Date(Date.now() + 1000 * 60 * 30).toISOString(),
    status: 'busy',
    phone: '555-0105',
    email: 'david.wilson@example.com',
  },
];

// Status configuration
const statusConfig = {
  available: {
    icon: CheckCircle2,
    color: 'text-green-500',
    bgColor: 'bg-green-50',
    label: 'Available',
  },
  busy: {
    icon: Circle,
    color: 'text-red-500',
    bgColor: 'bg-red-50',
    label: 'Busy',
  },
  break: {
    icon: Coffee,
    color: 'text-yellow-500',
    bgColor: 'bg-yellow-50',
    label: 'On Break',
  },
  off: {
    icon: LogOut,
    color: 'text-gray-500',
    bgColor: 'bg-gray-50',
    label: 'Off Duty',
  },
};

export function StaffWidget({ config, className }: WidgetProps) {
  const staffConfig = config as StaffWidgetConfig;

  // Filter staff based on config
  const filteredStaff = useMemo(() => {
    let filtered = mockStaff;

    // Filter by roles
    if (staffConfig.roles?.length) {
      filtered = filtered.filter((s) => staffConfig.roles?.includes(s.role));
    }

    // Filter by status
    if (!staffConfig.showOnDuty) {
      filtered = filtered.filter((s) => s.status === 'off');
    }
    if (!staffConfig.showOffDuty) {
      filtered = filtered.filter((s) => s.status !== 'off');
    }

    return filtered;
  }, [staffConfig]);

  // Group by status
  const groupedStaff = useMemo(() => {
    const groups: Record<string, OnDutyStaff[]> = {
      available: [],
      busy: [],
      break: [],
      off: [],
    };

    filteredStaff.forEach((staff) => {
      groups[staff.status].push(staff);
    });

    return groups;
  }, [filteredStaff]);

  const activeStaff = filteredStaff.filter((s) => s.status !== 'off');

  return (
    <Card className={clsx('h-full flex flex-col', className)}>
      <CardHeader className="pb-3">
        <div className="flex items-center justify-between">
          <CardTitle className="text-lg font-semibold flex items-center gap-2">
            <Users className="h-5 w-5" />
            {config.title || 'On-Duty Staff'}
          </CardTitle>
          <Badge variant="secondary" className="text-xs">
            {activeStaff.length} active
          </Badge>
        </div>

        {/* Status summary */}
        <div className="flex flex-wrap gap-1 mt-2">
          {Object.entries(groupedStaff)
            .filter(([_, staff]) => staff.length > 0)
            .map(([status, staff]) => {
              const config = statusConfig[status as keyof typeof statusConfig];
              return (
                <Badge
                  key={status}
                  variant="outline"
                  className={clsx('text-[10px]', config?.color)}
                >
                  {staff.length} {config?.label || status}
                </Badge>
              );
            })}
        </div>
      </CardHeader>

      <CardContent className="flex-1 p-0">
        <ScrollArea className="h-full px-4">
          <div className="space-y-3 pb-4">
            {filteredStaff.length === 0 ? (
              <div className="text-center py-8 text-gray-400">
                <User className="h-8 w-8 mx-auto mb-2 opacity-50" />
                <p className="text-sm">No staff on duty</p>
              </div>
            ) : (
              filteredStaff.map((staff) => {
                const status = statusConfig[staff.status];
                const StatusIcon = status.icon;
                const shiftStart = parseISO(staff.shiftStart);
                const shiftEnd = parseISO(staff.shiftEnd);
                const now = new Date();
                const isShiftEndingSoon = isAfter(shiftEnd, now) && isBefore(shiftEnd, new Date(now.getTime() + 3600000));

                return (
                  <div
                    key={staff.id}
                    className={clsx(
                      'p-3 rounded-lg border transition-all hover:shadow-sm',
                      status.bgColor,
                      'border-gray-200'
                    )}
                  >
                    <div className="flex items-start gap-3">
                      <Avatar className="h-10 w-10 shrink-0">
                        <AvatarImage src={staff.avatar} />
                        <AvatarFallback className="text-sm font-medium">
                          {staff.name.split(' ').map((n) => n[0]).join('')}
                        </AvatarFallback>
                      </Avatar>

                      <div className="flex-1 min-w-0">
                        <div className="flex items-center gap-2">
                          <span className="font-medium text-sm truncate">
                            {staff.name}
                          </span>
                          <StatusIcon className={clsx('h-3.5 w-3.5', status.color)} />
                        </div>

                        <p className="text-xs text-gray-500">{staff.role}</p>

                        {staffConfig.showShiftInfo && (
                          <div className="flex items-center gap-1 mt-1.5 text-xs text-gray-400">
                            <Clock className="h-3 w-3" />
                            <span>
                              {format(shiftStart, 'h:mm a')} - {format(shiftEnd, 'h:mm a')}
                            </span>
                            {isShiftEndingSoon && (
                              <Badge variant="destructive" className="text-[10px] ml-1">
                                Ends soon
                              </Badge>
                            )}
                          </div>
                        )}

                        {staffConfig.showContactInfo && (
                          <div className="flex items-center gap-2 mt-2">
                            {staff.phone && (
                              <TooltipProvider>
                                <Tooltip>
                                  <TooltipTrigger asChild>
                                    <a
                                      href={`tel:${staff.phone}`}
                                      className="p-1 rounded hover:bg-white/50 transition-colors"
                                    >
                                      <Phone className="h-3.5 w-3.5 text-gray-500" />
                                    </a>
                                  </TooltipTrigger>
                                  <TooltipContent>
                                    <p>{staff.phone}</p>
                                  </TooltipContent>
                                </Tooltip>
                              </TooltipProvider>
                            )}
                            {staff.email && (
                              <TooltipProvider>
                                <Tooltip>
                                  <TooltipTrigger asChild>
                                    <a
                                      href={`mailto:${staff.email}`}
                                      className="p-1 rounded hover:bg-white/50 transition-colors"
                                    >
                                      <Mail className="h-3.5 w-3.5 text-gray-500" />
                                    </a>
                                  </TooltipTrigger>
                                  <TooltipContent>
                                    <p>{staff.email}</p>
                                  </TooltipContent>
                                </Tooltip>
                              </TooltipProvider>
                            )}
                          </div>
                        )}
                      </div>
                    </div>
                  </div>
                );
              })
            )}
          </div>

          <Button variant="ghost" className="w-full" asChild>
            <a href="/staff">
              View Full Schedule
              <ChevronRight className="h-4 w-4 ml-1" />
            </a>
          </Button>
        </ScrollArea>
      </CardContent>
    </Card>
  );
}

export default StaffWidget;

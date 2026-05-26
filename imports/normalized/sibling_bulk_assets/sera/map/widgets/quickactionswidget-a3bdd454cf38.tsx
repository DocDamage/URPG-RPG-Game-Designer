/**
 * Quick Actions Widget
 * 
 * Shortcut buttons for common tasks.
 */

'use client';

import React, { useMemo } from 'react';
import Link from 'next/link';
import { clsx } from 'clsx';
import {
  Plus,
  FileText,
  Pill,
  AlertTriangle,
  MessageSquare,
  Calendar,
  ClipboardCheck,
  UserPlus,
  FilePlus,
  Clock,
  Activity,
  Phone,
  Camera,
  Mic,
} from 'lucide-react';
import type { WidgetProps, QuickActionsWidgetConfig } from '../types';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import {
  Tooltip,
  TooltipContent,
  TooltipProvider,
  TooltipTrigger,
} from '@/components/ui/tooltip';

// Action definition
interface QuickAction {
  id: string;
  label: string;
  icon: React.ComponentType<{ className?: string }>;
  href?: string;
  onClick?: () => void;
  color?: string;
  shortcut?: string;
}

// Available actions
const availableActions: QuickAction[] = [
  {
    id: 'log_entry',
    label: 'New Log Entry',
    icon: FileText,
    href: '/logs/new',
    color: 'text-blue-600',
    shortcut: '⌘+L',
  },
  {
    id: 'medication',
    label: 'Medication',
    icon: Pill,
    href: '/medications',
    color: 'text-green-600',
    shortcut: '⌘+M',
  },
  {
    id: 'incident',
    label: 'Report Incident',
    icon: AlertTriangle,
    href: '/incidents/new',
    color: 'text-red-600',
    shortcut: '⌘+I',
  },
  {
    id: 'message',
    label: 'Send Message',
    icon: MessageSquare,
    href: '/messages/new',
    color: 'text-purple-600',
    shortcut: '⌘+Shift+M',
  },
  {
    id: 'schedule',
    label: 'Schedule',
    icon: Calendar,
    href: '/schedule',
    color: 'text-orange-600',
  },
  {
    id: 'assessment',
    label: 'Assessment',
    icon: ClipboardCheck,
    href: '/assessments',
    color: 'text-teal-600',
  },
  {
    id: 'new_individual',
    label: 'New Individual',
    icon: UserPlus,
    href: '/individuals/new',
    color: 'text-indigo-600',
  },
  {
    id: 'document',
    label: 'New Document',
    icon: FilePlus,
    href: '/documents/new',
    color: 'text-pink-600',
  },
  {
    id: 'time_clock',
    label: 'Time Clock',
    icon: Clock,
    href: '/time-clock',
    color: 'text-cyan-600',
  },
  {
    id: 'vitals',
    label: 'Record Vitals',
    icon: Activity,
    href: '/vitals/new',
    color: 'text-rose-600',
  },
  {
    id: 'call',
    label: 'Make Call',
    icon: Phone,
    href: '/calls',
    color: 'text-emerald-600',
  },
  {
    id: 'photo',
    label: 'Take Photo',
    icon: Camera,
    href: '/photos',
    color: 'text-violet-600',
  },
  {
    id: 'voice',
    label: 'Voice Note',
    icon: Mic,
    href: '/voice',
    color: 'text-amber-600',
  },
];

export function QuickActionsWidget({ config, className }: WidgetProps) {
  const actionsConfig = config as QuickActionsWidgetConfig;

  // Filter and limit actions
  const actions = useMemo(() => {
    let filtered = availableActions;

    if (actionsConfig.actions?.length) {
      filtered = availableActions.filter((a) =>
        actionsConfig.actions?.includes(a.id)
      );
    }

    if (actionsConfig.maxActions) {
      filtered = filtered.slice(0, actionsConfig.maxActions);
    }

    return filtered;
  }, [actionsConfig.actions, actionsConfig.maxActions]);

  const layout = actionsConfig.layout || 'grid';
  const showLabels = actionsConfig.showLabels !== false;

  return (
    <Card className={clsx('h-full', className)}>
      {config.display?.showHeader !== false && (
        <CardHeader className="pb-3">
          <CardTitle className="text-lg font-semibold">
            {config.title || 'Quick Actions'}
          </CardTitle>
        </CardHeader>
      )}

      <CardContent className={config.display?.showHeader === false ? 'pt-6' : ''}>
        {layout === 'grid' && (
          <div className="grid grid-cols-3 gap-2">
            {actions.map((action) => (
              <TooltipProvider key={action.id}>
                <Tooltip>
                  <TooltipTrigger asChild>
                    <Button
                      variant="outline"
                      className={clsx(
                        'h-auto flex flex-col items-center justify-center gap-1 py-3 transition-all',
                        'hover:shadow-md hover:scale-105',
                        !showLabels && 'py-4'
                      )}
                      asChild={!!action.href}
                      onClick={action.onClick}
                    >
                      {action.href ? (
                        <Link href={action.href}>
                          <action.icon className={clsx('h-5 w-5', action.color)} />
                          {showLabels && (
                            <span className="text-xs font-medium">{action.label}</span>
                          )}
                        </Link>
                      ) : (
                        <>
                          <action.icon className={clsx('h-5 w-5', action.color)} />
                          {showLabels && (
                            <span className="text-xs font-medium">{action.label}</span>
                          )}
                        </>
                      )}
                    </Button>
                  </TooltipTrigger>
                  <TooltipContent>
                    <p>{action.label}</p>
                    {action.shortcut && (
                      <p className="text-xs text-gray-400">{action.shortcut}</p>
                    )}
                  </TooltipContent>
                </Tooltip>
              </TooltipProvider>
            ))}
          </div>
        )}

        {layout === 'list' && (
          <div className="space-y-1">
            {actions.map((action) => (
              <TooltipProvider key={action.id}>
                <Tooltip>
                  <TooltipTrigger asChild>
                    <Button
                      variant="ghost"
                      className="w-full justify-start gap-3"
                      asChild={!!action.href}
                      onClick={action.onClick}
                    >
                      {action.href ? (
                        <Link href={action.href}>
                          <action.icon className={clsx('h-4 w-4', action.color)} />
                          <span className="text-sm">{action.label}</span>
                        </Link>
                      ) : (
                        <>
                          <action.icon className={clsx('h-4 w-4', action.color)} />
                          <span className="text-sm">{action.label}</span>
                        </>
                      )}
                    </Button>
                  </TooltipTrigger>
                  <TooltipContent>
                    <p>{action.label}</p>
                    {action.shortcut && (
                      <p className="text-xs text-gray-400">{action.shortcut}</p>
                    )}
                  </TooltipContent>
                </Tooltip>
              </TooltipProvider>
            ))}
          </div>
        )}

        {layout === 'compact' && (
          <div className="flex flex-wrap gap-1">
            {actions.map((action) => (
              <TooltipProvider key={action.id}>
                <Tooltip>
                  <TooltipTrigger asChild>
                    <Button
                      variant="outline"
                      size="icon"
                      className="h-9 w-9"
                      asChild={!!action.href}
                      onClick={action.onClick}
                    >
                      {action.href ? (
                        <Link href={action.href}>
                          <action.icon className={clsx('h-4 w-4', action.color)} />
                        </Link>
                      ) : (
                        <action.icon className={clsx('h-4 w-4', action.color)} />
                      )}
                    </Button>
                  </TooltipTrigger>
                  <TooltipContent>
                    <p>{action.label}</p>
                    {action.shortcut && (
                      <p className="text-xs text-gray-400">{action.shortcut}</p>
                    )}
                  </TooltipContent>
                </Tooltip>
              </TooltipProvider>
            ))}
          </div>
        )}
      </CardContent>
    </Card>
  );
}

export default QuickActionsWidget;

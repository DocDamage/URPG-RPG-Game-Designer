'use client';

import React from 'react';
import { Badge } from '@/components/ui/badge';
import type { ActivitySeverity } from '@/types/activity';

interface ActivitySeverityBadgeProps {
  severity: ActivitySeverity;
  className?: string;
}

export function ActivitySeverityBadge({ severity, className }: ActivitySeverityBadgeProps) {
  const variants: Record<ActivitySeverity, { variant: 'default' | 'secondary' | 'destructive' | 'outline'; label: string; className: string }> = {
    info: { 
      variant: 'secondary', 
      label: 'Info',
      className: 'bg-blue-100 text-blue-700 hover:bg-blue-100'
    },
    success: { 
      variant: 'default', 
      label: 'Success',
      className: 'bg-green-100 text-green-700 hover:bg-green-100'
    },
    warning: { 
      variant: 'outline', 
      label: 'Warning',
      className: 'bg-amber-100 text-amber-700 hover:bg-amber-100 border-amber-200'
    },
    error: { 
      variant: 'destructive', 
      label: 'Error',
      className: ''
    },
    critical: { 
      variant: 'destructive', 
      label: 'Critical',
      className: 'bg-red-700 text-white hover:bg-red-700'
    },
  };

  const config = variants[severity];

  return (
    <Badge 
      variant={config.variant} 
      className={`text-xs ${config.className} ${className || ''}`}
    >
      {config.label}
    </Badge>
  );
}

export default ActivitySeverityBadge;

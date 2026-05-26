'use client';

import React from 'react';
import {
  Pill,
  AlertTriangle,
  FileText,
  FileWarning,
  CheckCircle,
  Upload,
  Eye,
  Trash,
  CheckSquare,
  FilePlus,
  GraduationCap,
  PlayCircle,
  AlertCircle,
  LogIn,
  LogOut,
  Switch,
  UserCheck,
  UserMinus,
  CalendarPlus,
  CalendarEdit,
  CalendarX,
  Activity,
  Shield,
  HeartPulse,
  ClipboardCheck,
  Target,
  Send,
  MessageCircle,
  AlertOctagon,
  ShieldCheck,
  PackageX,
  PackagePlus,
  UserPlus,
  UserCog,
  Lock,
  Download,
  FileBarChart,
  LucideIcon,
} from 'lucide-react';
import type { ActivityType, ActivitySeverity } from '@/types/activity';
import { ACTIVITY_TYPE_CONFIGS } from '@/lib/api/activity';

interface ActivityIconProps {
  type: ActivityType;
  severity?: ActivitySeverity;
  size?: 'sm' | 'md' | 'lg';
  className?: string;
}

const iconMap: Record<string, LucideIcon> = {
  pill: Pill,
  'pill-off': Pill,
  'alert-triangle': AlertTriangle,
  'file-text': FileText,
  'file-warning': FileWarning,
  'file-edit': FileText,
  'check-circle': CheckCircle,
  upload: Upload,
  eye: Eye,
  trash: Trash,
  'check-square': CheckSquare,
  'file-plus': FilePlus,
  'graduation-cap': GraduationCap,
  'play-circle': PlayCircle,
  'alert-circle': AlertCircle,
  'log-in': LogIn,
  'log-out': LogOut,
  switch: Switch,
  'user-check': UserCheck,
  'user-minus': UserMinus,
  'calendar-plus': CalendarPlus,
  'calendar-edit': CalendarEdit,
  'calendar-x': CalendarX,
  activity: Activity,
  shield: Shield,
  'heart-pulse': HeartPulse,
  'clipboard-check': ClipboardCheck,
  target: Target,
  send: Send,
  'message-circle': MessageCircle,
  'alert-octagon': AlertOctagon,
  'shield-check': ShieldCheck,
  'package-x': PackageX,
  'package-plus': PackagePlus,
  'user-plus': UserPlus,
  'user-cog': UserCog,
  lock: Lock,
  download: Download,
  'file-bar-chart': FileBarChart,
};

const sizeClasses = {
  sm: 'h-4 w-4',
  md: 'h-5 w-5',
  lg: 'h-6 w-6',
};

const containerSizes = {
  sm: 'h-8 w-8',
  md: 'h-10 w-10',
  lg: 'h-12 w-12',
};

export function ActivityIcon({ type, severity, size = 'md', className = '' }: ActivityIconProps) {
  const config = ACTIVITY_TYPE_CONFIGS[type];
  const IconComponent = iconMap[config?.icon] || Activity;
  
  // Use provided severity or fall back to config default
  const effectiveSeverity = severity || config?.default_severity || 'info';
  
  // Get background color based on severity
  const getBackgroundColor = () => {
    switch (effectiveSeverity) {
      case 'success':
        return 'bg-green-100 text-green-600';
      case 'warning':
        return 'bg-amber-100 text-amber-600';
      case 'error':
        return 'bg-red-100 text-red-600';
      case 'critical':
        return 'bg-red-200 text-red-700';
      default:
        return 'bg-blue-100 text-blue-600';
    }
  };

  return (
    <div
      className={`
        ${containerSizes[size]}
        rounded-full flex items-center justify-center
        ${getBackgroundColor()}
        ${className}
      `}
    >
      <IconComponent className={sizeClasses[size]} />
    </div>
  );
}

export function getActivityColor(type: ActivityType): string {
  return ACTIVITY_TYPE_CONFIGS[type]?.color || '#6b7280';
}

export function getActivityLabel(type: ActivityType): string {
  return ACTIVITY_TYPE_CONFIGS[type]?.label || type.replace(/_/g, ' ');
}

export default ActivityIcon;

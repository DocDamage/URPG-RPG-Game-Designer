/**
 * Auto-Save Indicator
 * 
 * Visual indicator showing the current auto-save status
 * with animation and status messages
 */

'use client';

import React from 'react';
import { DraftStatus } from '../types';
import {
  Check,
  Loader2,
  AlertCircle,
  Cloud,
  CloudOff,
} from 'lucide-react';

interface AutoSaveIndicatorProps {
  status: DraftStatus;
  lastSaved: Date | null;
  hasUnsavedChanges: boolean;
  className?: string;
  showDetails?: boolean;
  isOffline?: boolean;
}

export function AutoSaveIndicator({
  status,
  lastSaved,
  hasUnsavedChanges,
  className = '',
  showDetails = true,
  isOffline = false,
}: AutoSaveIndicatorProps) {
  const getStatusConfig = () => {
    if (isOffline) {
      return {
        icon: CloudOff,
        text: 'Offline - Will sync when connected',
        color: 'text-amber-600',
        bgColor: 'bg-amber-50',
        animate: false,
      };
    }

    switch (status) {
      case 'saving':
        return {
          icon: Loader2,
          text: 'Saving...',
          color: 'text-blue-600',
          bgColor: 'bg-blue-50',
          animate: true,
        };
      case 'saved':
        return {
          icon: Check,
          text: 'Saved',
          color: 'text-green-600',
          bgColor: 'bg-green-50',
          animate: false,
        };
      case 'error':
        return {
          icon: AlertCircle,
          text: 'Save failed - will retry',
          color: 'text-red-600',
          bgColor: 'bg-red-50',
          animate: false,
        };
      default:
        return {
          icon: Cloud,
          text: hasUnsavedChanges ? 'Unsaved changes' : 'Up to date',
          color: hasUnsavedChanges ? 'text-gray-500' : 'text-gray-400',
          bgColor: 'bg-gray-50',
          animate: false,
        };
    }
  };

  const config = getStatusConfig();
  const Icon = config.icon;

  const formatLastSaved = (date: Date | null): string => {
    if (!date) return '';
    
    const now = new Date();
    const diff = now.getTime() - date.getTime();
    const seconds = Math.floor(diff / 1000);
    const minutes = Math.floor(seconds / 60);
    const hours = Math.floor(minutes / 60);

    if (seconds < 60) {
      return 'Just now';
    } else if (minutes < 60) {
      return `${minutes}m ago`;
    } else if (hours < 24) {
      return `${hours}h ago`;
    } else {
      return date.toLocaleDateString();
    }
  };

  return (
    <div
      className={`
        inline-flex items-center space-x-2 px-3 py-1.5 rounded-full
        transition-all duration-200
        ${config.bgColor}
        ${className}
      `}
      title={lastSaved ? `Last saved: ${lastSaved.toLocaleString()}` : undefined}
    >
      <Icon
        className={`
          w-4 h-4
          ${config.color}
          ${config.animate ? 'animate-spin' : ''}
        `}
      />
      {showDetails && (
        <div className="flex items-center space-x-2">
          <span className={`text-sm font-medium ${config.color}`}>
            {config.text}
          </span>
          {lastSaved && status !== 'saving' && (
            <span className="text-xs text-gray-400">
              • {formatLastSaved(lastSaved)}
            </span>
          )}
        </div>
      )}
    </div>
  );
}

// Compact version for tight spaces
export function AutoSaveIndicatorCompact({
  status,
  lastSaved,
  hasUnsavedChanges,
  className = '',
  isOffline = false,
}: AutoSaveIndicatorProps) {
  const getStatusColor = () => {
    if (isOffline) return 'text-amber-500';
    switch (status) {
      case 'saving':
        return 'text-blue-500';
      case 'saved':
        return 'text-green-500';
      case 'error':
        return 'text-red-500';
      default:
        return hasUnsavedChanges ? 'text-gray-400' : 'text-gray-300';
    }
  };

  const getIcon = () => {
    if (isOffline) return CloudOff;
    switch (status) {
      case 'saving':
        return Loader2;
      case 'saved':
        return Check;
      case 'error':
        return AlertCircle;
      default:
        return Cloud;
    }
  };

  const Icon = getIcon();
  const color = getStatusColor();

  return (
    <div
      className={`inline-flex items-center ${className}`}
      title={lastSaved ? `Last saved: ${lastSaved.toLocaleString()}` : undefined}
    >
      <Icon
        className={`
          w-4 h-4
          ${color}
          ${status === 'saving' ? 'animate-spin' : ''}
        `}
      />
    </div>
  );
}

// Inline version for form headers
export function AutoSaveIndicatorInline({
  status,
  lastSaved,
  hasUnsavedChanges,
  className = '',
}: AutoSaveIndicatorProps) {
  const getStatusText = () => {
    switch (status) {
      case 'saving':
        return 'Saving...';
      case 'saved':
        return lastSaved ? `Saved at ${lastSaved.toLocaleTimeString()}` : 'Saved';
      case 'error':
        return 'Save failed';
      default:
        return hasUnsavedChanges ? 'Unsaved changes' : '';
    }
  };

  const getStatusColor = () => {
    switch (status) {
      case 'saving':
        return 'text-blue-600';
      case 'saved':
        return 'text-green-600';
      case 'error':
        return 'text-red-600';
      default:
        return hasUnsavedChanges ? 'text-amber-600' : 'text-gray-400';
    }
  };

  const text = getStatusText();
  if (!text) return null;

  return (
    <span className={`text-sm ${getStatusColor()} ${className}`}>
      {text}
    </span>
  );
}

export default AutoSaveIndicator;

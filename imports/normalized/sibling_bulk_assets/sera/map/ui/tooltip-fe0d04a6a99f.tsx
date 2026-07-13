/**
 * Tooltip Component
 * Simple tooltip for UI elements
 */

import React, { useState } from 'react';
import { clsx } from 'clsx';

interface TooltipProviderProps {
  children: React.ReactNode;
}

export function TooltipProvider({ children }: TooltipProviderProps) {
  return <>{children}</>;
}

interface TooltipProps {
  children: React.ReactNode;
}

export function Tooltip({ children }: TooltipProps) {
  return <>{children}</>;
}

interface TooltipTriggerProps {
  children: React.ReactNode;
  asChild?: boolean;
}

export function TooltipTrigger({ children, asChild }: TooltipTriggerProps) {
  if (asChild && React.isValidElement(children)) {
    return children;
  }
  return <>{children}</>;
}

interface TooltipContentProps {
  children: React.ReactNode;
  className?: string;
}

export function TooltipContent({ children, className }: TooltipContentProps) {
  return (
    <div className={clsx(
      'z-50 px-2 py-1 text-xs bg-gray-900 text-white rounded shadow-lg',
      className
    )}>
      {children}
    </div>
  );
}

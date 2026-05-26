/**
 * ScrollArea Component
 * Scrollable content area with custom scrollbar
 */

import React, { forwardRef } from 'react';
import { clsx } from 'clsx';

interface ScrollAreaProps {
  children: React.ReactNode;
  className?: string;
  style?: React.CSSProperties;
}

export const ScrollArea = forwardRef<HTMLDivElement, ScrollAreaProps>(
  ({ children, className, style }, ref) => {
    return (
      <div
        ref={ref}
        className={clsx(
          'overflow-auto scrollbar-thin scrollbar-thumb-gray-300 scrollbar-track-transparent',
          'dark:scrollbar-thumb-gray-600',
          className
        )}
        style={style}
      >
        {children}
      </div>
    );
  }
);

ScrollArea.displayName = 'ScrollArea';

/**
 * UndoToast Component
 * 
 * Toast notification component with undo functionality.
 * Supports countdown timer, auto-dismiss, and multiple toasts.
 */

'use client';

import React, { useState, useEffect, useCallback, useRef } from 'react';
import { Undo2, X, AlertCircle, CheckCircle2, Info, AlertTriangle } from 'lucide-react';
import { UndoCommand, UndoToast as UndoToastType } from './types';
import { getUndoManager } from './undoManager';

// ============================================================================
// Types
// ============================================================================

export interface UndoToastProps {
  /** Toast data */
  toast: UndoToastType;
  
  /** Callback when undo is clicked */
  onUndo?: (command: UndoCommand) => Promise<void>;
  
  /** Callback when dismissed */
  onDismiss?: (toastId: string) => void;
  
  /** Custom styles */
  className?: string;
}

export interface UndoToastContainerProps {
  /** Maximum number of toasts to show */
  maxToasts?: number;
  
  /** Default timeout in ms */
  defaultTimeout?: number;
  
  /** Position on screen */
  position?: 'top-left' | 'top-right' | 'bottom-left' | 'bottom-right' | 'top-center' | 'bottom-center';
  
  /** Custom styles */
  className?: string;
}

// ============================================================================
// Individual Toast Component
// ============================================================================

export const UndoToastItem: React.FC<UndoToastProps> = ({
  toast,
  onUndo,
  onDismiss,
  className = '',
}) => {
  const [countdown, setCountdown] = useState(toast.countdown);
  const [isUndoing, setIsUndoing] = useState(false);
  const [isDismissing, setIsDismissing] = useState(false);
  const timerRef = useRef<NodeJS.Timeout | null>(null);
  const undoManager = getUndoManager();

  // Get icon based on type
  const getIcon = () => {
    switch (toast.type) {
      case 'success':
        return <CheckCircle2 className="w-5 h-5 text-green-500" />;
      case 'warning':
        return <AlertTriangle className="w-5 h-5 text-yellow-500" />;
      case 'info':
      default:
        return <Info className="w-5 h-5 text-blue-500" />;
    }
  };

  // Get background color based on type
  const getBackgroundColor = () => {
    switch (toast.type) {
      case 'success':
        return 'bg-green-50 border-green-200';
      case 'warning':
        return 'bg-yellow-50 border-yellow-200';
      case 'info':
      default:
        return 'bg-white border-gray-200';
    }
  };

  // Handle countdown
  useEffect(() => {
    if (toast.persistent || toast.command === null) return;

    timerRef.current = setInterval(() => {
      setCountdown((prev) => {
        if (prev <= 100) {
          // Time's up, dismiss
          handleDismiss();
          return 0;
        }
        return prev - 100;
      });
    }, 100);

    return () => {
      if (timerRef.current) {
        clearInterval(timerRef.current);
      }
    };
  }, [toast.persistent, toast.command]);

  // Handle undo
  const handleUndo = async () => {
    if (!toast.command || isUndoing) return;

    setIsUndoing(true);
    
    try {
      if (onUndo) {
        await onUndo(toast.command);
      } else {
        await undoManager.undo();
      }
      
      // Call the toast's onUndo callback
      toast.onUndo?.();
      
      // Dismiss after successful undo
      handleDismiss();
    } catch (error) {
      console.error('Undo failed:', error);
      // Show error state or retry
    } finally {
      setIsUndoing(false);
    }
  };

  // Handle dismiss
  const handleDismiss = useCallback(() => {
    if (isDismissing) return;
    
    setIsDismissing(true);
    
    // Clear timer
    if (timerRef.current) {
      clearInterval(timerRef.current);
    }

    // Animation delay before actual dismissal
    setTimeout(() => {
      toast.onDismiss?.();
      onDismiss?.(toast.id);
    }, 300);
  }, [toast, onDismiss, isDismissing]);

  // Calculate progress percentage
  const progressPercent = toast.persistent ? 100 : (countdown / toast.timeout) * 100;

  return (
    <div
      className={`
        relative overflow-hidden rounded-lg border shadow-lg 
        transition-all duration-300 ease-in-out
        ${getBackgroundColor()}
        ${isDismissing ? 'opacity-0 translate-x-full' : 'opacity-100 translate-x-0'}
        ${className}
      `}
      role="alert"
      aria-live="polite"
    >
      {/* Progress bar */}
      {!toast.persistent && (
        <div
          className="absolute bottom-0 left-0 h-1 bg-blue-500 transition-all duration-100"
          style={{ width: `${progressPercent}%` }}
        />
      )}

      <div className="p-4">
        <div className="flex items-start gap-3">
          {/* Icon */}
          <div className="flex-shrink-0 mt-0.5">
            {getIcon()}
          </div>

          {/* Content */}
          <div className="flex-1 min-w-0">
            <p className="text-sm font-medium text-gray-900">
              {toast.message}
            </p>
            
            {toast.command && (
              <p className="text-xs text-gray-500 mt-1">
                {toast.command.description}
              </p>
            )}
          </div>

          {/* Actions */}
          <div className="flex items-center gap-2">
            {toast.command && (
              <button
                onClick={handleUndo}
                disabled={isUndoing}
                className="
                  flex items-center gap-1.5 px-3 py-1.5
                  text-sm font-medium text-blue-600 
                  bg-blue-50 hover:bg-blue-100 
                  rounded-md transition-colors
                  disabled:opacity-50 disabled:cursor-not-allowed
                "
              >
                <Undo2 className={`w-4 h-4 ${isUndoing ? 'animate-spin' : ''}`} />
                {isUndoing ? 'Undoing...' : 'Undo'}
              </button>
            )}

            <button
              onClick={handleDismiss}
              className="
                p-1.5 text-gray-400 hover:text-gray-600
                hover:bg-gray-100 rounded-md transition-colors
              "
              aria-label="Dismiss"
            >
              <X className="w-4 h-4" />
            </button>
          </div>
        </div>

        {/* Countdown text */}
        {!toast.persistent && toast.command && (
          <p className="text-xs text-gray-400 mt-2 text-right">
            Auto-dismiss in {Math.ceil(countdown / 1000)}s
          </p>
        )}
      </div>
    </div>
  );
};

// ============================================================================
// Toast Container Component
// ============================================================================

export const UndoToastContainer: React.FC<UndoToastContainerProps> = ({
  maxToasts = 5,
  defaultTimeout = 5000,
  position = 'bottom-right',
  className = '',
}) => {
  const [toasts, setToasts] = useState<UndoToastType[]>([]);
  const undoManager = getUndoManager();

  // Position classes
  const positionClasses = {
    'top-left': 'top-4 left-4',
    'top-right': 'top-4 right-4',
    'top-center': 'top-4 left-1/2 -translate-x-1/2',
    'bottom-left': 'bottom-4 left-4',
    'bottom-right': 'bottom-4 right-4',
    'bottom-center': 'bottom-4 left-1/2 -translate-x-1/2',
  };

  // Add a new toast
  const addToast = useCallback((
    message: string,
    command: UndoCommand | null = null,
    options: {
      type?: UndoToastType['type'];
      timeout?: number;
      persistent?: boolean;
      onUndo?: () => void;
      onDismiss?: () => void;
    } = {}
  ): string => {
    const id = `${Date.now()}-${Math.random().toString(36).substr(2, 9)}`;
    const timeout = options.timeout ?? defaultTimeout;
    
    const newToast: UndoToastType = {
      id,
      message,
      command,
      timeout,
      countdown: timeout,
      persistent: options.persistent ?? false,
      type: options.type ?? 'info',
      onUndo: options.onUndo,
      onDismiss: options.onDismiss,
    };

    setToasts((prev) => {
      // Remove oldest if at max
      const newToasts = [...prev, newToast];
      if (newToasts.length > maxToasts) {
        return newToasts.slice(newToasts.length - maxToasts);
      }
      return newToasts;
    });

    return id;
  }, [defaultTimeout, maxToasts]);

  // Remove a toast
  const removeToast = useCallback((toastId: string) => {
    setToasts((prev) => prev.filter((t) => t.id !== toastId));
  }, []);

  // Show toast for a command
  const showCommandToast = useCallback((
    message: string,
    command: UndoCommand,
    options?: {
      timeout?: number;
      type?: UndoToastType['type'];
    }
  ) => {
    return addToast(message, command, {
      type: options?.type ?? 'success',
      timeout: options?.timeout ?? defaultTimeout,
      onUndo: () => {
        console.log('Undo completed for:', command.description);
      },
    });
  }, [addToast, defaultTimeout]);

  // Show success toast
  const showSuccess = useCallback((
    message: string,
    command?: UndoCommand,
    timeout?: number
  ) => {
    return addToast(message, command ?? null, {
      type: 'success',
      timeout: timeout ?? defaultTimeout,
    });
  }, [addToast, defaultTimeout]);

  // Show info toast
  const showInfo = useCallback((
    message: string,
    command?: UndoCommand,
    timeout?: number
  ) => {
    return addToast(message, command ?? null, {
      type: 'info',
      timeout: timeout ?? defaultTimeout,
    });
  }, [addToast, defaultTimeout]);

  // Show warning toast
  const showWarning = useCallback((
    message: string,
    command?: UndoCommand,
    timeout?: number
  ) => {
    return addToast(message, command ?? null, {
      type: 'warning',
      timeout: timeout ?? defaultTimeout,
    });
  }, [addToast, defaultTimeout]);

  // Show persistent toast (no auto-dismiss)
  const showPersistent = useCallback((
    message: string,
    command?: UndoCommand
  ) => {
    return addToast(message, command ?? null, {
      type: 'info',
      persistent: true,
    });
  }, [addToast]);

  // Clear all toasts
  const clearAll = useCallback(() => {
    setToasts([]);
  }, []);

  // Subscribe to undo manager for automatic toasts
  useEffect(() => {
    const unsubscribe = undoManager.subscribe((state) => {
      const lastCommand = state.undoStack[state.undoStack.length - 1];
      if (lastCommand) {
        // Optionally auto-show toast for new commands
        // Uncomment to enable:
        // addToast(
        //   `Action completed: ${lastCommand.description}`,
        //   lastCommand,
        //   { type: 'success' }
        // );
      }
    });

    return unsubscribe;
  }, [undoManager]);

  // Expose methods via ref
  useEffect(() => {
    (window as any).undoToast = {
      add: addToast,
      showSuccess,
      showInfo,
      showWarning,
      showPersistent,
      showCommand: showCommandToast,
      remove: removeToast,
      clearAll,
    };

    return () => {
      delete (window as any).undoToast;
    };
  }, [addToast, showSuccess, showInfo, showWarning, showPersistent, showCommandToast, removeToast, clearAll]);

  return (
    <div
      className={`
        fixed z-50 flex flex-col gap-2
        ${positionClasses[position]}
        ${className}
      `}
      style={{ maxWidth: '400px', width: '100%' }}
    >
      {toasts.map((toast) => (
        <UndoToastItem
          key={toast.id}
          toast={toast}
          onDismiss={removeToast}
        />
      ))}
    </div>
  );
};

// ============================================================================
// Hook for using toast methods
// ============================================================================

export function useUndoToast() {
  const addToast = useCallback((
    message: string,
    command?: UndoCommand,
    options?: Parameters<typeof window['undoToast']['add']>[2]
  ) => {
    if (typeof window !== 'undefined' && (window as any).undoToast) {
      return (window as any).undoToast.add(message, command ?? null, options);
    }
    console.warn('UndoToastContainer not mounted');
    return '';
  }, []);

  const showSuccess = useCallback((message: string, command?: UndoCommand) => {
    if (typeof window !== 'undefined' && (window as any).undoToast) {
      return (window as any).undoToast.showSuccess(message, command);
    }
    console.warn('UndoToastContainer not mounted');
    return '';
  }, []);

  const showInfo = useCallback((message: string, command?: UndoCommand) => {
    if (typeof window !== 'undefined' && (window as any).undoToast) {
      return (window as any).undoToast.showInfo(message, command);
    }
    console.warn('UndoToastContainer not mounted');
    return '';
  }, []);

  const showWarning = useCallback((message: string, command?: UndoCommand) => {
    if (typeof window !== 'undefined' && (window as any).undoToast) {
      return (window as any).undoToast.showWarning(message, command);
    }
    console.warn('UndoToastContainer not mounted');
    return '';
  }, []);

  const showPersistent = useCallback((message: string, command?: UndoCommand) => {
    if (typeof window !== 'undefined' && (window as any).undoToast) {
      return (window as any).undoToast.showPersistent(message, command);
    }
    console.warn('UndoToastContainer not mounted');
    return '';
  }, []);

  const removeToast = useCallback((toastId: string) => {
    if (typeof window !== 'undefined' && (window as any).undoToast) {
      (window as any).undoToast.remove(toastId);
    }
  }, []);

  const clearAll = useCallback(() => {
    if (typeof window !== 'undefined' && (window as any).undoToast) {
      (window as any).undoToast.clearAll();
    }
  }, []);

  return {
    addToast,
    showSuccess,
    showInfo,
    showWarning,
    showPersistent,
    removeToast,
    clearAll,
  };
}

export default UndoToastContainer;

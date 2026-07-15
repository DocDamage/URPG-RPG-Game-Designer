/**
 * QuickActionsFAB Component
 * Floating Action Button with expandable action menu
 */

'use client';

import React, { useState, useCallback, useEffect, useRef } from 'react';
import { clsx } from 'clsx';
import { Plus, X, Command } from 'lucide-react';
import { ActionMenu } from './ActionMenu';
import { useQuickActions } from './useQuickActions';
import {
  QuickAction,
  UserRole,
  QuickActionsConfig,
  DEFAULT_CONFIG,
} from './types';
import { Tooltip, TooltipTrigger, TooltipContent } from '../ui/tooltip';

interface QuickActionsFABProps {
  role: UserRole;
  individualId?: string;
  homeId?: string;
  config?: Partial<QuickActionsConfig>;
  onActionSelect?: (action: QuickAction) => void;
  onModalOpen?: (modalName: string, context?: any) => void;
  className?: string;
}

export function QuickActionsFAB({
  role,
  individualId,
  homeId,
  config = {},
  onActionSelect,
  onModalOpen,
  className,
}: QuickActionsFABProps) {
  const mergedConfig = { ...DEFAULT_CONFIG, ...config };
  
  // State
  const [isOpen, setIsOpen] = useState(false);
  const [isAnimating, setIsAnimating] = useState(false);
  const [activeModal, setActiveModal] = useState<string | null>(null);
  const fabRef = useRef<HTMLDivElement>(null);
  const menuRef = useRef<HTMLDivElement>(null);

  // Use quick actions hook
  const {
    availableActions,
    recentActions,
    pinnedActions,
    groupedActions,
    executeAction,
    pinAction,
    unpinAction,
  } = useQuickActions({
    role,
    individualId,
    homeId,
    config: mergedConfig,
  });

  // Handle FAB click
  const handleFabClick = useCallback(() => {
    if (isAnimating) return;
    
    setIsAnimating(true);
    setIsOpen(prev => !prev);
    
    setTimeout(() => {
      setIsAnimating(false);
    }, mergedConfig.animationDuration);
  }, [isAnimating, mergedConfig.animationDuration]);

  // Close menu
  const handleClose = useCallback(() => {
    setIsAnimating(true);
    setIsOpen(false);
    
    setTimeout(() => {
      setIsAnimating(false);
    }, mergedConfig.animationDuration);
  }, [mergedConfig.animationDuration]);

  // Handle action click
  const handleActionClick = useCallback(
    async (action: QuickAction) => {
      // Record action execution
      await executeAction(action.id, { individualId, homeId });

      // Call onActionSelect callback
      onActionSelect?.(action);

      // Handle modal or navigation
      if (action.modal) {
        setActiveModal(action.modal);
        onModalOpen?.(action.modal, { individualId, homeId });
      } else if (action.href) {
        // Navigation handled by parent/router
        handleClose();
      }

      // Close menu after action
      handleClose();
    },
    [executeAction, individualId, homeId, onActionSelect, onModalOpen, handleClose]
  );

  // Handle keyboard shortcuts
  useEffect(() => {
    if (!mergedConfig.enableKeyboardShortcuts) return;

    const handleKeyDown = (e: KeyboardEvent) => {
      // ESC to close
      if (e.key === 'Escape' && isOpen) {
        handleClose();
        return;
      }

      // Quick open shortcut (Ctrl/Cmd + Shift + A)
      if ((e.ctrlKey || e.metaKey) && e.shiftKey && e.key === 'A') {
        e.preventDefault();
        handleFabClick();
        return;
      }

      // Individual action shortcuts (only when menu is open)
      if (isOpen) {
        availableActions.forEach(action => {
          if (action.shortcut) {
            const keys = action.shortcut.split('+');
            const key = keys.pop()?.toLowerCase();
            const needsCtrl = keys.includes('Ctrl');
            const needsShift = keys.includes('Shift');
            const needsAlt = keys.includes('Alt');

            if (
              e.key.toLowerCase() === key &&
              e.ctrlKey === needsCtrl &&
              e.shiftKey === needsShift &&
              e.altKey === needsAlt
            ) {
              e.preventDefault();
              handleActionClick(action);
            }
          }
        });
      }
    };

    document.addEventListener('keydown', handleKeyDown);
    return () => document.removeEventListener('keydown', handleKeyDown);
  }, [isOpen, availableActions, handleClose, handleFabClick, handleActionClick, mergedConfig.enableKeyboardShortcuts]);

  // Handle click outside to close
  useEffect(() => {
    if (!isOpen) return;

    const handleClickOutside = (e: MouseEvent) => {
      if (
        fabRef.current &&
        !fabRef.current.contains(e.target as Node) &&
        menuRef.current &&
        !menuRef.current.contains(e.target as Node)
      ) {
        handleClose();
      }
    };

    document.addEventListener('mousedown', handleClickOutside);
    return () => document.removeEventListener('mousedown', handleClickOutside);
  }, [isOpen, handleClose]);

  // Handle touch events for mobile
  useEffect(() => {
    if (!isOpen) return;

    const handleTouchStart = (e: TouchEvent) => {
      if (
        fabRef.current &&
        !fabRef.current.contains(e.target as Node) &&
        menuRef.current &&
        !menuRef.current.contains(e.target as Node)
      ) {
        handleClose();
      }
    };

    document.addEventListener('touchstart', handleTouchStart);
    return () => document.removeEventListener('touchstart', handleTouchStart);
  }, [isOpen, handleClose]);

  // Get position classes
  const getPositionClasses = () => {
    switch (mergedConfig.position) {
      case 'bottom-left':
        return 'left-6 bottom-6';
      case 'top-right':
        return 'right-6 top-6';
      case 'top-left':
        return 'left-6 top-6';
      case 'bottom-right':
      default:
        return 'right-6 bottom-6';
    }
  };

  const fabContent = (
    <div
      ref={fabRef}
      className={clsx(
        'fixed z-50 flex flex-col items-end',
        getPositionClasses(),
        className
      )}
    >
      {/* Menu */}
      <div ref={menuRef}>
        <ActionMenu
          isOpen={isOpen}
          onClose={handleClose}
          actions={availableActions}
          recentActions={recentActions}
          pinnedActions={pinnedActions}
          groupedActions={groupedActions}
          userRole={role}
          menuType={mergedConfig.menuType}
          enableSearch={mergedConfig.enableSearch}
          enableTooltips={mergedConfig.enableTooltips}
          showLabels={true}
          selectedIndividualId={individualId}
          onActionClick={handleActionClick}
          onPinAction={pinAction}
          onUnpinAction={unpinAction}
          animationDuration={mergedConfig.animationDuration}
        />
      </div>

      {/* Backdrop */}
      {isOpen && (
        <div
          className="fixed inset-0 bg-black/20 backdrop-blur-sm z-[-1] animate-in fade-in duration-200"
          style={{ animationDuration: `${mergedConfig.animationDuration}ms` }}
          onClick={handleClose}
          aria-hidden="true"
        />
      )}

      {/* FAB Button */}
      <Tooltip>
        <TooltipTrigger asChild>
          <button
            onClick={handleFabClick}
            className={clsx(
              'relative w-14 h-14 rounded-full shadow-lg flex items-center justify-center',
              'transition-all duration-300 ease-out',
              'focus:outline-none focus:ring-4 focus:ring-blue-500/30',
              'bg-blue-600 hover:bg-blue-700 text-white',
              isOpen && 'bg-gray-700 hover:bg-gray-800 rotate-45',
              isAnimating && 'scale-95'
            )}
            style={{
              boxShadow: isOpen
                ? '0 10px 25px -5px rgba(0, 0, 0, 0.3)'
                : '0 10px 25px -5px rgba(37, 99, 235, 0.4)',
            }}
            aria-expanded={isOpen}
            aria-haspopup="true"
            aria-label={isOpen ? 'Close quick actions' : 'Open quick actions'}
          >
            {/* Icon */}
            <span className={clsx(
              'transition-transform duration-300',
              isOpen ? 'rotate-0' : 'rotate-0'
            )}>
              {isOpen ? (
                <X className="w-6 h-6" />
              ) : (
                <Plus className="w-6 h-6" />
              )}
            </span>

            {/* Ripple effect */}
            {!isOpen && (
              <span className="absolute inset-0 rounded-full animate-ping bg-blue-400 opacity-20" />
            )}

            {/* Badge for unread/high priority items */}
            {!isOpen && pinnedActions.some(a => a.badge) && (
              <span className="absolute -top-1 -right-1 w-5 h-5 bg-red-500 text-white text-xs font-bold rounded-full flex items-center justify-center animate-bounce">
                !
              </span>
            )}
          </button>
        </TooltipTrigger>
        <TooltipContent side="left">
          <div className="flex items-center gap-2">
            <span>{isOpen ? 'Close' : 'Quick Actions'}</span>
            <kbd className="hidden sm:inline-block px-1.5 py-0.5 text-[10px] font-mono bg-gray-700 rounded">
              ⌘⇧A
            </kbd>
          </div>
        </TooltipContent>
      </Tooltip>
    </div>
  );

  return fabContent;
}

// Convenience wrapper with modal management
interface QuickActionsFABWithModalsProps extends QuickActionsFABProps {
  renderModal: (modalName: string, props: {
    isOpen: boolean;
    onClose: () => void;
    individualId?: string;
    onSuccess?: () => void;
  }) => React.ReactNode;
}

export function QuickActionsFABWithModals({
  renderModal,
  ...props
}: QuickActionsFABWithModalsProps) {
  const [activeModal, setActiveModal] = useState<{
    name: string;
    context?: any;
  } | null>(null);

  const handleModalOpen = useCallback((modalName: string, context?: any) => {
    setActiveModal({ name: modalName, context });
  }, []);

  const handleModalClose = useCallback(() => {
    setActiveModal(null);
  }, []);

  const handleSuccess = useCallback(() => {
    setActiveModal(null);
    // Could trigger toast notification here
  }, []);

  return (
    <>
      <QuickActionsFAB
        {...props}
        onModalOpen={handleModalOpen}
      />
      
      {activeModal && renderModal(activeModal.name, {
        isOpen: true,
        onClose: handleModalClose,
        individualId: activeModal.context?.individualId,
        onSuccess: handleSuccess,
      })}
    </>
  );
}

export default QuickActionsFAB;

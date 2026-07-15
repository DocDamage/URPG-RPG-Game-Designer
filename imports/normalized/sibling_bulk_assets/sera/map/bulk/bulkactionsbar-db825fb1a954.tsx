"use client";

import React, { useCallback } from "react";
import { clsx } from "clsx";
import {
  Trash2,
  Download,
  UserPlus,
  CheckCircle,
  X,
  CheckSquare,
  Square,
  Loader2,
} from "lucide-react";
import { Button } from "../ui/button";

export type BulkActionType = "delete" | "export" | "assign" | "update-status" | "custom";

export interface BulkAction {
  /** Unique identifier for the action */
  id: string;
  /** Display label */
  label: string;
  /** Action type */
  type: BulkActionType;
  /** Icon component */
  icon?: React.ReactNode;
  /** Whether the action is destructive */
  destructive?: boolean;
  /** Whether the action requires confirmation */
  requiresConfirmation?: boolean;
  /** Confirmation message */
  confirmationMessage?: string;
  /** Whether the action is disabled */
  disabled?: boolean;
  /** Custom handler (if not using default) */
  onClick?: () => void;
}

export interface BulkActionsBarProps {
  /** Number of selected items */
  selectedCount: number;
  /** Total number of items available */
  totalCount?: number;
  /** Whether all items are selected */
  isAllSelected?: boolean;
  /** Whether some items are selected (indeterminate state) */
  isIndeterminate?: boolean;
  /** Available bulk actions */
  actions?: BulkAction[];
  /** Callback when an action is triggered */
  onAction?: (action: BulkAction) => void;
  /** Callback to select all items */
  onSelectAll?: () => void;
  /** Callback to clear selection */
  onClearSelection?: () => void;
  /** Callback to toggle select all */
  onToggleSelectAll?: () => void;
  /** Whether the bar is in a loading state */
  isLoading?: boolean;
  /** Loading message */
  loadingMessage?: string;
  /** Additional CSS classes */
  className?: string;
  /** Custom element to display on the left side */
  leftElement?: React.ReactNode;
  /** Custom element to display on the right side */
  rightElement?: React.ReactNode;
  /** Entity name (e.g., "users", "records") */
  entityName?: string;
  /** Whether the bar is sticky */
  sticky?: boolean;
  /** Sticky offset from top */
  stickyOffset?: number;
  /** Whether to show the select all toggle */
  showSelectAllToggle?: boolean;
  /** Whether to show the clear button */
  showClearButton?: boolean;
  /** Maximum number of items that can be selected */
  maxSelection?: number;
}

/**
 * Bulk Actions Bar Component
 * 
 * A sticky bar that displays when items are selected, providing:
 * - Selected count display
 * - Bulk action buttons (Delete, Export, Assign, Update Status)
 * - Clear selection button
 * - Select all/none toggle
 * - Responsive design
 */
export function BulkActionsBar({
  selectedCount,
  totalCount,
  isAllSelected = false,
  isIndeterminate = false,
  actions = [],
  onAction,
  onSelectAll,
  onClearSelection,
  onToggleSelectAll,
  isLoading = false,
  loadingMessage = "Processing...",
  className,
  leftElement,
  rightElement,
  entityName = "items",
  sticky = true,
  stickyOffset = 0,
  showSelectAllToggle = true,
  showClearButton = true,
  maxSelection,
}: BulkActionsBarProps) {
  const hasSelection = selectedCount > 0;

  const handleActionClick = useCallback(
    (action: BulkAction) => {
      if (action.disabled || isLoading) return;

      if (action.onClick) {
        action.onClick();
      } else {
        onAction?.(action);
      }
    },
    [onAction, isLoading]
  );

  // Default actions if none provided
  const defaultActions: BulkAction[] = [
    {
      id: "export",
      label: "Export",
      type: "export",
      icon: <Download className="h-4 w-4" />,
    },
    {
      id: "assign",
      label: "Assign",
      type: "assign",
      icon: <UserPlus className="h-4 w-4" />,
    },
    {
      id: "update-status",
      label: "Update Status",
      type: "update-status",
      icon: <CheckCircle className="h-4 w-4" />,
    },
    {
      id: "delete",
      label: "Delete",
      type: "delete",
      icon: <Trash2 className="h-4 w-4" />,
      destructive: true,
      requiresConfirmation: true,
    },
  ];

  const displayActions = actions.length > 0 ? actions : defaultActions;

  if (!hasSelection && !isLoading) {
    return null;
  }

  return (
    <div
      className={clsx(
        "bg-white border-b border-gray-200 shadow-sm",
        sticky && "sticky z-30",
        className
      )}
      style={sticky ? { top: stickyOffset } : undefined}
      role="toolbar"
      aria-label="Bulk actions"
    >
      <div className="max-w-full mx-auto px-4 sm:px-6 lg:px-8">
        <div className="flex items-center justify-between h-14">
          {/* Left Section: Selection Info & Controls */}
          <div className="flex items-center space-x-4">
            {leftElement}

            {showSelectAllToggle && (
              <button
                onClick={onToggleSelectAll}
                disabled={isLoading}
                className={clsx(
                  "flex items-center space-x-2 text-sm font-medium",
                  "text-gray-700 hover:text-gray-900",
                  "focus:outline-none focus:ring-2 focus:ring-blue-500 focus:ring-offset-2 rounded",
                  isLoading && "opacity-50 cursor-not-allowed"
                )}
                aria-label={isAllSelected ? "Deselect all" : "Select all"}
              >
                {isAllSelected ? (
                  <CheckSquare className="h-5 w-5 text-blue-600" />
                ) : isIndeterminate ? (
                  <div className="relative h-5 w-5">
                    <Square className="h-5 w-5 text-blue-600" />
                    <div className="absolute inset-0 flex items-center justify-center">
                      <div className="h-2 w-2 bg-blue-600 rounded-sm" />
                    </div>
                  </div>
                ) : (
                  <Square className="h-5 w-5 text-gray-400" />
                )}
                <span>
                  {isAllSelected
                    ? "Deselect all"
                    : isIndeterminate
                    ? `${selectedCount} selected`
                    : "Select all"}
                </span>
              </button>
            )}

            {/* Selected Count */}
            <div className="flex items-center">
              <span className="text-sm text-gray-600">
                <span className="font-semibold text-gray-900">{selectedCount}</span>
                {" "}{entityName} selected
                {maxSelection && (
                  <span className="text-gray-400 ml-1">
                    (max {maxSelection})
                  </span>
                )}
              </span>
              {totalCount !== undefined && (
                <span className="text-sm text-gray-400 ml-1">
                  of {totalCount}
                </span>
              )}
            </div>

            {isLoading && (
              <div className="flex items-center space-x-2 text-sm text-blue-600">
                <Loader2 className="h-4 w-4 animate-spin" />
                <span>{loadingMessage}</span>
              </div>
            )}
          </div>

          {/* Right Section: Actions & Clear */}
          <div className="flex items-center space-x-2">
            {rightElement}

            {/* Bulk Actions */}
            <div className="flex items-center space-x-1 sm:space-x-2">
              {displayActions.map((action) => (
                <Button
                  key={action.id}
                  variant={action.destructive ? "outline" : "default"}
                  size="sm"
                  onClick={() => handleActionClick(action)}
                  disabled={action.disabled || isLoading}
                  className={clsx(
                    "hidden sm:inline-flex items-center space-x-1",
                    action.destructive && "text-red-600 hover:text-red-700 hover:bg-red-50 border-red-200"
                  )}
                  title={action.label}
                >
                  {action.icon}
                  <span>{action.label}</span>
                </Button>
              ))}

              {/* Mobile Actions Menu (simplified) */}
              <div className="sm:hidden">
                <select
                  onChange={(e) => {
                    const action = displayActions.find((a) => a.id === e.target.value);
                    if (action) handleActionClick(action);
                    e.target.value = "";
                  }}
                  disabled={isLoading}
                  className="block w-full rounded-md border-gray-300 py-2 pl-3 pr-10 text-sm focus:border-blue-500 focus:outline-none focus:ring-blue-500"
                  value=""
                >
                  <option value="" disabled>
                    Actions
                  </option>
                  {displayActions.map((action) => (
                    <option key={action.id} value={action.id}>
                      {action.label}
                    </option>
                  ))}
                </select>
              </div>
            </div>

            {/* Clear Selection */}
            {showClearButton && (
              <button
                onClick={onClearSelection}
                disabled={isLoading}
                className={clsx(
                  "p-2 rounded-full",
                  "text-gray-400 hover:text-gray-600 hover:bg-gray-100",
                  "focus:outline-none focus:ring-2 focus:ring-gray-500 focus:ring-offset-2",
                  "transition-colors duration-200",
                  isLoading && "opacity-50 cursor-not-allowed"
                )}
                aria-label="Clear selection"
                title="Clear selection"
              >
                <X className="h-5 w-5" />
              </button>
            )}
          </div>
        </div>
      </div>
    </div>
  );
}

export default BulkActionsBar;

"use client";

import React, { useCallback, useState, useEffect } from "react";
import { clsx } from "clsx";
import {
  X,
  AlertTriangle,
  CheckCircle,
  XCircle,
  Loader2,
  RefreshCw,
  Download,
  FileText,
  ChevronDown,
  ChevronUp,
} from "lucide-react";
import { Dialog } from "../ui/dialog";
import { Button } from "../ui/button";
import { Progress } from "../ui/progress";

export type BulkOperationModalState =
  | "confirm"
  | "processing"
  | "success"
  | "error"
  | "partial";

export interface BulkOperationError {
  itemId: string;
  error: string;
  retryable?: boolean;
}

export interface BulkOperationResult {
  operationId: string;
  status: "completed" | "failed" | "partial";
  total: number;
  succeeded: number;
  failed: number;
  errors?: BulkOperationError[];
  downloadUrl?: string;
  fileName?: string;
}

export interface BulkOperationModalProps {
  /** Whether the modal is open */
  isOpen: boolean;
  /** Callback to close the modal */
  onClose: () => void;
  /** Current state of the operation */
  state: BulkOperationModalState;
  /** Operation type for display */
  operationType: "delete" | "export" | "assign" | "update" | string;
  /** Number of items being processed */
  itemCount: number;
  /** Progress percentage (0-100) */
  progress?: number;
  /** Current operation result */
  result?: BulkOperationResult;
  /** Confirmation message (overrides default) */
  confirmMessage?: React.ReactNode;
  /** Whether the operation is destructive */
  isDestructive?: boolean;
  /** Callback to confirm the operation */
  onConfirm?: () => void;
  /** Callback to cancel the operation */
  onCancel?: () => void;
  /** Callback to retry failed items */
  onRetry?: () => void;
  /** Callback to download exported file */
  onDownload?: (url: string, fileName: string) => void;
  /** Custom footer content */
  footer?: React.ReactNode;
  /** Entity name (e.g., "users", "records") */
  entityName?: string;
  /** Additional CSS classes */
  className?: string;
  /** Maximum number of errors to display initially */
  maxVisibleErrors?: number;
  /** Loading message during processing */
  processingMessage?: string;
  /** Success message */
  successMessage?: string;
  /** Error message */
  errorMessage?: string;
  /** Partial success message */
  partialMessage?: string;
}

/**
 * Bulk Operation Modal Component
 * 
 * A modal component for bulk operations providing:
 * - Confirmation for destructive actions
 * - Progress tracking for long operations
 * - Success/error results display
 * - Retry failed items
 * - Download for export operations
 */
export function BulkOperationModal({
  isOpen,
  onClose,
  state,
  operationType,
  itemCount,
  progress = 0,
  result,
  confirmMessage,
  isDestructive = false,
  onConfirm,
  onCancel,
  onRetry,
  onDownload,
  footer,
  entityName = "items",
  className,
  maxVisibleErrors = 5,
  processingMessage = "Processing...",
  successMessage = "Operation completed successfully",
  errorMessage = "Operation failed",
  partialMessage = "Operation completed with some errors",
}: BulkOperationModalProps) {
  const [showAllErrors, setShowAllErrors] = useState(false);
  const [isClosing, setIsClosing] = useState(false);

  // Reset state when modal opens
  useEffect(() => {
    if (isOpen) {
      setShowAllErrors(false);
      setIsClosing(false);
    }
  }, [isOpen]);

  const handleClose = useCallback(() => {
    if (state === "processing") {
      // Don't allow closing during processing, or show confirmation
      if (!window.confirm("Are you sure you want to cancel this operation?")) {
        return;
      }
      onCancel?.();
    }
    setIsClosing(true);
    setTimeout(() => {
      onClose();
      setIsClosing(false);
    }, 150);
  }, [state, onClose, onCancel]);

  const handleConfirm = useCallback(() => {
    onConfirm?.();
  }, [onConfirm]);

  const handleRetry = useCallback(() => {
    onRetry?.();
  }, [onRetry]);

  const handleDownload = useCallback(() => {
    if (result?.downloadUrl && result?.fileName) {
      onDownload?.(result.downloadUrl, result.fileName);
    }
  }, [result, onDownload]);

  // Get operation type display name
  const getOperationDisplayName = useCallback(() => {
    switch (operationType) {
      case "delete":
        return "Delete";
      case "export":
        return "Export";
      case "assign":
        return "Assign";
      case "update":
        return "Update";
      default:
        return operationType.charAt(0).toUpperCase() + operationType.slice(1);
    }
  }, [operationType]);

  // Get icon for current state
  const getStateIcon = useCallback(() => {
    switch (state) {
      case "confirm":
        return isDestructive ? (
          <AlertTriangle className="h-12 w-12 text-amber-500" />
        ) : (
          <CheckCircle className="h-12 w-12 text-blue-500" />
        );
      case "processing":
        return <Loader2 className="h-12 w-12 text-blue-500 animate-spin" />;
      case "success":
        return <CheckCircle className="h-12 w-12 text-green-500" />;
      case "error":
        return <XCircle className="h-12 w-12 text-red-500" />;
      case "partial":
        return <AlertTriangle className="h-12 w-12 text-amber-500" />;
      default:
        return null;
    }
  }, [state, isDestructive]);

  // Render confirmation content
  const renderConfirmContent = () => (
    <div className="space-y-4">
      {confirmMessage || (
        <div className="text-center space-y-3">
          <p className="text-lg text-gray-900">
            Are you sure you want to {operationType} {" "}
            <span className="font-semibold">{itemCount}</span> {entityName}?
          </p>
          {isDestructive && (
            <div className="bg-amber-50 border border-amber-200 rounded-lg p-4">
              <div className="flex items-start">
                <AlertTriangle className="h-5 w-5 text-amber-500 mt-0.5 mr-2 flex-shrink-0" />
                <p className="text-sm text-amber-800">
                  This action cannot be undone. The selected {entityName} will be
                  permanently removed.
                </p>
              </div>
            </div>
          )}
        </div>
      )}
    </div>
  );

  // Render processing content
  const renderProcessingContent = () => (
    <div className="space-y-6">
      <div className="text-center">
        <p className="text-lg font-medium text-gray-900 mb-2">
          {processingMessage}
        </p>
        <p className="text-sm text-gray-500">
          Please do not close this window
        </p>
      </div>
      <div className="space-y-2">
        <div className="flex justify-between text-sm">
          <span className="text-gray-600">Progress</span>
          <span className="font-medium text-gray-900">{progress}%</span>
        </div>
        <Progress value={progress} className="h-2" />
        {result && (
          <p className="text-xs text-gray-500 text-center">
            Processed {result.succeeded + result.failed} of {result.total} {entityName}
          </p>
        )}
      </div>
    </div>
  );

  // Render success content
  const renderSuccessContent = () => (
    <div className="space-y-4 text-center">
      <p className="text-lg text-gray-900">{successMessage}</p>
      <div className="bg-green-50 border border-green-200 rounded-lg p-4">
        <p className="text-sm text-green-800">
          Successfully processed{" "}
          <span className="font-semibold">{result?.succeeded || itemCount}</span>{" "}
          {entityName}
        </p>
      </div>
      {result?.downloadUrl && (
        <Button onClick={handleDownload} className="w-full sm:w-auto">
          <Download className="h-4 w-4 mr-2" />
          Download {result.fileName || "File"}
        </Button>
      )}
    </div>
  );

  // Render error content
  const renderErrorContent = () => (
    <div className="space-y-4">
      <div className="text-center">
        <p className="text-lg text-gray-900">{errorMessage}</p>
      </div>
      {result?.errors && result.errors.length > 0 && (
        <div className="bg-red-50 border border-red-200 rounded-lg p-4">
          <p className="text-sm font-medium text-red-800 mb-2">Errors:</p>
          <ul className="space-y-1 text-sm text-red-700">
            {(showAllErrors ? result.errors : result.errors.slice(0, maxVisibleErrors)).map(
              (error, index) => (
                <li key={index} className="flex items-start">
                  <span className="font-mono text-xs bg-red-100 px-1.5 py-0.5 rounded mr-2 mt-0.5">
                    {error.itemId}
                  </span>
                  <span>{error.error}</span>
                </li>
              )
            )}
          </ul>
          {result.errors.length > maxVisibleErrors && (
            <button
              onClick={() => setShowAllErrors(!showAllErrors)}
              className="mt-2 text-sm text-red-600 hover:text-red-800 flex items-center"
            >
              {showAllErrors ? (
                <>
                  <ChevronUp className="h-4 w-4 mr-1" />
                  Show less
                </>
              ) : (
                <>
                  <ChevronDown className="h-4 w-4 mr-1" />
                  Show {result.errors.length - maxVisibleErrors} more errors
                </>
              )}
            </button>
          )}
        </div>
      )}
    </div>
  );

  // Render partial success content
  const renderPartialContent = () => {
    const visibleErrors = showAllErrors
      ? result?.errors
      : result?.errors?.slice(0, maxVisibleErrors);

    return (
      <div className="space-y-4">
        <div className="text-center">
          <p className="text-lg text-gray-900">{partialMessage}</p>
        </div>
        <div className="grid grid-cols-3 gap-4">
          <div className="bg-gray-50 border border-gray-200 rounded-lg p-3 text-center">
            <p className="text-2xl font-bold text-gray-900">{result?.total}</p>
            <p className="text-xs text-gray-500">Total</p>
          </div>
          <div className="bg-green-50 border border-green-200 rounded-lg p-3 text-center">
            <p className="text-2xl font-bold text-green-600">{result?.succeeded}</p>
            <p className="text-xs text-green-600">Succeeded</p>
          </div>
          <div className="bg-red-50 border border-red-200 rounded-lg p-3 text-center">
            <p className="text-2xl font-bold text-red-600">{result?.failed}</p>
            <p className="text-xs text-red-600">Failed</p>
          </div>
        </div>

        {result?.errors && result.errors.length > 0 && (
          <div className="border border-gray-200 rounded-lg overflow-hidden">
            <div className="bg-gray-50 px-4 py-2 border-b border-gray-200 flex items-center justify-between">
              <p className="text-sm font-medium text-gray-700 flex items-center">
                <FileText className="h-4 w-4 mr-2" />
                Error Details
              </p>
              {result.errors.length > maxVisibleErrors && (
                <button
                  onClick={() => setShowAllErrors(!showAllErrors)}
                  className="text-sm text-blue-600 hover:text-blue-800 flex items-center"
                >
                  {showAllErrors ? (
                    <>
                      <ChevronUp className="h-4 w-4 mr-1" />
                      Less
                    </>
                  ) : (
                    <>
                      <ChevronDown className="h-4 w-4 mr-1" />
                      More ({result.errors.length - maxVisibleErrors})
                    </>
                  )}
                </button>
              )}
            </div>
            <div className="max-h-48 overflow-y-auto">
              <ul className="divide-y divide-gray-200">
                {visibleErrors?.map((error, index) => (
                  <li key={index} className="px-4 py-3 flex items-start">
                    <XCircle className="h-4 w-4 text-red-500 mt-0.5 mr-3 flex-shrink-0" />
                    <div className="flex-1 min-w-0">
                      <p className="text-sm font-medium text-gray-900">
                        {error.itemId}
                      </p>
                      <p className="text-sm text-gray-500 truncate">
                        {error.error}
                      </p>
                    </div>
                    {error.retryable && (
                      <span className="ml-2 inline-flex items-center px-2 py-0.5 rounded text-xs font-medium bg-yellow-100 text-yellow-800">
                        Retryable
                      </span>
                    )}
                  </li>
                ))}
              </ul>
            </div>
          </div>
        )}

        {result?.downloadUrl && (
          <Button onClick={handleDownload} variant="outline" className="w-full">
            <Download className="h-4 w-4 mr-2" />
            Download {result.fileName || "File"}
          </Button>
        )}
      </div>
    );
  };

  // Render content based on state
  const renderContent = () => {
    switch (state) {
      case "confirm":
        return renderConfirmContent();
      case "processing":
        return renderProcessingContent();
      case "success":
        return renderSuccessContent();
      case "error":
        return renderErrorContent();
      case "partial":
        return renderPartialContent();
      default:
        return null;
    }
  };

  // Get dialog title based on state
  const getDialogTitle = () => {
    switch (state) {
      case "confirm":
        return `Confirm ${getOperationDisplayName()}`;
      case "processing":
        return `${getOperationDisplayName()} in Progress`;
      case "success":
        return `${getOperationDisplayName()} Complete`;
      case "error":
        return `${getOperationDisplayName()} Failed`;
      case "partial":
        return `${getOperationDisplayName()} Partially Complete`;
      default:
        return getOperationDisplayName();
    }
  };

  // Get footer buttons based on state
  const getFooterButtons = () => {
    switch (state) {
      case "confirm":
        return (
          <>
            <Button variant="outline" onClick={handleClose}>
              Cancel
            </Button>
            <Button
              onClick={handleConfirm}
              variant={isDestructive ? "outline" : "default"}
              className={isDestructive ? "bg-red-600 text-white hover:bg-red-700" : ""}
            >
              {isDestructive ? "Delete" : "Confirm"}
            </Button>
          </>
        );
      case "processing":
        return (
          <Button variant="outline" onClick={handleClose}>
            Cancel
          </Button>
        );
      case "success":
        return (
          <Button onClick={handleClose}>Done</Button>
        );
      case "error":
        return (
          <>
            <Button variant="outline" onClick={handleClose}>
              Close
            </Button>
            {onRetry && (
              <Button onClick={handleRetry}>
                <RefreshCw className="h-4 w-4 mr-2" />
                Retry
              </Button>
            )}
          </>
        );
      case "partial":
        return (
          <>
            <Button variant="outline" onClick={handleClose}>
              Close
            </Button>
            {onRetry && result && result.failed > 0 && (
              <Button onClick={handleRetry}>
                <RefreshCw className="h-4 w-4 mr-2" />
                Retry {result.failed} Failed
              </Button>
            )}
          </>
        );
      default:
        return null;
    }
  };

  return (
    <Dialog
      isOpen={isOpen}
      onClose={handleClose}
      title={getDialogTitle()}
      size="md"
      footer={footer || getFooterButtons()}
    >
      <div
        className={clsx(
          "transition-all duration-150",
          isClosing ? "opacity-0 scale-95" : "opacity-100 scale-100",
          className
        )}
      >
        <div className="flex flex-col items-center space-y-6 py-4">
          {getStateIcon()}
          <div className="w-full">{renderContent()}</div>
        </div>
      </div>
    </Dialog>
  );
}

export default BulkOperationModal;

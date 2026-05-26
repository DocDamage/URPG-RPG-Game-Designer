/**
 * Export Button Component
 * Dropdown button with format options, loading state, and download handling
 */

'use client';

import React, { useState, useCallback, useRef, useEffect } from 'react';
import { clsx } from 'clsx';
import {
    Download,
    FileText,
    FileSpreadsheet,
    FileJson,
    Loader2,
    ChevronDown,
    Check,
    AlertCircle,
    X
} from 'lucide-react';
import {
    ExportFormat,
    ExportType,
    ExportRequest,
    ExportStatus,
    ExportButtonProps,
    ExportProgress
} from './types';
import { useExport } from './useExport';

// ============================================================================
// Format Configuration
// ============================================================================

interface FormatOption {
    format: ExportFormat;
    label: string;
    icon: React.ReactNode;
    description: string;
    mimeType: string;
}

const FORMAT_OPTIONS: FormatOption[] = [
    {
        format: ExportFormat.PDF,
        label: 'PDF Document',
        icon: <FileText className="w-4 h-4" />,
        description: 'Best for printing and sharing',
        mimeType: 'application/pdf'
    },
    {
        format: ExportFormat.EXCEL,
        label: 'Excel Spreadsheet',
        icon: <FileSpreadsheet className="w-4 h-4" />,
        description: 'For data analysis in Excel',
        mimeType: 'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet'
    },
    {
        format: ExportFormat.CSV,
        label: 'CSV File',
        icon: <FileJson className="w-4 h-4" />,
        description: 'Simple data format',
        mimeType: 'text/csv'
    }
];

// ============================================================================
// Progress Indicator Component
// ============================================================================

interface ExportProgressProps {
    progress: ExportProgress;
    onCancel?: () => void;
}

function ExportProgressIndicator({ progress, onCancel }: ExportProgressProps) {
    const { percentage, stage, message, estimatedTimeRemaining } = progress;

    return (
        <div className="fixed bottom-4 right-4 bg-white rounded-lg shadow-lg border border-gray-200 p-4 w-80 z-50">
            <div className="flex items-center justify-between mb-2">
                <span className="text-sm font-medium text-gray-700">
                    Exporting...
                </span>
                <span className="text-sm text-gray-500">{percentage}%</span>
            </div>
            
            <div className="w-full bg-gray-200 rounded-full h-2 mb-3">
                <div
                    className="bg-blue-600 h-2 rounded-full transition-all duration-300"
                    style={{ width: `${percentage}%` }}
                />
            </div>
            
            <p className="text-xs text-gray-500 mb-1">
                {message || stage}
            </p>
            
            {estimatedTimeRemaining !== undefined && (
                <p className="text-xs text-gray-400">
                    ~{Math.ceil(estimatedTimeRemaining)}s remaining
                </p>
            )}
            
            {onCancel && (
                <button
                    onClick={onCancel}
                    className="mt-3 text-xs text-red-600 hover:text-red-700 font-medium"
                >
                    Cancel
                </button>
            )}
        </div>
    );
}

// ============================================================================
// Error Toast Component
// ============================================================================

interface ExportErrorProps {
    message: string;
    onDismiss: () => void;
}

function ExportErrorToast({ message, onDismiss }: ExportErrorProps) {
    return (
        <div className="fixed bottom-4 right-4 bg-red-50 border border-red-200 rounded-lg shadow-lg p-4 w-80 z-50">
            <div className="flex items-start gap-3">
                <AlertCircle className="w-5 h-5 text-red-500 flex-shrink-0 mt-0.5" />
                <div className="flex-1 min-w-0">
                    <h4 className="text-sm font-medium text-red-800">
                        Export Failed
                    </h4>
                    <p className="text-xs text-red-600 mt-1">
                        {message}
                    </p>
                </div>
                <button
                    onClick={onDismiss}
                    className="text-red-400 hover:text-red-600"
                >
                    <X className="w-4 h-4" />
                </button>
            </div>
        </div>
    );
}

// ============================================================================
// Main Export Button Component
// ============================================================================

export function ExportButton({
    data,
    columns,
    title = 'Export',
    filename,
    type = ExportType.DATA_INDIVIDUALS,
    parameters = {},
    formats = [ExportFormat.PDF, ExportFormat.EXCEL, ExportFormat.CSV],
    disabled = false,
    loading: externalLoading = false,
    variant = 'default',
    size = 'md',
    className,
    onExportStart,
    onExportComplete,
    onExportError
}: ExportButtonProps) {
    const [isOpen, setIsOpen] = useState(false);
    const [showError, setShowError] = useState(false);
    const dropdownRef = useRef<HTMLDivElement>(null);

    const {
        exportData,
        isLoading,
        isPolling,
        progress,
        error,
        result,
        cancel,
        reset
    } = useExport({
        onSuccess: (result) => {
            setIsOpen(false);
            onExportComplete?.(result);
        },
        onError: (error) => {
            setShowError(true);
            onExportError?.(error);
        }
    });

    // Close dropdown when clicking outside
    useEffect(() => {
        function handleClickOutside(event: MouseEvent) {
            if (dropdownRef.current && !dropdownRef.current.contains(event.target as Node)) {
                setIsOpen(false);
            }
        }

        document.addEventListener('mousedown', handleClickOutside);
        return () => document.removeEventListener('mousedown', handleClickOutside);
    }, []);

    // Dismiss error after 5 seconds
    useEffect(() => {
        if (showError) {
            const timer = setTimeout(() => setShowError(false), 5000);
            return () => clearTimeout(timer);
        }
    }, [showError]);

    const handleExport = useCallback(async (format: ExportFormat) => {
        onExportStart?.();
        
        const exportParams: ExportRequest['parameters'] = {
            ...parameters,
            customData: data ? {
                headers: columns,
                rows: data,
                title: title || 'Export',
                metadata: {
                    title,
                    generatedAt: new Date()
                }
            } : undefined
        };

        await exportData({
            format,
            type,
            parameters: exportParams,
            async: true
        });
    }, [data, columns, title, type, parameters, exportData, onExportStart]);

    const handleCancel = useCallback(() => {
        cancel();
        setIsOpen(false);
    }, [cancel]);

    const availableFormats = FORMAT_OPTIONS.filter(f => formats.includes(f.format));
    const isExporting = isLoading || isPolling || externalLoading;

    // Size classes
    const sizeClasses = {
        sm: 'px-2 py-1 text-xs',
        md: 'px-3 py-2 text-sm',
        lg: 'px-4 py-2.5 text-base'
    };

    // Variant classes
    const variantClasses = {
        default: 'bg-blue-600 text-white hover:bg-blue-700 border-transparent',
        outline: 'bg-white text-gray-700 border-gray-300 hover:bg-gray-50',
        ghost: 'bg-transparent text-gray-700 hover:bg-gray-100 border-transparent'
    };

    return (
        <>
            <div ref={dropdownRef} className={clsx('relative inline-block', className)}>
                {/* Main Button */}
                <div className="flex">
                    <button
                        onClick={() => availableFormats.length === 1 
                            ? handleExport(availableFormats[0].format)
                            : setIsOpen(!isOpen)
                        }
                        disabled={disabled || isExporting}
                        className={clsx(
                            'inline-flex items-center gap-2 font-medium rounded-l-md transition-colors',
                            'focus:outline-none focus:ring-2 focus:ring-blue-500 focus:ring-offset-2',
                            'disabled:opacity-50 disabled:cursor-not-allowed',
                            variant === 'default' && 'border border-blue-600',
                            variant === 'outline' && 'border border-r-0 border-gray-300',
                            sizeClasses[size],
                            variantClasses[variant]
                        )}
                    >
                        {isExporting ? (
                            <Loader2 className="w-4 h-4 animate-spin" />
                        ) : (
                            <Download className="w-4 h-4" />
                        )}
                        <span>
                            {isExporting ? 'Exporting...' : title}
                        </span>
                    </button>

                    {availableFormats.length > 1 && (
                        <button
                            onClick={() => setIsOpen(!isOpen)}
                            disabled={disabled || isExporting}
                            className={clsx(
                                'inline-flex items-center justify-center rounded-r-md border-l transition-colors',
                                'focus:outline-none focus:ring-2 focus:ring-blue-500 focus:ring-offset-2',
                                'disabled:opacity-50 disabled:cursor-not-allowed',
                                variant === 'default' && [
                                    'border-blue-600 bg-blue-600 text-white hover:bg-blue-700',
                                    size === 'sm' && 'px-1.5',
                                    size === 'md' && 'px-2',
                                    size === 'lg' && 'px-2.5'
                                ],
                                variant === 'outline' && [
                                    'border-gray-300 bg-white text-gray-700 hover:bg-gray-50',
                                    size === 'sm' && 'px-1',
                                    size === 'md' && 'px-1.5',
                                    size === 'lg' && 'px-2'
                                ],
                                variant === 'ghost' && [
                                    'border-gray-200 bg-transparent text-gray-700 hover:bg-gray-100',
                                    size === 'sm' && 'px-1',
                                    size === 'md' && 'px-1.5',
                                    size === 'lg' && 'px-2'
                                ]
                            )}
                        >
                            <ChevronDown className={clsx(
                                'transition-transform',
                                size === 'sm' && 'w-3 h-3',
                                size === 'md' && 'w-4 h-4',
                                size === 'lg' && 'w-5 h-5',
                                isOpen && 'rotate-180'
                            )} />
                        </button>
                    )}
                </div>

                {/* Dropdown Menu */}
                {isOpen && availableFormats.length > 1 && (
                    <div className="absolute right-0 mt-1 w-64 bg-white rounded-md shadow-lg border border-gray-200 py-1 z-50">
                        <div className="px-3 py-2 text-xs font-medium text-gray-500 uppercase tracking-wider">
                            Export as
                        </div>
                        {availableFormats.map((format) => (
                            <button
                                key={format.format}
                                onClick={() => handleExport(format.format)}
                                disabled={isExporting}
                                className={clsx(
                                    'w-full flex items-start gap-3 px-3 py-2.5 text-left',
                                    'hover:bg-gray-50 transition-colors',
                                    'disabled:opacity-50 disabled:cursor-not-allowed'
                                )}
                            >
                                <div className="flex-shrink-0 mt-0.5 text-gray-500">
                                    {format.icon}
                                </div>
                                <div className="flex-1 min-w-0">
                                    <div className="text-sm font-medium text-gray-900">
                                        {format.label}
                                    </div>
                                    <div className="text-xs text-gray-500">
                                        {format.description}
                                    </div>
                                </div>
                            </button>
                        ))}
                    </div>
                )}
            </div>

            {/* Progress Indicator */}
            {isPolling && progress && (
                <ExportProgressIndicator
                    progress={progress}
                    onCancel={handleCancel}
                />
            )}

            {/* Error Toast */}
            {showError && error && (
                <ExportErrorToast
                    message={error.message}
                    onDismiss={() => setShowError(false)}
                />
            )}
        </>
    );
}

// ============================================================================
// Simple Export Button (Single Format)
// ============================================================================

interface SimpleExportButtonProps {
    onClick: () => void;
    label?: string;
    icon?: React.ReactNode;
    loading?: boolean;
    disabled?: boolean;
    variant?: 'default' | 'outline' | 'ghost';
    size?: 'sm' | 'md' | 'lg';
    className?: string;
}

export function SimpleExportButton({
    onClick,
    label = 'Export',
    icon = <Download className="w-4 h-4" />,
    loading = false,
    disabled = false,
    variant = 'default',
    size = 'md',
    className
}: SimpleExportButtonProps) {
    const sizeClasses = {
        sm: 'px-2 py-1 text-xs',
        md: 'px-3 py-2 text-sm',
        lg: 'px-4 py-2.5 text-base'
    };

    const variantClasses = {
        default: 'bg-blue-600 text-white hover:bg-blue-700 border-transparent',
        outline: 'bg-white text-gray-700 border-gray-300 hover:bg-gray-50',
        ghost: 'bg-transparent text-gray-700 hover:bg-gray-100 border-transparent'
    };

    return (
        <button
            onClick={onClick}
            disabled={disabled || loading}
            className={clsx(
                'inline-flex items-center gap-2 font-medium rounded-md transition-colors',
                'focus:outline-none focus:ring-2 focus:ring-blue-500 focus:ring-offset-2',
                'disabled:opacity-50 disabled:cursor-not-allowed border',
                sizeClasses[size],
                variantClasses[variant],
                className
            )}
        >
            {loading ? (
                <Loader2 className="w-4 h-4 animate-spin" />
            ) : (
                icon
            )}
            <span>{loading ? 'Exporting...' : label}</span>
        </button>
    );
}

// ============================================================================
// Export Group Component
// ============================================================================

interface ExportGroupProps {
    children: React.ReactNode;
    className?: string;
}

export function ExportGroup({ children, className }: ExportGroupProps) {
    return (
        <div className={clsx('flex items-center gap-2', className)}>
            {children}
        </div>
    );
}

export default ExportButton;

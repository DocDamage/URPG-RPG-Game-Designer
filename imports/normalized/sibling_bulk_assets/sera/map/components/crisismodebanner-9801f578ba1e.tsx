import React from 'react';
import { Alert } from './ui/alert';

export interface CrisisModeBannerProps {
    isActive: boolean;
    message?: string;
    onDeactivate?: () => void;
}

export function CrisisModeBanner({
    isActive,
    message = 'CRISIS MODE ACTIVE - All entries logged with enhanced supervision',
    onDeactivate
}: CrisisModeBannerProps) {
    if (!isActive) return null;

    return (
        <div className="fixed top-0 left-0 right-0 z-50 animate-pulse">
            <Alert
                variant="error"
                title="🚨 CRISIS MODE"
                onClose={onDeactivate}
            >
                <div className="flex items-center justify-between">
                    <span className="font-bold text-lg">{message}</span>
                    {onDeactivate && (
                        <button
                            onClick={onDeactivate}
                            className="ml-4 px-4 py-2 bg-white text-red-600 rounded font-medium hover:bg-red-50"
                        >
                            Deactivate
                        </button>
                    )}
                </div>
            </Alert>
        </div>
    );
}

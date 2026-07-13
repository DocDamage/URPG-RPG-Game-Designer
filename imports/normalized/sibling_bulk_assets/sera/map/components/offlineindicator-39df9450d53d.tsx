import React from 'react';
import { useOfflineSync } from '../lib/hooks/useOfflineSync';
import { Badge } from './ui/badge';

export function OfflineIndicator() {
    const { isOnline, isSyncing, pendingItems } = useOfflineSync();

    if (isOnline && !isSyncing && pendingItems === 0) {
        return null; // Don't show when everything is good
    }

    return (
        <div className="fixed bottom-4 right-4 z-50">
            {!isOnline && (
                <Badge variant="warning" size="lg" className="shadow-lg">
                    <span className="flex items-center space-x-2">
                        <svg className="w-4 h-4" fill="currentColor" viewBox="0 0 20 20">
                            <path d="M10 2a8 8 0 100 16 8 8 0 000-16zM9 5h2v2H9V5zm0 4h2v6H9V9z" />
                        </svg>
                        <span>Offline Mode</span>
                        {pendingItems > 0 && <span>({pendingItems} pending)</span>}
                    </span>
                </Badge>
            )}
            {isOnline && isSyncing && (
                <Badge variant="info" size="lg" className="shadow-lg">
                    <span className="flex items-center space-x-2">
                        <svg className="animate-spin w-4 h-4" fill="currentColor" viewBox="0 0 20 20">
                            <path d="M10 3a7 7 0 100 14 7 7 0 000-14zm0 2a5 5 0 110 10 5 5 0 010-10z" />
                        </svg>
                        <span>Syncing...</span>
                    </span>
                </Badge>
            )}
        </div>
    );
}

"use client";

import { createContext, useContext, ReactNode } from 'react';
import { useOfflineSync } from '../../lib/hooks/useOfflineSync';

interface OfflineContextType {
    isOnline: boolean;
    isSyncing: boolean;
    pendingItems: number;
    conflicts: number;
    lastSync: number | null;
    forceSync: () => Promise<void>;
    canSync: boolean;
}

const OfflineContext = createContext<OfflineContextType | undefined>(undefined);

export function OfflineProvider({ children }: { children: ReactNode }) {
    const syncState = useOfflineSync();

    return (
        <OfflineContext.Provider value={syncState}>
            {children}
        </OfflineContext.Provider>
    );
}

export function useOffline() {
    const context = useContext(OfflineContext);
    if (context === undefined) {
        throw new Error('useOffline must be used within an OfflineProvider');
    }
    return context;
}


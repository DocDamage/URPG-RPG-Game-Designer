"use client";

import { createContext, useContext, ReactNode, useEffect } from 'react';
import { useWebSocket } from '../../lib/hooks/useWebSocket';
import { isAuthenticated } from '../../lib/api/auth';

interface WebSocketContextType {
    isConnected: boolean;
    lastMessage: any;
    error: Error | null;
    sendMessage: (data: any) => void;
}

const WebSocketContext = createContext<WebSocketContextType | undefined>(undefined);

export function WebSocketProvider({ children }: { children: ReactNode }) {
    // Only connect if authenticated
    const wsUrl = typeof window !== 'undefined' && isAuthenticated()
        ? process.env.NEXT_PUBLIC_WS_URL || 'ws://localhost:3000'
        : '';

    // Only use WebSocket hook if we have a URL
    const wsHook = wsUrl 
        ? useWebSocket(wsUrl, {
            reconnect: true,
            reconnectInterval: 3000,
            reconnectAttempts: 10
          })
        : { isConnected: false, lastMessage: null, error: null, sendMessage: () => {} };

    const { isConnected, lastMessage, error, sendMessage } = wsHook;

    // Log connection status
    useEffect(() => {
        if (isConnected) {
            console.log('WebSocket connected');
        } else if (error) {
            console.error('WebSocket error:', error);
        }
    }, [isConnected, error]);

    return (
        <WebSocketContext.Provider value={{ isConnected, lastMessage, error, sendMessage }}>
            {children}
        </WebSocketContext.Provider>
    );
}

export function useWebSocketContext() {
    const context = useContext(WebSocketContext);
    if (context === undefined) {
        throw new Error('useWebSocketContext must be used within a WebSocketProvider');
    }
    return context;
}


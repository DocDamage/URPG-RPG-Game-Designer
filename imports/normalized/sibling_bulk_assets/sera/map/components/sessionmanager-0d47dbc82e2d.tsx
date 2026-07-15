'use client';

import { useState, useEffect, useCallback } from 'react';

interface Session {
    id: string;
    deviceId?: string;
    createdAt: string;
    lastActivity: string;
    expiresAt: string;
    ipAddress?: string;
    userAgent?: string;
    lockedAt?: string;
    role?: string;
}

interface SessionManagerProps {
    onSessionExpired?: () => void;
    onSessionLocked?: () => void;
    timeoutMinutes?: number;
}

export default function SessionManager({
    onSessionExpired,
    onSessionLocked,
    timeoutMinutes = 15
}: SessionManagerProps) {
    const [timeRemaining, setTimeRemaining] = useState<number | null>(null);
    const [isLocked, setIsLocked] = useState(false);
    const [showWarning, setShowWarning] = useState(false);
    const [sessions, setSessions] = useState<Session[]>([]);

    useEffect(() => {
        loadSessions();
        checkSessionStatus();
        setupActivityTracking();
        setupVisibilityTracking();

        // Check session status every minute
        const interval = setInterval(checkSessionStatus, 60000);
        return () => clearInterval(interval);
    }, []);

    const loadSessions = async () => {
        try {
            const token = localStorage.getItem('token');
            const response = await fetch('/api/sessions', {
                headers: {
                    'Authorization': `Bearer ${token}`
                }
            });

            if (response.ok) {
                const data = await response.json();
                setSessions(data);
            }
        } catch (err) {
            console.error('Failed to load sessions:', err);
        }
    };

    const checkSessionStatus = async () => {
        try {
            const token = localStorage.getItem('token');
            const response = await fetch('/api/auth/me', {
                headers: {
                    'Authorization': `Bearer ${token}`
                }
            });

            if (response.status === 401) {
                const data = await response.json();
                if (data.locked) {
                    setIsLocked(true);
                    onSessionLocked?.();
                } else {
                    onSessionExpired?.();
                }
            }
        } catch (err) {
            console.error('Failed to check session:', err);
        }
    };

    const setupActivityTracking = () => {
        let lastActivity = Date.now();
        const activityEvents = ['mousedown', 'mousemove', 'keypress', 'scroll', 'touchstart', 'click'];

        const updateActivity = () => {
            lastActivity = Date.now();
        };

        activityEvents.forEach(event => {
            document.addEventListener(event, updateActivity, true);
        });

        // Check for inactivity every 30 seconds
        const interval = setInterval(() => {
            const inactiveTime = Date.now() - lastActivity;
            const warningTime = (timeoutMinutes - 2) * 60 * 1000; // 2 minutes before timeout
            const timeoutTime = timeoutMinutes * 60 * 1000;

            if (inactiveTime >= timeoutTime) {
                onSessionExpired?.();
            } else if (inactiveTime >= warningTime) {
                setShowWarning(true);
                setTimeRemaining(Math.ceil((timeoutTime - inactiveTime) / 1000));
            } else {
                setShowWarning(false);
                setTimeRemaining(null);
            }
        }, 30000);

        return () => {
            activityEvents.forEach(event => {
                document.removeEventListener(event, updateActivity, true);
            });
            clearInterval(interval);
        };
    };

    const setupVisibilityTracking = () => {
        const handleVisibilityChange = () => {
            if (document.hidden) {
                // Lock session when tab is hidden
                lockSession();
            }
        };

        document.addEventListener('visibilitychange', handleVisibilityChange);
        return () => {
            document.removeEventListener('visibilitychange', handleVisibilityChange);
        };
    };

    const lockSession = async () => {
        try {
            const token = localStorage.getItem('token');
            const sessionId = localStorage.getItem('sessionId');
            
            if (sessionId) {
                await fetch(`/api/sessions/${sessionId}/lock`, {
                    method: 'POST',
                    headers: {
                        'Authorization': `Bearer ${token}`
                    }
                });
                setIsLocked(true);
                onSessionLocked?.();
            }
        } catch (err) {
            console.error('Failed to lock session:', err);
        }
    };

    const unlockSession = async () => {
        try {
            const token = localStorage.getItem('token');
            const sessionId = localStorage.getItem('sessionId');
            
            if (sessionId) {
                const response = await fetch(`/api/sessions/${sessionId}/unlock`, {
                    method: 'POST',
                    headers: {
                        'Authorization': `Bearer ${token}`
                    }
                });

                if (response.ok) {
                    setIsLocked(false);
                    setShowWarning(false);
                }
            }
        } catch (err) {
            console.error('Failed to unlock session:', err);
        }
    };

    const invalidateSession = async (sessionId: string) => {
        try {
            const token = localStorage.getItem('token');
            await fetch(`/api/sessions/${sessionId}`, {
                method: 'DELETE',
                headers: {
                    'Authorization': `Bearer ${token}`
                }
            });
            await loadSessions();
        } catch (err) {
            console.error('Failed to invalidate session:', err);
        }
    };

    const formatTime = (seconds: number) => {
        const mins = Math.floor(seconds / 60);
        const secs = seconds % 60;
        return `${mins}:${secs.toString().padStart(2, '0')}`;
    };

    if (isLocked) {
        return (
            <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50">
                <div className="bg-white rounded-lg p-6 max-w-md w-full mx-4">
                    <h2 className="text-xl font-semibold mb-4">Session Locked</h2>
                    <p className="text-gray-600 mb-6">
                        Your session has been locked for security. Please unlock to continue.
                    </p>
                    <button
                        onClick={unlockSession}
                        className="w-full px-4 py-2 bg-blue-600 text-white rounded hover:bg-blue-700"
                    >
                        Unlock Session
                    </button>
                </div>
            </div>
        );
    }

    return (
        <>
            {showWarning && timeRemaining && (
                <div className="fixed top-4 right-4 bg-yellow-50 border border-yellow-200 rounded-lg p-4 shadow-lg z-50 max-w-sm">
                    <div className="flex items-center justify-between">
                        <div>
                            <p className="font-medium text-yellow-800">Session Expiring Soon</p>
                            <p className="text-sm text-yellow-700">
                                Your session will expire in {formatTime(timeRemaining)}
                            </p>
                        </div>
                        <button
                            onClick={() => setShowWarning(false)}
                            className="text-yellow-600 hover:text-yellow-800"
                        >
                            ×
                        </button>
                    </div>
                </div>
            )}

            {/* Session Management UI - can be shown in settings */}
            {sessions.length > 0 && (
                <div className="space-y-2">
                    {sessions.map((session) => (
                        <div key={session.id} className="flex items-center justify-between p-3 bg-gray-50 rounded">
                            <div>
                                <p className="text-sm font-medium">
                                    {session.userAgent?.substring(0, 50) || 'Unknown Device'}
                                </p>
                                <p className="text-xs text-gray-500">
                                    Last activity: {new Date(session.lastActivity).toLocaleString()}
                                </p>
                            </div>
                            <button
                                onClick={() => invalidateSession(session.id)}
                                className="text-sm text-red-600 hover:text-red-800"
                            >
                                Revoke
                            </button>
                        </div>
                    ))}
                </div>
            )}
        </>
    );
}


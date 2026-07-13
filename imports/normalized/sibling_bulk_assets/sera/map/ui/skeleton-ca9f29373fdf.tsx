import React from 'react';

export interface SkeletonProps {
    width?: string;
    height?: string;
    className?: string;
    lines?: number;
}

export function Skeleton({ width, height = '1rem', className = '', lines = 1 }: SkeletonProps) {
    if (lines > 1) {
        return (
            <div className={`space-y-2 ${className}`}>
                {Array.from({ length: lines }).map((_, idx) => (
                    <div
                        key={idx}
                        className="animate-pulse bg-gray-200 rounded"
                        style={{ width: idx === lines - 1 ? '80%' : width, height }}
                    />
                ))}
            </div>
        );
    }

    return (
        <div
            className={`animate-pulse bg-gray-200 rounded ${className}`}
            style={{ width, height }}
        />
    );
}

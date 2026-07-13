"use client";
import React from 'react';
import { Card } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Badge } from '@/components/ui/badge';
import { Progress } from '@/components/ui/progress';

export interface TrainingModule {
    id: number;
    title: string;
    description: string;
    category: string;
    duration: number; // in minutes
    completed: boolean;
    progress: number; // 0-100
    requiredForCert?: boolean;
    dueDate?: string;
}

interface ModuleCardProps {
    module: TrainingModule;
    onStart?: (moduleId: number) => void;
    onContinue?: (moduleId: number) => void;
    onReview?: (moduleId: number) => void;
}

export function ModuleCard({ module, onStart, onContinue, onReview }: ModuleCardProps) {
    const getCategoryColor = (category: string) => {
        const colors: Record<string, string> = {
            'Safety': 'bg-red-100 text-red-800',
            'Clinical': 'bg-blue-100 text-blue-800',
            'Behavioral': 'bg-purple-100 text-purple-800',
            'Compliance': 'bg-yellow-100 text-yellow-800',
            'Leadership': 'bg-green-100 text-green-800',
        };
        return colors[category] || 'bg-gray-100 text-gray-800';
    };

    const getStatusBadge = () => {
        if (module.completed) {
            return <Badge className="bg-green-100 text-green-800">Completed</Badge>;
        }
        if (module.progress > 0) {
            return <Badge className="bg-blue-100 text-blue-800">In Progress</Badge>;
        }
        if (module.requiredForCert) {
            return <Badge className="bg-orange-100 text-orange-800">Required</Badge>;
        }
        return null;
    };

    const handleAction = () => {
        if (module.completed && onReview) {
            onReview(module.id);
        } else if (module.progress > 0 && onContinue) {
            onContinue(module.id);
        } else if (onStart) {
            onStart(module.id);
        }
    };

    const getActionLabel = () => {
        if (module.completed) return 'Review';
        if (module.progress > 0) return 'Continue';
        return 'Start Module';
    };

    return (
        <Card className="p-6 hover:shadow-lg transition-shadow">
            <div className="flex items-start justify-between mb-4">
                <div className="flex-1">
                    <div className="flex items-center gap-2 mb-2">
                        <Badge className={getCategoryColor(module.category)}>
                            {module.category}
                        </Badge>
                        {getStatusBadge()}
                    </div>
                    <h3 className="text-lg font-semibold text-gray-900 mb-2">
                        {module.title}
                    </h3>
                </div>
            </div>

            <p className="text-sm text-gray-600 mb-4 line-clamp-2">
                {module.description}
            </p>

            <div className="space-y-3">
                {/* Progress bar for modules in progress */}
                {module.progress > 0 && !module.completed && (
                    <div>
                        <div className="flex justify-between text-sm mb-1">
                            <span className="text-gray-600">Progress</span>
                            <span className="font-medium text-gray-900">{module.progress}%</span>
                        </div>
                        <Progress value={module.progress} color="blue" />
                    </div>
                )}

                <div className="flex items-center justify-between text-sm">
                    <div className="flex items-center gap-4">
                        <span className="text-gray-600">
                            <svg className="w-4 h-4 inline mr-1" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                                <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M12 8v4l3 3m6-3a9 9 0 11-18 0 9 9 0 0118 0z" />
                            </svg>
                            {module.duration} min
                        </span>
                        {module.dueDate && (
                            <span className="text-gray-600">
                                <svg className="w-4 h-4 inline mr-1" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                                    <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M8 7V3m8 4V3m-9 8h10M5 21h14a2 2 0 002-2V7a2 2 0 00-2-2H5a2 2 0 00-2 2v12a2 2 0 002 2z" />
                                </svg>
                                Due: {new Date(module.dueDate).toLocaleDateString()}
                            </span>
                        )}
                    </div>
                </div>

                <Button
                    onClick={handleAction}
                    variant={module.completed ? 'secondary' : 'primary'}
                    className="w-full mt-2"
                >
                    {getActionLabel()}
                </Button>
            </div>
        </Card>
    );
}
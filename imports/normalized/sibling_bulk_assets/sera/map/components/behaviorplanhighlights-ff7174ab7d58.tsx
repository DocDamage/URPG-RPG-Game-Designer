"use client";
import React from 'react';
import { Card } from '@/components/ui/card';
import { Badge } from '@/components/ui/badge';

interface BehaviorPlan {
    id: string;
    individualId: string;
    individualName?: string;
    planName: string;
    goals: string[];
    strategies: string[];
    triggers: string[];
    deEscalationSteps: string[];
    lastUpdated: string;
}

interface BehaviorPlanHighlightsProps {
    plans: BehaviorPlan[];
}

export function BehaviorPlanHighlights({ plans }: BehaviorPlanHighlightsProps) {
    if (plans.length === 0) {
        return (
            <Card className="p-6">
                <h3 className="text-lg font-semibold text-gray-900 mb-4">Behavior Plan Highlights</h3>
                <div className="text-center text-gray-500 py-4">
                    <p>No active behavior plans to review</p>
                </div>
            </Card>
        );
    }

    return (
        <Card className="p-6">
            <div className="flex items-center justify-between mb-4">
                <h3 className="text-lg font-semibold text-gray-900">Behavior Plan Highlights</h3>
                <Badge variant="info">{plans.length} plan{plans.length !== 1 ? 's' : ''}</Badge>
            </div>

            <div className="space-y-4">
                {plans.map((plan) => (
                    <div key={plan.id} className="p-4 bg-gray-50 rounded-lg border">
                        <div className="mb-3">
                            <h4 className="font-semibold text-gray-900">
                                {plan.individualName || `Individual ${plan.individualId}`}
                            </h4>
                            <p className="text-sm text-gray-600">{plan.planName}</p>
                        </div>

                        {plan.goals.length > 0 && (
                            <div className="mb-3">
                                <p className="text-xs font-medium text-gray-700 mb-1">Goals:</p>
                                <ul className="list-disc list-inside text-sm text-gray-700">
                                    {plan.goals.map((goal, idx) => (
                                        <li key={idx}>{goal}</li>
                                    ))}
                                </ul>
                            </div>
                        )}

                        {plan.triggers.length > 0 && (
                            <div className="mb-3 p-2 bg-yellow-50 rounded border border-yellow-200">
                                <p className="text-xs font-medium text-yellow-900 mb-1">⚠️ Known Triggers:</p>
                                <ul className="list-disc list-inside text-xs text-yellow-800">
                                    {plan.triggers.map((trigger, idx) => (
                                        <li key={idx}>{trigger}</li>
                                    ))}
                                </ul>
                            </div>
                        )}

                        {plan.deEscalationSteps.length > 0 && (
                            <div className="mb-3 p-2 bg-blue-50 rounded border border-blue-200">
                                <p className="text-xs font-medium text-blue-900 mb-1">De-escalation Steps:</p>
                                <ol className="list-decimal list-inside text-xs text-blue-800">
                                    {plan.deEscalationSteps.map((step, idx) => (
                                        <li key={idx}>{step}</li>
                                    ))}
                                </ol>
                            </div>
                        )}

                        {plan.strategies.length > 0 && (
                            <div>
                                <p className="text-xs font-medium text-gray-700 mb-1">Strategies:</p>
                                <ul className="list-disc list-inside text-sm text-gray-700">
                                    {plan.strategies.slice(0, 3).map((strategy, idx) => (
                                        <li key={idx}>{strategy}</li>
                                    ))}
                                    {plan.strategies.length > 3 && (
                                        <li className="text-xs text-gray-500">
                                            +{plan.strategies.length - 3} more strategies
                                        </li>
                                    )}
                                </ul>
                            </div>
                        )}
                    </div>
                ))}
            </div>

            <div className="mt-4 p-3 bg-blue-50 rounded border border-blue-200">
                <p className="text-xs text-blue-800">
                    💡 <strong>Tip:</strong> Review full behavior plans in individual records before shift start
                </p>
            </div>
        </Card>
    );
}


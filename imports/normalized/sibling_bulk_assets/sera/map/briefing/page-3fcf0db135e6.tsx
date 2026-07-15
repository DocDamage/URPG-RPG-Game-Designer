"use client";
import React from 'react';
import { BriefingCard } from '@/components/BriefingCard';
import { OfflineIndicator } from '@/components/OfflineIndicator';

export default function BriefingPage() {
    const userId = 'user123'; // Would come from auth

    return (
        <div className="min-h-screen bg-gray-100 p-6">
            <OfflineIndicator />

            <div className="max-w-4xl mx-auto">
                <div className="mb-6">
                    <h1 className="text-3xl font-bold text-gray-900">Pre-Shift Briefing</h1>
                    <p className="text-gray-600 mt-2">Review important information before starting your shift</p>
                </div>

                <BriefingCard userId={userId} />

                <div className="mt-6 bg-blue-50 border border-blue-200 rounded-lg p-4">
                    <h3 className="font-semibold text-blue-900 mb-2">💡 Briefing Tips</h3>
                    <ul className="text-sm text-blue-800 space-y-1">
                        <li>• Review all risk alerts before interacting with individuals</li>
                        <li>• Check medication reminders for today's schedule</li>
                        <li>• Note any overnight events that may affect behavior</li>
                        <li>• Acknowledge briefing to confirm you've reviewed the information</li>
                    </ul>
                </div>
            </div>
        </div>
    );
}

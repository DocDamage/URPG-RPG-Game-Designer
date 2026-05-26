'use client';

import React from 'react';
import { Card } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import Link from 'next/link';

/**
 * Operations Home Page
 * 
 * Main hub for operational tools and administrative functions.
 */
export default function OperationsPage() {
    const operations = [
        {
            title: 'Timesheets',
            description: 'Manage staff timesheets and hours',
            href: '/operations/timesheets',
            icon: '⏰',
            color: 'bg-blue-50 border-blue-200',
        },
        {
            title: 'Mileage Tracking',
            description: 'Track and manage mileage expenses',
            href: '/operations/mileage',
            icon: '🚗',
            color: 'bg-green-50 border-green-200',
        },
        {
            title: 'Inventory Management',
            description: 'Track supplies and inventory levels',
            href: '/operations/inventory',
            icon: '📦',
            color: 'bg-purple-50 border-purple-200',
        },
        {
            title: 'Expense Tracking',
            description: 'Record and manage operational expenses',
            href: '/operations/expenses',
            icon: '💰',
            color: 'bg-orange-50 border-orange-200',
        },
    ];

    return (
        <main className="container mx-auto p-6 bg-gradient-to-b from-gray-50 to-white min-h-screen">
            <div className="max-w-7xl mx-auto">
                {/* Header */}
                <div className="mb-8">
                    <h1 className="text-3xl font-bold text-gray-800 mb-2">Operations</h1>
                    <p className="text-gray-600">Manage operational tasks and administrative functions</p>
                </div>

                {/* Operations Grid */}
                <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
                    {operations.map((op) => (
                        <Link key={op.title} href={op.href}>
                            <Card
                                className={`p-6 hover:shadow-lg transition-shadow cursor-pointer border-2 ${op.color}`}
                            >
                                <div className="flex items-start space-x-4">
                                    <div className="text-4xl">{op.icon}</div>
                                    <div className="flex-1">
                                        <h2 className="text-xl font-semibold text-gray-800 mb-2">
                                            {op.title}
                                        </h2>
                                        <p className="text-gray-600 mb-4">{op.description}</p>
                                        <Button variant="outline" size="sm">
                                            Open →
                                        </Button>
                                    </div>
                                </div>
                            </Card>
                        </Link>
                    ))}
                </div>
            </div>
        </main>
    );
}



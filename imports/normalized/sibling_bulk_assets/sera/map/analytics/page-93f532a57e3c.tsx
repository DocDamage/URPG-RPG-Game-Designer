'use client';

import React from 'react';
import { Card } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Badge } from '@/components/ui/badge';
import Link from 'next/link';

/**
 * Analytics Home Page
 * 
 * Main dashboard for analytics and reporting features.
 * Provides navigation to different analytics views.
 */
export default function AnalyticsPage() {
    const analyticsCategories = [
        {
            title: 'Trend Analysis',
            description: 'View trends and patterns over time',
            href: '/analytics/trends',
            icon: '📈',
            color: 'bg-blue-50 border-blue-200',
        },
        {
            title: 'Compliance Dashboard',
            description: 'Monitor compliance scores and metrics',
            href: '/analytics/compliance',
            icon: '✅',
            color: 'bg-green-50 border-green-200',
        },
        {
            title: 'Quality Metrics',
            description: 'Track quality indicators and outcomes',
            href: '/analytics/quality',
            icon: '⭐',
            color: 'bg-purple-50 border-purple-200',
        },
        {
            title: 'Benchmarking',
            description: 'Compare performance against benchmarks',
            href: '/analytics/benchmarking',
            icon: '📊',
            color: 'bg-orange-50 border-orange-200',
        },
    ];

    const quickStats = [
        { label: 'Overall Compliance', value: '94%', trend: '+2%', status: 'good' },
        { label: 'Quality Score', value: '87', trend: '+3', status: 'good' },
        { label: 'Incidents (30d)', value: '12', trend: '-3', status: 'good' },
        { label: 'Staff Satisfaction', value: '4.2/5', trend: '+0.1', status: 'good' },
    ];

    return (
        <main className="container mx-auto p-6 bg-gradient-to-b from-gray-50 to-white min-h-screen">
            <div className="max-w-7xl mx-auto">
                {/* Header */}
                <div className="mb-8">
                    <h1 className="text-3xl font-bold text-gray-800 mb-2">Analytics & Insights</h1>
                    <p className="text-gray-600">Data-driven insights for better decision making</p>
                </div>

                {/* Quick Stats */}
                <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-4 mb-8">
                    {quickStats.map((stat) => (
                        <Card key={stat.label} className="p-4">
                            <div className="flex items-center justify-between">
                                <div>
                                    <p className="text-sm text-gray-600 mb-1">{stat.label}</p>
                                    <p className="text-2xl font-bold text-gray-800">{stat.value}</p>
                                </div>
                                <Badge
                                    variant={stat.status === 'good' ? 'default' : 'secondary'}
                                    className="text-xs"
                                >
                                    {stat.trend}
                                </Badge>
                            </div>
                        </Card>
                    ))}
                </div>

                {/* Analytics Categories */}
                <div className="grid grid-cols-1 md:grid-cols-2 gap-6 mb-8">
                    {analyticsCategories.map((category) => (
                        <Link key={category.title} href={category.href}>
                            <Card
                                className={`p-6 hover:shadow-lg transition-shadow cursor-pointer border-2 ${category.color}`}
                            >
                                <div className="flex items-start space-x-4">
                                    <div className="text-4xl">{category.icon}</div>
                                    <div className="flex-1">
                                        <h2 className="text-xl font-semibold text-gray-800 mb-2">
                                            {category.title}
                                        </h2>
                                        <p className="text-gray-600 mb-4">{category.description}</p>
                                        <Button variant="outline" size="sm">
                                            View Details →
                                        </Button>
                                    </div>
                                </div>
                            </Card>
                        </Link>
                    ))}
                </div>

                {/* Quick Actions */}
                <Card className="p-6">
                    <h2 className="text-xl font-semibold text-gray-800 mb-4">Quick Actions</h2>
                    <div className="flex flex-wrap gap-3">
                        <Link href="/reports">
                            <Button>Generate Report</Button>
                        </Link>
                        <Link href="/analytics/trends">
                            <Button variant="outline">View Trends</Button>
                        </Link>
                        <Link href="/analytics/compliance">
                            <Button variant="outline">Compliance Dashboard</Button>
                        </Link>
                        <Link href="/supervisor/analytics">
                            <Button variant="outline">Supervisor Analytics</Button>
                        </Link>
                    </div>
                </Card>
            </div>
        </main>
    );
}



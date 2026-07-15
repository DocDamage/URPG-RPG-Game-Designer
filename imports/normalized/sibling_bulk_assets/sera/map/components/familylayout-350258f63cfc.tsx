'use client';

import React from 'react';
import Link from 'next/link';
import { usePathname } from 'next/navigation';
import { Button } from '@/components/ui/button';
import { Badge } from '@/components/ui/badge';

/**
 * FamilyLayout Component
 * 
 * Provides a consistent layout and navigation for the family portal.
 * Includes sidebar navigation and header.
 */
interface FamilyLayoutProps {
    children: React.ReactNode;
    familyInfo?: {
        name?: string;
        relationship?: string;
        avatarUrl?: string;
    };
}

export function FamilyLayout({ children, familyInfo }: FamilyLayoutProps) {
    const pathname = usePathname();

    const navItems = [
        { href: '/family', label: 'Home', icon: '🏠' },
        { href: '/family/daily', label: 'Daily Summary', icon: '📅' },
        { href: '/family/milestones', label: 'Milestones', icon: '⭐' },
        { href: '/family/media', label: 'Media', icon: '📸' },
        { href: '/family/messages', label: 'Messages', icon: '💬' },
        { href: '/family/goals', label: 'Goals', icon: '🎯' },
    ];

    const isActive = (href: string) => {
        if (href === '/family') {
            return pathname === '/family';
        }
        return pathname?.startsWith(href);
    };

    return (
        <div className="min-h-screen bg-gradient-to-b from-indigo-50 to-white">
            {/* Header */}
            <header className="bg-white shadow-sm border-b">
                <div className="container mx-auto px-6 py-4">
                    <div className="flex items-center justify-between">
                        <div className="flex items-center space-x-4">
                            {familyInfo?.avatarUrl ? (
                                <img
                                    src={familyInfo.avatarUrl}
                                    alt={familyInfo.name || 'Family'}
                                    className="w-12 h-12 rounded-full object-cover"
                                />
                            ) : (
                                <div className="w-12 h-12 rounded-full bg-indigo-100 flex items-center justify-center text-indigo-600 text-lg font-semibold">
                                    {familyInfo?.name?.charAt(0) || 'F'}
                                </div>
                            )}
                            <div>
                                <h1 className="text-xl font-bold text-gray-800">
                                    {familyInfo?.name || 'Family Portal'}
                                </h1>
                                {familyInfo?.relationship && (
                                    <p className="text-sm text-gray-600">{familyInfo.relationship}</p>
                                )}
                            </div>
                        </div>
                        <div className="flex items-center space-x-2">
                            <Badge variant="outline" className="text-sm">
                                Family Access
                            </Badge>
                        </div>
                    </div>
                </div>
            </header>

            <div className="container mx-auto px-6 py-6">
                <div className="flex flex-col lg:flex-row gap-6">
                    {/* Sidebar Navigation */}
                    <aside className="lg:w-64 flex-shrink-0">
                        <nav className="bg-white rounded-lg shadow-sm p-4 space-y-2">
                            {navItems.map((item) => (
                                <Link key={item.href} href={item.href}>
                                    <Button
                                        variant={isActive(item.href) ? 'default' : 'ghost'}
                                        className={`w-full justify-start ${
                                            isActive(item.href)
                                                ? 'bg-indigo-600 text-white'
                                                : 'text-gray-700 hover:bg-gray-100'
                                        }`}
                                    >
                                        <span className="mr-2">{item.icon}</span>
                                        {item.label}
                                    </Button>
                                </Link>
                            ))}
                        </nav>

                        {/* Quick Actions */}
                        <div className="mt-6 bg-white rounded-lg shadow-sm p-4">
                            <h3 className="font-semibold text-gray-700 mb-3">Quick Actions</h3>
                            <div className="space-y-2">
                                <Link href="/family/messages">
                                    <Button variant="outline" className="w-full justify-start">
                                        💬 Send Message
                                    </Button>
                                </Link>
                                <Link href="/family/media">
                                    <Button variant="outline" className="w-full justify-start">
                                        📸 View Photos
                                    </Button>
                                </Link>
                            </div>
                        </div>
                    </aside>

                    {/* Main Content */}
                    <main className="flex-1">
                        {children}
                    </main>
                </div>
            </div>

            {/* Footer */}
            <footer className="bg-white border-t mt-12">
                <div className="container mx-auto px-6 py-4">
                    <div className="flex items-center justify-between text-sm text-gray-600">
                        <p>© 2024 S.E.R.A. Family Portal</p>
                        <div className="flex space-x-4">
                            <Link href="/family" className="hover:text-indigo-600">
                                Privacy Policy
                            </Link>
                            <Link href="/family" className="hover:text-indigo-600">
                                Terms of Service
                            </Link>
                            <Link href="/family/messages" className="hover:text-indigo-600">
                                Contact Support
                            </Link>
                        </div>
                    </div>
                </div>
            </footer>
        </div>
    );
}


import React, { useState } from 'react';

export interface Tab {
    id: string;
    label: string;
    content: React.ReactNode;
}

export interface TabsProps {
    tabs: Tab[];
    defaultTab?: string;
    onChange?: (tabId: string) => void;
}

export function Tabs({ tabs, defaultTab, onChange }: TabsProps) {
    const [activeTab, setActiveTab] = useState(defaultTab || tabs[0]?.id);

    const handleTabClick = (tabId: string) => {
        setActiveTab(tabId);
        onChange?.(tabId);
    };

    const activeContent = tabs.find(t => t.id === activeTab)?.content;

    return (
        <div className="w-full">
            {/* Tab Headers */}
            <div className="border-b border-gray-200">
                <nav className="flex space-x-8" aria-label="Tabs">
                    {tabs.map(tab => (
                        <button
                            key={tab.id}
                            onClick={() => handleTabClick(tab.id)}
                            className={`
                py-4 px-1 border-b-2 font-medium text-sm whitespace-nowrap
                transition-colors duration-200
                ${activeTab === tab.id
                                    ? 'border-blue-500 text-blue-600'
                                    : 'border-transparent text-gray-500 hover:text-gray-700 hover:border-gray-300'
                                }
              `}
                        >
                            {tab.label}
                        </button>
                    ))}
                </nav>
            </div>

            {/* Tab Content */}
            <div className="py-4">
                {activeContent}
            </div>
        </div>
    );
}

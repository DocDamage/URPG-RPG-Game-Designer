/**
 * Shortcuts Help Modal
 * Displays all available keyboard shortcuts with search and filtering
 */

'use client';

import React, { useState, useMemo, useCallback, useEffect, useRef } from 'react';
import { 
  Search, 
  X, 
  Keyboard, 
  Printer,
  ChevronDown,
  ChevronRight,
  Monitor,
  Info
} from 'lucide-react';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Badge } from '@/components/ui/badge';
import { ScrollArea } from '@/components/ui/scroll-area';
import { useTheme } from '@/components/theme-provider';
import { 
  Shortcut, 
  ShortcutCategory, 
  UserRole,
  ShortcutSearchResult 
} from './types';
import { 
  getAllShortcuts, 
  getCategoryDisplay, 
  getSortedCategories,
  CATEGORY_CONFIG 
} from './shortcuts';
import { buildShortcutDisplay, getPlatform } from './shortcutEngine';

interface ShortcutsHelpModalProps {
  isOpen: boolean;
  onClose: () => void;
  userRole?: UserRole | null;
}

// Fuzzy search function
function fuzzySearch(query: string, text: string): boolean {
  const queryLower = query.toLowerCase();
  const textLower = text.toLowerCase();
  let queryIndex = 0;
  
  for (let i = 0; i < textLower.length && queryIndex < queryLower.length; i++) {
    if (textLower[i] === queryLower[queryIndex]) {
      queryIndex++;
    }
  }
  
  return queryIndex === queryLower.length;
}

// Calculate search score
function calculateScore(query: string, shortcut: Shortcut): number {
  let score = 0;
  const queryLower = query.toLowerCase();
  const descLower = shortcut.description.toLowerCase();
  const keyLower = shortcut.key.toLowerCase();
  const categoryLower = shortcut.category.toLowerCase();
  
  // Exact match in description is highest
  if (descLower.includes(queryLower)) {
    score += 10;
    if (descLower.startsWith(queryLower)) score += 5;
  }
  
  // Match in key
  if (keyLower === queryLower) {
    score += 8;
  } else if (keyLower.includes(queryLower)) {
    score += 4;
  }
  
  // Match in category
  if (categoryLower.includes(queryLower)) {
    score += 2;
  }
  
  return score;
}

export function ShortcutsHelpModal({ isOpen, onClose, userRole }: ShortcutsHelpModalProps) {
  const [searchQuery, setSearchQuery] = useState('');
  const [selectedCategory, setSelectedCategory] = useState<ShortcutCategory | 'all'>('all');
  const [expandedCategories, setExpandedCategories] = useState<Set<string>>(new Set());
  const [showPrintView, setShowPrintView] = useState(false);
  const searchInputRef = useRef<HTMLInputElement>(null);
  const { theme } = useTheme();
  const platform = getPlatform();

  // Get all shortcuts
  const allShortcuts = useMemo(() => {
    // Mock router for shortcut generation
    const router = { push: (path: string) => console.log('Navigate to:', path) };
    return getAllShortcuts(router);
  }, []);

  // Filter shortcuts by role
  const roleFilteredShortcuts = useMemo(() => {
    if (!userRole) {
      return allShortcuts.filter(s => !s.roles || s.roles.length === 0);
    }
    return allShortcuts.filter(s => 
      !s.roles || s.roles.length === 0 || s.roles.includes(userRole)
    );
  }, [allShortcuts, userRole]);

  // Search and filter shortcuts
  const filteredShortcuts = useMemo(() => {
    let shortcuts = roleFilteredShortcuts;
    
    // Filter by category
    if (selectedCategory !== 'all') {
      shortcuts = shortcuts.filter(s => s.category === selectedCategory);
    }
    
    // Filter by search query
    if (searchQuery.trim()) {
      const results: ShortcutSearchResult[] = shortcuts
        .map(shortcut => ({
          shortcut: shortcut as Shortcut & { displayKey: string },
          score: calculateScore(searchQuery, shortcut),
          matches: {
            description: fuzzySearch(searchQuery, shortcut.description),
            key: fuzzySearch(searchQuery, shortcut.key),
            category: fuzzySearch(searchQuery, shortcut.category)
          }
        }))
        .filter(r => r.score > 0)
        .sort((a, b) => b.score - a.score);
      
      shortcuts = results.map(r => r.shortcut);
    }
    
    return shortcuts;
  }, [roleFilteredShortcuts, selectedCategory, searchQuery]);

  // Group shortcuts by category
  const groupedShortcuts = useMemo(() => {
    const groups: Record<string, Shortcut[]> = {};
    
    for (const shortcut of filteredShortcuts) {
      if (!groups[shortcut.category]) {
        groups[shortcut.category] = [];
      }
      groups[shortcut.category].push(shortcut);
    }
    
    return groups;
  }, [filteredShortcuts]);

  // Toggle category expansion
  const toggleCategory = useCallback((category: string) => {
    setExpandedCategories(prev => {
      const next = new Set(prev);
      if (next.has(category)) {
        next.delete(category);
      } else {
        next.add(category);
      }
      return next;
    });
  }, []);

  // Expand all categories
  const expandAll = useCallback(() => {
    setExpandedCategories(new Set(Object.keys(groupedShortcuts)));
  }, [groupedShortcuts]);

  // Collapse all categories
  const collapseAll = useCallback(() => {
    setExpandedCategories(new Set());
  }, []);

  // Handle print
  const handlePrint = useCallback(() => {
    setShowPrintView(true);
    setTimeout(() => {
      window.print();
      setShowPrintView(false);
    }, 100);
  }, []);

  // Clear search
  const clearSearch = useCallback(() => {
    setSearchQuery('');
    searchInputRef.current?.focus();
  }, []);

  // Keyboard navigation
  useEffect(() => {
    if (!isOpen) return;

    const handleKeyDown = (e: KeyboardEvent) => {
      // Escape closes modal
      if (e.key === 'Escape') {
        onClose();
      }
      
      // Ctrl+P for print
      if (e.key === 'p' && e.ctrlKey) {
        e.preventDefault();
        handlePrint();
      }
      
      // / focuses search
      if (e.key === '/' && !e.ctrlKey && !e.altKey && !e.metaKey) {
        e.preventDefault();
        searchInputRef.current?.focus();
      }
    };

    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, [isOpen, onClose, handlePrint]);

  // Focus search on open
  useEffect(() => {
    if (isOpen) {
      setTimeout(() => searchInputRef.current?.focus(), 100);
    }
  }, [isOpen]);

  // Count shortcuts per category
  const categoryCounts = useMemo(() => {
    const counts: Record<string, number> = {};
    for (const shortcut of roleFilteredShortcuts) {
      counts[shortcut.category] = (counts[shortcut.category] || 0) + 1;
    }
    return counts;
  }, [roleFilteredShortcuts]);

  // Sorted categories
  const sortedCategories = useMemo(() => {
    return getSortedCategories().filter(cat => categoryCounts[cat] > 0);
  }, [categoryCounts]);

  if (!isOpen) return null;

  return (
    <>
      {/* Print View - Hidden normally */}
      {showPrintView && (
        <div className="hidden print:block">
          <PrintableCheatSheet 
            shortcuts={roleFilteredShortcuts} 
            userRole={userRole}
          />
        </div>
      )}

      {/* Normal View */}
      <div className="fixed inset-0 z-50 flex items-center justify-center print:hidden">
        {/* Backdrop */}
        <div 
          className="fixed inset-0 bg-black/50 backdrop-blur-sm"
          onClick={onClose}
        />
        
        {/* Modal */}
        <div className="relative z-50 w-full max-w-4xl max-h-[90vh] bg-white dark:bg-gray-900 rounded-lg shadow-xl flex flex-col m-4">
          {/* Header */}
          <div className="flex items-center justify-between px-6 py-4 border-b dark:border-gray-700">
            <div className="flex items-center gap-3">
              <div className="p-2 bg-blue-100 dark:bg-blue-900/30 rounded-lg">
                <Keyboard className="w-6 h-6 text-blue-600 dark:text-blue-400" />
              </div>
              <div>
                <h2 className="text-xl font-semibold dark:text-white">
                  Keyboard Shortcuts
                </h2>
                <p className="text-sm text-gray-500 dark:text-gray-400">
                  Press <kbd className="px-1.5 py-0.5 bg-gray-100 dark:bg-gray-800 rounded text-xs">?</kbd> anytime to show this help
                </p>
              </div>
            </div>
            <div className="flex items-center gap-2">
              <Button
                variant="outline"
                size="sm"
                onClick={handlePrint}
                className="gap-2"
              >
                <Printer className="w-4 h-4" />
                Print
              </Button>
              <Button
                variant="ghost"
                size="sm"
                onClick={onClose}
                className="h-9 w-9 p-0"
              >
                <X className="w-5 h-5" />
              </Button>
            </div>
          </div>

          {/* Search and Filter Bar */}
          <div className="px-6 py-3 border-b dark:border-gray-700 space-y-3">
            <div className="relative">
              <Search className="absolute left-3 top-1/2 -translate-y-1/2 w-4 h-4 text-gray-400" />
              <Input
                ref={searchInputRef}
                value={searchQuery}
                onChange={(e) => setSearchQuery(e.target.value)}
                placeholder="Search shortcuts..."
                className="pl-10 pr-20"
              />
              <div className="absolute right-2 top-1/2 -translate-y-1/2 flex items-center gap-1">
                {searchQuery && (
                  <Button
                    variant="ghost"
                    size="sm"
                    className="h-6 w-6 p-0"
                    onClick={clearSearch}
                  >
                    <X className="w-3 h-3" />
                  </Button>
                )}
                <kbd className="hidden sm:inline-block px-1.5 py-0.5 bg-gray-100 dark:bg-gray-800 rounded text-xs text-gray-500">
                  /
                </kbd>
              </div>
            </div>

            {/* Category Tabs */}
            <div className="flex flex-wrap gap-2">
              <button
                onClick={() => setSelectedCategory('all')}
                className={`px-3 py-1.5 text-xs font-medium rounded-full transition-colors ${
                  selectedCategory === 'all'
                    ? 'bg-blue-100 text-blue-700 dark:bg-blue-900/30 dark:text-blue-300'
                    : 'bg-gray-100 text-gray-600 hover:bg-gray-200 dark:bg-gray-800 dark:text-gray-400'
                }`}
              >
                All
                <span className="ml-1.5 px-1.5 py-0.5 bg-white dark:bg-gray-700 rounded-full text-[10px]">
                  {roleFilteredShortcuts.length}
                </span>
              </button>
              {sortedCategories.map(category => {
                const { label } = getCategoryDisplay(category);
                return (
                  <button
                    key={category}
                    onClick={() => setSelectedCategory(category)}
                    className={`px-3 py-1.5 text-xs font-medium rounded-full transition-colors ${
                      selectedCategory === category
                        ? 'bg-blue-100 text-blue-700 dark:bg-blue-900/30 dark:text-blue-300'
                        : 'bg-gray-100 text-gray-600 hover:bg-gray-200 dark:bg-gray-800 dark:text-gray-400'
                    }`}
                  >
                    {label}
                    <span className="ml-1.5 px-1.5 py-0.5 bg-white dark:bg-gray-700 rounded-full text-[10px]">
                      {categoryCounts[category]}
                    </span>
                  </button>
                );
              })}
            </div>

            {/* Expand/Collapse Controls */}
            <div className="flex items-center justify-end gap-2 text-xs">
              <Button variant="link" size="sm" onClick={expandAll} className="h-auto py-0">
                Expand All
              </Button>
              <span className="text-gray-400">|</span>
              <Button variant="link" size="sm" onClick={collapseAll} className="h-auto py-0">
                Collapse All
              </Button>
            </div>
          </div>

          {/* Shortcuts List */}
          <ScrollArea className="flex-1 px-6 py-4 min-h-[400px] max-h-[60vh]">
            {Object.keys(groupedShortcuts).length === 0 ? (
              <div className="text-center py-12">
                <Search className="w-12 h-12 mx-auto text-gray-300 mb-4" />
                <p className="text-gray-500">
                  No shortcuts found matching &quot;{searchQuery}&quot;
                </p>
              </div>
            ) : (
              <div className="space-y-4">
                {sortedCategories
                  .filter(cat => groupedShortcuts[cat])
                  .map(category => {
                    const shortcuts = groupedShortcuts[category];
                    const isExpanded = expandedCategories.has(category) || searchQuery.length > 0;
                    const { label, color } = getCategoryDisplay(category);

                    return (
                      <div 
                        key={category} 
                        className="border dark:border-gray-700 rounded-lg overflow-hidden"
                      >
                        {/* Category Header */}
                        <button
                          onClick={() => toggleCategory(category)}
                          className="w-full flex items-center justify-between px-4 py-3 bg-gray-50 dark:bg-gray-800/50 hover:bg-gray-100 dark:hover:bg-gray-800 transition-colors"
                        >
                          <div className="flex items-center gap-3">
                            <div 
                              className="w-3 h-3 rounded-full"
                              style={{ backgroundColor: color }}
                            />
                            <span className="font-medium dark:text-white">{label}</span>
                            <Badge variant="secondary" size="sm">
                              {shortcuts.length}
                            </Badge>
                          </div>
                          {isExpanded ? (
                            <ChevronDown className="w-4 h-4 text-gray-400" />
                          ) : (
                            <ChevronRight className="w-4 h-4 text-gray-400" />
                          )}
                        </button>

                        {/* Category Content */}
                        {isExpanded && (
                          <div className="divide-y dark:divide-gray-700">
                            {shortcuts.map(shortcut => (
                              <ShortcutRow 
                                key={shortcut.id} 
                                shortcut={shortcut}
                                platform={platform}
                              />
                            ))}
                          </div>
                        )}
                      </div>
                    );
                  })}
              </div>
            )}
          </ScrollArea>

          {/* Footer */}
          <div className="px-6 py-3 border-t dark:border-gray-700 bg-gray-50 dark:bg-gray-800/50 flex items-center justify-between text-xs text-gray-500">
            <div className="flex items-center gap-4">
              <span className="flex items-center gap-1">
                <Monitor className="w-3 h-3" />
                Platform: {platform === 'mac' ? 'Mac' : platform === 'windows' ? 'Windows' : 'Linux'}
              </span>
            </div>
            <div className="flex items-center gap-1">
              <Info className="w-3 h-3" />
              {filteredShortcuts.length} shortcuts available
              {userRole && (
                <span className="ml-1">
                  for role: <span className="font-medium capitalize">{userRole.replace('_', ' ')}</span>
                </span>
              )}
            </div>
          </div>
        </div>
      </div>
    </>
  );
}

// Individual shortcut row component
interface ShortcutRowProps {
  shortcut: Shortcut;
  platform: string;
}

function ShortcutRow({ shortcut, platform }: ShortcutRowProps) {
  const displayKey = buildShortcutDisplay(
    shortcut.key, 
    shortcut.modifiers, 
    platform as any
  );
  
  // Split display key into parts
  const keyParts = displayKey.split('+');

  return (
    <div className="flex items-center justify-between px-4 py-3 hover:bg-gray-50 dark:hover:bg-gray-800/50 transition-colors">
      <div className="flex-1 min-w-0 mr-4">
        <p className="font-medium text-sm dark:text-gray-200">{shortcut.description}</p>
        {shortcut.roles && shortcut.roles.length > 0 && (
          <div className="flex flex-wrap gap-1 mt-1">
            {shortcut.roles.map(role => (
              <span 
                key={role}
                className="text-[10px] px-1.5 py-0.5 bg-gray-100 dark:bg-gray-700 text-gray-600 dark:text-gray-400 rounded"
              >
                {role.replace('_', ' ')}
              </span>
            ))}
          </div>
        )}
        {shortcut.contexts && shortcut.contexts.length > 0 && !shortcut.contexts.includes('global') && (
          <p className="text-xs text-gray-500 dark:text-gray-400 mt-1">
            Context: {shortcut.contexts.join(', ')}
          </p>
        )}
      </div>
      <div className="flex items-center gap-1 shrink-0">
        {keyParts.map((part, index) => (
          <React.Fragment key={index}>
            <kbd className="px-2 py-1 bg-gray-100 dark:bg-gray-800 border dark:border-gray-700 rounded text-xs font-mono min-w-[24px] text-center dark:text-gray-300">
              {part}
            </kbd>
            {index < keyParts.length - 1 && (
              <span className="text-gray-400">+</span>
            )}
          </React.Fragment>
        ))}
      </div>
    </div>
  );
}

// Printable cheat sheet component
interface PrintableCheatSheetProps {
  shortcuts: Shortcut[];
  userRole?: UserRole | null;
}

function PrintableCheatSheet({ shortcuts, userRole }: PrintableCheatSheetProps) {
  const platform = getPlatform();
  
  // Group by category
  const grouped: Record<string, Shortcut[]> = {};
  for (const s of shortcuts) {
    if (!grouped[s.category]) grouped[s.category] = [];
    grouped[s.category].push(s);
  }

  return (
    <div className="p-8">
      <div className="text-center mb-8">
        <h1 className="text-2xl font-bold mb-2">S.E.R.A. Keyboard Shortcuts</h1>
        <p className="text-gray-600">
          {userRole ? `For ${userRole.replace('_', ' ')} role` : 'All shortcuts'} • 
          Platform: {platform === 'mac' ? 'Mac' : platform === 'windows' ? 'Windows' : 'Linux'}
        </p>
      </div>

      <div className="grid grid-cols-2 gap-6">
        {getSortedCategories()
          .filter(cat => grouped[cat])
          .map(category => {
            const { label, color } = getCategoryDisplay(category);
            return (
              <div key={category} className="break-inside-avoid">
                <h2 
                  className="text-lg font-semibold mb-3 pb-1 border-b-2"
                  style={{ borderColor: color }}
                >
                  {label}
                </h2>
                <table className="w-full text-sm">
                  <tbody>
                    {grouped[category].map(shortcut => (
                      <tr key={shortcut.id} className="border-b border-gray-100">
                        <td className="py-2 pr-4">{shortcut.description}</td>
                        <td className="py-2 text-right font-mono whitespace-nowrap">
                          {buildShortcutDisplay(shortcut.key, shortcut.modifiers, platform as any)}
                        </td>
                      </tr>
                    ))}
                  </tbody>
                </table>
              </div>
            );
          })}
      </div>

      <div className="mt-8 pt-4 border-t text-center text-sm text-gray-600">
        <p>S.E.R.A. - Support, Evaluation, Reporting & Administration</p>
        <p>Press ? anywhere in the app to open this help</p>
      </div>
    </div>
  );
}

export default ShortcutsHelpModal;

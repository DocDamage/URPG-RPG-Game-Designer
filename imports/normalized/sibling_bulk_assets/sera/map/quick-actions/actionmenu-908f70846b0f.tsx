/**
 * ActionMenu Component
 * Radial or vertical menu displaying quick actions
 */

'use client';

import React, { useState, useMemo, useCallback } from 'react';
import { clsx } from 'clsx';
import {
  FileText,
  FilePlus,
  Mic,
  Pill,
  Clock,
  ClipboardList,
  AlertTriangle,
  Siren,
  Heart,
  MessageSquare,
  Bell,
  Phone,
  Play,
  Square,
  Calendar,
  Repeat,
  UserPlus,
  User,
  Target,
  ScanLine,
  Calculator,
  Search,
  HelpCircle,
  X,
  Pin,
  History,
  Star,
  ChevronRight,
  MoreHorizontal,
} from 'lucide-react';
import { QuickAction, ActionCategory, ActionGroup, UserRole } from './types';
import { CATEGORY_CONFIG } from './types';
import { Tooltip, TooltipTrigger, TooltipContent } from '../ui/tooltip';
import { Input } from '../ui/input';

// Icon mapping
const ICON_MAP: Record<string, React.ComponentType<{ className?: string }>> = {
  FileText,
  FilePlus,
  Mic,
  Pill,
  Clock,
  ClipboardList,
  AlertTriangle,
  Siren,
  Heart,
  MessageSquare,
  Bell,
  Phone,
  Play,
  Square,
  Calendar,
  Repeat,
  UserPlus,
  User,
  Target,
  ScanLine,
  Calculator,
  Search,
  HelpCircle,
  X,
  Pin,
  History,
  Star,
  ChevronRight,
  MoreHorizontal,
};

interface ActionMenuProps {
  isOpen: boolean;
  onClose: () => void;
  actions: QuickAction[];
  recentActions: QuickAction[];
  pinnedActions: QuickAction[];
  groupedActions: ActionGroup[];
  userRole: UserRole;
  menuType?: 'radial' | 'vertical' | 'horizontal';
  enableSearch?: boolean;
  enableTooltips?: boolean;
  showLabels?: boolean;
  selectedIndividualId?: string;
  onActionClick: (action: QuickAction) => void;
  onPinAction: (actionId: string) => void;
  onUnpinAction: (actionId: string) => void;
  animationDuration?: number;
}

export function ActionMenu({
  isOpen,
  onClose,
  actions,
  recentActions,
  pinnedActions,
  groupedActions,
  userRole,
  menuType = 'vertical',
  enableSearch = true,
  enableTooltips = true,
  showLabels = true,
  selectedIndividualId,
  onActionClick,
  onPinAction,
  onUnpinAction,
  animationDuration = 200,
}: ActionMenuProps) {
  const [searchQuery, setSearchQuery] = useState('');
  const [selectedCategory, setSelectedCategory] = useState<ActionCategory | 'all'>('all');
  const [hoveredAction, setHoveredAction] = useState<string | null>(null);
  const [expandedGroups, setExpandedGroups] = useState<Set<string>>(new Set());

  // Filter actions based on search and category
  const filteredActions = useMemo(() => {
    let result = actions;

    // Filter by search query
    if (searchQuery.trim()) {
      const query = searchQuery.toLowerCase();
      result = result.filter(
        action =>
          action.label.toLowerCase().includes(query) ||
          action.description.toLowerCase().includes(query) ||
          action.category.toLowerCase().includes(query)
      );
    }

    // Filter by category
    if (selectedCategory !== 'all') {
      result = result.filter(action => action.category === selectedCategory);
    }

    return result;
  }, [actions, searchQuery, selectedCategory]);

  // Get display actions (prioritize pinned and recent)
  const displayActions = useMemo(() => {
    if (searchQuery || selectedCategory !== 'all') {
      return filteredActions;
    }

    // Start with pinned actions
    const display = [...pinnedActions];

    // Add recent actions that aren't pinned
    recentActions.forEach(action => {
      if (!display.some(a => a.id === action.id)) {
        display.push(action);
      }
    });

    // Add remaining actions that aren't pinned or recent
    actions.forEach(action => {
      if (!display.some(a => a.id === action.id)) {
        display.push(action);
      }
    });

    return display.slice(0, 12); // Limit to 12 actions
  }, [filteredActions, pinnedActions, recentActions, actions, searchQuery, selectedCategory]);

  // Handle action click
  const handleActionClick = useCallback(
    (action: QuickAction) => {
      if (action.disabled) return;
      onActionClick(action);
    },
    [onActionClick]
  );

  // Handle pin toggle
  const handlePinToggle = useCallback(
    (e: React.MouseEvent, action: QuickAction) => {
      e.stopPropagation();
      const isPinned = pinnedActions.some(a => a.id === action.id);
      if (isPinned) {
        onUnpinAction(action.id);
      } else {
        onPinAction(action.id);
      }
    },
    [pinnedActions, onPinAction, onUnpinAction]
  );

  // Toggle group expansion
  const toggleGroup = useCallback((category: string) => {
    setExpandedGroups(prev => {
      const next = new Set(prev);
      if (next.has(category)) {
        next.delete(category);
      } else {
        next.add(category);
      }
      return next;
    });
  }, []);

  // Get icon component
  const getIcon = (iconName: string) => {
    const IconComponent = ICON_MAP[iconName] || HelpCircle;
    return <IconComponent className="w-5 h-5" />;
  };

  // Format keyboard shortcut for display
  const formatShortcut = (shortcut?: string) => {
    if (!shortcut) return null;
    return shortcut.replace('Ctrl+', '⌘').replace('Shift+', '⇧');
  };

  if (!isOpen) return null;

  return (
    <div
      className={clsx(
        'absolute z-50',
        menuType === 'vertical' && 'bottom-full right-0 mb-3',
        menuType === 'horizontal' && 'bottom-full right-0 mb-3',
        menuType === 'radial' && 'bottom-0 right-0'
      )}
    >
      {/* Menu Container */}
      <div
        className={clsx(
          'bg-white dark:bg-gray-800 rounded-2xl shadow-2xl border border-gray-200 dark:border-gray-700 overflow-hidden',
          'animate-in fade-in zoom-in-95 duration-200',
          menuType === 'vertical' && 'w-80',
          menuType === 'horizontal' && 'w-auto min-w-[400px]',
          menuType === 'radial' && 'w-80'
        )}
        style={{ animationDuration: `${animationDuration}ms` }}
      >
        {/* Header */}
        <div className="px-4 py-3 border-b border-gray-200 dark:border-gray-700 flex items-center justify-between">
          <div>
            <h3 className="font-semibold text-gray-900 dark:text-white">Quick Actions</h3>
            <p className="text-xs text-gray-500 dark:text-gray-400">
              {selectedIndividualId ? 'Individual selected' : 'No individual selected'}
            </p>
          </div>
          <button
            onClick={onClose}
            className="p-1.5 hover:bg-gray-100 dark:hover:bg-gray-700 rounded-lg text-gray-500 dark:text-gray-400 transition-colors"
            aria-label="Close menu"
          >
            <X className="w-4 h-4" />
          </button>
        </div>

        {/* Search Bar */}
        {enableSearch && (
          <div className="px-4 py-2 border-b border-gray-200 dark:border-gray-700">
            <div className="relative">
              <Search className="absolute left-3 top-1/2 -translate-y-1/2 w-4 h-4 text-gray-400" />
              <Input
                type="text"
                placeholder="Search actions..."
                value={searchQuery}
                onChange={e => setSearchQuery(e.target.value)}
                className="pl-9 pr-4 py-1.5 text-sm w-full"
              />
            </div>
          </div>
        )}

        {/* Category Tabs */}
        {!searchQuery && (
          <div className="px-4 py-2 border-b border-gray-200 dark:border-gray-700 overflow-x-auto">
            <div className="flex gap-1 min-w-max">
              <button
                onClick={() => setSelectedCategory('all')}
                className={clsx(
                  'px-3 py-1 text-xs font-medium rounded-full transition-colors whitespace-nowrap',
                  selectedCategory === 'all'
                    ? 'bg-blue-100 text-blue-700 dark:bg-blue-900 dark:text-blue-300'
                    : 'text-gray-600 dark:text-gray-400 hover:bg-gray-100 dark:hover:bg-gray-700'
                )}
              >
                All
              </button>
              {groupedActions.map(group => (
                <button
                  key={group.category}
                  onClick={() => setSelectedCategory(group.category)}
                  className={clsx(
                    'px-3 py-1 text-xs font-medium rounded-full transition-colors whitespace-nowrap flex items-center gap-1',
                    selectedCategory === group.category
                      ? 'bg-blue-100 text-blue-700 dark:bg-blue-900 dark:text-blue-300'
                      : 'text-gray-600 dark:text-gray-400 hover:bg-gray-100 dark:hover:bg-gray-700'
                  )}
                >
                  {getIcon(group.icon)}
                  <span className="hidden sm:inline">{group.label}</span>
                </button>
              ))}
            </div>
          </div>
        )}

        {/* Pinned Section */}
        {pinnedActions.length > 0 && !searchQuery && selectedCategory === 'all' && (
          <div className="px-4 py-2 border-b border-gray-200 dark:border-gray-700 bg-yellow-50/50 dark:bg-yellow-900/10">
            <div className="flex items-center gap-2 text-xs font-medium text-gray-600 dark:text-gray-400 mb-2">
              <Star className="w-3.5 h-3.5 text-yellow-500" />
              <span>Pinned</span>
            </div>
            <div className="grid grid-cols-4 gap-1">
              {pinnedActions.slice(0, 4).map(action => (
                <ActionButton
                  key={action.id}
                  action={action}
                  isPinned={true}
                  showLabel={false}
                  onClick={() => handleActionClick(action)}
                  onPinToggle={e => handlePinToggle(e, action)}
                  getIcon={getIcon}
                  enableTooltip={enableTooltips}
                />
              ))}
            </div>
          </div>
        )}

        {/* Recent Section */}
        {recentActions.length > 0 && !searchQuery && selectedCategory === 'all' && (
          <div className="px-4 py-2 border-b border-gray-200 dark:border-gray-700">
            <div className="flex items-center gap-2 text-xs font-medium text-gray-600 dark:text-gray-400 mb-2">
              <History className="w-3.5 h-3.5" />
              <span>Recent</span>
            </div>
            <div className="flex gap-1 overflow-x-auto pb-1">
              {recentActions.slice(0, 5).map(action => (
                <button
                  key={action.id}
                  onClick={() => handleActionClick(action)}
                  onMouseEnter={() => setHoveredAction(action.id)}
                  onMouseLeave={() => setHoveredAction(null)}
                  className={clsx(
                    'flex items-center gap-2 px-3 py-1.5 rounded-lg text-xs whitespace-nowrap transition-colors',
                    'bg-gray-100 dark:bg-gray-700 text-gray-700 dark:text-gray-300 hover:bg-gray-200 dark:hover:bg-gray-600',
                    action.disabled && 'opacity-50 cursor-not-allowed'
                  )}
                  disabled={action.disabled}
                  title={action.disabled ? action.disabledReason : action.description}
                >
                  {getIcon(action.icon)}
                  <span>{action.label}</span>
                </button>
              ))}
            </div>
          </div>
        )}

        {/* Actions Grid/List */}
        <div className="p-3 max-h-[50vh] overflow-y-auto">
          {menuType === 'vertical' ? (
            <div className="space-y-1">
              {displayActions.map(action => (
                <ActionListItem
                  key={action.id}
                  action={action}
                  isPinned={pinnedActions.some(a => a.id === action.id)}
                  onClick={() => handleActionClick(action)}
                  onPinToggle={e => handlePinToggle(e, action)}
                  getIcon={getIcon}
                  formatShortcut={formatShortcut}
                  enableTooltip={enableTooltips}
                />
              ))}
            </div>
          ) : (
            <div className="grid grid-cols-3 gap-2">
              {displayActions.map(action => (
                <ActionButton
                  key={action.id}
                  action={action}
                  isPinned={pinnedActions.some(a => a.id === action.id)}
                  showLabel={showLabels}
                  onClick={() => handleActionClick(action)}
                  onPinToggle={e => handlePinToggle(e, action)}
                  getIcon={getIcon}
                  enableTooltip={enableTooltips}
                />
              ))}
            </div>
          )}

          {displayActions.length === 0 && (
            <div className="text-center py-8 text-gray-500 dark:text-gray-400">
              <Search className="w-8 h-8 mx-auto mb-2 opacity-50" />
              <p className="text-sm">No actions found</p>
            </div>
          )}
        </div>

        {/* Footer */}
        <div className="px-4 py-2 border-t border-gray-200 dark:border-gray-700 bg-gray-50 dark:bg-gray-900/50">
          <div className="flex items-center justify-between text-xs text-gray-500 dark:text-gray-400">
            <span>Press ESC to close</span>
            <span>{filteredActions.length} available</span>
          </div>
        </div>
      </div>
    </div>
  );
}

// Action List Item (for vertical menu)
interface ActionListItemProps {
  action: QuickAction;
  isPinned: boolean;
  onClick: () => void;
  onPinToggle: (e: React.MouseEvent) => void;
  getIcon: (iconName: string) => React.ReactNode;
  formatShortcut: (shortcut?: string) => string | null;
  enableTooltip: boolean;
}

function ActionListItem({
  action,
  isPinned,
  onClick,
  onPinToggle,
  getIcon,
  formatShortcut,
  enableTooltip,
}: ActionListItemProps) {
  const shortcut = formatShortcut(action.shortcut);

  const content = (
    <button
      onClick={onClick}
      disabled={action.disabled}
      className={clsx(
        'w-full flex items-center gap-3 px-3 py-2.5 rounded-lg transition-colors text-left group',
        action.disabled
          ? 'opacity-50 cursor-not-allowed'
          : 'hover:bg-gray-100 dark:hover:bg-gray-700'
      )}
    >
      {/* Icon */}
      <div
        className={clsx(
          'w-9 h-9 rounded-lg flex items-center justify-center flex-shrink-0',
          action.color || 'bg-gray-100 dark:bg-gray-700 text-gray-600 dark:text-gray-400'
        )}
      >
        {getIcon(action.icon)}
      </div>

      {/* Content */}
      <div className="flex-1 min-w-0">
        <div className="flex items-center gap-2">
          <span className="font-medium text-sm text-gray-900 dark:text-white truncate">
            {action.label}
          </span>
          {action.badge && (
            <span className="px-1.5 py-0.5 text-[10px] font-medium bg-red-100 text-red-700 rounded-full">
              {action.badge}
            </span>
          )}
        </div>
        <p className="text-xs text-gray-500 dark:text-gray-400 truncate">
          {action.description}
        </p>
      </div>

      {/* Shortcut */}
      {shortcut && (
        <kbd className="hidden sm:inline-block px-2 py-0.5 text-[10px] font-mono bg-gray-100 dark:bg-gray-700 text-gray-500 dark:text-gray-400 rounded">
          {shortcut}
        </kbd>
      )}

      {/* Pin button */}
      <button
        onClick={onPinToggle}
        className={clsx(
          'p-1.5 rounded opacity-0 group-hover:opacity-100 transition-opacity',
          isPinned ? 'text-yellow-500' : 'text-gray-400 hover:text-gray-600 dark:hover:text-gray-300'
        )}
        title={isPinned ? 'Unpin' : 'Pin to top'}
      >
        <Pin className={clsx('w-3.5 h-3.5', isPinned && 'fill-current')} />
      </button>
    </button>
  );

  if (enableTooltip && action.disabled && action.disabledReason) {
    return (
      <Tooltip>
        <TooltipTrigger asChild>{content}</TooltipTrigger>
        <TooltipContent>{action.disabledReason}</TooltipContent>
      </Tooltip>
    );
  }

  return content;
}

// Action Button (for grid/radial menu)
interface ActionButtonProps {
  action: QuickAction;
  isPinned: boolean;
  showLabel: boolean;
  onClick: () => void;
  onPinToggle: (e: React.MouseEvent) => void;
  getIcon: (iconName: string) => React.ReactNode;
  enableTooltip: boolean;
}

function ActionButton({
  action,
  isPinned,
  showLabel,
  onClick,
  onPinToggle,
  getIcon,
  enableTooltip,
}: ActionButtonProps) {
  const [showPin, setShowPin] = useState(false);

  const content = (
    <div
      className="relative group"
      onMouseEnter={() => setShowPin(true)}
      onMouseLeave={() => setShowPin(false)}
    >
      <button
        onClick={onClick}
        disabled={action.disabled}
        className={clsx(
          'w-full flex flex-col items-center gap-1.5 p-2 rounded-lg transition-colors',
          action.disabled
            ? 'opacity-50 cursor-not-allowed'
            : 'hover:bg-gray-100 dark:hover:bg-gray-700'
        )}
        title={!enableTooltip ? action.description : undefined}
      >
        {/* Icon Container */}
        <div
          className={clsx(
            'w-10 h-10 rounded-xl flex items-center justify-center text-white shadow-sm',
            action.color || 'bg-gray-500'
          )}
        >
          {getIcon(action.icon)}
        </div>

        {/* Label */}
        {showLabel && (
          <span className="text-xs text-center text-gray-700 dark:text-gray-300 leading-tight line-clamp-2">
            {action.label}
          </span>
        )}
      </button>

      {/* Pin indicator */}
      {(isPinned || showPin) && (
        <button
          onClick={onPinToggle}
          className={clsx(
            'absolute -top-1 -right-1 w-5 h-5 rounded-full flex items-center justify-center shadow-sm transition-all',
            isPinned
              ? 'bg-yellow-400 text-yellow-800'
              : 'bg-gray-200 text-gray-500 opacity-0 group-hover:opacity-100'
          )}
        >
          <Pin className={clsx('w-3 h-3', isPinned && 'fill-current')} />
        </button>
      )}
    </div>
  );

  if (enableTooltip) {
    return (
      <Tooltip>
        <TooltipTrigger asChild>{content}</TooltipTrigger>
        <TooltipContent>
          <div className="text-left">
            <p className="font-medium">{action.label}</p>
            <p className="text-xs text-gray-400">{action.description}</p>
            {action.shortcut && (
              <p className="text-xs text-gray-500 mt-1">{action.shortcut}</p>
            )}
          </div>
        </TooltipContent>
      </Tooltip>
    );
  }

  return content;
}

export default ActionMenu;

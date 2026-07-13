/**
 * Filter Chips Component
 * 
 * Displays active filters as removable chips with inline editing capability.
 * Supports sorting, filtering, and overflow management.
 */

'use client';

import React, { useState, useCallback, useRef, useEffect } from 'react';
import { clsx } from 'clsx';
import {
  X,
  Edit2,
  SlidersHorizontal,
  ChevronRight,
  ChevronLeft,
} from 'lucide-react';
import { ActiveFilter, FilterChipsProps } from './types';

interface FilterChipProps {
  filter: ActiveFilter;
  onRemove: () => void;
  onEdit?: () => void;
  disabled?: boolean;
  className?: string;
}

function FilterChip({ filter, onRemove, onEdit, disabled, className }: FilterChipProps) {
  const [isEditing, setIsEditing] = useState(false);
  const [editValue, setEditValue] = useState(filter.displayValue);
  const inputRef = useRef<HTMLInputElement>(null);
  
  useEffect(() => {
    if (isEditing && inputRef.current) {
      inputRef.current.focus();
      inputRef.current.select();
    }
  }, [isEditing]);
  
  const handleEditSubmit = useCallback(() => {
    setIsEditing(false);
    // onEdit would be called with new value here
  }, []);
  
  const handleKeyDown = useCallback((e: React.KeyboardEvent) => {
    if (e.key === 'Enter') {
      handleEditSubmit();
    } else if (e.key === 'Escape') {
      setIsEditing(false);
      setEditValue(filter.displayValue);
    }
  }, [handleEditSubmit, filter.displayValue]);
  
  // Format the operator for display
  const formatOperator = (operator: string): string => {
    const operatorMap: Record<string, string> = {
      equals: '=',
      not_equals: '≠',
      contains: 'contains',
      starts_with: 'starts with',
      ends_with: 'ends with',
      greater_than: '>',
      less_than: '<',
      greater_or_equal: '≥',
      less_or_equal: '≤',
      between: 'between',
      in: 'in',
      not_in: 'not in',
      is_null: 'is empty',
      is_not_null: 'is not empty',
      date_before: 'before',
      date_after: 'after',
      date_between: 'between',
    };
    return operatorMap[operator] || operator;
  };
  
  return (
    <div
      className={clsx(
        'group inline-flex items-center max-w-xs',
        'bg-blue-50 border border-blue-200 rounded-full',
        'text-sm transition-all duration-200',
        'hover:bg-blue-100 hover:border-blue-300',
        disabled && 'opacity-50 cursor-not-allowed',
        className
      )}
    >
      {/* Filter Label */}
      <span className="px-2.5 py-1 text-blue-700 font-medium truncate">
        {filter.label}
      </span>
      
      {/* Operator */}
      <span className="text-blue-400 text-xs">
        {formatOperator(filter.operator)}
      </span>
      
      {/* Value - Editable or Display */}
      {isEditing ? (
        <input
          ref={inputRef}
          type="text"
          value={editValue}
          onChange={(e) => setEditValue(e.target.value)}
          onBlur={handleEditSubmit}
          onKeyDown={handleKeyDown}
          className="w-24 px-1 py-0.5 mx-1 text-sm bg-white border border-blue-300 rounded focus:outline-none focus:ring-1 focus:ring-blue-500"
        />
      ) : (
        <span 
          className={clsx(
            'px-2 py-1 text-blue-800 truncate max-w-[150px]',
            filter.editable !== false && 'cursor-text hover:underline'
          )}
          onClick={() => {
            if (filter.editable !== false && onEdit) {
              setIsEditing(true);
            }
          }}
          title={filter.displayValue}
        >
          {filter.displayValue}
        </span>
      )}
      
      {/* Edit Button (shown on hover) */}
      {filter.editable !== false && onEdit && !isEditing && (
        <button
          onClick={() => setIsEditing(true)}
          className="opacity-0 group-hover:opacity-100 p-1 text-blue-400 hover:text-blue-600 transition-opacity"
          title="Edit filter"
        >
          <Edit2 className="h-3 w-3" />
        </button>
      )}
      
      {/* Remove Button */}
      {filter.removable !== false && (
        <button
          onClick={onRemove}
          disabled={disabled}
          className="p-1 mr-1 text-blue-400 hover:text-red-500 hover:bg-red-50 rounded-full transition-colors"
          title="Remove filter"
        >
          <X className="h-3.5 w-3.5" />
        </button>
      )}
    </div>
  );
}

export function FilterChips({
  filters,
  onRemove,
  onEdit,
  onClearAll,
  sortable = false,
  maxVisible = 5,
  className,
}: FilterChipsProps) {
  const [visibleCount, setVisibleCount] = useState(maxVisible);
  const [showScrollButtons, setShowScrollButtons] = useState(false);
  const containerRef = useRef<HTMLDivElement>(null);
  
  // Check if scrolling is needed
  useEffect(() => {
    const checkOverflow = () => {
      if (containerRef.current) {
        const { scrollWidth, clientWidth } = containerRef.current;
        setShowScrollButtons(scrollWidth > clientWidth);
      }
    };
    
    checkOverflow();
    window.addEventListener('resize', checkOverflow);
    return () => window.removeEventListener('resize', checkOverflow);
  }, [filters]);
  
  const hasMoreFilters = filters.length > visibleCount;
  const visibleFilters = filters.slice(0, visibleCount);
  const hiddenCount = filters.length - visibleCount;
  
  const scroll = (direction: 'left' | 'right') => {
    if (containerRef.current) {
      const scrollAmount = 200;
      containerRef.current.scrollBy({
        left: direction === 'left' ? -scrollAmount : scrollAmount,
        behavior: 'smooth',
      });
    }
  };
  
  const handleShowMore = () => {
    setVisibleCount(prev => Math.min(prev + 5, filters.length));
  };
  
  const handleShowLess = () => {
    setVisibleCount(maxVisible);
  };
  
  if (filters.length === 0) {
    return null;
  }
  
  return (
    <div className={clsx('flex items-center space-x-2', className)}>
      {/* Filter Icon */}
      <div className="flex-shrink-0 p-1.5 text-gray-400">
        <SlidersHorizontal className="h-4 w-4" />
      </div>
      
      {/* Scroll Left Button */}
      {showScrollButtons && (
        <button
          onClick={() => scroll('left')}
          className="flex-shrink-0 p-1 text-gray-400 hover:text-gray-600 hover:bg-gray-100 rounded-full"
        >
          <ChevronLeft className="h-4 w-4" />
        </button>
      )}
      
      {/* Chips Container */}
      <div
        ref={containerRef}
        className={clsx(
          'flex items-center space-x-2 overflow-x-auto scrollbar-hide',
          sortable && 'cursor-move'
        )}
        style={{ scrollbarWidth: 'none', msOverflowStyle: 'none' }}
      >
        {visibleFilters.map((filter) => (
          <FilterChip
            key={filter.id}
            filter={filter}
            onRemove={() => onRemove(filter.id)}
            onEdit={onEdit ? () => onEdit(filter) : undefined}
          />
        ))}
        
        {/* Hidden Count Badge */}
        {hasMoreFilters && (
          <button
            onClick={handleShowMore}
            className="flex-shrink-0 px-2.5 py-1 bg-gray-100 text-gray-600 text-sm font-medium rounded-full hover:bg-gray-200 transition-colors"
          >
            +{hiddenCount} more
          </button>
        )}
        
        {/* Show Less Button */}
        {visibleCount > maxVisible && (
          <button
            onClick={handleShowLess}
            className="flex-shrink-0 px-2.5 py-1 bg-gray-100 text-gray-600 text-sm font-medium rounded-full hover:bg-gray-200 transition-colors"
          >
            Show less
          </button>
        )}
      </div>
      
      {/* Scroll Right Button */}
      {showScrollButtons && (
        <button
          onClick={() => scroll('right')}
          className="flex-shrink-0 p-1 text-gray-400 hover:text-gray-600 hover:bg-gray-100 rounded-full"
        >
          <ChevronRight className="h-4 w-4" />
        </button>
      )}
      
      {/* Clear All Button */}
      {onClearAll && (
        <button
          onClick={onClearAll}
          className="flex-shrink-0 px-3 py-1 text-sm text-red-600 hover:text-red-700 hover:bg-red-50 rounded-full transition-colors"
        >
          Clear all
        </button>
      )}
    </div>
  );
}

export default FilterChips;

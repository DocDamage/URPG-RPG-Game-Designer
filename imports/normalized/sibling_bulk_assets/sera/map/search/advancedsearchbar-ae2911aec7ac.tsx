/**
 * Advanced Search Bar Component
 * 
 * Full-featured search input with debouncing, filter chips, recent searches,
 * autocomplete suggestions, and voice search integration.
 */

'use client';

import React, { useState, useRef, useEffect, useCallback, useMemo, KeyboardEvent } from 'react';
import { clsx } from 'clsx';
import {
  Search,
  X,
  Mic,
  MicOff,
  Clock,
  Star,
  Sparkles,
  Filter,
  ChevronDown,
  History,
  Trash2,
} from 'lucide-react';
import { Input } from '../ui/input';
import { Button } from '../ui/button';
import { FilterChips } from './FilterChips';
import {
  AdvancedSearchBarProps,
  SearchSuggestion,
  SearchHistoryItem,
  ActiveFilter,
} from './types';

interface SuggestionItemProps {
  suggestion: SearchSuggestion;
  isHighlighted?: boolean;
  onClick: () => void;
  onRemove?: () => void;
}

function SuggestionItem({ suggestion, isHighlighted, onClick, onRemove }: SuggestionItemProps) {
  const getIcon = () => {
    if (suggestion.icon) return suggestion.icon;
    
    switch (suggestion.type) {
      case 'recent':
        return <Clock className="h-4 w-4 text-gray-400" />;
      case 'favorite':
        return <Star className="h-4 w-4 text-yellow-400" />;
      case 'predicted':
        return <Sparkles className="h-4 w-4 text-purple-400" />;
      case 'entity':
        return <Search className="h-4 w-4 text-blue-400" />;
      case 'filter':
        return <Filter className="h-4 w-4 text-green-400" />;
      default:
        return <Search className="h-4 w-4 text-gray-400" />;
    }
  };
  
  return (
    <div
      className={clsx(
        'flex items-center justify-between px-4 py-2 cursor-pointer',
        'hover:bg-gray-100 transition-colors',
        isHighlighted && 'bg-blue-50'
      )}
      onClick={onClick}
    >
      <div className="flex items-center space-x-3">
        {getIcon()}
        <span className="text-sm text-gray-700">{suggestion.displayText}</span>
      </div>
      {onRemove && (
        <button
          onClick={(e) => {
            e.stopPropagation();
            onRemove();
          }}
          className="p-1 rounded hover:bg-gray-200 text-gray-400 hover:text-red-500"
        >
          <Trash2 className="h-3 w-3" />
        </button>
      )}
    </div>
  );
}

export function AdvancedSearchBar({
  value,
  onChange,
  onSearch,
  filters,
  onFilterRemove,
  onClearFilters,
  suggestions = [],
  history = [],
  placeholder = 'Search...',
  disabled = false,
  loading = false,
  voiceEnabled = true,
  autoFocus = false,
  className,
}: AdvancedSearchBarProps) {
  const [inputValue, setInputValue] = useState(value);
  const [isFocused, setIsFocused] = useState(false);
  const [highlightedIndex, setHighlightedIndex] = useState(-1);
  const [showDropdown, setShowDropdown] = useState(false);
  const [isListening, setIsListening] = useState(false);
  const [voiceSupported, setVoiceSupported] = useState(false);
  
  const inputRef = useRef<HTMLInputElement>(null);
  const containerRef = useRef<HTMLDivElement>(null);
  const dropdownRef = useRef<HTMLDivElement>(null);
  
  // Check for voice support
  useEffect(() => {
    if (typeof window !== 'undefined') {
      const SpeechRecognition = window.SpeechRecognition || window.webkitSpeechRecognition;
      setVoiceSupported(!!SpeechRecognition);
    }
  }, []);
  
  // Sync input value with prop
  useEffect(() => {
    setInputValue(value);
  }, [value]);
  
  // Handle click outside to close dropdown
  useEffect(() => {
    function handleClickOutside(event: MouseEvent) {
      if (containerRef.current && !containerRef.current.contains(event.target as Node)) {
        setShowDropdown(false);
        setIsFocused(false);
      }
    }
    
    document.addEventListener('mousedown', handleClickOutside);
    return () => document.removeEventListener('mousedown', handleClickOutside);
  }, []);
  
  // Combine suggestions and history for dropdown
  const dropdownItems: (SearchSuggestion | SearchHistoryItem)[] = useMemo(() => {
    if (inputValue.trim()) {
      return suggestions;
    }
    
    // Show history when input is empty
    return history.map(h => ({
      id: h.id,
      type: h.favorite ? 'favorite' : 'recent',
      text: h.query,
      displayText: h.query,
      metadata: { filters: h.filters },
    } as SearchSuggestion));
  }, [inputValue, suggestions, history]);
  
  // Handle input change
  const handleInputChange = useCallback((e: React.ChangeEvent<HTMLInputElement>) => {
    const newValue = e.target.value;
    setInputValue(newValue);
    onChange(newValue);
    setShowDropdown(true);
    setHighlightedIndex(-1);
  }, [onChange]);
  
  // Handle search submission
  const handleSubmit = useCallback(() => {
    if (inputValue.trim()) {
      onSearch(inputValue.trim());
      setShowDropdown(false);
      setHighlightedIndex(-1);
    }
  }, [inputValue, onSearch]);
  
  // Handle suggestion selection
  const handleSelectSuggestion = useCallback((item: SearchSuggestion | SearchHistoryItem) => {
    if ('query' in item) {
      // It's a history item
      setInputValue(item.query);
      onChange(item.query);
      onSearch(item.query);
    } else {
      // It's a suggestion
      if (item.action) {
        item.action();
      } else {
        setInputValue(item.text);
        onChange(item.text);
        onSearch(item.text);
      }
    }
    setShowDropdown(false);
    setHighlightedIndex(-1);
  }, [onChange, onSearch]);
  
  // Handle keyboard navigation
  const handleKeyDown = useCallback((e: KeyboardEvent<HTMLInputElement>) => {
    switch (e.key) {
      case 'ArrowDown':
        e.preventDefault();
        setShowDropdown(true);
        setHighlightedIndex(prev => 
          prev < dropdownItems.length - 1 ? prev + 1 : prev
        );
        break;
        
      case 'ArrowUp':
        e.preventDefault();
        setHighlightedIndex(prev => (prev > 0 ? prev - 1 : -1));
        break;
        
      case 'Enter':
        e.preventDefault();
        if (highlightedIndex >= 0 && dropdownItems[highlightedIndex]) {
          handleSelectSuggestion(dropdownItems[highlightedIndex]);
        } else {
          handleSubmit();
        }
        break;
        
      case 'Escape':
        setShowDropdown(false);
        setHighlightedIndex(-1);
        inputRef.current?.blur();
        break;
        
      case 'Tab':
        setShowDropdown(false);
        break;
    }
  }, [dropdownItems, highlightedIndex, handleSelectSuggestion, handleSubmit]);
  
  // Handle voice search
  const toggleVoiceSearch = useCallback(() => {
    if (!voiceSupported) return;
    
    const SpeechRecognition = window.SpeechRecognition || window.webkitSpeechRecognition;
    if (!SpeechRecognition) return;
    
    if (isListening) {
      setIsListening(false);
      return;
    }
    
    const recognition = new SpeechRecognition();
    recognition.continuous = false;
    recognition.interimResults = true;
    recognition.lang = 'en-US';
    
    recognition.onstart = () => {
      setIsListening(true);
    };
    
    recognition.onend = () => {
      setIsListening(false);
    };
    
    recognition.onresult = (event: any) => {
      const transcript = event.results[0][0].transcript;
      setInputValue(transcript);
      onChange(transcript);
      
      if (event.results[0].isFinal) {
        onSearch(transcript);
      }
    };
    
    recognition.onerror = () => {
      setIsListening(false);
    };
    
    recognition.start();
  }, [isListening, voiceSupported, onChange, onSearch]);
  
  // Clear search
  const handleClear = useCallback(() => {
    setInputValue('');
    onChange('');
    setShowDropdown(false);
    inputRef.current?.focus();
  }, [onChange]);
  
  // Scroll highlighted item into view
  useEffect(() => {
    if (highlightedIndex >= 0 && dropdownRef.current) {
      const items = dropdownRef.current.querySelectorAll('[data-suggestion-index]');
      const highlightedItem = items[highlightedIndex] as HTMLElement;
      if (highlightedItem) {
        highlightedItem.scrollIntoView({ block: 'nearest' });
      }
    }
  }, [highlightedIndex]);
  
  const hasFilters = filters.length > 0;
  const showClearButton = inputValue || hasFilters;
  
  return (
    <div ref={containerRef} className={clsx('relative w-full', className)}>
      {/* Search Input Container */}
      <div
        className={clsx(
          'relative flex items-center bg-white border rounded-lg transition-all duration-200',
          isFocused ? 'border-blue-500 ring-2 ring-blue-100' : 'border-gray-300',
          disabled && 'bg-gray-100 cursor-not-allowed'
        )}
      >
        {/* Search Icon */}
        <div className="flex-shrink-0 pl-3">
          {loading ? (
            <div className="h-5 w-5 border-2 border-blue-500 border-t-transparent rounded-full animate-spin" />
          ) : (
            <Search className="h-5 w-5 text-gray-400" />
          )}
        </div>
        
        {/* Input */}
        <input
          ref={inputRef}
          type="text"
          value={inputValue}
          onChange={handleInputChange}
          onFocus={() => {
            setIsFocused(true);
            setShowDropdown(true);
          }}
          onKeyDown={handleKeyDown}
          placeholder={placeholder}
          disabled={disabled}
          autoFocus={autoFocus}
          className={clsx(
            'flex-1 px-3 py-2.5 bg-transparent border-none outline-none text-sm',
            'placeholder:text-gray-400',
            disabled && 'cursor-not-allowed'
          )}
        />
        
        {/* Filter Count Badge */}
        {hasFilters && (
          <button
            onClick={() => {
              setIsFocused(true);
              setShowDropdown(false);
            }}
            className="flex-shrink-0 mr-2 px-2 py-0.5 bg-blue-100 text-blue-700 text-xs font-medium rounded-full hover:bg-blue-200"
          >
            {filters.length} filter{filters.length !== 1 ? 's' : ''}
          </button>
        )}
        
        {/* Voice Search Button */}
        {voiceEnabled && voiceSupported && (
          <button
            onClick={toggleVoiceSearch}
            disabled={disabled}
            className={clsx(
              'flex-shrink-0 p-2 rounded-md transition-colors',
              isListening 
                ? 'text-red-500 bg-red-50 animate-pulse' 
                : 'text-gray-400 hover:text-gray-600 hover:bg-gray-100',
              disabled && 'opacity-50 cursor-not-allowed'
            )}
            title={isListening ? 'Stop listening' : 'Voice search'}
          >
            {isListening ? <Mic className="h-4 w-4" /> : <MicOff className="h-4 w-4" />}
          </button>
        )}
        
        {/* Clear Button */}
        {showClearButton && (
          <button
            onClick={handleClear}
            disabled={disabled}
            className="flex-shrink-0 p-2 text-gray-400 hover:text-gray-600 hover:bg-gray-100 rounded-md transition-colors"
            title="Clear search"
          >
            <X className="h-4 w-4" />
          </button>
        )}
        
        {/* Dropdown Toggle */}
        <button
          onClick={() => setShowDropdown(!showDropdown)}
          disabled={disabled}
          className={clsx(
            'flex-shrink-0 p-2 text-gray-400 hover:text-gray-600 rounded-md transition-colors',
            showDropdown && 'bg-gray-100 text-gray-600'
          )}
        >
          <ChevronDown className={clsx('h-4 w-4 transition-transform', showDropdown && 'rotate-180')} />
        </button>
      </div>
      
      {/* Filter Chips */}
      {hasFilters && (
        <div className="mt-2">
          <FilterChips
            filters={filters}
            onRemove={onFilterRemove}
            onClearAll={onClearFilters}
            maxVisible={5}
          />
        </div>
      )}
      
      {/* Suggestions/History Dropdown */}
      {showDropdown && (dropdownItems.length > 0 || history.length > 0) && (
        <div
          ref={dropdownRef}
          className="absolute z-50 top-full left-0 right-0 mt-1 bg-white border border-gray-200 rounded-lg shadow-lg max-h-80 overflow-auto"
        >
          {/* Section Header */}
          {inputValue.trim() ? (
            <div className="px-4 py-2 text-xs font-semibold text-gray-500 uppercase tracking-wider">
              Suggestions
            </div>
          ) : history.length > 0 ? (
            <div className="flex items-center justify-between px-4 py-2">
              <span className="text-xs font-semibold text-gray-500 uppercase tracking-wider">
                Recent Searches
              </span>
              <button
                onClick={() => {
                  // Clear history callback would go here
                }}
                className="text-xs text-gray-400 hover:text-red-500"
              >
                Clear all
              </button>
            </div>
          ) : null}
          
          {/* Items */}
          {dropdownItems.length > 0 ? (
            dropdownItems.map((item, index) => (
              <div key={item.id} data-suggestion-index={index}>
                <SuggestionItem
                  suggestion={'type' in item ? item : {
                    id: item.id,
                    type: item.favorite ? 'favorite' : 'recent',
                    text: item.query,
                    displayText: item.query,
                  }}
                  isHighlighted={index === highlightedIndex}
                  onClick={() => handleSelectSuggestion(item)}
                  onRemove={'timestamp' in item ? () => {
                    // Remove from history
                  } : undefined}
                />
              </div>
            ))
          ) : (
            <div className="px-4 py-8 text-center text-gray-400">
              <History className="h-8 w-8 mx-auto mb-2 opacity-50" />
              <p className="text-sm">No recent searches</p>
            </div>
          )}
        </div>
      )}
    </div>
  );
}

export default AdvancedSearchBar;

// Extend Window interface for Web Speech API
declare global {
  interface Window {
    SpeechRecognition?: any;
    webkitSpeechRecognition?: any;
  }
}

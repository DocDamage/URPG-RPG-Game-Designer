/**
 * Filter Panel Component
 * 
 * Collapsible filter sidebar/drawer with multiple filter types,
 * filter groups, quick presets, and save/load functionality.
 */

'use client';

import React, { useState, useCallback, useMemo } from 'react';
import { clsx } from 'clsx';
import {
  SlidersHorizontal,
  ChevronDown,
  ChevronRight,
  Save,
  Trash2,
  Star,
  Plus,
  X,
  Search,
  RotateCcw,
  Check,
} from 'lucide-react';
import { Button } from '../ui/button';
import { Input } from '../ui/input';
import { Switch } from '../ui/switch';
import {
  FilterPanelProps,
  FilterGroup,
  FilterConfig,
  FilterPreset,
  FilterType,
  FilterOperator,
  FilterOption,
} from './types';

interface FilterInputProps {
  filter: FilterConfig;
  value: any;
  onChange: (value: any, operator?: FilterOperator) => void;
}

function FilterInput({ filter, value, onChange }: FilterInputProps) {
  const { type, config = {}, options = [], placeholder } = filter;
  
  switch (type) {
    case 'text':
      return (
        <div className="space-y-2">
          <select
            value={filter.operator}
            onChange={(e) => onChange(value, e.target.value as FilterOperator)}
            className="w-full px-3 py-1.5 text-sm border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
          >
            <option value="contains">Contains</option>
            <option value="starts_with">Starts with</option>
            <option value="ends_with">Ends with</option>
            <option value="equals">Equals</option>
          </select>
          <input
            type="text"
            value={value || ''}
            onChange={(e) => onChange(e.target.value)}
            placeholder={placeholder || 'Enter text...'}
            className="w-full px-3 py-2 text-sm border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
          />
        </div>
      );
      
    case 'number':
      return (
        <div className="space-y-2">
          <select
            value={filter.operator}
            onChange={(e) => onChange(value, e.target.value as FilterOperator)}
            className="w-full px-3 py-1.5 text-sm border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
          >
            <option value="equals">Equals</option>
            <option value="greater_than">Greater than</option>
            <option value="less_than">Less than</option>
            <option value="greater_or_equal">Greater or equal</option>
            <option value="less_or_equal">Less or equal</option>
            <option value="between">Between</option>
          </select>
          {filter.operator === 'between' ? (
            <div className="flex items-center space-x-2">
              <input
                type="number"
                value={Array.isArray(value) ? value[0] || '' : ''}
                onChange={(e) => onChange([Number(e.target.value), Array.isArray(value) ? value[1] : null])}
                placeholder="Min"
                min={config.min}
                max={config.max}
                step={config.step}
                className="flex-1 px-3 py-2 text-sm border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
              />
              <span className="text-gray-400">to</span>
              <input
                type="number"
                value={Array.isArray(value) ? value[1] || '' : ''}
                onChange={(e) => onChange([Array.isArray(value) ? value[0] : null, Number(e.target.value)])}
                placeholder="Max"
                min={config.min}
                max={config.max}
                step={config.step}
                className="flex-1 px-3 py-2 text-sm border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
              />
            </div>
          ) : (
            <div className="flex items-center space-x-2">
              <input
                type="number"
                value={value || ''}
                onChange={(e) => onChange(Number(e.target.value))}
                placeholder={placeholder || 'Enter number...'}
                min={config.min}
                max={config.max}
                step={config.step}
                className="flex-1 px-3 py-2 text-sm border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
              />
              {config.unit && (
                <span className="text-sm text-gray-500">{config.unit}</span>
              )}
            </div>
          )}
        </div>
      );
      
    case 'date':
      return (
        <div className="space-y-2">
          <select
            value={filter.operator}
            onChange={(e) => onChange(value, e.target.value as FilterOperator)}
            className="w-full px-3 py-1.5 text-sm border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
          >
            <option value="date_between">Between</option>
            <option value="date_after">After</option>
            <option value="date_before">Before</option>
            <option value="equals">On</option>
          </select>
          {filter.operator === 'date_between' ? (
            <div className="flex items-center space-x-2">
              <input
                type="date"
                value={Array.isArray(value) ? value[0] || '' : ''}
                onChange={(e) => onChange([e.target.value, Array.isArray(value) ? value[1] : ''])}
                className="flex-1 px-3 py-2 text-sm border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
              />
              <span className="text-gray-400">to</span>
              <input
                type="date"
                value={Array.isArray(value) ? value[1] || '' : ''}
                onChange={(e) => onChange([Array.isArray(value) ? value[0] : '', e.target.value])}
                className="flex-1 px-3 py-2 text-sm border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
              />
            </div>
          ) : (
            <input
              type={config.enableTime ? 'datetime-local' : 'date'}
              value={value || ''}
              onChange={(e) => onChange(e.target.value)}
              className="w-full px-3 py-2 text-sm border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
            />
          )}
        </div>
      );
      
    case 'select':
      return (
        <select
          value={value || ''}
          onChange={(e) => onChange(e.target.value || null)}
          className="w-full px-3 py-2 text-sm border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
        >
          <option value="">{placeholder || 'Select...'}</option>
          {options.map((option) => (
            <option key={option.value} value={option.value} disabled={option.disabled}>
              {option.label}
            </option>
          ))}
        </select>
      );
      
    case 'multi_select':
      return (
        <div className="space-y-2 max-h-48 overflow-y-auto">
          {options.map((option) => {
            const isSelected = Array.isArray(value) && value.includes(option.value);
            return (
              <label
                key={option.value}
                className={clsx(
                  'flex items-center space-x-2 p-2 rounded cursor-pointer transition-colors',
                  isSelected ? 'bg-blue-50' : 'hover:bg-gray-50',
                  option.disabled && 'opacity-50 cursor-not-allowed'
                )}
              >
                <input
                  type="checkbox"
                  checked={isSelected}
                  onChange={(e) => {
                    const currentValues = Array.isArray(value) ? value : [];
                    if (e.target.checked) {
                      if (!config.multiSelectLimit || currentValues.length < config.multiSelectLimit) {
                        onChange([...currentValues, option.value]);
                      }
                    } else {
                      onChange(currentValues.filter((v) => v !== option.value));
                    }
                  }}
                  disabled={option.disabled}
                  className="rounded border-gray-300 text-blue-600 focus:ring-blue-500"
                />
                <span className="text-sm text-gray-700">{option.label}</span>
              </label>
            );
          })}
          {config.multiSelectLimit && Array.isArray(value) && value.length >= config.multiSelectLimit && (
            <p className="text-xs text-amber-600 px-2">
              Maximum {config.multiSelectLimit} selections allowed
            </p>
          )}
        </div>
      );
      
    case 'boolean':
      return (
        <div className="flex items-center justify-between">
          <span className="text-sm text-gray-700">
            {value === true ? 'Yes' : value === false ? 'No' : 'Any'}
          </span>
          <Switch
            checked={value === true}
            onCheckedChange={(checked) => onChange(checked)}
          />
        </div>
      );
      
    case 'range':
      const [min, max] = Array.isArray(value) ? value : [config.min, config.max];
      return (
        <div className="space-y-4">
          <div className="flex items-center justify-between text-sm text-gray-600">
            <span>{min ?? config.min}</span>
            <span>{max ?? config.max}</span>
          </div>
          <input
            type="range"
            min={config.min}
            max={config.max}
            step={config.step}
            value={min ?? config.min}
            onChange={(e) => onChange([Number(e.target.value), max])}
            className="w-full"
          />
          <input
            type="range"
            min={config.min}
            max={config.max}
            step={config.step}
            value={max ?? config.max}
            onChange={(e) => onChange([min, Number(e.target.value)])}
            className="w-full"
          />
          {config.unit && (
            <p className="text-xs text-gray-500 text-center">
              {min ?? config.min} - {max ?? config.max} {config.unit}
            </p>
          )}
        </div>
      );
      
    default:
      return null;
  }
}

interface FilterGroupSectionProps {
  group: FilterGroup;
  activeFilters: Map<string, any>;
  onFilterChange: (filter: FilterConfig, value: any, operator?: FilterOperator) => void;
  searchTerm: string;
}

function FilterGroupSection({ group, activeFilters, onFilterChange, searchTerm }: FilterGroupSectionProps) {
  const [isExpanded, setIsExpanded] = useState(group.expanded !== false);
  
  const filteredFilters = useMemo(() => {
    if (!searchTerm) return group.filters;
    return group.filters.filter(f => 
      f.label.toLowerCase().includes(searchTerm.toLowerCase()) ||
      f.field.toLowerCase().includes(searchTerm.toLowerCase())
    );
  }, [group.filters, searchTerm]);
  
  const activeCount = useMemo(() => {
    return group.filters.filter(f => activeFilters.has(f.field)).length;
  }, [group.filters, activeFilters]);
  
  if (filteredFilters.length === 0) return null;
  
  return (
    <div className="border-b border-gray-200 last:border-b-0">
      <button
        onClick={() => setIsExpanded(!isExpanded)}
        className="w-full flex items-center justify-between px-4 py-3 hover:bg-gray-50 transition-colors"
      >
        <div className="flex items-center space-x-2">
          {group.icon}
          <span className="font-medium text-gray-700">{group.label}</span>
          {activeCount > 0 && (
            <span className="px-2 py-0.5 bg-blue-100 text-blue-700 text-xs rounded-full">
              {activeCount}
            </span>
          )}
        </div>
        <ChevronDown className={clsx(
          'h-4 w-4 text-gray-400 transition-transform',
          !isExpanded && '-rotate-90'
        )} />
      </button>
      
      {isExpanded && (
        <div className="px-4 pb-4 space-y-4">
          {group.description && (
            <p className="text-xs text-gray-500">{group.description}</p>
          )}
          {filteredFilters.map((filter) => {
            const activeValue = activeFilters.get(filter.field);
            const isActive = activeValue !== undefined;
            
            return (
              <div key={filter.id} className={clsx(
                'p-3 rounded-lg transition-colors',
                isActive ? 'bg-blue-50 border border-blue-200' : 'bg-gray-50'
              )}>
                <div className="flex items-center justify-between mb-2">
                  <label className="text-sm font-medium text-gray-700">
                    {filter.label}
                    {filter.required && <span className="text-red-500 ml-1">*</span>}
                  </label>
                  {isActive && (
                    <button
                      onClick={() => onFilterChange(filter, null)}
                      className="text-xs text-red-500 hover:text-red-700"
                    >
                      Clear
                    </button>
                  )}
                </div>
                {filter.description && (
                  <p className="text-xs text-gray-500 mb-2">{filter.description}</p>
                )}
                <FilterInput
                  filter={filter}
                  value={activeValue}
                  onChange={(value, operator) => onFilterChange(filter, value, operator)}
                />
              </div>
            );
          })}
        </div>
      )}
    </div>
  );
}

export function FilterPanel({
  groups,
  activeFilters,
  presets = [],
  onFilterChange,
  onFilterRemove,
  onClearFilters,
  onSavePreset,
  onLoadPreset,
  onDeletePreset,
  expanded = true,
  onExpandedChange,
  className,
}: FilterPanelProps) {
  const [isExpanded, setIsExpanded] = useState(expanded);
  const [searchTerm, setSearchTerm] = useState('');
  const [showSavePreset, setShowSavePreset] = useState(false);
  const [presetName, setPresetName] = useState('');
  const [presetDescription, setPresetDescription] = useState('');
  
  // Convert active filters array to map for easier lookup
  const activeFiltersMap = useMemo(() => {
    const map = new Map<string, any>();
    activeFilters.forEach(f => map.set(f.field, f.value));
    return map;
  }, [activeFilters]);
  
  // Handle filter value change
  const handleFilterChange = useCallback((filter: FilterConfig, value: any, operator?: FilterOperator) => {
    if (value === null || value === undefined || value === '' || 
        (Array.isArray(value) && value.length === 0)) {
      onFilterRemove(filter.id);
    } else {
      onFilterChange({
        ...filter,
        value,
        operator: operator || filter.operator,
      });
    }
  }, [onFilterChange, onFilterRemove]);
  
  // Handle save preset
  const handleSavePreset = useCallback(() => {
    if (!presetName.trim()) return;
    
    onSavePreset({
      name: presetName.trim(),
      description: presetDescription.trim() || undefined,
      filters: [...activeFilters],
    });
    
    setPresetName('');
    setPresetDescription('');
    setShowSavePreset(false);
  }, [presetName, presetDescription, activeFilters, onSavePreset]);
  
  // Toggle expanded state
  const toggleExpanded = useCallback(() => {
    const newState = !isExpanded;
    setIsExpanded(newState);
    onExpandedChange?.(newState);
  }, [isExpanded, onExpandedChange]);
  
  const hasActiveFilters = activeFilters.length > 0;
  
  return (
    <div className={clsx(
      'bg-white border border-gray-200 rounded-lg shadow-sm overflow-hidden',
      className
    )}>
      {/* Header */}
      <div className="flex items-center justify-between px-4 py-3 border-b border-gray-200">
        <button
          onClick={toggleExpanded}
          className="flex items-center space-x-2 font-medium text-gray-700 hover:text-gray-900"
        >
          <SlidersHorizontal className="h-5 w-5" />
          <span>Filters</span>
          {hasActiveFilters && (
            <span className="px-2 py-0.5 bg-blue-100 text-blue-700 text-xs rounded-full">
              {activeFilters.length}
            </span>
          )}
        </button>
        <div className="flex items-center space-x-1">
          {hasActiveFilters && (
            <button
              onClick={onClearFilters}
              className="px-3 py-1.5 text-sm text-red-600 hover:text-red-700 hover:bg-red-50 rounded-md transition-colors"
            >
              Clear all
            </button>
          )}
          <button
            onClick={toggleExpanded}
            className="p-1.5 text-gray-400 hover:text-gray-600 hover:bg-gray-100 rounded-md transition-colors"
          >
            <ChevronDown className={clsx(
              'h-5 w-5 transition-transform',
              !isExpanded && '-rotate-90'
            )} />
          </button>
        </div>
      </div>
      
      {isExpanded && (
        <>
          {/* Presets Section */}
          {presets.length > 0 && (
            <div className="px-4 py-3 border-b border-gray-200 bg-gray-50">
              <div className="flex items-center justify-between mb-2">
                <span className="text-xs font-semibold text-gray-500 uppercase tracking-wider">
                  Saved Presets
                </span>
                <button
                  onClick={() => setShowSavePreset(true)}
                  className="text-xs text-blue-600 hover:text-blue-700 flex items-center space-x-1"
                >
                  <Save className="h-3 w-3" />
                  <span>Save current</span>
                </button>
              </div>
              <div className="flex flex-wrap gap-2">
                {presets.map((preset) => (
                  <button
                    key={preset.id}
                    onClick={() => onLoadPreset(preset.id)}
                    className={clsx(
                      'inline-flex items-center space-x-1 px-3 py-1.5 text-sm rounded-full border transition-colors',
                      preset.isDefault
                        ? 'bg-blue-100 border-blue-300 text-blue-700'
                        : 'bg-white border-gray-300 text-gray-700 hover:bg-gray-50'
                    )}
                    title={preset.description}
                  >
                    {preset.isDefault && <Star className="h-3 w-3" />}
                    <span>{preset.name}</span>
                    {!preset.isSystem && onDeletePreset && (
                      <span
                        onClick={(e) => {
                          e.stopPropagation();
                          onDeletePreset(preset.id);
                        }}
                        className="ml-1 text-gray-400 hover:text-red-500"
                      >
                        <X className="h-3 w-3" />
                      </span>
                    )}
                  </button>
                ))}
              </div>
            </div>
          )}
          
          {/* Save Preset Modal */}
          {showSavePreset && (
            <div className="px-4 py-3 border-b border-gray-200 bg-blue-50">
              <div className="space-y-3">
                <input
                  type="text"
                  value={presetName}
                  onChange={(e) => setPresetName(e.target.value)}
                  placeholder="Preset name..."
                  className="w-full px-3 py-2 text-sm border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
                  autoFocus
                />
                <input
                  type="text"
                  value={presetDescription}
                  onChange={(e) => setPresetDescription(e.target.value)}
                  placeholder="Description (optional)..."
                  className="w-full px-3 py-2 text-sm border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
                />
                <div className="flex items-center justify-end space-x-2">
                  <button
                    onClick={() => setShowSavePreset(false)}
                    className="px-3 py-1.5 text-sm text-gray-600 hover:text-gray-800"
                  >
                    Cancel
                  </button>
                  <button
                    onClick={handleSavePreset}
                    disabled={!presetName.trim()}
                    className="px-3 py-1.5 text-sm bg-blue-600 text-white rounded-md hover:bg-blue-700 disabled:opacity-50 disabled:cursor-not-allowed"
                  >
                    Save
                  </button>
                </div>
              </div>
            </div>
          )}
          
          {/* Filter Search */}
          <div className="px-4 py-3 border-b border-gray-200">
            <div className="relative">
              <Search className="absolute left-3 top-1/2 -translate-y-1/2 h-4 w-4 text-gray-400" />
              <input
                type="text"
                value={searchTerm}
                onChange={(e) => setSearchTerm(e.target.value)}
                placeholder="Search filters..."
                className="w-full pl-9 pr-9 py-2 text-sm border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
              />
              {searchTerm && (
                <button
                  onClick={() => setSearchTerm('')}
                  className="absolute right-3 top-1/2 -translate-y-1/2 text-gray-400 hover:text-gray-600"
                >
                  <X className="h-4 w-4" />
                </button>
              )}
            </div>
          </div>
          
          {/* Filter Groups */}
          <div className="max-h-[60vh] overflow-y-auto">
            {groups.map((group) => (
              <FilterGroupSection
                key={group.id}
                group={group}
                activeFilters={activeFiltersMap}
                onFilterChange={handleFilterChange}
                searchTerm={searchTerm}
              />
            ))}
          </div>
          
          {/* Footer Actions */}
          <div className="px-4 py-3 border-t border-gray-200 bg-gray-50 flex items-center justify-between">
            <span className="text-sm text-gray-500">
              {activeFilters.length} filter{activeFilters.length !== 1 ? 's' : ''} active
            </span>
            <div className="flex items-center space-x-2">
              {hasActiveFilters && (
                <button
                  onClick={onClearFilters}
                  className="flex items-center space-x-1 px-3 py-1.5 text-sm text-gray-600 hover:text-gray-800 hover:bg-gray-200 rounded-md transition-colors"
                >
                  <RotateCcw className="h-4 w-4" />
                  <span>Reset</span>
                </button>
              )}
            </div>
          </div>
        </>
      )}
    </div>
  );
}

export default FilterPanel;

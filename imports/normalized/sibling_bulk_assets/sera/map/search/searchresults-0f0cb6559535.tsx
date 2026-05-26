/**
 * Search Results Component
 * 
 * Virtualized, sortable results list with pagination, bulk selection,
 * and export capabilities.
 */

'use client';

import React, { useState, useCallback, useMemo, useRef, useEffect } from 'react';
import { clsx } from 'clsx';
import {
  ChevronDown,
  ChevronUp,
  ChevronLeft,
  ChevronRight,
  Download,
  FileSpreadsheet,
  FileJson,
  FileText,
  AlertCircle,
  Loader2,
  CheckSquare,
  Square,
  Minus,
} from 'lucide-react';
import { Button } from '../ui/button';
import { BulkActionsBar } from '../bulk/BulkActionsBar';
import {
  SearchResultsProps,
  ColumnConfig,
  SearchResult,
  SelectionState,
  ExportConfig,
  SortConfig,
} from './types';

interface TableHeaderProps<T> {
  columns: ColumnConfig<T>[];
  sort?: SortConfig;
  onSort?: (sort: SortConfig) => void;
  selection?: SelectionState;
  onSelectAll?: () => void;
  allSelectable?: boolean;
}

function TableHeader<T>({
  columns,
  sort,
  onSort,
  selection,
  onSelectAll,
  allSelectable,
}: TableHeaderProps<T>) {
  const visibleColumns = columns.filter(c => c.visible !== false);
  
  const handleSort = useCallback((column: ColumnConfig<T>) => {
    if (!column.sortable || !onSort) return;
    
    const newSort: SortConfig = {
      field: String(column.field),
      direction: sort?.field === column.field && sort.direction === 'asc' ? 'desc' : 'asc',
    };
    onSort(newSort);
  }, [sort, onSort]);
  
  return (
    <thead className="bg-gray-50 sticky top-0 z-10">
      <tr>
        {/* Selection Header */}
        {selection && (
          <th className="w-10 px-4 py-3 border-b border-gray-200">
            <button
              onClick={onSelectAll}
              className="flex items-center justify-center"
            >
              {selection.isAllSelected ? (
                <CheckSquare className="h-5 w-5 text-blue-600" />
              ) : selection.isIndeterminate ? (
                <div className="relative">
                  <Square className="h-5 w-5 text-blue-600" />
                  <div className="absolute inset-0 flex items-center justify-center">
                    <Minus className="h-3 w-3 text-blue-600" />
                  </div>
                </div>
              ) : (
                <Square className="h-5 w-5 text-gray-400" />
              )}
            </button>
          </th>
        )}
        
        {/* Column Headers */}
        {visibleColumns.map((column) => (
          <th
            key={column.id}
            className={clsx(
              'px-4 py-3 border-b border-gray-200 text-left text-xs font-semibold text-gray-600 uppercase tracking-wider',
              column.sortable && 'cursor-pointer hover:bg-gray-100 select-none',
              column.align === 'center' && 'text-center',
              column.align === 'right' && 'text-right',
              column.pinned === 'left' && 'sticky left-0 bg-gray-50 z-20',
              column.pinned === 'right' && 'sticky right-0 bg-gray-50 z-20',
            )}
            style={{
              width: column.width,
              minWidth: column.minWidth,
              maxWidth: column.maxWidth,
            }}
            onClick={() => handleSort(column)}
          >
            <div className={clsx(
              'flex items-center space-x-1',
              column.align === 'center' && 'justify-center',
              column.align === 'right' && 'justify-end'
            )}>
              <span>{column.header}</span>
              {column.sortable && sort?.field === column.field && (
                sort.direction === 'asc' ? (
                  <ChevronUp className="h-4 w-4 text-blue-600" />
                ) : (
                  <ChevronDown className="h-4 w-4 text-blue-600" />
                )
              )}
            </div>
          </th>
        ))}
      </tr>
    </thead>
  );
}

interface TableRowProps<T> {
  item: T;
  columns: ColumnConfig<T>[];
  index: number;
  selection?: SelectionState;
  onSelect?: (item: T, selected: boolean) => void;
  onClick?: (item: T) => void;
  isSelected?: boolean;
}

function TableRow<T>({
  item,
  columns,
  index,
  selection,
  onSelect,
  onClick,
  isSelected,
}: TableRowProps<T>) {
  const visibleColumns = columns.filter(c => c.visible !== false);
  const rowId = (item as any).id || index;
  
  const handleSelect = useCallback(() => {
    onSelect?.(item, !isSelected);
  }, [item, isSelected, onSelect]);
  
  return (
    <tr
      className={clsx(
        'hover:bg-gray-50 transition-colors',
        onClick && 'cursor-pointer',
        isSelected && 'bg-blue-50'
      )}
      onClick={() => onClick?.(item)}
    >
      {/* Selection Cell */}
      {selection && (
        <td 
          className="w-10 px-4 py-3 border-b border-gray-200"
          onClick={(e) => e.stopPropagation()}
        >
          <button onClick={handleSelect}>
            {isSelected ? (
              <CheckSquare className="h-5 w-5 text-blue-600" />
            ) : (
              <Square className="h-5 w-5 text-gray-400" />
            )}
          </button>
        </td>
      )}
      
      {/* Data Cells */}
      {visibleColumns.map((column) => {
        const value = (item as any)[column.field];
        const formattedValue = column.formatter 
          ? column.formatter(value, item)
          : value;
        
        return (
          <td
            key={column.id}
            className={clsx(
              'px-4 py-3 border-b border-gray-200 text-sm',
              column.align === 'center' && 'text-center',
              column.align === 'right' && 'text-right',
              column.pinned === 'left' && 'sticky left-0 bg-white z-10',
              column.pinned === 'right' && 'sticky right-0 bg-white z-10',
              typeof column.cellClassName === 'function' 
                ? column.cellClassName(item)
                : column.cellClassName
            )}
            style={{
              width: column.width,
              minWidth: column.minWidth,
              maxWidth: column.maxWidth,
            }}
          >
            {formattedValue ?? '-'}
          </td>
        );
      })}
    </tr>
  );
}

interface PaginationProps {
  currentPage: number;
  pageSize: number;
  totalCount: number;
  pageCount: number;
  onPageChange: (page: number) => void;
  onPageSizeChange: (pageSize: number) => void;
  pageSizeOptions?: number[];
}

function Pagination({
  currentPage,
  pageSize,
  totalCount,
  pageCount,
  onPageChange,
  onPageSizeChange,
  pageSizeOptions = [10, 25, 50, 100],
}: PaginationProps) {
  const startItem = (currentPage - 1) * pageSize + 1;
  const endItem = Math.min(currentPage * pageSize, totalCount);
  
  const pages = useMemo(() => {
    const result: (number | string)[] = [];
    const maxVisible = 5;
    
    if (pageCount <= maxVisible) {
      for (let i = 1; i <= pageCount; i++) {
        result.push(i);
      }
    } else {
      if (currentPage <= 3) {
        for (let i = 1; i <= 4; i++) result.push(i);
        result.push('...');
        result.push(pageCount);
      } else if (currentPage >= pageCount - 2) {
        result.push(1);
        result.push('...');
        for (let i = pageCount - 3; i <= pageCount; i++) result.push(i);
      } else {
        result.push(1);
        result.push('...');
        for (let i = currentPage - 1; i <= currentPage + 1; i++) result.push(i);
        result.push('...');
        result.push(pageCount);
      }
    }
    
    return result;
  }, [currentPage, pageCount]);
  
  return (
    <div className="flex items-center justify-between px-4 py-3 border-t border-gray-200 bg-gray-50">
      {/* Info */}
      <div className="text-sm text-gray-600">
        Showing <span className="font-medium">{startItem}</span> to{' '}
        <span className="font-medium">{endItem}</span> of{' '}
        <span className="font-medium">{totalCount}</span> results
      </div>
      
      {/* Page Size Selector */}
      <div className="flex items-center space-x-2">
        <span className="text-sm text-gray-600">Show:</span>
        <select
          value={pageSize}
          onChange={(e) => onPageSizeChange(Number(e.target.value))}
          className="px-2 py-1 text-sm border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
        >
          {pageSizeOptions.map((size) => (
            <option key={size} value={size}>
              {size}
            </option>
          ))}
        </select>
      </div>
      
      {/* Page Navigation */}
      <div className="flex items-center space-x-1">
        <button
          onClick={() => onPageChange(currentPage - 1)}
          disabled={currentPage === 1}
          className="p-2 text-gray-400 hover:text-gray-600 hover:bg-gray-200 rounded-md disabled:opacity-50 disabled:cursor-not-allowed"
        >
          <ChevronLeft className="h-4 w-4" />
        </button>
        
        {pages.map((page, index) => (
          page === '...' ? (
            <span key={`ellipsis-${index}`} className="px-3 py-2 text-gray-400">
              ...
            </span>
          ) : (
            <button
              key={page}
              onClick={() => onPageChange(page as number)}
              className={clsx(
                'px-3 py-1.5 text-sm font-medium rounded-md transition-colors',
                currentPage === page
                  ? 'bg-blue-600 text-white'
                  : 'text-gray-600 hover:bg-gray-200'
              )}
            >
              {page}
            </button>
          )
        ))}
        
        <button
          onClick={() => onPageChange(currentPage + 1)}
          disabled={currentPage === pageCount}
          className="p-2 text-gray-400 hover:text-gray-600 hover:bg-gray-200 rounded-md disabled:opacity-50 disabled:cursor-not-allowed"
        >
          <ChevronRight className="h-4 w-4" />
        </button>
      </div>
    </div>
  );
}

interface ExportMenuProps {
  onExport: (config: ExportConfig) => void;
  disabled?: boolean;
}

function ExportMenu({ onExport, disabled }: ExportMenuProps) {
  const [isOpen, setIsOpen] = useState(false);
  const menuRef = useRef<HTMLDivElement>(null);
  
  useEffect(() => {
    function handleClickOutside(event: MouseEvent) {
      if (menuRef.current && !menuRef.current.contains(event.target as Node)) {
        setIsOpen(false);
      }
    }
    
    document.addEventListener('mousedown', handleClickOutside);
    return () => document.removeEventListener('mousedown', handleClickOutside);
  }, []);
  
  const handleExport = (format: ExportConfig['format']) => {
    onExport({ format });
    setIsOpen(false);
  };
  
  return (
    <div className="relative" ref={menuRef}>
      <button
        onClick={() => setIsOpen(!isOpen)}
        disabled={disabled}
        className="flex items-center space-x-2 px-3 py-2 text-sm text-gray-600 hover:text-gray-800 hover:bg-gray-100 rounded-md transition-colors disabled:opacity-50"
      >
        <Download className="h-4 w-4" />
        <span>Export</span>
      </button>
      
      {isOpen && (
        <div className="absolute right-0 mt-1 w-48 bg-white border border-gray-200 rounded-lg shadow-lg z-50">
          <button
            onClick={() => handleExport('csv')}
            className="w-full flex items-center space-x-3 px-4 py-3 text-sm text-gray-700 hover:bg-gray-50"
          >
            <FileText className="h-4 w-4 text-green-600" />
            <span>Export as CSV</span>
          </button>
          <button
            onClick={() => handleExport('excel')}
            className="w-full flex items-center space-x-3 px-4 py-3 text-sm text-gray-700 hover:bg-gray-50"
          >
            <FileSpreadsheet className="h-4 w-4 text-green-600" />
            <span>Export as Excel</span>
          </button>
          <button
            onClick={() => handleExport('json')}
            className="w-full flex items-center space-x-3 px-4 py-3 text-sm text-gray-700 hover:bg-gray-50"
          >
            <FileJson className="h-4 w-4 text-blue-600" />
            <span>Export as JSON</span>
          </button>
        </div>
      )}
    </div>
  );
}

export function SearchResults<T = any>({
  results,
  columns,
  selection,
  onSelectionChange,
  onSort,
  onPageChange,
  onPageSizeChange,
  onRowClick,
  onExport,
  loading = false,
  error = null,
  emptyMessage = 'No results found',
  className,
  enableVirtualization = false,
  enableBulkActions = false,
}: SearchResultsProps<T>) {
  const { items, totalCount, pageCount, currentPage, pageSize, executionTimeMs } = results;
  
  // Selection handling
  const handleSelectAll = useCallback(() => {
    if (!onSelectionChange || !selection) return;
    
    const allSelected = !selection.isAllSelected;
    const newSelectedIds = allSelected 
      ? new Set(items.map((item: any) => item.id))
      : new Set<string>();
    
    onSelectionChange({
      selectedIds: newSelectedIds,
      selectedItems: allSelected ? items : [],
      isAllSelected: allSelected,
      isIndeterminate: false,
    });
  }, [items, selection, onSelectionChange]);
  
  const handleSelectItem = useCallback((item: T, selected: boolean) => {
    if (!onSelectionChange || !selection) return;
    
    const itemId = (item as any).id;
    const newSelectedIds = new Set(selection.selectedIds);
    
    if (selected) {
      newSelectedIds.add(itemId);
    } else {
      newSelectedIds.delete(itemId);
    }
    
    const selectedItems = items.filter((i: any) => newSelectedIds.has(i.id));
    const isAllSelected = newSelectedIds.size === items.length && items.length > 0;
    const isIndeterminate = newSelectedIds.size > 0 && !isAllSelected;
    
    onSelectionChange({
      selectedIds: newSelectedIds,
      selectedItems,
      isAllSelected,
      isIndeterminate,
    });
  }, [items, selection, onSelectionChange]);
  
  const handleClearSelection = useCallback(() => {
    if (!onSelectionChange) return;
    
    onSelectionChange({
      selectedIds: new Set(),
      selectedItems: [],
      isAllSelected: false,
      isIndeterminate: false,
    });
  }, [onSelectionChange]);
  
  // Loading state
  if (loading) {
    return (
      <div className="flex items-center justify-center py-12">
        <Loader2 className="h-8 w-8 text-blue-600 animate-spin" />
        <span className="ml-3 text-gray-600">Loading results...</span>
      </div>
    );
  }
  
  // Error state
  if (error) {
    return (
      <div className="flex flex-col items-center justify-center py-12 text-center">
        <AlertCircle className="h-12 w-12 text-red-500 mb-4" />
        <h3 className="text-lg font-medium text-gray-900 mb-2">Error loading results</h3>
        <p className="text-sm text-gray-600 max-w-md">{error.message}</p>
      </div>
    );
  }
  
  // Empty state
  if (items.length === 0) {
    return (
      <div className="flex flex-col items-center justify-center py-12 text-center">
        <Search className="h-12 w-12 text-gray-300 mb-4" />
        <h3 className="text-lg font-medium text-gray-900 mb-2">{emptyMessage}</h3>
        <p className="text-sm text-gray-600">Try adjusting your filters or search query</p>
      </div>
    );
  }
  
  return (
    <div className={clsx('bg-white rounded-lg border border-gray-200 overflow-hidden', className)}>
      {/* Bulk Actions Bar */}
      {enableBulkActions && selection && (
        <BulkActionsBar
          selectedCount={selection.selectedIds.size}
          totalCount={totalCount}
          isAllSelected={selection.isAllSelected}
          isIndeterminate={selection.isIndeterminate}
          onToggleSelectAll={handleSelectAll}
          onClearSelection={handleClearSelection}
        />
      )}
      
      {/* Results Header */}
      <div className="flex items-center justify-between px-4 py-3 border-b border-gray-200">
        <div className="flex items-center space-x-4">
          <span className="text-sm text-gray-600">
            <span className="font-semibold text-gray-900">{totalCount.toLocaleString()}</span> results
          </span>
          {executionTimeMs > 0 && (
            <span className="text-xs text-gray-400">
              ({executionTimeMs.toFixed(2)} ms)
            </span>
          )}
        </div>
        
        {onExport && (
          <ExportMenu onExport={onExport} disabled={items.length === 0} />
        )}
      </div>
      
      {/* Results Table */}
      <div className="overflow-x-auto">
        <table className="w-full">
          <TableHeader
            columns={columns}
            sort={results.sort}
            onSort={onSort}
            selection={selection}
            onSelectAll={handleSelectAll}
          />
          <tbody>
            {items.map((item, index) => (
              <TableRow
                key={(item as any).id || index}
                item={item}
                columns={columns}
                index={index}
                selection={selection}
                onSelect={handleSelectItem}
                onClick={onRowClick}
                isSelected={selection?.selectedIds.has((item as any).id)}
              />
            ))}
          </tbody>
        </table>
      </div>
      
      {/* Pagination */}
      {onPageChange && pageCount > 1 && (
        <Pagination
          currentPage={currentPage}
          pageSize={pageSize}
          totalCount={totalCount}
          pageCount={pageCount}
          onPageChange={onPageChange}
          onPageSizeChange={onPageSizeChange || (() => {})}
        />
      )}
    </div>
  );
}

export default SearchResults;

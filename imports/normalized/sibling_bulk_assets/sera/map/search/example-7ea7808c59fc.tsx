/**
 * Search System Example Usage
 * 
 * This file demonstrates how to use the advanced search and filter system
 * in S.E.R.A. frontend components.
 */

'use client';

import React, { useState, useCallback } from 'react';
import {
  AdvancedSearchBar,
  FilterPanel,
  FilterChips,
  SearchResults,
  useSearch,
  useFilters,
  FilterGroup,
  ColumnConfig,
  ActiveFilter,
  SearchSuggestion,
} from './index';

// Example entity type
interface Individual {
  id: string;
  firstName: string;
  lastName: string;
  dateOfBirth: string;
  status: 'active' | 'inactive' | 'on_hold';
  homeId: string;
  homeName: string;
  primaryDiagnosis: string;
  admissionDate: string;
}

// Example filter configuration
const filterGroups: FilterGroup[] = [
  {
    id: 'demographics',
    label: 'Demographics',
    icon: <span>👤</span>,
    description: 'Personal information filters',
    expanded: true,
    filters: [
      {
        id: 'firstName',
        field: 'firstName',
        label: 'First Name',
        type: 'text',
        operator: 'contains',
        value: '',
        placeholder: 'Search first name...',
      },
      {
        id: 'lastName',
        field: 'lastName',
        label: 'Last Name',
        type: 'text',
        operator: 'contains',
        value: '',
        placeholder: 'Search last name...',
      },
      {
        id: 'dateOfBirth',
        field: 'dateOfBirth',
        label: 'Date of Birth',
        type: 'date',
        operator: 'date_between',
        value: null,
        config: {
          minDate: new Date('1900-01-01'),
          maxDate: new Date(),
        },
      },
    ],
  },
  {
    id: 'status',
    label: 'Status & Location',
    icon: <span>📍</span>,
    filters: [
      {
        id: 'status',
        field: 'status',
        label: 'Status',
        type: 'multi_select',
        operator: 'in',
        value: [],
        options: [
          { value: 'active', label: 'Active', color: '#22c55e' },
          { value: 'inactive', label: 'Inactive', color: '#ef4444' },
          { value: 'on_hold', label: 'On Hold', color: '#f59e0b' },
        ],
      },
      {
        id: 'homeId',
        field: 'homeId',
        label: 'Home',
        type: 'select',
        operator: 'equals',
        value: '',
        options: [
          { value: 'home1', label: 'Sunrise House' },
          { value: 'home2', label: 'Maple Grove' },
          { value: 'home3', label: 'Oakwood Villa' },
        ],
      },
    ],
  },
  {
    id: 'clinical',
    label: 'Clinical',
    icon: <span>🏥</span>,
    filters: [
      {
        id: 'primaryDiagnosis',
        field: 'primaryDiagnosis',
        label: 'Primary Diagnosis',
        type: 'select',
        operator: 'equals',
        value: '',
        options: [
          { value: 'autism', label: 'Autism Spectrum Disorder' },
          { value: 'idd', label: 'Intellectual Disability' },
          { value: 'cp', label: 'Cerebral Palsy' },
          { value: 'ds', label: 'Down Syndrome' },
        ],
      },
      {
        id: 'admissionDate',
        field: 'admissionDate',
        label: 'Admission Date',
        type: 'date',
        operator: 'date_between',
        value: null,
      },
    ],
  },
];

// Example column configuration
const columns: ColumnConfig<Individual>[] = [
  {
    id: 'firstName',
    field: 'firstName',
    header: 'First Name',
    sortable: true,
    filterable: true,
    width: '150px',
  },
  {
    id: 'lastName',
    field: 'lastName',
    header: 'Last Name',
    sortable: true,
    filterable: true,
    width: '150px',
  },
  {
    id: 'dateOfBirth',
    field: 'dateOfBirth',
    header: 'Date of Birth',
    sortable: true,
    width: '130px',
    formatter: (value) => value ? new Date(value).toLocaleDateString() : '-',
  },
  {
    id: 'status',
    field: 'status',
    header: 'Status',
    sortable: true,
    width: '120px',
    formatter: (value) => {
      const colors: Record<string, string> = {
        active: 'bg-green-100 text-green-800',
        inactive: 'bg-red-100 text-red-800',
        on_hold: 'bg-yellow-100 text-yellow-800',
      };
      return (
        <span className={`px-2 py-1 text-xs font-medium rounded-full ${colors[value] || 'bg-gray-100'}`}>
          {value?.replace('_', ' ')}
        </span>
      );
    },
  },
  {
    id: 'homeName',
    field: 'homeName',
    header: 'Home',
    sortable: true,
    width: '180px',
  },
  {
    id: 'primaryDiagnosis',
    field: 'primaryDiagnosis',
    header: 'Primary Diagnosis',
    sortable: true,
    width: '200px',
  },
  {
    id: 'admissionDate',
    field: 'admissionDate',
    header: 'Admission Date',
    sortable: true,
    width: '130px',
    formatter: (value) => value ? new Date(value).toLocaleDateString() : '-',
  },
];

// Mock API call
async function searchIndividuals(query: any) {
  // In real usage, this would call your API
  console.log('Searching with query:', query);
  
  // Simulate API delay
  await new Promise(resolve => setTimeout(resolve, 500));
  
  // Return mock data
  return {
    items: [
      {
        id: '1',
        firstName: 'John',
        lastName: 'Smith',
        dateOfBirth: '1990-05-15',
        status: 'active',
        homeId: 'home1',
        homeName: 'Sunrise House',
        primaryDiagnosis: 'autism',
        admissionDate: '2020-01-15',
      },
      {
        id: '2',
        firstName: 'Jane',
        lastName: 'Doe',
        dateOfBirth: '1985-08-22',
        status: 'active',
        homeId: 'home2',
        homeName: 'Maple Grove',
        primaryDiagnosis: 'idd',
        admissionDate: '2019-03-10',
      },
    ],
    totalCount: 2,
    pageCount: 1,
    currentPage: 1,
    pageSize: 25,
    hasNextPage: false,
    hasPreviousPage: false,
    executionTimeMs: 125,
  };
}

/**
 * Example: Basic Search with useSearch Hook
 */
export function BasicSearchExample() {
  const [results, setResults] = useState<any>({
    items: [],
    totalCount: 0,
    pageCount: 0,
    currentPage: 1,
    pageSize: 25,
    hasNextPage: false,
    hasPreviousPage: false,
    executionTimeMs: 0,
  });
  
  const handleSearch = useCallback(async (searchQuery: any) => {
    const data = await searchIndividuals(searchQuery);
    setResults(data);
  }, []);
  
  const {
    query,
    filters,
    isLoading,
    suggestions,
    history,
    setQuery,
    search,
    addFilter,
    removeFilter,
    clearFilters,
    updateSort,
    updatePagination,
    hasActiveFilters,
  } = useSearch({
    syncWithUrl: true,
    debounceMs: 300,
    onSearch: handleSearch,
  });
  
  // Convert search filters to ActiveFilter format
  const activeFilters: ActiveFilter[] = filters.map(f => ({
    id: f.id,
    field: f.field,
    label: f.label,
    type: f.type,
    operator: f.operator,
    value: f.value,
    displayValue: String(f.value),
  }));
  
  return (
    <div className="space-y-4">
      <AdvancedSearchBar
        value={query}
        onChange={setQuery}
        onSearch={search}
        filters={activeFilters}
        onFilterRemove={removeFilter}
        onClearFilters={clearFilters}
        suggestions={suggestions}
        history={history}
        loading={isLoading}
        voiceEnabled={true}
      />
      
      <SearchResults<Individual>
        results={results}
        columns={columns}
        onSort={updateSort}
        onPageChange={(page) => updatePagination({ page })}
        onPageSizeChange={(pageSize) => updatePagination({ pageSize })}
        loading={isLoading}
      />
    </div>
  );
}

/**
 * Example: Advanced Search with Filter Panel
 */
export function AdvancedFilterExample() {
  const [results, setResults] = useState<any>({
    items: [],
    totalCount: 0,
    pageCount: 0,
    currentPage: 1,
    pageSize: 25,
    hasNextPage: false,
    hasPreviousPage: false,
    executionTimeMs: 0,
  });
  
  const [query, setQuery] = useState('');
  
  const handleSearch = useCallback(async () => {
    const searchParams = {
      text: query,
      filters: activeFilters,
      pagination: { page: 1, pageSize: 25 },
    };
    const data = await searchIndividuals(searchParams);
    setResults(data);
  }, [query]);
  
  const {
    activeFilters,
    presets,
    filterGroups,
    setFilter,
    removeFilter,
    clearFilters,
    savePreset,
    loadPreset,
    deletePreset,
    hasActiveFilters,
  } = useFilters({
    filterConfig: filterGroups,
    syncWithUrl: true,
    onFilterChange: handleSearch,
  });
  
  return (
    <div className="flex gap-4">
      {/* Filter Sidebar */}
      <div className="w-80 flex-shrink-0">
        <FilterPanel
          groups={filterGroups}
          activeFilters={activeFilters}
          presets={presets}
          onFilterChange={setFilter}
          onFilterRemove={removeFilter}
          onClearFilters={clearFilters}
          onSavePreset={savePreset}
          onLoadPreset={loadPreset}
          onDeletePreset={deletePreset}
          expanded={true}
        />
      </div>
      
      {/* Results Area */}
      <div className="flex-1 space-y-4">
        <AdvancedSearchBar
          value={query}
          onChange={setQuery}
          onSearch={handleSearch}
          filters={activeFilters}
          onFilterRemove={removeFilter}
          onClearFilters={clearFilters}
        />
        
        <SearchResults<Individual>
          results={results}
          columns={columns}
          loading={false}
        />
      </div>
    </div>
  );
}

/**
 * Example: Search with Bulk Selection
 */
export function BulkSelectionExample() {
  const [results, setResults] = useState<any>({
    items: [
      {
        id: '1',
        firstName: 'John',
        lastName: 'Smith',
        dateOfBirth: '1990-05-15',
        status: 'active',
        homeId: 'home1',
        homeName: 'Sunrise House',
        primaryDiagnosis: 'autism',
        admissionDate: '2020-01-15',
      },
      {
        id: '2',
        firstName: 'Jane',
        lastName: 'Doe',
        dateOfBirth: '1985-08-22',
        status: 'active',
        homeId: 'home2',
        homeName: 'Maple Grove',
        primaryDiagnosis: 'idd',
        admissionDate: '2019-03-10',
      },
    ],
    totalCount: 2,
    pageCount: 1,
    currentPage: 1,
    pageSize: 25,
    hasNextPage: false,
    hasPreviousPage: false,
    executionTimeMs: 50,
  });
  
  const [selection, setSelection] = useState({
    selectedIds: new Set<string>(),
    selectedItems: [],
    isAllSelected: false,
    isIndeterminate: false,
  });
  
  const handleExport = useCallback((config: any) => {
    console.log('Exporting with config:', config);
    console.log('Selected items:', selection.selectedItems);
  }, [selection]);
  
  return (
    <SearchResults<Individual>
      results={results}
      columns={columns}
      selection={selection}
      onSelectionChange={setSelection}
      onExport={handleExport}
      enableBulkActions={true}
    />
  );
}

/**
 * Example: Complete Search Page
 */
export function CompleteSearchPage() {
  return (
    <div className="p-6 space-y-6">
      <h1 className="text-2xl font-bold text-gray-900">Individual Search</h1>
      <AdvancedFilterExample />
    </div>
  );
}

export default CompleteSearchPage;

/**
 * Undo/Redo System Examples
 * 
 * Common usage patterns for the S.E.R.A. undo system.
 */

import React, { useState } from 'react';
import {
  useUndo,
  useUndoForm,
  useUndoToast,
  useUndoManager,
  deleteIndividual,
  updateLogEntry,
  changeMARStatus,
  UndoToastContainer,
} from './index';

// ============================================================================
// Example 1: Basic Counter with Undo
// ============================================================================

function CounterExample() {
  const { state, setState, undo, redo, canUndo, canRedo } = useUndo({
    initialState: { count: 0 },
    entityType: 'custom',
    entityId: 'counter-example'
  });

  return (
    <div className="p-4 border rounded">
      <h3 className="font-bold mb-2">Counter with Undo</h3>
      <p className="text-2xl mb-4">Count: {state.count}</p>
      
      <div className="space-x-2">
        <button
          onClick={() => setState({ count: state.count + 1 }, 'Increment')}
          className="px-3 py-1 bg-blue-500 text-white rounded"
        >
          +1
        </button>
        <button
          onClick={() => setState({ count: state.count - 1 }, 'Decrement')}
          className="px-3 py-1 bg-blue-500 text-white rounded"
        >
          -1
        </button>
        <button
          onClick={() => setState({ count: 0 }, 'Reset')}
          className="px-3 py-1 bg-gray-500 text-white rounded"
        >
          Reset
        </button>
      </div>
      
      <div className="mt-4 space-x-2">
        <button
          onClick={undo}
          disabled={!canUndo}
          className="px-3 py-1 bg-yellow-500 text-white rounded disabled:opacity-50"
        >
          Undo
        </button>
        <button
          onClick={redo}
          disabled={!canRedo}
          className="px-3 py-1 bg-green-500 text-white rounded disabled:opacity-50"
        >
          Redo
        </button>
      </div>
    </div>
  );
}

// ============================================================================
// Example 2: Form with Undo
// ============================================================================

interface UserFormData {
  firstName: string;
  lastName: string;
  email: string;
}

function FormExample() {
  const {
    state,
    setField,
    undo,
    redo,
    canUndo,
    canRedo,
    submit,
    reset,
    isDirty,
    isSubmitting,
    errors
  } = useUndoForm<UserFormData>(
    {
      firstName: '',
      lastName: '',
      email: ''
    },
    {
      entityType: 'custom',
      entityId: 'user-form',
      debounceMs: 300,
      validate: (state) => {
        const errors: Partial<Record<keyof UserFormData, string>> = {};
        if (!state.firstName) errors.firstName = 'First name is required';
        if (!state.email) errors.email = 'Email is required';
        if (state.email && !state.email.includes('@')) {
          errors.email = 'Invalid email';
        }
        return Object.keys(errors).length > 0 ? errors : null;
      },
      onSubmit: async (state) => {
        // Submit to API
        await fetch('/api/users', {
          method: 'POST',
          body: JSON.stringify(state)
        });
      }
    }
  );

  return (
    <form className="p-4 border rounded" onSubmit={(e) => { e.preventDefault(); submit(); }}>
      <h3 className="font-bold mb-2">Form with Undo</h3>
      
      <div className="space-y-3">
        <div>
          <input
            type="text"
            placeholder="First Name"
            value={state.firstName}
            onChange={(e) => setField('firstName', e.target.value)}
            className="border p-2 rounded w-full"
          />
          {errors?.firstName && (
            <p className="text-red-500 text-sm">{errors.firstName}</p>
          )}
        </div>
        
        <div>
          <input
            type="text"
            placeholder="Last Name"
            value={state.lastName}
            onChange={(e) => setField('lastName', e.target.value)}
            className="border p-2 rounded w-full"
          />
        </div>
        
        <div>
          <input
            type="email"
            placeholder="Email"
            value={state.email}
            onChange={(e) => setField('email', e.target.value)}
            className="border p-2 rounded w-full"
          />
          {errors?.email && (
            <p className="text-red-500 text-sm">{errors.email}</p>
          )}
        </div>
      </div>
      
      <div className="mt-4 space-x-2">
        <button
          type="submit"
          disabled={isSubmitting || !isDirty}
          className="px-3 py-1 bg-blue-500 text-white rounded disabled:opacity-50"
        >
          {isSubmitting ? 'Saving...' : 'Save'}
        </button>
        <button
          type="button"
          onClick={undo}
          disabled={!canUndo}
          className="px-3 py-1 bg-yellow-500 text-white rounded disabled:opacity-50"
        >
          Undo
        </button>
        <button
          type="button"
          onClick={redo}
          disabled={!canRedo}
          className="px-3 py-1 bg-green-500 text-white rounded disabled:opacity-50"
        >
          Redo
        </button>
        <button
          type="button"
          onClick={reset}
          disabled={!isDirty}
          className="px-3 py-1 bg-gray-500 text-white rounded disabled:opacity-50"
        >
          Reset
        </button>
      </div>
      
      {isDirty && (
        <p className="text-orange-500 text-sm mt-2">You have unsaved changes</p>
      )}
    </form>
  );
}

// ============================================================================
// Example 3: Delete with Undo Toast
// ============================================================================

function DeleteWithUndoExample() {
  const { showSuccess } = useUndoToast();
  const [individuals, setIndividuals] = useState([
    { id: '1', firstName: 'John', lastName: 'Doe' },
    { id: '2', firstName: 'Jane', lastName: 'Smith' }
  ]);

  const handleDelete = async (individual: typeof individuals[0]) => {
    try {
      await deleteIndividual(individual, {
        userId: 'current-user-id',
        onSuccess: () => {
          // Remove from local state
          setIndividuals(prev => prev.filter(i => i.id !== individual.id));
          showSuccess(`Deleted ${individual.firstName} ${individual.lastName}`);
        }
      });
    } catch (error) {
      console.error('Delete failed:', error);
    }
  };

  return (
    <div className="p-4 border rounded">
      <h3 className="font-bold mb-2">Delete with Undo</h3>
      
      <ul className="space-y-2">
        {individuals.map(individual => (
          <li key={individual.id} className="flex justify-between items-center p-2 bg-gray-50 rounded">
            <span>{individual.firstName} {individual.lastName}</span>
            <button
              onClick={() => handleDelete(individual)}
              className="px-2 py-1 bg-red-500 text-white text-sm rounded"
            >
              Delete
            </button>
          </li>
        ))}
      </ul>
      
      {individuals.length === 0 && (
        <p className="text-gray-500">No individuals. Undo the last delete to restore.</p>
      )}
    </div>
  );
}

// ============================================================================
// Example 4: Batch Operations with Transactions
// ============================================================================

function BatchOperationsExample() {
  const { undoManager, canUndo, undo } = useUndoManager();
  const [items, setItems] = useState([
    { id: '1', name: 'Item 1', selected: false },
    { id: '2', name: 'Item 2', selected: false },
    { id: '3', name: 'Item 3', selected: false }
  ]);

  const toggleSelection = (id: string) => {
    setItems(prev => prev.map(item =>
      item.id === id ? { ...item, selected: !item.selected } : item
    ));
  };

  const deleteSelected = async () => {
    const selectedItems = items.filter(i => i.selected);
    
    if (selectedItems.length === 0) return;

    undoManager.startTransaction(`Delete ${selectedItems.length} items`);
    
    try {
      // Simulate API calls
      for (const item of selectedItems) {
        await new Promise(resolve => setTimeout(resolve, 100));
      }
      
      setItems(prev => prev.filter(i => !i.selected));
      undoManager.endTransaction();
    } catch (error) {
      undoManager.cancelTransaction();
      console.error('Batch delete failed:', error);
    }
  };

  const selectedCount = items.filter(i => i.selected).length;

  return (
    <div className="p-4 border rounded">
      <h3 className="font-bold mb-2">Batch Operations</h3>
      
      <ul className="space-y-1 mb-4">
        {items.map(item => (
          <li key={item.id} className="flex items-center gap-2">
            <input
              type="checkbox"
              checked={item.selected}
              onChange={() => toggleSelection(item.id)}
              id={`item-${item.id}`}
            />
            <label htmlFor={`item-${item.id}`}>{item.name}</label>
          </li>
        ))}
      </ul>
      
      <div className="space-x-2">
        <button
          onClick={deleteSelected}
          disabled={selectedCount === 0}
          className="px-3 py-1 bg-red-500 text-white rounded disabled:opacity-50"
        >
          Delete Selected ({selectedCount})
        </button>
        <button
          onClick={undo}
          disabled={!canUndo}
          className="px-3 py-1 bg-yellow-500 text-white rounded disabled:opacity-50"
        >
          Undo Batch
        </button>
      </div>
    </div>
  );
}

// ============================================================================
// Example 5: Global Undo Manager Access
// ============================================================================

function GlobalUndoExample() {
  const { undoManager, canUndo, canRedo, undoStackSize } = useUndoManager();

  return (
    <div className="p-4 border rounded">
      <h3 className="font-bold mb-2">Global Undo Status</h3>
      
      <div className="space-y-2 text-sm">
        <p>Can Undo: {canUndo ? 'Yes' : 'No'}</p>
        <p>Can Redo: {canRedo ? 'Yes' : 'No'}</p>
        <p>Undo Stack Size: {undoStackSize}</p>
      </div>
      
      <div className="mt-4 space-x-2">
        <button
          onClick={() => undoManager.clearHistory()}
          className="px-3 py-1 bg-gray-500 text-white rounded"
        >
          Clear All History
        </button>
      </div>
    </div>
  );
}

// ============================================================================
// Example 6: Complete App Setup
// ============================================================================

export function UndoSystemDemo() {
  return (
    <div className="max-w-4xl mx-auto p-6 space-y-6">
      <h1 className="text-2xl font-bold">Undo/Redo System Examples</h1>
      
      <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
        <CounterExample />
        <FormExample />
        <DeleteWithUndoExample />
        <BatchOperationsExample />
        <GlobalUndoExample />
      </div>
      
      {/* Toast Container - Required for toast notifications */}
      <UndoToastContainer position="bottom-right" defaultTimeout={5000} />
    </div>
  );
}

export default UndoSystemDemo;

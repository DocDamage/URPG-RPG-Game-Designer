/**
 * Keyboard Shortcuts Usage Examples
 * 
 * This file demonstrates how to use the keyboard shortcuts system
 * in various scenarios. These are examples only - don't import this file.
 */

// ============================================
// Example 1: Basic shortcut registration
// ============================================

import { useEffect } from 'react';
import { useKeyboardShortcuts } from './useKeyboardShortcuts';

function ExampleComponent() {
  const { register } = useKeyboardShortcuts();

  useEffect(() => {
    // Register a simple shortcut
    const unregister = register({
      id: 'example-focus-search',
      key: 'f',
      modifiers: { ctrl: true },
      description: 'Focus the search field',
      category: 'actions',
      action: () => {
        document.getElementById('search')?.focus();
      },
      showToast: true,
      toastMessage: 'Search field focused'
    });

    // Cleanup on unmount
    return () => unregister();
  }, [register]);

  return <input id="search" placeholder="Search..." />;
}

// ============================================
// Example 2: Page-specific shortcuts
// ============================================

import { usePageShortcuts } from './useKeyboardShortcuts';

function DashboardPage() {
  // These shortcuts only work on the dashboard
  usePageShortcuts({
    context: 'dashboard',
    shortcuts: [
      {
        id: 'dashboard-refresh',
        key: 'r',
        description: 'Refresh dashboard data',
        category: 'page-specific',
        action: () => {
          console.log('Refreshing dashboard...');
        },
        showToast: true
      },
      {
        id: 'dashboard-export',
        key: 'e',
        modifiers: { ctrl: true, shift: true },
        description: 'Export dashboard report',
        category: 'page-specific',
        action: () => {
          console.log('Exporting report...');
        },
        showToast: true
      }
    ]
  });

  return <div>Dashboard Content</div>;
}

// ============================================
// Example 3: Conditional shortcuts
// ============================================

import { useConditionalShortcut } from './useKeyboardShortcuts';

function EditableTable({ isEditing }: { isEditing: boolean }) {
  // This shortcut only works when in edit mode
  useConditionalShortcut({
    shortcut: {
      key: 'Delete',
      description: 'Delete selected row',
      category: 'actions',
      action: () => {
        console.log('Deleting row...');
      },
      showToast: true
    },
    condition: isEditing
  });

  return <table>...</table>;
}

// ============================================
// Example 4: Role-based shortcuts
// ============================================

function AdminPanel() {
  const { register } = useKeyboardShortcuts();

  useEffect(() => {
    // This shortcut is only available to admins
    const unregister = register({
      id: 'admin-purge-cache',
      key: 'p',
      modifiers: { ctrl: true, alt: true, shift: true },
      description: 'Purge system cache',
      category: 'admin',
      roles: ['admin', 'super_admin'], // Only these roles can use it
      action: () => {
        console.log('Purging cache...');
      },
      showToast: true,
      toastMessage: 'System cache purged'
    });

    return () => unregister();
  }, [register]);

  return <div>Admin Panel</div>;
}

// ============================================
// Example 5: Multiple shortcuts at once
// ============================================

import { useShortcutsBatch } from './useKeyboardShortcuts';

function ComplexForm() {
  useShortcutsBatch([
    {
      id: 'form-save',
      key: 's',
      modifiers: { ctrl: true },
      description: 'Save form',
      category: 'actions',
      action: () => saveForm(),
      preventDefault: true // Prevent browser save dialog
    },
    {
      id: 'form-validate',
      key: 'v',
      modifiers: { ctrl: true },
      description: 'Validate form',
      category: 'actions',
      action: () => validateForm()
    },
    {
      id: 'form-reset',
      key: 'r',
      modifiers: { ctrl: true, alt: true },
      description: 'Reset form',
      category: 'actions',
      action: () => resetForm(),
      showToast: true
    }
  ]);

  return <form>...</form>;
}

// ============================================
// Example 6: Using the shortcut context
// ============================================

import { useShortcutContext } from './useKeyboardShortcuts';

function HelpButton() {
  const { showHelp, isHelpOpen, userRole } = useShortcutContext();

  return (
    <div>
      <button onClick={showHelp}>
        Show Shortcuts Help
        {userRole && <span> (Role: {userRole})</span>}
      </button>
      {isHelpOpen && <span>Help is open</span>}
    </div>
  );
}

// ============================================
// Example 7: Listening to shortcut events
// ============================================

import { useEffect } from 'react';

function EventListenerExample() {
  useEffect(() => {
    // Listen for the global save event
    const handleSave = () => {
      console.log('Save shortcut triggered!');
    };

    document.addEventListener('shortcut-save', handleSave);
    return () => document.removeEventListener('shortcut-save', handleSave);
  }, []);

  return <div>Listening for save events...</div>;
}

// ============================================
// Example 8: Custom events from shortcuts
// ============================================

function CustomEventExample() {
  const { register } = useKeyboardShortcuts();

  useEffect(() => {
    const unregister = register({
      id: 'custom-action',
      key: 'x',
      modifiers: { ctrl: true, shift: true },
      description: 'Trigger custom action',
      category: 'quick',
      action: () => {
        // Dispatch a custom event that other components can listen to
        document.dispatchEvent(new CustomEvent('my-custom-event', {
          detail: { timestamp: Date.now() }
        }));
      }
    });

    return () => unregister();
  }, [register]);

  return <div>Press Ctrl+Shift+X to trigger custom event</div>;
}

// ============================================
// Helper functions (for demonstration)
// ============================================

function saveForm() { console.log('Saving...'); }
function validateForm() { console.log('Validating...'); }
function resetForm() { console.log('Resetting...'); }

// Export empty to make this a module
export {};

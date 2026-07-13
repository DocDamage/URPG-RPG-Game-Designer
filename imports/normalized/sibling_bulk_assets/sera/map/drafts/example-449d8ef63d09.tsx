/**
 * Draft System Usage Examples
 * 
 * This file demonstrates how to use the auto-save draft system
 * with different form types in S.E.R.A.
 */

'use client';

import React, { useState, useEffect } from 'react';
import {
  useAutoSave,
  AutoSaveIndicator,
  DraftRecoveryModal,
  DraftList,
  draftManager,
  draftEncryption,
  Draft,
} from './index';

// ============================================
// Example 1: Basic DSP Log Form with Auto-Save
// ============================================

interface LogFormData {
  content: string;
  timestamp: string;
  mood?: string;
  activities?: string[];
}

export function ExampleLogForm({ individualId, individualName }: { 
  individualId: string; 
  individualName: string;
}) {
  const {
    data,
    setData,
    status,
    lastSaved,
    hasUnsavedChanges,
    saveNow,
    discardDraft,
  } = useAutoSave<LogFormData>({
    formType: 'log',
    individualId,
    initialData: {
      content: '',
      timestamp: new Date().toISOString(),
    },
    title: `Log for ${individualName}`,
  });

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    
    // Save one more time to ensure all changes are captured
    await saveNow();
    
    // Submit to server
    console.log('Submitting log:', data);
    
    // Clean up draft after successful submission
    await discardDraft();
    
    alert('Log submitted successfully!');
  };

  return (
    <form onSubmit={handleSubmit} className="space-y-4 p-6">
      <div className="flex justify-between items-center">
        <h2 className="text-xl font-semibold">Daily Log Entry</h2>
        <AutoSaveIndicator
          status={status}
          lastSaved={lastSaved}
          hasUnsavedChanges={hasUnsavedChanges}
        />
      </div>

      <div>
        <label className="block text-sm font-medium mb-1">Log Content</label>
        <textarea
          className="w-full p-3 border rounded-lg h-40"
          value={data.content}
          onChange={(e) => setData({ ...data, content: e.target.value })}
          placeholder="Enter log details..."
        />
      </div>

      <div>
        <label className="block text-sm font-medium mb-1">Mood</label>
        <select
          className="w-full p-2 border rounded"
          value={data.mood || ''}
          onChange={(e) => setData({ ...data, mood: e.target.value })}
        >
          <option value="">Select mood...</option>
          <option value="happy">Happy</option>
          <option value="calm">Calm</option>
          <option value="agitated">Agitated</option>
          <option value="distressed">Distressed</option>
        </select>
      </div>

      <div className="flex justify-between items-center pt-4">
        <p className="text-sm text-gray-500">
          {hasUnsavedChanges && 'You have unsaved changes'}
        </p>
        <button
          type="submit"
          className="px-6 py-2 bg-blue-600 text-white rounded-lg hover:bg-blue-700"
        >
          Submit Log
        </button>
      </div>
    </form>
  );
}

// ============================================
// Example 2: Incident Form with Conflict Detection
// ============================================

interface IncidentFormData {
  type: string;
  severity: 'low' | 'medium' | 'high' | 'critical';
  description: string;
  location: string;
  witnesses: string[];
}

export function ExampleIncidentForm({
  incidentId,
  serverData,
  serverTimestamp,
}: {
  incidentId: string;
  serverData?: IncidentFormData;
  serverTimestamp?: number;
}) {
  const [showRecovery, setShowRecovery] = useState(false);
  const [resolved, setResolved] = useState(false);

  const {
    data,
    setData,
    status,
    lastSaved,
    hasConflict,
    conflict,
    resolveConflict,
    checkForConflict,
    saveNow,
    discardDraft,
  } = useAutoSave<IncidentFormData>({
    formType: 'incident',
    formId: incidentId,
    initialData: serverData || {
      type: '',
      severity: 'low',
      description: '',
      location: '',
      witnesses: [],
    },
    title: `Incident #${incidentId}`,
    onConflict: (conflict) => {
      console.log('Conflict detected:', conflict);
      setShowRecovery(true);
    },
  });

  // Check for conflict on mount
  useEffect(() => {
    if (serverData && serverTimestamp && !resolved) {
      checkForConflict(serverData, serverTimestamp);
    }
  }, [serverData, serverTimestamp]);

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    await saveNow();
    console.log('Submitting incident:', data);
    await discardDraft();
    alert('Incident report submitted!');
  };

  return (
    <>
      <form onSubmit={handleSubmit} className="space-y-4 p-6">
        <div className="flex justify-between items-center">
          <h2 className="text-xl font-semibold">Incident Report</h2>
          <div className="flex items-center gap-2">
            {hasConflict && (
              <span className="text-amber-600 text-sm flex items-center">
                ⚠️ Conflict detected
              </span>
            )}
            <AutoSaveIndicator
              status={status}
              lastSaved={lastSaved}
              hasUnsavedChanges={false}
            />
          </div>
        </div>

        {hasConflict && (
          <div className="bg-amber-50 border border-amber-200 p-4 rounded-lg">
            <p className="text-amber-800 text-sm">
              This incident has been modified by another user. 
              <button
                type="button"
                onClick={() => setShowRecovery(true)}
                className="text-amber-600 underline ml-1"
              >
                Resolve conflict
              </button>
            </p>
          </div>
        )}

        <div className="grid grid-cols-2 gap-4">
          <div>
            <label className="block text-sm font-medium mb-1">Incident Type</label>
            <select
              className="w-full p-2 border rounded"
              value={data.type}
              onChange={(e) => setData({ ...data, type: e.target.value })}
            >
              <option value="">Select type...</option>
              <option value="behavioral">Behavioral</option>
              <option value="medical">Medical</option>
              <option value="safety">Safety</option>
              <option value="other">Other</option>
            </select>
          </div>

          <div>
            <label className="block text-sm font-medium mb-1">Severity</label>
            <select
              className="w-full p-2 border rounded"
              value={data.severity}
              onChange={(e) => setData({ ...data, severity: e.target.value as any })}
            >
              <option value="low">Low</option>
              <option value="medium">Medium</option>
              <option value="high">High</option>
              <option value="critical">Critical</option>
            </select>
          </div>
        </div>

        <div>
          <label className="block text-sm font-medium mb-1">Description</label>
          <textarea
            className="w-full p-3 border rounded-lg h-32"
            value={data.description}
            onChange={(e) => setData({ ...data, description: e.target.value })}
            placeholder="Describe the incident..."
          />
        </div>

        <button
          type="submit"
          className="w-full py-2 bg-red-600 text-white rounded-lg hover:bg-red-700"
        >
          Submit Incident Report
        </button>
      </form>

      {showRecovery && (
        <DraftRecoveryModal
          formType="incident"
          formId={incidentId}
          serverData={serverData}
          serverTimestamp={serverTimestamp}
          onRecover={async (recoveredData, resolution) => {
            await resolveConflict(resolution, recoveredData);
            setShowRecovery(false);
            setResolved(true);
          }}
          onDiscard={() => {
            setShowRecovery(false);
          }}
          onClose={() => setShowRecovery(false)}
        />
      )}
    </>
  );
}

// ============================================
// Example 3: MAR Form with Encryption
// ============================================

interface MARFormData {
  medication: string;
  dosage: string;
  administeredAt: string;
  notes: string;
  witness?: string;
}

export function ExampleMARForm({ individualId }: { individualId: string }) {
  const {
    data,
    setData,
    status,
    lastSaved,
    saveNow,
    discardDraft,
  } = useAutoSave<MARFormData>({
    formType: 'mar',
    individualId,
    initialData: {
      medication: '',
      dosage: '',
      administeredAt: new Date().toISOString(),
      notes: '',
    },
    title: 'Medication Administration',
    // Encryption is enabled by default for MAR
  });

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    await saveNow();
    console.log('Submitting MAR:', data);
    await discardDraft();
    alert('MAR entry submitted!');
  };

  return (
    <form onSubmit={handleSubmit} className="space-y-4 p-6">
      <div className="flex justify-between items-center">
        <div>
          <h2 className="text-xl font-semibold">Medication Administration</h2>
          <p className="text-sm text-gray-500">This form is encrypted for HIPAA compliance</p>
        </div>
        <AutoSaveIndicator
          status={status}
          lastSaved={lastSaved}
          hasUnsavedChanges={false}
        />
      </div>

      <div className="grid grid-cols-2 gap-4">
        <div>
          <label className="block text-sm font-medium mb-1">Medication</label>
          <input
            type="text"
            className="w-full p-2 border rounded"
            value={data.medication}
            onChange={(e) => setData({ ...data, medication: e.target.value })}
          />
        </div>

        <div>
          <label className="block text-sm font-medium mb-1">Dosage</label>
          <input
            type="text"
            className="w-full p-2 border rounded"
            value={data.dosage}
            onChange={(e) => setData({ ...data, dosage: e.target.value })}
          />
        </div>
      </div>

      <div>
        <label className="block text-sm font-medium mb-1">Notes</label>
        <textarea
          className="w-full p-3 border rounded-lg h-24"
          value={data.notes}
          onChange={(e) => setData({ ...data, notes: e.target.value })}
        />
      </div>

      <button
        type="submit"
        className="w-full py-2 bg-green-600 text-white rounded-lg hover:bg-green-700"
      >
        Record Administration
      </button>
    </form>
  );
}

// ============================================
// Example 4: Draft Management Page
// ============================================

export function ExampleDraftsPage() {
  const [selectedDraft, setSelectedDraft] = useState<Draft | null>(null);

  const handleResume = (draft: Draft) => {
    setSelectedDraft(draft);
    console.log('Resuming draft:', draft);
    // Navigate to appropriate form with draft data
  };

  return (
    <div className="p-6">
      <h1 className="text-2xl font-bold mb-6">Draft Management</h1>
      
      <DraftList
        onResumeDraft={handleResume}
        initialFilter={{}}
      />

      {selectedDraft && (
        <div className="mt-6 p-4 bg-blue-50 rounded-lg">
          <h3 className="font-semibold">Selected Draft</h3>
          <pre className="text-sm mt-2 overflow-auto">
            {JSON.stringify(selectedDraft, null, 2)}
          </pre>
        </div>
      )}
    </div>
  );
}

// ============================================
// Example 5: Initialization (in your auth flow)
// ============================================

export async function initializeDraftSystem(sessionToken: string) {
  // Initialize encryption with session token
  await draftEncryption.initialize(sessionToken);
  
  // Initialize draft manager
  await draftManager.initialize();
  
  console.log('Draft system initialized');
}

export function cleanupDraftSystem() {
  // Clear encryption keys on logout
  draftEncryption.clear();
  console.log('Draft system cleaned up');
}

// ============================================
// Example 6: Form with Custom Configuration
// ============================================

export function ExampleCustomConfigForm() {
  const { data, setData, status } = useAutoSave({
    formType: 'assessment',
    initialData: { questions: [] },
    title: 'Quarterly Assessment',
    config: {
      intervalMs: 60000,    // Save every minute
      debounceMs: 3000,     // Wait 3 seconds after typing
      retentionDays: 180,   // Keep for 6 months
      encryptSensitive: true,
      conflictStrategy: 'manual',
    },
    onSave: (draft) => {
      console.log('Draft saved:', draft.id);
    },
    onError: (error) => {
      console.error('Save failed:', error);
    },
  });

  return (
    <div className="p-6">
      <div className="flex justify-between items-center mb-4">
        <h2 className="text-xl font-semibold">Quarterly Assessment</h2>
        <AutoSaveIndicator status={status} lastSaved={null} hasUnsavedChanges={false} />
      </div>
      
      <p className="text-gray-600">
        This form auto-saves every minute and keeps drafts for 6 months.
      </p>
    </div>
  );
}

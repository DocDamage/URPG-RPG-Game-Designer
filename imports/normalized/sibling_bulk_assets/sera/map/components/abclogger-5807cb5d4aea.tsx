"use client";

import React, { useState } from 'react';

type Severity = 'low' | 'medium' | 'high';

interface ABCEntry {
  antecedent: string;
  behavior: string;
  consequence: string;
  severity: Severity;
}

const initialEntry: ABCEntry = {
  antecedent: '',
  behavior: '',
  consequence: '',
  severity: 'low',
};

export function ABCLogger() {
  const [entry, setEntry] = useState<ABCEntry>(initialEntry);
  const [savedEntries, setSavedEntries] = useState<ABCEntry[]>([]);

  const updateEntry = (field: keyof ABCEntry, value: string) => {
    setEntry(current => ({ ...current, [field]: value }));
  };

  const saveEntry = () => {
    if (!entry.antecedent.trim() || !entry.behavior.trim() || !entry.consequence.trim()) return;
    setSavedEntries(current => [entry, ...current]);
    setEntry(initialEntry);
  };

  return (
    <section className="rounded-lg bg-white p-6 shadow-sm">
      <div className="mb-4">
        <h2 className="text-xl font-semibold text-gray-900">ABC Logger</h2>
        <p className="text-sm text-gray-600">Record antecedent, behavior, and consequence notes for review.</p>
      </div>

      <div className="grid gap-4">
        <label className="grid gap-2">
          <span className="text-sm font-medium text-gray-700">Antecedent</span>
          <textarea
            className="min-h-24 rounded-md border border-gray-300 p-3"
            value={entry.antecedent}
            onChange={event => updateEntry('antecedent', event.target.value)}
            placeholder="What happened before the behavior?"
          />
        </label>

        <label className="grid gap-2">
          <span className="text-sm font-medium text-gray-700">Behavior</span>
          <textarea
            className="min-h-24 rounded-md border border-gray-300 p-3"
            value={entry.behavior}
            onChange={event => updateEntry('behavior', event.target.value)}
            placeholder="Describe the observable behavior."
          />
        </label>

        <label className="grid gap-2">
          <span className="text-sm font-medium text-gray-700">Consequence</span>
          <textarea
            className="min-h-24 rounded-md border border-gray-300 p-3"
            value={entry.consequence}
            onChange={event => updateEntry('consequence', event.target.value)}
            placeholder="What happened immediately after?"
          />
        </label>

        <label className="grid gap-2">
          <span className="text-sm font-medium text-gray-700">Severity</span>
          <select
            className="rounded-md border border-gray-300 p-3"
            value={entry.severity}
            onChange={event => updateEntry('severity', event.target.value as Severity)}
          >
            <option value="low">Low</option>
            <option value="medium">Medium</option>
            <option value="high">High</option>
          </select>
        </label>

        <button
          type="button"
          className="rounded-md bg-blue-600 px-4 py-2 font-medium text-white disabled:opacity-50"
          onClick={saveEntry}
          disabled={!entry.antecedent.trim() || !entry.behavior.trim() || !entry.consequence.trim()}
        >
          Save ABC Entry
        </button>
      </div>

      {savedEntries.length > 0 && (
        <div className="mt-6 space-y-3">
          <h3 className="font-semibold text-gray-900">Recent Entries</h3>
          {savedEntries.map((savedEntry, index) => (
            <article key={`${savedEntry.behavior}-${index}`} className="rounded-md border border-gray-200 p-4">
              <div className="mb-2 text-sm font-medium uppercase text-gray-500">Severity: {savedEntry.severity}</div>
              <p><strong>Antecedent:</strong> {savedEntry.antecedent}</p>
              <p><strong>Behavior:</strong> {savedEntry.behavior}</p>
              <p><strong>Consequence:</strong> {savedEntry.consequence}</p>
            </article>
          ))}
        </div>
      )}
    </section>
  );
}

export default ABCLogger;

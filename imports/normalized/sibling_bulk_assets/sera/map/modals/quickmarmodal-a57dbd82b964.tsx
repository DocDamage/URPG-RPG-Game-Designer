/**
 * QuickMARModal Component
 * Quick medication administration entry modal
 */

'use client';

import React, { useState, useCallback, useEffect } from 'react';
import { clsx } from 'clsx';
import {
  Pill,
  Clock,
  User,
  AlertCircle,
  Check,
  X,
  Mic,
  MicOff,
  Camera,
  ScanLine,
  Shield,
  AlertTriangle,
} from 'lucide-react';
import { Dialog } from '../../ui/dialog';
import { Input } from '../../ui/input';
import { Textarea } from '../../ui/textarea';
import { Button } from '../../ui/button';
import { Select } from '../../ui/select';
import { QuickActionModalProps, QuickMARData } from '../types';
import { validateQuickMARData } from '../actions';
import { medicationsAPI } from '../../../lib/api/medications';
import { useVoiceInput } from '../../../lib/hooks/useVoiceInput';

interface QuickMARModalProps extends QuickActionModalProps {
  individualId?: string;
  medicationId?: string;
}

// Sample medications - in real app, fetch from API
const SAMPLE_MEDICATIONS = [
  { value: '', label: 'Select medication...' },
  { value: 'med_1', label: 'Lisinopril 10mg', dosage: '10mg', instructions: 'Take once daily in the morning' },
  { value: 'med_2', label: 'Metformin 500mg', dosage: '500mg', instructions: 'Take with meals' },
  { value: 'med_3', label: 'Atorvastatin 20mg', dosage: '20mg', instructions: 'Take once daily at bedtime' },
  { value: 'med_4', label: 'Levothyroxine 50mcg', dosage: '50mcg', instructions: 'Take on empty stomach' },
  { value: 'med_5', label: 'Omeprazole 20mg', dosage: '20mg', instructions: 'Take before breakfast' },
];

const REFUSAL_REASONS = [
  { value: '', label: 'Select reason...' },
  { value: 'taste', label: 'Dislikes taste' },
  { value: 'swallowing', label: 'Difficulty swallowing' },
  { value: 'nausea', label: 'Feeling nauseous' },
  { value: 'side_effects', label: 'Concerned about side effects' },
  { value: 'not_needed', label: 'Believes not needed' },
  { value: 'asleep', label: 'Individual asleep' },
  { value: 'away', label: 'Individual away from home' },
  { value: 'other', label: 'Other (specify in notes)' },
];

export function QuickMARModal({
  isOpen,
  onClose,
  context,
  onSuccess,
  individualId: propIndividualId,
  medicationId: propMedicationId,
}: QuickMARModalProps) {
  // Form state
  const [formData, setFormData] = useState<Partial<QuickMARData>>({
    individualId: propIndividualId || context?.individualId || '',
    medication: '',
    dosage: '',
    administeredAt: new Date().toISOString().slice(0, 16),
    notes: '',
    refused: false,
    refusalReason: '',
  });

  const [errors, setErrors] = useState<string[]>([]);
  const [isSubmitting, setIsSubmitting] = useState(false);
  const [showSuccess, setShowSuccess] = useState(false);
  const [showRefused, setShowRefused] = useState(false);
  const [selectedMedInfo, setSelectedMedInfo] = useState<typeof SAMPLE_MEDICATIONS[0] | null>(null);
  const [showBarcodeScanner, setShowBarcodeScanner] = useState(false);

  // Voice input hook
  const {
    isListening,
    interimTranscript,
    isSupported: isVoiceSupported,
    startListening,
    stopListening,
    resetTranscript,
  } = useVoiceInput({
    onResult: (text) => {
      setFormData(prev => ({
        ...prev,
        notes: prev.notes + (prev.notes ? ' ' : '') + text,
      }));
    },
  });

  // Handle medication selection
  const handleMedicationChange = useCallback((medicationValue: string) => {
    const med = SAMPLE_MEDICATIONS.find(m => m.value === medicationValue);
    setSelectedMedInfo(med || null);
    
    setFormData(prev => ({
      ...prev,
      medication: med?.label || medicationValue,
      dosage: med?.dosage || '',
    }));
  }, []);

  // Handle form changes
  const handleChange = useCallback((field: keyof QuickMARData, value: any) => {
    setFormData(prev => {
      const next = { ...prev, [field]: value };
      
      // If refused is unchecked, clear refusal reason
      if (field === 'refused' && !value) {
        next.refusalReason = '';
      }
      
      return next;
    });
    
    if (errors.length > 0) {
      setErrors([]);
    }
  }, [errors.length]);

  // Toggle voice input
  const toggleVoiceInput = useCallback(() => {
    if (isListening) {
      stopListening();
    } else {
      resetTranscript();
      startListening();
    }
  }, [isListening, startListening, stopListening, resetTranscript]);

  // Handle barcode scan (simulated)
  const handleBarcodeScan = useCallback(() => {
    setShowBarcodeScanner(true);
    // Simulate scan
    setTimeout(() => {
      handleMedicationChange('med_1');
      setShowBarcodeScanner(false);
    }, 1500);
  }, [handleMedicationChange]);

  // Handle submit
  const handleSubmit = useCallback(async (e: React.FormEvent) => {
    e.preventDefault();
    
    // Validate
    const validationErrors = validateQuickMARData(formData);
    if (validationErrors.length > 0) {
      setErrors(validationErrors);
      return;
    }

    setIsSubmitting(true);

    try {
      await medicationsAPI.mar.create({
        individualId: Number(formData.individualId),
        medication: formData.medication!,
        dosage: formData.dosage!,
        administeredAt: formData.administeredAt!,
        notes: formData.notes,
      });

      setShowSuccess(true);
      
      setTimeout(() => {
        onSuccess?.();
        onClose();
        // Reset form
        setFormData({
          individualId: '',
          medication: '',
          dosage: '',
          administeredAt: new Date().toISOString().slice(0, 16),
          notes: '',
          refused: false,
          refusalReason: '',
        });
        setSelectedMedInfo(null);
        setShowRefused(false);
        setShowSuccess(false);
      }, 1000);
    } catch (error) {
      setErrors(['Failed to save MAR entry. Please try again.']);
    } finally {
      setIsSubmitting(false);
    }
  }, [formData, onClose, onSuccess]);

  // Handle close
  const handleClose = useCallback(() => {
    if (isListening) {
      stopListening();
    }
    onClose();
  }, [isListening, stopListening, onClose]);

  // Footer content
  const footer = (
    <div className="flex justify-end gap-3">
      <Button
        type="button"
        variant="outline"
        onClick={handleClose}
        disabled={isSubmitting}
      >
        Cancel
      </Button>
      <Button
        type="submit"
        form="quick-mar-form"
        disabled={isSubmitting}
        className={clsx(
          'gap-2',
          formData.refused ? 'bg-amber-600 hover:bg-amber-700' : 'bg-green-600 hover:bg-green-700'
        )}
      >
        {isSubmitting ? (
          <>
            <div className="w-4 h-4 border-2 border-white/30 border-t-white rounded-full animate-spin" />
            Saving...
          </>
        ) : showSuccess ? (
          <>
            <Check className="w-4 h-4" />
            Saved!
          </>
        ) : formData.refused ? (
          <>
            <X className="w-4 h-4" />
            Record Refusal
          </>
        ) : (
          <>
            <Check className="w-4 h-4" />
            Record Administration
          </>
        )}
      </Button>
    </div>
  );

  return (
    <Dialog
      isOpen={isOpen}
      onClose={handleClose}
      title={
        <div className="flex items-center gap-2">
          <Pill className="w-5 h-5 text-green-500" />
          Quick MAR Entry
        </div>
      }
      footer={footer}
      size="lg"
    >
      <form id="quick-mar-form" onSubmit={handleSubmit} className="space-y-4">
        {/* Error Messages */}
        {errors.length > 0 && (
          <div className="p-3 bg-red-50 dark:bg-red-900/20 border border-red-200 dark:border-red-800 rounded-lg">
            <div className="flex items-center gap-2 text-red-700 dark:text-red-300 mb-1">
              <AlertCircle className="w-4 h-4" />
              <span className="font-medium text-sm">Please fix the following:</span>
            </div>
            <ul className="text-sm text-red-600 dark:text-red-400 list-disc list-inside">
              {errors.map((error, i) => (
                <li key={i}>{error}</li>
              ))}
            </ul>
          </div>
        )}

        {/* Individual Selection */}
        <div className="grid grid-cols-1 sm:grid-cols-2 gap-4">
          <div>
            <label className="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-1">
              <span className="flex items-center gap-1.5">
                <User className="w-4 h-4" />
                Individual *
              </span>
            </label>
            <Select
              value={formData.individualId}
              onChange={e => handleChange('individualId', e.target.value)}
              options={[
                { value: '', label: 'Select individual...' },
                { value: '1', label: 'John Smith' },
                { value: '2', label: 'Jane Doe' },
                { value: '3', label: 'Bob Johnson' },
              ]}
              required
            />
          </div>

          <div>
            <label className="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-1">
              <span className="flex items-center gap-1.5">
                <Clock className="w-4 h-4" />
                Date & Time *
              </span>
            </label>
            <Input
              type="datetime-local"
              value={formData.administeredAt}
              onChange={e => handleChange('administeredAt', e.target.value)}
              required
            />
          </div>
        </div>

        {/* Medication Selection */}
        <div>
          <div className="flex items-center justify-between mb-1">
            <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
              <span className="flex items-center gap-1.5">
                <Pill className="w-4 h-4" />
                Medication *
              </span>
            </label>
            <button
              type="button"
              onClick={handleBarcodeScan}
              disabled={showBarcodeScanner}
              className="flex items-center gap-1 text-xs text-blue-600 hover:text-blue-700 disabled:opacity-50"
            >
              <ScanLine className="w-3.5 h-3.5" />
              {showBarcodeScanner ? 'Scanning...' : 'Scan Barcode'}
            </button>
          </div>
          
          <Select
            value={selectedMedInfo?.value || ''}
            onChange={e => handleMedicationChange(e.target.value)}
            options={SAMPLE_MEDICATIONS}
            required
          />

          {/* Medication Info Card */}
          {selectedMedInfo && selectedMedInfo.value && (
            <div className="mt-2 p-3 bg-blue-50 dark:bg-blue-900/20 rounded-lg">
              <div className="flex items-start gap-2">
                <Shield className="w-4 h-4 text-blue-600 dark:text-blue-400 mt-0.5" />
                <div>
                  <p className="text-sm font-medium text-blue-900 dark:text-blue-200">
                    {selectedMedInfo.label}
                  </p>
                  <p className="text-xs text-blue-700 dark:text-blue-300 mt-0.5">
                    {selectedMedInfo.instructions}
                  </p>
                </div>
              </div>
            </div>
          )}
        </div>

        {/* Dosage */}
        <div>
          <label className="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-1">
            Dosage Administered *
          </label>
          <Input
            type="text"
            value={formData.dosage}
            onChange={e => handleChange('dosage', e.target.value)}
            placeholder="e.g., 10mg, 1 tablet, 5ml"
            required
          />
        </div>

        {/* Refused Toggle */}
        <div className="p-3 bg-gray-50 dark:bg-gray-800 rounded-lg">
          <label className="flex items-center gap-3 cursor-pointer">
            <input
              type="checkbox"
              checked={formData.refused}
              onChange={e => handleChange('refused', e.target.checked)}
              className="w-5 h-5 text-amber-600 rounded border-gray-300 focus:ring-amber-500"
            />
            <div className="flex-1">
              <span className="flex items-center gap-2 font-medium text-gray-900 dark:text-white">
                <X className="w-4 h-4 text-amber-500" />
                Medication Refused
              </span>
              <p className="text-xs text-gray-500 dark:text-gray-400 mt-0.5">
                Check if the individual refused to take the medication
              </p>
            </div>
          </label>
        </div>

        {/* Refusal Reason */}
        {formData.refused && (
          <div className="animate-in slide-in-from-top-2">
            <label className="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-1">
              <span className="flex items-center gap-1.5 text-amber-600">
                <AlertTriangle className="w-4 h-4" />
                Refusal Reason *
              </span>
            </label>
            <Select
              value={formData.refusalReason}
              onChange={e => handleChange('refusalReason', e.target.value)}
              options={REFUSAL_REASONS}
              required={formData.refused}
            />
          </div>
        )}

        {/* Notes */}
        <div>
          <div className="flex items-center justify-between mb-1">
            <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
              Notes
            </label>
            <div className="flex items-center gap-2">
              {isVoiceSupported && (
                <button
                  type="button"
                  onClick={toggleVoiceInput}
                  className={clsx(
                    'p-1.5 rounded-lg transition-colors',
                    isListening
                      ? 'bg-red-100 text-red-600 dark:bg-red-900 dark:text-red-300 animate-pulse'
                      : 'bg-gray-100 text-gray-600 dark:bg-gray-700 dark:text-gray-400 hover:bg-gray-200'
                  )}
                >
                  {isListening ? <MicOff className="w-4 h-4" /> : <Mic className="w-4 h-4" />}
                </button>
              )}
            </div>
          </div>

          {isListening && interimTranscript && (
            <div className="mb-2 p-2 bg-blue-50 dark:bg-blue-900/20 rounded-lg">
              <p className="text-sm text-blue-600 dark:text-blue-300 italic">
                {interimTranscript}
              </p>
            </div>
          )}

          <Textarea
            value={formData.notes}
            onChange={e => handleChange('notes', e.target.value)}
            placeholder={formData.refused 
              ? "Document refusal details, individual's condition, attempts made..."
              : "Any observations, side effects noted, or additional information..."
            }
            rows={3}
            className="resize-none"
          />
        </div>

        {/* Quick Note Templates */}
        <div className="flex flex-wrap gap-2">
          <span className="text-xs text-gray-500">Quick add:</span>
          {(formData.refused ? 
            ['Individual alert', 'No distress noted', 'Will retry later', 'Family notified'] :
            ['Taken without issue', 'With water', 'After meal', 'Observed swallowing']
          ).map(template => (
            <button
              key={template}
              type="button"
              onClick={() => handleChange('notes', formData.notes + (formData.notes ? '; ' : '') + template)}
              className="text-xs px-2 py-1 bg-gray-100 dark:bg-gray-700 text-gray-600 dark:text-gray-400 rounded hover:bg-gray-200 transition-colors"
            >
              + {template}
            </button>
          ))}
        </div>

        {/* Signature/Photo Evidence Hint */}
        <div className="p-3 bg-yellow-50 dark:bg-yellow-900/20 rounded-lg flex items-start gap-2">
          <Camera className="w-4 h-4 text-yellow-600 dark:text-yellow-400 mt-0.5" />
          <p className="text-xs text-yellow-700 dark:text-yellow-300">
            {formData.refused 
              ? 'Consider documenting refusal with a photo if appropriate and per policy.'
              : 'You may be prompted for a signature or photo evidence based on medication type.'
            }
          </p>
        </div>
      </form>
    </Dialog>
  );
}

export default QuickMARModal;

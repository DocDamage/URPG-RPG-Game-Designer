/**
 * QuickIncidentModal Component
 * Rapid incident reporting modal for quick actions
 */

'use client';

import React, { useState, useCallback } from 'react';
import { clsx } from 'clsx';
import {
  AlertTriangle,
  Clock,
  User,
  MapPin,
  AlertCircle,
  Check,
  Siren,
  Shield,
  FileText,
  Camera,
  Mic,
  MicOff,
} from 'lucide-react';
import { Dialog } from '../../ui/dialog';
import { Input } from '../../ui/input';
import { Textarea } from '../../ui/textarea';
import { Button } from '../../ui/button';
import { Select } from '../../ui/select';
import { QuickActionModalProps, QuickIncidentData } from '../types';
import { validateQuickIncidentData, INCIDENT_TYPES, SEVERITY_LEVELS, COMMON_LOCATIONS } from '../actions';
import { incidentsAPI } from '../../../lib/api/incidents';
import { useVoiceInput } from '../../../lib/hooks/useVoiceInput';

interface QuickIncidentModalProps extends QuickActionModalProps {
  individualId?: string;
}

export function QuickIncidentModal({
  isOpen,
  onClose,
  context,
  onSuccess,
  individualId: propIndividualId,
}: QuickIncidentModalProps) {
  // Form state
  const [formData, setFormData] = useState<Partial<QuickIncidentData>>({
    individualId: propIndividualId || context?.individualId || '',
    type: '',
    severity: 'minor',
    description: '',
    occurredAt: new Date().toISOString().slice(0, 16),
    location: '',
    immediateActions: '',
    isCrisisMode: false,
  });

  const [errors, setErrors] = useState<string[]>([]);
  const [isSubmitting, setIsSubmitting] = useState(false);
  const [showSuccess, setShowSuccess] = useState(false);
  const [step, setStep] = useState<1 | 2>(1);
  const [charCount, setCharCount] = useState(0);

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
        description: prev.description + (prev.description ? ' ' : '') + text,
      }));
      setCharCount(prev => prev + text.length);
    },
  });

  // Handle form changes
  const handleChange = useCallback((field: keyof QuickIncidentData, value: any) => {
    setFormData(prev => ({ ...prev, [field]: value }));
    if (field === 'description') {
      setCharCount(value.length);
    }
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

  // Handle next step
  const handleNext = useCallback(() => {
    const step1Fields = ['individualId', 'type', 'severity', 'occurredAt'];
    const step1Errors = validateQuickIncidentData(formData).filter(
      err => step1Fields.some(field => err.toLowerCase().includes(field.toLowerCase()))
    );
    
    if (step1Errors.length > 0) {
      setErrors(step1Errors);
      return;
    }
    
    setErrors([]);
    setStep(2);
  }, [formData]);

  // Handle submit
  const handleSubmit = useCallback(async (e: React.FormEvent) => {
    e.preventDefault();
    
    // Validate
    const validationErrors = validateQuickIncidentData(formData);
    if (validationErrors.length > 0) {
      setErrors(validationErrors);
      return;
    }

    setIsSubmitting(true);

    try {
      await incidentsAPI.create({
        individualId: formData.individualId!,
        reportedBy: 'current-user', // Would come from auth context
        reportedAt: new Date().toISOString(),
        type: formData.type!,
        category: formData.type!,
        severity: formData.severity!,
        status: 'draft',
        occurredAt: formData.occurredAt!,
        location: formData.location || 'Unknown',
        description: formData.description!,
        immediateActions: formData.immediateActions ? [formData.immediateActions] : [],
        isCrisisMode: formData.isCrisisMode || false,
      });

      setShowSuccess(true);
      
      setTimeout(() => {
        onSuccess?.();
        onClose();
        // Reset form
        setFormData({
          individualId: '',
          type: '',
          severity: 'minor',
          description: '',
          occurredAt: new Date().toISOString().slice(0, 16),
          location: '',
          immediateActions: '',
          isCrisisMode: false,
        });
        setStep(1);
        setCharCount(0);
        setShowSuccess(false);
      }, 1000);
    } catch (error) {
      setErrors(['Failed to submit incident report. Please try again.']);
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

  // Get severity color
  const getSeverityColor = (severity: string) => {
    const config = SEVERITY_LEVELS.find(s => s.value === severity);
    return config?.color || 'bg-gray-500';
  };

  // Footer content
  const footer = (
    <div className="flex justify-between items-center w-full">
      <div className="text-xs text-gray-500">
        {step === 1 ? 'Step 1 of 2: Basic Information' : 'Step 2 of 2: Details'}
      </div>
      <div className="flex gap-3">
        {step === 2 && (
          <Button
            type="button"
            variant="outline"
            onClick={() => setStep(1)}
            disabled={isSubmitting}
          >
            Back
          </Button>
        )}
        <Button
          type="button"
          variant="outline"
          onClick={handleClose}
          disabled={isSubmitting}
        >
          Cancel
        </Button>
        {step === 1 ? (
          <Button type="button" onClick={handleNext}>
            Next
          </Button>
        ) : (
          <Button
            type="submit"
            form="quick-incident-form"
            disabled={isSubmitting}
            className={clsx(
              'gap-2',
              formData.isCrisisMode && 'bg-red-600 hover:bg-red-700'
            )}
          >
            {isSubmitting ? (
              <>
                <div className="w-4 h-4 border-2 border-white/30 border-t-white rounded-full animate-spin" />
                Submitting...
              </>
            ) : showSuccess ? (
              <>
                <Check className="w-4 h-4" />
                Submitted!
              </>
            ) : (
              <>
                <AlertTriangle className="w-4 h-4" />
                {formData.isCrisisMode ? 'Report Crisis' : 'Report Incident'}
              </>
            )}
          </Button>
        )}
      </div>
    </div>
  );

  return (
    <Dialog
      isOpen={isOpen}
      onClose={handleClose}
      title={
        <div className="flex items-center gap-2">
          <AlertTriangle className="w-5 h-5 text-red-500" />
          Report Incident
        </div>
      }
      footer={footer}
      size="lg"
    >
      <form id="quick-incident-form" onSubmit={handleSubmit} className="space-y-4">
        {/* Crisis Mode Alert */}
        {formData.isCrisisMode && (
          <div className="p-3 bg-red-50 dark:bg-red-900/20 border border-red-200 dark:border-red-800 rounded-lg animate-pulse">
            <div className="flex items-center gap-2 text-red-700 dark:text-red-300">
              <Siren className="w-5 h-5" />
              <span className="font-semibold">CRISIS MODE ACTIVATED</span>
            </div>
            <p className="text-sm text-red-600 dark:text-red-400 mt-1">
              This will immediately notify supervisors and administration.
            </p>
          </div>
        )}

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

        {step === 1 ? (
          /* Step 1: Basic Information */
          <>
            {/* Individual & Time */}
            <div className="grid grid-cols-1 sm:grid-cols-2 gap-4">
              <div>
                <label className="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-1">
                  <span className="flex items-center gap-1.5">
                    <User className="w-4 h-4" />
                    Individual Involved *
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
                    Date & Time of Incident *
                  </span>
                </label>
                <Input
                  type="datetime-local"
                  value={formData.occurredAt}
                  onChange={e => handleChange('occurredAt', e.target.value)}
                  required
                />
              </div>
            </div>

            {/* Incident Type */}
            <div>
              <label className="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                Incident Type *
              </label>
              <div className="grid grid-cols-2 sm:grid-cols-3 gap-2">
                {INCIDENT_TYPES.map(type => (
                  <button
                    key={type.value}
                    type="button"
                    onClick={() => handleChange('type', type.value)}
                    className={clsx(
                      'px-3 py-2 text-sm rounded-lg transition-colors text-left',
                      formData.type === type.value
                        ? 'bg-blue-100 text-blue-700 dark:bg-blue-900 dark:text-blue-300 border-2 border-blue-500'
                        : 'bg-gray-100 text-gray-600 dark:bg-gray-700 dark:text-gray-400 hover:bg-gray-200 dark:hover:bg-gray-600 border-2 border-transparent'
                    )}
                  >
                    {type.label}
                  </button>
                ))}
              </div>
            </div>

            {/* Severity */}
            <div>
              <label className="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                Severity Level *
              </label>
              <div className="flex gap-2">
                {SEVERITY_LEVELS.map(severity => (
                  <button
                    key={severity.value}
                    type="button"
                    onClick={() => handleChange('severity', severity.value)}
                    className={clsx(
                      'flex-1 px-3 py-2 text-sm rounded-lg transition-colors',
                      formData.severity === severity.value
                        ? 'ring-2 ring-offset-2 ring-gray-400'
                        : 'opacity-70 hover:opacity-100',
                      severity.color,
                      'text-white font-medium'
                    )}
                  >
                    {severity.label}
                  </button>
                ))}
              </div>
            </div>

            {/* Crisis Mode Toggle */}
            <div className="pt-2">
              <label className="flex items-center gap-3 p-3 bg-red-50 dark:bg-red-900/20 rounded-lg cursor-pointer border border-red-200 dark:border-red-800">
                <input
                  type="checkbox"
                  checked={formData.isCrisisMode}
                  onChange={e => handleChange('isCrisisMode', e.target.checked)}
                  className="w-5 h-5 text-red-600 rounded border-gray-300 focus:ring-red-500"
                />
                <div className="flex-1">
                  <span className="flex items-center gap-2 font-medium text-red-700 dark:text-red-300">
                    <Siren className="w-4 h-4" />
                    Activate Crisis Mode
                  </span>
                  <p className="text-xs text-red-600 dark:text-red-400 mt-0.5">
                    Immediate supervisor notification and emergency protocols
                  </p>
                </div>
              </label>
            </div>
          </>
        ) : (
          /* Step 2: Details */
          <>
            {/* Location */}
            <div>
              <label className="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-1">
                <span className="flex items-center gap-1.5">
                  <MapPin className="w-4 h-4" />
                  Location *
                </span>
              </label>
              <div className="flex flex-wrap gap-2 mb-2">
                {COMMON_LOCATIONS.map(loc => (
                  <button
                    key={loc}
                    type="button"
                    onClick={() => handleChange('location', loc)}
                    className={clsx(
                      'px-2 py-1 text-xs rounded-full transition-colors',
                      formData.location === loc
                        ? 'bg-blue-100 text-blue-700 dark:bg-blue-900 dark:text-blue-300'
                        : 'bg-gray-100 text-gray-600 dark:bg-gray-700 dark:text-gray-400 hover:bg-gray-200'
                    )}
                  >
                    {loc}
                  </button>
                ))}
              </div>
              <Input
                type="text"
                value={formData.location}
                onChange={e => handleChange('location', e.target.value)}
                placeholder="Or type location..."
                required
              />
            </div>

            {/* Description */}
            <div>
              <div className="flex items-center justify-between mb-1">
                <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                  Description of Incident *
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
                  <span className={clsx(
                    'text-xs',
                    charCount < 20 ? 'text-red-500' : 'text-gray-500'
                  )}>
                    {charCount} chars (min 20)
                  </span>
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
                value={formData.description}
                onChange={e => handleChange('description', e.target.value)}
                placeholder="Describe what happened in detail: What occurred? Who was involved? What were the circumstances?..."
                rows={4}
                required
                className="resize-none"
              />
            </div>

            {/* Immediate Actions */}
            <div>
              <label className="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-1">
                <span className="flex items-center gap-1.5">
                  <Shield className="w-4 h-4" />
                  Immediate Actions Taken
                </span>
              </label>
              <Textarea
                value={formData.immediateActions}
                onChange={e => handleChange('immediateActions', e.target.value)}
                placeholder="What actions did you take immediately after the incident? First aid provided? Notifications made?..."
                rows={3}
                className="resize-none"
              />
            </div>

            {/* Quick Actions Templates */}
            <div className="flex flex-wrap gap-2">
              <span className="text-xs text-gray-500">Quick add:</span>
              {['Individual was assessed', 'First aid administered', 'Family notified', 'Supervisor contacted', 'Documentation completed'].map(template => (
                <button
                  key={template}
                  type="button"
                  onClick={() => handleChange('immediateActions', formData.immediateActions + (formData.immediateActions ? '; ' : '') + template)}
                  className="text-xs px-2 py-1 bg-gray-100 dark:bg-gray-700 text-gray-600 dark:text-gray-400 rounded hover:bg-gray-200 transition-colors"
                >
                  + {template}
                </button>
              ))}
            </div>

            {/* Attach Evidence Hint */}
            <div className="p-3 bg-yellow-50 dark:bg-yellow-900/20 rounded-lg flex items-start gap-2">
              <Camera className="w-4 h-4 text-yellow-600 dark:text-yellow-400 mt-0.5" />
              <p className="text-xs text-yellow-700 dark:text-yellow-300">
                You can attach photos and additional documentation after submitting this initial report.
              </p>
            </div>
          </>
        )}
      </form>
    </Dialog>
  );
}

export default QuickIncidentModal;

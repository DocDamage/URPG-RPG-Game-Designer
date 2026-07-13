/**
 * QuickLogModal Component
 * Fast log entry modal for quick actions
 */

'use client';

import React, { useState, useCallback } from 'react';
import { clsx } from 'clsx';
import { FileText, Clock, User, Mic, MicOff, Lock, Sparkles, AlertCircle, Check } from 'lucide-react';
import { Dialog } from '../../ui/dialog';
import { Input } from '../../ui/input';
import { Textarea } from '../../ui/textarea';
import { Button } from '../../ui/button';
import { Select } from '../../ui/select';
import { QuickActionModalProps, QuickLogData } from '../types';
import { validateQuickLogData } from '../actions';
import { logsAPI } from '../../../lib/api/logs';
import { useVoiceInput } from '../../../lib/hooks/useVoiceInput';

interface QuickLogModalProps extends QuickActionModalProps {
  individualId?: string;
  prefilledContent?: string;
}

const LOG_CATEGORIES = [
  { value: 'daily', label: 'Daily Activity' },
  { value: 'behavior', label: 'Behavioral Support' },
  { value: 'health', label: 'Health & Wellness' },
  { value: 'community', label: 'Community Integration' },
  { value: 'skill', label: 'Skill Development' },
  { value: 'incident', label: 'Incident Related' },
  { value: 'other', label: 'Other' },
];

export function QuickLogModal({
  isOpen,
  onClose,
  context,
  onSuccess,
  individualId: propIndividualId,
  prefilledContent,
}: QuickLogModalProps) {
  // Form state
  const [formData, setFormData] = useState<Partial<QuickLogData>>({
    individualId: propIndividualId || context?.individualId || '',
    content: prefilledContent || '',
    timestamp: new Date().toISOString().slice(0, 16),
    category: 'daily',
    isEncrypted: false,
    useVoice: false,
  });

  const [errors, setErrors] = useState<string[]>([]);
  const [isSubmitting, setIsSubmitting] = useState(false);
  const [showSuccess, setShowSuccess] = useState(false);
  const [charCount, setCharCount] = useState(prefilledContent?.length || 0);

  // Voice input hook
  const {
    isListening,
    transcript,
    interimTranscript,
    isSupported: isVoiceSupported,
    startListening,
    stopListening,
    resetTranscript,
  } = useVoiceInput({
    onResult: (text) => {
      setFormData(prev => ({
        ...prev,
        content: prev.content + (prev.content ? ' ' : '') + text,
      }));
      setCharCount(prev => prev + text.length);
    },
  });

  // Handle form changes
  const handleChange = useCallback((field: keyof QuickLogData, value: any) => {
    setFormData(prev => ({ ...prev, [field]: value }));
    if (field === 'content') {
      setCharCount(value.length);
    }
    // Clear errors when user starts typing
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

  // Handle submit
  const handleSubmit = useCallback(async (e: React.FormEvent) => {
    e.preventDefault();
    
    // Validate
    const validationErrors = validateQuickLogData(formData);
    if (validationErrors.length > 0) {
      setErrors(validationErrors);
      return;
    }

    setIsSubmitting(true);

    try {
      await logsAPI.create({
        individualId: Number(formData.individualId),
        content: formData.content!,
        timestamp: formData.timestamp || new Date().toISOString(),
        encrypted: formData.isEncrypted,
        voiceToText: formData.useVoice,
        audioData: formData.audioData,
      });

      setShowSuccess(true);
      
      setTimeout(() => {
        onSuccess?.();
        onClose();
        // Reset form
        setFormData({
          individualId: '',
          content: '',
          timestamp: new Date().toISOString().slice(0, 16),
          category: 'daily',
          isEncrypted: false,
          useVoice: false,
        });
        setCharCount(0);
        setShowSuccess(false);
      }, 1000);
    } catch (error) {
      setErrors(['Failed to save log entry. Please try again.']);
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
        form="quick-log-form"
        disabled={isSubmitting || charCount < 10}
        className="gap-2"
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
        ) : (
          <>
            <FileText className="w-4 h-4" />
            Save Log Entry
          </>
        )}
      </Button>
    </div>
  );

  return (
    <Dialog
      isOpen={isOpen}
      onClose={handleClose}
      title="Quick Log Entry"
      footer={footer}
      size="lg"
    >
      <form id="quick-log-form" onSubmit={handleSubmit} className="space-y-4">
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
              value={formData.timestamp}
              onChange={e => handleChange('timestamp', e.target.value)}
              required
            />
          </div>
        </div>

        {/* Category */}
        <div>
          <label className="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-1">
            Category
          </label>
          <div className="flex flex-wrap gap-2">
            {LOG_CATEGORIES.map(cat => (
              <button
                key={cat.value}
                type="button"
                onClick={() => handleChange('category', cat.value)}
                className={clsx(
                  'px-3 py-1.5 text-sm rounded-full transition-colors',
                  formData.category === cat.value
                    ? 'bg-blue-100 text-blue-700 dark:bg-blue-900 dark:text-blue-300'
                    : 'bg-gray-100 text-gray-600 dark:bg-gray-700 dark:text-gray-400 hover:bg-gray-200 dark:hover:bg-gray-600'
                )}
              >
                {cat.label}
              </button>
            ))}
          </div>
        </div>

        {/* Content */}
        <div>
          <div className="flex items-center justify-between mb-1">
            <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
              Log Entry *
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
                      : 'bg-gray-100 text-gray-600 dark:bg-gray-700 dark:text-gray-400 hover:bg-gray-200 dark:hover:bg-gray-600'
                  )}
                  title={isListening ? 'Stop recording' : 'Start voice input'}
                >
                  {isListening ? <MicOff className="w-4 h-4" /> : <Mic className="w-4 h-4" />}
                </button>
              )}
              <span className={clsx(
                'text-xs',
                charCount < 10 ? 'text-red-500' : 'text-gray-500'
              )}>
                {charCount} chars (min 10)
              </span>
            </div>
          </div>

          {/* Voice transcription preview */}
          {isListening && interimTranscript && (
            <div className="mb-2 p-2 bg-blue-50 dark:bg-blue-900/20 rounded-lg">
              <p className="text-sm text-blue-600 dark:text-blue-300 italic">
                {interimTranscript}
              </p>
            </div>
          )}

          <Textarea
            value={formData.content}
            onChange={e => handleChange('content', e.target.value)}
            placeholder="Describe what happened, individual's activities, behaviors, interventions used, outcomes..."
            rows={5}
            required
            className="resize-none"
          />

          {/* Quick templates */}
          <div className="mt-2 flex flex-wrap gap-2">
            <span className="text-xs text-gray-500">Quick add:</span>
            {['Participated in', 'Refused', 'Assisted with', 'Independent in', 'Required prompting'].map(template => (
              <button
                key={template}
                type="button"
                onClick={() => handleChange('content', formData.content + (formData.content ? ' ' : '') + template)}
                className="text-xs px-2 py-1 bg-gray-100 dark:bg-gray-700 text-gray-600 dark:text-gray-400 rounded hover:bg-gray-200 dark:hover:bg-gray-600 transition-colors"
              >
                + {template}
              </button>
            ))}
          </div>
        </div>

        {/* Options */}
        <div className="flex flex-wrap items-center gap-4 pt-2">
          <label className="flex items-center gap-2 cursor-pointer">
            <input
              type="checkbox"
              checked={formData.isEncrypted}
              onChange={e => handleChange('isEncrypted', e.target.checked)}
              className="w-4 h-4 text-blue-600 rounded border-gray-300 focus:ring-blue-500"
            />
            <span className="flex items-center gap-1.5 text-sm text-gray-700 dark:text-gray-300">
              <Lock className="w-3.5 h-3.5" />
              Encrypt log entry
            </span>
          </label>

          <label className="flex items-center gap-2 cursor-pointer">
            <input
              type="checkbox"
              checked={formData.useVoice}
              onChange={e => handleChange('useVoice', e.target.checked)}
              className="w-4 h-4 text-blue-600 rounded border-gray-300 focus:ring-blue-500"
            />
            <span className="flex items-center gap-1.5 text-sm text-gray-700 dark:text-gray-300">
              <Sparkles className="w-3.5 h-3.5" />
              AI-enhanced
            </span>
          </label>
        </div>

        {/* Tips */}
        <div className="p-3 bg-blue-50 dark:bg-blue-900/20 rounded-lg">
          <p className="text-xs text-blue-700 dark:text-blue-300">
            <strong>Tip:</strong> Be specific, objective, and include dates, times, and quotes when relevant. 
            Avoid subjective language or personal opinions.
          </p>
        </div>
      </form>
    </Dialog>
  );
}

export default QuickLogModal;

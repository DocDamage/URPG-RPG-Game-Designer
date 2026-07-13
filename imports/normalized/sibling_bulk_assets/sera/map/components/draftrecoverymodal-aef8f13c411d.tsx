/**
 * Draft Recovery Modal
 * 
 * Modal component for detecting and recovering unsaved drafts
 * with conflict resolution between local and server versions
 */

'use client';

import React, { useState, useEffect } from 'react';
import { Dialog, DialogProps } from '@/components/ui/dialog';
import { Button } from '@/components/ui/button';
import {
  Card,
  CardContent,
  CardDescription,
  CardHeader,
  CardTitle,
} from '@/components/ui/card';
import {
  Draft,
  DraftConflict,
  DraftConflictResolution,
  FormType,
} from '../types';
import { draftManager } from '../draftManager';
import {
  formatRelativeTime,
  getFormTypeDisplayName,
  generateDraftPreview,
} from '../utils';
import {
  AlertTriangle,
  Clock,
  FileText,
  User,
  ChevronLeft,
  ChevronRight,
  Check,
} from 'lucide-react';

interface DraftRecoveryModalProps extends Omit<DialogProps, 'isOpen' | 'onClose'> {
  formType: FormType;
  formId?: string;
  individualId?: string;
  individualName?: string;
  serverData?: any;
  serverTimestamp?: number;
  onRecover: (data: any, resolution: DraftConflictResolution) => void;
  onDiscard: () => void;
  onClose: () => void;
}

export function DraftRecoveryModal({
  formType,
  formId,
  individualId,
  individualName,
  serverData,
  serverTimestamp,
  onRecover,
  onDiscard,
  onClose,
  title = 'Recover Unsaved Draft',
  ...dialogProps
}: DraftRecoveryModalProps) {
  const [draft, setDraft] = useState<Draft | null>(null);
  const [conflict, setConflict] = useState<DraftConflict | null>(null);
  const [loading, setLoading] = useState(true);
  const [selectedVersion, setSelectedVersion] = useState<'local' | 'server' | 'merge'>('local');
  const [mergedData, setMergedData] = useState<any>(null);
  const [showComparison, setShowComparison] = useState(false);

  useEffect(() => {
    checkForDraft();
  }, [formType, formId, individualId]);

  const checkForDraft = async () => {
    try {
      await draftManager.initialize();
      
      const existingDraft = await draftManager.getDraftForForm(
        formType,
        formId,
        individualId
      );

      if (existingDraft) {
        setDraft(existingDraft);

        // Check for conflict if server data provided
        if (serverData && serverTimestamp) {
          const detectedConflict = await draftManager.detectConflict(
            existingDraft.id,
            serverData,
            serverTimestamp
          );

          if (detectedConflict) {
            setConflict(detectedConflict);
            // Pre-populate merge with local data
            setMergedData(existingDraft.data);
          }
        }
      }
    } catch (error) {
      console.error('Failed to check for draft:', error);
    } finally {
      setLoading(false);
    }
  };

  const handleRecover = () => {
    if (!draft) return;

    const dataToRecover =
      selectedVersion === 'server'
        ? serverData
        : selectedVersion === 'merge'
        ? mergedData
        : draft.data;

    onRecover(dataToRecover, selectedVersion);
  };

  const handleDiscard = async () => {
    if (draft) {
      await draftManager.deleteDraft(draft.id);
    }
    onDiscard();
    onClose();
  };

  if (loading) {
    return (
      <Dialog isOpen={true} onClose={onClose} title={title} {...dialogProps}>
        <div className="flex items-center justify-center py-12">
          <div className="animate-spin rounded-full h-8 w-8 border-b-2 border-primary" />
        </div>
      </Dialog>
    );
  }

  if (!draft) {
    return null;
  }

  const isConflict = !!conflict;
  const hasServerVersion = !!serverData && serverTimestamp! > draft.updatedAt;

  return (
    <Dialog
      isOpen={true}
      onClose={onClose}
      title={isConflict ? 'Conflict Detected' : 'Recover Draft?'}
      size="lg"
      footer={
        <div className="flex justify-between w-full">
          <Button variant="outline" onClick={handleDiscard}>
            Discard Draft
          </Button>
          <div className="flex space-x-2">
            {showComparison && (
              <Button variant="outline" onClick={() => setShowComparison(false)}>
                <ChevronLeft className="w-4 h-4 mr-1" />
                Back
              </Button>
            )}
            <Button onClick={handleRecover}>
              <Check className="w-4 h-4 mr-1" />
              {isConflict
                ? selectedVersion === 'local'
                  ? 'Keep Local'
                  : selectedVersion === 'server'
                  ? 'Use Server'
                  : 'Use Merged'
                : 'Recover Draft'}
            </Button>
          </div>
        </div>
      }
      {...dialogProps}
    >
      <div className="space-y-4">
        {/* Warning for conflict */}
        {isConflict && (
          <div className="bg-amber-50 border border-amber-200 rounded-lg p-4 flex items-start">
            <AlertTriangle className="w-5 h-5 text-amber-600 mr-3 flex-shrink-0 mt-0.5" />
            <div>
              <h4 className="font-medium text-amber-800">Data Conflict Detected</h4>
              <p className="text-sm text-amber-700 mt-1">
                This form has been modified both locally and on the server. Please choose which version to keep.
              </p>
            </div>
          </div>
        )}

        {/* Draft info */}
        <div className="flex items-center text-sm text-gray-500">
          <FileText className="w-4 h-4 mr-1" />
          <span>{getFormTypeDisplayName(formType)}</span>
          {individualName && (
            <>
              <span className="mx-2">•</span>
              <User className="w-4 h-4 mr-1" />
              <span>{individualName}</span>
            </>
          )}
        </div>

        {!showComparison ? (
          <div className="grid gap-4">
            {/* Local version option */}
            <VersionCard
              title="Local Version"
              subtitle={`Saved ${formatRelativeTime(draft.updatedAt)}`}
              description={generateDraftPreview(draft)}
              selected={selectedVersion === 'local'}
              onClick={() => setSelectedVersion('local')}
              badge={isConflict ? 'Modified locally' : undefined}
              badgeColor="blue"
            />

            {/* Server version option (if available) */}
            {hasServerVersion && (
              <VersionCard
                title="Server Version"
                subtitle={`Last saved ${formatRelativeTime(serverTimestamp!)}`}
                description={
                  typeof serverData === 'string'
                    ? serverData.substring(0, 100)
                    : JSON.stringify(serverData).substring(0, 100)
                }
                selected={selectedVersion === 'server'}
                onClick={() => setSelectedVersion('server')}
                badge="Server version"
                badgeColor="green"
              />
            )}

            {/* Merge option (if conflict) */}
            {isConflict && (
              <VersionCard
                title="Merge Changes"
                subtitle="Combine local and server versions"
                description="Review and merge changes from both versions"
                selected={selectedVersion === 'merge'}
                onClick={() => {
                  setSelectedVersion('merge');
                  setShowComparison(true);
                }}
                badge="Manual merge"
                badgeColor="purple"
              />
            )}
          </div>
        ) : (
          /* Comparison view for merge */
          <div className="space-y-4">
            <div className="grid grid-cols-2 gap-4">
              <div>
                <h4 className="font-medium text-sm text-gray-700 mb-2">Local Version</h4>
                <pre className="bg-gray-50 p-3 rounded-lg text-xs overflow-auto max-h-64">
                  {JSON.stringify(draft.data, null, 2)}
                </pre>
              </div>
              <div>
                <h4 className="font-medium text-sm text-gray-700 mb-2">Server Version</h4>
                <pre className="bg-gray-50 p-3 rounded-lg text-xs overflow-auto max-h-64">
                  {JSON.stringify(serverData, null, 2)}
                </pre>
              </div>
            </div>
            <div>
              <h4 className="font-medium text-sm text-gray-700 mb-2">Merged Result</h4>
              <textarea
                className="w-full h-32 p-3 border rounded-lg text-sm font-mono"
                value={JSON.stringify(mergedData, null, 2)}
                onChange={(e) => {
                  try {
                    setMergedData(JSON.parse(e.target.value));
                  } catch {
                    // Invalid JSON, ignore
                  }
                }}
              />
            </div>
          </div>
        )}

        {/* Footer note */}
        <p className="text-xs text-gray-400 text-center">
          {isConflict
            ? 'Choosing a version will overwrite the other. This action cannot be undone.'
            : 'Recovering will restore your unsaved changes.'}
        </p>
      </div>
    </Dialog>
  );
}

interface VersionCardProps {
  title: string;
  subtitle: string;
  description: string;
  selected: boolean;
  onClick: () => void;
  badge?: string;
  badgeColor?: 'blue' | 'green' | 'purple' | 'amber';
}

function VersionCard({
  title,
  subtitle,
  description,
  selected,
  onClick,
  badge,
  badgeColor = 'blue',
}: VersionCardProps) {
  const badgeColors = {
    blue: 'bg-blue-100 text-blue-700',
    green: 'bg-green-100 text-green-700',
    purple: 'bg-purple-100 text-purple-700',
    amber: 'bg-amber-100 text-amber-700',
  };

  return (
    <Card
      className={`cursor-pointer transition-all ${
        selected ? 'ring-2 ring-primary border-primary' : 'hover:border-gray-300'
      }`}
      onClick={onClick}
    >
      <CardHeader className="pb-2">
        <div className="flex items-center justify-between">
          <CardTitle className="text-base">{title}</CardTitle>
          {badge && (
            <span className={`text-xs px-2 py-1 rounded-full ${badgeColors[badgeColor]}`}>
              {badge}
            </span>
          )}
        </div>
        <CardDescription className="flex items-center">
          <Clock className="w-3 h-3 mr-1" />
          {subtitle}
        </CardDescription>
      </CardHeader>
      <CardContent>
        <p className="text-sm text-gray-600 line-clamp-2">{description}</p>
      </CardContent>
    </Card>
  );
}

export default DraftRecoveryModal;

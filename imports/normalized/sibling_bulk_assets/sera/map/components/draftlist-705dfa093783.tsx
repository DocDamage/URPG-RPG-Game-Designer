/**
 * Draft List Component
 * 
 * Displays and manages all saved drafts with filtering, sorting,
 * searching, and actions to resume editing or delete drafts
 */

'use client';

import React, { useState, useEffect, useCallback } from 'react';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Badge } from '@/components/ui/badge';
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from '@/components/ui/select';
import {
  AlertTriangle,
  Clock,
  FileText,
  Search,
  Trash2,
  Edit3,
  Download,
  RefreshCw,
  Lock,
  ChevronLeft,
  ChevronRight,
  X,
} from 'lucide-react';
import {
  Draft,
  DraftListItem,
  DraftFilter,
  FormType,
  DraftStats,
} from '../types';
import { draftManager } from '../draftManager';
import {
  formatRelativeTime,
  getFormTypeDisplayName,
  getFormTypeStyles,
  isExpiringSoon,
  exportDrafts,
  downloadDrafts,
} from '../utils';
import { Dialog } from '@/components/ui/dialog';

interface DraftListProps {
  onResumeDraft: (draft: Draft) => void;
  onClose?: () => void;
  initialFilter?: DraftFilter;
}

const ITEMS_PER_PAGE = 10;

export function DraftList({
  onResumeDraft,
  onClose,
  initialFilter = {},
}: DraftListProps) {
  const [drafts, setDrafts] = useState<DraftListItem[]>([]);
  const [filteredDrafts, setFilteredDrafts] = useState<DraftListItem[]>([]);
  const [loading, setLoading] = useState(true);
  const [stats, setStats] = useState<DraftStats | null>(null);
  const [currentPage, setCurrentPage] = useState(1);
  const [deleteConfirmId, setDeleteConfirmId] = useState<string | null>(null);

  // Filter states
  const [searchQuery, setSearchQuery] = useState(initialFilter.searchQuery || '');
  const [formTypeFilter, setFormTypeFilter] = useState<FormType | 'all'>(
    initialFilter.formType || 'all'
  );
  const [conflictFilter, setConflictFilter] = useState<'all' | 'conflict' | 'no-conflict'>('all');
  const [sortBy, setSortBy] = useState<'updated' | 'created' | 'title'>('updated');

  const loadDrafts = useCallback(async () => {
    setLoading(true);
    try {
      await draftManager.initialize();

      const filter: DraftFilter = {
        searchQuery: searchQuery || undefined,
        formType: formTypeFilter === 'all' ? undefined : formTypeFilter,
        hasConflict: conflictFilter === 'all' ? undefined : conflictFilter === 'conflict',
      };

      const [draftList, draftStats] = await Promise.all([
        draftManager.listDrafts(filter),
        draftManager.getStats(),
      ]);

      setDrafts(draftList);
      setStats(draftStats);
    } catch (error) {
      console.error('Failed to load drafts:', error);
    } finally {
      setLoading(false);
    }
  }, [searchQuery, formTypeFilter, conflictFilter]);

  useEffect(() => {
    loadDrafts();
  }, [loadDrafts]);

  useEffect(() => {
    // Sort and filter drafts
    let sorted = [...drafts];

    switch (sortBy) {
      case 'updated':
        sorted.sort((a, b) => b.updatedAt - a.updatedAt);
        break;
      case 'created':
        sorted.sort((a, b) => b.updatedAt - a.updatedAt);
        break;
      case 'title':
        sorted.sort((a, b) => a.title.localeCompare(b.title));
        break;
    }

    setFilteredDrafts(sorted);
    setCurrentPage(1);
  }, [drafts, sortBy]);

  const handleDelete = async (draftId: string) => {
    try {
      await draftManager.deleteDraft(draftId);
      setDeleteConfirmId(null);
      loadDrafts();
    } catch (error) {
      console.error('Failed to delete draft:', error);
    }
  };

  const handleResume = async (draftItem: DraftListItem) => {
    const fullDraft = await draftManager.getDraft(draftItem.id);
    if (fullDraft) {
      onResumeDraft(fullDraft);
    }
  };

  const handleExport = async () => {
    try {
      const filter: DraftFilter = {
        formType: formTypeFilter === 'all' ? undefined : formTypeFilter,
      };
      const jsonData = await exportDrafts(filter);
      downloadDrafts(jsonData);
    } catch (error) {
      console.error('Failed to export drafts:', error);
    }
  };

  const handleCleanup = async () => {
    try {
      const deleted = await draftManager.cleanupExpiredDrafts();
      if (deleted > 0) {
        loadDrafts();
      }
    } catch (error) {
      console.error('Failed to cleanup drafts:', error);
    }
  };

  // Pagination
  const totalPages = Math.ceil(filteredDrafts.length / ITEMS_PER_PAGE);
  const paginatedDrafts = filteredDrafts.slice(
    (currentPage - 1) * ITEMS_PER_PAGE,
    currentPage * ITEMS_PER_PAGE
  );

  const formTypes: FormType[] = ['log', 'incident', 'mar', 'prn', 'behavioral', 'assessment', 'custom'];

  return (
    <Card className="w-full max-w-4xl mx-auto">
      <CardHeader className="border-b">
        <div className="flex items-center justify-between">
          <div>
            <CardTitle className="text-xl flex items-center">
              <FileText className="w-5 h-5 mr-2" />
              Saved Drafts
              {stats && (
                <Badge variant="secondary" className="ml-2">
                  {stats.totalDrafts}
                </Badge>
              )}
            </CardTitle>
            {stats && stats.conflicts > 0 && (
              <p className="text-sm text-amber-600 mt-1">
                <AlertTriangle className="w-4 h-4 inline mr-1" />
                {stats.conflicts} draft{stats.conflicts > 1 ? 's' : ''} with conflicts need attention
              </p>
            )}
          </div>
          <div className="flex items-center space-x-2">
            <Button variant="outline" size="sm" onClick={handleExport}>
              <Download className="w-4 h-4 mr-1" />
              Export
            </Button>
            <Button variant="outline" size="sm" onClick={handleCleanup}>
              <RefreshCw className="w-4 h-4 mr-1" />
              Cleanup
            </Button>
            {onClose && (
              <Button variant="ghost" size="sm" onClick={onClose}>
                <X className="w-4 h-4" />
              </Button>
            )}
          </div>
        </div>
      </CardHeader>

      <CardContent className="p-6">
        {/* Filters */}
        <div className="flex flex-wrap gap-4 mb-6">
          {/* Search */}
          <div className="relative flex-1 min-w-[200px]">
            <Search className="absolute left-3 top-1/2 transform -translate-y-1/2 text-gray-400 w-4 h-4" />
            <Input
              placeholder="Search drafts..."
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
              className="pl-9"
            />
          </div>

          {/* Form Type Filter */}
          <Select value={formTypeFilter} onValueChange={(v) => setFormTypeFilter(v as FormType | 'all')}>
            <SelectTrigger className="w-[160px]">
              <SelectValue placeholder="All Types" />
            </SelectTrigger>
            <SelectContent>
              <SelectItem value="all">All Types</SelectItem>
              {formTypes.map((type) => (
                <SelectItem key={type} value={type}>
                  {getFormTypeDisplayName(type)}
                </SelectItem>
              ))}
            </SelectContent>
          </Select>

          {/* Conflict Filter */}
          <Select value={conflictFilter} onValueChange={(v) => setConflictFilter(v as any)}>
            <SelectTrigger className="w-[160px]">
              <SelectValue placeholder="All Status" />
            </SelectTrigger>
            <SelectContent>
              <SelectItem value="all">All Status</SelectItem>
              <SelectItem value="conflict">Has Conflict</SelectItem>
              <SelectItem value="no-conflict">No Conflict</SelectItem>
            </SelectContent>
          </Select>

          {/* Sort */}
          <Select value={sortBy} onValueChange={(v) => setSortBy(v as any)}>
            <SelectTrigger className="w-[140px]">
              <SelectValue placeholder="Sort by" />
            </SelectTrigger>
            <SelectContent>
              <SelectItem value="updated">Last Updated</SelectItem>
              <SelectItem value="created">Date Created</SelectItem>
              <SelectItem value="title">Title</SelectItem>
            </SelectContent>
          </Select>
        </div>

        {/* Drafts List */}
        {loading ? (
          <div className="flex items-center justify-center py-12">
            <div className="animate-spin rounded-full h-8 w-8 border-b-2 border-primary" />
          </div>
        ) : paginatedDrafts.length === 0 ? (
          <div className="text-center py-12 text-gray-500">
            <FileText className="w-12 h-12 mx-auto mb-4 text-gray-300" />
            <p>No drafts found</p>
            {(searchQuery || formTypeFilter !== 'all') && (
              <p className="text-sm mt-1">Try adjusting your filters</p>
            )}
          </div>
        ) : (
          <div className="space-y-3">
            {paginatedDrafts.map((draft) => (
              <DraftListItemCard
                key={draft.id}
                draft={draft}
                onResume={() => handleResume(draft)}
                onDelete={() => setDeleteConfirmId(draft.id)}
              />
            ))}
          </div>
        )}

        {/* Pagination */}
        {totalPages > 1 && (
          <div className="flex items-center justify-between mt-6 pt-4 border-t">
            <p className="text-sm text-gray-500">
              Showing {(currentPage - 1) * ITEMS_PER_PAGE + 1} to{' '}
              {Math.min(currentPage * ITEMS_PER_PAGE, filteredDrafts.length)} of{' '}
              {filteredDrafts.length} drafts
            </p>
            <div className="flex items-center space-x-2">
              <Button
                variant="outline"
                size="sm"
                onClick={() => setCurrentPage((p) => Math.max(1, p - 1))}
                disabled={currentPage === 1}
              >
                <ChevronLeft className="w-4 h-4" />
              </Button>
              <span className="text-sm">
                Page {currentPage} of {totalPages}
              </span>
              <Button
                variant="outline"
                size="sm"
                onClick={() => setCurrentPage((p) => Math.min(totalPages, p + 1))}
                disabled={currentPage === totalPages}
              >
                <ChevronRight className="w-4 h-4" />
              </Button>
            </div>
          </div>
        )}
      </CardContent>

      {/* Delete Confirmation Dialog */}
      <Dialog
        isOpen={!!deleteConfirmId}
        onClose={() => setDeleteConfirmId(null)}
        title="Delete Draft?"
        size="sm"
        footer={
          <>
            <Button variant="outline" onClick={() => setDeleteConfirmId(null)}>
              Cancel
            </Button>
            <Button
              variant="destructive"
              onClick={() => deleteConfirmId && handleDelete(deleteConfirmId)}
            >
              Delete
            </Button>
          </>
        }
      >
        <p className="text-sm text-gray-600">
          This action cannot be undone. The draft will be permanently deleted.
        </p>
      </Dialog>
    </Card>
  );
}

interface DraftListItemCardProps {
  draft: DraftListItem;
  onResume: () => void;
  onDelete: () => void;
}

function DraftListItemCard({ draft, onResume, onDelete }: DraftListItemCardProps) {
  const styles = getFormTypeStyles(draft.formType);
  const expiringSoon = isExpiringSoon(draft);

  return (
    <div
      className={`flex items-center justify-between p-4 rounded-lg border hover:border-gray-300 transition-colors ${
        draft.hasConflict ? 'bg-amber-50 border-amber-200' : 'bg-white'
      }`}
    >
      <div className="flex items-start space-x-3 flex-1 min-w-0">
        <div className={`p-2 rounded-lg ${styles.bgColor}`}>
          <FileText className={`w-5 h-5 ${styles.color}`} />
        </div>
        <div className="flex-1 min-w-0">
          <div className="flex items-center space-x-2">
            <h4 className="font-medium truncate">{draft.title}</h4>
            {draft.hasConflict && (
              <Badge variant="destructive" className="text-xs">
                <AlertTriangle className="w-3 h-3 mr-1" />
                Conflict
              </Badge>
            )}
            {draft.isEncrypted && (
              <Badge variant="secondary" className="text-xs">
                <Lock className="w-3 h-3 mr-1" />
                Encrypted
              </Badge>
            )}
            {expiringSoon && (
              <Badge variant="outline" className="text-xs text-amber-600 border-amber-300">
                <Clock className="w-3 h-3 mr-1" />
                Expiring
              </Badge>
            )}
          </div>
          <p className="text-sm text-gray-500 mt-1 flex items-center flex-wrap gap-2">
            <span>{getFormTypeDisplayName(draft.formType)}</span>
            {draft.individualName && (
              <>
                <span>•</span>
                <span>{draft.individualName}</span>
              </>
            )}
            <span>•</span>
            <span>{formatRelativeTime(draft.updatedAt)}</span>
          </p>
        </div>
      </div>
      <div className="flex items-center space-x-2 ml-4">
        <Button variant="ghost" size="sm" onClick={onResume}>
          <Edit3 className="w-4 h-4" />
        </Button>
        <Button variant="ghost" size="sm" onClick={onDelete} className="text-red-600 hover:text-red-700">
          <Trash2 className="w-4 h-4" />
        </Button>
      </div>
    </div>
  );
}

export default DraftList;

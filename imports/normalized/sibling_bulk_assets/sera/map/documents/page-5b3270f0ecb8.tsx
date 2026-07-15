"use client";

import React, { useState, useEffect } from 'react';
import { Folder, File, Search, Upload, Grid, List, AlertCircle, MoreVertical } from 'lucide-react';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Tabs, TabsContent, TabsList, TabsTrigger } from '@/components/ui/tabs';
import { Alert, AlertDescription } from '@/components/ui/alert';
import {
  DropdownMenu,
  DropdownMenuContent,
  DropdownMenuItem,
  DropdownMenuTrigger,
} from '@/components/ui/dropdown-menu';
import documentsApi, { Document, DocumentFolder } from '@/lib/api/documents';

export default function DocumentsPage() {
  const [view, setView] = useState<'grid' | 'list'>('grid');
  const [documents, setDocuments] = useState<Document[]>([]);
  const [folders, setFolders] = useState<DocumentFolder[]>([]);
  const [currentFolder, setCurrentFolder] = useState<string | undefined>();
  const [searchQuery, setSearchQuery] = useState('');
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  const fetchDocuments = async () => {
    setLoading(true);
    setError(null);
    try {
      const [docsRes, foldersRes] = await Promise.all([
        documentsApi.getDocuments(currentFolder),
        documentsApi.getFolders(),
      ]);

      if (docsRes.data) {
        setDocuments(docsRes.data);
      }
      if (foldersRes.data) {
        setFolders(foldersRes.data);
      }
    } catch (err) {
      console.error('Failed to fetch documents:', err);
      setError('Failed to load documents. Please try again.');
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchDocuments();
  }, [currentFolder]);

  const handleSearch = async () => {
    if (!searchQuery.trim()) {
      fetchDocuments();
      return;
    }
    
    setLoading(true);
    try {
      const response = await documentsApi.searchDocuments(searchQuery);
      if (response.data) {
        setDocuments(response.data);
      }
    } catch (err) {
      console.error('Failed to search documents:', err);
    } finally {
      setLoading(false);
    }
  };

  const formatFileSize = (bytes: number): string => {
    if (bytes === 0) return '0 Bytes';
    const k = 1024;
    const sizes = ['Bytes', 'KB', 'MB', 'GB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
  };

  const getFileIcon = (mimeType: string) => {
    if (mimeType.startsWith('image/')) {
      return <div className="w-10 h-10 bg-purple-100 rounded-lg flex items-center justify-center"><File className="h-5 w-5 text-purple-600" /></div>;
    }
    if (mimeType.includes('pdf')) {
      return <div className="w-10 h-10 bg-red-100 rounded-lg flex items-center justify-center"><File className="h-5 w-5 text-red-600" /></div>;
    }
    if (mimeType.includes('word') || mimeType.includes('document')) {
      return <div className="w-10 h-10 bg-blue-100 rounded-lg flex items-center justify-center"><File className="h-5 w-5 text-blue-600" /></div>;
    }
    if (mimeType.includes('excel') || mimeType.includes('sheet')) {
      return <div className="w-10 h-10 bg-green-100 rounded-lg flex items-center justify-center"><File className="h-5 w-5 text-green-600" /></div>;
    }
    return <div className="w-10 h-10 bg-gray-100 rounded-lg flex items-center justify-center"><File className="h-5 w-5 text-gray-600" /></div>;
  };

  return (
    <div className="container mx-auto py-6 space-y-6">
      {/* Header */}
      <div className="flex flex-col sm:flex-row sm:items-center sm:justify-between gap-4">
        <div>
          <h1 className="text-3xl font-bold tracking-tight">Documents</h1>
          <p className="text-muted-foreground">
            Manage files, templates, and document workflows
          </p>
        </div>
        <div className="flex items-center gap-2">
          <Button>
            <Upload className="mr-2 h-4 w-4" />
            Upload
          </Button>
        </div>
      </div>

      {/* Error Alert */}
      {error && (
        <Alert variant="destructive">
          <AlertCircle className="h-4 w-4" />
          <AlertDescription>{error}</AlertDescription>
        </Alert>
      )}

      {/* Toolbar */}
      <div className="flex flex-col sm:flex-row gap-4">
        <div className="flex-1 flex gap-2">
          <div className="relative flex-1 max-w-md">
            <Search className="absolute left-3 top-1/2 -translate-y-1/2 h-4 w-4 text-muted-foreground" />
            <Input
              placeholder="Search documents..."
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
              onKeyDown={(e) => e.key === 'Enter' && handleSearch()}
              className="pl-9"
            />
          </div>
          <Button variant="outline" onClick={handleSearch}>
            Search
          </Button>
        </div>
        <div className="flex items-center gap-2">
          <Tabs value={view} onValueChange={(v) => setView(v as 'grid' | 'list')}>
            <TabsList>
              <TabsTrigger value="grid">
                <Grid className="h-4 w-4" />
              </TabsTrigger>
              <TabsTrigger value="list">
                <List className="h-4 w-4" />
              </TabsTrigger>
            </TabsList>
          </Tabs>
        </div>
      </div>

      {/* Folder Breadcrumb */}
      <div className="flex items-center gap-2 text-sm">
        <Button
          variant="ghost"
          size="sm"
          className={!currentFolder ? 'font-medium' : ''}
          onClick={() => setCurrentFolder(undefined)}
        >
          <Folder className="mr-1 h-4 w-4" />
          All Documents
        </Button>
        {currentFolder && (
          <>
            <span className="text-muted-foreground">/</span>
            <span className="font-medium">
              {folders.find((f) => f.id === currentFolder)?.name}
            </span>
          </>
        )}
      </div>

      {/* Folders */}
      {folders.length > 0 && !currentFolder && (
        <div className="space-y-3">
          <h3 className="text-sm font-medium text-muted-foreground">Folders</h3>
          <div className="grid grid-cols-2 sm:grid-cols-3 md:grid-cols-4 lg:grid-cols-6 gap-4">
            {folders.map((folder) => (
              <button
                key={folder.id}
                onClick={() => setCurrentFolder(folder.id)}
                className="flex flex-col items-center p-4 rounded-lg border hover:bg-muted/50 transition-colors"
              >
                <Folder className="h-10 w-10 text-yellow-500 mb-2" />
                <span className="text-sm font-medium text-center truncate w-full">
                  {folder.name}
                </span>
              </button>
            ))}
          </div>
        </div>
      )}

      {/* Documents */}
      {loading ? (
        <div className="text-center py-12 text-muted-foreground">Loading documents...</div>
      ) : documents.length === 0 ? (
        <div className="text-center py-12 bg-muted/50 rounded-lg">
          <File className="mx-auto h-12 w-12 text-muted-foreground" />
          <h3 className="mt-4 text-lg font-medium">No Documents</h3>
          <p className="text-muted-foreground">
            Upload files or create folders to get started
          </p>
        </div>
      ) : view === 'grid' ? (
        <div className="grid grid-cols-2 sm:grid-cols-3 md:grid-cols-4 lg:grid-cols-5 gap-4">
          {documents.map((doc) => (
            <div
              key={doc.id}
              className="group relative flex flex-col p-4 rounded-lg border hover:shadow-md transition-shadow"
            >
              <div className="absolute top-2 right-2 opacity-0 group-hover:opacity-100 transition-opacity">
                <DropdownMenu>
                  <DropdownMenuTrigger asChild>
                    <Button variant="ghost" size="icon" className="h-8 w-8">
                      <MoreVertical className="h-4 w-4" />
                    </Button>
                  </DropdownMenuTrigger>
                  <DropdownMenuContent align="end">
                    <DropdownMenuItem>Download</DropdownMenuItem>
                    <DropdownMenuItem>View Details</DropdownMenuItem>
                    <DropdownMenuItem>Share</DropdownMenuItem>
                    <DropdownMenuItem className="text-red-600">Delete</DropdownMenuItem>
                  </DropdownMenuContent>
                </DropdownMenu>
              </div>
              <div className="flex justify-center mb-3">
                {getFileIcon(doc.mimeType)}
              </div>
              <p className="text-sm font-medium text-center truncate" title={doc.name}>
                {doc.name}
              </p>
              <p className="text-xs text-muted-foreground text-center mt-1">
                {formatFileSize(doc.sizeBytes)}
              </p>
              {doc.version > 1 && (
                <p className="text-xs text-muted-foreground text-center">
                  v{doc.version}
                </p>
              )}
            </div>
          ))}
        </div>
      ) : (
        <div className="border rounded-lg divide-y">
          {documents.map((doc) => (
            <div
              key={doc.id}
              className="flex items-center gap-4 p-4 hover:bg-muted/50 transition-colors"
            >
              {getFileIcon(doc.mimeType)}
              <div className="flex-1 min-w-0">
                <p className="font-medium truncate">{doc.name}</p>
                <div className="flex items-center gap-4 text-sm text-muted-foreground">
                  <span>{formatFileSize(doc.sizeBytes)}</span>
                  <span>{new Date(doc.createdAt).toLocaleDateString()}</span>
                  {doc.version > 1 && <span>v{doc.version}</span>}
                </div>
              </div>
              <DropdownMenu>
                <DropdownMenuTrigger asChild>
                  <Button variant="ghost" size="icon">
                    <MoreVertical className="h-4 w-4" />
                  </Button>
                </DropdownMenuTrigger>
                <DropdownMenuContent align="end">
                  <DropdownMenuItem>Download</DropdownMenuItem>
                  <DropdownMenuItem>View Details</DropdownMenuItem>
                  <DropdownMenuItem>View Versions</DropdownMenuItem>
                  <DropdownMenuItem>Share</DropdownMenuItem>
                  <DropdownMenuItem className="text-red-600">Delete</DropdownMenuItem>
                </DropdownMenuContent>
              </DropdownMenu>
            </div>
          ))}
        </div>
      )}
    </div>
  );
}

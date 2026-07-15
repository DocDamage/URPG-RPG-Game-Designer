'use client';

import React, { useState, useEffect } from 'react';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { getDocuments, getFolders, searchDocuments, Document, DocumentFolder, uploadDocument, createFolder } from '@/lib/api/documents';
import { useToast } from '@/components/ui/use-toast';
import { 
  Folder, 
  FileText, 
  Search, 
  ChevronLeft, 
  Upload, 
  Plus,
  FolderOpen,
  MoreVertical,
  Download,
  Trash2,
  File
} from 'lucide-react';
import {
  DropdownMenu,
  DropdownMenuContent,
  DropdownMenuItem,
  DropdownMenuTrigger,
} from '@/components/ui/dropdown-menu';
import {
  Dialog,
  DialogContent,
  DialogDescription,
  DialogFooter,
  DialogHeader,
  DialogTitle,
  DialogTrigger,
} from '@/components/ui/dialog';
import { Label } from '@/components/ui/label';

export function DocumentExplorer() {
  const [documents, setDocuments] = useState<Document[]>([]);
  const [folders, setFolders] = useState<DocumentFolder[]>([]);
  const [currentFolder, setCurrentFolder] = useState<string | null>(null);
  const [searchQuery, setSearchQuery] = useState('');
  const [loading, setLoading] = useState(true);
  const [newFolderName, setNewFolderName] = useState('');
  const [isCreateFolderOpen, setIsCreateFolderOpen] = useState(false);
  const { toast } = useToast();

  useEffect(() => {
    loadData();
  }, [currentFolder]);

  const loadData = async () => {
    try {
      setLoading(true);
      const [docsResponse, foldersResponse] = await Promise.all([
        getDocuments({ folderId: currentFolder || undefined }),
        getFolders(currentFolder || undefined),
      ]);

      if (docsResponse.success) {
        setDocuments(docsResponse.data);
      }

      if (foldersResponse.success) {
        setFolders(foldersResponse.data);
      }
    } catch (error) {
      toast({
        title: 'Error',
        description: 'Failed to load documents',
        variant: 'destructive',
      });
    } finally {
      setLoading(false);
    }
  };

  const handleSearch = async () => {
    if (!searchQuery.trim()) {
      loadData();
      return;
    }

    try {
      setLoading(true);
      const response = await searchDocuments(searchQuery);
      if (response.success) {
        setDocuments(response.data);
        setFolders([]);
      }
    } catch (error) {
      toast({
        title: 'Error',
        description: 'Search failed',
        variant: 'destructive',
      });
    } finally {
      setLoading(false);
    }
  };

  const handleFileUpload = async (event: React.ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    if (!file) return;

    try {
      await uploadDocument(file, currentFolder || undefined);
      toast({
        title: 'Success',
        description: 'File uploaded successfully',
      });
      loadData();
    } catch (error) {
      toast({
        title: 'Error',
        description: 'Failed to upload file',
        variant: 'destructive',
      });
    }
  };

  const handleCreateFolder = async () => {
    if (!newFolderName.trim()) return;

    try {
      await createFolder(newFolderName, currentFolder || undefined);
      toast({
        title: 'Success',
        description: 'Folder created successfully',
      });
      setNewFolderName('');
      setIsCreateFolderOpen(false);
      loadData();
    } catch (error) {
      toast({
        title: 'Error',
        description: 'Failed to create folder',
        variant: 'destructive',
      });
    }
  };

  const getFileIcon = (fileType: string) => {
    if (fileType.includes('pdf')) return <File className="h-5 w-5 text-red-500" />;
    if (fileType.includes('image')) return <File className="h-5 w-5 text-blue-500" />;
    if (fileType.includes('word') || fileType.includes('document')) return <File className="h-5 w-5 text-blue-700" />;
    if (fileType.includes('excel') || fileType.includes('sheet')) return <File className="h-5 w-5 text-green-600" />;
    return <FileText className="h-5 w-5 text-gray-500" />;
  };

  const formatFileSize = (bytes: number) => {
    if (bytes < 1024) return bytes + ' B';
    if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + ' KB';
    return (bytes / (1024 * 1024)).toFixed(1) + ' MB';
  };

  if (loading) {
    return (
      <Card>
        <CardContent className="p-6">
          <div className="flex items-center justify-center h-64">
            <div className="animate-spin rounded-full h-8 w-8 border-b-2 border-primary"></div>
          </div>
        </CardContent>
      </Card>
    );
  }

  return (
    <Card className="h-[calc(100vh-200px)]">
      <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-4">
        <CardTitle className="flex items-center gap-2">
          <FolderOpen className="h-5 w-5" />
          Document Explorer
        </CardTitle>
        <div className="flex items-center gap-2">
          <div className="relative">
            <Search className="absolute left-2 top-2.5 h-4 w-4 text-muted-foreground" />
            <Input
              placeholder="Search documents..."
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
              onKeyDown={(e) => e.key === 'Enter' && handleSearch()}
              className="pl-8 w-[250px]"
            />
          </div>
          
          <Dialog open={isCreateFolderOpen} onOpenChange={setIsCreateFolderOpen}>
            <DialogTrigger asChild>
              <Button variant="outline" size="icon">
                <Plus className="h-4 w-4" />
              </Button>
            </DialogTrigger>
            <DialogContent>
              <DialogHeader>
                <DialogTitle>Create New Folder</DialogTitle>
                <DialogDescription>
                  Enter a name for the new folder
                </DialogDescription>
              </DialogHeader>
              <div className="space-y-4 py-4">
                <div className="space-y-2">
                  <Label htmlFor="folder-name">Folder Name</Label>
                  <Input
                    id="folder-name"
                    value={newFolderName}
                    onChange={(e) => setNewFolderName(e.target.value)}
                    placeholder="My Folder"
                  />
                </div>
              </div>
              <DialogFooter>
                <Button variant="outline" onClick={() => setIsCreateFolderOpen(false)}>
                  Cancel
                </Button>
                <Button onClick={handleCreateFolder}>Create</Button>
              </DialogFooter>
            </DialogContent>
          </Dialog>

          <Button variant="outline" size="icon" asChild>
            <label className="cursor-pointer">
              <Upload className="h-4 w-4" />
              <input
                type="file"
                className="hidden"
                onChange={handleFileUpload}
              />
            </label>
          </Button>
        </div>
      </CardHeader>
      <CardContent>
        {currentFolder && (
          <Button
            variant="ghost"
            size="sm"
            className="mb-4"
            onClick={() => setCurrentFolder(null)}
          >
            <ChevronLeft className="h-4 w-4 mr-1" />
            Back
          </Button>
        )}

        <div className="space-y-2">
          {/* Folders */}
          {folders.map((folder) => (
            <div
              key={folder.id}
              className="flex items-center justify-between p-3 rounded-lg hover:bg-muted cursor-pointer group"
              onClick={() => setCurrentFolder(folder.id)}
            >
              <div className="flex items-center gap-3">
                <Folder className="h-5 w-5 text-yellow-500" />
                <div>
                  <p className="font-medium">{folder.name}</p>
                  <p className="text-sm text-muted-foreground">Folder</p>
                </div>
              </div>
            </div>
          ))}

          {/* Documents */}
          {documents.map((doc) => (
            <div
              key={doc.id}
              className="flex items-center justify-between p-3 rounded-lg hover:bg-muted group"
            >
              <div className="flex items-center gap-3">
                {getFileIcon(doc.fileType)}
                <div>
                  <p className="font-medium">{doc.name}</p>
                  <p className="text-sm text-muted-foreground">
                    {formatFileSize(doc.fileSize)} • Version {doc.version} • {new Date(doc.uploadedAt).toLocaleDateString()}
                  </p>
                </div>
              </div>
              <DropdownMenu>
                <DropdownMenuTrigger asChild>
                  <Button variant="ghost" size="icon">
                    <MoreVertical className="h-4 w-4" />
                  </Button>
                </DropdownMenuTrigger>
                <DropdownMenuContent align="end">
                  <DropdownMenuItem>
                    <Download className="h-4 w-4 mr-2" />
                    Download
                  </DropdownMenuItem>
                  <DropdownMenuItem className="text-red-600">
                    <Trash2 className="h-4 w-4 mr-2" />
                    Delete
                  </DropdownMenuItem>
                </DropdownMenuContent>
              </DropdownMenu>
            </div>
          ))}

          {folders.length === 0 && documents.length === 0 && (
            <div className="text-center py-12 text-muted-foreground">
              <FolderOpen className="h-12 w-12 mx-auto mb-3 opacity-50" />
              <p>No documents found</p>
              <p className="text-sm">Upload files or create folders to get started</p>
            </div>
          )}
        </div>
      </CardContent>
    </Card>
  );
}

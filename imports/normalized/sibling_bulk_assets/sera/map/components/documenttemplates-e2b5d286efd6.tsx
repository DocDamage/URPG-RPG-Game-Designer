'use client';

import React, { useState, useEffect } from 'react';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Label } from '@/components/ui/label';
import { Textarea } from '@/components/ui/textarea';
import { getTemplates, renderTemplate, DocumentTemplate } from '@/lib/api/documents';
import { useToast } from '@/components/ui/use-toast';
import { FileText, Plus, Eye, Download, FilePlus } from 'lucide-react';
import {
  Dialog,
  DialogContent,
  DialogDescription,
  DialogFooter,
  DialogHeader,
  DialogTitle,
  DialogTrigger,
} from '@/components/ui/dialog';
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from '@/components/ui/select';

const templateCategories = [
  'Care Plans',
  'Incident Reports',
  'Medical Forms',
  'Administrative',
  'Training',
];

export function DocumentTemplates() {
  const [templates, setTemplates] = useState<DocumentTemplate[]>([]);
  const [selectedTemplate, setSelectedTemplate] = useState<DocumentTemplate | null>(null);
  const [variables, setVariables] = useState<Record<string, string>>({});
  const [preview, setPreview] = useState('');
  const [loading, setLoading] = useState(true);
  const [isPreviewOpen, setIsPreviewOpen] = useState(false);
  const { toast } = useToast();

  useEffect(() => {
    loadTemplates();
  }, []);

  const loadTemplates = async () => {
    try {
      setLoading(true);
      const response = await getTemplates();
      if (response.success) {
        setTemplates(response.data);
      }
    } catch (error) {
      toast({
        title: 'Error',
        description: 'Failed to load templates',
        variant: 'destructive',
      });
    } finally {
      setLoading(false);
    }
  };

  const handlePreview = async () => {
    if (!selectedTemplate) return;

    try {
      const response = await renderTemplate(selectedTemplate.id, variables);
      if (response.success) {
        setPreview(response.data.content);
        setIsPreviewOpen(true);
      }
    } catch (error) {
      toast({
        title: 'Error',
        description: 'Failed to render template',
        variant: 'destructive',
      });
    }
  };

  const handleSelectTemplate = (template: DocumentTemplate) => {
    setSelectedTemplate(template);
    // Initialize variables
    const initialVars: Record<string, string> = {};
    template.variables.forEach((v) => {
      initialVars[v] = '';
    });
    setVariables(initialVars);
  };

  const getCategoryIcon = (category: string) => {
    switch (category) {
      case 'Care Plans':
        return <FileText className="h-5 w-5 text-blue-500" />;
      case 'Incident Reports':
        return <FileText className="h-5 w-5 text-red-500" />;
      case 'Medical Forms':
        return <FileText className="h-5 w-5 text-green-500" />;
      default:
        return <FileText className="h-5 w-5 text-gray-500" />;
    }
  };

  if (loading) {
    return (
      <Card>
        <CardContent className="p-6">
          <div className="flex items-center justify-center h-32">
            <div className="animate-spin rounded-full h-8 w-8 border-b-2 border-primary"></div>
          </div>
        </CardContent>
      </Card>
    );
  }

  return (
    <div className="space-y-6">
      <Card>
        <CardHeader className="flex flex-row items-center justify-between">
          <CardTitle className="flex items-center gap-2">
            <FilePlus className="h-5 w-5" />
            Document Templates
          </CardTitle>
          <Button>
            <Plus className="h-4 w-4 mr-2" />
            Create Template
          </Button>
        </CardHeader>
        <CardContent>
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
            {templates.map((template) => (
              <Card
                key={template.id}
                className={`cursor-pointer transition-all ${
                  selectedTemplate?.id === template.id ? 'ring-2 ring-primary' : ''
                }`}
                onClick={() => handleSelectTemplate(template)}
              >
                <CardContent className="p-4">
                  <div className="flex items-start gap-3">
                    {getCategoryIcon(template.category)}
                    <div className="flex-1 min-w-0">
                      <h4 className="font-medium truncate">{template.name}</h4>
                      <p className="text-sm text-muted-foreground">
                        {template.category}
                      </p>
                      <p className="text-xs text-muted-foreground mt-1">
                        {template.variables.length} variables
                      </p>
                    </div>
                  </div>
                </CardContent>
              </Card>
            ))}
          </div>

          {templates.length === 0 && (
            <div className="text-center py-8 text-muted-foreground">
              <FileText className="h-12 w-12 mx-auto mb-3 opacity-50" />
              <p>No templates available</p>
            </div>
          )}
        </CardContent>
      </Card>

      {selectedTemplate && (
        <Card>
          <CardHeader>
            <CardTitle>Fill Template: {selectedTemplate.name}</CardTitle>
          </CardHeader>
          <CardContent>
            <div className="space-y-4">
              {selectedTemplate.variables.map((variable) => (
                <div key={variable} className="space-y-2">
                  <Label htmlFor={variable}>
                    {variable.replace(/_/g, ' ').replace(/\b\w/g, (l) => l.toUpperCase())}
                  </Label>
                  <Input
                    id={variable}
                    value={variables[variable] || ''}
                    onChange={(e) =>
                      setVariables({ ...variables, [variable]: e.target.value })
                    }
                    placeholder={`Enter ${variable.replace(/_/g, ' ')}`}
                  />
                </div>
              ))}

              <div className="flex gap-2 pt-4">
                <Button onClick={handlePreview} variant="outline">
                  <Eye className="h-4 w-4 mr-2" />
                  Preview
                </Button>
                <Button>
                  <Download className="h-4 w-4 mr-2" />
                  Generate Document
                </Button>
              </div>
            </div>
          </CardContent>
        </Card>
      )}

      <Dialog open={isPreviewOpen} onOpenChange={setIsPreviewOpen}>
        <DialogContent className="max-w-3xl max-h-[80vh]">
          <DialogHeader>
            <DialogTitle>Document Preview</DialogTitle>
            <DialogDescription>
              Preview of the generated document
            </DialogDescription>
          </DialogHeader>
          <div className="overflow-auto max-h-[500px] border rounded-lg p-4 bg-white">
            <pre className="whitespace-pre-wrap font-mono text-sm">{preview}</pre>
          </div>
          <DialogFooter>
            <Button variant="outline" onClick={() => setIsPreviewOpen(false)}>
              Close
            </Button>
            <Button>
              <Download className="h-4 w-4 mr-2" />
              Download
            </Button>
          </DialogFooter>
        </DialogContent>
      </Dialog>
    </div>
  );
}

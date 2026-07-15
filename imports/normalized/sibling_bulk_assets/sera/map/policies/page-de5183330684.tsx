'use client';

import { useState, useEffect, useCallback } from 'react';
import {
  FileText,
  Plus,
  Search,
  Filter,
  MoreHorizontal,
  Edit,
  Eye,
  History,
  Users,
  CheckCircle,
  Clock,
  AlertCircle,
  Download,
  Trash2,
  Calendar,
  ChevronRight,
  Check,
  X,
  Loader2,
  Copy,
  FilePlus,
  Send
} from 'lucide-react';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Card, CardContent, CardHeader, CardTitle, CardDescription } from '@/components/ui/card';
import { Badge } from '@/components/ui/badge';
import { Dialog, DialogContent, DialogHeader, DialogTitle, DialogFooter } from '@/components/ui/dialog';
import { Tabs, TabsContent, TabsList, TabsTrigger } from '@/components/ui/tabs';
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from '@/components/ui/select';
import { Label } from '@/components/ui/label';
import { Textarea } from '@/components/ui/textarea';
import { Alert, AlertDescription } from '@/components/ui/alert';
import { Skeleton } from '@/components/ui/skeleton';
import { Progress } from '@/components/ui/progress';
import { Switch } from '@/components/ui/switch';
import { useToast } from '@/components/ui/use-toast';
import {
  policiesApi,
  Policy,
  PolicyTemplate,
  PolicyVersion,
  PolicyAcknowledgment,
  DistributionTracking,
  PolicyReview
} from '@/lib/api/policies';

const POLICY_CATEGORIES = [
  { value: 'hipaa_privacy', label: 'HIPAA Privacy' },
  { value: 'hipaa_security', label: 'HIPAA Security' },
  { value: 'code_of_conduct', label: 'Code of Conduct' },
  { value: 'hr', label: 'HR Policy' },
  { value: 'clinical', label: 'Clinical' },
  { value: 'safety', label: 'Safety' },
  { value: 'it_security', label: 'IT Security' },
  { value: 'incident_response', label: 'Incident Response' },
  { value: 'business_continuity', label: 'Business Continuity' },
  { value: 'other', label: 'Other' }
];

export default function PoliciesPage() {
  const { toast } = useToast();
  const [policies, setPolicies] = useState<Policy[]>([]);
  const [templates, setTemplates] = useState<PolicyTemplate[]>([]);
  const [loading, setLoading] = useState(true);
  const [searchQuery, setSearchQuery] = useState('');
  const [categoryFilter, setCategoryFilter] = useState('all');
  const [statusFilter, setStatusFilter] = useState('all');
  const [selectedPolicy, setSelectedPolicy] = useState<Policy | null>(null);
  const [selectedTemplate, setSelectedTemplate] = useState<PolicyTemplate | null>(null);
  const [isCreateModalOpen, setIsCreateModalOpen] = useState(false);
  const [isEditorModalOpen, setIsEditorModalOpen] = useState(false);
  const [isVersionsModalOpen, setIsVersionsModalOpen] = useState(false);
  const [isDistributionModalOpen, setIsDistributionModalOpen] = useState(false);
  const [isScheduleModalOpen, setIsScheduleModalOpen] = useState(false);
  const [versions, setVersions] = useState<PolicyVersion[]>([]);
  const [acknowledgments, setAcknowledgments] = useState<PolicyAcknowledgment[]>([]);
  const [distribution, setDistribution] = useState<DistributionTracking | null>(null);
  const [reviews, setReviews] = useState<PolicyReview[]>([]);
  const [activeTab, setActiveTab] = useState('policies');

  // Form states
  const [policyForm, setPolicyForm] = useState({
    title: '',
    category: 'other',
    content: '',
    effectiveDate: '',
    reviewDate: '',
    requiresAcknowledgment: true,
    acknowledgmentDueDays: 30,
    distributionScope: 'organization' as const,
    distributionTargets: [] as string[]
  });

  const [editorContent, setEditorContent] = useState('');
  const [saving, setSaving] = useState(false);

  const fetchPolicies = useCallback(async () => {
    try {
      setLoading(true);
      const filters: any = {};
      if (categoryFilter !== 'all') filters.category = categoryFilter;
      if (statusFilter !== 'all') filters.status = statusFilter;
      if (searchQuery) filters.search = searchQuery;

      const [policiesData, templatesData] = await Promise.all([
        policiesApi.getPolicies(filters),
        policiesApi.getTemplates()
      ]);
      setPolicies(policiesData);
      setTemplates(templatesData);
    } catch (err) {
      toast({
        title: 'Error',
        description: 'Failed to load policies',
        variant: 'destructive'
      });
    } finally {
      setLoading(false);
    }
  }, [categoryFilter, statusFilter, searchQuery, toast]);

  useEffect(() => {
    fetchPolicies();
  }, [fetchPolicies]);

  const fetchPolicyDetails = async (policy: Policy) => {
    try {
      const [versionsData, acksData, distData, reviewsData] = await Promise.all([
        policiesApi.getPolicyVersions(policy.id),
        policiesApi.getAcknowledgments(policy.id),
        policiesApi.getDistributionTracking(policy.id),
        policiesApi.getReviews(policy.id)
      ]);
      setVersions(versionsData);
      setAcknowledgments(acksData);
      setDistribution(distData);
      setReviews(reviewsData);
    } catch (err) {
      console.error('Failed to load policy details:', err);
    }
  };

  const handleCreateFromTemplate = async (templateId: string) => {
    try {
      const template = templates.find(t => t.id === templateId);
      if (!template) return;

      const policy = await policiesApi.createFromTemplate(templateId, {
        title: template.title,
        category: template.category
      });

      toast({
        title: 'Success',
        description: 'Policy created from template'
      });

      setIsCreateModalOpen(false);
      fetchPolicies();
      openEditor(policy);
    } catch (err) {
      toast({
        title: 'Error',
        description: 'Failed to create policy',
        variant: 'destructive'
      });
    }
  };

  const handleCreateNew = async (e: React.FormEvent) => {
    e.preventDefault();
    try {
      setSaving(true);
      const policy = await policiesApi.createPolicy({
        title: policyForm.title,
        category: policyForm.category as any,
        content: policyForm.content,
        effectiveDate: policyForm.effectiveDate,
        reviewDate: policyForm.reviewDate || undefined,
        requiresAcknowledgment: policyForm.requiresAcknowledgment,
        acknowledgmentDueDays: policyForm.acknowledgmentDueDays,
        distributionScope: policyForm.distributionScope,
        distributionTargets: policyForm.distributionTargets
      });

      toast({
        title: 'Success',
        description: 'Policy created successfully'
      });

      setIsCreateModalOpen(false);
      fetchPolicies();
      openEditor(policy);
    } catch (err) {
      toast({
        title: 'Error',
        description: 'Failed to create policy',
        variant: 'destructive'
      });
    } finally {
      setSaving(false);
    }
  };

  const handleSaveContent = async () => {
    if (!selectedPolicy) return;

    try {
      setSaving(true);
      await policiesApi.createVersion(selectedPolicy.id, editorContent, 'Content update');
      toast({
        title: 'Success',
        description: 'Policy updated'
      });
      fetchPolicies();
    } catch (err) {
      toast({
        title: 'Error',
        description: 'Failed to save policy',
        variant: 'destructive'
      });
    } finally {
      setSaving(false);
    }
  };

  const handlePublish = async () => {
    if (!selectedPolicy) return;

    try {
      await policiesApi.publishPolicy(selectedPolicy.id);
      toast({
        title: 'Success',
        description: 'Policy published'
      });
      fetchPolicies();
    } catch (err) {
      toast({
        title: 'Error',
        description: 'Failed to publish policy',
        variant: 'destructive'
      });
    }
  };

  const openEditor = async (policy: Policy) => {
    setSelectedPolicy(policy);
    setEditorContent(policy.content);
    await fetchPolicyDetails(policy);
    setIsEditorModalOpen(true);
  };

  const getStatusBadge = (status: string) => {
    const variants: Record<string, 'default' | 'success' | 'warning' | 'danger' | 'info'> = {
      draft: 'default',
      pending_review: 'warning',
      approved: 'info',
      active: 'success',
      under_review: 'warning',
      superseded: 'danger',
      retired: 'danger'
    };
    return (
      <Badge variant={variants[status] || 'default'}>
        {status.replace('_', ' ').replace(/\b\w/g, l => l.toUpperCase())}
      </Badge>
    );
  };

  const getAcknowledgmentRate = () => {
    if (!distribution) return 0;
    return distribution.acknowledgmentRate;
  };

  if (loading) {
    return (
      <div className="container mx-auto p-6 space-y-6">
        <Skeleton className="h-10 w-64" />
        <Skeleton className="h-96" />
      </div>
    );
  }

  return (
    <div className="container mx-auto p-6 space-y-6">
      {/* Header */}
      <div className="flex flex-col md:flex-row md:items-center md:justify-between gap-4">
        <div>
          <h1 className="text-3xl font-bold flex items-center gap-2">
            <FileText className="w-8 h-8 text-blue-600" />
            Policy Management
          </h1>
          <p className="text-gray-500 mt-1">Create, manage, and distribute policies</p>
        </div>
        <div className="flex items-center gap-2">
          <Button onClick={() => setIsCreateModalOpen(true)} className="gap-2">
            <Plus className="w-4 h-4" />
            New Policy
          </Button>
        </div>
      </div>

      {/* Filters */}
      <Card>
        <CardContent className="p-4">
          <div className="flex flex-col md:flex-row gap-4">
            <div className="relative flex-1">
              <Search className="absolute left-3 top-1/2 -translate-y-1/2 w-4 h-4 text-gray-400" />
              <Input
                placeholder="Search policies..."
                value={searchQuery}
                onChange={(e) => setSearchQuery(e.target.value)}
                className="pl-10"
              />
            </div>
            <Select value={categoryFilter} onValueChange={setCategoryFilter}>
              <SelectTrigger className="w-40">
                <Filter className="w-4 h-4 mr-2" />
                <SelectValue placeholder="Category" />
              </SelectTrigger>
              <SelectContent>
                <SelectItem value="all">All Categories</SelectItem>
                {POLICY_CATEGORIES.map(cat => (
                  <SelectItem key={cat.value} value={cat.value}>{cat.label}</SelectItem>
                ))}
              </SelectContent>
            </Select>
            <Select value={statusFilter} onValueChange={setStatusFilter}>
              <SelectTrigger className="w-40">
                <SelectValue placeholder="Status" />
              </SelectTrigger>
              <SelectContent>
                <SelectItem value="all">All Status</SelectItem>
                <SelectItem value="draft">Draft</SelectItem>
                <SelectItem value="pending_review">Pending Review</SelectItem>
                <SelectItem value="active">Active</SelectItem>
                <SelectItem value="retired">Retired</SelectItem>
              </SelectContent>
            </Select>
          </div>
        </CardContent>
      </Card>

      {/* Policies List */}
      <div className="space-y-4">
        {policies.map((policy) => (
          <Card key={policy.id} className="hover:shadow-md transition-shadow">
            <CardContent className="p-4">
              <div className="flex items-start justify-between">
                <div className="flex items-start gap-4">
                  <div className="p-2 bg-blue-100 dark:bg-blue-900 rounded-lg">
                    <FileText className="w-5 h-5 text-blue-600 dark:text-blue-300" />
                  </div>
                  <div>
                    <div className="flex items-center gap-2 mb-1">
                      <h3 className="font-medium">{policy.title}</h3>
                      {getStatusBadge(policy.status)}
                      {policy.requiresAcknowledgment && (
                        <Badge variant="outline" className="gap-1">
                          <CheckCircle className="w-3 h-3" />
                          Ack Required
                        </Badge>
                      )}
                    </div>
                    <p className="text-sm text-gray-500 mb-2">
                      {POLICY_CATEGORIES.find(c => c.value === policy.category)?.label || policy.category}
                      {' • '}Version {policy.version}
                      {' • '}Owner: {policy.ownerName}
                    </p>
                    <div className="flex items-center gap-4 text-sm text-gray-500">
                      <span className="flex items-center gap-1">
                        <Calendar className="w-3 h-3" />
                        Effective: {new Date(policy.effectiveDate).toLocaleDateString()}
                      </span>
                      {policy.reviewDate && (
                        <span className="flex items-center gap-1">
                          <Clock className="w-3 h-3" />
                          Review: {new Date(policy.reviewDate).toLocaleDateString()}
                        </span>
                      )}
                    </div>
                  </div>
                </div>
                <div className="flex items-center gap-2">
                  <Button variant="ghost" size="sm" onClick={() => openEditor(policy)}>
                    <Edit className="w-4 h-4" />
                  </Button>
                  {policy.status === 'active' && (
                    <Button variant="ghost" size="sm" onClick={() => {
                      setSelectedPolicy(policy);
                      setIsDistributionModalOpen(true);
                    }}>
                      <Users className="w-4 h-4" />
                    </Button>
                  )}
                  <Button variant="ghost" size="sm">
                    <MoreHorizontal className="w-4 h-4" />
                  </Button>
                </div>
              </div>
            </CardContent>
          </Card>
        ))}
        {policies.length === 0 && (
          <div className="text-center py-12 text-gray-500">
            <FileText className="w-12 h-12 mx-auto mb-4 text-gray-300" />
            <p>No policies found</p>
          </div>
        )}
      </div>

      {/* Create Policy Modal */}
      <Dialog open={isCreateModalOpen} onOpenChange={setIsCreateModalOpen}>
        <DialogContent className="max-w-2xl max-h-[90vh] overflow-y-auto">
          <DialogHeader>
            <DialogTitle>Create New Policy</DialogTitle>
          </DialogHeader>
          <Tabs defaultValue="template">
            <TabsList className="w-full">
              <TabsTrigger value="template">From Template</TabsTrigger>
              <TabsTrigger value="scratch">From Scratch</TabsTrigger>
            </TabsList>

            <TabsContent value="template" className="space-y-4 mt-4">
              <div className="grid gap-4">
                {templates.filter(t => t.isActive).map((template) => (
                  <Card
                    key={template.id}
                    className="cursor-pointer hover:border-blue-500 transition-colors"
                    onClick={() => handleCreateFromTemplate(template.id)}
                  >
                    <CardContent className="p-4">
                      <div className="flex items-center justify-between">
                        <div>
                          <p className="font-medium">{template.title}</p>
                          <p className="text-sm text-gray-500">{template.description}</p>
                          <div className="flex items-center gap-2 mt-2">
                            <Badge variant="outline">
                              {POLICY_CATEGORIES.find(c => c.value === template.category)?.label}
                            </Badge>
                            <span className="text-sm text-gray-500">
                              Review every {template.defaultReviewIntervalMonths} months
                            </span>
                          </div>
                        </div>
                        <ChevronRight className="w-5 h-5 text-gray-400" />
                      </div>
                    </CardContent>
                  </Card>
                ))}
              </div>
            </TabsContent>

            <TabsContent value="scratch" className="space-y-4 mt-4">
              <form onSubmit={handleCreateNew} className="space-y-4">
                <div>
                  <Label htmlFor="title">Policy Title *</Label>
                  <Input
                    id="title"
                    value={policyForm.title}
                    onChange={(e) => setPolicyForm({ ...policyForm, title: e.target.value })}
                    placeholder="Enter policy title"
                    required
                  />
                </div>
                <div>
                  <Label htmlFor="category">Category *</Label>
                  <Select
                    value={policyForm.category}
                    onValueChange={(v) => setPolicyForm({ ...policyForm, category: v })}
                  >
                    <SelectTrigger>
                      <SelectValue />
                    </SelectTrigger>
                    <SelectContent>
                      {POLICY_CATEGORIES.map(cat => (
                        <SelectItem key={cat.value} value={cat.value}>{cat.label}</SelectItem>
                      ))}
                    </SelectContent>
                  </Select>
                </div>
                <div>
                  <Label htmlFor="content">Content</Label>
                  <Textarea
                    id="content"
                    value={policyForm.content}
                    onChange={(e) => setPolicyForm({ ...policyForm, content: e.target.value })}
                    placeholder="Enter policy content..."
                    rows={6}
                  />
                </div>
                <div className="grid grid-cols-2 gap-4">
                  <div>
                    <Label htmlFor="effectiveDate">Effective Date *</Label>
                    <Input
                      id="effectiveDate"
                      type="date"
                      value={policyForm.effectiveDate}
                      onChange={(e) => setPolicyForm({ ...policyForm, effectiveDate: e.target.value })}
                      required
                    />
                  </div>
                  <div>
                    <Label htmlFor="reviewDate">Review Date</Label>
                    <Input
                      id="reviewDate"
                      type="date"
                      value={policyForm.reviewDate}
                      onChange={(e) => setPolicyForm({ ...policyForm, reviewDate: e.target.value })}
                    />
                  </div>
                </div>
                <div className="flex items-center gap-4">
                  <div className="flex items-center gap-2">
                    <Switch
                      checked={policyForm.requiresAcknowledgment}
                      onCheckedChange={(v) => setPolicyForm({ ...policyForm, requiresAcknowledgment: v })}
                      id="ack-required"
                    />
                    <Label htmlFor="ack-required">Require Acknowledgment</Label>
                  </div>
                  {policyForm.requiresAcknowledgment && (
                    <div className="flex items-center gap-2">
                      <Label>Due in</Label>
                      <Input
                        type="number"
                        value={policyForm.acknowledgmentDueDays}
                        onChange={(e) => setPolicyForm({ ...policyForm, acknowledgmentDueDays: parseInt(e.target.value) })}
                        className="w-20"
                      />
                      <span>days</span>
                    </div>
                  )}
                </div>
                <DialogFooter>
                  <Button type="button" variant="outline" onClick={() => setIsCreateModalOpen(false)}>
                    Cancel
                  </Button>
                  <Button type="submit" disabled={saving}>
                    {saving ? <Loader2 className="w-4 h-4 animate-spin mr-2" /> : null}
                    Create Policy
                  </Button>
                </DialogFooter>
              </form>
            </TabsContent>
          </Tabs>
        </DialogContent>
      </Dialog>

      {/* Policy Editor Modal */}
      <Dialog open={isEditorModalOpen} onOpenChange={setIsEditorModalOpen}>
        <DialogContent className="max-w-4xl max-h-[90vh] overflow-y-auto">
          <DialogHeader>
            <div className="flex items-center justify-between">
              <DialogTitle className="flex items-center gap-2">
                <FileText className="w-5 h-5" />
                {selectedPolicy?.title}
              </DialogTitle>
              <div className="flex items-center gap-2">
                {selectedPolicy?.status === 'draft' && (
                  <Button size="sm" onClick={handlePublish} className="gap-1">
                    <Send className="w-4 h-4" />
                    Publish
                  </Button>
                )}
                <Button size="sm" onClick={handleSaveContent} disabled={saving}>
                  {saving ? <Loader2 className="w-4 h-4 animate-spin" /> : <Check className="w-4 h-4" />}
                  Save
                </Button>
              </div>
            </div>
          </DialogHeader>

          {selectedPolicy && (
            <Tabs defaultValue="editor" className="mt-4">
              <TabsList>
                <TabsTrigger value="editor">Editor</TabsTrigger>
                <TabsTrigger value="versions">Versions ({versions.length})</TabsTrigger>
                <TabsTrigger value="distribution">Distribution</TabsTrigger>
                <TabsTrigger value="reviews">Reviews</TabsTrigger>
              </TabsList>

              <TabsContent value="editor" className="mt-4 space-y-4">
                <div className="flex items-center gap-2 text-sm text-gray-500">
                  <span>Category: {POLICY_CATEGORIES.find(c => c.value === selectedPolicy.category)?.label}</span>
                  <span>•</span>
                  <span>Version: {selectedPolicy.version}</span>
                  <span>•</span>
                  <span>Status: {getStatusBadge(selectedPolicy.status)}</span>
                </div>
                <Textarea
                  value={editorContent}
                  onChange={(e) => setEditorContent(e.target.value)}
                  className="min-h-[400px] font-mono text-sm"
                  placeholder="Enter policy content..."
                />
              </TabsContent>

              <TabsContent value="versions" className="mt-4 space-y-3">
                {versions.map((version) => (
                  <Card key={version.id}>
                    <CardContent className="p-4">
                      <div className="flex items-center justify-between">
                        <div>
                          <p className="font-medium">Version {version.version}</p>
                          <p className="text-sm text-gray-500">{version.changeSummary}</p>
                          <p className="text-sm text-gray-500">
                            By {version.createdBy} on {new Date(version.createdAt).toLocaleDateString()}
                          </p>
                        </div>
                        <Button variant="outline" size="sm">
                          <Eye className="w-4 h-4 mr-2" />
                          View
                        </Button>
                      </div>
                    </CardContent>
                  </Card>
                ))}
              </TabsContent>

              <TabsContent value="distribution" className="mt-4">
                {distribution && (
                  <div className="space-y-6">
                    <div className="flex items-center gap-4">
                      <div className="flex-1">
                        <div className="flex items-center justify-between mb-2">
                          <span className="font-medium">Acknowledgment Rate</span>
                          <span className="text-2xl font-bold">{getAcknowledgmentRate()}%</span>
                        </div>
                        <Progress value={getAcknowledgmentRate()} className="h-3" />
                      </div>
                    </div>
                    <div className="grid grid-cols-3 gap-4">
                      <Card>
                        <CardContent className="p-4 text-center">
                          <p className="text-2xl font-bold text-green-600">{distribution.acknowledged}</p>
                          <p className="text-sm text-gray-500">Acknowledged</p>
                        </CardContent>
                      </Card>
                      <Card>
                        <CardContent className="p-4 text-center">
                          <p className="text-2xl font-bold text-yellow-600">{distribution.pending}</p>
                          <p className="text-sm text-gray-500">Pending</p>
                        </CardContent>
                      </Card>
                      <Card>
                        <CardContent className="p-4 text-center">
                          <p className="text-2xl font-bold text-red-600">{distribution.overdue}</p>
                          <p className="text-sm text-gray-500">Overdue</p>
                        </CardContent>
                      </Card>
                    </div>
                    <Card>
                      <CardHeader>
                        <CardTitle className="text-sm">Individual Acknowledgments</CardTitle>
                      </CardHeader>
                      <CardContent>
                        <div className="space-y-2">
                          {acknowledgments.slice(0, 10).map((ack) => (
                            <div key={ack.id} className="flex items-center justify-between p-2 bg-gray-50 dark:bg-gray-800 rounded">
                              <div className="flex items-center gap-2">
                                {ack.status === 'acknowledged' ? (
                                  <CheckCircle className="w-4 h-4 text-green-500" />
                                ) : ack.status === 'overdue' ? (
                                  <AlertCircle className="w-4 h-4 text-red-500" />
                                ) : (
                                  <Clock className="w-4 h-4 text-yellow-500" />
                                )}
                                <span className="text-sm">{ack.userName}</span>
                                <Badge variant="outline" size="sm">{ack.userRole}</Badge>
                              </div>
                              <span className="text-sm text-gray-500">
                                {ack.acknowledgedAt ? new Date(ack.acknowledgedAt).toLocaleDateString() : 'Pending'}
                              </span>
                            </div>
                          ))}
                        </div>
                      </CardContent>
                    </Card>
                  </div>
                )}
              </TabsContent>

              <TabsContent value="reviews" className="mt-4">
                <div className="space-y-3">
                  {reviews.map((review) => (
                    <Card key={review.id}>
                      <CardContent className="p-4">
                        <div className="flex items-center justify-between">
                          <div>
                            <p className="font-medium capitalize">{review.reviewType} Review</p>
                            <p className="text-sm text-gray-500">
                              Scheduled: {new Date(review.scheduledDate).toLocaleDateString()}
                            </p>
                            <p className="text-sm text-gray-500">
                              Reviewer: {review.reviewerName}
                            </p>
                          </div>
                          <Badge variant={review.status === 'completed' ? 'success' : 'warning'}>
                            {review.status.replace('_', ' ')}
                          </Badge>
                        </div>
                      </CardContent>
                    </Card>
                  ))}
                  <Button
                    variant="outline"
                    className="w-full"
                    onClick={() => setIsScheduleModalOpen(true)}
                  >
                    <Calendar className="w-4 h-4 mr-2" />
                    Schedule Review
                  </Button>
                </div>
              </TabsContent>
            </Tabs>
          )}
        </DialogContent>
      </Dialog>
    </div>
  );
}

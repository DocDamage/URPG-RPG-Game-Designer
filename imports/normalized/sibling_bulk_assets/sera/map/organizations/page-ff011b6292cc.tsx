'use client';

import { useState, useEffect, useCallback } from 'react';
import { useRouter } from 'next/navigation';
import {
  Building2,
  Plus,
  Search,
  Filter,
  MoreHorizontal,
  MapPin,
  Users,
  Settings,
  ChevronRight,
  ChevronDown,
  TreePine,
  Edit,
  Trash2,
  Eye,
  AlertCircle,
  CheckCircle2,
  XCircle,
  Loader2
} from 'lucide-react';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Badge } from '@/components/ui/badge';
import { Dialog, DialogContent, DialogHeader, DialogTitle, DialogFooter } from '@/components/ui/dialog';
import { Tabs, TabsContent, TabsList, TabsTrigger } from '@/components/ui/tabs';
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from '@/components/ui/select';
import { Label } from '@/components/ui/label';
import { Textarea } from '@/components/ui/textarea';
import { Alert, AlertDescription } from '@/components/ui/alert';
import { Skeleton } from '@/components/ui/skeleton';
import { organizationsApi, Organization, Location, OrganizationHierarchy } from '@/lib/api/organizations';
import { useToast } from '@/components/ui/use-toast';

export default function OrganizationsPage() {
  const router = useRouter();
  const { toast } = useToast();
  const [organizations, setOrganizations] = useState<Organization[]>([]);
  const [locations, setLocations] = useState<Location[]>([]);
  const [hierarchy, setHierarchy] = useState<OrganizationHierarchy | null>(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [searchQuery, setSearchQuery] = useState('');
  const [statusFilter, setStatusFilter] = useState<string>('all');
  const [typeFilter, setTypeFilter] = useState<string>('all');
  const [selectedOrg, setSelectedOrg] = useState<Organization | null>(null);
  const [isCreateModalOpen, setIsCreateModalOpen] = useState(false);
  const [isDetailsModalOpen, setIsDetailsModalOpen] = useState(false);
  const [activeTab, setActiveTab] = useState('list');

  // Form state
  const [formData, setFormData] = useState({
    name: '',
    slug: '',
    description: '',
    type: 'provider',
    contact: {
      email: '',
      phone: '',
      address: {
        street1: '',
        street2: '',
        city: '',
        state: '',
        zipCode: '',
        country: 'US'
      }
    }
  });

  const fetchOrganizations = useCallback(async () => {
    try {
      setLoading(true);
      setError(null);
      const filters: any = {};
      if (statusFilter !== 'all') filters.status = statusFilter;
      if (typeFilter !== 'all') filters.type = typeFilter;
      if (searchQuery) filters.search = searchQuery;
      
      const data = await organizationsApi.getOrganizations(filters);
      setOrganizations(data);
    } catch (err) {
      setError('Failed to load organizations');
      toast({
        title: 'Error',
        description: 'Failed to load organizations. Please try again.',
        variant: 'destructive'
      });
    } finally {
      setLoading(false);
    }
  }, [statusFilter, typeFilter, searchQuery, toast]);

  const fetchHierarchy = useCallback(async () => {
    try {
      const data = await organizationsApi.getOrganizationHierarchy();
      setHierarchy(data);
    } catch (err) {
      console.error('Failed to load hierarchy:', err);
    }
  }, []);

  const fetchLocations = useCallback(async (orgId: string) => {
    try {
      const data = await organizationsApi.getLocations(orgId);
      setLocations(data);
    } catch (err) {
      console.error('Failed to load locations:', err);
    }
  }, []);

  useEffect(() => {
    fetchOrganizations();
    fetchHierarchy();
  }, [fetchOrganizations, fetchHierarchy]);

  const handleCreateSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    try {
      await organizationsApi.createOrganization({
        name: formData.name,
        slug: formData.slug,
        description: formData.description,
        type: formData.type as any,
        contact: formData.contact
      });
      
      toast({
        title: 'Success',
        description: 'Organization created successfully'
      });
      
      setIsCreateModalOpen(false);
      fetchOrganizations();
      fetchHierarchy();
      
      // Reset form
      setFormData({
        name: '',
        slug: '',
        description: '',
        type: 'provider',
        contact: {
          email: '',
          phone: '',
          address: {
            street1: '',
            street2: '',
            city: '',
            state: '',
            zipCode: '',
            country: 'US'
          }
        }
      });
    } catch (err) {
      toast({
        title: 'Error',
        description: 'Failed to create organization',
        variant: 'destructive'
      });
    }
  };

  const handleDelete = async (id: string) => {
    if (!confirm('Are you sure you want to delete this organization?')) return;
    
    try {
      await organizationsApi.deleteOrganization(id);
      toast({
        title: 'Success',
        description: 'Organization deleted successfully'
      });
      fetchOrganizations();
      fetchHierarchy();
    } catch (err) {
      toast({
        title: 'Error',
        description: 'Failed to delete organization',
        variant: 'destructive'
      });
    }
  };

  const openDetails = async (org: Organization) => {
    setSelectedOrg(org);
    await fetchLocations(org.id);
    setIsDetailsModalOpen(true);
  };

  const getStatusBadge = (status: string) => {
    const variants: Record<string, { variant: 'default' | 'success' | 'warning' | 'danger'; icon: any }> = {
      active: { variant: 'success', icon: CheckCircle2 },
      inactive: { variant: 'warning', icon: XCircle },
      suspended: { variant: 'danger', icon: AlertCircle }
    };
    const config = variants[status] || variants.inactive;
    const Icon = config.icon;
    
    return (
      <Badge variant={config.variant} className="gap-1">
        <Icon className="w-3 h-3" />
        {status.charAt(0).toUpperCase() + status.slice(1)}
      </Badge>
    );
  };

  const getTypeBadge = (type: string) => {
    const labels: Record<string, string> = {
      provider: 'Provider',
      agency: 'Agency',
      enterprise: 'Enterprise'
    };
    return <Badge variant="info">{labels[type] || type}</Badge>;
  };

  const renderHierarchyNode = (node: OrganizationHierarchy, depth = 0) => (
    <div key={node.id} className={`${depth > 0 ? 'ml-6 border-l-2 border-gray-200 pl-4' : ''}`}>
      <div className="flex items-center gap-3 p-3 rounded-lg hover:bg-gray-50 dark:hover:bg-gray-800 transition-colors">
        <div className="p-2 bg-blue-100 dark:bg-blue-900 rounded-lg">
          <Building2 className="w-5 h-5 text-blue-600 dark:text-blue-300" />
        </div>
        <div className="flex-1">
          <h4 className="font-medium">{node.name}</h4>
          <p className="text-sm text-gray-500">{node.type} • {node.locations.length} locations</p>
        </div>
        <div className="flex items-center gap-2">
          {getStatusBadge(node.status)}
          <Button variant="ghost" size="sm" onClick={() => openDetails(organizations.find(o => o.id === node.id) || {} as Organization)}>
            <Eye className="w-4 h-4" />
          </Button>
        </div>
      </div>
      {node.locations.length > 0 && (
        <div className="ml-6 border-l-2 border-gray-200 pl-4">
          {node.locations.map(loc => (
            <div key={loc.id} className="flex items-center gap-3 p-2 rounded-lg hover:bg-gray-50 dark:hover:bg-gray-800">
              <MapPin className="w-4 h-4 text-gray-400" />
              <span className="text-sm">{loc.name}</span>
              <Badge variant={loc.status === 'active' ? 'success' : 'warning'} size="sm">
                {loc.status}
              </Badge>
            </div>
          ))}
        </div>
      )}
      {node.children.map(child => renderHierarchyNode(child, depth + 1))}
    </div>
  );

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
            <Building2 className="w-8 h-8 text-blue-600" />
            Organization Management
          </h1>
          <p className="text-gray-500 mt-1">Manage organizations, locations, and hierarchy</p>
        </div>
        <Button onClick={() => setIsCreateModalOpen(true)} className="gap-2">
          <Plus className="w-4 h-4" />
          Create Organization
        </Button>
      </div>

      {/* Error Alert */}
      {error && (
        <Alert variant="destructive">
          <AlertCircle className="w-4 h-4" />
          <AlertDescription>{error}</AlertDescription>
        </Alert>
      )}

      {/* Filters */}
      <Card>
        <CardContent className="p-4">
          <div className="flex flex-col md:flex-row gap-4">
            <div className="relative flex-1">
              <Search className="absolute left-3 top-1/2 -translate-y-1/2 w-4 h-4 text-gray-400" />
              <Input
                placeholder="Search organizations..."
                value={searchQuery}
                onChange={(e) => setSearchQuery(e.target.value)}
                className="pl-10"
              />
            </div>
            <Select value={statusFilter} onValueChange={setStatusFilter}>
              <SelectTrigger className="w-40">
                <Filter className="w-4 h-4 mr-2" />
                <SelectValue placeholder="Status" />
              </SelectTrigger>
              <SelectContent>
                <SelectItem value="all">All Status</SelectItem>
                <SelectItem value="active">Active</SelectItem>
                <SelectItem value="inactive">Inactive</SelectItem>
                <SelectItem value="suspended">Suspended</SelectItem>
              </SelectContent>
            </Select>
            <Select value={typeFilter} onValueChange={setTypeFilter}>
              <SelectTrigger className="w-40">
                <SelectValue placeholder="Type" />
              </SelectTrigger>
              <SelectContent>
                <SelectItem value="all">All Types</SelectItem>
                <SelectItem value="provider">Provider</SelectItem>
                <SelectItem value="agency">Agency</SelectItem>
                <SelectItem value="enterprise">Enterprise</SelectItem>
              </SelectContent>
            </Select>
          </div>
        </CardContent>
      </Card>

      {/* Main Content */}
      <Tabs value={activeTab} onValueChange={setActiveTab}>
        <TabsList>
          <TabsTrigger value="list" className="gap-2">
            <Building2 className="w-4 h-4" />
            List View
          </TabsTrigger>
          <TabsTrigger value="hierarchy" className="gap-2">
            <TreePine className="w-4 h-4" />
            Hierarchy
          </TabsTrigger>
        </TabsList>

        <TabsContent value="list" className="mt-6">
          <Card>
            <CardContent className="p-0">
              <div className="overflow-x-auto">
                <table className="w-full">
                  <thead className="bg-gray-50 dark:bg-gray-800 border-b">
                    <tr>
                      <th className="text-left p-4 font-medium">Organization</th>
                      <th className="text-left p-4 font-medium">Type</th>
                      <th className="text-left p-4 font-medium">Status</th>
                      <th className="text-left p-4 font-medium">Contact</th>
                      <th className="text-left p-4 font-medium">Created</th>
                      <th className="text-right p-4 font-medium">Actions</th>
                    </tr>
                  </thead>
                  <tbody>
                    {organizations.map((org) => (
                      <tr key={org.id} className="border-b hover:bg-gray-50 dark:hover:bg-gray-800">
                        <td className="p-4">
                          <div className="flex items-center gap-3">
                            <div className="p-2 bg-blue-100 dark:bg-blue-900 rounded-lg">
                              <Building2 className="w-4 h-4 text-blue-600 dark:text-blue-300" />
                            </div>
                            <div>
                              <p className="font-medium">{org.name}</p>
                              <p className="text-sm text-gray-500">{org.slug}</p>
                            </div>
                          </div>
                        </td>
                        <td className="p-4">{getTypeBadge(org.type)}</td>
                        <td className="p-4">{getStatusBadge(org.status)}</td>
                        <td className="p-4">
                          <div className="text-sm">
                            <p>{org.contact.email}</p>
                            {org.contact.phone && <p className="text-gray-500">{org.contact.phone}</p>}
                          </div>
                        </td>
                        <td className="p-4 text-sm text-gray-500">
                          {new Date(org.createdAt).toLocaleDateString()}
                        </td>
                        <td className="p-4 text-right">
                          <div className="flex items-center justify-end gap-2">
                            <Button variant="ghost" size="sm" onClick={() => openDetails(org)}>
                              <Eye className="w-4 h-4" />
                            </Button>
                            <Button variant="ghost" size="sm">
                              <Edit className="w-4 h-4" />
                            </Button>
                            <Button variant="ghost" size="sm" onClick={() => handleDelete(org.id)}>
                              <Trash2 className="w-4 h-4 text-red-500" />
                            </Button>
                          </div>
                        </td>
                      </tr>
                    ))}
                  </tbody>
                </table>
              </div>
              {organizations.length === 0 && (
                <div className="p-8 text-center text-gray-500">
                  <Building2 className="w-12 h-12 mx-auto mb-4 text-gray-300" />
                  <p>No organizations found</p>
                </div>
              )}
            </CardContent>
          </Card>
        </TabsContent>

        <TabsContent value="hierarchy" className="mt-6">
          <Card>
            <CardHeader>
              <CardTitle>Organization Hierarchy</CardTitle>
            </CardHeader>
            <CardContent>
              {hierarchy ? (
                renderHierarchyNode(hierarchy)
              ) : (
                <div className="p-8 text-center text-gray-500">
                  <TreePine className="w-12 h-12 mx-auto mb-4 text-gray-300" />
                  <p>No hierarchy data available</p>
                </div>
              )}
            </CardContent>
          </Card>
        </TabsContent>
      </Tabs>

      {/* Create Modal */}
      <Dialog open={isCreateModalOpen} onOpenChange={setIsCreateModalOpen}>
        <DialogContent className="max-w-2xl max-h-[90vh] overflow-y-auto">
          <DialogHeader>
            <DialogTitle>Create New Organization</DialogTitle>
          </DialogHeader>
          <form onSubmit={handleCreateSubmit} className="space-y-6">
            <div className="space-y-4">
              <div>
                <Label htmlFor="name">Organization Name *</Label>
                <Input
                  id="name"
                  value={formData.name}
                  onChange={(e) => setFormData({ ...formData, name: e.target.value })}
                  placeholder="Enter organization name"
                  required
                />
              </div>
              <div>
                <Label htmlFor="slug">Slug *</Label>
                <Input
                  id="slug"
                  value={formData.slug}
                  onChange={(e) => setFormData({ ...formData, slug: e.target.value })}
                  placeholder="organization-slug"
                  required
                />
              </div>
              <div>
                <Label htmlFor="type">Type *</Label>
                <Select
                  value={formData.type}
                  onValueChange={(value) => setFormData({ ...formData, type: value })}
                >
                  <SelectTrigger>
                    <SelectValue />
                  </SelectTrigger>
                  <SelectContent>
                    <SelectItem value="provider">Provider</SelectItem>
                    <SelectItem value="agency">Agency</SelectItem>
                    <SelectItem value="enterprise">Enterprise</SelectItem>
                  </SelectContent>
                </Select>
              </div>
              <div>
                <Label htmlFor="description">Description</Label>
                <Textarea
                  id="description"
                  value={formData.description}
                  onChange={(e) => setFormData({ ...formData, description: e.target.value })}
                  placeholder="Enter organization description"
                  rows={3}
                />
              </div>
              <div className="grid grid-cols-2 gap-4">
                <div>
                  <Label htmlFor="email">Email *</Label>
                  <Input
                    id="email"
                    type="email"
                    value={formData.contact.email}
                    onChange={(e) => setFormData({
                      ...formData,
                      contact: { ...formData.contact, email: e.target.value }
                    })}
                    placeholder="contact@organization.com"
                    required
                  />
                </div>
                <div>
                  <Label htmlFor="phone">Phone</Label>
                  <Input
                    id="phone"
                    value={formData.contact.phone}
                    onChange={(e) => setFormData({
                      ...formData,
                      contact: { ...formData.contact, phone: e.target.value }
                    })}
                    placeholder="(555) 123-4567"
                  />
                </div>
              </div>
            </div>
            <DialogFooter>
              <Button type="button" variant="outline" onClick={() => setIsCreateModalOpen(false)}>
                Cancel
              </Button>
              <Button type="submit">Create Organization</Button>
            </DialogFooter>
          </form>
        </DialogContent>
      </Dialog>

      {/* Details Modal */}
      <Dialog open={isDetailsModalOpen} onOpenChange={setIsDetailsModalOpen}>
        <DialogContent className="max-w-4xl max-h-[90vh] overflow-y-auto">
          <DialogHeader>
            <DialogTitle className="flex items-center gap-2">
              <Building2 className="w-6 h-6" />
              {selectedOrg?.name}
            </DialogTitle>
          </DialogHeader>
          {selectedOrg && (
            <Tabs defaultValue="details">
              <TabsList className="w-full">
                <TabsTrigger value="details">Details</TabsTrigger>
                <TabsTrigger value="locations">
                  Locations ({locations.length})
                </TabsTrigger>
                <TabsTrigger value="settings">Settings</TabsTrigger>
              </TabsList>

              <TabsContent value="details" className="space-y-4 mt-4">
                <div className="grid grid-cols-2 gap-4">
                  <Card>
                    <CardHeader>
                      <CardTitle className="text-sm font-medium">Basic Information</CardTitle>
                    </CardHeader>
                    <CardContent className="space-y-2">
                      <div className="flex justify-between">
                        <span className="text-gray-500">Type</span>
                        <Badge variant="info">{selectedOrg.type}</Badge>
                      </div>
                      <div className="flex justify-between">
                        <span className="text-gray-500">Status</span>
                        {getStatusBadge(selectedOrg.status)}
                      </div>
                      <div className="flex justify-between">
                        <span className="text-gray-500">Created</span>
                        <span>{new Date(selectedOrg.createdAt).toLocaleDateString()}</span>
                      </div>
                    </CardContent>
                  </Card>
                  <Card>
                    <CardHeader>
                      <CardTitle className="text-sm font-medium">Contact Information</CardTitle>
                    </CardHeader>
                    <CardContent className="space-y-2">
                      <div>
                        <span className="text-gray-500">Email: </span>
                        <span>{selectedOrg.contact.email}</span>
                      </div>
                      {selectedOrg.contact.phone && (
                        <div>
                          <span className="text-gray-500">Phone: </span>
                          <span>{selectedOrg.contact.phone}</span>
                        </div>
                      )}
                    </CardContent>
                  </Card>
                </div>
                {selectedOrg.description && (
                  <Card>
                    <CardHeader>
                      <CardTitle className="text-sm font-medium">Description</CardTitle>
                    </CardHeader>
                    <CardContent>
                      <p>{selectedOrg.description}</p>
                    </CardContent>
                  </Card>
                )}
              </TabsContent>

              <TabsContent value="locations" className="mt-4">
                <div className="space-y-4">
                  {locations.map((location) => (
                    <Card key={location.id}>
                      <CardContent className="p-4">
                        <div className="flex items-center justify-between">
                          <div className="flex items-center gap-3">
                            <MapPin className="w-5 h-5 text-blue-600" />
                            <div>
                              <p className="font-medium">{location.name}</p>
                              <p className="text-sm text-gray-500">
                                {location.address.city}, {location.address.state}
                              </p>
                            </div>
                          </div>
                          <Badge variant={location.status === 'active' ? 'success' : 'warning'}>
                            {location.status}
                          </Badge>
                        </div>
                      </CardContent>
                    </Card>
                  ))}
                  {locations.length === 0 && (
                    <div className="text-center py-8 text-gray-500">
                      <MapPin className="w-12 h-12 mx-auto mb-4 text-gray-300" />
                      <p>No locations found</p>
                    </div>
                  )}
                </div>
              </TabsContent>

              <TabsContent value="settings" className="mt-4">
                <Card>
                  <CardHeader>
                    <CardTitle className="text-sm font-medium">Organization Settings</CardTitle>
                  </CardHeader>
                  <CardContent className="space-y-4">
                    <div className="grid grid-cols-2 gap-4">
                      <div>
                        <Label>Timezone</Label>
                        <p className="text-sm">{selectedOrg.settings.timezone}</p>
                      </div>
                      <div>
                        <Label>Locale</Label>
                        <p className="text-sm">{selectedOrg.settings.locale}</p>
                      </div>
                      <div>
                        <Label>Date Format</Label>
                        <p className="text-sm">{selectedOrg.settings.dateFormat}</p>
                      </div>
                      <div>
                        <Label>Time Format</Label>
                        <p className="text-sm">{selectedOrg.settings.timeFormat}</p>
                      </div>
                    </div>
                    <div>
                      <Label>Features</Label>
                      <div className="flex flex-wrap gap-2 mt-2">
                        {selectedOrg.settings.features.map((feature) => (
                          <Badge key={feature} variant="secondary">{feature}</Badge>
                        ))}
                      </div>
                    </div>
                    <div>
                      <Label>Limits</Label>
                      <div className="grid grid-cols-3 gap-4 mt-2">
                        <Card>
                          <CardContent className="p-4 text-center">
                            <Users className="w-6 h-6 mx-auto mb-2 text-blue-600" />
                            <p className="text-2xl font-bold">{selectedOrg.settings.limits.maxUsers}</p>
                            <p className="text-sm text-gray-500">Max Users</p>
                          </CardContent>
                        </Card>
                        <Card>
                          <CardContent className="p-4 text-center">
                            <MapPin className="w-6 h-6 mx-auto mb-2 text-green-600" />
                            <p className="text-2xl font-bold">{selectedOrg.settings.limits.maxLocations}</p>
                            <p className="text-sm text-gray-500">Max Locations</p>
                          </CardContent>
                        </Card>
                        <Card>
                          <CardContent className="p-4 text-center">
                            <Settings className="w-6 h-6 mx-auto mb-2 text-purple-600" />
                            <p className="text-2xl font-bold">{selectedOrg.settings.limits.storageGb}GB</p>
                            <p className="text-sm text-gray-500">Storage</p>
                          </CardContent>
                        </Card>
                      </div>
                    </div>
                  </CardContent>
                </Card>
              </TabsContent>
            </Tabs>
          )}
        </DialogContent>
      </Dialog>
    </div>
  );
}

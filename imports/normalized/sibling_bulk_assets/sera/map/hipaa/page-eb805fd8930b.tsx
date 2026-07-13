'use client';

import { useState, useEffect, useCallback } from 'react';
import {
  Shield,
  AlertTriangle,
  CheckCircle,
  TrendingUp,
  TrendingDown,
  Users,
  FileText,
  Activity,
  Eye,
  Lock,
  AlertCircle,
  Clock,
  MoreHorizontal,
  Download,
  RefreshCw,
  Filter,
  ChevronRight,
  Check,
  X,
  Loader2,
  Calendar,
  Search,
  ExternalLink
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
import { Checkbox } from '@/components/ui/checkbox';
import { useToast } from '@/components/ui/use-toast';
import {
  LineChart,
  Line,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  ResponsiveContainer,
  AreaChart,
  Area
} from 'recharts';
import {
  hipaaComplianceApi,
  ComplianceScore,
  ComplianceAlert,
  PHICAccessSummary,
  BreachStatus,
  SafeguardChecklist,
  ActionItem
} from '@/lib/api/hipaaCompliance';

export default function HIPAACompliancePage() {
  const { toast } = useToast();
  const [loading, setLoading] = useState(true);
  const [complianceScore, setComplianceScore] = useState<ComplianceScore | null>(null);
  const [alerts, setAlerts] = useState<ComplianceAlert[]>([]);
  const [phiSummary, setPhiSummary] = useState<PHICAccessSummary | null>(null);
  const [breaches, setBreaches] = useState<BreachStatus[]>([]);
  const [safeguards, setSafeguards] = useState<SafeguardChecklist[]>([]);
  const [actionItems, setActionItems] = useState<ActionItem[]>([]);
  const [trends, setTrends] = useState<any[]>([]);
  const [activeTab, setActiveTab] = useState('overview');
  const [selectedAlert, setSelectedAlert] = useState<ComplianceAlert | null>(null);
  const [isAlertModalOpen, setIsAlertModalOpen] = useState(false);
  const [acknowledgeNotes, setAcknowledgeNotes] = useState('');

  const fetchData = useCallback(async () => {
    try {
      setLoading(true);
      const [
        scoreData,
        alertsData,
        phiData,
        breachesData,
        safeguardsData,
        actionItemsData,
        trendsData
      ] = await Promise.all([
        hipaaComplianceApi.getComplianceScore(),
        hipaaComplianceApi.getAlerts(),
        hipaaComplianceApi.getPHICAccessSummary(),
        hipaaComplianceApi.getBreaches(),
        hipaaComplianceApi.getSafeguardChecklist(),
        hipaaComplianceApi.getActionItems(),
        hipaaComplianceApi.getComplianceTrends()
      ]);

      setComplianceScore(scoreData);
      setAlerts(alertsData);
      setPhiSummary(phiData);
      setBreaches(breachesData);
      setSafeguards(safeguardsData);
      setActionItems(actionItemsData);
      setTrends(trendsData.map((t: any) => ({
        date: new Date(t.date).toLocaleDateString('en-US', { month: 'short', day: 'numeric' }),
        score: t.overall,
        administrative: t.administrative,
        physical: t.physical,
        technical: t.technical
      })));
    } catch (err) {
      toast({
        title: 'Error',
        description: 'Failed to load compliance data',
        variant: 'destructive'
      });
    } finally {
      setLoading(false);
    }
  }, [toast]);

  useEffect(() => {
    fetchData();
  }, [fetchData]);

  const handleAcknowledgeAlert = async () => {
    if (!selectedAlert) return;

    try {
      await hipaaComplianceApi.acknowledgeAlert(selectedAlert.id, {
        notes: acknowledgeNotes
      });
      toast({
        title: 'Success',
        description: 'Alert acknowledged'
      });
      setIsAlertModalOpen(false);
      setAcknowledgeNotes('');
      fetchData();
    } catch (err) {
      toast({
        title: 'Error',
        description: 'Failed to acknowledge alert',
        variant: 'destructive'
      });
    }
  };

  const handleResolveAlert = async (alertId: string) => {
    try {
      await hipaaComplianceApi.resolveAlert(alertId, 'Resolved by user');
      toast({
        title: 'Success',
        description: 'Alert resolved'
      });
      fetchData();
    } catch (err) {
      toast({
        title: 'Error',
        description: 'Failed to resolve alert',
        variant: 'destructive'
      });
    }
  };

  const getSeverityBadge = (severity: string) => {
    const config: Record<string, { variant: 'default' | 'success' | 'warning' | 'danger'; icon: any }> = {
      critical: { variant: 'danger', icon: AlertCircle },
      high: { variant: 'warning', icon: AlertTriangle },
      medium: { variant: 'default', icon: AlertCircle },
      low: { variant: 'success', icon: CheckCircle }
    };
    const { variant, icon: Icon } = config[severity] || config.medium;
    return (
      <Badge variant={variant} className="gap-1">
        <Icon className="w-3 h-3" />
        {severity.charAt(0).toUpperCase() + severity.slice(1)}
      </Badge>
    );
  };

  const getTrendIcon = (trend: string) => {
    if (trend === 'improving') return <TrendingUp className="w-5 h-5 text-green-500" />;
    if (trend === 'declining') return <TrendingDown className="w-5 h-5 text-red-500" />;
    return <Activity className="w-5 h-5 text-blue-500" />;
  };

  const getSafeguardStatusBadge = (status: string) => {
    const variants: Record<string, 'success' | 'warning' | 'danger' | 'default'> = {
      implemented: 'success',
      partial: 'warning',
      not_implemented: 'danger',
      not_applicable: 'default'
    };
    return (
      <Badge variant={variants[status] || 'default'}>
        {status.replace('_', ' ').replace(/\b\w/g, l => l.toUpperCase())}
      </Badge>
    );
  };

  if (loading) {
    return (
      <div className="container mx-auto p-6 space-y-6">
        <Skeleton className="h-10 w-64" />
        <div className="grid grid-cols-1 md:grid-cols-4 gap-4">
          {[1, 2, 3, 4].map(i => <Skeleton key={i} className="h-32" />)}
        </div>
      </div>
    );
  }

  return (
    <div className="container mx-auto p-6 space-y-6">
      {/* Header */}
      <div className="flex flex-col md:flex-row md:items-center md:justify-between gap-4">
        <div>
          <h1 className="text-3xl font-bold flex items-center gap-2">
            <Shield className="w-8 h-8 text-blue-600" />
            HIPAA Compliance
          </h1>
          <p className="text-gray-500 mt-1">Monitor and maintain HIPAA compliance status</p>
        </div>
        <Button onClick={fetchData} variant="outline" className="gap-2">
          <RefreshCw className="w-4 h-4" />
          Refresh
        </Button>
      </div>

      {/* Compliance Score Widget */}
      {complianceScore && (
        <Card className="bg-gradient-to-br from-blue-50 to-indigo-50 dark:from-blue-900/20 dark:to-indigo-900/20">
          <CardContent className="p-6">
            <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
              <div className="flex items-center gap-6">
                <div className="relative w-32 h-32">
                  <svg className="w-full h-full -rotate-90" viewBox="0 0 100 100">
                    <circle
                      cx="50"
                      cy="50"
                      r="45"
                      fill="none"
                      stroke="currentColor"
                      strokeWidth="8"
                      className="text-gray-200 dark:text-gray-700"
                    />
                    <circle
                      cx="50"
                      cy="50"
                      r="45"
                      fill="none"
                      stroke="currentColor"
                      strokeWidth="8"
                      strokeLinecap="round"
                      strokeDasharray={`${complianceScore.overall * 2.83} 283`}
                      className={
                        complianceScore.overall >= 90 ? 'text-green-500' :
                        complianceScore.overall >= 70 ? 'text-yellow-500' : 'text-red-500'
                      }
                    />
                  </svg>
                  <div className="absolute inset-0 flex items-center justify-center">
                    <div className="text-center">
                      <p className="text-3xl font-bold">{complianceScore.overall}%</p>
                      <p className="text-xs text-gray-500">Compliance</p>
                    </div>
                  </div>
                </div>
                <div>
                  <div className="flex items-center gap-2 mb-2">
                    <h2 className="text-xl font-semibold">Overall Score</h2>
                    {getTrendIcon(complianceScore.trend)}
                  </div>
                  <p className="text-sm text-gray-500">
                    Last assessed: {new Date(complianceScore.lastAssessmentDate).toLocaleDateString()}
                  </p>
                  <p className="text-sm text-gray-500">
                    Next due: {new Date(complianceScore.nextAssessmentDue).toLocaleDateString()}
                  </p>
                </div>
              </div>
              <div className="grid grid-cols-2 gap-4">
                <div className="space-y-2">
                  <div className="flex items-center justify-between">
                    <span className="text-sm">Administrative</span>
                    <span className="font-medium">{complianceScore.categories.administrative}%</span>
                  </div>
                  <Progress value={complianceScore.categories.administrative} className="h-2" />
                </div>
                <div className="space-y-2">
                  <div className="flex items-center justify-between">
                    <span className="text-sm">Physical</span>
                    <span className="font-medium">{complianceScore.categories.physical}%</span>
                  </div>
                  <Progress value={complianceScore.categories.physical} className="h-2" />
                </div>
                <div className="space-y-2">
                  <div className="flex items-center justify-between">
                    <span className="text-sm">Technical</span>
                    <span className="font-medium">{complianceScore.categories.technical}%</span>
                  </div>
                  <Progress value={complianceScore.categories.technical} className="h-2" />
                </div>
                <div className="space-y-2">
                  <div className="flex items-center justify-between">
                    <span className="text-sm">Organizational</span>
                    <span className="font-medium">{complianceScore.categories.organizational}%</span>
                  </div>
                  <Progress value={complianceScore.categories.organizational} className="h-2" />
                </div>
              </div>
            </div>
          </CardContent>
        </Card>
      )}

      {/* Main Content Tabs */}
      <Tabs value={activeTab} onValueChange={setActiveTab}>
        <TabsList className="w-full">
          <TabsTrigger value="overview" className="gap-2">
            <Activity className="w-4 h-4" />
            Overview
          </TabsTrigger>
          <TabsTrigger value="alerts" className="gap-2">
            <AlertTriangle className="w-4 h-4" />
            Alerts ({alerts.filter(a => a.status === 'open').length})
          </TabsTrigger>
          <TabsTrigger value="phi" className="gap-2">
            <Eye className="w-4 h-4" />
            PHI Access
          </TabsTrigger>
          <TabsTrigger value="breaches" className="gap-2">
            <Lock className="w-4 h-4" />
            Breaches
          </TabsTrigger>
          <TabsTrigger value="safeguards" className="gap-2">
            <CheckCircle className="w-4 h-4" />
            Safeguards
          </TabsTrigger>
          <TabsTrigger value="actions" className="gap-2">
            <FileText className="w-4 h-4" />
            Action Items
          </TabsTrigger>
        </TabsList>

        {/* Overview Tab */}
        <TabsContent value="overview" className="space-y-6 mt-6">
          {/* Trend Chart */}
          <Card>
            <CardHeader>
              <CardTitle>Compliance Trend (6 Months)</CardTitle>
            </CardHeader>
            <CardContent>
              <div className="h-80">
                <ResponsiveContainer width="100%" height="100%">
                  <AreaChart data={trends}>
                    <defs>
                      <linearGradient id="colorScore" x1="0" y1="0" x2="0" y2="1">
                        <stop offset="5%" stopColor="#2563eb" stopOpacity={0.3}/>
                        <stop offset="95%" stopColor="#2563eb" stopOpacity={0}/>
                      </linearGradient>
                    </defs>
                    <CartesianGrid strokeDasharray="3 3" />
                    <XAxis dataKey="date" />
                    <YAxis domain={[0, 100]} />
                    <Tooltip />
                    <Area
                      type="monotone"
                      dataKey="score"
                      stroke="#2563eb"
                      fillOpacity={1}
                      fill="url(#colorScore)"
                      name="Overall Score"
                    />
                    <Area
                      type="monotone"
                      dataKey="administrative"
                      stroke="#22c55e"
                      fill="none"
                      name="Administrative"
                    />
                    <Area
                      type="monotone"
                      dataKey="technical"
                      stroke="#f59e0b"
                      fill="none"
                      name="Technical"
                    />
                  </AreaChart>
                </ResponsiveContainer>
              </div>
            </CardContent>
          </Card>

          {/* Quick Stats */}
          <div className="grid grid-cols-1 md:grid-cols-4 gap-4">
            <Card>
              <CardContent className="p-4">
                <div className="flex items-center gap-3">
                  <div className="p-3 bg-red-100 dark:bg-red-900 rounded-lg">
                    <AlertCircle className="w-5 h-5 text-red-600 dark:text-red-300" />
                  </div>
                  <div>
                    <p className="text-2xl font-bold">
                      {alerts.filter(a => a.status === 'open' && ['critical', 'high'].includes(a.severity)).length}
                    </p>
                    <p className="text-sm text-gray-500">Critical Alerts</p>
                  </div>
                </div>
              </CardContent>
            </Card>
            <Card>
              <CardContent className="p-4">
                <div className="flex items-center gap-3">
                  <div className="p-3 bg-blue-100 dark:bg-blue-900 rounded-lg">
                    <Eye className="w-5 h-5 text-blue-600 dark:text-blue-300" />
                  </div>
                  <div>
                    <p className="text-2xl font-bold">{phiSummary?.totalAccesses.toLocaleString() || 0}</p>
                    <p className="text-sm text-gray-500">PHI Accesses (30d)</p>
                  </div>
                </div>
              </CardContent>
            </Card>
            <Card>
              <CardContent className="p-4">
                <div className="flex items-center gap-3">
                  <div className="p-3 bg-purple-100 dark:bg-purple-900 rounded-lg">
                    <Lock className="w-5 h-5 text-purple-600 dark:text-purple-300" />
                  </div>
                  <div>
                    <p className="text-2xl font-bold">{breaches.filter(b => b.status !== 'closed').length}</p>
                    <p className="text-sm text-gray-500">Open Breaches</p>
                  </div>
                </div>
              </CardContent>
            </Card>
            <Card>
              <CardContent className="p-4">
                <div className="flex items-center gap-3">
                  <div className="p-3 bg-orange-100 dark:bg-orange-900 rounded-lg">
                    <FileText className="w-5 h-5 text-orange-600 dark:text-orange-300" />
                  </div>
                  <div>
                    <p className="text-2xl font-bold">{actionItems.filter(a => a.status !== 'completed').length}</p>
                    <p className="text-sm text-gray-500">Pending Actions</p>
                  </div>
                </div>
              </CardContent>
            </Card>
          </div>
        </TabsContent>

        {/* Alerts Tab */}
        <TabsContent value="alerts" className="mt-6">
          <div className="space-y-4">
            {alerts.map((alert) => (
              <Card key={alert.id} className={alert.status === 'open' ? 'border-l-4 border-l-red-500' : ''}>
                <CardContent className="p-4">
                  <div className="flex items-start justify-between">
                    <div className="flex items-start gap-4">
                      <div className={`p-2 rounded-lg ${
                        alert.severity === 'critical' ? 'bg-red-100 dark:bg-red-900' :
                        alert.severity === 'high' ? 'bg-orange-100 dark:bg-orange-900' :
                        alert.severity === 'medium' ? 'bg-yellow-100 dark:bg-yellow-900' :
                        'bg-blue-100 dark:bg-blue-900'
                      }`}>
                        <AlertTriangle className={`w-5 h-5 ${
                          alert.severity === 'critical' ? 'text-red-600' :
                          alert.severity === 'high' ? 'text-orange-600' :
                          alert.severity === 'medium' ? 'text-yellow-600' :
                          'text-blue-600'
                        }`} />
                      </div>
                      <div>
                        <div className="flex items-center gap-2 mb-1">
                          <h3 className="font-medium">{alert.title}</h3>
                          {getSeverityBadge(alert.severity)}
                          <Badge variant="outline">{alert.category}</Badge>
                        </div>
                        <p className="text-sm text-gray-500 mb-2">{alert.description}</p>
                        <div className="flex items-center gap-4 text-sm text-gray-500">
                          <span className="flex items-center gap-1">
                            <Clock className="w-3 h-3" />
                            {new Date(alert.createdAt).toLocaleDateString()}
                          </span>
                          {alert.dueDate && (
                            <span className="flex items-center gap-1">
                              <Calendar className="w-3 h-3" />
                              Due: {new Date(alert.dueDate).toLocaleDateString()}
                            </span>
                          )}
                        </div>
                      </div>
                    </div>
                    <div className="flex items-center gap-2">
                      {alert.status === 'open' && (
                        <>
                          <Button
                            variant="outline"
                            size="sm"
                            onClick={() => {
                              setSelectedAlert(alert);
                              setIsAlertModalOpen(true);
                            }}
                          >
                            Acknowledge
                          </Button>
                          <Button
                            variant="ghost"
                            size="sm"
                            onClick={() => handleResolveAlert(alert.id)}
                          >
                            <Check className="w-4 h-4 text-green-500" />
                          </Button>
                        </>
                      )}
                      {alert.status === 'acknowledged' && (
                        <Badge variant="info">Acknowledged</Badge>
                      )}
                      {alert.status === 'resolved' && (
                        <Badge variant="success">Resolved</Badge>
                      )}
                    </div>
                  </div>
                </CardContent>
              </Card>
            ))}
            {alerts.length === 0 && (
              <div className="text-center py-12 text-gray-500">
                <CheckCircle className="w-12 h-12 mx-auto mb-4 text-green-300" />
                <p>No compliance alerts</p>
              </div>
            )}
          </div>
        </TabsContent>

        {/* PHI Access Tab */}
        <TabsContent value="phi" className="mt-6">
          {phiSummary && (
            <div className="space-y-6">
              <div className="grid grid-cols-1 md:grid-cols-4 gap-4">
                <Card>
                  <CardContent className="p-4 text-center">
                    <p className="text-3xl font-bold">{phiSummary.totalAccesses.toLocaleString()}</p>
                    <p className="text-sm text-gray-500">Total Accesses</p>
                  </CardContent>
                </Card>
                <Card>
                  <CardContent className="p-4 text-center">
                    <p className="text-3xl font-bold">{phiSummary.uniqueUsers}</p>
                    <p className="text-sm text-gray-500">Unique Users</p>
                  </CardContent>
                </Card>
                <Card>
                  <CardContent className="p-4 text-center">
                    <p className="text-3xl font-bold">{phiSummary.uniquePatients}</p>
                    <p className="text-sm text-gray-500">Unique Patients</p>
                  </CardContent>
                </Card>
                <Card>
                  <CardContent className="p-4 text-center">
                    <p className="text-3xl font-bold">
                      {Object.values(phiSummary.accessesByType).reduce((a, b) => a + b, 0).toLocaleString()}
                    </p>
                    <p className="text-sm text-gray-500">Total Events</p>
                  </CardContent>
                </Card>
              </div>

              <Card>
                <CardHeader>
                  <CardTitle>Access by Type</CardTitle>
                </CardHeader>
                <CardContent>
                  <div className="grid grid-cols-2 md:grid-cols-5 gap-4">
                    {Object.entries(phiSummary.accessesByType).map(([type, count]) => (
                      <div key={type} className="p-4 bg-gray-50 dark:bg-gray-800 rounded-lg text-center">
                        <p className="text-2xl font-bold">{count.toLocaleString()}</p>
                        <p className="text-sm text-gray-500 capitalize">{type}</p>
                      </div>
                    ))}
                  </div>
                </CardContent>
              </Card>
            </div>
          )}
        </TabsContent>

        {/* Breaches Tab */}
        <TabsContent value="breaches" className="mt-6">
          <div className="space-y-4">
            {breaches.map((breach) => (
              <Card key={breach.id} className={breach.status !== 'closed' ? 'border-l-4 border-l-red-500' : ''}>
                <CardContent className="p-4">
                  <div className="flex items-start justify-between">
                    <div className="flex items-start gap-4">
                      <div className="p-2 bg-red-100 dark:bg-red-900 rounded-lg">
                        <Lock className="w-5 h-5 text-red-600 dark:text-red-300" />
                      </div>
                      <div>
                        <div className="flex items-center gap-2 mb-1">
                          <h3 className="font-medium capitalize">{breach.type.replace('_', ' ')}</h3>
                          <Badge variant={breach.status === 'closed' ? 'success' : 'danger'}>
                            {breach.status.replace('_', ' ')}
                          </Badge>
                          <Badge variant={breach.riskAssessment === 'high' ? 'danger' : breach.riskAssessment === 'moderate' ? 'warning' : 'success'}>
                            {breach.riskAssessment} risk
                          </Badge>
                        </div>
                        <p className="text-sm text-gray-500 mb-2">{breach.description}</p>
                        <div className="flex items-center gap-4 text-sm text-gray-500">
                          <span>{breach.affectedIndividuals.toLocaleString()} individuals affected</span>
                          <span>{breach.affectedRecords.toLocaleString()} records</span>
                        </div>
                      </div>
                    </div>
                    <div className="text-right text-sm text-gray-500">
                      <p>Discovered: {new Date(breach.discoveryDate).toLocaleDateString()}</p>
                      {breach.hhsNotificationDate && (
                        <p>HHS Notified: {new Date(breach.hhsNotificationDate).toLocaleDateString()}</p>
                      )}
                    </div>
                  </div>
                </CardContent>
              </Card>
            ))}
            {breaches.length === 0 && (
              <div className="text-center py-12 text-gray-500">
                <Shield className="w-12 h-12 mx-auto mb-4 text-green-300" />
                <p>No breach incidents</p>
              </div>
            )}
          </div>
        </TabsContent>

        {/* Safeguards Tab */}
        <TabsContent value="safeguards" className="mt-6">
          <Card>
            <CardContent className="p-0">
              <div className="overflow-x-auto">
                <table className="w-full">
                  <thead className="bg-gray-50 dark:bg-gray-800 border-b">
                    <tr>
                      <th className="text-left p-4 font-medium">Requirement</th>
                      <th className="text-left p-4 font-medium">Category</th>
                      <th className="text-left p-4 font-medium">Status</th>
                      <th className="text-left p-4 font-medium">Review Date</th>
                    </tr>
                  </thead>
                  <tbody>
                    {safeguards.map((safeguard) => (
                      <tr key={safeguard.id} className="border-b hover:bg-gray-50 dark:hover:bg-gray-800">
                        <td className="p-4">
                          <p className="font-medium">{safeguard.requirement}</p>
                          <p className="text-sm text-gray-500">{safeguard.description}</p>
                        </td>
                        <td className="p-4">
                          <Badge variant="outline" className="capitalize">
                            {safeguard.category}
                          </Badge>
                        </td>
                        <td className="p-4">{getSafeguardStatusBadge(safeguard.implementationStatus)}</td>
                        <td className="p-4 text-sm text-gray-500">
                          {safeguard.reviewDate ? new Date(safeguard.reviewDate).toLocaleDateString() : '-'}
                        </td>
                      </tr>
                    ))}
                  </tbody>
                </table>
              </div>
            </CardContent>
          </Card>
        </TabsContent>

        {/* Action Items Tab */}
        <TabsContent value="actions" className="mt-6">
          <div className="space-y-4">
            {actionItems.map((item) => (
              <Card key={item.id} className={item.status !== 'completed' ? 'border-l-4 border-l-orange-500' : ''}>
                <CardContent className="p-4">
                  <div className="flex items-start justify-between">
                    <div className="flex items-start gap-3">
                      <Checkbox checked={item.status === 'completed'} disabled />
                      <div>
                        <h3 className={`font-medium ${item.status === 'completed' ? 'line-through text-gray-500' : ''}`}>
                          {item.title}
                        </h3>
                        <p className="text-sm text-gray-500">{item.description}</p>
                        <div className="flex items-center gap-2 mt-2">
                          <Badge variant="outline">{item.category}</Badge>
                          <Badge variant={item.priority === 'critical' ? 'danger' : item.priority === 'high' ? 'warning' : 'default'}>
                            {item.priority}
                          </Badge>
                        </div>
                      </div>
                    </div>
                    <div className="text-right">
                      <p className="text-sm">{item.assignedToName}</p>
                      <p className="text-sm text-gray-500">Due: {new Date(item.dueDate).toLocaleDateString()}</p>
                    </div>
                  </div>
                </CardContent>
              </Card>
            ))}
            {actionItems.length === 0 && (
              <div className="text-center py-12 text-gray-500">
                <CheckCircle className="w-12 h-12 mx-auto mb-4 text-green-300" />
                <p>No action items</p>
              </div>
            )}
          </div>
        </TabsContent>
      </Tabs>

      {/* Acknowledge Alert Modal */}
      <Dialog open={isAlertModalOpen} onOpenChange={setIsAlertModalOpen}>
        <DialogContent>
          <DialogHeader>
            <DialogTitle>Acknowledge Alert</DialogTitle>
          </DialogHeader>
          {selectedAlert && (
            <div className="space-y-4">
              <div className="p-3 bg-yellow-50 dark:bg-yellow-900/20 rounded-lg">
                <p className="font-medium">{selectedAlert.title}</p>
                <p className="text-sm text-gray-500">{selectedAlert.description}</p>
              </div>
              <div>
                <Label htmlFor="notes">Notes (optional)</Label>
                <Textarea
                  id="notes"
                  value={acknowledgeNotes}
                  onChange={(e) => setAcknowledgeNotes(e.target.value)}
                  placeholder="Add any notes about this acknowledgment..."
                  rows={3}
                />
              </div>
              <DialogFooter>
                <Button variant="outline" onClick={() => setIsAlertModalOpen(false)}>
                  Cancel
                </Button>
                <Button onClick={handleAcknowledgeAlert}>
                  Acknowledge Alert
                </Button>
              </DialogFooter>
            </div>
          )}
        </DialogContent>
      </Dialog>
    </div>
  );
}

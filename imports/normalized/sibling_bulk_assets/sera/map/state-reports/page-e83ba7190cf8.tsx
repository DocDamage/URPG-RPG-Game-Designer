'use client';

import { useState, useEffect, useCallback } from 'react';
import {
  FileText,
  Calendar,
  Download,
  Send,
  CheckCircle,
  AlertCircle,
  Clock,
  Plus,
  Search,
  Filter,
  MoreHorizontal,
  History,
  CalendarDays,
  FileDown,
  Bell,
  ChevronRight,
  Loader2,
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
import { Alert, AlertDescription } from '@/components/ui/alert';
import { Skeleton } from '@/components/ui/skeleton';
import { Progress } from '@/components/ui/progress';
import { useToast } from '@/components/ui/use-toast';
import { stateReportingApi, ReportTemplate, GeneratedReport, ReportSchedule } from '@/lib/api/stateReporting';

const STATE_OPTIONS = [
  { code: 'CA', name: 'California' },
  { code: 'TX', name: 'Texas' },
  { code: 'NY', name: 'New York' },
  { code: 'FL', name: 'Florida' },
  { code: 'PA', name: 'Pennsylvania' },
  { code: 'IL', name: 'Illinois' },
  { code: 'OH', name: 'Ohio' },
  { code: 'GA', name: 'Georgia' },
  { code: 'NC', name: 'North Carolina' },
  { code: 'MI', name: 'Michigan' }
];

export default function StateReportsPage() {
  const { toast } = useToast();
  const [templates, setTemplates] = useState<ReportTemplate[]>([]);
  const [reports, setReports] = useState<GeneratedReport[]>([]);
  const [schedules, setSchedules] = useState<ReportSchedule[]>([]);
  const [loading, setLoading] = useState(true);
  const [selectedState, setSelectedState] = useState<string>('all');
  const [selectedTemplate, setSelectedTemplate] = useState<ReportTemplate | null>(null);
  const [isGenerateModalOpen, setIsGenerateModalOpen] = useState(false);
  const [isScheduleModalOpen, setIsScheduleModalOpen] = useState(false);
  const [generating, setGenerating] = useState(false);
  const [activeTab, setActiveTab] = useState('templates');
  
  // Form states
  const [generateForm, setGenerateForm] = useState({
    startDate: '',
    endDate: '',
    locationIds: [] as string[],
    format: 'pdf'
  });

  const [scheduleForm, setScheduleForm] = useState({
    frequency: 'monthly',
    nextDueDate: '',
    isAutoGenerate: false,
    isAutoSubmit: false,
    reminderDays: [7, 3, 1] as number[]
  });

  const fetchData = useCallback(async () => {
    try {
      setLoading(true);
      const [templatesData, reportsData, schedulesData] = await Promise.all([
        stateReportingApi.getReportTemplates(selectedState === 'all' ? undefined : selectedState),
        stateReportingApi.getReports(),
        stateReportingApi.getSchedules()
      ]);
      setTemplates(templatesData);
      setReports(reportsData);
      setSchedules(schedulesData);
    } catch (err) {
      toast({
        title: 'Error',
        description: 'Failed to load reports data',
        variant: 'destructive'
      });
    } finally {
      setLoading(false);
    }
  }, [selectedState, toast]);

  useEffect(() => {
    fetchData();
  }, [fetchData]);

  const handleGenerate = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!selectedTemplate) return;

    try {
      setGenerating(true);
      const report = await stateReportingApi.generateReport({
        templateId: selectedTemplate.id,
        reportingPeriod: {
          startDate: generateForm.startDate,
          endDate: generateForm.endDate
        },
        locationIds: generateForm.locationIds.length > 0 ? generateForm.locationIds : undefined,
        options: {
          format: generateForm.format as any
        }
      });

      toast({
        title: 'Success',
        description: 'Report generated successfully'
      });

      setIsGenerateModalOpen(false);
      fetchData();
      setActiveTab('history');
    } catch (err) {
      toast({
        title: 'Error',
        description: 'Failed to generate report',
        variant: 'destructive'
      });
    } finally {
      setGenerating(false);
    }
  };

  const handleDownload = async (reportId: string, fileName: string) => {
    try {
      const blob = await stateReportingApi.downloadReport(reportId);
      const url = window.URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = fileName;
      document.body.appendChild(a);
      a.click();
      document.body.removeChild(a);
      window.URL.revokeObjectURL(url);
    } catch (err) {
      toast({
        title: 'Error',
        description: 'Failed to download report',
        variant: 'destructive'
      });
    }
  };

  const handleSubmit = async (reportId: string) => {
    try {
      await stateReportingApi.submitReport(reportId);
      toast({
        title: 'Success',
        description: 'Report submitted successfully'
      });
      fetchData();
    } catch (err) {
      toast({
        title: 'Error',
        description: 'Failed to submit report',
        variant: 'destructive'
      });
    }
  };

  const getStatusBadge = (status: string) => {
    const config: Record<string, { variant: 'default' | 'success' | 'warning' | 'info' | 'danger'; icon: any }> = {
      generating: { variant: 'info', icon: Loader2 },
      ready: { variant: 'success', icon: CheckCircle },
      error: { variant: 'danger', icon: AlertCircle },
      submitted: { variant: 'info', icon: Send },
      accepted: { variant: 'success', icon: CheckCircle },
      rejected: { variant: 'danger', icon: AlertCircle }
    };
    const { variant, icon: Icon } = config[status] || config.ready;
    return (
      <Badge variant={variant} className="gap-1">
        <Icon className={`w-3 h-3 ${status === 'generating' ? 'animate-spin' : ''}`} />
        {status.charAt(0).toUpperCase() + status.slice(1)}
      </Badge>
    );
  };

  const getFrequencyLabel = (freq: string) => {
    const labels: Record<string, string> = {
      daily: 'Daily',
      weekly: 'Weekly',
      monthly: 'Monthly',
      quarterly: 'Quarterly',
      annual: 'Annual',
      'ad-hoc': 'Ad-hoc'
    };
    return labels[freq] || freq;
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
            State Reporting
          </h1>
          <p className="text-gray-500 mt-1">Generate and manage state compliance reports</p>
        </div>
        <div className="flex items-center gap-2">
          <Select value={selectedState} onValueChange={setSelectedState}>
            <SelectTrigger className="w-48">
              <SelectValue placeholder="Select State" />
            </SelectTrigger>
            <SelectContent>
              <SelectItem value="all">All States</SelectItem>
              {STATE_OPTIONS.map(state => (
                <SelectItem key={state.code} value={state.code}>{state.name}</SelectItem>
              ))}
            </SelectContent>
          </Select>
        </div>
      </div>

      {/* Stats */}
      <div className="grid grid-cols-1 md:grid-cols-4 gap-4">
        <Card>
          <CardContent className="p-4">
            <div className="flex items-center gap-3">
              <div className="p-3 bg-blue-100 dark:bg-blue-900 rounded-lg">
                <FileText className="w-5 h-5 text-blue-600 dark:text-blue-300" />
              </div>
              <div>
                <p className="text-2xl font-bold">{templates.length}</p>
                <p className="text-sm text-gray-500">Report Templates</p>
              </div>
            </div>
          </CardContent>
        </Card>
        <Card>
          <CardContent className="p-4">
            <div className="flex items-center gap-3">
              <div className="p-3 bg-green-100 dark:bg-green-900 rounded-lg">
                <CheckCircle className="w-5 h-5 text-green-600 dark:text-green-300" />
              </div>
              <div>
                <p className="text-2xl font-bold">
                  {reports.filter(r => r.status === 'accepted').length}
                </p>
                <p className="text-sm text-gray-500">Accepted Reports</p>
              </div>
            </div>
          </CardContent>
        </Card>
        <Card>
          <CardContent className="p-4">
            <div className="flex items-center gap-3">
              <div className="p-3 bg-yellow-100 dark:bg-yellow-900 rounded-lg">
                <Clock className="w-5 h-5 text-yellow-600 dark:text-yellow-300" />
              </div>
              <div>
                <p className="text-2xl font-bold">
                  {reports.filter(r => r.status === 'ready' || r.status === 'generating').length}
                </p>
                <p className="text-sm text-gray-500">Pending</p>
              </div>
            </div>
          </CardContent>
        </Card>
        <Card>
          <CardContent className="p-4">
            <div className="flex items-center gap-3">
              <div className="p-3 bg-purple-100 dark:bg-purple-900 rounded-lg">
                <CalendarDays className="w-5 h-5 text-purple-600 dark:text-purple-300" />
              </div>
              <div>
                <p className="text-2xl font-bold">{schedules.length}</p>
                <p className="text-sm text-gray-500">Scheduled</p>
              </div>
            </div>
          </CardContent>
        </Card>
      </div>

      {/* Main Content */}
      <Tabs value={activeTab} onValueChange={setActiveTab}>
        <TabsList>
          <TabsTrigger value="templates" className="gap-2">
            <FileText className="w-4 h-4" />
            Templates
          </TabsTrigger>
          <TabsTrigger value="history" className="gap-2">
            <History className="w-4 h-4" />
            History
          </TabsTrigger>
          <TabsTrigger value="schedules" className="gap-2">
            <Calendar className="w-4 h-4" />
            Schedules
          </TabsTrigger>
          <TabsTrigger value="downloads" className="gap-2">
            <FileDown className="w-4 h-4" />
            Downloads
          </TabsTrigger>
        </TabsList>

        {/* Templates Tab */}
        <TabsContent value="templates" className="mt-6">
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
            {templates.map((template) => (
              <Card key={template.id} className="hover:shadow-lg transition-shadow">
                <CardHeader>
                  <div className="flex items-start justify-between">
                    <div className="p-2 bg-blue-100 dark:bg-blue-900 rounded-lg">
                      <FileText className="w-5 h-5 text-blue-600 dark:text-blue-300" />
                    </div>
                    <Badge variant={template.isActive ? 'success' : 'warning'}>
                      {template.isActive ? 'Active' : 'Inactive'}
                    </Badge>
                  </div>
                  <CardTitle className="text-lg mt-2">{template.name}</CardTitle>
                  <CardDescription>{template.description}</CardDescription>
                </CardHeader>
                <CardContent className="space-y-3">
                  <div className="flex items-center justify-between text-sm">
                    <span className="text-gray-500">State</span>
                    <Badge variant="secondary">{template.stateCode}</Badge>
                  </div>
                  <div className="flex items-center justify-between text-sm">
                    <span className="text-gray-500">Frequency</span>
                    <span className="font-medium">{getFrequencyLabel(template.frequency)}</span>
                  </div>
                  <div className="flex items-center justify-between text-sm">
                    <span className="text-gray-500">Format</span>
                    <Badge variant="outline">{template.fileFormat.toUpperCase()}</Badge>
                  </div>
                  <div className="flex items-center justify-between text-sm">
                    <span className="text-gray-500">Submission</span>
                    <span className="capitalize">{template.submissionMethod}</span>
                  </div>
                  <Button
                    className="w-full mt-4"
                    onClick={() => {
                      setSelectedTemplate(template);
                      setIsGenerateModalOpen(true);
                    }}
                  >
                    <Plus className="w-4 h-4 mr-2" />
                    Generate Report
                  </Button>
                </CardContent>
              </Card>
            ))}
          </div>
          {templates.length === 0 && (
            <div className="text-center py-12 text-gray-500">
              <FileText className="w-12 h-12 mx-auto mb-4 text-gray-300" />
              <p>No report templates found</p>
            </div>
          )}
        </TabsContent>

        {/* History Tab */}
        <TabsContent value="history" className="mt-6">
          <Card>
            <CardContent className="p-0">
              <div className="overflow-x-auto">
                <table className="w-full">
                  <thead className="bg-gray-50 dark:bg-gray-800 border-b">
                    <tr>
                      <th className="text-left p-4 font-medium">Report</th>
                      <th className="text-left p-4 font-medium">Period</th>
                      <th className="text-left p-4 font-medium">Status</th>
                      <th className="text-left p-4 font-medium">Generated</th>
                      <th className="text-right p-4 font-medium">Actions</th>
                    </tr>
                  </thead>
                  <tbody>
                    {reports.map((report) => (
                      <tr key={report.id} className="border-b hover:bg-gray-50 dark:hover:bg-gray-800">
                        <td className="p-4">
                          <div>
                            <p className="font-medium">{report.templateName}</p>
                            <p className="text-sm text-gray-500">{report.stateCode}</p>
                          </div>
                        </td>
                        <td className="p-4 text-sm">
                          {new Date(report.reportingPeriod.startDate).toLocaleDateString()} - {new Date(report.reportingPeriod.endDate).toLocaleDateString()}
                        </td>
                        <td className="p-4">{getStatusBadge(report.status)}</td>
                        <td className="p-4 text-sm text-gray-500">
                          {report.generatedAt ? new Date(report.generatedAt).toLocaleDateString() : '-'}
                        </td>
                        <td className="p-4 text-right">
                          <div className="flex items-center justify-end gap-2">
                            {report.status === 'ready' && (
                              <Button
                                variant="ghost"
                                size="sm"
                                onClick={() => handleSubmit(report.id)}
                              >
                                <Send className="w-4 h-4" />
                              </Button>
                            )}
                            {report.fileUrl && (
                              <Button
                                variant="ghost"
                                size="sm"
                                onClick={() => handleDownload(report.id, `${report.templateName}.${report.format}`)}
                              >
                                <Download className="w-4 h-4" />
                              </Button>
                            )}
                          </div>
                        </td>
                      </tr>
                    ))}
                  </tbody>
                </table>
              </div>
              {reports.length === 0 && (
                <div className="p-8 text-center text-gray-500">
                  <History className="w-12 h-12 mx-auto mb-4 text-gray-300" />
                  <p>No reports generated yet</p>
                </div>
              )}
            </CardContent>
          </Card>
        </TabsContent>

        {/* Schedules Tab */}
        <TabsContent value="schedules" className="mt-6">
          <div className="flex justify-end mb-4">
            <Button onClick={() => setIsScheduleModalOpen(true)} className="gap-2">
              <Plus className="w-4 h-4" />
              Add Schedule
            </Button>
          </div>
          <div className="space-y-4">
            {schedules.map((schedule) => (
              <Card key={schedule.id}>
                <CardContent className="p-4">
                  <div className="flex items-center justify-between">
                    <div className="flex items-center gap-4">
                      <div className="p-3 bg-purple-100 dark:bg-purple-900 rounded-lg">
                        <Calendar className="w-5 h-5 text-purple-600 dark:text-purple-300" />
                      </div>
                      <div>
                        <p className="font-medium">{schedule.templateName}</p>
                        <p className="text-sm text-gray-500">
                          {schedule.stateCode} • {getFrequencyLabel(schedule.frequency)}
                        </p>
                      </div>
                    </div>
                    <div className="flex items-center gap-6">
                      <div className="text-right">
                        <p className="text-sm text-gray-500">Next Due</p>
                        <p className="font-medium">
                          {new Date(schedule.nextDueDate).toLocaleDateString()}
                        </p>
                      </div>
                      <Badge variant={schedule.status === 'active' ? 'success' : 'warning'}>
                        {schedule.status}
                      </Badge>
                      <div className="flex items-center gap-2">
                        {schedule.isAutoGenerate && (
                          <Badge variant="info" className="gap-1">
                            <Loader2 className="w-3 h-3" />
                            Auto
                          </Badge>
                        )}
                        {schedule.isAutoSubmit && (
                          <Badge variant="success" className="gap-1">
                            <Send className="w-3 h-3" />
                            Submit
                          </Badge>
                        )}
                      </div>
                    </div>
                  </div>
                </CardContent>
              </Card>
            ))}
            {schedules.length === 0 && (
              <div className="text-center py-12 text-gray-500">
                <Calendar className="w-12 h-12 mx-auto mb-4 text-gray-300" />
                <p>No schedules configured</p>
              </div>
            )}
          </div>
        </TabsContent>

        {/* Downloads Tab */}
        <TabsContent value="downloads" className="mt-6">
          <Card>
            <CardHeader>
              <CardTitle>Download Center</CardTitle>
              <CardDescription>Access all generated report files</CardDescription>
            </CardHeader>
            <CardContent>
              <div className="space-y-3">
                {reports
                  .filter(r => r.fileUrl)
                  .map((report) => (
                    <div
                      key={report.id}
                      className="flex items-center justify-between p-4 border rounded-lg hover:bg-gray-50 dark:hover:bg-gray-800"
                    >
                      <div className="flex items-center gap-3">
                        <FileDown className="w-5 h-5 text-blue-600" />
                        <div>
                          <p className="font-medium">{report.templateName}</p>
                          <p className="text-sm text-gray-500">
                            {new Date(report.reportingPeriod.startDate).toLocaleDateString()} - {new Date(report.reportingPeriod.endDate).toLocaleDateString()}
                            • {report.format.toUpperCase()}
                          </p>
                        </div>
                      </div>
                      <Button
                        variant="outline"
                        size="sm"
                        onClick={() => handleDownload(report.id, `${report.templateName}.${report.format}`)}
                      >
                        <Download className="w-4 h-4 mr-2" />
                        Download
                      </Button>
                    </div>
                  ))}
                {reports.filter(r => r.fileUrl).length === 0 && (
                  <div className="text-center py-8 text-gray-500">
                    <FileDown className="w-12 h-12 mx-auto mb-4 text-gray-300" />
                    <p>No files available for download</p>
                  </div>
                )}
              </div>
            </CardContent>
          </Card>
        </TabsContent>
      </Tabs>

      {/* Generate Modal */}
      <Dialog open={isGenerateModalOpen} onOpenChange={setIsGenerateModalOpen}>
        <DialogContent>
          <DialogHeader>
            <DialogTitle>Generate Report</DialogTitle>
          </DialogHeader>
          {selectedTemplate && (
            <form onSubmit={handleGenerate} className="space-y-4">
              <div className="p-3 bg-blue-50 dark:bg-blue-900/20 rounded-lg">
                <p className="font-medium">{selectedTemplate.name}</p>
                <p className="text-sm text-gray-500">{selectedTemplate.description}</p>
              </div>
              <div className="grid grid-cols-2 gap-4">
                <div>
                  <Label htmlFor="startDate">Start Date *</Label>
                  <Input
                    id="startDate"
                    type="date"
                    value={generateForm.startDate}
                    onChange={(e) => setGenerateForm({ ...generateForm, startDate: e.target.value })}
                    required
                  />
                </div>
                <div>
                  <Label htmlFor="endDate">End Date *</Label>
                  <Input
                    id="endDate"
                    type="date"
                    value={generateForm.endDate}
                    onChange={(e) => setGenerateForm({ ...generateForm, endDate: e.target.value })}
                    required
                  />
                </div>
              </div>
              <div>
                <Label htmlFor="format">Output Format</Label>
                <Select
                  value={generateForm.format}
                  onValueChange={(v) => setGenerateForm({ ...generateForm, format: v })}
                >
                  <SelectTrigger>
                    <SelectValue />
                  </SelectTrigger>
                  <SelectContent>
                    <SelectItem value="pdf">PDF</SelectItem>
                    <SelectItem value="csv">CSV</SelectItem>
                    <SelectItem value="xml">XML</SelectItem>
                    <SelectItem value="json">JSON</SelectItem>
                  </SelectContent>
                </Select>
              </div>
              <DialogFooter>
                <Button type="button" variant="outline" onClick={() => setIsGenerateModalOpen(false)}>
                  Cancel
                </Button>
                <Button type="submit" disabled={generating}>
                  {generating ? (
                    <>
                      <Loader2 className="w-4 h-4 mr-2 animate-spin" />
                      Generating...
                    </>
                  ) : (
                    <>
                      <FileText className="w-4 h-4 mr-2" />
                      Generate
                    </>
                  )}
                </Button>
              </DialogFooter>
            </form>
          )}
        </DialogContent>
      </Dialog>
    </div>
  );
}

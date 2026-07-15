/**
 * Widget Configuration Modal
 * 
 * Per-widget settings and configuration interface.
 */

'use client';

import React, { useState, useEffect } from 'react';
import { clsx } from 'clsx';
import {
  X,
  Save,
  RefreshCw,
  Layout,
  Palette,
  Database,
  Clock,
  Filter,
  BarChart2,
} from 'lucide-react';
import type {
  WidgetInstance,
  WidgetConfig,
  WidgetType,
  RefreshInterval,
} from './types';
import { WIDGET_REGISTRY } from './widgetRegistry';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Label } from '@/components/ui/label';
import { Switch } from '@/components/ui/switch';
import { Slider } from '@/components/ui/slider';
import {
  Dialog,
  DialogContent,
  DialogHeader,
  DialogTitle,
} from '@/components/ui/dialog';
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from '@/components/ui/select';
import {
  Tabs,
  TabsContent,
  TabsList,
  TabsTrigger,
} from '@/components/ui/tabs';
import { Badge } from '@/components/ui/badge';

interface WidgetConfigModalProps {
  widget: WidgetInstance | null;
  isOpen: boolean;
  onClose: () => void;
  onSave: (widgetId: string, config: WidgetConfig) => void;
}

// Refresh interval options
const REFRESH_INTERVALS: { value: RefreshInterval; label: string }[] = [
  { value: 'off', label: 'Off' },
  { value: '30s', label: '30 seconds' },
  { value: '1m', label: '1 minute' },
  { value: '5m', label: '5 minutes' },
  { value: '15m', label: '15 minutes' },
  { value: '30m', label: '30 minutes' },
  { value: '1h', label: '1 hour' },
];

// Chart type options
const CHART_TYPES = [
  { value: 'line', label: 'Line Chart' },
  { value: 'bar', label: 'Bar Chart' },
  { value: 'pie', label: 'Pie Chart' },
  { value: 'area', label: 'Area Chart' },
  { value: 'donut', label: 'Donut Chart' },
];

// Activity type options
const ACTIVITY_TYPES = [
  'medication_administered',
  'medication_refused',
  'incident_reported',
  'log_entry_created',
  'shift_started',
  'individual_check_in',
];

// Severity options
const SEVERITY_OPTIONS = ['info', 'success', 'warning', 'error', 'critical'];

export function WidgetConfigModal({
  widget,
  isOpen,
  onClose,
  onSave,
}: WidgetConfigModalProps) {
  const [config, setConfig] = useState<WidgetConfig>({});
  const [activeTab, setActiveTab] = useState('general');

  const widgetDef = widget ? WIDGET_REGISTRY[widget.type] : null;

  // Reset config when widget changes
  useEffect(() => {
    if (widget) {
      setConfig({ ...widget.config });
    }
  }, [widget]);

  if (!widget || !widgetDef) return null;

  const handleSave = () => {
    onSave(widget.id, config);
    onClose();
  };

  const updateConfig = (updates: Partial<WidgetConfig>) => {
    setConfig((prev) => ({ ...prev, ...updates }));
  };

  const updateDisplay = (updates: Partial<WidgetConfig['display']>) => {
    setConfig((prev) => ({
      ...prev,
      display: { ...prev.display, ...updates },
    }));
  };

  return (
    <Dialog open={isOpen} onOpenChange={onClose}>
      <DialogContent className="max-w-lg max-h-[90vh] overflow-hidden flex flex-col">
        <DialogHeader>
          <DialogTitle className="flex items-center gap-2">
            <span>Configure Widget</span>
            <Badge variant="secondary" className="text-xs">
              {widgetDef.name}
            </Badge>
          </DialogTitle>
        </DialogHeader>

        <Tabs value={activeTab} onValueChange={setActiveTab} className="flex-1 flex flex-col min-h-0">
          <TabsList className="grid grid-cols-4">
            <TabsTrigger value="general">
              <Layout className="h-4 w-4 mr-1" />
              General
            </TabsTrigger>
            <TabsTrigger value="display">
              <Palette className="h-4 w-4 mr-1" />
              Display
            </TabsTrigger>
            <TabsTrigger value="data">
              <Database className="h-4 w-4 mr-1" />
              Data
            </TabsTrigger>
            <TabsTrigger value="refresh">
              <Clock className="h-4 w-4 mr-1" />
              Refresh
            </TabsTrigger>
          </TabsList>

          <div className="flex-1 overflow-y-auto py-4">
            {/* General Settings */}
            <TabsContent value="general" className="space-y-4 mt-0">
              <div className="space-y-2">
                <Label htmlFor="title">Widget Title</Label>
                <Input
                  id="title"
                  value={config.title || ''}
                  onChange={(e) => updateConfig({ title: e.target.value })}
                  placeholder={widgetDef.name}
                />
              </div>

              <div className="space-y-2">
                <Label htmlFor="description">Description</Label>
                <Input
                  id="description"
                  value={config.description || ''}
                  onChange={(e) => updateConfig({ description: e.target.value })}
                  placeholder="Optional description"
                />
              </div>

              {/* Widget-specific settings */}
              {widget.type === 'stats' && (
                <div className="space-y-4 pt-4 border-t">
                  <Label>Metrics Configuration</Label>
                  <p className="text-sm text-gray-500">
                    Configure metrics in the main dashboard settings.
                  </p>
                </div>
              )}

              {widget.type === 'todo' && (
                <div className="space-y-4 pt-4 border-t">
                  <div className="space-y-2">
                    <Label>Sort By</Label>
                    <Select
                      value={(config as any).sortBy || 'dueDate'}
                      onValueChange={(value) =>
                        updateConfig({ sortBy: value as any })
                      }
                    >
                      <SelectTrigger>
                        <SelectValue />
                      </SelectTrigger>
                      <SelectContent>
                        <SelectItem value="dueDate">Due Date</SelectItem>
                        <SelectItem value="priority">Priority</SelectItem>
                        <SelectItem value="created">Created Date</SelectItem>
                      </SelectContent>
                    </Select>
                  </div>

                  <div className="flex items-center justify-between">
                    <Label htmlFor="showCompleted">Show Completed Tasks</Label>
                    <Switch
                      id="showCompleted"
                      checked={(config as any).showCompleted || false}
                      onCheckedChange={(checked) =>
                        updateConfig({ showCompleted: checked })
                      }
                    />
                  </div>
                </div>
              )}

              {widget.type === 'activity' && (
                <div className="space-y-4 pt-4 border-t">
                  <div className="space-y-2">
                    <Label>Activity Limit</Label>
                    <Slider
                      value={[(config as any).limit || 5]}
                      min={1}
                      max={20}
                      step={1}
                      onValueChange={([value]) =>
                        updateConfig({ limit: value })
                      }
                    />
                    <p className="text-xs text-gray-500">
                      Show {(config as any).limit || 5} activities
                    </p>
                  </div>

                  <div className="flex items-center justify-between">
                    <Label htmlFor="showViewAll">Show View All Link</Label>
                    <Switch
                      id="showViewAll"
                      checked={(config as any).showViewAll !== false}
                      onCheckedChange={(checked) =>
                        updateConfig({ showViewAll: checked })
                      }
                    />
                  </div>

                  <div className="flex items-center justify-between">
                    <Label htmlFor="groupByDate">Group by Date</Label>
                    <Switch
                      id="groupByDate"
                      checked={(config as any).groupByDate !== false}
                      onCheckedChange={(checked) =>
                        updateConfig({ groupByDate: checked })
                      }
                    />
                  </div>
                </div>
              )}
            </TabsContent>

            {/* Display Settings */}
            <TabsContent value="display" className="space-y-4 mt-0">
              <div className="space-y-4">
                <div className="flex items-center justify-between">
                  <Label htmlFor="showHeader">Show Header</Label>
                  <Switch
                    id="showHeader"
                    checked={config.display?.showHeader !== false}
                    onCheckedChange={(checked) =>
                      updateDisplay({ showHeader: checked })
                    }
                  />
                </div>

                <div className="flex items-center justify-between">
                  <Label htmlFor="showBorder">Show Border</Label>
                  <Switch
                    id="showBorder"
                    checked={config.display?.showBorder !== false}
                    onCheckedChange={(checked) =>
                      updateDisplay({ showBorder: checked })
                    }
                  />
                </div>

                <div className="flex items-center justify-between">
                  <Label htmlFor="compact">Compact Mode</Label>
                  <Switch
                    id="compact"
                    checked={config.display?.compact || false}
                    onCheckedChange={(checked) =>
                      updateDisplay({ compact: checked })
                    }
                  />
                </div>

                <div className="space-y-2 pt-4 border-t">
                  <Label>Theme</Label>
                  <Select
                    value={config.display?.theme || 'default'}
                    onValueChange={(value) =>
                      updateDisplay({ theme: value as any })
                    }
                  >
                    <SelectTrigger>
                      <SelectValue />
                    </SelectTrigger>
                    <SelectContent>
                      <SelectItem value="default">Default</SelectItem>
                      <SelectItem value="muted">Muted</SelectItem>
                      <SelectItem value="accent">Accent</SelectItem>
                      <SelectItem value="gradient">Gradient</SelectItem>
                    </SelectContent>
                  </Select>
                </div>
              </div>

              {widget.type === 'chart' && (
                <div className="space-y-4 pt-4 border-t">
                  <div className="space-y-2">
                    <Label>Chart Type</Label>
                    <Select
                      value={(config as any).chartType || 'line'}
                      onValueChange={(value) =>
                        updateConfig({ chartType: value as any })
                      }
                    >
                      <SelectTrigger>
                        <SelectValue />
                      </SelectTrigger>
                      <SelectContent>
                        {CHART_TYPES.map((type) => (
                          <SelectItem key={type.value} value={type.value}>
                            {type.label}
                          </SelectItem>
                        ))}
                      </SelectContent>
                    </Select>
                  </div>

                  <div className="flex items-center justify-between">
                    <Label htmlFor="showLegend">Show Legend</Label>
                    <Switch
                      id="showLegend"
                      checked={(config as any).showLegend !== false}
                      onCheckedChange={(checked) =>
                        updateConfig({ showLegend: checked })
                      }
                    />
                  </div>

                  <div className="flex items-center justify-between">
                    <Label htmlFor="showGrid">Show Grid</Label>
                    <Switch
                      id="showGrid"
                      checked={(config as any).showGrid !== false}
                      onCheckedChange={(checked) =>
                        updateConfig({ showGrid: checked })
                      }
                    />
                  </div>

                  <div className="flex items-center justify-between">
                    <Label htmlFor="animate">Animate</Label>
                    <Switch
                      id="animate"
                      checked={(config as any).animate !== false}
                      onCheckedChange={(checked) =>
                        updateConfig({ animate: checked })
                      }
                    />
                  </div>
                </div>
              )}
            </TabsContent>

            {/* Data Settings */}
            <TabsContent value="data" className="space-y-4 mt-0">
              <div className="space-y-2">
                <Label>Data Source</Label>
                <Select
                  value={config.dataSource?.type || 'api'}
                  onValueChange={(value) =>
                    updateConfig({
                      dataSource: { ...config.dataSource, type: value as any },
                    })
                  }
                >
                  <SelectTrigger>
                    <SelectValue />
                  </SelectTrigger>
                  <SelectContent>
                    <SelectItem value="api">API</SelectItem>
                    <SelectItem value="websocket">WebSocket</SelectItem>
                    <SelectItem value="static">Static</SelectItem>
                    <SelectItem value="computed">Computed</SelectItem>
                  </SelectContent>
                </Select>
              </div>

              {widget.type === 'activity' && (
                <div className="space-y-4 pt-4 border-t">
                  <Label>Filter by Activity Type</Label>
                  <div className="flex flex-wrap gap-2">
                    {ACTIVITY_TYPES.map((type) => (
                      <Badge
                        key={type}
                        variant={(config as any).filterTypes?.includes(type) ? 'default' : 'outline'}
                        className="cursor-pointer"
                        onClick={() => {
                          const current = (config as any).filterTypes || [];
                          const updated = current.includes(type)
                            ? current.filter((t: string) => t !== type)
                            : [...current, type];
                          updateConfig({ filterTypes: updated });
                        }}
                      >
                        {type.replace(/_/g, ' ')}
                      </Badge>
                    ))}
                  </div>
                </div>
              )}

              {widget.type === 'alerts' && (
                <div className="space-y-4 pt-4 border-t">
                  <Label>Filter by Severity</Label>
                  <div className="flex flex-wrap gap-2">
                    {SEVERITY_OPTIONS.map((severity) => (
                      <Badge
                        key={severity}
                        variant={(config as any).severity?.includes(severity) ? 'default' : 'outline'}
                        className="cursor-pointer capitalize"
                        onClick={() => {
                          const current = (config as any).severity || [];
                          const updated = current.includes(severity)
                            ? current.filter((s: string) => s !== severity)
                            : [...current, severity];
                          updateConfig({ severity: updated });
                        }}
                      >
                        {severity}
                      </Badge>
                    ))}
                  </div>
                </div>
              )}
            </TabsContent>

            {/* Refresh Settings */}
            <TabsContent value="refresh" className="space-y-4 mt-0">
              <div className="space-y-2">
                <Label>Refresh Interval</Label>
                <Select
                  value={config.refreshInterval || '5m'}
                  onValueChange={(value) =>
                    updateConfig({ refreshInterval: value as RefreshInterval })
                  }
                >
                  <SelectTrigger>
                    <SelectValue />
                  </SelectTrigger>
                  <SelectContent>
                    {REFRESH_INTERVALS.map((interval) => (
                      <SelectItem key={interval.value} value={interval.value}>
                        {interval.label}
                      </SelectItem>
                    ))}
                  </SelectContent>
                </Select>
                <p className="text-xs text-gray-500">
                  How often the widget should refresh its data
                </p>
              </div>

              {widget.type === 'activity' && (
                <div className="space-y-4 pt-4 border-t">
                  <div className="flex items-center justify-between">
                    <div>
                      <Label htmlFor="enableRealtime">Real-time Updates</Label>
                      <p className="text-xs text-gray-500">
                        Receive updates via WebSocket
                      </p>
                    </div>
                    <Switch
                      id="enableRealtime"
                      checked={(config as any).enableRealtime !== false}
                      onCheckedChange={(checked) =>
                        updateConfig({ enableRealtime: checked })
                      }
                    />
                  </div>
                </div>
              )}
            </TabsContent>
          </div>
        </Tabs>

        {/* Footer */}
        <div className="flex justify-end gap-2 pt-4 border-t">
          <Button variant="outline" onClick={onClose}>
            Cancel
          </Button>
          <Button onClick={handleSave}>
            <Save className="h-4 w-4 mr-2" />
            Save Changes
          </Button>
        </div>
      </DialogContent>
    </Dialog>
  );
}

export default WidgetConfigModal;

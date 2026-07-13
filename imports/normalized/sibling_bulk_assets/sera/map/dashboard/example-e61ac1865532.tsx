/**
 * Dashboard Example
 * 
 * Example usage of the dashboard system.
 * This file demonstrates how to integrate the dashboard into a page.
 */

'use client';

import React, { useState } from 'react';
import { clsx } from 'clsx';
import {
  LayoutDashboard,
  Plus,
  RotateCcw,
  Save,
  Download,
  Upload,
  Undo,
  Redo,
} from 'lucide-react';
import {
  DashboardGrid,
  WidgetConfigModal,
  useDashboard,
  getWidgetsByRole,
  WIDGET_REGISTRY,
} from './index';
import { Button } from '@/components/ui/button';
import {
  Dialog,
  DialogContent,
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
import { Badge } from '@/components/ui/badge';
import { Tabs, TabsList, TabsTrigger } from '@/components/ui/tabs';
import type { UserRole, WidgetType } from './types';

interface DashboardExampleProps {
  userId: string;
  userRole: UserRole;
}

export function DashboardExample({ userId, userRole }: DashboardExampleProps) {
  const [showAddWidget, setShowAddWidget] = useState(false);
  
  const dashboard = useDashboard({
    userId,
    role: userRole,
    enableAutoSave: true,
    autoSaveInterval: 30000,
  });

  // Get available widgets for this role
  const availableWidgets = getWidgetsByRole(userRole);

  const handleAddWidget = (type: WidgetType) => {
    dashboard.addWidget(type);
    setShowAddWidget(false);
  };

  const handleImportLayout = (event: React.ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    if (file) {
      const reader = new FileReader();
      reader.onload = (e) => {
        const json = e.target?.result as string;
        if (dashboard.importNewLayout(json)) {
          alert('Layout imported successfully!');
        } else {
          alert('Failed to import layout. Invalid format.');
        }
      };
      reader.readAsText(file);
    }
  };

  return (
    <div className="min-h-screen bg-gray-50">
      {/* Header */}
      <header className="bg-white border-b sticky top-0 z-40">
        <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
          <div className="flex items-center justify-between h-16">
            {/* Left: Title and Layout Selector */}
            <div className="flex items-center gap-4">
              <div className="flex items-center gap-2">
                <LayoutDashboard className="h-6 w-6 text-blue-600" />
                <h1 className="text-xl font-semibold">Dashboard</h1>
              </div>

              {dashboard.preferences && dashboard.preferences.layouts.length > 1 && (
                <Select
                  value={dashboard.layout.id}
                  onValueChange={dashboard.changeLayout}
                >
                  <SelectTrigger className="w-48">
                    <SelectValue />
                  </SelectTrigger>
                  <SelectContent>
                    {dashboard.preferences.layouts.map((layout) => (
                      <SelectItem key={layout.id} value={layout.id}>
                        {layout.name}
                      </SelectItem>
                    ))}
                  </SelectContent>
                </Select>
              )}
            </div>

            {/* Center: Column Layout */}
            {dashboard.isEditing && (
              <Tabs
                value={dashboard.layout.columns.toString()}
                onValueChange={(v) => {
                  const newLayout = { ...dashboard.layout, columns: parseInt(v) as 1 | 2 | 3 | 4 };
                  dashboard.updateWidget('__layout__', { ...newLayout } as any);
                }}
              >
                <TabsList>
                  <TabsTrigger value="1">1 Col</TabsTrigger>
                  <TabsTrigger value="2">2 Col</TabsTrigger>
                  <TabsTrigger value="3">3 Col</TabsTrigger>
                  <TabsTrigger value="4">4 Col</TabsTrigger>
                </TabsList>
              </Tabs>
            )}

            {/* Right: Actions */}
            <div className="flex items-center gap-2">
              {/* Undo/Redo */}
              {dashboard.isEditing && (
                <>
                  <Button
                    variant="ghost"
                    size="icon"
                    onClick={dashboard.undo}
                    disabled={!dashboard.canUndo}
                    title="Undo"
                  >
                    <Undo className="h-4 w-4" />
                  </Button>
                  <Button
                    variant="ghost"
                    size="icon"
                    onClick={dashboard.redo}
                    disabled={!dashboard.canRedo}
                    title="Redo"
                  >
                    <Redo className="h-4 w-4" />
                  </Button>
                </>
              )}

              {/* Add Widget */}
              <Dialog open={showAddWidget} onOpenChange={setShowAddWidget}>
                <DialogTrigger asChild>
                  <Button variant="outline" className="gap-2">
                    <Plus className="h-4 w-4" />
                    Add Widget
                  </Button>
                </DialogTrigger>
                <DialogContent className="max-w-2xl">
                  <DialogHeader>
                    <DialogTitle>Add Widget</DialogTitle>
                  </DialogHeader>
                  <div className="grid grid-cols-2 gap-4 py-4">
                    {availableWidgets.map((widget) => (
                      <button
                        key={widget.type}
                        onClick={() => handleAddWidget(widget.type)}
                        className={clsx(
                          'flex items-start gap-3 p-4 rounded-lg border text-left',
                          'hover:border-blue-500 hover:bg-blue-50 transition-colors'
                        )}
                      >
                        <div className="p-2 bg-white rounded-lg shadow-sm">
                          {/* Icon placeholder */}
                          <div className="h-5 w-5 text-gray-600" />
                        </div>
                        <div>
                          <h3 className="font-medium">{widget.name}</h3>
                          <p className="text-sm text-gray-500 mt-1">
                            {widget.description}
                          </p>
                          <Badge variant="ghost" className="mt-2 text-xs">
                            {widget.defaultSize}
                          </Badge>
                        </div>
                      </button>
                    ))}
                  </div>
                </DialogContent>
              </Dialog>

              {/* Import/Export */}
              <div className="flex items-center gap-1">
                <Button
                  variant="ghost"
                  size="icon"
                  onClick={() => {
                    const json = dashboard.exportCurrentLayout();
                    const blob = new Blob([json], { type: 'application/json' });
                    const url = URL.createObjectURL(blob);
                    const a = document.createElement('a');
                    a.href = url;
                    a.download = `dashboard-${dashboard.layout.name}.json`;
                    a.click();
                  }}
                  title="Export Layout"
                >
                  <Download className="h-4 w-4" />
                </Button>
                <label className="cursor-pointer">
                  <input
                    type="file"
                    accept=".json"
                    onChange={handleImportLayout}
                    className="hidden"
                  />
                  <Button variant="ghost" size="icon" title="Import Layout" asChild>
                    <span>
                      <Upload className="h-4 w-4" />
                    </span>
                  </Button>
                </label>
              </div>

              {/* Reset */}
              {dashboard.isEditing && (
                <Button
                  variant="ghost"
                  size="icon"
                  onClick={dashboard.resetLayout}
                  title="Reset to Default"
                >
                  <RotateCcw className="h-4 w-4" />
                </Button>
              )}

              {/* Edit Toggle */}
              <Button
                variant={dashboard.isEditing ? 'default' : 'outline'}
                onClick={() => dashboard.setEditing(!dashboard.isEditing)}
              >
                {dashboard.isEditing ? 'Done' : 'Edit Layout'}
              </Button>

              {/* Save */}
              {dashboard.isDirty && (
                <Button onClick={dashboard.saveLayout}>
                  <Save className="h-4 w-4 mr-2" />
                  Save
                </Button>
              )}
            </div>
          </div>
        </div>
      </header>

      {/* Main Content */}
      <main className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
        {/* Status bar */}
        {dashboard.isEditing && (
          <div className="mb-4 p-3 bg-blue-50 border border-blue-200 rounded-lg text-sm text-blue-800">
            <p className="flex items-center gap-2">
              <span className="font-medium">Editing Mode:</span>
              Drag widgets to reposition. Drag the bottom-right corner to resize.
              Click the gear icon to configure widget settings.
            </p>
          </div>
        )}

        {/* Dashboard Grid */}
        <DashboardGrid
          layout={dashboard.layout}
          isEditing={dashboard.isEditing}
          onMoveWidget={dashboard.moveWidget}
          onResizeWidget={dashboard.resizeWidget}
          onRemoveWidget={dashboard.removeWidget}
          onConfigureWidget={dashboard.openWidgetConfig}
          onDuplicateWidget={dashboard.duplicateWidget}
          onRefreshWidget={dashboard.onRefreshWidget}
        />

        {/* Empty state hint */}
        {dashboard.layout.widgets.length === 0 && !dashboard.isEditing && (
          <div className="text-center py-16">
            <LayoutDashboard className="h-16 w-16 mx-auto text-gray-300 mb-4" />
            <h2 className="text-xl font-semibold text-gray-600 mb-2">
              Your dashboard is empty
            </h2>
            <p className="text-gray-500 mb-4">
              Click &ldquo;Edit Layout&rdquo; to add widgets and customize your dashboard.
            </p>
            <Button onClick={() => dashboard.setEditing(true)}>
              <Plus className="h-4 w-4 mr-2" />
              Add Widgets
            </Button>
          </div>
        )}
      </main>

      {/* Widget Configuration Modal */}
      <WidgetConfigModal
        widget={dashboard.configuredWidgetId 
          ? dashboard.getWidget(dashboard.configuredWidgetId) 
          : null
        }
        isOpen={!!dashboard.configuredWidgetId}
        onClose={dashboard.closeWidgetConfig}
        onSave={(widgetId, config) =>
          dashboard.updateWidgetConfig(widgetId, config)
        }
      />
    </div>
  );
}

export default DashboardExample;

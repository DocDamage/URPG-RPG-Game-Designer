/**
 * Dashboard Grid
 * 
 * Drag-and-drop widget layout with responsive grid system.
 */

'use client';

import React, { useState, useCallback, useRef, useEffect, Suspense } from 'react';
import { clsx } from 'clsx';
import {
  GripVertical,
  Settings,
  X,
  Maximize2,
  Minimize2,
  RefreshCw,
  MoreHorizontal,
  Copy,
  Trash2,
} from 'lucide-react';
import type {
  DashboardLayout,
  WidgetInstance,
  WidgetPosition,
  UserRole,
} from './types';
import { WIDGET_REGISTRY } from './widgetRegistry';
import { Card } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import {
  DropdownMenu,
  DropdownMenuContent,
  DropdownMenuItem,
  DropdownMenuSeparator,
  DropdownMenuTrigger,
} from '@/components/ui/dropdown-menu';
import {
  Tooltip,
  TooltipContent,
  TooltipProvider,
  TooltipTrigger,
} from '@/components/ui/tooltip';
import { Skeleton } from '@/components/ui/skeleton';

// Grid cell size in pixels
const CELL_WIDTH = 100;
const CELL_HEIGHT = 50;
const GAP = 16;

interface DashboardGridProps {
  layout: DashboardLayout;
  isEditing: boolean;
  onMoveWidget: (widgetId: string, position: WidgetPosition) => void;
  onResizeWidget: (widgetId: string, size: Partial<WidgetPosition>) => void;
  onRemoveWidget: (widgetId: string) => void;
  onConfigureWidget: (widgetId: string) => void;
  onDuplicateWidget: (widgetId: string) => void;
  onRefreshWidget?: (widgetId: string) => void;
  className?: string;
}

export function DashboardGrid({
  layout,
  isEditing,
  onMoveWidget,
  onResizeWidget,
  onRemoveWidget,
  onConfigureWidget,
  onDuplicateWidget,
  onRefreshWidget,
  className,
}: DashboardGridProps) {
  const gridRef = useRef<HTMLDivElement>(null);
  const [dragState, setDragState] = useState<{
    widgetId: string | null;
    startX: number;
    startY: number;
    initialX: number;
    initialY: number;
    currentX: number;
    currentY: number;
  } | null>(null);

  const [resizeState, setResizeState] = useState<{
    widgetId: string | null;
    startX: number;
    startY: number;
    initialW: number;
    initialH: number;
    currentW: number;
    currentH: number;
  } | null>(null);

  // Calculate grid dimensions
  const gridColumns = layout.columns;
  const gridWidth = gridRef.current?.clientWidth || 1200;
  const cellWidth = (gridWidth - (gridColumns - 1) * GAP) / gridColumns;

  // Convert grid coordinates to pixels
  const gridToPixels = useCallback(
    (x: number, y: number, w: number, h: number) => ({
      left: x * (cellWidth + GAP),
      top: y * (CELL_HEIGHT + GAP),
      width: w * cellWidth + (w - 1) * GAP,
      height: h * CELL_HEIGHT + (h - 1) * GAP,
    }),
    [cellWidth]
  );

  // Convert pixels to grid coordinates
  const pixelsToGrid = useCallback(
    (left: number, top: number) => ({
      x: Math.round(left / (cellWidth + GAP)),
      y: Math.round(top / (CELL_HEIGHT + GAP)),
    }),
    [cellWidth]
  );

  // Handle drag start
  const handleDragStart = useCallback(
    (e: React.MouseEvent, widgetId: string) => {
      if (!isEditing) return;
      e.preventDefault();

      const widget = layout.widgets.find((w) => w.id === widgetId);
      if (!widget) return;

      const pixels = gridToPixels(
        widget.position.x,
        widget.position.y,
        widget.position.w,
        widget.position.h
      );

      setDragState({
        widgetId,
        startX: e.clientX,
        startY: e.clientY,
        initialX: pixels.left,
        initialY: pixels.top,
        currentX: pixels.left,
        currentY: pixels.top,
      });
    },
    [isEditing, layout.widgets, gridToPixels]
  );

  // Handle resize start
  const handleResizeStart = useCallback(
    (e: React.MouseEvent, widgetId: string) => {
      if (!isEditing) return;
      e.preventDefault();
      e.stopPropagation();

      const widget = layout.widgets.find((w) => w.id === widgetId);
      if (!widget) return;

      setResizeState({
        widgetId,
        startX: e.clientX,
        startY: e.clientY,
        initialW: widget.position.w,
        initialH: widget.position.h,
        currentW: widget.position.w,
        currentH: widget.position.h,
      });
    },
    [isEditing, layout.widgets]
  );

  // Handle mouse move
  useEffect(() => {
    const handleMouseMove = (e: MouseEvent) => {
      if (dragState) {
        const dx = e.clientX - dragState.startX;
        const dy = e.clientY - dragState.startY;

        setDragState((prev) =>
          prev
            ? {
                ...prev,
                currentX: prev.initialX + dx,
                currentY: prev.initialY + dy,
              }
            : null
        );
      }

      if (resizeState) {
        const dx = e.clientX - resizeState.startX;
        const dy = e.clientY - resizeState.startY;

        const dw = Math.round(dx / cellWidth);
        const dh = Math.round(dy / CELL_HEIGHT);

        setResizeState((prev) =>
          prev
            ? {
                ...prev,
                currentW: Math.max(1, prev.initialW + dw),
                currentH: Math.max(2, prev.initialH + dh),
              }
            : null
        );
      }
    };

    const handleMouseUp = () => {
      if (dragState?.widgetId) {
        const gridCoords = pixelsToGrid(dragState.currentX, dragState.currentY);
        const widget = layout.widgets.find((w) => w.id === dragState.widgetId);

        if (widget) {
          onMoveWidget(dragState.widgetId, {
            ...widget.position,
            x: Math.max(0, Math.min(gridColumns - widget.position.w, gridCoords.x)),
            y: Math.max(0, gridCoords.y),
          });
        }

        setDragState(null);
      }

      if (resizeState?.widgetId) {
        const widget = layout.widgets.find((w) => w.id === resizeState.widgetId);

        if (widget) {
          onResizeWidget(resizeState.widgetId, {
            w: Math.min(gridColumns, resizeState.currentW),
            h: resizeState.currentH,
          });
        }

        setResizeState(null);
      }
    };

    if (dragState || resizeState) {
      window.addEventListener('mousemove', handleMouseMove);
      window.addEventListener('mouseup', handleMouseUp);

      return () => {
        window.removeEventListener('mousemove', handleMouseMove);
        window.removeEventListener('mouseup', handleMouseUp);
      };
    }
  }, [dragState, resizeState, cellWidth, gridColumns, layout.widgets, onMoveWidget, onResizeWidget, pixelsToGrid]);

  // Calculate max height for grid
  const maxHeight = layout.widgets.reduce(
    (max, w) => Math.max(max, w.position.y + w.position.h),
    0
  );

  return (
    <div
      ref={gridRef}
      className={clsx(
        'relative w-full',
        isEditing && 'min-h-[600px]',
        className
      )}
      style={{
        minHeight: isEditing ? Math.max(600, maxHeight * (CELL_HEIGHT + GAP)) : undefined,
      }}
    >
      {/* Grid background when editing */}
      {isEditing && (
        <div
          className="absolute inset-0 pointer-events-none opacity-10"
          style={{
            backgroundImage: `
              linear-gradient(to right, #e5e7eb 1px, transparent 1px),
              linear-gradient(to bottom, #e5e7eb 1px, transparent 1px)
            `,
            backgroundSize: `${cellWidth + GAP}px ${CELL_HEIGHT + GAP}px`,
          }}
        />
      )}

      {/* Widgets */}
      {layout.widgets.map((widget) => {
        const isDragging = dragState?.widgetId === widget.id;
        const isResizing = resizeState?.widgetId === widget.id;
        const widgetDef = WIDGET_REGISTRY[widget.type];

        const position = isDragging
          ? {
              left: dragState.currentX,
              top: dragState.currentY,
              width: widget.position.w * cellWidth + (widget.position.w - 1) * GAP,
              height: widget.position.h * CELL_HEIGHT + (widget.position.h - 1) * GAP,
            }
          : gridToPixels(
              widget.position.x,
              widget.position.y,
              isResizing ? resizeState.currentW : widget.position.w,
              isResizing ? resizeState.currentH : widget.position.h
            );

        const WidgetComponent = widgetDef?.component;

        return (
          <div
            key={widget.id}
            className={clsx(
              'absolute transition-shadow',
              isEditing && 'cursor-move',
              (isDragging || isResizing) && 'z-50 shadow-2xl',
              isEditing && !isDragging && !isResizing && 'hover:shadow-lg'
            )}
            style={{
              left: position.left,
              top: position.top,
              width: position.width,
              height: position.height,
            }}
          >
            <Card
              className={clsx(
                'h-full overflow-hidden',
                isEditing && 'ring-2 ring-transparent hover:ring-blue-400',
                (isDragging || isResizing) && 'ring-2 ring-blue-500'
              )}
            >
              {/* Widget header with controls when editing */}
              {isEditing && (
                <div className="flex items-center justify-between px-2 py-1 bg-gray-50 border-b">
                  <div
                    className="flex items-center gap-1 cursor-grab active:cursor-grabbing"
                    onMouseDown={(e) => handleDragStart(e, widget.id)}
                  >
                    <GripVertical className="h-4 w-4 text-gray-400" />
                    <span className="text-xs font-medium text-gray-600 truncate max-w-[120px]">
                      {widget.config.title || widgetDef?.name || widget.type}
                    </span>
                  </div>

                  <div className="flex items-center gap-0.5">
                    <TooltipProvider>
                      {onRefreshWidget && (
                        <Tooltip>
                          <TooltipTrigger asChild>
                            <Button
                              variant="ghost"
                              size="icon"
                              className="h-6 w-6"
                              onClick={() => onRefreshWidget(widget.id)}
                            >
                              <RefreshCw className="h-3 w-3" />
                            </Button>
                          </TooltipTrigger>
                          <TooltipContent>Refresh</TooltipContent>
                        </Tooltip>
                      )}

                      <Tooltip>
                        <TooltipTrigger asChild>
                          <Button
                            variant="ghost"
                            size="icon"
                            className="h-6 w-6"
                            onClick={() => onConfigureWidget(widget.id)}
                          >
                            <Settings className="h-3 w-3" />
                          </Button>
                        </TooltipTrigger>
                        <TooltipContent>Configure</TooltipContent>
                      </Tooltip>

                      <DropdownMenu>
                        <DropdownMenuTrigger asChild>
                          <Button variant="ghost" size="icon" className="h-6 w-6">
                            <MoreHorizontal className="h-3 w-3" />
                          </Button>
                        </DropdownMenuTrigger>
                        <DropdownMenuContent align="end">
                          <DropdownMenuItem onClick={() => onDuplicateWidget(widget.id)}>
                            <Copy className="h-4 w-4 mr-2" />
                            Duplicate
                          </DropdownMenuItem>
                          <DropdownMenuItem onClick={() => onConfigureWidget(widget.id)}>
                            <Settings className="h-4 w-4 mr-2" />
                            Configure
                          </DropdownMenuItem>
                          <DropdownMenuSeparator />
                          <DropdownMenuItem
                            onClick={() => onRemoveWidget(widget.id)}
                            className="text-red-600"
                          >
                            <Trash2 className="h-4 w-4 mr-2" />
                            Remove
                          </DropdownMenuItem>
                        </DropdownMenuContent>
                      </DropdownMenu>

                      <Tooltip>
                        <TooltipTrigger asChild>
                          <Button
                            variant="ghost"
                            size="icon"
                            className="h-6 w-6 text-red-500 hover:text-red-600"
                            onClick={() => onRemoveWidget(widget.id)}
                          >
                            <X className="h-3 w-3" />
                          </Button>
                        </TooltipTrigger>
                        <TooltipContent>Remove</TooltipContent>
                      </Tooltip>
                    </TooltipProvider>
                  </div>
                </div>
              )}

              {/* Widget content */}
              <div className={clsx('h-full', isEditing && 'pt-0')}>
                {WidgetComponent ? (
                  <Suspense
                    fallback={
                      <div className="h-full p-4">
                        <Skeleton className="h-full w-full" />
                      </div>
                    }
                  >
                    <WidgetComponent
                      widgetId={widget.id}
                      config={widget.config}
                      className="h-full border-0 shadow-none"
                    />
                  </Suspense>
                ) : (
                  <div className="flex items-center justify-center h-full text-gray-400">
                    <p className="text-sm">Widget not found</p>
                  </div>
                )}
              </div>

              {/* Resize handle */}
              {isEditing && (
                <div
                  className="absolute bottom-0 right-0 w-4 h-4 cursor-se-resize"
                  onMouseDown={(e) => handleResizeStart(e, widget.id)}
                >
                  <div className="absolute bottom-1 right-1 w-2 h-2 bg-gray-400 rounded-full" />
                </div>
              )}
            </Card>
          </div>
        );
      })}

      {/* Empty state */}
      {layout.widgets.length === 0 && (
        <div className="flex flex-col items-center justify-center h-64 text-gray-400">
          <div className="p-4 rounded-full bg-gray-100 mb-4">
            <GripVertical className="h-8 w-8" />
          </div>
          <p className="text-sm font-medium">No widgets added</p>
          <p className="text-xs mt-1">Click &ldquo;Add Widget&rdquo; to get started</p>
        </div>
      )}
    </div>
  );
}

export default DashboardGrid;

/**
 * Chart Widget
 * 
 * Line/bar/pie charts for data visualization.
 */

'use client';

import React, { useMemo } from 'react';
import { clsx } from 'clsx';
import type { WidgetProps, ChartWidgetConfig, ChartData } from '../types';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Badge } from '@/components/ui/badge';

// Mock chart data
const mockChartData: Record<string, ChartData> = {
  incidents: {
    labels: ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'],
    datasets: [
      {
        label: 'Incidents',
        data: [2, 1, 3, 0, 2, 1, 2],
        color: '#ef4444',
      },
      {
        label: 'Near Misses',
        data: [3, 2, 1, 2, 3, 2, 1],
        color: '#f59e0b',
      },
    ],
  },
  medications: {
    labels: ['Given', 'Refused', 'Missed', 'Pending'],
    datasets: [
      {
        label: 'Medications',
        data: [145, 8, 3, 12],
        color: '#3b82f6',
      },
    ],
  },
  activity: {
    labels: ['6am', '9am', '12pm', '3pm', '6pm', '9pm'],
    datasets: [
      {
        label: 'Log Entries',
        data: [12, 28, 35, 22, 18, 8],
        color: '#10b981',
        fill: true,
      },
    ],
  },
};

// Simple bar chart component
function BarChart({ data, showGrid, animate }: { data: ChartData; showGrid?: boolean; animate?: boolean }) {
  const maxValue = Math.max(...data.datasets.flatMap((d) => d.data));
  const colors = ['#3b82f6', '#10b981', '#f59e0b', '#ef4444', '#8b5cf6', '#ec4899'];

  return (
    <div className="h-full flex flex-col">
      {/* Y-axis labels */}
      <div className="flex-1 flex">
        <div className="flex flex-col justify-between text-xs text-gray-400 pr-2">
          {[...Array(5)].map((_, i) => (
            <span key={i}>{Math.round((maxValue / 4) * (4 - i))}</span>
          ))}
        </div>

        {/* Chart area */}
        <div className="flex-1 relative">
          {/* Grid lines */}
          {showGrid && (
            <div className="absolute inset-0 flex flex-col justify-between">
              {[...Array(5)].map((_, i) => (
                <div key={i} className="border-t border-gray-100" />
              ))}
            </div>
          )}

          {/* Bars */}
          <div className="absolute inset-0 flex items-end justify-around gap-2">
            {data.labels.map((label, index) => (
              <div key={label} className="flex-1 flex flex-col items-center gap-1">
                <div className="w-full flex justify-center gap-0.5 h-full items-end">
                  {data.datasets.map((dataset, datasetIndex) => {
                    const value = dataset.data[index];
                    const height = maxValue > 0 ? (value / maxValue) * 100 : 0;
                    const color = dataset.color || colors[datasetIndex % colors.length];

                    return (
                      <div
                        key={datasetIndex}
                        className={clsx(
                          'rounded-t transition-all',
                          animate && 'animate-grow'
                        )}
                        style={{
                          height: `${height}%`,
                          backgroundColor: color,
                          width: `${100 / data.datasets.length - 5}%`,
                          animationDelay: `${index * 50}ms`,
                        }}
                      />
                    );
                  })}
                </div>
              </div>
            ))}
          </div>
        </div>
      </div>

      {/* X-axis labels */}
      <div className="flex justify-around mt-2 pt-2 border-t">
        {data.labels.map((label) => (
          <span key={label} className="text-xs text-gray-500 text-center flex-1">
            {label}
          </span>
        ))}
      </div>
    </div>
  );
}

// Simple line chart component
function LineChart({ data, showGrid, animate, fill }: { data: ChartData; showGrid?: boolean; animate?: boolean; fill?: boolean }) {
  const maxValue = Math.max(...data.datasets.flatMap((d) => d.data));
  const colors = ['#3b82f6', '#10b981', '#f59e0b', '#ef4444', '#8b5cf6', '#ec4899'];

  return (
    <div className="h-full flex flex-col">
      {/* Y-axis labels */}
      <div className="flex-1 flex">
        <div className="flex flex-col justify-between text-xs text-gray-400 pr-2">
          {[...Array(5)].map((_, i) => (
            <span key={i}>{Math.round((maxValue / 4) * (4 - i))}</span>
          ))}
        </div>

        {/* Chart area */}
        <div className="flex-1 relative">
          {/* Grid lines */}
          {showGrid && (
            <div className="absolute inset-0 flex flex-col justify-between">
              {[...Array(5)].map((_, i) => (
                <div key={i} className="border-t border-gray-100" />
              ))}
            </div>
          )}

          {/* Lines */}
          <svg className="absolute inset-0 w-full h-full" preserveAspectRatio="none">
            {data.datasets.map((dataset, datasetIndex) => {
              const color = dataset.color || colors[datasetIndex % colors.length];
              const points = dataset.data.map((value, index) => {
                const x = (index / (dataset.data.length - 1)) * 100;
                const y = maxValue > 0 ? 100 - (value / maxValue) * 100 : 100;
                return `${x},${y}`;
              }).join(' ');

              const fillPoints = fill
                ? `0,100 ${points} 100,100`
                : points;

              return (
                <g key={datasetIndex}>
                  {fill && (
                    <polygon
                      points={fillPoints}
                      fill={color}
                      fillOpacity="0.1"
                      className={animate ? 'animate-fade-in' : ''}
                    />
                  )}
                  <polyline
                    points={points}
                    fill="none"
                    stroke={color}
                    strokeWidth="2"
                    strokeLinecap="round"
                    strokeLinejoin="round"
                    className={animate ? 'animate-draw' : ''}
                    style={{ animationDelay: `${datasetIndex * 100}ms` }}
                  />
                  {/* Data points */}
                  {dataset.data.map((value, index) => {
                    const x = (index / (dataset.data.length - 1)) * 100;
                    const y = maxValue > 0 ? 100 - (value / maxValue) * 100 : 100;
                    return (
                      <circle
                        key={index}
                        cx={`${x}%`}
                        cy={`${y}%`}
                        r="3"
                        fill={color}
                        className={animate ? 'animate-fade-in' : ''}
                        style={{ animationDelay: `${(index * 50) + (datasetIndex * 100)}ms` }}
                      />
                    );
                  })}
                </g>
              );
            })}
          </svg>
        </div>
      </div>

      {/* X-axis labels */}
      <div className="flex justify-between mt-2 pt-2 border-t">
        {data.labels.map((label) => (
          <span key={label} className="text-xs text-gray-500">
            {label}
          </span>
        ))}
      </div>
    </div>
  );
}

// Simple pie/donut chart component
function PieChart({ data, animate, donut }: { data: ChartData; animate?: boolean; donut?: boolean }) {
  const total = data.datasets[0].data.reduce((a, b) => a + b, 0);
  const colors = ['#3b82f6', '#10b981', '#f59e0b', '#ef4444', '#8b5cf6', '#ec4899'];

  let currentAngle = 0;

  return (
    <div className="h-full flex">
      {/* Chart */}
      <div className="flex-1 flex items-center justify-center">
        <svg viewBox="0 0 100 100" className="w-full h-full max-h-[200px]">
          {data.datasets[0].data.map((value, index) => {
            const percentage = total > 0 ? value / total : 0;
            const angle = percentage * 360;
            const startAngle = currentAngle;
            const endAngle = currentAngle + angle;
            currentAngle += angle;

            const startRad = ((startAngle - 90) * Math.PI) / 180;
            const endRad = ((endAngle - 90) * Math.PI) / 180;

            const x1 = 50 + 40 * Math.cos(startRad);
            const y1 = 50 + 40 * Math.sin(startRad);
            const x2 = 50 + 40 * Math.cos(endRad);
            const y2 = 50 + 40 * Math.sin(endRad);

            const largeArc = angle > 180 ? 1 : 0;

            const pathData = donut
              ? [
                  `M ${50 + 25 * Math.cos(startRad)} ${50 + 25 * Math.sin(startRad)}`,
                  `L ${x1} ${y1}`,
                  `A 40 40 0 ${largeArc} 1 ${x2} ${y2}`,
                  `L ${50 + 25 * Math.cos(endRad)} ${50 + 25 * Math.sin(endRad)}`,
                  `A 25 25 0 ${largeArc} 0 ${50 + 25 * Math.cos(startRad)} ${50 + 25 * Math.sin(startRad)}`,
                  'Z',
                ].join(' ')
              : [
                  `M 50 50`,
                  `L ${x1} ${y1}`,
                  `A 40 40 0 ${largeArc} 1 ${x2} ${y2}`,
                  'Z',
                ].join(' ');

            return (
              <path
                key={index}
                d={pathData}
                fill={colors[index % colors.length]}
                stroke="white"
                strokeWidth="2"
                className={animate ? 'animate-scale-in' : ''}
                style={{ animationDelay: `${index * 100}ms`, transformOrigin: 'center' }}
              />
            );
          })}
        </svg>
      </div>

      {/* Legend */}
      <div className="w-1/3 flex flex-col justify-center gap-2 pl-4">
        {data.labels.map((label, index) => {
          const value = data.datasets[0].data[index];
          const percentage = total > 0 ? Math.round((value / total) * 100) : 0;
          const color = colors[index % colors.length];

          return (
            <div key={label} className="flex items-center gap-2">
              <span
                className="w-3 h-3 rounded-full shrink-0"
                style={{ backgroundColor: color }}
              />
              <div className="min-w-0">
                <p className="text-xs font-medium truncate">{label}</p>
                <p className="text-[10px] text-gray-400">
                  {value} ({percentage}%)
                </p>
              </div>
            </div>
          );
        })}
      </div>
    </div>
  );
}

export function ChartWidget({ config, className }: WidgetProps) {
  const chartConfig = config as ChartWidgetConfig;

  // Get data based on config (mock implementation)
  const data = useMemo(() => {
    if (chartConfig.dataKeys?.[0]) {
      return mockChartData[chartConfig.dataKeys[0]] || mockChartData.incidents;
    }
    return mockChartData.incidents;
  }, [chartConfig.dataKeys]);

  const renderChart = () => {
    switch (chartConfig.chartType) {
      case 'bar':
        return (
          <BarChart
            data={data}
            showGrid={chartConfig.showGrid}
            animate={chartConfig.animate}
          />
        );
      case 'line':
      case 'area':
        return (
          <LineChart
            data={data}
            showGrid={chartConfig.showGrid}
            animate={chartConfig.animate}
            fill={chartConfig.chartType === 'area'}
          />
        );
      case 'pie':
      case 'donut':
        return (
          <PieChart
            data={data}
            animate={chartConfig.animate}
            donut={chartConfig.chartType === 'donut'}
          />
        );
      default:
        return <BarChart data={data} showGrid={chartConfig.showGrid} animate={chartConfig.animate} />;
    }
  };

  return (
    <Card className={clsx('h-full flex flex-col', className)}>
      {config.display?.showHeader !== false && (
        <CardHeader className="pb-3">
          <div className="flex items-center justify-between">
            <CardTitle className="text-lg font-semibold">
              {config.title || 'Chart'}
            </CardTitle>
            {chartConfig.showLegend && chartConfig.chartType !== 'pie' && chartConfig.chartType !== 'donut' && (
              <div className="flex gap-2">
                {data.datasets.map((dataset, index) => {
                  const colors = ['#3b82f6', '#10b981', '#f59e0b', '#ef4444'];
                  return (
                    <Badge key={index} variant="ghost" className="text-xs">
                      <span
                        className="w-2 h-2 rounded-full mr-1"
                        style={{ backgroundColor: dataset.color || colors[index] }}
                      />
                      {dataset.label}
                    </Badge>
                  );
                })}
              </div>
            )}
          </div>
        </CardHeader>
      )}

      <CardContent className={clsx('flex-1', config.display?.showHeader === false && 'pt-6')}>
        {renderChart()}
      </CardContent>
    </Card>
  );
}

export default ChartWidget;

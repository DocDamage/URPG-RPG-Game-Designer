'use client';

import React, { useState, useEffect } from 'react';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Badge } from '@/components/ui/badge';
import { Button } from '@/components/ui/button';
import { 
  Clock, 
  FileText, 
  Pill, 
  AlertTriangle, 
  CheckCircle,
  ChevronRight,
  RefreshCw
} from 'lucide-react';
import Link from 'next/link';
import { formatDistanceToNow } from 'date-fns';

interface Activity {
  id: string;
  type: 'log' | 'medication' | 'incident' | 'training' | 'note';
  title: string;
  description?: string;
  timestamp: string;
  individualName?: string;
  status?: 'completed' | 'pending' | 'alert';
  link?: string;
}

interface RecentActivityProps {
  limit?: number;
  refreshInterval?: number;
}

export function RecentActivity({ limit = 5, refreshInterval = 60000 }: RecentActivityProps) {
  const [activities, setActivities] = useState<Activity[]>([]);
  const [loading, setLoading] = useState(true);

  // Mock data - in production this would come from an API
  const mockActivities: Activity[] = [
    {
      id: '1',
      type: 'medication',
      title: 'Administered Lisinopril',
      description: '10mg - Taken as prescribed',
      timestamp: new Date(Date.now() - 1000 * 60 * 15).toISOString(),
      individualName: 'John Doe',
      status: 'completed',
      link: '/mar',
    },
    {
      id: '2',
      type: 'log',
      title: 'Daily Progress Note',
      description: 'Completed morning routine without issues',
      timestamp: new Date(Date.now() - 1000 * 60 * 60).toISOString(),
      individualName: 'Jane Smith',
      status: 'completed',
      link: '/logs',
    },
    {
      id: '3',
      type: 'incident',
      title: 'Minor Fall Reported',
      description: 'Slipped in bathroom - no injuries',
      timestamp: new Date(Date.now() - 1000 * 60 * 60 * 2).toISOString(),
      individualName: 'Bob Johnson',
      status: 'alert',
      link: '/incidents',
    },
    {
      id: '4',
      type: 'training',
      title: 'Completed CPI Training',
      description: 'De-escalation techniques module',
      timestamp: new Date(Date.now() - 1000 * 60 * 60 * 4).toISOString(),
      status: 'completed',
      link: '/training',
    },
    {
      id: '5',
      type: 'note',
      title: 'Family Visit',
      description: 'Sister visited for 2 hours',
      timestamp: new Date(Date.now() - 1000 * 60 * 60 * 5).toISOString(),
      individualName: 'John Doe',
      link: '/logs',
    },
  ];

  const loadActivities = async () => {
    setLoading(true);
    // Simulate API call
    await new Promise(resolve => setTimeout(resolve, 500));
    setActivities(mockActivities.slice(0, limit));
    setLoading(false);
  };

  useEffect(() => {
    loadActivities();
    
    // Auto-refresh
    const interval = setInterval(loadActivities, refreshInterval);
    return () => clearInterval(interval);
  }, [limit, refreshInterval]);

  const getActivityIcon = (type: Activity['type']) => {
    switch (type) {
      case 'medication':
        return <Pill className="h-4 w-4 text-blue-500" />;
      case 'log':
        return <FileText className="h-4 w-4 text-green-500" />;
      case 'incident':
        return <AlertTriangle className="h-4 w-4 text-red-500" />;
      case 'training':
        return <CheckCircle className="h-4 w-4 text-purple-500" />;
      case 'note':
        return <FileText className="h-4 w-4 text-gray-500" />;
    }
  };

  const getStatusBadge = (status?: Activity['status']) => {
    switch (status) {
      case 'completed':
        return <Badge variant="outline" className="text-green-600">Completed</Badge>;
      case 'pending':
        return <Badge variant="outline" className="text-yellow-600">Pending</Badge>;
      case 'alert':
        return <Badge variant="destructive">Alert</Badge>;
      default:
        return null;
    }
  };

  return (
    <Card>
      <CardHeader className="flex flex-row items-center justify-between">
        <CardTitle className="flex items-center gap-2 text-lg">
          <Clock className="h-5 w-5" />
          Recent Activity
        </CardTitle>
        <Button variant="ghost" size="sm" onClick={loadActivities} disabled={loading}>
          <RefreshCw className={`h-4 w-4 ${loading ? 'animate-spin' : ''}`} />
        </Button>
      </CardHeader>
      <CardContent>
        <div className="space-y-4">
          {activities.map((activity) => (
            <div
              key={activity.id}
              className="flex items-start gap-3 p-3 rounded-lg hover:bg-muted transition-colors"
            >
              <div className="mt-0.5">{getActivityIcon(activity.type)}</div>
              <div className="flex-1 min-w-0">
                <div className="flex items-start justify-between gap-2">
                  <div>
                    <p className="font-medium text-sm">{activity.title}</p>
                    {activity.individualName && (
                      <p className="text-xs text-muted-foreground">
                        {activity.individualName}
                      </p>
                    )}
                    {activity.description && (
                      <p className="text-xs text-muted-foreground line-clamp-1">
                        {activity.description}
                      </p>
                    )}
                  </div>
                  {getStatusBadge(activity.status)}
                </div>
                <p className="text-xs text-muted-foreground mt-1">
                  {formatDistanceToNow(new Date(activity.timestamp), { addSuffix: true })}
                </p>
              </div>
              {activity.link && (
                <Link href={activity.link}>
                  <ChevronRight className="h-4 w-4 text-muted-foreground" />
                </Link>
              )}
            </div>
          ))}
        </div>
        
        <Button variant="ghost" className="w-full mt-4" asChild>
          <Link href="/activity">View All Activity</Link>
        </Button>
      </CardContent>
    </Card>
  );
}

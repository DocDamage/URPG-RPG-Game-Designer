"use client";

import React from 'react';
import { format, parseISO, differenceInDays } from 'date-fns';
import { BookOpen, CheckCircle2, Clock, Award, AlertCircle, PlayCircle } from 'lucide-react';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Progress } from '@/components/ui/progress';
import { Badge } from '@/components/ui/badge';
import { cn } from '@/lib/utils';

export interface TrainingAssignment {
  id: string;
  userId: string;
  moduleId: string;
  moduleTitle?: string;
  assignedBy: string;
  assignedAt: string;
  dueDate?: string;
  completedAt?: string;
  score?: number;
  status: 'assigned' | 'in_progress' | 'completed' | 'overdue' | 'expired';
  certificateUrl?: string;
  timeSpentMinutes: number;
  attempts: number;
}

export interface Certification {
  id: string;
  certificationType: string;
  issuingOrganization?: string;
  expirationDate: string;
  status: 'active' | 'expired' | 'revoked' | 'pending_verification';
}

interface TrainingDashboardProps {
  assignments: TrainingAssignment[];
  certifications: Certification[];
  onContinueTraining?: (assignmentId: string) => void;
  onStartTraining?: (assignmentId: string) => void;
  className?: string;
}

export function TrainingDashboard({
  assignments,
  certifications,
  onContinueTraining,
  onStartTraining,
  className,
}: TrainingDashboardProps) {
  const inProgress = assignments.filter((a) => a.status === 'in_progress');
  const assigned = assignments.filter((a) => a.status === 'assigned');
  const completed = assignments.filter((a) => a.status === 'completed');
  const overdue = assignments.filter((a) => a.status === 'overdue');

  const activeCertifications = certifications.filter((c) => c.status === 'active');
  const expiringSoon = activeCertifications.filter(
    (c) => differenceInDays(parseISO(c.expirationDate), new Date()) <= 30
  );

  const getStatusColor = (status: TrainingAssignment['status']) => {
    switch (status) {
      case 'assigned':
        return 'bg-blue-100 text-blue-800';
      case 'in_progress':
        return 'bg-yellow-100 text-yellow-800';
      case 'completed':
        return 'bg-green-100 text-green-800';
      case 'overdue':
        return 'bg-red-100 text-red-800';
      case 'expired':
        return 'bg-gray-100 text-gray-800';
      default:
        return 'bg-gray-100 text-gray-800';
    }
  };

  const getStatusIcon = (status: TrainingAssignment['status']) => {
    switch (status) {
      case 'completed':
        return <CheckCircle2 className="h-4 w-4" />;
      case 'in_progress':
        return <PlayCircle className="h-4 w-4" />;
      case 'overdue':
        return <AlertCircle className="h-4 w-4" />;
      default:
        return <BookOpen className="h-4 w-4" />;
    }
  };

  return (
    <div className={cn("space-y-6", className)}>
      {/* Stats Overview */}
      <div className="grid grid-cols-2 md:grid-cols-4 gap-4">
        <Card>
          <CardContent className="p-4">
            <div className="flex items-center gap-3">
              <div className="p-2 bg-blue-100 rounded-lg">
                <BookOpen className="h-5 w-5 text-blue-600" />
              </div>
              <div>
                <p className="text-sm text-muted-foreground">Assigned</p>
                <p className="text-2xl font-bold">{assigned.length}</p>
              </div>
            </div>
          </CardContent>
        </Card>

        <Card>
          <CardContent className="p-4">
            <div className="flex items-center gap-3">
              <div className="p-2 bg-yellow-100 rounded-lg">
                <Clock className="h-5 w-5 text-yellow-600" />
              </div>
              <div>
                <p className="text-sm text-muted-foreground">In Progress</p>
                <p className="text-2xl font-bold">{inProgress.length}</p>
              </div>
            </div>
          </CardContent>
        </Card>

        <Card>
          <CardContent className="p-4">
            <div className="flex items-center gap-3">
              <div className="p-2 bg-green-100 rounded-lg">
                <CheckCircle2 className="h-5 w-5 text-green-600" />
              </div>
              <div>
                <p className="text-sm text-muted-foreground">Completed</p>
                <p className="text-2xl font-bold">{completed.length}</p>
              </div>
            </div>
          </CardContent>
        </Card>

        <Card>
          <CardContent className="p-4">
            <div className="flex items-center gap-3">
              <div className="p-2 bg-purple-100 rounded-lg">
                <Award className="h-5 w-5 text-purple-600" />
              </div>
              <div>
                <p className="text-sm text-muted-foreground">Certifications</p>
                <p className="text-2xl font-bold">{activeCertifications.length}</p>
              </div>
            </div>
          </CardContent>
        </Card>
      </div>

      {/* In Progress & Assigned */}
      <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
        {/* Continue Learning */}
        <Card>
          <CardHeader>
            <CardTitle className="text-lg flex items-center gap-2">
              <PlayCircle className="h-5 w-5" />
              Continue Learning
            </CardTitle>
          </CardHeader>
          <CardContent className="space-y-3">
            {inProgress.length === 0 ? (
              <p className="text-sm text-muted-foreground text-center py-4">
                No trainings in progress
              </p>
            ) : (
              inProgress.slice(0, 3).map((assignment) => (
                <div
                  key={assignment.id}
                  className="flex items-center justify-between p-3 rounded-lg border hover:bg-muted/50 transition-colors"
                >
                  <div className="flex-1 min-w-0">
                    <p className="font-medium truncate">{assignment.moduleTitle}</p>
                    <div className="flex items-center gap-2 mt-1">
                      <Clock className="h-3 w-3 text-muted-foreground" />
                      <span className="text-xs text-muted-foreground">
                        {assignment.timeSpentMinutes} min spent
                      </span>
                    </div>
                    {assignment.dueDate && (
                      <p className="text-xs text-muted-foreground mt-1">
                        Due {format(parseISO(assignment.dueDate), 'MMM d')}
                      </p>
                    )}
                  </div>
                  <Button
                    size="sm"
                    onClick={() => onContinueTraining?.(assignment.id)}
                  >
                    Continue
                  </Button>
                </div>
              ))
            )}
          </CardContent>
        </Card>

        {/* Assigned Trainings */}
        <Card>
          <CardHeader>
            <CardTitle className="text-lg flex items-center gap-2">
              <BookOpen className="h-5 w-5" />
              Assigned to You
            </CardTitle>
          </CardHeader>
          <CardContent className="space-y-3">
            {assigned.length === 0 ? (
              <p className="text-sm text-muted-foreground text-center py-4">
                No pending assignments
              </p>
            ) : (
              assigned.slice(0, 3).map((assignment) => (
                <div
                  key={assignment.id}
                  className="flex items-center justify-between p-3 rounded-lg border hover:bg-muted/50 transition-colors"
                >
                  <div className="flex-1 min-w-0">
                    <p className="font-medium truncate">{assignment.moduleTitle}</p>
                    {assignment.dueDate && (
                      <p className="text-xs text-muted-foreground">
                        Due {format(parseISO(assignment.dueDate), 'MMM d')}
                      </p>
                    )}
                  </div>
                  <Button
                    size="sm"
                    variant="outline"
                    onClick={() => onStartTraining?.(assignment.id)}
                  >
                    Start
                  </Button>
                </div>
              ))
            )}
          </CardContent>
        </Card>
      </div>

      {/* Certifications Expiring Soon */}
      {expiringSoon.length > 0 && (
        <Card className="border-yellow-200 bg-yellow-50/50">
          <CardHeader>
            <CardTitle className="text-lg flex items-center gap-2 text-yellow-800">
              <AlertCircle className="h-5 w-5" />
              Certifications Expiring Soon
            </CardTitle>
          </CardHeader>
          <CardContent>
            <div className="space-y-2">
              {expiringSoon.map((cert) => (
                <div
                  key={cert.id}
                  className="flex items-center justify-between p-3 bg-white rounded-lg border border-yellow-200"
                >
                  <div>
                    <p className="font-medium">{cert.certificationType}</p>
                    <p className="text-sm text-muted-foreground">
                      Expires {format(parseISO(cert.expirationDate), 'MMMM d, yyyy')}
                    </p>
                  </div>
                  <Badge variant="outline" className="border-yellow-400 text-yellow-700">
                    {differenceInDays(parseISO(cert.expirationDate), new Date())} days left
                  </Badge>
                </div>
              ))}
            </div>
          </CardContent>
        </Card>
      )}

      {/* Recent Completions */}
      {completed.length > 0 && (
        <Card>
          <CardHeader>
            <CardTitle className="text-lg flex items-center gap-2">
              <CheckCircle2 className="h-5 w-5" />
              Recently Completed
            </CardTitle>
          </CardHeader>
          <CardContent>
            <div className="space-y-2">
              {completed.slice(0, 3).map((assignment) => (
                <div
                  key={assignment.id}
                  className="flex items-center justify-between p-3 rounded-lg border"
                >
                  <div className="flex items-center gap-3">
                    <div className="p-2 bg-green-100 rounded-full">
                      <CheckCircle2 className="h-4 w-4 text-green-600" />
                    </div>
                    <div>
                      <p className="font-medium">{assignment.moduleTitle}</p>
                      <p className="text-sm text-muted-foreground">
                        Completed {assignment.completedAt && format(parseISO(assignment.completedAt), 'MMM d')}
                      </p>
                    </div>
                  </div>
                  {assignment.score !== undefined && (
                    <Badge variant={assignment.score >= 80 ? 'default' : 'secondary'}>
                      {assignment.score}%
                    </Badge>
                  )}
                </div>
              ))}
            </div>
          </CardContent>
        </Card>
      )}
    </div>
  );
}

export default TrainingDashboard;

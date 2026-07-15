"use client";

import React, { useState, useEffect } from 'react';
import { BookOpen, Award, FileCheck, AlertCircle } from 'lucide-react';
import { Tabs, TabsContent, TabsList, TabsTrigger } from '@/components/ui/tabs';
import { Alert, AlertDescription } from '@/components/ui/alert';
import { Button } from '@/components/ui/button';
import { TrainingDashboard } from '@/components/training/TrainingDashboard';
import { CertificationBadge } from '@/components/training/CertificationBadge';
import trainingApi, { TrainingAssignment, Certification } from '@/lib/api/training';

export default function TrainingPage() {
  const [activeTab, setActiveTab] = useState('dashboard');
  const [assignments, setAssignments] = useState<TrainingAssignment[]>([]);
  const [certifications, setCertifications] = useState<Certification[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  const fetchData = async () => {
    setLoading(true);
    setError(null);
    try {
      const [assignmentsRes, certificationsRes] = await Promise.all([
        trainingApi.getMyAssignments(true),
        trainingApi.getCertifications(),
      ]);

      if (assignmentsRes.data) {
        setAssignments(assignmentsRes.data);
      }
      if (certificationsRes.data) {
        setCertifications(certificationsRes.data);
      }
    } catch (err) {
      console.error('Failed to fetch training data:', err);
      setError('Failed to load training data. Please try again.');
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchData();
  }, []);

  const handleContinueTraining = (assignmentId: string) => {
    console.log('Continue training:', assignmentId);
    // Navigate to training module or open modal
  };

  const handleStartTraining = (assignmentId: string) => {
    console.log('Start training:', assignmentId);
    // Navigate to training module or open modal
  };

  const assignedTrainings = assignments.filter((a) => a.status === 'assigned');
  const inProgressTrainings = assignments.filter((a) => a.status === 'in_progress');
  const completedTrainings = assignments.filter((a) => a.status === 'completed');

  return (
    <div className="container mx-auto py-6 space-y-6">
      {/* Header */}
      <div className="flex flex-col sm:flex-row sm:items-center sm:justify-between gap-4">
        <div>
          <h1 className="text-3xl font-bold tracking-tight">Training & Certifications</h1>
          <p className="text-muted-foreground">
            Complete required training and manage your certifications
          </p>
        </div>
      </div>

      {/* Error Alert */}
      {error && (
        <Alert variant="destructive">
          <AlertCircle className="h-4 w-4" />
          <AlertDescription>{error}</AlertDescription>
        </Alert>
      )}

      {/* Tabs */}
      <Tabs value={activeTab} onValueChange={setActiveTab}>
        <TabsList className="grid w-full grid-cols-4 lg:w-[400px]">
          <TabsTrigger value="dashboard">
            <BookOpen className="mr-2 h-4 w-4" />
            Dashboard
          </TabsTrigger>
          <TabsTrigger value="assigned">
            <FileCheck className="mr-2 h-4 w-4" />
            Assigned
          </TabsTrigger>
          <TabsTrigger value="completed">
            <Award className="mr-2 h-4 w-4" />
            Completed
          </TabsTrigger>
          <TabsTrigger value="certifications">
            <FileCheck className="mr-2 h-4 w-4" />
            Certifications
          </TabsTrigger>
        </TabsList>

        {/* Dashboard Tab */}
        <TabsContent value="dashboard" className="mt-6">
          <TrainingDashboard
            assignments={assignments}
            certifications={certifications}
            onContinueTraining={handleContinueTraining}
            onStartTraining={handleStartTraining}
          />
        </TabsContent>

        {/* Assigned Tab */}
        <TabsContent value="assigned" className="mt-6">
          <div className="space-y-4">
            {loading ? (
              <div className="text-center py-8 text-muted-foreground">Loading...</div>
            ) : assignedTrainings.length === 0 ? (
              <div className="text-center py-12 bg-muted/50 rounded-lg">
                <BookOpen className="mx-auto h-12 w-12 text-muted-foreground" />
                <h3 className="mt-4 text-lg font-medium">No Assigned Trainings</h3>
                <p className="text-muted-foreground">
                  You don&apos;t have any pending training assignments
                </p>
              </div>
            ) : (
              assignedTrainings.map((assignment) => (
                <div
                  key={assignment.id}
                  className="flex items-center justify-between p-4 bg-white rounded-lg border"
                >
                  <div>
                    <h3 className="font-medium">{assignment.moduleTitle}</h3>
                    <p className="text-sm text-muted-foreground">
                      Assigned {new Date(assignment.assignedAt).toLocaleDateString()}
                    </p>
                    {assignment.dueDate && (
                      <p className="text-sm text-muted-foreground">
                        Due {new Date(assignment.dueDate).toLocaleDateString()}
                      </p>
                    )}
                  </div>
                  <Button onClick={() => handleStartTraining(assignment.id)}>
                    Start Training
                  </Button>
                </div>
              ))
            )}
          </div>
        </TabsContent>

        {/* Completed Tab */}
        <TabsContent value="completed" className="mt-6">
          <div className="space-y-4">
            {loading ? (
              <div className="text-center py-8 text-muted-foreground">Loading...</div>
            ) : completedTrainings.length === 0 ? (
              <div className="text-center py-12 bg-muted/50 rounded-lg">
                <Award className="mx-auto h-12 w-12 text-muted-foreground" />
                <h3 className="mt-4 text-lg font-medium">No Completed Trainings</h3>
                <p className="text-muted-foreground">
                  Complete training modules to see them here
                </p>
              </div>
            ) : (
              completedTrainings.map((assignment) => (
                <div
                  key={assignment.id}
                  className="flex items-center justify-between p-4 bg-white rounded-lg border"
                >
                  <div className="flex items-center gap-4">
                    <div className="p-2 bg-green-100 rounded-full">
                      <Award className="h-5 w-5 text-green-600" />
                    </div>
                    <div>
                      <h3 className="font-medium">{assignment.moduleTitle}</h3>
                      <p className="text-sm text-muted-foreground">
                        Completed {assignment.completedAt && new Date(assignment.completedAt).toLocaleDateString()}
                      </p>
                    </div>
                  </div>
                  {assignment.score !== undefined && (
                    <div className="text-right">
                      <p className="text-2xl font-bold">{assignment.score}%</p>
                      <p className="text-sm text-muted-foreground">Score</p>
                    </div>
                  )}
                </div>
              ))
            )}
          </div>
        </TabsContent>

        {/* Certifications Tab */}
        <TabsContent value="certifications" className="mt-6">
          <div className="space-y-4">
            {loading ? (
              <div className="text-center py-8 text-muted-foreground">Loading...</div>
            ) : certifications.length === 0 ? (
              <div className="text-center py-12 bg-muted/50 rounded-lg">
                <FileCheck className="mx-auto h-12 w-12 text-muted-foreground" />
                <h3 className="mt-4 text-lg font-medium">No Certifications</h3>
                <p className="text-muted-foreground">
                  Add your certifications to track expiration dates
                </p>
              </div>
            ) : (
              certifications.map((cert) => (
                <CertificationBadge
                  key={cert.id}
                  certification={cert}
                  showDetails
                />
              ))
            )}
          </div>
        </TabsContent>
      </Tabs>
    </div>
  );
}

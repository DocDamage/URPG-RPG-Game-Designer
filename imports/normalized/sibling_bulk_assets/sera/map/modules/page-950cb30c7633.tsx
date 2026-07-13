'use client';

import React, { useState, useEffect } from 'react';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Badge } from '@/components/ui/badge';
import { Progress } from '@/components/ui/progress';
import { Tabs, TabsContent, TabsList, TabsTrigger } from '@/components/ui/tabs';
import { getModules, getMyAssignments, TrainingModule, TrainingAssignment } from '@/lib/api/training';
import { useToast } from '@/components/ui/use-toast';
import { BookOpen, Clock, Search, Award, Play, CheckCircle } from 'lucide-react';
import Link from 'next/link';

export default function TrainingModulesPage() {
  const [modules, setModules] = useState<TrainingModule[]>([]);
  const [assignments, setAssignments] = useState<TrainingAssignment[]>([]);
  const [loading, setLoading] = useState(true);
  const [searchQuery, setSearchQuery] = useState('');
  const { toast } = useToast();

  useEffect(() => {
    loadData();
  }, []);

  const loadData = async () => {
    try {
      setLoading(true);
      const [modulesResponse, assignmentsResponse] = await Promise.all([
        getModules(),
        getMyAssignments(),
      ]);

      if (modulesResponse.success) {
        setModules(modulesResponse.data);
      }

      if (assignmentsResponse.success) {
        setAssignments(assignmentsResponse.data);
      }
    } catch (error) {
      toast({
        title: 'Error',
        description: 'Failed to load training data',
        variant: 'destructive',
      });
    } finally {
      setLoading(false);
    }
  };

  const getAssignmentForModule = (moduleId: string) => {
    return assignments.find(a => a.moduleId === moduleId);
  };

  const filteredModules = modules.filter(module =>
    module.title.toLowerCase().includes(searchQuery.toLowerCase()) ||
    module.description?.toLowerCase().includes(searchQuery.toLowerCase()) ||
    module.category.toLowerCase().includes(searchQuery.toLowerCase())
  );

  const getCategoryColor = (category: string) => {
    const colors: Record<string, string> = {
      'Safety': 'bg-red-100 text-red-800',
      'Clinical': 'bg-blue-100 text-blue-800',
      'Behavioral': 'bg-purple-100 text-purple-800',
      'Compliance': 'bg-yellow-100 text-yellow-800',
      'Leadership': 'bg-green-100 text-green-800',
    };
    return colors[category] || 'bg-gray-100 text-gray-800';
  };

  const getStatusBadge = (assignment?: TrainingAssignment) => {
    if (!assignment) return null;
    if (assignment.status === 'completed') {
      return <Badge className="bg-green-100 text-green-800"><CheckCircle className="h-3 w-3 mr-1" /> Completed</Badge>;
    }
    if (assignment.status === 'in_progress') {
      return <Badge className="bg-blue-100 text-blue-800">In Progress</Badge>;
    }
    if (assignment.status === 'overdue') {
      return <Badge variant="destructive">Overdue</Badge>;
    }
    return <Badge variant="secondary">Not Started</Badge>;
  };

  const getButtonLabel = (assignment?: TrainingAssignment) => {
    if (!assignment) return 'Start Module';
    if (assignment.status === 'completed') return 'Review';
    if (assignment.status === 'in_progress') return 'Continue';
    return 'Start Module';
  };

  if (loading) {
    return (
      <div className="container mx-auto p-6">
        <div className="flex items-center justify-center h-64">
          <div className="animate-spin rounded-full h-8 w-8 border-b-2 border-primary"></div>
        </div>
      </div>
    );
  }

  return (
    <div className="container mx-auto p-6 space-y-6">
      <div className="flex items-center justify-between">
        <div>
          <h1 className="text-3xl font-bold flex items-center gap-2">
            <BookOpen className="h-8 w-8" />
            Training Modules
          </h1>
          <p className="text-muted-foreground mt-1">
            Complete your assigned training modules and track your progress
          </p>
        </div>
      </div>

      {/* Search */}
      <div className="relative">
        <Search className="absolute left-3 top-3 h-4 w-4 text-muted-foreground" />
        <Input
          placeholder="Search modules by title, description, or category..."
          value={searchQuery}
          onChange={(e) => setSearchQuery(e.target.value)}
          className="pl-10"
        />
      </div>

      <Tabs defaultValue="all" className="space-y-6">
        <TabsList>
          <TabsTrigger value="all">All Modules</TabsTrigger>
          <TabsTrigger value="assigned">My Assignments</TabsTrigger>
          <TabsTrigger value="completed">Completed</TabsTrigger>
        </TabsList>

        <TabsContent value="all" className="space-y-4">
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
            {filteredModules.map((module) => {
              const assignment = getAssignmentForModule(module.id);
              return (
                <Card key={module.id} className="hover:shadow-lg transition-shadow">
                  <CardHeader>
                    <div className="flex items-start justify-between">
                      <div>
                        <Badge className={getCategoryColor(module.category)}>
                          {module.category}
                        </Badge>
                        {getStatusBadge(assignment)}
                      </div>
                    </div>
                    <CardTitle className="text-lg mt-2">{module.title}</CardTitle>
                  </CardHeader>
                  <CardContent className="space-y-4">
                    <p className="text-sm text-muted-foreground line-clamp-2">
                      {module.description}
                    </p>

                    {assignment && assignment.progress > 0 && assignment.status !== 'completed' && (
                      <div>
                        <div className="flex justify-between text-sm mb-1">
                          <span>Progress</span>
                          <span>{assignment.progress}%</span>
                        </div>
                        <Progress value={assignment.progress} />
                      </div>
                    )}

                    <div className="flex items-center gap-4 text-sm text-muted-foreground">
                      <span className="flex items-center gap-1">
                        <Clock className="h-4 w-4" />
                        {module.durationMinutes} min
                      </span>
                      {module.passScore && (
                        <span className="flex items-center gap-1">
                          <Award className="h-4 w-4" />
                          Pass: {module.passScore}%
                        </span>
                      )}
                    </div>

                    {assignment?.dueDate && (
                      <p className="text-sm text-muted-foreground">
                        Due: {new Date(assignment.dueDate).toLocaleDateString()}
                      </p>
                    )}

                    <Button className="w-full">
                      <Play className="h-4 w-4 mr-2" />
                      {getButtonLabel(assignment)}
                    </Button>
                  </CardContent>
                </Card>
              );
            })}
          </div>
        </TabsContent>

        <TabsContent value="assigned">
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
            {assignments
              .filter(a => a.status !== 'completed')
              .map((assignment) => {
                const module = modules.find(m => m.id === assignment.moduleId);
                if (!module) return null;
                return (
                  <Card key={assignment.id} className="hover:shadow-lg transition-shadow">
                    <CardHeader>
                      <div className="flex items-start justify-between">
                        <div>
                          <Badge className={getCategoryColor(module.category)}>
                            {module.category}
                          </Badge>
                          {getStatusBadge(assignment)}
                        </div>
                      </div>
                      <CardTitle className="text-lg mt-2">{module.title}</CardTitle>
                    </CardHeader>
                    <CardContent className="space-y-4">
                      {assignment.progress > 0 && (
                        <div>
                          <div className="flex justify-between text-sm mb-1">
                            <span>Progress</span>
                            <span>{assignment.progress}%</span>
                          </div>
                          <Progress value={assignment.progress} />
                        </div>
                      )}
                      {assignment.dueDate && (
                        <p className="text-sm text-muted-foreground">
                          Due: {new Date(assignment.dueDate).toLocaleDateString()}
                        </p>
                      )}
                      <Button className="w-full">
                        <Play className="h-4 w-4 mr-2" />
                        Continue
                      </Button>
                    </CardContent>
                  </Card>
                );
              })}
          </div>
        </TabsContent>

        <TabsContent value="completed">
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
            {assignments
              .filter(a => a.status === 'completed')
              .map((assignment) => {
                const module = modules.find(m => m.id === assignment.moduleId);
                if (!module) return null;
                return (
                  <Card key={assignment.id}>
                    <CardHeader>
                      <div className="flex items-start justify-between">
                        <Badge className="bg-green-100 text-green-800">
                          <CheckCircle className="h-3 w-3 mr-1" /> Completed
                        </Badge>
                        {assignment.score && (
                          <Badge variant="outline">Score: {assignment.score}%</Badge>
                        )}
                      </div>
                      <CardTitle className="text-lg mt-2">{module.title}</CardTitle>
                    </CardHeader>
                    <CardContent>
                      <p className="text-sm text-muted-foreground">
                        Completed on: {assignment.completedAt ? new Date(assignment.completedAt).toLocaleDateString() : 'N/A'}
                      </p>
                      <Button variant="outline" className="w-full mt-4">
                        Review
                      </Button>
                    </CardContent>
                  </Card>
                );
              })}
          </div>
        </TabsContent>
      </Tabs>
    </div>
  );
}

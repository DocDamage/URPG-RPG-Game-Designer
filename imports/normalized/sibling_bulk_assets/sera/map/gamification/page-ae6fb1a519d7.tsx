"use client";

import React, { useState, useEffect } from 'react';
import { Trophy, Medal, Star, Heart, Target, Users, Gift, TrendingUp } from 'lucide-react';
import { Button } from '@/components/ui/button';
import { Tabs, TabsContent, TabsList, TabsTrigger } from '@/components/ui/tabs';
import { Alert, AlertDescription } from '@/components/ui/alert';
import { Badge } from '@/components/ui/badge';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Progress } from '@/components/ui/progress';
import { Avatar, AvatarFallback } from '@/components/ui/avatar';
import gamificationApi, { Achievement, UserPoints, LeaderboardEntry, Challenge, Recognition } from '@/lib/api/gamification';

export default function GamificationPage() {
  const [activeTab, setActiveTab] = useState('overview');
  const [achievements, setAchievements] = useState<Achievement[]>([]);
  const [points, setPoints] = useState<UserPoints | null>(null);
  const [leaderboard, setLeaderboard] = useState<LeaderboardEntry[]>([]);
  const [challenges, setChallenges] = useState<Challenge[]>([]);
  const [recognitions, setRecognitions] = useState<(Recognition & { fromName: string; toName: string })[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  const fetchData = async () => {
    setLoading(true);
    setError(null);
    try {
      const [achievementsRes, pointsRes, leaderboardRes, challengesRes, recognitionsRes] = await Promise.all([
        gamificationApi.getMyAchievements(),
        gamificationApi.getMyPoints(),
        gamificationApi.getLeaderboard({ limit: 10 }),
        gamificationApi.getActiveChallenges(),
        gamificationApi.getPublicRecognitions(10),
      ]);

      if (achievementsRes.data) setAchievements(achievementsRes.data);
      if (pointsRes.data) setPoints(pointsRes.data);
      if (leaderboardRes.data) setLeaderboard(leaderboardRes.data);
      if (challengesRes.data) setChallenges(challengesRes.data);
      if (recognitionsRes.data) setRecognitions(recognitionsRes.data);
    } catch (err) {
      console.error('Failed to fetch gamification data:', err);
      setError('Failed to load gamification data. Please try again.');
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchData();
  }, []);

  const getCategoryIcon = (category: string) => {
    switch (category) {
      case 'documentation':
        return <Medal className="h-4 w-4" />;
      case 'safety':
        return <Star className="h-4 w-4" />;
      case 'training':
        return <TrendingUp className="h-4 w-4" />;
      case 'teamwork':
        return <Users className="h-4 w-4" />;
      default:
        return <Trophy className="h-4 w-4" />;
    }
  };

  const getCategoryColor = (category: string) => {
    switch (category) {
      case 'documentation':
        return 'bg-blue-100 text-blue-800';
      case 'safety':
        return 'bg-green-100 text-green-800';
      case 'training':
        return 'bg-purple-100 text-purple-800';
      case 'teamwork':
        return 'bg-orange-100 text-orange-800';
      default:
        return 'bg-gray-100 text-gray-800';
    }
  };

  const getLevelProgress = () => {
    if (!points) return 0;
    const pointsInLevel = points.totalPoints % 1000;
    return (pointsInLevel / 1000) * 100;
  };

  return (
    <div className="container mx-auto py-6 space-y-6">
      {/* Header */}
      <div className="flex flex-col sm:flex-row sm:items-center sm:justify-between gap-4">
        <div>
          <h1 className="text-3xl font-bold tracking-tight">Achievements & Recognition</h1>
          <p className="text-muted-foreground">
            Earn badges, climb the leaderboard, and recognize your colleagues
          </p>
        </div>
      </div>

      {/* Error Alert */}
      {error && (
        <Alert variant="destructive">
          <AlertDescription>{error}</AlertDescription>
        </Alert>
      )}

      {/* Stats Cards */}
      <div className="grid grid-cols-2 md:grid-cols-4 gap-4">
        <Card>
          <CardContent className="p-4">
            <div className="flex items-center gap-3">
              <div className="p-2 bg-yellow-100 rounded-lg">
                <Trophy className="h-5 w-5 text-yellow-600" />
              </div>
              <div>
                <p className="text-sm text-muted-foreground">Total Points</p>
                <p className="text-2xl font-bold">{points?.totalPoints || 0}</p>
              </div>
            </div>
          </CardContent>
        </Card>

        <Card>
          <CardContent className="p-4">
            <div className="flex items-center gap-3">
              <div className="p-2 bg-blue-100 rounded-lg">
                <Medal className="h-5 w-5 text-blue-600" />
              </div>
              <div>
                <p className="text-sm text-muted-foreground">Level</p>
                <p className="text-2xl font-bold">{points?.level || 1}</p>
              </div>
            </div>
          </CardContent>
        </Card>

        <Card>
          <CardContent className="p-4">
            <div className="flex items-center gap-3">
              <div className="p-2 bg-purple-100 rounded-lg">
                <Star className="h-5 w-5 text-purple-600" />
              </div>
              <div>
                <p className="text-sm text-muted-foreground">Badges</p>
                <p className="text-2xl font-bold">{achievements.length}</p>
              </div>
            </div>
          </CardContent>
        </Card>

        <Card>
          <CardContent className="p-4">
            <div className="flex items-center gap-3">
              <div className="p-2 bg-green-100 rounded-lg">
                <TrendingUp className="h-5 w-5 text-green-600" />
              </div>
              <div>
                <p className="text-sm text-muted-foreground">This Week</p>
                <p className="text-2xl font-bold">{points?.pointsThisWeek || 0}</p>
              </div>
            </div>
          </CardContent>
        </Card>
      </div>

      {/* Level Progress */}
      {points && (
        <Card>
          <CardContent className="p-4">
            <div className="flex items-center justify-between mb-2">
              <span className="font-medium">Level {points.level}</span>
              <span className="text-sm text-muted-foreground">
                {points.totalPoints % 1000} / 1000 XP to Level {points.level + 1}
              </span>
            </div>
            <Progress value={getLevelProgress()} className="h-2" />
          </CardContent>
        </Card>
      )}

      {/* Tabs */}
      <Tabs value={activeTab} onValueChange={setActiveTab}>
        <TabsList className="grid w-full grid-cols-4 lg:w-[500px]">
          <TabsTrigger value="overview">Overview</TabsTrigger>
          <TabsTrigger value="achievements">Badges</TabsTrigger>
          <TabsTrigger value="leaderboard">Leaderboard</TabsTrigger>
          <TabsTrigger value="recognition">Recognition</TabsTrigger>
        </TabsList>

        {/* Overview Tab */}
        <TabsContent value="overview" className="mt-6 space-y-6">
          {/* Active Challenges */}
          <div>
            <h3 className="text-lg font-semibold mb-3">Active Challenges</h3>
            {challenges.length === 0 ? (
              <p className="text-muted-foreground">No active challenges</p>
            ) : (
              <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
                {challenges.map((challenge) => (
                  <Card key={challenge.id}>
                    <CardHeader>
                      <div className="flex items-center justify-between">
                        <CardTitle className="text-lg">{challenge.title}</CardTitle>
                        <Badge>{challenge.type}</Badge>
                      </div>
                    </CardHeader>
                    <CardContent className="space-y-2">
                      <p className="text-sm text-muted-foreground">{challenge.description}</p>
                      <div className="flex items-center gap-2 text-sm">
                        <Target className="h-4 w-4" />
                        <span>Target: {challenge.target} {challenge.metric}</span>
                      </div>
                      <div className="flex items-center gap-2 text-sm">
                        <Gift className="h-4 w-4" />
                        <span>Reward: {challenge.reward}</span>
                      </div>
                      <p className="text-xs text-muted-foreground">
                        Ends: {new Date(challenge.endDate).toLocaleDateString()}
                      </p>
                    </CardContent>
                  </Card>
                ))}
              </div>
            )}
          </div>

          {/* Recent Achievements */}
          <div>
            <h3 className="text-lg font-semibold mb-3">Recent Achievements</h3>
            {achievements.length === 0 ? (
              <p className="text-muted-foreground">No achievements yet</p>
            ) : (
              <div className="grid grid-cols-2 md:grid-cols-4 gap-4">
                {achievements.slice(0, 4).map((achievement) => (
                  <Card key={achievement.id} className="text-center">
                    <CardContent className="p-4">
                      <div className={`w-12 h-12 rounded-full mx-auto mb-2 flex items-center justify-center bg-${achievement.color}-100`}>
                        <Trophy className={`h-6 w-6 text-${achievement.color}-600`} />
                      </div>
                      <p className="font-medium text-sm">{achievement.title}</p>
                      <p className="text-xs text-muted-foreground">+{achievement.points} pts</p>
                    </CardContent>
                  </Card>
                ))}
              </div>
            )}
          </div>
        </TabsContent>

        {/* Achievements Tab */}
        <TabsContent value="achievements" className="mt-6">
          {loading ? (
            <div className="text-center py-8 text-muted-foreground">Loading...</div>
          ) : achievements.length === 0 ? (
            <div className="text-center py-12 bg-muted/50 rounded-lg">
              <Trophy className="mx-auto h-12 w-12 text-muted-foreground" />
              <h3 className="mt-4 text-lg font-medium">No Achievements Yet</h3>
              <p className="text-muted-foreground">Complete tasks to earn badges</p>
            </div>
          ) : (
            <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
              {achievements.map((achievement) => (
                <Card key={achievement.id}>
                  <CardContent className="p-4">
                    <div className="flex items-start gap-4">
                      <div className={`p-3 rounded-lg ${getCategoryColor(achievement.category)}`}>
                        {getCategoryIcon(achievement.category)}
                      </div>
                      <div className="flex-1">
                        <p className="font-medium">{achievement.title}</p>
                        <p className="text-sm text-muted-foreground">{achievement.description}</p>
                        <div className="flex items-center gap-2 mt-2">
                          <Badge variant="outline">+{achievement.points} pts</Badge>
                          <span className="text-xs text-muted-foreground">
                            {new Date(achievement.earnedAt).toLocaleDateString()}
                          </span>
                        </div>
                      </div>
                    </div>
                  </CardContent>
                </Card>
              ))}
            </div>
          )}
        </TabsContent>

        {/* Leaderboard Tab */}
        <TabsContent value="leaderboard" className="mt-6">
          <Card>
            <CardHeader>
              <CardTitle>Top Performers</CardTitle>
            </CardHeader>
            <CardContent>
              <div className="space-y-4">
                {leaderboard.map((entry) => (
                  <div
                    key={entry.userId}
                    className={`flex items-center gap-4 p-3 rounded-lg ${
                      entry.rank === 1 ? 'bg-yellow-50 border border-yellow-200' :
                      entry.rank === 2 ? 'bg-gray-50 border border-gray-200' :
                      entry.rank === 3 ? 'bg-orange-50 border border-orange-200' :
                      ''
                    }`}
                  >
                    <div className={`w-8 h-8 rounded-full flex items-center justify-center font-bold ${
                      entry.rank === 1 ? 'bg-yellow-400 text-yellow-900' :
                      entry.rank === 2 ? 'bg-gray-300 text-gray-900' :
                      entry.rank === 3 ? 'bg-orange-300 text-orange-900' :
                      'bg-muted text-muted-foreground'
                    }`}>
                      {entry.rank}
                    </div>
                    <Avatar>
                      <AvatarFallback>{entry.userName.charAt(0)}</AvatarFallback>
                    </Avatar>
                    <div className="flex-1">
                      <p className="font-medium">{entry.userName}</p>
                      <p className="text-sm text-muted-foreground">
                        Level {entry.level} • {entry.achievements} badges
                      </p>
                    </div>
                    <div className="text-right">
                      <p className="font-bold">{entry.totalPoints.toLocaleString()}</p>
                      <p className="text-xs text-muted-foreground">points</p>
                    </div>
                  </div>
                ))}
              </div>
            </CardContent>
          </Card>
        </TabsContent>

        {/* Recognition Tab */}
        <TabsContent value="recognition" className="mt-6">
          <div className="space-y-4">
            {recognitions.length === 0 ? (
              <div className="text-center py-12 bg-muted/50 rounded-lg">
                <Heart className="mx-auto h-12 w-12 text-muted-foreground" />
                <h3 className="mt-4 text-lg font-medium">No Recognitions Yet</h3>
                <p className="text-muted-foreground">Be the first to recognize a colleague!</p>
              </div>
            ) : (
              recognitions.map((recognition) => (
                <Card key={recognition.id}>
                  <CardContent className="p-4">
                    <div className="flex items-start gap-4">
                      <div className="p-3 bg-pink-100 rounded-full">
                        <Heart className="h-5 w-5 text-pink-600" />
                      </div>
                      <div className="flex-1">
                        <p className="font-medium">
                          {recognition.fromName} sent {recognition.toName} a
                          <Badge variant="outline" className="ml-2">
                            {recognition.type.replace('_', ' ')}
                          </Badge>
                        </p>
                        <p className="text-muted-foreground mt-1">{recognition.message}</p>
                        <p className="text-xs text-muted-foreground mt-2">
                          {new Date(recognition.createdAt).toLocaleString()}
                        </p>
                      </div>
                    </div>
                  </CardContent>
                </Card>
              ))
            )}
          </div>
        </TabsContent>
      </Tabs>
    </div>
  );
}

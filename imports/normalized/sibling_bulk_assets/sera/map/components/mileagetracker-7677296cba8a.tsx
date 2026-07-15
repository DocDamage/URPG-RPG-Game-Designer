'use client';

import React, { useState, useEffect } from 'react';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Label } from '@/components/ui/label';
import { Badge } from '@/components/ui/badge';
import { 
  startMileageTracking, 
  endMileageTracking, 
  getMileageLogs,
  MileageLog 
} from '@/lib/api/location';
import { useToast } from '@/components/ui/use-toast';
import { Car, Play, Square, Route, DollarSign, Clock } from 'lucide-react';

export function MileageTracker() {
  const [activeTracking, setActiveTracking] = useState<MileageLog | null>(null);
  const [mileageLogs, setMileageLogs] = useState<MileageLog[]>([]);
  const [purpose, setPurpose] = useState('');
  const [loading, setLoading] = useState(true);
  const [tracking, setTracking] = useState(false);
  const { toast } = useToast();

  useEffect(() => {
    loadMileageLogs();
  }, []);

  const loadMileageLogs = async () => {
    try {
      setLoading(true);
      const response = await getMileageLogs();
      if (response.success) {
        setMileageLogs(response.data);
      }
    } catch (error) {
      toast({
        title: 'Error',
        description: 'Failed to load mileage logs',
        variant: 'destructive',
      });
    } finally {
      setLoading(false);
    }
  };

  const handleStartTracking = async () => {
    if (!purpose.trim()) {
      toast({
        title: 'Validation Error',
        description: 'Please enter a purpose for this trip',
        variant: 'destructive',
      });
      return;
    }

    if (!('geolocation' in navigator)) {
      toast({
        title: 'Error',
        description: 'Geolocation is not supported by your browser',
        variant: 'destructive',
      });
      return;
    }

    try {
      setTracking(true);
      const position = await new Promise<GeolocationPosition>((resolve, reject) => {
        navigator.geolocation.getCurrentPosition(resolve, reject, { enableHighAccuracy: true });
      });

      const response = await startMileageTracking(purpose, {
        latitude: position.coords.latitude,
        longitude: position.coords.longitude,
      });

      if (response.success) {
        setActiveTracking(response.data);
        toast({
          title: 'Tracking Started',
          description: 'Mileage tracking is now active',
        });
      }
    } catch (error) {
      toast({
        title: 'Error',
        description: 'Failed to start tracking',
        variant: 'destructive',
      });
    } finally {
      setTracking(false);
    }
  };

  const handleEndTracking = async () => {
    if (!activeTracking) return;

    if (!('geolocation' in navigator)) {
      toast({
        title: 'Error',
        description: 'Geolocation is not supported by your browser',
        variant: 'destructive',
      });
      return;
    }

    try {
      setTracking(true);
      const position = await new Promise<GeolocationPosition>((resolve, reject) => {
        navigator.geolocation.getCurrentPosition(resolve, reject, { enableHighAccuracy: true });
      });

      const response = await endMileageTracking(activeTracking.id, {
        latitude: position.coords.latitude,
        longitude: position.coords.longitude,
      });

      if (response.success) {
        toast({
          title: 'Tracking Ended',
          description: `Trip recorded: ${response.data.miles.toFixed(1)} miles`,
        });
        setActiveTracking(null);
        setPurpose('');
        loadMileageLogs();
      }
    } catch (error) {
      toast({
        title: 'Error',
        description: 'Failed to end tracking',
        variant: 'destructive',
      });
    } finally {
      setTracking(false);
    }
  };

  const formatDuration = (minutes?: number) => {
    if (!minutes) return 'N/A';
    const hours = Math.floor(minutes / 60);
    const mins = minutes % 60;
    return `${hours}h ${mins}m`;
  };

  const getStatusBadge = (status: string) => {
    switch (status) {
      case 'approved':
        return <Badge className="bg-green-100 text-green-800">Approved</Badge>;
      case 'rejected':
        return <Badge variant="destructive">Rejected</Badge>;
      default:
        return <Badge variant="secondary">Pending</Badge>;
    }
  };

  return (
    <div className="space-y-6">
      <Card>
        <CardHeader>
          <CardTitle className="flex items-center gap-2">
            <Car className="h-5 w-5" />
            Mileage Tracker
          </CardTitle>
        </CardHeader>
        <CardContent>
          {activeTracking ? (
            <div className="space-y-4">
              <div className="p-4 bg-blue-50 border border-blue-200 rounded-lg">
                <div className="flex items-center gap-2 text-blue-800 mb-2">
                  <Route className="h-5 w-5 animate-pulse" />
                  <span className="font-semibold">Tracking Active</span>
                </div>
                <p className="text-blue-700">Purpose: {activeTracking.purpose}</p>
                <p className="text-blue-600 text-sm">
                  Started: {new Date(activeTracking.startTime || '').toLocaleString()}
                </p>
              </div>

              <Button
                onClick={handleEndTracking}
                disabled={tracking}
                variant="destructive"
                className="w-full"
              >
                {tracking ? (
                  <div className="animate-spin rounded-full h-4 w-4 border-b-2 border-white mr-2" />
                ) : (
                  <Square className="h-4 w-4 mr-2" />
                )}
                End Trip
              </Button>
            </div>
          ) : (
            <div className="space-y-4">
              <div className="space-y-2">
                <Label htmlFor="purpose">Trip Purpose</Label>
                <Input
                  id="purpose"
                  value={purpose}
                  onChange={(e) => setPurpose(e.target.value)}
                  placeholder="e.g., Client visit, Supply run, Training"
                />
              </div>

              <Button
                onClick={handleStartTracking}
                disabled={tracking}
                className="w-full"
              >
                {tracking ? (
                  <div className="animate-spin rounded-full h-4 w-4 border-b-2 border-white mr-2" />
                ) : (
                  <Play className="h-4 w-4 mr-2" />
                )}
                Start Trip
              </Button>
            </div>
          )}
        </CardContent>
      </Card>

      <Card>
        <CardHeader>
          <CardTitle className="flex items-center gap-2">
            <Clock className="h-5 w-5" />
            Recent Trips
          </CardTitle>
        </CardHeader>
        <CardContent>
          {mileageLogs.length === 0 ? (
            <p className="text-muted-foreground text-center py-4">No trips recorded</p>
          ) : (
            <div className="space-y-3">
              {mileageLogs.map((log) => (
                <div
                  key={log.id}
                  className="flex items-center justify-between p-4 border rounded-lg"
                >
                  <div className="flex-1">
                    <div className="flex items-center gap-2 mb-1">
                      <h4 className="font-medium">{log.purpose}</h4>
                      {getStatusBadge(log.status)}
                    </div>
                    <div className="flex items-center gap-4 text-sm text-muted-foreground">
                      <span className="flex items-center gap-1">
                        <Route className="h-4 w-4" />
                        {log.miles.toFixed(1)} miles
                      </span>
                      <span className="flex items-center gap-1">
                        <Clock className="h-4 w-4" />
                        {formatDuration(log.durationMinutes)}
                      </span>
                      <span>{new Date(log.date).toLocaleDateString()}</span>
                    </div>
                  </div>
                  {log.reimbursementAmount && (
                    <div className="text-right">
                      <span className="flex items-center gap-1 font-medium text-green-600">
                        <DollarSign className="h-4 w-4" />
                        {log.reimbursementAmount.toFixed(2)}
                      </span>
                    </div>
                  )}
                </div>
              ))}
            </div>
          )}
        </CardContent>
      </Card>
    </div>
  );
}

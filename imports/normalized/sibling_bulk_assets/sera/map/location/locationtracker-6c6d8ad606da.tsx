"use client";

import React, { useState, useEffect, useCallback } from 'react';
import { MapPin, Navigation, AlertCircle, CheckCircle2, Clock } from 'lucide-react';
import { Button } from '@/components/ui/button';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Alert, AlertDescription } from '@/components/ui/alert';
import { Badge } from '@/components/ui/badge';
import locationApi from '@/lib/api/location';

interface LocationTrackerProps {
  onCheckIn?: (location: { latitude: number; longitude: number; address?: string }) => void;
  checkinType?: string;
  className?: string;
}

export function LocationTracker({
  onCheckIn,
  checkinType = 'task',
  className,
}: LocationTrackerProps) {
  const [location, setLocation] = useState<GeolocationPosition | null>(null);
  const [address, setAddress] = useState<string>('');
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [checkedIn, setCheckedIn] = useState(false);
  const [watchId, setWatchId] = useState<number | null>(null);

  // Get current position
  const getCurrentLocation = useCallback(() => {
    setLoading(true);
    setError(null);

    if (!navigator.geolocation) {
      setError('Geolocation is not supported by your browser');
      setLoading(false);
      return;
    }

    navigator.geolocation.getCurrentPosition(
      (position) => {
        setLocation(position);
        setLoading(false);
        // In production, would reverse geocode to get address
        setAddress(`${position.coords.latitude.toFixed(6)}, ${position.coords.longitude.toFixed(6)}`);
      },
      (err) => {
        setError(`Failed to get location: ${err.message}`);
        setLoading(false);
      },
      {
        enableHighAccuracy: true,
        timeout: 10000,
        maximumAge: 0,
      }
    );
  }, []);

  // Start watching position
  const startTracking = useCallback(() => {
    if (!navigator.geolocation) {
      setError('Geolocation is not supported by your browser');
      return;
    }

    const id = navigator.geolocation.watchPosition(
      (position) => {
        setLocation(position);
      },
      (err) => {
        setError(`Tracking error: ${err.message}`);
      },
      {
        enableHighAccuracy: true,
        timeout: 10000,
        maximumAge: 10000,
      }
    );

    setWatchId(id);
  }, []);

  // Stop watching position
  const stopTracking = useCallback(() => {
    if (watchId !== null) {
      navigator.geolocation.clearWatch(watchId);
      setWatchId(null);
    }
  }, [watchId]);

  // Handle check-in
  const handleCheckIn = async () => {
    if (!location) return;

    setLoading(true);
    try {
      const response = await locationApi.recordCheckin({
        checkinType,
        latitude: location.coords.latitude,
        longitude: location.coords.longitude,
        accuracyMeters: location.coords.accuracy,
        altitude: location.coords.altitude || undefined,
        address: address || undefined,
      });

      if (response.data) {
        setCheckedIn(true);
        onCheckIn?.({
          latitude: location.coords.latitude,
          longitude: location.coords.longitude,
          address: address || undefined,
        });
      }
    } catch (err) {
      setError('Failed to record check-in. Please try again.');
    } finally {
      setLoading(false);
    }
  };

  // Cleanup on unmount
  useEffect(() => {
    return () => {
      stopTracking();
    };
  }, [stopTracking]);

  return (
    <Card className={className}>
      <CardHeader>
        <CardTitle className="flex items-center gap-2">
          <MapPin className="h-5 w-5" />
          Location Check-in
        </CardTitle>
      </CardHeader>
      <CardContent className="space-y-4">
        {error && (
          <Alert variant="destructive">
            <AlertCircle className="h-4 w-4" />
            <AlertDescription>{error}</AlertDescription>
          </Alert>
        )}

        {checkedIn ? (
          <Alert className="bg-green-50 border-green-200">
            <CheckCircle2 className="h-4 w-4 text-green-600" />
            <AlertDescription className="text-green-800">
              Successfully checked in at {new Date().toLocaleTimeString()}
            </AlertDescription>
          </Alert>
        ) : (
          <>
            {/* Location Status */}
            <div className="bg-muted rounded-lg p-4 space-y-3">
              <div className="flex items-center justify-between">
                <span className="text-sm font-medium">GPS Status</span>
                <Badge variant={location ? 'default' : 'secondary'}>
                  {location ? 'Located' : 'Not Located'}
                </Badge>
              </div>

              {location ? (
                <div className="space-y-2 text-sm">
                  <div className="flex justify-between">
                    <span className="text-muted-foreground">Latitude:</span>
                    <span>{location.coords.latitude.toFixed(6)}</span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-muted-foreground">Longitude:</span>
                    <span>{location.coords.longitude.toFixed(6)}</span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-muted-foreground">Accuracy:</span>
                    <span>±{Math.round(location.coords.accuracy)}m</span>
                  </div>
                  {location.coords.altitude && (
                    <div className="flex justify-between">
                      <span className="text-muted-foreground">Altitude:</span>
                      <span>{Math.round(location.coords.altitude)}m</span>
                    </div>
                  )}
                </div>
              ) : (
                <p className="text-sm text-muted-foreground text-center py-2">
                  Location not yet acquired
                </p>
              )}
            </div>

            {/* Address Display */}
            {address && (
              <div className="flex items-start gap-2">
                <Navigation className="h-4 w-4 mt-0.5 text-muted-foreground" />
                <p className="text-sm">{address}</p>
              </div>
            )}

            {/* Actions */}
            <div className="flex gap-2">
              {!location ? (
                <Button onClick={getCurrentLocation} disabled={loading} className="flex-1">
                  {loading ? (
                    <>
                      <Clock className="mr-2 h-4 w-4 animate-spin" />
                      Getting Location...
                    </>
                  ) : (
                    <>
                      <MapPin className="mr-2 h-4 w-4" />
                      Get Location
                    </>
                  )}
                </Button>
              ) : (
                <Button onClick={handleCheckIn} disabled={loading} className="flex-1">
                  {loading ? 'Checking In...' : 'Check In'}
                </Button>
              )}
            </div>

            {/* Refresh Location */}
            {location && (
              <Button variant="ghost" size="sm" onClick={getCurrentLocation} className="w-full">
                Refresh Location
              </Button>
            )}
          </>
        )}
      </CardContent>
    </Card>
  );
}

export default LocationTracker;

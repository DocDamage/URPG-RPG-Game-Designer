'use client';

import React, { useState, useEffect } from 'react';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Badge } from '@/components/ui/badge';
import { recordCheckin, getLocationHistory, LocationCheckin } from '@/lib/api/location';
import { useToast } from '@/components/ui/use-toast';
import { MapPin, Clock, Battery, Navigation, CheckCircle } from 'lucide-react';

const checkinTypes = [
  { value: 'shift_start', label: 'Shift Start', color: 'bg-green-100 text-green-800' },
  { value: 'shift_end', label: 'Shift End', color: 'bg-red-100 text-red-800' },
  { value: 'task', label: 'Task Check-in', color: 'bg-blue-100 text-blue-800' },
  { value: 'emergency', label: 'Emergency', color: 'bg-red-600 text-white' },
];

export function LocationCheckinComponent() {
  const [loading, setLoading] = useState(false);
  const [location, setLocation] = useState<GeolocationPosition | null>(null);
  const [address, setAddress] = useState('');
  const [recentCheckins, setRecentCheckins] = useState<LocationCheckin[]>([]);
  const { toast } = useToast();

  useEffect(() => {
    loadRecentCheckins();
    getCurrentLocation();
  }, []);

  const getCurrentLocation = () => {
    if ('geolocation' in navigator) {
      navigator.geolocation.getCurrentPosition(
        (position) => {
          setLocation(position);
          // Reverse geocoding would go here
          setAddress('Current Location');
        },
        (error) => {
          toast({
            title: 'Location Error',
            description: 'Unable to get your location. Please enable location services.',
            variant: 'destructive',
          });
        },
        { enableHighAccuracy: true }
      );
    }
  };

  const loadRecentCheckins = async () => {
    try {
      const endDate = new Date();
      const startDate = new Date();
      startDate.setDate(startDate.getDate() - 7);

      const response = await getLocationHistory(startDate.toISOString(), endDate.toISOString());
      if (response.success) {
        setRecentCheckins(response.data.slice(0, 5));
      }
    } catch (error) {
      console.error('Failed to load check-ins', error);
    }
  };

  const handleCheckin = async (type: string) => {
    if (!location) {
      toast({
        title: 'Location Required',
        description: 'Please allow location access to check in.',
        variant: 'destructive',
      });
      return;
    }

    try {
      setLoading(true);
      const response = await recordCheckin({
        checkin_type: type,
        latitude: location.coords.latitude,
        longitude: location.coords.longitude,
        accuracy_meters: location.coords.accuracy,
        altitude: location.coords.altitude || undefined,
        address: address,
        battery_level: getBatteryLevel(),
      });

      if (response.success) {
        toast({
          title: 'Success',
          description: `Checked in successfully at ${new Date().toLocaleTimeString()}`,
        });
        loadRecentCheckins();
      }
    } catch (error) {
      toast({
        title: 'Error',
        description: 'Failed to check in',
        variant: 'destructive',
      });
    } finally {
      setLoading(false);
    }
  };

  const getBatteryLevel = () => {
    // Battery API is not widely supported, return null if unavailable
    const nav = navigator as any;
    if (nav.getBattery) {
      nav.getBattery().then((battery: any) => {
        return Math.round(battery.level * 100);
      });
    }
    return undefined;
  };

  const formatTime = (timestamp: string) => {
    return new Date(timestamp).toLocaleString('en-US', {
      month: 'short',
      day: 'numeric',
      hour: '2-digit',
      minute: '2-digit',
    });
  };

  const getCheckinTypeLabel = (type: string) => {
    const checkinType = checkinTypes.find((t) => t.value === type);
    return checkinType?.label || type;
  };

  return (
    <div className="space-y-6">
      <Card>
        <CardHeader>
          <CardTitle className="flex items-center gap-2">
            <MapPin className="h-5 w-5" />
            Check In
          </CardTitle>
        </CardHeader>
        <CardContent>
          {location ? (
            <div className="space-y-4">
              <div className="p-4 bg-muted rounded-lg">
                <div className="flex items-center gap-2 text-sm text-muted-foreground mb-2">
                  <Navigation className="h-4 w-4" />
                  Current Location
                </div>
                <p className="font-medium">
                  {location.coords.latitude.toFixed(6)}, {location.coords.longitude.toFixed(6)}
                </p>
                <p className="text-sm text-muted-foreground">
                  Accuracy: ±{Math.round(location.coords.accuracy)}m
                </p>
              </div>

              <div className="grid grid-cols-2 gap-3">
                {checkinTypes.map((type) => (
                  <Button
                    key={type.value}
                    onClick={() => handleCheckin(type.value)}
                    disabled={loading}
                    variant={type.value === 'emergency' ? 'destructive' : 'default'}
                    className={`h-auto py-4 ${type.value !== 'emergency' ? type.color : ''}`}
                  >
                    {loading ? (
                      <div className="animate-spin rounded-full h-4 w-4 border-b-2 border-current" />
                    ) : (
                      <>
                        <CheckCircle className="h-4 w-4 mr-2" />
                        {type.label}
                      </>
                    )}
                  </Button>
                ))}
              </div>
            </div>
          ) : (
            <div className="text-center py-8">
              <MapPin className="h-12 w-12 mx-auto mb-3 text-muted-foreground" />
              <p className="text-muted-foreground">Getting your location...</p>
              <Button onClick={getCurrentLocation} variant="outline" className="mt-4">
                Retry
              </Button>
            </div>
          )}
        </CardContent>
      </Card>

      <Card>
        <CardHeader>
          <CardTitle className="flex items-center gap-2">
            <Clock className="h-5 w-5" />
            Recent Check-ins
          </CardTitle>
        </CardHeader>
        <CardContent>
          {recentCheckins.length === 0 ? (
            <p className="text-muted-foreground text-center py-4">No recent check-ins</p>
          ) : (
            <div className="space-y-3">
              {recentCheckins.map((checkin) => (
                <div
                  key={checkin.id}
                  className="flex items-center justify-between p-3 border rounded-lg"
                >
                  <div className="flex items-center gap-3">
                    <MapPin className="h-4 w-4 text-muted-foreground" />
                    <div>
                      <p className="font-medium">{getCheckinTypeLabel(checkin.checkinType)}</p>
                      <p className="text-sm text-muted-foreground">
                        {formatTime(checkin.timestamp)}
                      </p>
                    </div>
                  </div>
                  <Badge variant="secondary">
                    {checkin.accuracyMeters
                      ? `±${Math.round(checkin.accuracyMeters)}m`
                      : 'N/A'}
                  </Badge>
                </div>
              ))}
            </div>
          )}
        </CardContent>
      </Card>
    </div>
  );
}

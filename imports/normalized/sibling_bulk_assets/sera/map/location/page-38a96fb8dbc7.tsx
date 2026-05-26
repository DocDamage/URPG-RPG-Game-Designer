"use client";

import React, { useState, useEffect } from 'react';
import { MapPin, Shield, AlertTriangle, Navigation, History, Settings, Activity } from 'lucide-react';
import { Button } from '@/components/ui/button';
import { Tabs, TabsContent, TabsList, TabsTrigger } from '@/components/ui/tabs';
import { Alert, AlertDescription } from '@/components/ui/alert';
import { Badge } from '@/components/ui/badge';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Switch } from '@/components/ui/switch';
import { Label } from '@/components/ui/label';
import { LocationTracker } from '@/components/location/LocationTracker';
import locationApi, { GeoFence, GeoFenceAlert, LocationPrivacySettings } from '@/lib/api/location';

export default function LocationPage() {
  const [activeTab, setActiveTab] = useState('tracker');
  const [geoFences, setGeoFences] = useState<GeoFence[]>([]);
  const [alerts, setAlerts] = useState<GeoFenceAlert[]>([]);
  const [privacySettings, setPrivacySettings] = useState<LocationPrivacySettings | null>(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(error);

  const fetchData = async () => {
    setLoading(true);
    setError(null);
    try {
      const [fencesRes, alertsRes, privacyRes] = await Promise.all([
        locationApi.getGeoFences(),
        locationApi.getGeoFenceAlerts({ acknowledged: false }),
        locationApi.getPrivacySettings(),
      ]);

      if (fencesRes.data) setGeoFences(fencesRes.data);
      if (alertsRes.data) setAlerts(alertsRes.data);
      if (privacyRes.data) setPrivacySettings(privacyRes.data);
    } catch (err) {
      console.error('Failed to fetch location data:', err);
      setError('Failed to load location data. Please try again.');
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchData();
  }, []);

  const handlePrivacyUpdate = async (key: keyof LocationPrivacySettings, value: boolean | number) => {
    if (!privacySettings) return;
    
    try {
      const updated = { ...privacySettings, [key]: value };
      const response = await locationApi.updatePrivacySettings({
        trackingEnabled: updated.trackingEnabled,
        trackOnlyDuringShifts: updated.trackOnlyDuringShifts,
        shareWithSupervisor: updated.shareWithSupervisor,
        shareWithTeam: updated.shareWithTeam,
        locationHistoryRetentionDays: updated.locationHistoryRetentionDays,
        allowEmergencyOverride: updated.allowEmergencyOverride,
      });
      if (response.data) {
        setPrivacySettings(response.data);
      }
    } catch (err) {
      console.error('Failed to update privacy settings:', err);
    }
  };

  const handleAcknowledgeAlert = async (alertId: string) => {
    try {
      await locationApi.acknowledgeAlert(alertId);
      setAlerts(alerts.filter(a => a.id !== alertId));
    } catch (err) {
      console.error('Failed to acknowledge alert:', err);
    }
  };

  return (
    <div className="container mx-auto py-6 space-y-6">
      {/* Header */}
      <div className="flex flex-col sm:flex-row sm:items-center sm:justify-between gap-4">
        <div>
          <h1 className="text-3xl font-bold tracking-tight">Location Services</h1>
          <p className="text-muted-foreground">
            GPS check-in, geo-fencing, and location tracking
          </p>
        </div>
      </div>

      {/* Error Alert */}
      {error && (
        <Alert variant="destructive">
          <AlertTriangle className="h-4 w-4" />
          <AlertDescription>{error}</AlertDescription>
        </Alert>
      )}

      {/* Stats */}
      <div className="grid grid-cols-2 md:grid-cols-4 gap-4">
        <Card>
          <CardContent className="p-4">
            <div className="flex items-center gap-3">
              <div className="p-2 bg-blue-100 rounded-lg">
                <MapPin className="h-5 w-5 text-blue-600" />
              </div>
              <div>
                <p className="text-sm text-muted-foreground">Geo-fences</p>
                <p className="text-2xl font-bold">{geoFences.length}</p>
              </div>
            </div>
          </CardContent>
        </Card>

        <Card>
          <CardContent className="p-4">
            <div className="flex items-center gap-3">
              <div className="p-2 bg-yellow-100 rounded-lg">
                <AlertTriangle className="h-5 w-5 text-yellow-600" />
              </div>
              <div>
                <p className="text-sm text-muted-foreground">Active Alerts</p>
                <p className="text-2xl font-bold">{alerts.length}</p>
              </div>
            </div>
          </CardContent>
        </Card>

        <Card>
          <CardContent className="p-4">
            <div className="flex items-center gap-3">
              <div className="p-2 bg-green-100 rounded-lg">
                <Navigation className="h-5 w-5 text-green-600" />
              </div>
              <div>
                <p className="text-sm text-muted-foreground">Tracking</p>
                <p className="text-2xl font-bold">
                  {privacySettings?.trackingEnabled ? 'On' : 'Off'}
                </p>
              </div>
            </div>
          </CardContent>
        </Card>

        <Card>
          <CardContent className="p-4">
            <div className="flex items-center gap-3">
              <div className="p-2 bg-purple-100 rounded-lg">
                <Shield className="h-5 w-5 text-purple-600" />
              </div>
              <div>
                <p className="text-sm text-muted-foreground">Privacy</p>
                <p className="text-2xl font-bold">
                  {privacySettings?.trackOnlyDuringShifts ? 'Work' : 'Always'}
                </p>
              </div>
            </div>
          </CardContent>
        </Card>
      </div>

      {/* Tabs */}
      <Tabs value={activeTab} onValueChange={setActiveTab}>
        <TabsList className="grid w-full grid-cols-4 lg:w-[500px]">
          <TabsTrigger value="tracker">
            <MapPin className="mr-2 h-4 w-4" />
            Tracker
          </TabsTrigger>
          <TabsTrigger value="geofences">
            <Shield className="mr-2 h-4 w-4" />
            Geo-fences
          </TabsTrigger>
          <TabsTrigger value="alerts">
            <AlertTriangle className="mr-2 h-4 w-4" />
            Alerts
          </TabsTrigger>
          <TabsTrigger value="settings">
            <Settings className="mr-2 h-4 w-4" />
            Settings
          </TabsTrigger>
        </TabsList>

        {/* Tracker Tab */}
        <TabsContent value="tracker" className="mt-6">
          <div className="max-w-md mx-auto">
            <LocationTracker
              onCheckIn={(location) => console.log('Checked in:', location)}
            />
          </div>
        </TabsContent>

        {/* Geo-fences Tab */}
        <TabsContent value="geofences" className="mt-6">
          {loading ? (
            <div className="text-center py-8 text-muted-foreground">Loading...</div>
          ) : geoFences.length === 0 ? (
            <div className="text-center py-12 bg-muted/50 rounded-lg">
              <Shield className="mx-auto h-12 w-12 text-muted-foreground" />
              <h3 className="mt-4 text-lg font-medium">No Geo-fences</h3>
              <p className="text-muted-foreground">Create geo-fences to monitor location boundaries</p>
            </div>
          ) : (
            <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
              {geoFences.map((fence) => (
                <Card key={fence.id}>
                  <CardHeader>
                    <div className="flex items-center justify-between">
                      <CardTitle className="text-lg">{fence.name}</CardTitle>
                      <Badge variant={fence.isActive ? 'default' : 'secondary'}>
                        {fence.isActive ? 'Active' : 'Inactive'}
                      </Badge>
                    </div>
                  </CardHeader>
                  <CardContent className="space-y-2">
                    <p className="text-sm text-muted-foreground">{fence.description}</p>
                    <div className="flex items-center gap-2 text-sm">
                      <MapPin className="h-4 w-4" />
                      <span>{fence.address || `${fence.centerLatitude?.toFixed(4)}, ${fence.centerLongitude?.toFixed(4)}`}</span>
                    </div>
                    <div className="flex items-center gap-2 text-sm">
                      <Activity className="h-4 w-4" />
                      <span>Radius: {fence.radiusMeters}m</span>
                    </div>
                    <div className="flex gap-2 mt-4">
                      <Badge variant="outline">
                        Alert on {fence.alertOnEnter ? 'Enter' : fence.alertOnExit ? 'Exit' : 'None'}
                      </Badge>
                      <Badge variant="outline">Type: {fence.fenceType}</Badge>
                    </div>
                  </CardContent>
                </Card>
              ))}
            </div>
          )}
        </TabsContent>

        {/* Alerts Tab */}
        <TabsContent value="alerts" className="mt-6">
          {alerts.length === 0 ? (
            <div className="text-center py-12 bg-muted/50 rounded-lg">
              <AlertTriangle className="mx-auto h-12 w-12 text-muted-foreground" />
              <h3 className="mt-4 text-lg font-medium">No Active Alerts</h3>
              <p className="text-muted-foreground">All geo-fence alerts have been acknowledged</p>
            </div>
          ) : (
            <div className="space-y-4">
              {alerts.map((alert) => (
                <Alert key={alert.id} variant="default" className="bg-yellow-50 border-yellow-200">
                  <AlertTriangle className="h-4 w-4 text-yellow-600" />
                  <AlertDescription className="flex items-center justify-between">
                    <div>
                      <p className="font-medium">
                        {alert.alertType === 'enter' ? 'Entered' : 'Exited'} Geo-fence
                      </p>
                      <p className="text-sm text-muted-foreground">
                        {new Date(alert.timestamp).toLocaleString()}
                      </p>
                      <p className="text-sm">
                        Location: {alert.locationLat.toFixed(6)}, {alert.locationLng.toFixed(6)}
                      </p>
                    </div>
                    <Button size="sm" onClick={() => handleAcknowledgeAlert(alert.id)}>
                      Acknowledge
                    </Button>
                  </AlertDescription>
                </Alert>
              ))}
            </div>
          )}
        </TabsContent>

        {/* Settings Tab */}
        <TabsContent value="settings" className="mt-6">
          {privacySettings && (
            <Card className="max-w-lg">
              <CardHeader>
                <CardTitle>Location Privacy Settings</CardTitle>
              </CardHeader>
              <CardContent className="space-y-6">
                <div className="flex items-center justify-between">
                  <div className="space-y-0.5">
                    <Label htmlFor="tracking">Location Tracking</Label>
                    <p className="text-sm text-muted-foreground">
                      Allow the app to access your location
                    </p>
                  </div>
                  <Switch
                    id="tracking"
                    checked={privacySettings.trackingEnabled}
                    onCheckedChange={(checked) => handlePrivacyUpdate('trackingEnabled', checked)}
                  />
                </div>

                <div className="flex items-center justify-between">
                  <div className="space-y-0.5">
                    <Label htmlFor="shifts">Track Only During Shifts</Label>
                    <p className="text-sm text-muted-foreground">
                      Only record location when scheduled to work
                    </p>
                  </div>
                  <Switch
                    id="shifts"
                    checked={privacySettings.trackOnlyDuringShifts}
                    onCheckedChange={(checked) => handlePrivacyUpdate('trackOnlyDuringShifts', checked)}
                  />
                </div>

                <div className="flex items-center justify-between">
                  <div className="space-y-0.5">
                    <Label htmlFor="supervisor">Share with Supervisor</Label>
                    <p className="text-sm text-muted-foreground">
                      Allow supervisors to see your location
                    </p>
                  </div>
                  <Switch
                    id="supervisor"
                    checked={privacySettings.shareWithSupervisor}
                    onCheckedChange={(checked) => handlePrivacyUpdate('shareWithSupervisor', checked)}
                  />
                </div>

                <div className="flex items-center justify-between">
                  <div className="space-y-0.5">
                    <Label htmlFor="team">Share with Team</Label>
                    <p className="text-sm text-muted-foreground">
                      Allow team members to see your location
                    </p>
                  </div>
                  <Switch
                    id="team"
                    checked={privacySettings.shareWithTeam}
                    onCheckedChange={(checked) => handlePrivacyUpdate('shareWithTeam', checked)}
                  />
                </div>

                <div className="flex items-center justify-between">
                  <div className="space-y-0.5">
                    <Label htmlFor="emergency">Emergency Override</Label>
                    <p className="text-sm text-muted-foreground">
                      Allow emergency location sharing
                    </p>
                  </div>
                  <Switch
                    id="emergency"
                    checked={privacySettings.allowEmergencyOverride}
                    onCheckedChange={(checked) => handlePrivacyUpdate('allowEmergencyOverride', checked)}
                  />
                </div>

                <div className="pt-4 border-t">
                  <p className="text-sm text-muted-foreground">
                    Last updated: {new Date(privacySettings.updatedAt).toLocaleString()}
                  </p>
                </div>
              </CardContent>
            </Card>
          )}
        </TabsContent>
      </Tabs>
    </div>
  );
}

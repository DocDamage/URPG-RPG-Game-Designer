'use client';

import React, { useState, useEffect } from 'react';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Label } from '@/components/ui/label';
import { Badge } from '@/components/ui/badge';
import { Switch } from '@/components/ui/switch';
import { getGeoFences, createGeoFence, GeoFence } from '@/lib/api/location';
import { useToast } from '@/components/ui/use-toast';
import { MapPin, Plus, Circle, Square, AlertTriangle, Bell } from 'lucide-react';
import {
  Dialog,
  DialogContent,
  DialogDescription,
  DialogFooter,
  DialogHeader,
  DialogTitle,
  DialogTrigger,
} from '@/components/ui/dialog';
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from '@/components/ui/select';

const fenceTypes = [
  { value: 'safe_zone', label: 'Safe Zone', color: 'bg-green-100 text-green-800' },
  { value: 'restricted', label: 'Restricted Area', color: 'bg-red-100 text-red-800' },
  { value: 'waypoint', label: 'Waypoint', color: 'bg-blue-100 text-blue-800' },
  { value: 'home_base', label: 'Home Base', color: 'bg-purple-100 text-purple-800' },
];

export function GeoFenceManager() {
  const [geoFences, setGeoFences] = useState<GeoFence[]>([]);
  const [loading, setLoading] = useState(true);
  const [isCreateOpen, setIsCreateOpen] = useState(false);
  const { toast } = useToast();

  // New fence form state
  const [newFence, setNewFence] = useState({
    name: '',
    fenceType: 'safe_zone',
    shape: 'circle',
    radiusMeters: 100,
    alertOnEnter: true,
    alertOnExit: false,
  });
  const [currentLocation, setCurrentLocation] = useState<{ lat: number; lng: number } | null>(null);

  useEffect(() => {
    loadGeoFences();
    getLocation();
  }, []);

  const getLocation = () => {
    if ('geolocation' in navigator) {
      navigator.geolocation.getCurrentPosition(
        (position) => {
          setCurrentLocation({
            lat: position.coords.latitude,
            lng: position.coords.longitude,
          });
        },
        () => {
          // Silent fail - user can enter coordinates manually
        }
      );
    }
  };

  const loadGeoFences = async () => {
    try {
      setLoading(true);
      const response = await getGeoFences();
      if (response.success) {
        setGeoFences(response.data);
      }
    } catch (error) {
      toast({
        title: 'Error',
        description: 'Failed to load geo-fences',
        variant: 'destructive',
      });
    } finally {
      setLoading(false);
    }
  };

  const handleCreateFence = async () => {
    if (!newFence.name || !currentLocation) {
      toast({
        title: 'Validation Error',
        description: 'Please provide a name and allow location access',
        variant: 'destructive',
      });
      return;
    }

    try {
      const response = await createGeoFence({
        name: newFence.name,
        fence_type: newFence.fenceType,
        shape: newFence.shape,
        center_latitude: currentLocation.lat,
        center_longitude: currentLocation.lng,
        radius_meters: newFence.radiusMeters,
        alert_on_enter: newFence.alertOnEnter,
        alert_on_exit: newFence.alertOnExit,
      });

      if (response.success) {
        toast({
          title: 'Success',
          description: 'Geo-fence created successfully',
        });
        setIsCreateOpen(false);
        loadGeoFences();
        // Reset form
        setNewFence({
          name: '',
          fenceType: 'safe_zone',
          shape: 'circle',
          radiusMeters: 100,
          alertOnEnter: true,
          alertOnExit: false,
        });
      }
    } catch (error) {
      toast({
        title: 'Error',
        description: 'Failed to create geo-fence',
        variant: 'destructive',
      });
    }
  };

  const getFenceTypeBadge = (type: string) => {
    const fenceType = fenceTypes.find((t) => t.value === type);
    return fenceType ? (
      <Badge className={fenceType.color}>{fenceType.label}</Badge>
    ) : (
      <Badge>{type}</Badge>
    );
  };

  if (loading) {
    return (
      <Card>
        <CardContent className="p-6">
          <div className="flex items-center justify-center h-32">
            <div className="animate-spin rounded-full h-8 w-8 border-b-2 border-primary"></div>
          </div>
        </CardContent>
      </Card>
    );
  }

  return (
    <Card>
      <CardHeader className="flex flex-row items-center justify-between">
        <CardTitle className="flex items-center gap-2">
          <MapPin className="h-5 w-5" />
          Geo-fences
        </CardTitle>
        <Dialog open={isCreateOpen} onOpenChange={setIsCreateOpen}>
          <DialogTrigger asChild>
            <Button>
              <Plus className="h-4 w-4 mr-2" />
              Add Geo-fence
            </Button>
          </DialogTrigger>
          <DialogContent>
            <DialogHeader>
              <DialogTitle>Create Geo-fence</DialogTitle>
              <DialogDescription>
                Define a virtual boundary for location-based alerts
              </DialogDescription>
            </DialogHeader>
            <div className="space-y-4 py-4">
              <div className="space-y-2">
                <Label htmlFor="name">Name</Label>
                <Input
                  id="name"
                  value={newFence.name}
                  onChange={(e) => setNewFence({ ...newFence, name: e.target.value })}
                  placeholder="e.g., Main Campus"
                />
              </div>

              <div className="space-y-2">
                <Label>Type</Label>
                <Select
                  value={newFence.fenceType}
                  onValueChange={(value) => setNewFence({ ...newFence, fenceType: value })}
                >
                  <SelectTrigger>
                    <SelectValue />
                  </SelectTrigger>
                  <SelectContent>
                    {fenceTypes.map((type) => (
                      <SelectItem key={type.value} value={type.value}>
                        {type.label}
                      </SelectItem>
                    ))}
                  </SelectContent>
                </Select>
              </div>

              <div className="space-y-2">
                <Label>Shape</Label>
                <div className="flex gap-2">
                  <Button
                    type="button"
                    variant={newFence.shape === 'circle' ? 'default' : 'outline'}
                    className="flex-1"
                    onClick={() => setNewFence({ ...newFence, shape: 'circle' })}
                  >
                    <Circle className="h-4 w-4 mr-2" />
                    Circle
                  </Button>
                  <Button
                    type="button"
                    variant={newFence.shape === 'polygon' ? 'default' : 'outline'}
                    className="flex-1"
                    onClick={() => setNewFence({ ...newFence, shape: 'polygon' })}
                  >
                    <Square className="h-4 w-4 mr-2" />
                    Polygon
                  </Button>
                </div>
              </div>

              {newFence.shape === 'circle' && (
                <div className="space-y-2">
                  <Label htmlFor="radius">Radius (meters)</Label>
                  <Input
                    id="radius"
                    type="number"
                    value={newFence.radiusMeters}
                    onChange={(e) =>
                      setNewFence({ ...newFence, radiusMeters: parseInt(e.target.value) })
                    }
                    min={10}
                    max={10000}
                  />
                </div>
              )}

              <div className="space-y-4">
                <div className="flex items-center justify-between">
                  <div className="space-y-0.5">
                    <Label>Alert on Enter</Label>
                    <p className="text-sm text-muted-foreground">
                      Notify when someone enters this area
                    </p>
                  </div>
                  <Switch
                    checked={newFence.alertOnEnter}
                    onCheckedChange={(checked) =>
                      setNewFence({ ...newFence, alertOnEnter: checked })
                    }
                  />
                </div>

                <div className="flex items-center justify-between">
                  <div className="space-y-0.5">
                    <Label>Alert on Exit</Label>
                    <p className="text-sm text-muted-foreground">
                      Notify when someone leaves this area
                    </p>
                  </div>
                  <Switch
                    checked={newFence.alertOnExit}
                    onCheckedChange={(checked) =>
                      setNewFence({ ...newFence, alertOnExit: checked })
                    }
                  />
                </div>
              </div>

              {currentLocation && (
                <div className="p-3 bg-muted rounded-lg text-sm">
                  <p className="text-muted-foreground">Center Location:</p>
                  <p className="font-medium">
                    {currentLocation.lat.toFixed(6)}, {currentLocation.lng.toFixed(6)}
                  </p>
                </div>
              )}
            </div>
            <DialogFooter>
              <Button variant="outline" onClick={() => setIsCreateOpen(false)}>
                Cancel
              </Button>
              <Button onClick={handleCreateFence}>Create</Button>
            </DialogFooter>
          </DialogContent>
        </Dialog>
      </CardHeader>
      <CardContent>
        {geoFences.length === 0 ? (
          <div className="text-center py-8 text-muted-foreground">
            <MapPin className="h-12 w-12 mx-auto mb-3 opacity-50" />
            <p>No geo-fences configured</p>
            <p className="text-sm">Create geo-fences to monitor location-based activities</p>
          </div>
        ) : (
          <div className="space-y-3">
            {geoFences.map((fence) => (
              <div
                key={fence.id}
                className="flex items-center justify-between p-4 border rounded-lg"
              >
                <div className="flex-1">
                  <div className="flex items-center gap-2 mb-1">
                    <h4 className="font-medium">{fence.name}</h4>
                    {getFenceTypeBadge(fence.fenceType)}
                  </div>
                  <div className="flex items-center gap-4 text-sm text-muted-foreground">
                    <span className="flex items-center gap-1">
                      <Circle className="h-3 w-3" />
                      {fence.shape === 'circle'
                        ? `${fence.radiusMeters}m radius`
                        : 'Polygon'}
                    </span>
                    {fence.alertOnEnter && (
                      <span className="flex items-center gap-1 text-green-600">
                        <Bell className="h-3 w-3" />
                        Enter alert
                      </span>
                    )}
                    {fence.alertOnExit && (
                      <span className="flex items-center gap-1 text-amber-600">
                        <AlertTriangle className="h-3 w-3" />
                        Exit alert
                      </span>
                    )}
                  </div>
                </div>
              </div>
            ))}
          </div>
        )}
      </CardContent>
    </Card>
  );
}

'use client';

import React, { useState, useEffect } from 'react';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Badge } from '@/components/ui/badge';
import { 
  logTemperature, 
  getTemperatureHistory,
  acknowledgeTemperatureAlert,
  TemperatureLog 
} from '@/lib/api/medicationInventory';
import { useToast } from '@/components/ui/use-toast';
import { Thermometer, AlertTriangle, History, CheckCircle, TrendingUp } from 'lucide-react';
import { Label } from '@/components/ui/label';
import {
  LineChart,
  Line,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  ResponsiveContainer,
} from 'recharts';

interface TemperatureDataPoint {
  time: string;
  temperature: number;
  location: string;
}

export function TemperatureMonitor() {
  const [temperature, setTemperature] = useState('');
  const [location, setLocation] = useState('medication_fridge');
  const [humidity, setHumidity] = useState('');
  const [logs, setLogs] = useState<TemperatureLog[]>([]);
  const [chartData, setChartData] = useState<TemperatureDataPoint[]>([]);
  const [loading, setLoading] = useState(false);
  const { toast } = useToast();

  useEffect(() => {
    loadTemperatureHistory();
  }, []);

  const loadTemperatureHistory = async () => {
    try {
      const endDate = new Date();
      const startDate = new Date();
      startDate.setDate(startDate.getDate() - 7);

      const response = await getTemperatureHistory(
        location,
        startDate.toISOString(),
        endDate.toISOString()
      );

      if (response.success) {
        setLogs(response.data);
        
        // Format data for chart
        const formatted = response.data.map((log) => ({
          time: new Date(log.loggedAt).toLocaleDateString('en-US', {
            month: 'short',
            day: 'numeric',
            hour: '2-digit',
          }),
          temperature: log.temperature,
          location: log.location,
        }));
        setChartData(formatted);
      }
    } catch (error) {
      console.error('Failed to load temperature history', error);
    }
  };

  const handleLogTemperature = async () => {
    if (!temperature || !location) {
      toast({
        title: 'Validation Error',
        description: 'Please enter temperature and location',
        variant: 'destructive',
      });
      return;
    }

    try {
      setLoading(true);
      const temp = parseFloat(temperature);
      
      // Check for alerts (typical fridge temp: 2-8°C)
      const isAlert = temp < 2 || temp > 8;
      const alertType = temp < 2 ? 'too_low' : temp > 8 ? 'too_high' : undefined;

      const response = await logTemperature({
        location,
        temperature: temp,
        humidity: humidity ? parseFloat(humidity) : undefined,
        is_alert: isAlert,
        alert_type: alertType,
      });

      if (response.success) {
        toast({
          title: 'Success',
          description: isAlert 
            ? `Temperature logged. ALERT: Temperature ${alertType?.replace('_', ' ')}!` 
            : 'Temperature logged successfully',
          variant: isAlert ? 'destructive' : 'default',
        });
        
        setTemperature('');
        setHumidity('');
        loadTemperatureHistory();
      }
    } catch (error) {
      toast({
        title: 'Error',
        description: 'Failed to log temperature',
        variant: 'destructive',
      });
    } finally {
      setLoading(false);
    }
  };

  const handleAcknowledge = async (logId: string) => {
    try {
      await acknowledgeTemperatureAlert(logId);
      toast({
        title: 'Success',
        description: 'Alert acknowledged',
      });
      loadTemperatureHistory();
    } catch (error) {
      toast({
        title: 'Error',
        description: 'Failed to acknowledge alert',
        variant: 'destructive',
      });
    }
  };

  const getTemperatureColor = (temp: number) => {
    if (temp < 2 || temp > 8) return 'text-red-600';
    if (temp < 3 || temp > 7) return 'text-yellow-600';
    return 'text-green-600';
  };

  const activeAlerts = logs.filter((log) => log.isAlert && !log.acknowledgedBy);

  return (
    <div className="space-y-6">
      {/* Active Alerts */}
      {activeAlerts.length > 0 && (
        <Card className="border-red-200 bg-red-50">
          <CardHeader>
            <CardTitle className="flex items-center gap-2 text-red-800">
              <AlertTriangle className="h-5 w-5" />
              Active Temperature Alerts
            </CardTitle>
          </CardHeader>
          <CardContent>
            <div className="space-y-2">
              {activeAlerts.map((alert) => (
                <div
                  key={alert.id}
                  className="flex items-center justify-between p-3 bg-white rounded-lg"
                >
                  <div>
                    <p className="font-medium">
                      {alert.location}: {alert.temperature}°C
                    </p>
                    <p className="text-sm text-muted-foreground">
                      {new Date(alert.loggedAt).toLocaleString()}
                    </p>
                  </div>
                  <Button
                    size="sm"
                    variant="outline"
                    onClick={() => handleAcknowledge(alert.id)}
                  >
                    <CheckCircle className="h-4 w-4 mr-1" />
                    Acknowledge
                  </Button>
                </div>
              ))}
            </div>
          </CardContent>
        </Card>
      )}

      {/* Temperature Chart */}
      {chartData.length > 0 && (
        <Card>
          <CardHeader>
            <CardTitle className="flex items-center gap-2">
              <TrendingUp className="h-5 w-5" />
              Temperature History
            </CardTitle>
          </CardHeader>
          <CardContent>
            <div className="h-[300px]">
              <ResponsiveContainer width="100%" height="100%">
                <LineChart data={chartData}>
                  <CartesianGrid strokeDasharray="3 3" />
                  <XAxis dataKey="time" />
                  <YAxis domain={[0, 15]} />
                  <Tooltip />
                  <Line
                    type="monotone"
                    dataKey="temperature"
                    stroke="#2563eb"
                    strokeWidth={2}
                    dot={false}
                  />
                </LineChart>
              </ResponsiveContainer>
            </div>
            <div className="flex justify-center gap-4 mt-4 text-sm">
              <div className="flex items-center gap-2">
                <div className="w-3 h-3 bg-green-500 rounded-full" />
                <span>Normal (2-8°C)</span>
              </div>
              <div className="flex items-center gap-2">
                <div className="w-3 h-3 bg-yellow-500 rounded-full" />
                <span>Warning (near threshold)</span>
              </div>
              <div className="flex items-center gap-2">
                <div className="w-3 h-3 bg-red-500 rounded-full" />
                <span>Alert (outside range)</span>
              </div>
            </div>
          </CardContent>
        </Card>
      )}

      {/* Log Temperature */}
      <Card>
        <CardHeader>
          <CardTitle className="flex items-center gap-2">
            <Thermometer className="h-5 w-5" />
            Log Temperature
          </CardTitle>
        </CardHeader>
        <CardContent className="space-y-4">
          <div className="grid grid-cols-2 gap-4">
            <div className="space-y-2">
              <Label htmlFor="location">Location</Label>
              <select
                id="location"
                value={location}
                onChange={(e) => setLocation(e.target.value)}
                className="w-full p-2 border rounded-md"
              >
                <option value="medication_fridge">Medication Fridge</option>
                <option value="medication_room">Medication Room</option>
                <option value="freezer">Freezer</option>
              </select>
            </div>
            <div className="space-y-2">
              <Label htmlFor="temperature">Temperature (°C)</Label>
              <Input
                id="temperature"
                type="number"
                step="0.1"
                value={temperature}
                onChange={(e) => setTemperature(e.target.value)}
                placeholder="e.g., 5.5"
              />
            </div>
          </div>
          <div className="space-y-2">
            <Label htmlFor="humidity">Humidity % (Optional)</Label>
            <Input
              id="humidity"
              type="number"
              value={humidity}
              onChange={(e) => setHumidity(e.target.value)}
              placeholder="e.g., 45"
            />
          </div>
          <Button 
            onClick={handleLogTemperature} 
            disabled={loading}
            className="w-full"
          >
            {loading ? (
              <div className="animate-spin rounded-full h-4 w-4 border-b-2 border-white mr-2" />
            ) : (
              <Thermometer className="h-4 w-4 mr-2" />
            )}
            Log Temperature
          </Button>
        </CardContent>
      </Card>

      {/* Recent Logs */}
      <Card>
        <CardHeader>
          <CardTitle className="flex items-center gap-2">
            <History className="h-5 w-5" />
            Recent Logs
          </CardTitle>
        </CardHeader>
        <CardContent>
          {logs.length === 0 ? (
            <p className="text-muted-foreground text-center py-4">No temperature logs</p>
          ) : (
            <div className="space-y-2">
              {logs.slice(0, 10).map((log) => (
                <div
                  key={log.id}
                  className={`flex items-center justify-between p-3 rounded-lg ${
                    log.isAlert ? 'bg-red-50 border border-red-200' : 'bg-muted'
                  }`}
                >
                  <div>
                    <div className="flex items-center gap-2">
                      <span className={`font-bold text-lg ${getTemperatureColor(log.temperature)}`}>
                        {log.temperature}°C
                      </span>
                      <span className="text-muted-foreground">{log.location}</span>
                    </div>
                    <p className="text-sm text-muted-foreground">
                      {new Date(log.loggedAt).toLocaleString()}
                    </p>
                  </div>
                  {log.isAlert && (
                    <Badge variant="destructive">
                      <AlertTriangle className="h-3 w-3 mr-1" />
                      {log.alertType?.replace('_', ' ')}
                    </Badge>
                  )}
                  {log.acknowledgedBy && (
                    <Badge variant="secondary">Acknowledged</Badge>
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

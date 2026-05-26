'use client';

import React, { useState, useEffect } from 'react';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from '@/components/ui/select';
import { Badge } from '@/components/ui/badge';
import { ArrowRightLeft, User, Clock, CheckCircle, XCircle } from 'lucide-react';
import { getMySchedule, getShiftSwaps, requestShiftSwap, approveShiftSwap, ShiftAssignment, ShiftSwap as ShiftSwapType } from '@/lib/api/scheduling';
import { useToast } from '@/components/ui/use-toast';

export function ShiftSwap() {
  const [myShifts, setMyShifts] = useState<ShiftAssignment[]>([]);
  const [swaps, setSwaps] = useState<ShiftSwapType[]>([]);
  const [selectedShift, setSelectedShift] = useState('');
  const [targetShift, setTargetShift] = useState('');
  const [loading, setLoading] = useState(true);
  const { toast } = useToast();

  useEffect(() => {
    loadData();
  }, []);

  const loadData = async () => {
    try {
      setLoading(true);
      const [shiftsResponse, swapsResponse] = await Promise.all([
        getMySchedule(),
        getShiftSwaps(),
      ]);

      if (shiftsResponse.success) {
        setMyShifts(shiftsResponse.data);
      }

      if (swapsResponse.success) {
        setSwaps(swapsResponse.data);
      }
    } catch (error) {
      toast({
        title: 'Error',
        description: 'Failed to load data',
        variant: 'destructive',
      });
    } finally {
      setLoading(false);
    }
  };

  const handleRequestSwap = async () => {
    if (!selectedShift) {
      toast({
        title: 'Validation Error',
        description: 'Please select a shift to swap',
        variant: 'destructive',
      });
      return;
    }

    try {
      const response = await requestShiftSwap(selectedShift, targetShift || undefined);

      if (response.success) {
        toast({
          title: 'Success',
          description: 'Shift swap requested successfully',
        });
        loadData();
        setSelectedShift('');
        setTargetShift('');
      }
    } catch (error) {
      toast({
        title: 'Error',
        description: 'Failed to request shift swap',
        variant: 'destructive',
      });
    }
  };

  const handleApproveSwap = async (swapId: string) => {
    try {
      const response = await approveShiftSwap(swapId);

      if (response.success) {
        toast({
          title: 'Success',
          description: 'Shift swap approved',
        });
        loadData();
      }
    } catch (error) {
      toast({
        title: 'Error',
        description: 'Failed to approve shift swap',
        variant: 'destructive',
      });
    }
  };

  const getStatusBadge = (status: string) => {
    switch (status) {
      case 'pending':
        return <Badge variant="secondary">Pending</Badge>;
      case 'approved':
        return <Badge className="bg-yellow-100 text-yellow-800">Approved</Badge>;
      case 'completed':
        return <Badge className="bg-green-100 text-green-800">Completed</Badge>;
      case 'denied':
        return <Badge variant="destructive">Denied</Badge>;
      default:
        return <Badge variant="secondary">{status}</Badge>;
    }
  };

  const formatShiftTime = (startTime: string) => {
    const date = new Date(startTime);
    return date.toLocaleDateString('en-US', { 
      weekday: 'short', 
      month: 'short', 
      day: 'numeric',
      hour: '2-digit',
      minute: '2-digit'
    });
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
    <div className="space-y-6">
      <Card>
        <CardHeader>
          <CardTitle className="flex items-center gap-2">
            <ArrowRightLeft className="h-5 w-5" />
            Request Shift Swap
          </CardTitle>
        </CardHeader>
        <CardContent>
          <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
            <div className="space-y-2">
              <label className="text-sm font-medium">Your Shift</label>
              <Select value={selectedShift} onValueChange={setSelectedShift}>
                <SelectTrigger>
                  <SelectValue placeholder="Select your shift" />
                </SelectTrigger>
                <SelectContent>
                  {myShifts
                    .filter((shift) => new Date(shift.startTime) > new Date())
                    .map((shift) => (
                      <SelectItem key={shift.id} value={shift.id}>
                        {formatShiftTime(shift.startTime)}
                      </SelectItem>
                    ))}
                </SelectContent>
              </Select>
            </div>
            <div className="space-y-2">
              <label className="text-sm font-medium">Target Shift (Optional)</label>
              <Select value={targetShift} onValueChange={setTargetShift}>
                <SelectTrigger>
                  <SelectValue placeholder="Select target shift" />
                </SelectTrigger>
                <SelectContent>
                  {myShifts
                    .filter((shift) => shift.id !== selectedShift && new Date(shift.startTime) > new Date())
                    .map((shift) => (
                      <SelectItem key={shift.id} value={shift.id}>
                        {formatShiftTime(shift.startTime)}
                      </SelectItem>
                    ))}
                </SelectContent>
              </Select>
            </div>
          </div>
          <Button 
            className="mt-4" 
            onClick={handleRequestSwap}
            disabled={!selectedShift}
          >
            <ArrowRightLeft className="h-4 w-4 mr-2" />
            Request Swap
          </Button>
        </CardContent>
      </Card>

      <Card>
        <CardHeader>
          <CardTitle>Pending Swaps</CardTitle>
        </CardHeader>
        <CardContent>
          {swaps.length === 0 ? (
            <p className="text-muted-foreground text-center py-4">No shift swaps</p>
          ) : (
            <div className="space-y-3">
              {swaps.map((swap) => (
                <div
                  key={swap.id}
                  className="flex items-center justify-between p-3 border rounded-lg"
                >
                  <div className="flex items-center gap-3">
                    <div className="flex flex-col">
                      <span className="font-medium">
                        Swap Request #{swap.id.slice(0, 8)}
                      </span>
                      <span className="text-sm text-muted-foreground">
                        Requested {new Date(swap.requestedAt).toLocaleDateString()}
                      </span>
                    </div>
                  </div>
                  <div className="flex items-center gap-2">
                    {getStatusBadge(swap.status)}
                    {swap.status === 'pending' && (
                      <Button
                        size="sm"
                        variant="outline"
                        onClick={() => handleApproveSwap(swap.id)}
                      >
                        <CheckCircle className="h-4 w-4" />
                      </Button>
                    )}
                  </div>
                </div>
              ))}
            </div>
          )}
        </CardContent>
      </Card>
    </div>
  );
}

'use client';

import React, { useState, useRef, useEffect } from 'react';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Badge } from '@/components/ui/badge';
import { getMedicationByBarcode, logTransaction, MedicationInventoryItem } from '@/lib/api/medicationInventory';
import { useToast } from '@/components/ui/use-toast';
import { Scan, Camera, QrCode, Pill, CheckCircle, AlertTriangle } from 'lucide-react';
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from '@/components/ui/select';
import { Label } from '@/components/ui/label';

const transactionTypes = [
  { value: 'administered', label: 'Administered' },
  { value: 'wasted', label: 'Wasted' },
  { value: 'returned', label: 'Returned' },
  { value: 'disposed', label: 'Disposed' },
  { value: 'count_adjustment', label: 'Count Adjustment' },
];

export function BarcodeScanner() {
  const [barcode, setBarcode] = useState('');
  const [medication, setMedication] = useState<MedicationInventoryItem | null>(null);
  const [loading, setLoading] = useState(false);
  const [transactionType, setTransactionType] = useState('administered');
  const [quantity, setQuantity] = useState('1');
  const [notes, setNotes] = useState('');
  const [scanning, setScanning] = useState(false);
  const videoRef = useRef<HTMLVideoElement>(null);
  const { toast } = useToast();

  // Note: Real barcode scanning would require a library like @zxing/browser
  // This is a simplified implementation

  const handleLookup = async () => {
    if (!barcode.trim()) {
      toast({
        title: 'Error',
        description: 'Please enter a barcode',
        variant: 'destructive',
      });
      return;
    }

    try {
      setLoading(true);
      const response = await getMedicationByBarcode(barcode);
      if (response.success) {
        setMedication(response.data);
        toast({
          title: 'Success',
          description: `Found: ${response.data.genericName}`,
        });
      }
    } catch (error) {
      toast({
        title: 'Not Found',
        description: 'No medication found with this barcode',
        variant: 'destructive',
      });
      setMedication(null);
    } finally {
      setLoading(false);
    }
  };

  const handleLogTransaction = async () => {
    if (!medication) return;

    try {
      setLoading(true);
      const response = await logTransaction(medication.id, {
        transaction_type: transactionType,
        quantity: parseInt(quantity),
        notes: notes || undefined,
        barcode_scanned: true,
        scanned_barcode: barcode,
      });

      if (response.success) {
        toast({
          title: 'Success',
          description: 'Transaction logged successfully',
        });
        // Reset form
        setMedication(null);
        setBarcode('');
        setQuantity('1');
        setNotes('');
      }
    } catch (error) {
      toast({
        title: 'Error',
        description: 'Failed to log transaction',
        variant: 'destructive',
      });
    } finally {
      setLoading(false);
    }
  };

  const startCamera = async () => {
    try {
      const stream = await navigator.mediaDevices.getUserMedia({ video: { facingMode: 'environment' } });
      if (videoRef.current) {
        videoRef.current.srcObject = stream;
        setScanning(true);
      }
    } catch (error) {
      toast({
        title: 'Camera Error',
        description: 'Unable to access camera. Please check permissions.',
        variant: 'destructive',
      });
    }
  };

  const stopCamera = () => {
    if (videoRef.current && videoRef.current.srcObject) {
      const stream = videoRef.current.srcObject as MediaStream;
      stream.getTracks().forEach(track => track.stop());
      videoRef.current.srcObject = null;
      setScanning(false);
    }
  };

  useEffect(() => {
    return () => {
      stopCamera();
    };
  }, []);

  return (
    <div className="space-y-6">
      <Card>
        <CardHeader>
          <CardTitle className="flex items-center gap-2">
            <Scan className="h-5 w-5" />
            Barcode Scanner
          </CardTitle>
        </CardHeader>
        <CardContent className="space-y-4">
          {/* Camera Preview */}
          {scanning ? (
            <div className="relative aspect-video bg-black rounded-lg overflow-hidden">
              <video
                ref={videoRef}
                autoPlay
                playsInline
                className="w-full h-full object-cover"
              />
              <div className="absolute inset-0 flex items-center justify-center">
                <div className="w-48 h-32 border-2 border-white rounded-lg opacity-50" />
              </div>
              <Button
                variant="destructive"
                size="sm"
                className="absolute bottom-4 left-1/2 transform -translate-x-1/2"
                onClick={stopCamera}
              >
                Stop Scanning
              </Button>
            </div>
          ) : (
            <div className="aspect-video bg-muted rounded-lg flex flex-col items-center justify-center">
              <QrCode className="h-16 w-16 text-muted-foreground mb-4" />
              <Button onClick={startCamera}>
                <Camera className="h-4 w-4 mr-2" />
                Start Camera
              </Button>
            </div>
          )}

          {/* Manual Entry */}
          <div className="space-y-2">
            <Label>Or enter barcode manually</Label>
            <div className="flex gap-2">
              <Input
                placeholder="Scan or type barcode..."
                value={barcode}
                onChange={(e) => setBarcode(e.target.value)}
                onKeyDown={(e) => e.key === 'Enter' && handleLookup()}
              />
              <Button onClick={handleLookup} disabled={loading}>
                {loading ? (
                  <div className="animate-spin rounded-full h-4 w-4 border-b-2 border-white" />
                ) : (
                  <Scan className="h-4 w-4" />
                )}
              </Button>
            </div>
          </div>
        </CardContent>
      </Card>

      {/* Medication Details */}
      {medication && (
        <Card>
          <CardHeader>
            <CardTitle className="flex items-center gap-2">
              <Pill className="h-5 w-5" />
              {medication.genericName}
            </CardTitle>
          </CardHeader>
          <CardContent className="space-y-4">
            <div className="grid grid-cols-2 gap-4">
              <div>
                <Label className="text-muted-foreground">Current Stock</Label>
                <p className="text-2xl font-bold">{medication.countCurrent}</p>
              </div>
              <div>
                <Label className="text-muted-foreground">Reorder Point</Label>
                <p className="text-2xl font-bold">{medication.reorderPoint}</p>
              </div>
            </div>

            <div className="space-y-2">
              <Label>Transaction Type</Label>
              <Select value={transactionType} onValueChange={setTransactionType}>
                <SelectTrigger>
                  <SelectValue />
                </SelectTrigger>
                <SelectContent>
                  {transactionTypes.map((type) => (
                    <SelectItem key={type.value} value={type.value}>
                      {type.label}
                    </SelectItem>
                  ))}
                </SelectContent>
              </Select>
            </div>

            <div className="space-y-2">
              <Label>Quantity</Label>
              <Input
                type="number"
                min={1}
                value={quantity}
                onChange={(e) => setQuantity(e.target.value)}
              />
            </div>

            <div className="space-y-2">
              <Label>Notes (Optional)</Label>
              <Input
                placeholder="Add any additional notes..."
                value={notes}
                onChange={(e) => setNotes(e.target.value)}
              />
            </div>

            <Button 
              className="w-full" 
              onClick={handleLogTransaction}
              disabled={loading}
            >
              {loading ? (
                <div className="animate-spin rounded-full h-4 w-4 border-b-2 border-white mr-2" />
              ) : (
                <CheckCircle className="h-4 w-4 mr-2" />
              )}
              Log Transaction
            </Button>
          </CardContent>
        </Card>
      )}
    </div>
  );
}

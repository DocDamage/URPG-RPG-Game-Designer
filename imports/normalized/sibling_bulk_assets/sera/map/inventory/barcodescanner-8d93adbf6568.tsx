"use client";

import React, { useState, useRef, useCallback } from 'react';
import { Camera, Scan, X, CheckCircle2, AlertCircle } from 'lucide-react';
import { Button } from '@/components/ui/button';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Alert, AlertDescription } from '@/components/ui/alert';
import { Input } from '@/components/ui/input';
import { Badge } from '@/components/ui/badge';
import medicationInventoryApi, { BarcodeScanResult } from '@/lib/api/medicationInventory';

interface BarcodeScannerProps {
  onScan?: (result: BarcodeScanResult) => void;
  onVerify?: (medicationId: string) => void;
  verifyMode?: boolean;
  expectedMedicationId?: string;
  className?: string;
}

export function BarcodeScanner({
  onScan,
  onVerify,
  verifyMode = false,
  expectedMedicationId,
  className,
}: BarcodeScannerProps) {
  const [scanning, setScanning] = useState(false);
  const [manualEntry, setManualEntry] = useState(false);
  const [barcodeInput, setBarcodeInput] = useState('');
  const [result, setResult] = useState<BarcodeScanResult | null>(null);
  const [verificationResult, setVerificationResult] = useState<{ verified: boolean; message: string } | null>(null);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const videoRef = useRef<HTMLVideoElement>(null);

  // Manual barcode entry
  const handleManualScan = async () => {
    if (!barcodeInput.trim()) return;
    setLoading(true);
    setError(null);
    setResult(null);
    setVerificationResult(null);

    try {
      const response = await medicationInventoryApi.scanBarcode(barcodeInput.trim());
      if (response.data) {
        setResult(response.data);
        onScan?.(response.data);

        // If in verify mode, also verify against expected medication
        if (verifyMode && expectedMedicationId) {
          const verifyResponse = await medicationInventoryApi.verifyBarcode(
            barcodeInput.trim(),
            expectedMedicationId
          );
          if (verifyResponse.data) {
            setVerificationResult(verifyResponse.data);
            if (verifyResponse.data.verified) {
              onVerify?.(expectedMedicationId);
            }
          }
        }
      }
    } catch (err) {
      setError('Failed to scan barcode. Please try again.');
    } finally {
      setLoading(false);
    }
  };

  // Simulate camera scanning (in production, would use a barcode scanning library)
  const startCamera = useCallback(() => {
    setScanning(true);
    setError(null);
    // In production, this would initialize the camera and barcode scanner
    // For now, we'll just switch to manual entry mode after a simulated delay
    setTimeout(() => {
      setScanning(false);
      setManualEntry(true);
    }, 1000);
  }, []);

  const stopScanning = useCallback(() => {
    setScanning(false);
    setManualEntry(false);
    setResult(null);
    setVerificationResult(null);
    setBarcodeInput('');
  }, []);

  const getTypeBadgeColor = (type: string) => {
    switch (type) {
      case 'ndc':
        return 'bg-blue-100 text-blue-800';
      case 'qr_internal':
        return 'bg-green-100 text-green-800';
      case 'gtin':
        return 'bg-purple-100 text-purple-800';
      default:
        return 'bg-gray-100 text-gray-800';
    }
  };

  return (
    <Card className={className}>
      <CardHeader>
        <CardTitle className="flex items-center gap-2">
          <Scan className="h-5 w-5" />
          {verifyMode ? 'Verify Medication' : 'Scan Barcode'}
        </CardTitle>
      </CardHeader>
      <CardContent className="space-y-4">
        {error && (
          <Alert variant="destructive">
            <AlertCircle className="h-4 w-4" />
            <AlertDescription>{error}</AlertDescription>
          </Alert>
        )}

        {/* Camera View or Placeholder */}
        {!manualEntry && !result && (
          <div className="relative aspect-video bg-gray-900 rounded-lg overflow-hidden">
            {scanning ? (
              <>
                <video
                  ref={videoRef}
                  className="w-full h-full object-cover"
                  autoPlay
                  playsInline
                />
                <div className="absolute inset-0 flex items-center justify-center">
                  <div className="w-48 h-32 border-2 border-white/50 rounded-lg">
                    <div className="w-full h-0.5 bg-red-500/50 animate-pulse absolute top-1/2" />
                  </div>
                </div>
                <Button
                  variant="secondary"
                  size="sm"
                  className="absolute top-2 right-2"
                  onClick={stopScanning}
                >
                  <X className="h-4 w-4" />
                </Button>
              </>
            ) : (
              <div className="w-full h-full flex flex-col items-center justify-center text-white">
                <Camera className="h-12 w-12 mb-4 opacity-50" />
                <p className="text-sm opacity-70 mb-4">Camera scanning coming soon</p>
                <div className="flex gap-2">
                  <Button variant="secondary" onClick={startCamera} disabled={scanning}>
                    <Camera className="mr-2 h-4 w-4" />
                    Start Camera
                  </Button>
                  <Button variant="outline" onClick={() => setManualEntry(true)}>
                    Enter Manually
                  </Button>
                </div>
              </div>
            )}
          </div>
        )}

        {/* Manual Entry */}
        {manualEntry && !result && (
          <div className="space-y-4">
            <div className="flex gap-2">
              <Input
                placeholder="Enter barcode or NDC number..."
                value={barcodeInput}
                onChange={(e) => setBarcodeInput(e.target.value)}
                onKeyDown={(e) => e.key === 'Enter' && handleManualScan()}
                className="flex-1"
              />
              <Button onClick={handleManualScan} disabled={loading || !barcodeInput.trim()}>
                {loading ? 'Scanning...' : 'Scan'}
              </Button>
            </div>
            <Button variant="ghost" size="sm" onClick={() => setManualEntry(false)}>
              Back to Camera
            </Button>
          </div>
        )}

        {/* Scan Result */}
        {result && (
          <div className="space-y-4">
            <div className="flex items-center justify-between">
              <Badge className={getTypeBadgeColor(result.type)}>
                {result.type.toUpperCase()}
              </Badge>
              <span className="text-sm text-muted-foreground">
                Confidence: {Math.round(result.confidence * 100)}%
              </span>
            </div>

            <div className="bg-muted rounded-lg p-4 space-y-2">
              <p className="text-sm font-medium">Raw Value:</p>
              <code className="text-xs bg-background p-2 rounded block break-all">
                {result.rawValue}
              </code>
            </div>

            {Object.keys(result.parsedData).length > 0 && (
              <div className="space-y-2">
                <p className="text-sm font-medium">Parsed Data:</p>
                <div className="bg-muted rounded-lg p-3 space-y-1">
                  {result.parsedData.ndc && (
                    <div className="flex justify-between text-sm">
                      <span className="text-muted-foreground">NDC:</span>
                      <span>{result.parsedData.ndc}</span>
                    </div>
                  )}
                  {result.parsedData.lotNumber && (
                    <div className="flex justify-between text-sm">
                      <span className="text-muted-foreground">Lot:</span>
                      <span>{result.parsedData.lotNumber}</span>
                    </div>
                  )}
                  {result.parsedData.expirationDate && (
                    <div className="flex justify-between text-sm">
                      <span className="text-muted-foreground">Expires:</span>
                      <span>{result.parsedData.expirationDate}</span>
                    </div>
                  )}
                  {result.parsedData.serialNumber && (
                    <div className="flex justify-between text-sm">
                      <span className="text-muted-foreground">Serial:</span>
                      <span>{result.parsedData.serialNumber}</span>
                    </div>
                  )}
                </div>
              </div>
            )}

            {result.medicationInfo && (
              <div className="bg-primary/5 rounded-lg p-4 space-y-2">
                <p className="text-sm font-medium text-primary">Medication Info:</p>
                {result.medicationInfo.genericName && (
                  <p className="font-medium">{result.medicationInfo.genericName}</p>
                )}
                {result.medicationInfo.brandName && (
                  <p className="text-sm text-muted-foreground">{result.medicationInfo.brandName}</p>
                )}
                {result.medicationInfo.strength && (
                  <p className="text-sm">{result.medicationInfo.strength}</p>
                )}
              </div>
            )}

            {/* Verification Result */}
            {verificationResult && (
              <Alert variant={verificationResult.verified ? 'default' : 'destructive'} className={verificationResult.verified ? 'bg-green-50 border-green-200' : undefined}>
                {verificationResult.verified ? (
                  <CheckCircle2 className="h-4 w-4 text-green-600" />
                ) : (
                  <AlertCircle className="h-4 w-4" />
                )}
                <AlertDescription className={verificationResult.verified ? 'text-green-800' : undefined}>
                  {verificationResult.message}
                </AlertDescription>
              </Alert>
            )}

            <Button variant="outline" className="w-full" onClick={stopScanning}>
              Scan Another
            </Button>
          </div>
        )}
      </CardContent>
    </Card>
  );
}

export default BarcodeScanner;

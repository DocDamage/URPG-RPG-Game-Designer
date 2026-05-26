"use client";

import React, { useState, useEffect } from 'react';
import { Package, AlertTriangle, Thermometer, Plus, Search, RotateCcw, ClipboardList } from 'lucide-react';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Tabs, TabsContent, TabsList, TabsTrigger } from '@/components/ui/tabs';
import { Alert, AlertDescription } from '@/components/ui/alert';
import { Badge } from '@/components/ui/badge';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { BarcodeScanner } from '@/components/inventory/BarcodeScanner';
import medicationInventoryApi, { MedicationInventoryItem, RecallInfo, ReorderSuggestion } from '@/lib/api/medicationInventory';

export default function MedicationInventoryPage() {
  const [activeTab, setActiveTab] = useState('inventory');
  const [inventory, setInventory] = useState<MedicationInventoryItem[]>([]);
  const [expiringMeds, setExpiringMeds] = useState<MedicationInventoryItem[]>([]);
  const [recalls, setRecalls] = useState<RecallInfo[]>([]);
  const [reorders, setReorders] = useState<ReorderSuggestion[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [searchQuery, setSearchQuery] = useState('');

  const fetchData = async () => {
    setLoading(true);
    setError(null);
    try {
      const [inventoryRes, expiringRes, recallsRes, reordersRes] = await Promise.all([
        medicationInventoryApi.getInventory(),
        medicationInventoryApi.getExpiringMedications(30),
        medicationInventoryApi.getRecalls(),
        medicationInventoryApi.getReorderSuggestions(),
      ]);

      if (inventoryRes.data) setInventory(inventoryRes.data);
      if (expiringRes.data) setExpiringMeds(expiringRes.data);
      if (recallsRes.data) setRecalls(recallsRes.data);
      if (reordersRes.data) setReorders(reordersRes.data);
    } catch (err) {
      console.error('Failed to fetch inventory data:', err);
      setError('Failed to load inventory data. Please try again.');
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchData();
  }, []);

  const filteredInventory = inventory.filter((med) =>
    med.genericName.toLowerCase().includes(searchQuery.toLowerCase()) ||
    med.brandName?.toLowerCase().includes(searchQuery.toLowerCase()) ||
    med.ndcNumber?.includes(searchQuery)
  );

  const getStockStatus = (med: MedicationInventoryItem) => {
    if (med.countCurrent <= 0) return { label: 'Out of Stock', color: 'bg-red-100 text-red-800' };
    if (med.countCurrent <= med.reorderPoint) return { label: 'Low Stock', color: 'bg-yellow-100 text-yellow-800' };
    return { label: 'In Stock', color: 'bg-green-100 text-green-800' };
  };

  const getDaysUntilExpiration = (expirationDate: string) => {
    const days = Math.ceil((new Date(expirationDate).getTime() - Date.now()) / (1000 * 60 * 60 * 24));
    return days;
  };

  return (
    <div className="container mx-auto py-6 space-y-6">
      {/* Header */}
      <div className="flex flex-col sm:flex-row sm:items-center sm:justify-between gap-4">
        <div>
          <h1 className="text-3xl font-bold tracking-tight">Medication Inventory</h1>
          <p className="text-muted-foreground">
            Manage medication stock, track expirations, and monitor recalls
          </p>
        </div>
        <div className="flex items-center gap-2">
          <Button>
            <Plus className="mr-2 h-4 w-4" />
            Add Medication
          </Button>
        </div>
      </div>

      {/* Error Alert */}
      {error && (
        <Alert variant="destructive">
          <AlertTriangle className="h-4 w-4" />
          <AlertDescription>{error}</AlertDescription>
        </Alert>
      )}

      {/* Stats Cards */}
      <div className="grid grid-cols-2 md:grid-cols-4 gap-4">
        <Card>
          <CardContent className="p-4">
            <div className="flex items-center gap-3">
              <div className="p-2 bg-blue-100 rounded-lg">
                <Package className="h-5 w-5 text-blue-600" />
              </div>
              <div>
                <p className="text-sm text-muted-foreground">Total Items</p>
                <p className="text-2xl font-bold">{inventory.length}</p>
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
                <p className="text-sm text-muted-foreground">Expiring Soon</p>
                <p className="text-2xl font-bold">{expiringMeds.length}</p>
              </div>
            </div>
          </CardContent>
        </Card>

        <Card>
          <CardContent className="p-4">
            <div className="flex items-center gap-3">
              <div className="p-2 bg-red-100 rounded-lg">
                <RotateCcw className="h-5 w-5 text-red-600" />
              </div>
              <div>
                <p className="text-sm text-muted-foreground">Active Recalls</p>
                <p className="text-2xl font-bold">{recalls.filter(r => r.status === 'active').length}</p>
              </div>
            </div>
          </CardContent>
        </Card>

        <Card>
          <CardContent className="p-4">
            <div className="flex items-center gap-3">
              <div className="p-2 bg-purple-100 rounded-lg">
                <ClipboardList className="h-5 w-5 text-purple-600" />
              </div>
              <div>
                <p className="text-sm text-muted-foreground">Need Reorder</p>
                <p className="text-2xl font-bold">{reorders.length}</p>
              </div>
            </div>
          </CardContent>
        </Card>
      </div>

      {/* Main Content */}
      <Tabs value={activeTab} onValueChange={setActiveTab}>
        <TabsList className="grid w-full grid-cols-4 lg:w-[500px]">
          <TabsTrigger value="inventory">Inventory</TabsTrigger>
          <TabsTrigger value="scanner">Scanner</TabsTrigger>
          <TabsTrigger value="expiring">Expiring</TabsTrigger>
          <TabsTrigger value="recalls">Recalls</TabsTrigger>
        </TabsList>

        {/* Inventory Tab */}
        <TabsContent value="inventory" className="mt-6">
          <div className="flex items-center gap-2 mb-4">
            <Search className="h-4 w-4 text-muted-foreground" />
            <Input
              placeholder="Search medications..."
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
              className="max-w-sm"
            />
          </div>

          {loading ? (
            <div className="text-center py-8 text-muted-foreground">Loading...</div>
          ) : filteredInventory.length === 0 ? (
            <div className="text-center py-12 bg-muted/50 rounded-lg">
              <Package className="mx-auto h-12 w-12 text-muted-foreground" />
              <h3 className="mt-4 text-lg font-medium">No Medications Found</h3>
              <p className="text-muted-foreground">Add medications to your inventory</p>
            </div>
          ) : (
            <div className="border rounded-lg divide-y">
              {filteredInventory.map((med) => {
                const status = getStockStatus(med);
                return (
                  <div key={med.id} className="flex items-center justify-between p-4 hover:bg-muted/50">
                    <div className="flex-1">
                      <div className="flex items-center gap-2">
                        <p className="font-medium">{med.genericName}</p>
                        <Badge variant="outline" className={status.color}>
                          {status.label}
                        </Badge>
                      </div>
                      <p className="text-sm text-muted-foreground">
                        {med.brandName} • {med.strength} • {med.form}
                      </p>
                      <div className="flex items-center gap-4 mt-1 text-sm text-muted-foreground">
                        <span>Stock: {med.countCurrent} units</span>
                        <span>Exp: {new Date(med.expirationDate).toLocaleDateString()}</span>
                        {med.storageLocation && <span>Loc: {med.storageLocation}</span>}
                      </div>
                    </div>
                    <Button variant="ghost" size="sm">
                      View
                    </Button>
                  </div>
                );
              })}
            </div>
          )}
        </TabsContent>

        {/* Scanner Tab */}
        <TabsContent value="scanner" className="mt-6">
          <div className="max-w-md mx-auto">
            <BarcodeScanner onScan={(result) => console.log('Scanned:', result)} />
          </div>
        </TabsContent>

        {/* Expiring Tab */}
        <TabsContent value="expiring" className="mt-6">
          {expiringMeds.length === 0 ? (
            <div className="text-center py-12 bg-muted/50 rounded-lg">
              <AlertTriangle className="mx-auto h-12 w-12 text-muted-foreground" />
              <h3 className="mt-4 text-lg font-medium">No Expiring Medications</h3>
              <p className="text-muted-foreground">All medications are good for the next 30 days</p>
            </div>
          ) : (
            <div className="space-y-3">
              {expiringMeds.map((med) => {
                const daysUntil = getDaysUntilExpiration(med.expirationDate);
                return (
                  <Alert key={med.id} variant={daysUntil <= 7 ? 'destructive' : 'default'} className={daysUntil > 7 ? 'bg-yellow-50 border-yellow-200' : undefined}>
                    <AlertTriangle className={`h-4 w-4 ${daysUntil > 7 ? 'text-yellow-600' : ''}`} />
                    <AlertDescription className="flex items-center justify-between">
                      <div>
                        <p className="font-medium">{med.genericName}</p>
                        <p className="text-sm">{med.strength} • {med.form}</p>
                      </div>
                      <Badge variant={daysUntil <= 7 ? 'destructive' : 'outline'} className={daysUntil > 7 ? 'border-yellow-400 text-yellow-700' : undefined}>
                        {daysUntil <= 0 ? 'Expired' : `${daysUntil} days left`}
                      </Badge>
                    </AlertDescription>
                  </Alert>
                );
              })}
            </div>
          )}
        </TabsContent>

        {/* Recalls Tab */}
        <TabsContent value="recalls" className="mt-6">
          {recalls.length === 0 ? (
            <div className="text-center py-12 bg-muted/50 rounded-lg">
              <RotateCcw className="mx-auto h-12 w-12 text-muted-foreground" />
              <h3 className="mt-4 text-lg font-medium">No Active Recalls</h3>
              <p className="text-muted-foreground">No medication recalls at this time</p>
            </div>
          ) : (
            <div className="space-y-4">
              {recalls.map((recall) => (
                <Card key={recall.id} className={recall.status === 'active' ? 'border-red-200' : undefined}>
                  <CardHeader>
                    <div className="flex items-center justify-between">
                      <CardTitle className="text-lg">Recall #{recall.recallNumber}</CardTitle>
                      <Badge variant={recall.status === 'active' ? 'destructive' : 'outline'}>
                        {recall.status}
                      </Badge>
                    </div>
                  </CardHeader>
                  <CardContent className="space-y-2">
                    <p><span className="font-medium">Reason:</span> {recall.recallReason}</p>
                    {recall.recallClass && (
                      <p><span className="font-medium">Class:</span> {recall.recallClass}</p>
                    )}
                    {recall.lotNumbers && recall.lotNumbers.length > 0 && (
                      <p><span className="font-medium">Affected Lots:</span> {recall.lotNumbers.join(', ')}</p>
                    )}
                    <p className="text-sm text-muted-foreground">
                      Initiated: {new Date(recall.initiatedAt).toLocaleDateString()}
                    </p>
                  </CardContent>
                </Card>
              ))}
            </div>
          )}
        </TabsContent>
      </Tabs>
    </div>
  );
}

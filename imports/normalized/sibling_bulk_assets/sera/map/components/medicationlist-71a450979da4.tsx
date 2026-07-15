'use client';

import React, { useState, useEffect } from 'react';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Badge } from '@/components/ui/badge';
import { 
  getMedicationInventory, 
  getExpiringMedications,
  MedicationInventoryItem 
} from '@/lib/api/medicationInventory';
import { useToast } from '@/components/ui/use-toast';
import { 
  Pill, 
  Search, 
  AlertTriangle, 
  Package, 
  Thermometer,
  Calendar,
  Barcode
} from 'lucide-react';
import {
  Dialog,
  DialogContent,
  DialogDescription,
  DialogHeader,
  DialogTitle,
  DialogTrigger,
} from '@/components/ui/dialog';

export function MedicationList() {
  const [medications, setMedications] = useState<MedicationInventoryItem[]>([]);
  const [expiringMeds, setExpiringMeds] = useState<MedicationInventoryItem[]>([]);
  const [searchQuery, setSearchQuery] = useState('');
  const [loading, setLoading] = useState(true);
  const [selectedMed, setSelectedMed] = useState<MedicationInventoryItem | null>(null);
  const { toast } = useToast();

  useEffect(() => {
    loadData();
  }, []);

  const loadData = async () => {
    try {
      setLoading(true);
      const [inventoryResponse, expiringResponse] = await Promise.all([
        getMedicationInventory({ activeOnly: true }),
        getExpiringMedications(30),
      ]);

      if (inventoryResponse.success) {
        setMedications(inventoryResponse.data);
      }

      if (expiringResponse.success) {
        setExpiringMeds(expiringResponse.data);
      }
    } catch (error) {
      toast({
        title: 'Error',
        description: 'Failed to load medications',
        variant: 'destructive',
      });
    } finally {
      setLoading(false);
    }
  };

  const filteredMedications = medications.filter((med) =>
    med.genericName.toLowerCase().includes(searchQuery.toLowerCase()) ||
    med.brandName?.toLowerCase().includes(searchQuery.toLowerCase()) ||
    med.ndcNumber?.includes(searchQuery)
  );

  const getStockStatus = (med: MedicationInventoryItem) => {
    if (med.countCurrent <= med.reorderPoint) {
      return { label: 'Low Stock', color: 'bg-red-100 text-red-800' };
    }
    if (med.countCurrent <= med.reorderPoint * 1.5) {
      return { label: 'Medium', color: 'bg-yellow-100 text-yellow-800' };
    }
    return { label: 'Good', color: 'bg-green-100 text-green-800' };
  };

  const getExpirationStatus = (expirationDate: string) => {
    const expDate = new Date(expirationDate);
    const today = new Date();
    const daysUntilExp = Math.floor((expDate.getTime() - today.getTime()) / (1000 * 60 * 60 * 24));

    if (daysUntilExp < 0) {
      return { label: 'Expired', color: 'bg-red-600 text-white' };
    }
    if (daysUntilExp <= 30) {
      return { label: `${daysUntilExp} days`, color: 'bg-red-100 text-red-800' };
    }
    if (daysUntilExp <= 90) {
      return { label: `${Math.floor(daysUntilExp / 30)} months`, color: 'bg-yellow-100 text-yellow-800' };
    }
    return { label: 'Good', color: 'bg-green-100 text-green-800' };
  };

  if (loading) {
    return (
      <Card>
        <CardContent className="p-6">
          <div className="flex items-center justify-center h-64">
            <div className="animate-spin rounded-full h-8 w-8 border-b-2 border-primary"></div>
          </div>
        </CardContent>
      </Card>
    );
  }

  return (
    <div className="space-y-6">
      {/* Alerts */}
      {(expiringMeds.length > 0 || medications.some(m => m.countCurrent <= m.reorderPoint)) && (
        <Card className="border-red-200 bg-red-50">
          <CardHeader>
            <CardTitle className="flex items-center gap-2 text-red-800">
              <AlertTriangle className="h-5 w-5" />
              Alerts
            </CardTitle>
          </CardHeader>
          <CardContent>
            <div className="space-y-2">
              {expiringMeds.length > 0 && (
                <div className="flex items-center gap-2 text-red-700">
                  <Calendar className="h-4 w-4" />
                  <span>{expiringMeds.length} medications expiring soon</span>
                </div>
              )}
              {medications.filter(m => m.countCurrent <= m.reorderPoint).length > 0 && (
                <div className="flex items-center gap-2 text-red-700">
                  <Package className="h-4 w-4" />
                  <span>{medications.filter(m => m.countCurrent <= m.reorderPoint).length} medications below reorder point</span>
                </div>
              )}
            </div>
          </CardContent>
        </Card>
      )}

      {/* Search */}
      <div className="relative">
        <Search className="absolute left-3 top-3 h-4 w-4 text-muted-foreground" />
        <Input
          placeholder="Search medications by name or NDC..."
          value={searchQuery}
          onChange={(e) => setSearchQuery(e.target.value)}
          className="pl-10"
        />
      </div>

      {/* Medication List */}
      <Card>
        <CardHeader className="flex flex-row items-center justify-between">
          <CardTitle className="flex items-center gap-2">
            <Pill className="h-5 w-5" />
            Medication Inventory
          </CardTitle>
          <Badge variant="secondary">{filteredMedications.length} items</Badge>
        </CardHeader>
        <CardContent>
          {filteredMedications.length === 0 ? (
            <div className="text-center py-8 text-muted-foreground">
              <Package className="h-12 w-12 mx-auto mb-3 opacity-50" />
              <p>No medications found</p>
            </div>
          ) : (
            <div className="space-y-3">
              {filteredMedications.map((med) => {
                const stockStatus = getStockStatus(med);
                const expStatus = getExpirationStatus(med.expirationDate);

                return (
                  <Dialog key={med.id}>
                    <DialogTrigger asChild>
                      <div
                        className="flex items-center justify-between p-4 border rounded-lg cursor-pointer hover:bg-muted transition-colors"
                        onClick={() => setSelectedMed(med)}
                      >
                        <div className="flex-1">
                          <div className="flex items-center gap-2 mb-1">
                            <h4 className="font-medium">{med.genericName}</h4>
                            {med.brandName && (
                              <span className="text-sm text-muted-foreground">
                                ({med.brandName})
                              </span>
                            )}
                          </div>
                          <div className="flex items-center gap-4 text-sm text-muted-foreground">
                            <span>{med.strength} {med.form}</span>
                            {med.ndcNumber && (
                              <span className="flex items-center gap-1">
                                <Barcode className="h-3 w-3" />
                                {med.ndcNumber}
                              </span>
                            )}
                            {med.storageConditions !== 'room temp' && (
                              <span className="flex items-center gap-1">
                                <Thermometer className="h-3 w-3" />
                                {med.storageConditions}
                              </span>
                            )}
                          </div>
                        </div>
                        <div className="flex items-center gap-2">
                          <Badge className={stockStatus.color}>{stockStatus.label}</Badge>
                          <Badge className={expStatus.color}>{expStatus.label}</Badge>
                          <span className="font-bold ml-2">{med.countCurrent}</span>
                        </div>
                      </div>
                    </DialogTrigger>
                    <DialogContent>
                      <DialogHeader>
                        <DialogTitle>{med.genericName}</DialogTitle>
                        <DialogDescription>
                          {med.brandName && `Brand: ${med.brandName}`}
                        </DialogDescription>
                      </DialogHeader>
                      <div className="space-y-4 py-4">
                        <div className="grid grid-cols-2 gap-4">
                          <div>
                            <Label>Strength</Label>
                            <p className="font-medium">{med.strength}</p>
                          </div>
                          <div>
                            <Label>Form</Label>
                            <p className="font-medium">{med.form}</p>
                          </div>
                          <div>
                            <Label>Route</Label>
                            <p className="font-medium">{med.route}</p>
                          </div>
                          <div>
                            <Label>Current Count</Label>
                            <p className="font-medium text-lg">{med.countCurrent}</p>
                          </div>
                          <div>
                            <Label>Reorder Point</Label>
                            <p className="font-medium">{med.reorderPoint}</p>
                          </div>
                          <div>
                            <Label>Expiration</Label>
                            <p className="font-medium">
                              {new Date(med.expirationDate).toLocaleDateString()}
                            </p>
                          </div>
                        </div>
                        {med.lotNumber && (
                          <div>
                            <Label>Lot Number</Label>
                            <p className="font-medium">{med.lotNumber}</p>
                          </div>
                        )}
                        {med.storageLocation && (
                          <div>
                            <Label>Storage Location</Label>
                            <p className="font-medium">{med.storageLocation}</p>
                          </div>
                        )}
                        <div className="flex gap-2 pt-4">
                          <Button className="flex-1">Log Transaction</Button>
                          <Button variant="outline" className="flex-1">Physical Count</Button>
                        </div>
                      </div>
                    </DialogContent>
                  </Dialog>
                );
              })}
            </div>
          )}
        </CardContent>
      </Card>
    </div>
  );
}

function Label({ children }: { children: React.ReactNode }) {
  return <p className="text-sm text-muted-foreground mb-1">{children}</p>;
}

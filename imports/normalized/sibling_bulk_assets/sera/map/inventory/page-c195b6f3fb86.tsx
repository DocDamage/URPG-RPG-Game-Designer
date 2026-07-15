'use client';

import React, { useState, useEffect } from 'react';
import { Card } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Badge } from '@/components/ui/badge';
import { Alert } from '@/components/ui/alert';
import Link from 'next/link';
import { MediaCapture } from '@/app/incidents/components/MediaCapture';
import { apiClient } from '@/lib/api/client';

interface InventoryItem {
    id: string;
    name: string;
    category: string;
    quantity: number;
    minQuantity: number;
    unit: string;
    location?: string;
    supplier?: string;
    photoUrl?: string;
    photoId?: number;
    status: 'low' | 'adequate' | 'good';
}

interface OrderItem {
    inventoryId?: string;
    itemName: string;
    quantity: number;
    unit: string;
    unitPrice?: number;
    totalPrice?: number;
}

/**
 * Inventory Management Page
 * 
 * Track supplies, capture photos, and order items
 */
export default function InventoryPage() {
    const [searchTerm, setSearchTerm] = useState('');
    const [categoryFilter, setCategoryFilter] = useState<string>('');
    const [inventoryItems, setInventoryItems] = useState<InventoryItem[]>([]);
    const [loading, setLoading] = useState(true);
    const [showAddItem, setShowAddItem] = useState(false);
    const [showOrderModal, setShowOrderModal] = useState(false);
    const [selectedItem, setSelectedItem] = useState<InventoryItem | null>(null);
    const [orderItems, setOrderItems] = useState<OrderItem[]>([]);
    const [homeId] = useState('home_1'); // TODO: Get from context

    useEffect(() => {
        loadInventory();
    }, [homeId, categoryFilter]);

    const loadInventory = async () => {
        try {
            setLoading(true);
            const items = await apiClient.get<InventoryItem[]>(`/inventory/${homeId}${categoryFilter ? `?category=${categoryFilter}` : ''}`);
            
            // Calculate status for each item
            const itemsWithStatus = items.map(item => ({
                ...item,
                status: item.quantity <= item.minQuantity ? 'low' as const :
                       item.quantity <= item.minQuantity * 2 ? 'adequate' as const : 'good' as const
            }));
            
            setInventoryItems(itemsWithStatus);
        } catch (error) {
            console.error('Failed to load inventory:', error);
        } finally {
            setLoading(false);
        }
    };

    const handleAddPhoto = async (itemId: string, files: File[], enableAI: boolean = false) => {
        if (files.length === 0) return;

        try {
            const file = files[0];
            const reader = new FileReader();
            
            reader.onloadend = async () => {
                const base64 = reader.result as string;
                const base64Data = base64.split(',')[1]; // Remove data:image/jpeg;base64, prefix

                // Try AI recognition if enabled and itemId is new
                if (enableAI && (!itemId || itemId.startsWith('new'))) {
                    try {
                        const recognition = await apiClient.post('/inventory/ai/recognize', {
                            photo: {
                                data: base64Data,
                                mimeType: file.type
                            }
                        });
                        
                        if (recognition.confidence > 0.5) {
                            // Auto-fill form with recognized item
                            alert(`AI recognized: ${recognition.itemName} (${recognition.category}) - Confidence: ${(recognition.confidence * 100).toFixed(0)}%`);
                            // Could auto-populate form here
                        }
                    } catch (aiError) {
                        console.log('AI recognition failed, continuing with manual upload');
                    }
                }

                if (itemId && !itemId.startsWith('new')) {
                    await apiClient.post(`/inventory/${homeId}/items/${itemId}/photo`, {
                        photo: {
                            data: base64Data,
                            mimeType: file.type
                        },
                        isPrimary: true
                    });
                }

                loadInventory(); // Reload to show new photo
            };

            reader.readAsDataURL(file);
        } catch (error) {
            console.error('Failed to upload photo:', error);
            alert('Failed to upload photo');
        }
    };

    const handleCreateOrder = async () => {
        if (orderItems.length === 0) {
            alert('Please add items to order');
            return;
        }

        try {
            await apiClient.post('/orders', {
                homeId,
                items: orderItems,
                notes: 'Order created from inventory page'
            });

            alert('Order created successfully! Awaiting supervisor approval.');
            setShowOrderModal(false);
            setOrderItems([]);
            loadInventory();
        } catch (error: any) {
            console.error('Failed to create order:', error);
            alert(error.message || 'Failed to create order');
        }
    };

    const handleAddToOrder = (item: InventoryItem) => {
        const orderQuantity = (item.minQuantity * 3) - item.quantity; // Order enough to restock
        
        setOrderItems(prev => {
            const existing = prev.findIndex(oi => oi.inventoryId === item.id);
            if (existing >= 0) {
                const updated = [...prev];
                updated[existing].quantity += orderQuantity;
                return updated;
            }
            return [...prev, {
                inventoryId: item.id,
                itemName: item.name,
                quantity: orderQuantity,
                unit: item.unit
            }];
        });
    };

    const handleAutoOrder = async () => {
        try {
            const result = await apiClient.post(`/orders/auto/${homeId}`, {
                supplier: 'default'
            });
            alert('Auto-order created from low stock items!');
            loadInventory();
        } catch (error: any) {
            console.error('Failed to create auto order:', error);
            alert(error.message || 'Failed to create auto order');
        }
    };

    const filteredItems = inventoryItems.filter((item) =>
        item.name.toLowerCase().includes(searchTerm.toLowerCase()) ||
        item.category.toLowerCase().includes(searchTerm.toLowerCase())
    );

    const lowStockItems = inventoryItems.filter(item => item.status === 'low');

    return (
        <main className="container mx-auto p-6 bg-gradient-to-b from-gray-50 to-white min-h-screen">
            <div className="max-w-7xl mx-auto">
                {/* Header */}
                <div className="flex items-center justify-between mb-6">
                    <div>
                        <h1 className="text-3xl font-bold text-gray-800">Inventory Management</h1>
                        <p className="text-gray-600 mt-1">Track supplies, capture photos, and order items</p>
                    </div>
                    <Link href="/operations">
                        <Button variant="outline">Back to Operations</Button>
                    </Link>
                </div>

                {/* Low Stock Alert */}
                {lowStockItems.length > 0 && (
                    <Alert variant="warning" className="mb-6">
                        <div className="flex items-center justify-between">
                            <div>
                                <strong>{lowStockItems.length} item(s) need reordering</strong>
                                <p className="text-sm mt-1">Click "Auto-Order Low Stock" to create an order automatically</p>
                            </div>
                            <Button onClick={handleAutoOrder} size="sm">
                                Auto-Order Low Stock
                            </Button>
                        </div>
                    </Alert>
                )}

                {/* Search and Actions */}
                <Card className="p-4 mb-6">
                    <div className="flex flex-wrap gap-4 items-end">
                        <div className="flex-1 min-w-[200px]">
                            <Input
                                placeholder="Search inventory..."
                                value={searchTerm}
                                onChange={(e) => setSearchTerm(e.target.value)}
                            />
                        </div>
                        <select
                            value={categoryFilter}
                            onChange={(e) => setCategoryFilter(e.target.value)}
                            className="px-3 py-2 border rounded-md bg-gray-100 dark:bg-gray-800 border-gray-300 dark:border-gray-700"
                        >
                            <option value="">All Categories</option>
                            <option value="groceries">Groceries</option>
                            <option value="medical_supplies">Medical Supplies</option>
                            <option value="household">Household</option>
                            <option value="office">Office</option>
                            <option value="equipment">Equipment</option>
                        </select>
                        <Button onClick={() => setShowAddItem(true)}>Add Item</Button>
                        <Button variant="outline" onClick={() => setShowOrderModal(true)}>
                            Create Order ({orderItems.length})
                        </Button>
                    </div>
                </Card>

                {/* Inventory Items */}
                {loading ? (
                    <div className="text-center py-12">Loading inventory...</div>
                ) : (
                    <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
                        {filteredItems.map((item) => (
                            <Card key={item.id} className="p-6">
                                <div className="flex items-start justify-between mb-4">
                                    <div className="flex-1">
                                        <h3 className="text-lg font-semibold text-gray-800 mb-1">
                                            {item.name}
                                        </h3>
                                        <p className="text-sm text-gray-600">{item.category}</p>
                                    </div>
                                    <Badge
                                        variant={
                                            item.status === 'low'
                                                ? 'destructive'
                                                : item.status === 'adequate'
                                                ? 'default'
                                                : 'secondary'
                                        }
                                    >
                                        {item.status}
                                    </Badge>
                                </div>

                                {/* Photo Display */}
                                {item.photoUrl ? (
                                    <div className="mb-4">
                                        <img
                                            src={item.photoUrl}
                                            alt={item.name}
                                            className="w-full h-32 object-cover rounded border"
                                        />
                                    </div>
                                ) : (
                                    <div className="mb-4 h-32 bg-gray-100 rounded border flex items-center justify-center text-gray-400 text-sm">
                                        No photo
                                    </div>
                                )}

                                <div className="space-y-2 text-sm mb-4">
                                    <div className="flex justify-between">
                                        <span className="text-gray-600">Quantity:</span>
                                        <span className="font-semibold text-gray-800">
                                            {item.quantity} {item.unit}
                                        </span>
                                    </div>
                                    <div className="flex justify-between">
                                        <span className="text-gray-600">Minimum:</span>
                                        <span className="text-gray-800">{item.minQuantity} {item.unit}</span>
                                    </div>
                                    {item.location && (
                                        <div className="flex justify-between">
                                            <span className="text-gray-600">Location:</span>
                                            <span className="text-gray-800">{item.location}</span>
                                        </div>
                                    )}
                                    {item.supplier && (
                                        <div className="flex justify-between">
                                            <span className="text-gray-600">Supplier:</span>
                                            <span className="text-gray-800">{item.supplier}</span>
                                        </div>
                                    )}
                                </div>

                                {item.status === 'low' && (
                                    <div className="mt-4 p-2 bg-yellow-50 border border-yellow-200 rounded text-sm text-yellow-800 mb-4">
                                        ⚠️ Low stock - reorder needed
                                    </div>
                                )}

                                <div className="mt-4 flex flex-col gap-2">
                                    <PhotoCaptureButton
                                        itemId={item.id}
                                        onPhotoCapture={(files) => handleAddPhoto(item.id, files)}
                                    />
                                    <div className="flex gap-2">
                                        <Button
                                            variant="outline"
                                            size="sm"
                                            className="flex-1"
                                            onClick={async () => {
                                                // Get AI suggestion before showing update modal
                                                try {
                                                    const suggestion = await apiClient.get(`/inventory/ai/suggest/${item.id}?homeId=${homeId}`);
                                                    if (suggestion.confidence > 0.5) {
                                                        alert(`AI Suggestion: Order ${suggestion.suggestedQuantity} ${item.unit}\n${suggestion.reasoning}`);
                                                    }
                                                } catch (error) {
                                                    // Ignore AI errors, just show update modal
                                                }
                                                setSelectedItem(item);
                                                setShowAddItem(true);
                                            }}
                                        >
                                            Update
                                        </Button>
                                        {item.status === 'low' && (
                                            <Button
                                                size="sm"
                                                className="flex-1"
                                                onClick={async () => {
                                                    // Get AI suggestion for quantity
                                                    try {
                                                        const suggestion = await apiClient.get(`/inventory/ai/suggest/${item.id}?homeId=${homeId}`);
                                                        if (suggestion.confidence > 0.5) {
                                                            const useAI = confirm(`AI suggests ordering ${suggestion.suggestedQuantity} ${item.unit}.\n\n${suggestion.reasoning}\n\nUse AI suggestion?`);
                                                            if (useAI) {
                                                                handleAddToOrder({
                                                                    ...item,
                                                                    minQuantity: suggestion.suggestedQuantity
                                                                });
                                                                return;
                                                            }
                                                        }
                                                    } catch (error) {
                                                        // Ignore AI errors, use default
                                                    }
                                                    handleAddToOrder(item);
                                                }}
                                            >
                                                Add to Order
                                            </Button>
                                        )}
                                    </div>
                                </div>
                            </Card>
                        ))}
                    </div>
                )}

                {/* Add Item Modal */}
                {showAddItem && (
                    <AddItemModal
                        item={selectedItem}
                        homeId={homeId}
                        onClose={() => {
                            setShowAddItem(false);
                            setSelectedItem(null);
                        }}
                        onSuccess={() => {
                            setShowAddItem(false);
                            setSelectedItem(null);
                            loadInventory();
                        }}
                    />
                )}

                {/* Order Modal */}
                {showOrderModal && (
                    <OrderModal
                        items={orderItems}
                        onClose={() => setShowOrderModal(false)}
                        onUpdateItems={setOrderItems}
                        onCreateOrder={handleCreateOrder}
                    />
                )}
            </div>
        </main>
    );
}

function PhotoCaptureButton({ itemId, onPhotoCapture }: { itemId: string; onPhotoCapture: (files: File[]) => void }) {
    const [showCapture, setShowCapture] = useState(false);
    const [files, setFiles] = useState<File[]>([]);

    return (
        <div>
            <Button
                variant="outline"
                size="sm"
                className="w-full"
                onClick={() => setShowCapture(!showCapture)}
            >
                📷 {showCapture ? 'Hide Camera' : 'Take Photo'}
            </Button>
            {showCapture && (
                <div className="mt-2">
                    <MediaCapture files={files} onFilesChange={setFiles} />
                    {files.length > 0 && (
                        <Button
                            size="sm"
                            className="w-full mt-2"
                            onClick={() => {
                                onPhotoCapture(files);
                                setFiles([]);
                                setShowCapture(false);
                            }}
                        >
                            Upload Photo
                        </Button>
                    )}
                </div>
            )}
        </div>
    );
}

function AddItemModal({ item, homeId, onClose, onSuccess }: {
    item: InventoryItem | null;
    homeId: string;
    onClose: () => void;
    onSuccess: () => void;
}) {
    const [formData, setFormData] = useState({
        name: item?.name || '',
        category: item?.category || 'groceries',
        quantity: item?.quantity || 0,
        unit: item?.unit || '',
        minQuantity: item?.minQuantity || 0,
        maxQuantity: item?.maxQuantity || 0,
        location: item?.location || '',
        supplier: item?.supplier || '',
        autoReorder: false
    });
    const [photoFiles, setPhotoFiles] = useState<File[]>([]);
    const [loading, setLoading] = useState(false);
    const [aiRecognizing, setAiRecognizing] = useState(false);

    const handleNameChange = async (name: string) => {
        setFormData({ ...formData, name });
        
        // Auto-categorize when name changes (if new item)
        if (!item && name.length > 3) {
            try {
                const result = await apiClient.post('/inventory/ai/categorize', {
                    itemName: name
                });
                if (result.confidence > 0.5) {
                    setFormData(prev => ({ ...prev, category: result.category }));
                }
            } catch (error) {
                // Ignore AI errors
            }
        }
    };

    const handlePhotoCapture = async (files: File[]) => {
        setPhotoFiles(files);
        
        // Try AI recognition if this is a new item
        if (!item && files.length > 0) {
            setAiRecognizing(true);
            try {
                const file = files[0];
                const reader = new FileReader();
                
                reader.onloadend = async () => {
                    const base64 = reader.result as string;
                    const base64Data = base64.split(',')[1];
                    
                    try {
                        const recognition = await apiClient.post('/inventory/ai/recognize', {
                            photo: {
                                data: base64Data,
                                mimeType: file.type
                            }
                        });
                        
                        if (recognition.confidence > 0.5) {
                            setFormData(prev => ({
                                ...prev,
                                name: recognition.itemName,
                                category: recognition.category,
                                unit: recognition.suggestedUnit || prev.unit,
                                supplier: recognition.suggestedSupplier || prev.supplier
                            }));
                            alert(`AI recognized: ${recognition.itemName} (${(recognition.confidence * 100).toFixed(0)}% confidence)`);
                        }
                    } catch (aiError) {
                        console.log('AI recognition failed');
                    } finally {
                        setAiRecognizing(false);
                    }
                };
                
                reader.readAsDataURL(file);
            } catch (error) {
                setAiRecognizing(false);
            }
        }
    };

    const handleSubmit = async (e: React.FormEvent) => {
        e.preventDefault();
        setLoading(true);

        try {
            let photoData = null;
            if (photoFiles.length > 0) {
                const file = photoFiles[0];
                const reader = new FileReader();
                photoData = await new Promise<string>((resolve, reject) => {
                    reader.onloadend = () => {
                        const base64 = reader.result as string;
                        resolve(base64.split(',')[1]);
                    };
                    reader.onerror = reject;
                    reader.readAsDataURL(file);
                });
            }

            const payload: any = {
                ...formData,
                photo: photoData ? {
                    data: photoData,
                    mimeType: photoFiles[0].type
                } : undefined
            };

            if (item) {
                // Update existing
                await apiClient.put(`/inventory/${homeId}/items/${item.id}`, payload);
            } else {
                // Create new
                await apiClient.post(`/inventory/${homeId}/items`, payload);
            }

            onSuccess();
        } catch (error: any) {
            console.error('Failed to save item:', error);
            alert(error.message || 'Failed to save item');
        } finally {
            setLoading(false);
        }
    };

    return (
        <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50">
            <Card className="p-6 max-w-2xl w-full max-h-[90vh] overflow-y-auto">
                <h2 className="text-2xl font-bold mb-4">{item ? 'Update Item' : 'Add New Item'}</h2>
                <form onSubmit={handleSubmit} className="space-y-4">
                    <Input
                        label="Item Name"
                        value={formData.name}
                        onChange={(e) => handleNameChange(e.target.value)}
                        required
                    />
                    {!item && formData.name.length > 0 && (
                        <p className="text-xs text-gray-500">💡 AI will auto-categorize as you type</p>
                    )}
                    <div>
                        <label className="block text-sm font-medium mb-1">Category</label>
                        <select
                            value={formData.category}
                            onChange={(e) => setFormData({ ...formData, category: e.target.value })}
                            className="w-full px-3 py-2 border rounded-md bg-gray-100 dark:bg-gray-800 border-gray-300 dark:border-gray-700"
                            required
                        >
                            <option value="groceries">Groceries</option>
                            <option value="medical_supplies">Medical Supplies</option>
                            <option value="household">Household</option>
                            <option value="office">Office</option>
                            <option value="equipment">Equipment</option>
                        </select>
                    </div>
                    <div className="grid grid-cols-2 gap-4">
                        <Input
                            label="Quantity"
                            type="number"
                            value={formData.quantity}
                            onChange={(e) => setFormData({ ...formData, quantity: parseInt(e.target.value) || 0 })}
                            required
                        />
                        <Input
                            label="Unit"
                            value={formData.unit}
                            onChange={(e) => setFormData({ ...formData, unit: e.target.value })}
                            placeholder="e.g., boxes, rolls, items"
                            required
                        />
                    </div>
                    <div className="grid grid-cols-2 gap-4">
                        <Input
                            label="Min Quantity (Reorder Point)"
                            type="number"
                            value={formData.minQuantity}
                            onChange={(e) => setFormData({ ...formData, minQuantity: parseInt(e.target.value) || 0 })}
                            required
                        />
                        <Input
                            label="Max Quantity (Optional)"
                            type="number"
                            value={formData.maxQuantity}
                            onChange={(e) => setFormData({ ...formData, maxQuantity: parseInt(e.target.value) || 0 })}
                        />
                    </div>
                    <div className="grid grid-cols-2 gap-4">
                        <Input
                            label="Location"
                            value={formData.location}
                            onChange={(e) => setFormData({ ...formData, location: e.target.value })}
                        />
                        <Input
                            label="Supplier"
                            value={formData.supplier}
                            onChange={(e) => setFormData({ ...formData, supplier: e.target.value })}
                        />
                    </div>
                    <div>
                        <label className="flex items-center gap-2">
                            <input
                                type="checkbox"
                                checked={formData.autoReorder}
                                onChange={(e) => setFormData({ ...formData, autoReorder: e.target.checked })}
                            />
                            <span>Auto-reorder when low</span>
                        </label>
                    </div>
                    <div>
                        <label className="block text-sm font-medium mb-2">
                            Photo (Optional) {!item && <span className="text-blue-600">- AI Recognition Available</span>}
                        </label>
                        {aiRecognizing && (
                            <div className="mb-2 text-sm text-blue-600">🤖 AI is analyzing the photo...</div>
                        )}
                        <MediaCapture files={photoFiles} onFilesChange={handlePhotoCapture} />
                    </div>
                    <div className="flex gap-2 justify-end">
                        <Button type="button" variant="outline" onClick={onClose}>
                            Cancel
                        </Button>
                        <Button type="submit" disabled={loading}>
                            {loading ? 'Saving...' : item ? 'Update' : 'Add Item'}
                        </Button>
                    </div>
                </form>
            </Card>
        </div>
    );
}

function OrderModal({ items, onClose, onUpdateItems, onCreateOrder }: {
    items: OrderItem[];
    onClose: () => void;
    onUpdateItems: (items: OrderItem[]) => void;
    onCreateOrder: () => void;
}) {
    const [notes, setNotes] = useState('');

    const removeItem = (index: number) => {
        onUpdateItems(items.filter((_, i) => i !== index));
    };

    const updateItemQuantity = (index: number, quantity: number) => {
        const updated = [...items];
        updated[index].quantity = quantity;
        if (updated[index].unitPrice) {
            updated[index].totalPrice = updated[index].unitPrice * quantity;
        }
        onUpdateItems(updated);
    };

    const updateItemPrice = (index: number, price: number) => {
        const updated = [...items];
        updated[index].unitPrice = price;
        updated[index].totalPrice = price * updated[index].quantity;
        onUpdateItems(updated);
    };

    const total = items.reduce((sum, item) => sum + (item.totalPrice || 0), 0);

    return (
        <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50">
            <Card className="p-6 max-w-3xl w-full max-h-[90vh] overflow-y-auto">
                <h2 className="text-2xl font-bold mb-4">Create Purchase Order</h2>
                
                {items.length === 0 ? (
                    <div className="text-center py-8 text-gray-500">
                        No items in order. Add items from the inventory list.
                    </div>
                ) : (
                    <div className="space-y-4">
                        <div className="space-y-2">
                            {items.map((item, index) => (
                                <div key={index} className="border rounded p-4 flex items-center justify-between">
                                    <div className="flex-1">
                                        <h4 className="font-semibold">{item.itemName}</h4>
                                        <p className="text-sm text-gray-600">{item.unit}</p>
                                    </div>
                                    <div className="flex items-center gap-2">
                                        <Input
                                            type="number"
                                            value={item.quantity}
                                            onChange={(e) => updateItemQuantity(index, parseInt(e.target.value) || 0)}
                                            className="w-20"
                                        />
                                        <span className="text-sm">@</span>
                                        <Input
                                            type="number"
                                            step="0.01"
                                            placeholder="Price"
                                            value={item.unitPrice || ''}
                                            onChange={(e) => updateItemPrice(index, parseFloat(e.target.value) || 0)}
                                            className="w-24"
                                        />
                                        <span className="text-sm font-semibold w-20 text-right">
                                            ${(item.totalPrice || 0).toFixed(2)}
                                        </span>
                                        <Button
                                            variant="outline"
                                            size="sm"
                                            onClick={() => removeItem(index)}
                                        >
                                            Remove
                                        </Button>
                                    </div>
                                </div>
                            ))}
                        </div>
                        <div className="border-t pt-4">
                            <div className="flex justify-between text-lg font-bold">
                                <span>Total:</span>
                                <span>${total.toFixed(2)}</span>
                            </div>
                        </div>
                        <div>
                            <label className="block text-sm font-medium mb-2">Notes (Optional)</label>
                            <textarea
                                value={notes}
                                onChange={(e) => setNotes(e.target.value)}
                                className="w-full px-3 py-2 border rounded-md bg-gray-100 dark:bg-gray-800 border-gray-300 dark:border-gray-700"
                                rows={3}
                                placeholder="Add any notes about this order..."
                            />
                        </div>
                    </div>
                )}

                <div className="flex gap-2 justify-end mt-6">
                    <Button variant="outline" onClick={onClose}>
                        Cancel
                    </Button>
                    <Button onClick={onCreateOrder} disabled={items.length === 0}>
                        Create Order
                    </Button>
                </div>
            </Card>
        </div>
    );
}

"use client";
import React, { useState } from 'react';
import { VoiceInput } from '@/components/VoiceInput';
import { Input } from '@/components/ui/input';
import { Select } from '@/components/ui/select';
import { Textarea } from '@/components/ui/textarea';
import { Alert } from '@/components/ui/alert';
import { Badge } from '@/components/ui/badge';
import { medicationsAPI } from '@/lib/api/medications';

export default function PRNEnhancedPage() {
    const [formData, setFormData] = useState({
        individualId: '',
        medication: '',
        dosage: '',
        notes: ''
    });
    const [loading, setLoading] = useState(false);
    const [result, setResult] = useState<any>(null);

    const handleVoiceTranscript = (text: string) => {
        setFormData(prev => ({ ...prev, notes: prev.notes + ' ' + text }));
    };

    const handleSubmit = async (e: React.FormEvent) => {
        e.preventDefault();
        setLoading(true);

        try {
            const response = await medicationsAPI.prn.create({
                individualId: Number(formData.individualId),
                medication: formData.medication,
                dosage: formData.dosage,
                requestedAt: new Date().toISOString(),
                status: 'pending',
                notes: formData.notes
            });

            setResult(response);
            setFormData({ individualId: '', medication: '', dosage: '', notes: '' });
        } catch (error) {
            console.error('Failed to submit PRN:', error);
        } finally {
            setLoading(false);
        }
    };

    return (
        <div className="min-h-screen bg-gray-100 p-6">
            <div className="max-w-4xl mx-auto">
                <h1 className="text-3xl font-bold text-gray-900 mb-6">PRN Request (Enhanced with Voice)</h1>

                {result && (
                    <Alert
                        variant={result.status === 'approved' ? 'success' : result.status === 'denied' ? 'error' : 'warning'}
                        title={`PRN Request ${result.status}`}
                        className="mb-6"
                        onClose={() => setResult(null)}
                    >
                        <div className="space-y-2">
                            <p><strong>Medication:</strong> {result.medication} {result.dosage}</p>
                            {result.safetyWarnings && result.safetyWarnings.length > 0 && (
                                <div>
                                    <strong>Safety Warnings:</strong>
                                    <ul className="list-disc list-inside mt-1">
                                        {result.safetyWarnings.map((warning: string, idx: number) => (
                                            <li key={idx} className="text-sm">{warning}</li>
                                        ))}
                                    </ul>
                                </div>
                            )}
                            {result.safetyScore && (
                                <p><strong>Safety Score:</strong> {result.safetyScore}/100</p>
                            )}
                        </div>
                    </Alert>
                )}

                <div className="bg-white rounded-lg shadow p-6">
                    <form onSubmit={handleSubmit} className="space-y-6">
                        <Input
                            label="Individual ID"
                            type="number"
                            value={formData.individualId}
                            onChange={(e) => setFormData({ ...formData, individualId: e.target.value })}
                            required
                        />

                        <Input
                            label="Medication"
                            value={formData.medication}
                            onChange={(e) => setFormData({ ...formData, medication: e.target.value })}
                            required
                            placeholder="e.g., Ibuprofen"
                        />

                        <Input
                            label="Dosage"
                            value={formData.dosage}
                            onChange={(e) => setFormData({ ...formData, dosage: e.target.value })}
                            required
                            placeholder="e.g., 400mg"
                        />

                        <div>
                            <label className="block text-sm font-medium text-gray-700 mb-3">
                                Notes (Type or Use Voice)
                            </label>
                            <Textarea
                                value={formData.notes}
                                onChange={(e) => setFormData({ ...formData, notes: e.target.value })}
                                rows={4}
                                placeholder="Add notes about the PRN request..."
                            />
                            <div className="mt-3">
                                <VoiceInput onTranscript={handleVoiceTranscript} />
                            </div>
                        </div>

                        <button
                            type="submit"
                            disabled={loading}
                            className="w-full bg-blue-600 text-white py-3 rounded-lg font-medium hover:bg-blue-700 disabled:bg-gray-400"
                        >
                            {loading ? 'Submitting...' : 'Submit PRN Request'}
                        </button>
                    </form>

                    <div className="mt-6 p-4 bg-blue-50 rounded border border-blue-200">
                        <h3 className="font-semibold text-blue-900 mb-2">🛡️ Safety Features Active</h3>
                        <ul className="text-sm text-blue-800 space-y-1">
                            <li>✓ Drug interaction checking</li>
                            <li>✓ Allergy verification</li>
                            <li>✓ Dosage validation</li>
                            <li>✓ Maximum daily dose monitoring</li>
                        </ul>
                    </div>
                </div>
            </div>
        </div>
    );
}

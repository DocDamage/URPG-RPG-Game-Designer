import React from 'react';
import { Badge } from './ui/badge';

export interface Medication {
    name: string;
    dosage: string;
    time: string;
    status?: 'pending' | 'administered' | 'missed';
    warnings?: string[];
}

export interface MedicationCardProps {
    medication: Medication;
    onAdminister?: () => void;
}

export function MedicationCard({ medication, onAdminister }: MedicationCardProps) {
    const statusColors = {
        pending: 'warning' as const,
        administered: 'success' as const,
        missed: 'danger' as const
    };

    return (
        <div className="bg-white border rounded-lg p-4 shadow-sm hover:shadow-md transition-shadow">
            <div className="flex items-start justify-between">
                <div className="flex-1">
                    <h3 className="text-lg font-semibold text-gray-900">{medication.name}</h3>
                    <p className="text-sm text-gray-600 mt-1">
                        Dosage: <span className="font-medium">{medication.dosage}</span>
                    </p>
                    <p className="text-sm text-gray-600">
                        Time: <span className="font-medium">{medication.time}</span>
                    </p>
                </div>

                {medication.status && (
                    <Badge variant={statusColors[medication.status]}>
                        {medication.status}
                    </Badge>
                )}
            </div>

            {medication.warnings && medication.warnings.length > 0 && (
                <div className="mt-3 p-3 bg-yellow-50 border border-yellow-200 rounded">
                    <p className="text-xs font-semibold text-yellow-800 mb-1">⚠️ Warnings</p>
                    {medication.warnings.map((warning, idx) => (
                        <p key={idx} className="text-xs text-yellow-700">• {warning}</p>
                    ))}
                </div>
            )}

            {onAdminister && medication.status === 'pending' && (
                <button
                    onClick={onAdminister}
                    className="mt-4 w-full py-2 bg-green-600 text-white rounded font-medium hover:bg-green-700"
                >
                    Mark as Administered
                </button>
            )}
        </div>
    );
}

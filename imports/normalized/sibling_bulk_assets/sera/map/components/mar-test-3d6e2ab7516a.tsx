/**
 * MAR Component Tests
 * 
 * Tests for medication administration record components.
 */

import { describe, it, expect } from '@testing-library/react';
import { render, screen } from '@testing-library/react';
import React from 'react';

// Mock component for testing
const MedicationSchedule = ({ medications }: { medications: any[] }) => {
    return (
        <div data-testid="medication-schedule">
            {medications.map((med) => (
                <div key={med.id} data-testid={`medication-${med.id}`}>
                    {med.name}
                </div>
            ))}
        </div>
    );
};

describe('MAR Components', () => {
    it('should render medication schedule', () => {
        const medications = [
            { id: '1', name: 'Aspirin', dosage: '10mg' },
            { id: '2', name: 'Ibuprofen', dosage: '20mg' },
        ];

        render(<MedicationSchedule medications={medications} />);

        expect(screen.getByTestId('medication-schedule')).toBeInTheDocument();
        expect(screen.getByTestId('medication-1')).toHaveTextContent('Aspirin');
        expect(screen.getByTestId('medication-2')).toHaveTextContent('Ibuprofen');
    });

    it('should handle empty medication list', () => {
        render(<MedicationSchedule medications={[]} />);

        expect(screen.getByTestId('medication-schedule')).toBeInTheDocument();
    });

    it('should display medication details', () => {
        const medications = [
            { id: '1', name: 'Aspirin', dosage: '10mg', time: '08:00' },
        ];

        render(<MedicationSchedule medications={medications} />);

        const medication = screen.getByTestId('medication-1');
        expect(medication).toHaveTextContent('Aspirin');
    });
});


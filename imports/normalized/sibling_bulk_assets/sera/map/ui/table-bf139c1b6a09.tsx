import React from 'react';

export interface Column<T> {
    key: keyof T | string;
    header: string;
    render?: (value: any, row: T) => React.ReactNode;
    width?: string;
}

export interface TableProps<T> {
    data: T[];
    columns: Column<T>[];
    onRowClick?: (row: T) => void;
    className?: string;
}

export function Table<T extends Record<string, any>>({
    data,
    columns,
    onRowClick,
    className = ''
}: TableProps<T>) {
    return (
        <div className={`overflow-x-auto ${className}`}>
            <table className="min-w-full divide-y divide-gray-200">
                <thead className="bg-gray-50">
                    <tr>
                        {columns.map((col, idx) => (
                            <th
                                key={idx}
                                className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider"
                                style={{ width: col.width }}
                            >
                                {col.header}
                            </th>
                        ))}
                    </tr>
                </thead>
                <tbody className="bg-white divide-y divide-gray-200">
                    {data.map((row, rowIdx) => (
                        <tr
                            key={rowIdx}
                            onClick={() => onRowClick?.(row)}
                            className={onRowClick ? 'cursor-pointer hover:bg-gray-50' : ''}
                        >
                            {columns.map((col, colIdx) => (
                                <td key={colIdx} className="px-6 py-4 whitespace-nowrap text-sm text-gray-900">
                                    {col.render
                                        ? col.render(row[col.key as keyof T], row)
                                        : row[col.key as keyof T]
                                    }
                                </td>
                            ))}
                        </tr>
                    ))}
                </tbody>
            </table>
            {data.length === 0 && (
                <div className="text-center py-8 text-gray-500">
                    No data available
                </div>
            )}
        </div>
    );
}

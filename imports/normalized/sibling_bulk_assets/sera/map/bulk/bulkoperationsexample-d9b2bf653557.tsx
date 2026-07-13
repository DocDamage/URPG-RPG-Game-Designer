"use client";

import React, { useState, useCallback } from "react";
import {
  useBulkSelection,
  SelectionCheckbox,
  BulkActionsBar,
  BulkOperationModal,
  useBulkOperations,
} from "./";

// Example data type
interface Individual {
  id: string;
  firstName: string;
  lastName: string;
  email: string;
  status: string;
  riskLevel: string;
}

// Mock data
const mockIndividuals: Individual[] = [
  { id: "1", firstName: "John", lastName: "Doe", email: "john@example.com", status: "active", riskLevel: "low" },
  { id: "2", firstName: "Jane", lastName: "Smith", email: "jane@example.com", status: "active", riskLevel: "medium" },
  { id: "3", firstName: "Bob", lastName: "Johnson", email: "bob@example.com", status: "inactive", riskLevel: "high" },
  { id: "4", firstName: "Alice", lastName: "Williams", email: "alice@example.com", status: "active", riskLevel: "low" },
  { id: "5", firstName: "Charlie", lastName: "Brown", email: "charlie@example.com", status: "pending", riskLevel: "medium" },
];

/**
 * Example component demonstrating the bulk operations system
 * 
 * This shows how to integrate all the bulk operation components
 * together in a real-world data table scenario.
 */
export function BulkOperationsExample() {
  const [individuals] = useState<Individual[]>(mockIndividuals);
  const [modalState, setModalState] = useState<"closed" | "confirm" | "processing" | "success" | "error" | "partial">("closed");
  const [currentAction, setCurrentAction] = useState<"delete" | "export" | "assign" | "update">("delete");
  const [progress, setProgress] = useState(0);

  // Bulk selection hook
  const {
    selectedIds,
    isSelected,
    toggleSelection,
    selectAll,
    clearSelection,
    selectRange,
    lastSelectedId,
    setLastSelectedId,
    selectedCount,
    hasSelection,
  } = useBulkSelection<Individual>({
    idField: "id",
    onSelectionChange: (ids) => {
      console.log("Selection changed:", ids);
    },
  });

  // Bulk operations API hook
  const {
    status: operationStatus,
    result: operationResult,
    bulkDelete,
    bulkExport,
    bulkAssign,
    bulkUpdate,
    isLoading: isOperationLoading,
    reset: resetOperation,
  } = useBulkOperations({
    baseEndpoint: "/api/individuals",
    onComplete: (result) => {
      console.log("Operation completed:", result);
      if (result.status === "completed") {
        setModalState("success");
      } else if (result.status === "partial") {
        setModalState("partial");
      } else {
        setModalState("error");
      }
    },
    onError: (error) => {
      console.error("Operation failed:", error);
      setModalState("error");
    },
  });

  // Get all IDs for select all functionality
  const allIds = individuals.map((i) => i.id);

  // Handle checkbox change
  const handleCheckboxChange = useCallback(
    (id: string, checked: boolean, event: React.MouseEvent | React.ChangeEvent) => {
      // Check if shift key was held
      const nativeEvent = (event as any).nativeEvent;
      const isShiftClick = nativeEvent?.shiftKey;

      if (isShiftClick && lastSelectedId) {
        // Range selection
        selectRange(lastSelectedId, id, allIds);
      } else {
        // Single selection
        toggleSelection(id);
        if (checked) {
          setLastSelectedId(id);
        }
      }
    },
    [toggleSelection, selectRange, lastSelectedId, allIds, setLastSelectedId]
  );

  // Handle header checkbox (select all)
  const handleSelectAllToggle = useCallback(() => {
    const allSelected = allIds.every((id) => selectedIds.has(id));
    if (allSelected) {
      clearSelection();
    } else {
      selectAll(allIds);
    }
  }, [allIds, selectedIds, selectAll, clearSelection]);

  // Handle bulk actions
  const handleBulkAction = useCallback(
    (action: { id: string; type: string }) => {
      setCurrentAction(action.type as any);

      switch (action.type) {
        case "delete":
          setModalState("confirm");
          break;
        case "export":
          handleExport();
          break;
        case "assign":
          setModalState("confirm");
          break;
        case "update-status":
          setModalState("confirm");
          break;
        default:
          break;
      }
    },
    []
  );

  // Handle confirm action
  const handleConfirm = useCallback(async () => {
    setModalState("processing");
    setProgress(0);

    const selectedIdsArray = Array.from(selectedIds);

    try {
      switch (currentAction) {
        case "delete":
          await bulkDelete({
            ids: selectedIdsArray,
            softDelete: true,
            reason: "Bulk delete operation",
          });
          break;
        case "export":
          await bulkExport({
            ids: selectedIdsArray,
            format: "csv",
          });
          break;
        case "assign":
          await bulkAssign({
            ids: selectedIdsArray,
            assignTo: "staff-123",
            assigneeType: "staff",
            reason: "Reassignment",
          });
          break;
        case "update":
          await bulkUpdate({
            updates: selectedIdsArray.map((id) => ({
              id,
              data: { status: "active" },
            })),
          });
          break;
      }

      // Simulate progress updates
      const progressInterval = setInterval(() => {
        setProgress((prev) => {
          if (prev >= 100) {
            clearInterval(progressInterval);
            return 100;
          }
          return prev + 10;
        });
      }, 500);

    } catch (error) {
      console.error("Operation failed:", error);
      setModalState("error");
    }
  }, [currentAction, selectedIds, bulkDelete, bulkExport, bulkAssign, bulkUpdate]);

  // Handle export
  const handleExport = useCallback(async () => {
    setModalState("processing");
    setProgress(0);

    try {
      await bulkExport({
        ids: Array.from(selectedIds),
        format: "csv",
      });
    } catch (error) {
      setModalState("error");
    }
  }, [selectedIds, bulkExport]);

  // Handle modal close
  const handleCloseModal = useCallback(() => {
    setModalState("closed");
    resetOperation();
    setProgress(0);
  }, [resetOperation]);

  // Handle retry
  const handleRetry = useCallback(async () => {
    if (operationResult?.operationId) {
      setModalState("processing");
      // Retry logic would go here
    }
  }, [operationResult]);

  // Handle download
  const handleDownload = useCallback((url: string, fileName: string) => {
    console.log("Downloading:", url, fileName);
    // In a real implementation, this would trigger a file download
    const link = document.createElement("a");
    link.href = url;
    link.download = fileName;
    link.click();
  }, []);

  // Custom actions configuration
  const customActions = [
    {
      id: "export",
      label: "Export",
      type: "export" as const,
    },
    {
      id: "assign",
      label: "Assign",
      type: "assign" as const,
    },
    {
      id: "update-status",
      label: "Update Status",
      type: "update-status" as const,
    },
    {
      id: "delete",
      label: "Delete",
      type: "delete" as const,
      destructive: true,
      requiresConfirmation: true,
    },
  ];

  return (
    <div className="space-y-4">
      <h2 className="text-2xl font-bold text-gray-900">Bulk Operations Demo</h2>
      <p className="text-gray-600">
        Select items using checkboxes. Hold Shift and click to select a range.
      </p>

      {/* Bulk Actions Bar */}
      <BulkActionsBar
        selectedCount={selectedCount}
        totalCount={individuals.length}
        isAllSelected={allIds.every((id) => selectedIds.has(id))}
        isIndeterminate={selectedCount > 0 && selectedCount < allIds.length}
        actions={customActions}
        onAction={handleBulkAction}
        onSelectAll={() => selectAll(allIds)}
        onClearSelection={clearSelection}
        onToggleSelectAll={handleSelectAllToggle}
        isLoading={isOperationLoading}
        entityName="individuals"
        sticky={true}
        stickyOffset={0}
      />

      {/* Data Table */}
      <div className="bg-white shadow overflow-hidden border-b border-gray-200 sm:rounded-lg">
        <table className="min-w-full divide-y divide-gray-200">
          <thead className="bg-gray-50">
            <tr>
              <th scope="col" className="px-6 py-3 w-12">
                <SelectionCheckbox
                  id="header"
                  checked={allIds.every((id) => selectedIds.has(id))}
                  indeterminate={selectedCount > 0 && selectedCount < allIds.length}
                  onChange={() => handleSelectAllToggle()}
                  label="Select all"
                />
              </th>
              <th
                scope="col"
                className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider"
              >
                Name
              </th>
              <th
                scope="col"
                className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider"
              >
                Email
              </th>
              <th
                scope="col"
                className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider"
              >
                Status
              </th>
              <th
                scope="col"
                className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider"
              >
                Risk Level
              </th>
            </tr>
          </thead>
          <tbody className="bg-white divide-y divide-gray-200">
            {individuals.map((individual) => (
              <tr
                key={individual.id}
                className={selectedIds.has(individual.id) ? "bg-blue-50" : "hover:bg-gray-50"}
              >
                <td className="px-6 py-4 whitespace-nowrap">
                  <SelectionCheckbox
                    id={individual.id}
                    checked={isSelected(individual.id)}
                    onChange={(checked) =>
                      handleCheckboxChange(individual.id, checked, {} as any)
                    }
                    label={`Select ${individual.firstName} ${individual.lastName}`}
                  />
                </td>
                <td className="px-6 py-4 whitespace-nowrap">
                  <div className="text-sm font-medium text-gray-900">
                    {individual.firstName} {individual.lastName}
                  </div>
                </td>
                <td className="px-6 py-4 whitespace-nowrap">
                  <div className="text-sm text-gray-500">{individual.email}</div>
                </td>
                <td className="px-6 py-4 whitespace-nowrap">
                  <span
                    className={`px-2 inline-flex text-xs leading-5 font-semibold rounded-full ${
                      individual.status === "active"
                        ? "bg-green-100 text-green-800"
                        : individual.status === "inactive"
                        ? "bg-red-100 text-red-800"
                        : "bg-yellow-100 text-yellow-800"
                    }`}
                  >
                    {individual.status}
                  </span>
                </td>
                <td className="px-6 py-4 whitespace-nowrap text-sm text-gray-500">
                  <span
                    className={`capitalize ${
                      individual.riskLevel === "high"
                        ? "text-red-600 font-semibold"
                        : individual.riskLevel === "medium"
                        ? "text-yellow-600"
                        : "text-green-600"
                    }`}
                  >
                    {individual.riskLevel}
                  </span>
                </td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>

      {/* Bulk Operation Modal */}
      <BulkOperationModal
        isOpen={modalState !== "closed"}
        onClose={handleCloseModal}
        state={modalState === "closed" ? "confirm" : modalState}
        operationType={currentAction}
        itemCount={selectedCount}
        progress={progress}
        result={operationResult || undefined}
        isDestructive={currentAction === "delete"}
        onConfirm={handleConfirm}
        onCancel={handleCloseModal}
        onRetry={handleRetry}
        onDownload={handleDownload}
        entityName="individuals"
        maxVisibleErrors={5}
      />

      {/* Debug Info */}
      <div className="mt-8 p-4 bg-gray-100 rounded-lg">
        <h3 className="font-semibold mb-2">Debug Info</h3>
        <p className="text-sm text-gray-600">Selected IDs: {Array.from(selectedIds).join(", ") || "None"}</p>
        <p className="text-sm text-gray-600">Last Selected ID: {lastSelectedId || "None"}</p>
        <p className="text-sm text-gray-600">Operation Status: {operationStatus}</p>
      </div>
    </div>
  );
}

export default BulkOperationsExample;

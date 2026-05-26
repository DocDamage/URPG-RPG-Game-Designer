"use client";

import React, { useCallback, useRef, useState, useEffect } from "react";
import { clsx } from "clsx";
import { Minus } from "lucide-react";

export interface SelectionCheckboxProps {
  /** Unique identifier for this checkbox */
  id: string;
  /** Whether the checkbox is checked */
  checked: boolean;
  /** Whether the checkbox is in indeterminate state */
  indeterminate?: boolean;
  /** Callback when checkbox state changes */
  onChange: (checked: boolean, event: React.ChangeEvent<HTMLInputElement>) => void;
  /** Callback for shift+click range selection */
  onRangeSelect?: (id: string) => void;
  /** Label for accessibility */
  label?: string;
  /** Whether the checkbox is disabled */
  disabled?: boolean;
  /** Additional CSS classes */
  className?: string;
  /** Size variant */
  size?: "sm" | "md" | "lg";
  /** Test ID for testing */
  "data-testid"?: string;
}

/**
 * Selection Checkbox Component
 * 
 * A checkbox component with support for:
 * - Indeterminate state (partial selection)
 * - Row-level selection
 * - Shift+click range selection
 * - Accessible labels and keyboard navigation
 */
export const SelectionCheckbox = React.forwardRef<HTMLInputElement, SelectionCheckboxProps>(
  (
    {
      id,
      checked,
      indeterminate = false,
      onChange,
      onRangeSelect,
      label,
      disabled = false,
      className,
      size = "md",
      "data-testid": testId,
    },
    forwardedRef
  ) => {
    const internalRef = useRef<HTMLInputElement>(null);
    const checkboxRef = (forwardedRef as React.RefObject<HTMLInputElement>) || internalRef;
    const [isShiftPressed, setIsShiftPressed] = useState(false);

    // Set indeterminate state on the DOM element
    useEffect(() => {
      if (checkboxRef.current) {
        checkboxRef.current.indeterminate = indeterminate;
      }
    }, [indeterminate]);

    // Track shift key state
    useEffect(() => {
      const handleKeyDown = (e: KeyboardEvent) => {
        if (e.key === "Shift") {
          setIsShiftPressed(true);
        }
      };

      const handleKeyUp = (e: KeyboardEvent) => {
        if (e.key === "Shift") {
          setIsShiftPressed(false);
        }
      };

      window.addEventListener("keydown", handleKeyDown);
      window.addEventListener("keyup", handleKeyUp);

      return () => {
        window.removeEventListener("keydown", handleKeyDown);
        window.removeEventListener("keyup", handleKeyUp);
      };
    }, []);

    const handleChange = useCallback(
      (event: React.ChangeEvent<HTMLInputElement>) => {
        onChange(event.target.checked, event);
      },
      [onChange]
    );

    const handleClick = useCallback(
      (event: React.MouseEvent<HTMLInputElement>) => {
        // Handle shift+click for range selection
        if (isShiftPressed && onRangeSelect) {
          event.preventDefault();
          onRangeSelect(id);
        }
      },
      [isShiftPressed, onRangeSelect, id]
    );

    const sizeClasses = {
      sm: "h-4 w-4",
      md: "h-5 w-5",
      lg: "h-6 w-6",
    };

    const iconSizes = {
      sm: "h-3 w-3",
      md: "h-4 w-4",
      lg: "h-5 w-5",
    };

    return (
      <label
        htmlFor={`checkbox-${id}`}
        className={clsx(
          "inline-flex items-center justify-center cursor-pointer",
          disabled && "cursor-not-allowed opacity-50",
          className
        )}
      >
        <span className="sr-only">{label || `Select item ${id}`}</span>
        <div className="relative">
          <input
            ref={checkboxRef}
            id={`checkbox-${id}`}
            type="checkbox"
            checked={checked}
            onChange={handleChange}
            onClick={handleClick}
            disabled={disabled}
            data-testid={testId}
            className={clsx(
              "peer appearance-none rounded border border-gray-300 bg-white",
              "checked:bg-blue-600 checked:border-blue-600",
              "indeterminate:bg-blue-600 indeterminate:border-blue-600",
              "focus:outline-none focus:ring-2 focus:ring-blue-500 focus:ring-offset-2",
              "transition-colors duration-200",
              "disabled:cursor-not-allowed disabled:opacity-50",
              sizeClasses[size]
            )}
            aria-checked={indeterminate ? "mixed" : checked}
            aria-label={label || `Select item ${id}`}
          />
          {/* Check icon */}
          <svg
            className={clsx(
              "absolute inset-0 m-auto pointer-events-none",
              "text-white opacity-0 peer-checked:opacity-100",
              "transition-opacity duration-200",
              iconSizes[size]
            )}
            fill="none"
            viewBox="0 0 24 24"
            stroke="currentColor"
            strokeWidth={3}
          >
            <path
              strokeLinecap="round"
              strokeLinejoin="round"
              d="M5 13l4 4L19 7"
            />
          </svg>
          {/* Indeterminate icon */}
          {indeterminate && (
            <Minus
              className={clsx(
                "absolute inset-0 m-auto pointer-events-none text-white",
                iconSizes[size]
              )}
              strokeWidth={3}
            />
          )}
        </div>
        {label && (
          <span className="ml-2 text-sm text-gray-700">{label}</span>
        )}
      </label>
    );
  }
);

SelectionCheckbox.displayName = "SelectionCheckbox";

export default SelectionCheckbox;

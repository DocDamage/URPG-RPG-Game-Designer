"use client";

/**
 * Branded Button Component
 * 
 * Button component that uses the organization's brand colors.
 * Supports multiple variants, sizes, and states.
 */

import React, { forwardRef } from 'react';
import { clsx } from 'clsx';
import type { BrandedButtonProps } from '../types';

export const BrandedButton = forwardRef<HTMLButtonElement, BrandedButtonProps>(
  (
    {
      children,
      variant = 'primary',
      size = 'md',
      isLoading = false,
      loadingText,
      leftIcon,
      rightIcon,
      isFullWidth = false,
      className,
      disabled,
      ...props
    },
    ref
  ) => {
    // Base styles using CSS variables
    const baseStyles = `
      inline-flex
      items-center
      justify-center
      font-medium
      rounded-[var(--sera-radius-md)]
      transition-all
      duration-200
      focus:outline-none
      focus:ring-2
      focus:ring-offset-2
      focus:ring-[var(--sera-primary)]
      disabled:opacity-50
      disabled:cursor-not-allowed
      disabled:pointer-events-none
    `;

    // Variant styles using brand colors
    const variantStyles = {
      primary: `
        bg-[var(--sera-primary)]
        text-[var(--sera-text-inverse)]
        hover:bg-[var(--sera-primary-hover)]
        active:bg-[var(--sera-primary-active)]
        shadow-sm
      `,
      secondary: `
        bg-[var(--sera-secondary)]
        text-[var(--sera-text-inverse)]
        hover:bg-[var(--sera-secondary-hover)]
      `,
      outline: `
        bg-transparent
        border-2
        border-[var(--sera-primary)]
        text-[var(--sera-primary)]
        hover:bg-[var(--sera-primary)]
        hover:text-[var(--sera-text-inverse)]
      `,
      ghost: `
        bg-transparent
        text-[var(--sera-primary)]
        hover:bg-[var(--sera-primary-light)]
        hover:bg-opacity-10
      `,
      danger: `
        bg-[var(--sera-error)]
        text-white
        hover:opacity-90
        active:opacity-80
      `,
    };

    // Size styles
    const sizeStyles = {
      sm: 'px-3 py-1.5 text-sm gap-1.5',
      md: 'px-4 py-2 text-base gap-2',
      lg: 'px-6 py-3 text-lg gap-2.5',
    };

    // Width style
    const widthStyle = isFullWidth ? 'w-full' : '';

    return (
      <button
        ref={ref}
        className={clsx(
          baseStyles,
          variantStyles[variant],
          sizeStyles[size],
          widthStyle,
          className
        )}
        disabled={disabled || isLoading}
        {...props}
      >
        {isLoading ? (
          <>
            <LoadingSpinner size={size} />
            {loadingText || children}
          </>
        ) : (
          <>
            {leftIcon && <span className="flex-shrink-0">{leftIcon}</span>}
            {children}
            {rightIcon && <span className="flex-shrink-0">{rightIcon}</span>}
          </>
        )}
      </button>
    );
  }
);

BrandedButton.displayName = 'BrandedButton';

/**
 * Loading Spinner Component
 */
interface LoadingSpinnerProps {
  size: 'sm' | 'md' | 'lg';
}

function LoadingSpinner({ size }: LoadingSpinnerProps) {
  const sizeMap = {
    sm: 'w-4 h-4',
    md: 'w-5 h-5',
    lg: 'w-6 h-6',
  };

  return (
    <svg
      className={clsx('animate-spin', sizeMap[size])}
      xmlns="http://www.w3.org/2000/svg"
      fill="none"
      viewBox="0 0 24 24"
    >
      <circle
        className="opacity-25"
        cx="12"
        cy="12"
        r="10"
        stroke="currentColor"
        strokeWidth="4"
      />
      <path
        className="opacity-75"
        fill="currentColor"
        d="M4 12a8 8 0 018-8V0C5.373 0 0 5.373 0 12h4zm2 5.291A7.962 7.962 0 014 12H0c0 3.042 1.135 5.824 3 7.938l3-2.647z"
      />
    </svg>
  );
}

// Export default
export default BrandedButton;

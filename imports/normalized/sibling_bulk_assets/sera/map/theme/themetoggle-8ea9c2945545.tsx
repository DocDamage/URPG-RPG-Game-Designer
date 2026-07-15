"use client";

/**
 * Theme Toggle Component
 * 
 * Accessible toggle/dropdown component for switching between light, dark, and system themes.
 * 
 * Features:
 * - Sun/moon icons with smooth transitions
 * - Dropdown menu for selecting light/dark/system
 * - Keyboard shortcut support (Ctrl/Cmd+Shift+L)
 * - Full ARIA accessibility support
 * - Integration with keyboard navigation
 * - Smooth icon animations
 */

import React, { useState, useCallback } from 'react';
import { clsx } from 'clsx';
import { 
  Sun, 
  Moon, 
  Monitor, 
  Check,
  type LucideProps 
} from 'lucide-react';
import { useTheme } from './ThemeProvider';
import { useThemeShortcut } from './useThemeShortcut';
import { ThemeToggleProps, ThemeDropdownProps } from './types';
import {
  DropdownMenu,
  DropdownMenuContent,
  DropdownMenuItem,
  DropdownMenuTrigger,
  DropdownMenuSeparator,
  DropdownMenuLabel,
  DropdownMenuShortcut,
} from '@/components/ui/dropdown-menu';
import { Button } from '@/components/ui/button';

// ============================================================================
// Icons
// ============================================================================

const SunIcon = (props: LucideProps) => (
  <Sun {...props} className={clsx("transition-all duration-300", props.className)} />
);

const MoonIcon = (props: LucideProps) => (
  <Moon {...props} className={clsx("transition-all duration-300", props.className)} />
);

const SystemIcon = (props: LucideProps) => (
  <Monitor {...props} className={clsx("transition-all duration-300", props.className)} />
);

// ============================================================================
// Simple Toggle Button
// ============================================================================

/**
 * Simple theme toggle button that cycles between light and dark modes.
 * Shows sun icon in light mode, moon icon in dark mode.
 */
export function ThemeToggleButton({
  variant = 'outline',
  size = 'sm',
  className,
  sunIcon,
  moonIcon,
  ariaLabel,
  disabled,
  ...props
}: ThemeToggleProps) {
  const { resolvedTheme, toggleTheme, isInitialized } = useTheme();
  
  // Enable keyboard shortcut
  useThemeShortcut();
  
  const handleToggle = useCallback(() => {
    toggleTheme();
  }, [toggleTheme]);
  
  // Determine which icon to show
  const isDark = resolvedTheme === 'dark';
  
  // Default ARIA label
  const defaultAriaLabel = isDark ? 'Switch to light mode' : 'Switch to dark mode';
  
  if (!isInitialized) {
    // Render placeholder during SSR/hydration to prevent mismatch
    return (
      <Button
        variant={variant === 'minimal' ? 'ghost' : variant}
        size={size}
        className={clsx(
          'opacity-50 cursor-not-allowed',
          variant === 'minimal' && 'px-2',
          className
        )}
        disabled
        aria-label="Loading theme toggle"
      >
        <SunIcon className="h-4 w-4 opacity-0" />
      </Button>
    );
  }
  
  return (
    <Button
      variant={variant === 'minimal' ? 'ghost' : variant}
      size={size}
      className={clsx(
        'relative overflow-hidden',
        variant === 'minimal' && 'px-2',
        className
      )}
      onClick={handleToggle}
      disabled={disabled}
      aria-label={ariaLabel || defaultAriaLabel}
      aria-pressed={isDark}
      {...props}
    >
      <span className="relative flex items-center justify-center w-4 h-4">
        {/* Sun icon - visible in light mode */}
        <span
          className={clsx(
            'absolute inset-0 flex items-center justify-center transition-all duration-300',
            isDark ? 'opacity-0 rotate-90 scale-0' : 'opacity-100 rotate-0 scale-100'
          )}
          aria-hidden={isDark}
        >
          {sunIcon || <SunIcon className="h-4 w-4" />}
        </span>
        
        {/* Moon icon - visible in dark mode */}
        <span
          className={clsx(
            'absolute inset-0 flex items-center justify-center transition-all duration-300',
            isDark ? 'opacity-100 rotate-0 scale-100' : 'opacity-0 -rotate-90 scale-0'
          )}
          aria-hidden={!isDark}
        >
          {moonIcon || <MoonIcon className="h-4 w-4" />}
        </span>
      </span>
      
      {/* Screen reader text */}
      <span className="sr-only">
        {isDark ? 'Currently in dark mode' : 'Currently in light mode'}
      </span>
    </Button>
  );
}

// ============================================================================
// Dropdown Menu Item
// ============================================================================

interface ThemeOption {
  value: 'light' | 'dark' | 'system';
  label: string;
  icon: React.ReactNode;
  description?: string;
}

const THEME_OPTIONS: ThemeOption[] = [
  {
    value: 'light',
    label: 'Light',
    icon: <SunIcon className="h-4 w-4" />,
    description: 'Always use light mode',
  },
  {
    value: 'dark',
    label: 'Dark',
    icon: <MoonIcon className="h-4 w-4" />,
    description: 'Always use dark mode',
  },
  {
    value: 'system',
    label: 'System',
    icon: <SystemIcon className="h-4 w-4" />,
    description: 'Follow system preference',
  },
];

// ============================================================================
// Theme Dropdown
// ============================================================================

/**
 * Dropdown menu for selecting theme with light/dark/system options.
 * Includes icons, descriptions, and keyboard shortcut hints.
 */
export function ThemeDropdown({
  children,
  align = 'end',
  className,
}: ThemeDropdownProps) {
  const { theme, setTheme, resolvedTheme, isInitialized } = useTheme();
  
  if (!isInitialized) {
    return children;
  }
  
  return (
    <DropdownMenu>
      <DropdownMenuTrigger asChild>
        {children}
      </DropdownMenuTrigger>
      
      <DropdownMenuContent align={align} className={clsx('w-56', className)}>
        <DropdownMenuLabel className="text-xs font-semibold text-gray-500 dark:text-gray-400 uppercase tracking-wider">
          Appearance
        </DropdownMenuLabel>
        
        <DropdownMenuSeparator />
        
        {THEME_OPTIONS.map((option) => {
          const isSelected = theme === option.value;
          const isActive = resolvedTheme === option.value || 
                          (option.value === 'system' && theme === 'system');
          
          return (
            <DropdownMenuItem
              key={option.value}
              onClick={() => setTheme(option.value)}
              className={clsx(
                'flex items-center gap-3 cursor-pointer',
                isSelected && 'bg-gray-100 dark:bg-gray-800'
              )}
              aria-checked={isSelected}
              role="menuitemradio"
            >
              <span className={clsx(
                'flex items-center justify-center w-5 h-5 rounded',
                isActive 
                  ? 'text-primary dark:text-primary-light' 
                  : 'text-gray-500 dark:text-gray-400'
              )}>
                {option.icon}
              </span>
              
              <div className="flex flex-col flex-1">
                <span className={clsx(
                  'text-sm font-medium',
                  isSelected && 'text-gray-900 dark:text-gray-100'
                )}>
                  {option.label}
                </span>
                <span className="text-xs text-gray-500 dark:text-gray-400">
                  {option.description}
                </span>
              </div>
              
              {isSelected && (
                <Check className="h-4 w-4 text-primary dark:text-primary-light" />
              )}
            </DropdownMenuItem>
          );
        })}
        
        <DropdownMenuSeparator />
        
        <div className="px-2 py-1.5 text-xs text-gray-500 dark:text-gray-400">
          <DropdownMenuShortcut className="ml-0">Ctrl/⌘</DropdownMenuShortcut>
          <DropdownMenuShortcut className="ml-1">Shift</DropdownMenuShortcut>
          <DropdownMenuShortcut className="ml-1">L</DropdownMenuShortcut>
          <span className="ml-2">to toggle</span>
        </div>
      </DropdownMenuContent>
    </DropdownMenu>
  );
}

// ============================================================================
// Full Theme Toggle (with Dropdown)
// ============================================================================

/**
 * Complete theme toggle component with dropdown menu.
 * Shows current theme icon and opens dropdown on click.
 */
export function ThemeToggle({
  variant = 'outline',
  size = 'sm',
  showDropdown = true,
  className,
  sunIcon,
  moonIcon,
  systemIcon,
  ariaLabel,
  disabled,
  ...props
}: ThemeToggleProps) {
  const { theme, resolvedTheme, toggleTheme, isInitialized } = useTheme();
  
  // Enable keyboard shortcut
  useThemeShortcut();
  
  // Get current icon based on theme
  const getCurrentIcon = () => {
    if (theme === 'system') {
      return systemIcon || <SystemIcon className="h-4 w-4" />;
    }
    return resolvedTheme === 'dark' 
      ? (moonIcon || <MoonIcon className="h-4 w-4" />)
      : (sunIcon || <SunIcon className="h-4 w-4" />);
  };
  
  // Get ARIA label
  const getAriaLabel = () => {
    if (theme === 'system') {
      return `Using system theme (${resolvedTheme} mode). Click to change.`;
    }
    return resolvedTheme === 'dark' 
      ? 'Currently in dark mode. Click to toggle.'
      : 'Currently in light mode. Click to toggle.';
  };
  
  // Button content
  const buttonContent = (
    <Button
      variant={variant === 'minimal' ? 'ghost' : variant}
      size={size}
      className={clsx(
        'relative overflow-hidden gap-2',
        variant === 'minimal' && 'px-2',
        className
      )}
      disabled={disabled || !isInitialized}
      aria-label={ariaLabel || getAriaLabel()}
      {...props}
    >
      <span className="relative flex items-center justify-center w-4 h-4">
        {isInitialized ? (
          <>
            {/* Light icon */}
            <span
              className={clsx(
                'absolute inset-0 flex items-center justify-center transition-all duration-300',
                resolvedTheme === 'dark' && theme !== 'system'
                  ? 'opacity-0 rotate-90 scale-0' 
                  : 'opacity-100 rotate-0 scale-100'
              )}
            >
              {sunIcon || <SunIcon className="h-4 w-4" />}
            </span>
            
            {/* Dark icon */}
            <span
              className={clsx(
                'absolute inset-0 flex items-center justify-center transition-all duration-300',
                resolvedTheme === 'dark' && theme !== 'system'
                  ? 'opacity-100 rotate-0 scale-100' 
                  : 'opacity-0 -rotate-90 scale-0'
              )}
            >
              {moonIcon || <MoonIcon className="h-4 w-4" />}
            </span>
            
            {/* System icon (shown only when system is selected) */}
            {theme === 'system' && (
              <span className="absolute inset-0 flex items-center justify-center">
                {systemIcon || <SystemIcon className="h-4 w-4" />}
              </span>
            )}
          </>
        ) : (
          // Loading placeholder
          <span className="opacity-0">
            <SunIcon className="h-4 w-4" />
          </span>
        )}
      </span>
      
      {/* Optional: Show current theme label */}
      <span className="hidden sm:inline text-xs">
        {theme === 'system' ? 'Auto' : resolvedTheme === 'dark' ? 'Dark' : 'Light'}
      </span>
    </Button>
  );
  
  // If dropdown is disabled, just return the button with click handler
  if (!showDropdown) {
    return (
      <div onClick={toggleTheme} role="button" tabIndex={0}>
        {buttonContent}
      </div>
    );
  }
  
  // Return with dropdown
  return (
    <ThemeDropdown align="end">
      {buttonContent}
    </ThemeDropdown>
  );
}

// ============================================================================
// Exports
// ============================================================================

export default ThemeToggle;

// Named exports for individual components
export { SunIcon, MoonIcon, SystemIcon };

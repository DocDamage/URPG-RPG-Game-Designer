"use client";

/**
 * Branded Header Component
 * 
 * Application header with organization branding, navigation, and actions.
 */

import React, { useState } from 'react';
import { clsx } from 'clsx';
import { BrandedLogo } from './BrandedLogo';
import { BrandedButton } from './BrandedButton';
import { useBranding, useTheme, useOrganization } from '../useBranding';
import type { BrandedHeaderProps } from '../types';

export function BrandedHeader({
  className,
  navigation,
  actions,
  showNavigation = true,
  isFixed = true,
}: BrandedHeaderProps) {
  const { branding, isLoading } = useBranding();
  const { isDark, toggleTheme } = useTheme();
  const { name: orgName } = useOrganization();
  const [mobileMenuOpen, setMobileMenuOpen] = useState(false);

  // Loading skeleton
  if (isLoading) {
    return (
      <header
        className={clsx(
          'bg-[var(--sera-bg-surface)] border-b border-[var(--sera-border-light)]',
          isFixed && 'fixed top-0 left-0 right-0 z-50',
          className
        )}
      >
        <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
          <div className="flex items-center justify-between h-16">
            <div className="animate-pulse bg-gray-200 rounded w-32 h-8" />
          </div>
        </div>
      </header>
    );
  }

  return (
    <header
      className={clsx(
        'bg-[var(--sera-bg-surface)] border-b border-[var(--sera-border-light)]',
        isFixed && 'fixed top-0 left-0 right-0 z-50',
        className
      )}
      style={{
        backdropFilter: branding?.enableBlur ? 'blur(8px)' : undefined,
      }}
    >
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <div className="flex items-center justify-between h-16">
          {/* Logo */}
          <div className="flex items-center flex-shrink-0">
            <a href="/" className="flex items-center gap-3">
              <BrandedLogo height={40} />
              {branding?.tagline && (
                <span className="hidden md:block text-sm text-[var(--sera-text-muted)]">
                  {branding.tagline}
                </span>
              )}
            </a>
          </div>

          {/* Desktop Navigation */}
          {showNavigation && navigation && navigation.length > 0 && (
            <nav className="hidden md:flex items-center space-x-1">
              {navigation.map((item) => (
                <a
                  key={item.href}
                  href={item.href}
                  className={clsx(
                    'px-3 py-2 rounded-[var(--sera-radius-md)] text-sm font-medium',
                    'text-[var(--sera-text-secondary)] hover:text-[var(--sera-text-primary)]',
                    'hover:bg-[var(--sera-bg-elevated)]',
                    'transition-colors duration-200',
                    'flex items-center gap-2'
                  )}
                >
                  {item.icon && <span className="w-4 h-4">{item.icon}</span>}
                  {item.label}
                </a>
              ))}
            </nav>
          )}

          {/* Actions */}
          <div className="flex items-center gap-2">
            {/* Theme Toggle */}
            <button
              onClick={toggleTheme}
              className={clsx(
                'p-2 rounded-[var(--sera-radius-md)]',
                'text-[var(--sera-text-secondary)] hover:text-[var(--sera-text-primary)]',
                'hover:bg-[var(--sera-bg-elevated)]',
                'transition-colors duration-200'
              )}
              aria-label={isDark ? 'Switch to light mode' : 'Switch to dark mode'}
            >
              {isDark ? <SunIcon /> : <MoonIcon />}
            </button>

            {/* Custom Actions */}
            {actions && (
              <div className="hidden md:flex items-center gap-2">
                {actions}
              </div>
            )}

            {/* Mobile Menu Button */}
            {showNavigation && navigation && navigation.length > 0 && (
              <button
                onClick={() => setMobileMenuOpen(!mobileMenuOpen)}
                className={clsx(
                  'md:hidden p-2 rounded-[var(--sera-radius-md)]',
                  'text-[var(--sera-text-secondary)] hover:text-[var(--sera-text-primary)]',
                  'hover:bg-[var(--sera-bg-elevated)]',
                  'transition-colors duration-200'
                )}
                aria-label="Toggle menu"
              >
                {mobileMenuOpen ? <XIcon /> : <MenuIcon />}
              </button>
            )}
          </div>
        </div>

        {/* Mobile Navigation */}
        {mobileMenuOpen && showNavigation && navigation && (
          <div className="md:hidden border-t border-[var(--sera-border-light)]">
            <div className="py-2 space-y-1">
              {navigation.map((item) => (
                <a
                  key={item.href}
                  href={item.href}
                  className={clsx(
                    'block px-3 py-2 rounded-[var(--sera-radius-md)] text-base font-medium',
                    'text-[var(--sera-text-secondary)] hover:text-[var(--sera-text-primary)]',
                    'hover:bg-[var(--sera-bg-elevated)]',
                    'transition-colors duration-200',
                    'flex items-center gap-3'
                  )}
                  onClick={() => setMobileMenuOpen(false)}
                >
                  {item.icon && <span className="w-5 h-5">{item.icon}</span>}
                  {item.label}
                </a>
              ))}
            </div>
          </div>
        )}
      </div>
    </header>
  );
}

// Icon Components
function SunIcon() {
  return (
    <svg
      xmlns="http://www.w3.org/2000/svg"
      className="h-5 w-5"
      fill="none"
      viewBox="0 0 24 24"
      stroke="currentColor"
    >
      <path
        strokeLinecap="round"
        strokeLinejoin="round"
        strokeWidth={2}
        d="M12 3v1m0 16v1m9-9h-1M4 12H3m15.364 6.364l-.707-.707M6.343 6.343l-.707-.707m12.728 0l-.707.707M6.343 17.657l-.707.707M16 12a4 4 0 11-8 0 4 4 0 018 0z"
      />
    </svg>
  );
}

function MoonIcon() {
  return (
    <svg
      xmlns="http://www.w3.org/2000/svg"
      className="h-5 w-5"
      fill="none"
      viewBox="0 0 24 24"
      stroke="currentColor"
    >
      <path
        strokeLinecap="round"
        strokeLinejoin="round"
        strokeWidth={2}
        d="M20.354 24.354A9 9 0 018.646 3.646 9.003 9.003 0 0012 21a9.003 9.003 0 008.354-5.646z"
      />
    </svg>
  );
}

function MenuIcon() {
  return (
    <svg
      xmlns="http://www.w3.org/2000/svg"
      className="h-5 w-5"
      fill="none"
      viewBox="0 0 24 24"
      stroke="currentColor"
    >
      <path
        strokeLinecap="round"
        strokeLinejoin="round"
        strokeWidth={2}
        d="M4 6h16M4 12h16M4 18h16"
      />
    </svg>
  );
}

function XIcon() {
  return (
    <svg
      xmlns="http://www.w3.org/2000/svg"
      className="h-5 w-5"
      fill="none"
      viewBox="0 0 24 24"
      stroke="currentColor"
    >
      <path
        strokeLinecap="round"
        strokeLinejoin="round"
        strokeWidth={2}
        d="M6 18L18 6M6 6l12 12"
      />
    </svg>
  );
}

// Export default
export default BrandedHeader;

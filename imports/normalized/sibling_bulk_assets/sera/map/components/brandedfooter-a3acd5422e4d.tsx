"use client";

/**
 * Branded Footer Component
 * 
 * Application footer with organization branding, links, and version info.
 */

import React from 'react';
import { clsx } from 'clsx';
import { BrandedLogo } from './BrandedLogo';
import { useBranding, useOrganization } from '../useBranding';
import type { BrandedFooterProps } from '../types';

export function BrandedFooter({
  className,
  links,
  showVersion = true,
  children,
}: BrandedFooterProps) {
  const { branding, isLoading } = useBranding();
  const { name: orgName } = useOrganization();

  // Default links if none provided
  const footerLinks = links || [
    { label: 'Privacy', href: '/privacy' },
    { label: 'Terms', href: '/terms' },
    { label: 'Support', href: '/support' },
  ];

  const currentYear = new Date().getFullYear();

  if (isLoading) {
    return (
      <footer
        className={clsx(
          'bg-[var(--sera-bg-surface)] border-t border-[var(--sera-border-light)]',
          className
        )}
      >
        <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
          <div className="animate-pulse bg-gray-200 rounded h-8 w-32" />
        </div>
      </footer>
    );
  }

  return (
    <footer
      className={clsx(
        'bg-[var(--sera-bg-surface)] border-t border-[var(--sera-border-light)]',
        className
      )}
    >
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        {/* Main Footer Content */}
        <div className="py-8">
          <div className="flex flex-col md:flex-row md:items-center md:justify-between gap-6">
            {/* Logo and Info */}
            <div className="flex flex-col gap-2">
              <BrandedLogo height={32} fallbackToName />
              {branding?.tagline && (
                <p className="text-sm text-[var(--sera-text-muted)]">
                  {branding.tagline}
                </p>
              )}
            </div>

            {/* Navigation Links */}
            <nav className="flex flex-wrap items-center gap-x-6 gap-y-2">
              {footerLinks.map((link) => (
                <a
                  key={link.href}
                  href={link.href}
                  className={clsx(
                    'text-sm text-[var(--sera-text-secondary)] hover:text-[var(--sera-text-primary)]',
                    'transition-colors duration-200'
                  )}
                >
                  {link.label}
                </a>
              ))}
            </nav>
          </div>

          {/* Custom Content */}
          {children && (
            <div className="mt-6 pt-6 border-t border-[var(--sera-border-light)]">
              {children}
            </div>
          )}
        </div>

        {/* Bottom Bar */}
        <div className="py-4 border-t border-[var(--sera-border-light)]">
          <div className="flex flex-col sm:flex-row sm:items-center sm:justify-between gap-2">
            <p className="text-sm text-[var(--sera-text-muted)]">
              &copy; {currentYear} {orgName}. All rights reserved.
            </p>
            
            {showVersion && (
              <p className="text-xs text-[var(--sera-text-muted)]">
                Powered by{' '}
                <a
                  href="https://sera-platform.com"
                  target="_blank"
                  rel="noopener noreferrer"
                  className="hover:text-[var(--sera-text-secondary)] transition-colors"
                >
                  S.E.R.A.
                </a>
                {branding?.themeVersion && (
                  <span className="ml-2 opacity-50">
                    v{branding.themeVersion.slice(0, 8)}
                  </span>
                )}
              </p>
            )}
          </div>
        </div>
      </div>
    </footer>
  );
}

// Additional Footer Components for flexibility

/**
 * Footer Section - Group related links/content
 */
interface FooterSectionProps {
  title: string;
  children: React.ReactNode;
}

export function FooterSection({ title, children }: FooterSectionProps) {
  return (
    <div className="flex flex-col gap-3">
      <h3
        className="text-sm font-semibold text-[var(--sera-text-primary)] uppercase tracking-wider"
        style={{ fontFamily: 'var(--sera-font-heading)' }}
      >
        {title}
      </h3>
      <div className="flex flex-col gap-2">
        {children}
      </div>
    </div>
  );
}

/**
 * Footer Link - Styled link for footer
 */
interface FooterLinkProps {
  href: string;
  children: React.ReactNode;
  external?: boolean;
}

export function FooterLink({ href, children, external }: FooterLinkProps) {
  return (
    <a
      href={href}
      target={external ? '_blank' : undefined}
      rel={external ? 'noopener noreferrer' : undefined}
      className={clsx(
        'text-sm text-[var(--sera-text-secondary)] hover:text-[var(--sera-text-primary)]',
        'transition-colors duration-200',
        'flex items-center gap-1'
      )}
    >
      {children}
      {external && <ExternalLinkIcon />}
    </a>
  );
}

/**
 * Footer Grid - Multi-column footer layout
 */
interface FooterGridProps {
  children: React.ReactNode;
  columns?: 2 | 3 | 4;
}

export function FooterGrid({ children, columns = 4 }: FooterGridProps) {
  const columnClasses = {
    2: 'grid-cols-2',
    3: 'grid-cols-2 md:grid-cols-3',
    4: 'grid-cols-2 md:grid-cols-4',
  };

  return (
    <div className={clsx('grid gap-8', columnClasses[columns])}>
      {children}
    </div>
  );
}

// Icon Components
function ExternalLinkIcon() {
  return (
    <svg
      xmlns="http://www.w3.org/2000/svg"
      className="h-3 w-3"
      fill="none"
      viewBox="0 0 24 24"
      stroke="currentColor"
    >
      <path
        strokeLinecap="round"
        strokeLinejoin="round"
        strokeWidth={2}
        d="M10 6H6a2 2 0 00-2 2v10a2 2 0 002 2h10a2 2 0 002-2v-4M14 4h6m0 0v6m0-6L10 14"
      />
    </svg>
  );
}

// Export all components
export { BrandedFooter as default, FooterSection, FooterLink, FooterGrid };

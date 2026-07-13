"use client";

/**
 * Branded Logo Component
 * 
 * Displays the organization logo with fallback to organization name.
 * Automatically selects appropriate variant based on theme.
 */

import React, { useState, useCallback } from 'react';
import { clsx } from 'clsx';
import { useBranding, useTheme, useOrganization } from '../useBranding';
import type { BrandedLogoProps } from '../types';

export function BrandedLogo({
  variant,
  alt,
  width,
  height,
  className,
  onClick,
  fallbackToName = true,
}: BrandedLogoProps) {
  const { branding, isLoading, getLogoUrl } = useBranding();
  const { isDark } = useTheme();
  const { name: orgName } = useOrganization();
  const [imageError, setImageError] = useState(false);

  // Determine which variant to use
  const logoVariant = variant || (isDark ? 'dark' : 'default');
  const logoUrl = getLogoUrl(logoVariant);

  // Fallback chain: specified variant -> default -> light -> any available
  const getBestLogoUrl = useCallback((): string | undefined => {
    if (logoUrl && !imageError) return logoUrl;

    const logos = branding?.logos;
    if (!logos) return undefined;

    // Try different variants
    const variants = [
      logoVariant,
      'default',
      isDark ? 'light' : 'dark',
      'monochrome',
    ];

    for (const v of variants) {
      const logo = logos[v as keyof typeof logos];
      if (logo?.url) return logo.url;
    }

    return undefined;
  }, [logoUrl, logoVariant, isDark, branding?.logos, imageError]);

  const bestLogoUrl = getBestLogoUrl();
  const displayAlt = alt || `${orgName} logo`;

  // Handle image load error
  const handleError = () => {
    setImageError(true);
  };

  // Loading state
  if (isLoading) {
    return (
      <div
        className={clsx(
          'animate-pulse bg-gray-200 rounded',
          className
        )}
        style={{ width: width || 120, height: height || 40 }}
      />
    );
  }

  // Show logo image if available and no error
  if (bestLogoUrl && !imageError) {
    return (
      <img
        src={bestLogoUrl}
        alt={displayAlt}
        width={width}
        height={height}
        className={clsx(
          'object-contain transition-opacity',
          onClick && 'cursor-pointer',
          className
        )}
        onClick={onClick}
        onError={handleError}
        style={{
          maxWidth: width ? `${width}px` : '100%',
          maxHeight: height ? `${height}px` : '100%',
        }}
      />
    );
  }

  // Fallback to organization name
  if (fallbackToName) {
    return (
      <span
        className={clsx(
          'font-semibold text-lg text-[var(--sera-text-primary)]',
          onClick && 'cursor-pointer',
          className
        )}
        onClick={onClick}
        style={{
          fontFamily: 'var(--sera-font-heading)',
        }}
      >
        {orgName}
      </span>
    );
  }

  // Return null if no fallback
  return null;
}

// Export default for convenience
export default BrandedLogo;

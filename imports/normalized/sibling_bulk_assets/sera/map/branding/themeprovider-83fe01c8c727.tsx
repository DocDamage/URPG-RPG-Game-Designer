"use client";

/**
 * Theme Provider
 * 
 * React context provider for theme and branding configuration.
 * Handles dynamic theme loading, CSS variable injection, and theme switching.
 */

import React, {
  createContext,
  useContext,
  useEffect,
  useState,
  useCallback,
  useMemo,
  ReactNode,
} from 'react';
import {
  ThemeContextValue,
  BrandingContextValue,
  OrganizationBranding,
  ColorPalette,
  OrganizationThemeResponse,
  LogoVariant,
} from './types';
import { brandingApi } from '../api/branding';

// ============================================================================
// Contexts
// ============================================================================

export const ThemeContext = createContext<ThemeContextValue | null>(null);
export const BrandingContext = createContext<BrandingContextValue | null>(null);

// ============================================================================
// Props
// ============================================================================

interface BrandingProviderProps {
  children: ReactNode;
  /** Organization ID to load branding for */
  organizationId: string;
  /** Initial branding data (optional, for SSR) */
  initialBranding?: OrganizationBranding;
  /** Whether to enable system theme detection */
  enableSystemTheme?: boolean;
  /** Storage key for theme preference */
  storageKey?: string;
  /** CSS selector for theme application */
  themeSelector?: string;
  /** Callback when theme changes */
  onThemeChange?: (theme: 'light' | 'dark') => void;
}

// ============================================================================
// Default Colors (fallback)
// ============================================================================

const DEFAULT_LIGHT_COLORS: ColorPalette = {
  primary: '#0ea5e9',
  secondary: '#64748b',
  accent: '#8b5cf6',
  background: {
    main: '#ffffff',
    surface: '#f8fafc',
    elevated: '#ffffff',
  },
  text: {
    primary: '#0f172a',
    secondary: '#475569',
    muted: '#94a3b8',
    inverse: '#ffffff',
  },
  semantic: {
    success: '#22c55e',
    warning: '#f59e0b',
    error: '#ef4444',
    info: '#3b82f6',
  },
  border: {
    light: '#e2e8f0',
    default: '#cbd5e1',
    strong: '#94a3b8',
  },
};

const DEFAULT_DARK_COLORS: ColorPalette = {
  primary: '#38bdf8',
  secondary: '#94a3b8',
  accent: '#a78bfa',
  background: {
    main: '#0f172a',
    surface: '#1e293b',
    elevated: '#334155',
  },
  text: {
    primary: '#f8fafc',
    secondary: '#cbd5e1',
    muted: '#64748b',
    inverse: '#0f172a',
  },
  semantic: {
    success: '#4ade80',
    warning: '#fbbf24',
    error: '#f87171',
    info: '#60a5fa',
  },
  border: {
    light: '#334155',
    default: '#475569',
    strong: '#64748b',
  },
};

// ============================================================================
// Branding Provider
// ============================================================================

export function BrandingProvider({
  children,
  organizationId,
  initialBranding,
  enableSystemTheme = true,
  storageKey = 'sera-theme',
  themeSelector = 'html',
  onThemeChange,
}: BrandingProviderProps) {
  // Branding state
  const [branding, setBranding] = useState<OrganizationBranding | null>(
    initialBranding || null
  );
  const [isLoading, setIsLoading] = useState(!initialBranding);
  const [error, setError] = useState<Error | null>(null);

  // Theme state
  const [colorScheme, setColorScheme] = useState<'light' | 'dark' | 'auto'>(
    initialBranding?.colorScheme || 'light'
  );
  const [resolvedTheme, setResolvedTheme] = useState<'light' | 'dark'>('light');
  const [systemTheme, setSystemTheme] = useState<'light' | 'dark'>('light');

  // ==========================================================================
  // Fetch Branding
  // ==========================================================================

  const fetchBranding = useCallback(async () => {
    if (!organizationId) return;

    setIsLoading(true);
    setError(null);

    try {
      const data = await brandingApi.fetchOrganizationBranding(organizationId);
      setBranding(data);
      setColorScheme(data.colorScheme);
    } catch (err) {
      console.error('Failed to fetch branding:', err);
      setError(err instanceof Error ? err : new Error('Failed to fetch branding'));
    } finally {
      setIsLoading(false);
    }
  }, [organizationId]);

  // Initial fetch
  useEffect(() => {
    if (!initialBranding) {
      fetchBranding();
    }
  }, [fetchBranding, initialBranding]);

  // ==========================================================================
  // System Theme Detection
  // ==========================================================================

  useEffect(() => {
    if (!enableSystemTheme) return;

    const mediaQuery = window.matchMedia('(prefers-color-scheme: dark)');
    
    const handleChange = (e: MediaQueryListEvent | MediaQueryList) => {
      setSystemTheme(e.matches ? 'dark' : 'light');
    };

    // Set initial value
    handleChange(mediaQuery);

    // Listen for changes
    mediaQuery.addEventListener('change', handleChange);
    return () => mediaQuery.removeEventListener('change', handleChange);
  }, [enableSystemTheme]);

  // ==========================================================================
  // Theme Resolution
  // ==========================================================================

  useEffect(() => {
    let theme: 'light' | 'dark';

    if (colorScheme === 'auto') {
      theme = systemTheme;
    } else {
      theme = colorScheme;
    }

    setResolvedTheme(theme);

    // Apply theme to DOM
    const element = document.querySelector(themeSelector);
    if (element) {
      element.setAttribute('data-theme', theme);
      element.classList.remove('light', 'dark');
      element.classList.add(theme);
    }

    // Store preference
    localStorage.setItem(storageKey, theme);

    // Callback
    onThemeChange?.(theme);
  }, [colorScheme, systemTheme, themeSelector, storageKey, onThemeChange]);

  // ==========================================================================
  // CSS Variable Injection
  // ==========================================================================

  useEffect(() => {
    if (!branding) return;

    const colors = resolvedTheme === 'dark' ? branding.darkColors : branding.lightColors;
    const root = document.documentElement;

    // Helper to set CSS variable
    const setVar = (name: string, value: string) => {
      root.style.setProperty(`--sera-${name}`, value);
    };

    // Primary colors
    setVar('primary', colors.primary);
    setVar('primary-hover', adjustColor(colors.primary, resolvedTheme === 'dark' ? 10 : -10));
    setVar('primary-active', adjustColor(colors.primary, resolvedTheme === 'dark' ? 15 : -15));
    setVar('primary-light', adjustColor(colors.primary, 40));
    setVar('primary-dark', adjustColor(colors.primary, -20));

    // Secondary
    setVar('secondary', colors.secondary);
    setVar('secondary-hover', adjustColor(colors.secondary, resolvedTheme === 'dark' ? 10 : -10));

    // Accent
    setVar('accent', colors.accent);
    setVar('accent-hover', adjustColor(colors.accent, resolvedTheme === 'dark' ? 10 : -10));

    // Background
    setVar('bg-main', colors.background.main);
    setVar('bg-surface', colors.background.surface);
    setVar('bg-elevated', colors.background.elevated);

    // Text
    setVar('text-primary', colors.text.primary);
    setVar('text-secondary', colors.text.secondary);
    setVar('text-muted', colors.text.muted);
    setVar('text-inverse', colors.text.inverse);

    // Semantic
    setVar('success', colors.semantic.success);
    setVar('success-light', adjustColor(colors.semantic.success, 45));
    setVar('warning', colors.semantic.warning);
    setVar('warning-light', adjustColor(colors.semantic.warning, 40));
    setVar('error', colors.semantic.error);
    setVar('error-light', adjustColor(colors.semantic.error, 40));
    setVar('info', colors.semantic.info);
    setVar('info-light', adjustColor(colors.semantic.info, 45));

    // Border
    setVar('border-light', colors.border.light);
    setVar('border-default', colors.border.default);
    setVar('border-strong', colors.border.strong);

    // Typography
    const { typography } = branding;
    setVar('font-heading', getFontFamily(typography.headings));
    setVar('font-body', getFontFamily(typography.body));
    setVar('font-mono', getFontFamily(typography.mono));

    // Font sizes
    const baseSize = typography.body.baseSize;
    const ratio = typography.scaleRatio;
    setVar('text-xs', `${(baseSize * Math.pow(ratio, -2)).toFixed(2)}px`);
    setVar('text-sm', `${(baseSize * Math.pow(ratio, -1)).toFixed(2)}px`);
    setVar('text-base', `${baseSize}px`);
    setVar('text-lg', `${(baseSize * Math.pow(ratio, 1)).toFixed(2)}px`);
    setVar('text-xl', `${(baseSize * Math.pow(ratio, 2)).toFixed(2)}px`);
    setVar('text-2xl', `${(baseSize * Math.pow(ratio, 3)).toFixed(2)}px`);
    setVar('text-3xl', `${(baseSize * Math.pow(ratio, 4)).toFixed(2)}px`);
    setVar('text-4xl', `${(baseSize * Math.pow(ratio, 5)).toFixed(2)}px`);

    // Border radius
    const radiusMap: Record<string, string> = {
      none: '0px',
      small: '4px',
      medium: '8px',
      large: '12px',
      full: '9999px',
    };
    const baseRadius = typeof branding.borderRadius === 'number'
      ? `${branding.borderRadius}px`
      : radiusMap[branding.borderRadius] || '8px';

    setVar('radius-none', '0px');
    setVar('radius-sm', `calc(${baseRadius} * 0.5)`);
    setVar('radius-md', baseRadius);
    setVar('radius-lg', `calc(${baseRadius} * 1.5)`);
    setVar('radius-xl', `calc(${baseRadius} * 2)`);
    setVar('radius-full', '9999px');

    // Shadows
    const shadows = getShadows(branding.shadowIntensity);
    setVar('shadow-sm', shadows.sm);
    setVar('shadow-md', shadows.md);
    setVar('shadow-lg', shadows.lg);
    setVar('shadow-xl', shadows.xl);

    // Spacing
    const spacingConfig = {
      compact: 0.25,
      comfortable: 0.25,
      spacious: 0.5,
    }[branding.spacing] || 0.25;

    const spacingValues = [0, 0.5, 1, 2, 3, 4, 5, 6, 8, 10, 12, 16, 20, 24, 32, 40, 48, 64, 80, 96];
    spacingValues.forEach(val => {
      setVar(`space-${val}`, `${val * spacingConfig}rem`);
    });

    // Load custom CSS if provided
    if (branding.customCssUrl) {
      loadCustomCSS(branding.customCssUrl);
    }

  }, [branding, resolvedTheme]);

  // ==========================================================================
  // Helpers
  // ==========================================================================

  const adjustColor = (hex: string, percent: number): string => {
    const num = parseInt(hex.replace('#', ''), 16);
    const amt = Math.round(2.55 * percent);
    const R = Math.max(0, Math.min(255, (num >> 16) + amt));
    const G = Math.max(0, Math.min(255, ((num >> 8) & 0x00ff) + amt));
    const B = Math.max(0, Math.min(255, (num & 0x0000ff) + amt));
    return `#${(0x1000000 + R * 0x10000 + G * 0x100 + B).toString(16).slice(1)}`;
  };

  const getFontFamily = (config: { family: string; customFamily?: string }): string => {
    if (config.family === 'custom' && config.customFamily) {
      return config.customFamily;
    }
    const fallbacks: Record<string, string> = {
      system: 'system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif',
      serif: 'Georgia, Cambria, "Times New Roman", Times, serif',
      'sans-serif': 'ui-sans-serif, system-ui, sans-serif',
      monospace: 'ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, monospace',
    };
    return fallbacks[config.family] || fallbacks.system;
  };

  const getShadows = (intensity: string): { sm: string; md: string; lg: string; xl: string } => {
    const shadowMap: Record<string, { sm: string; md: string; lg: string; xl: string }> = {
      none: {
        sm: 'none',
        md: 'none',
        lg: 'none',
        xl: 'none',
      },
      light: {
        sm: '0 1px 2px 0 rgb(0 0 0 / 0.05)',
        md: '0 4px 6px -1px rgb(0 0 0 / 0.1), 0 2px 4px -2px rgb(0 0 0 / 0.1)',
        lg: '0 10px 15px -3px rgb(0 0 0 / 0.1), 0 4px 6px -4px rgb(0 0 0 / 0.1)',
        xl: '0 20px 25px -5px rgb(0 0 0 / 0.1), 0 8px 10px -6px rgb(0 0 0 / 0.1)',
      },
      medium: {
        sm: '0 1px 3px 0 rgb(0 0 0 / 0.1), 0 1px 2px -1px rgb(0 0 0 / 0.1)',
        md: '0 4px 6px -1px rgb(0 0 0 / 0.15), 0 2px 4px -2px rgb(0 0 0 / 0.15)',
        lg: '0 10px 15px -3px rgb(0 0 0 / 0.15), 0 4px 6px -4px rgb(0 0 0 / 0.15)',
        xl: '0 20px 25px -5px rgb(0 0 0 / 0.15), 0 8px 10px -6px rgb(0 0 0 / 0.15)',
      },
      heavy: {
        sm: '0 1px 3px 0 rgb(0 0 0 / 0.2), 0 1px 2px -1px rgb(0 0 0 / 0.2)',
        md: '0 4px 6px -1px rgb(0 0 0 / 0.2), 0 2px 4px -2px rgb(0 0 0 / 0.2)',
        lg: '0 10px 15px -3px rgb(0 0 0 / 0.25), 0 4px 6px -4px rgb(0 0 0 / 0.25)',
        xl: '0 20px 25px -5px rgb(0 0 0 / 0.25), 0 8px 10px -6px rgb(0 0 0 / 0.25)',
      },
    };
    return shadowMap[intensity] || shadowMap.medium;
  };

  const loadCustomCSS = (url: string) => {
    // Remove existing custom CSS
    const existing = document.getElementById('sera-custom-css');
    if (existing) {
      existing.remove();
    }

    // Add new custom CSS
    const link = document.createElement('link');
    link.id = 'sera-custom-css';
    link.rel = 'stylesheet';
    link.href = url;
    document.head.appendChild(link);
  };

  // ==========================================================================
  // Context Values
  // ==========================================================================

  const themeContextValue = useMemo<ThemeContextValue>(
    () => ({
      theme: resolvedTheme,
      isDark: resolvedTheme === 'dark',
      toggleTheme: () => {
        setColorScheme(prev => (prev === 'light' ? 'dark' : 'light'));
      },
      setTheme: (theme: 'light' | 'dark') => setColorScheme(theme),
      colorScheme,
      isLoading,
    }),
    [resolvedTheme, colorScheme, isLoading]
  );

  const getLogoUrl = useCallback(
    (variant: LogoVariant = 'default'): string | undefined => {
      const logo = branding?.logos[variant];
      return logo?.url;
    },
    [branding]
  );

  const getCssVariable = useCallback((name: string): string => {
    if (typeof window === 'undefined') return '';
    return getComputedStyle(document.documentElement)
      .getPropertyValue(`--sera-${name}`)
      .trim();
  }, []);

  const brandingContextValue = useMemo<BrandingContextValue>(
    () => ({
      branding,
      isLoading,
      error,
      refetch: fetchBranding,
      getLogoUrl,
      getCssVariable,
      colors: resolvedTheme === 'dark' && branding?.darkColors
        ? branding.darkColors
        : branding?.lightColors || DEFAULT_LIGHT_COLORS,
    }),
    [branding, isLoading, error, fetchBranding, getLogoUrl, getCssVariable, resolvedTheme]
  );

  return (
    <BrandingContext.Provider value={brandingContextValue}>
      <ThemeContext.Provider value={themeContextValue}>
        {children}
      </ThemeContext.Provider>
    </BrandingContext.Provider>
  );
}

// ============================================================================
// Export default
// ============================================================================

export default BrandingProvider;

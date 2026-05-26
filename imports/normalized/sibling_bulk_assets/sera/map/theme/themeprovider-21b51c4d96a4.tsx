"use client";

/**
 * Theme Provider
 * 
 * Comprehensive React Context provider for theme state management.
 * Supports 'light', 'dark', and 'system' modes with persistence and smooth transitions.
 * 
 * Features:
 * - System preference detection (prefers-color-scheme)
 * - LocalStorage persistence
 * - Smooth transitions between modes
 * - Integration with existing branding/theme system
 * - Accessibility support (reduced motion)
 * - Custom event dispatching for theme changes
 */

import React, {
  createContext,
  useContext,
  useEffect,
  useState,
  useCallback,
  useMemo,
  useRef,
} from 'react';
import { ThemeContextValue, ThemeProviderProps, ThemeMode, ResolvedTheme } from './types';

// ============================================================================
// Context
// ============================================================================

const ThemeContext = createContext<ThemeContextValue | undefined>(undefined);

// ============================================================================
// Constants
// ============================================================================

const DEFAULT_STORAGE_KEY = 'sera-theme-preference';
const MEDIA_QUERY = '(prefers-color-scheme: dark)';
const REDUCED_MOTION_QUERY = '(prefers-reduced-motion: reduce)';

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * Get initial theme from localStorage or default
 */
const getInitialTheme = (storageKey: string, defaultTheme: ThemeMode): ThemeMode => {
  if (typeof window === 'undefined') return defaultTheme;
  
  try {
    const stored = localStorage.getItem(storageKey) as ThemeMode | null;
    if (stored && ['light', 'dark', 'system'].includes(stored)) {
      return stored;
    }
  } catch (e) {
    // localStorage might be disabled or unavailable
    console.warn('Failed to read theme from localStorage:', e);
  }
  
  return defaultTheme;
};

/**
 * Get system theme preference
 */
const getSystemTheme = (): ResolvedTheme => {
  if (typeof window === 'undefined') return 'light';
  return window.matchMedia(MEDIA_QUERY).matches ? 'dark' : 'light';
};

/**
 * Resolve theme based on current mode and system preference
 */
const resolveTheme = (theme: ThemeMode, systemTheme: ResolvedTheme): ResolvedTheme => {
  if (theme === 'system') {
    return systemTheme;
  }
  return theme;
};

/**
 * Disable/enable CSS transitions temporarily
 */
const disableTransitions = (disable: boolean, duration: number): (() => void) => {
  if (typeof document === 'undefined') return () => {};
  
  const css = document.createElement('style');
  css.textContent = `
    *, *::before, *::after {
      transition: none !important;
      animation: none !important;
    }
  `;
  
  if (disable) {
    document.head.appendChild(css);
  }
  
  return () => {
    if (disable && css.parentNode) {
      // Force a reflow to ensure styles are applied
      const _ = window.getComputedStyle(document.documentElement).opacity;
      css.parentNode.removeChild(css);
    }
  };
};

// ============================================================================
// Theme Provider Component
// ============================================================================

export function ThemeProvider({
  children,
  defaultTheme = 'system',
  storageKey = DEFAULT_STORAGE_KEY,
  enableSystem = true,
  disableTransitionOnChange = false,
  attribute = 'class',
  onThemeChange,
  enableSmoothTransitions = true,
  transitionDuration = 300,
}: ThemeProviderProps) {
  // Theme state
  const [theme, setThemeState] = useState<ThemeMode>(defaultTheme);
  const [resolvedTheme, setResolvedTheme] = useState<ResolvedTheme>(
    defaultTheme === 'system' ? 'light' : defaultTheme
  );
  const [systemTheme, setSystemTheme] = useState<ResolvedTheme | null>(null);
  const [isInitialized, setIsInitialized] = useState(false);
  const [reducedMotion, setReducedMotion] = useState(false);
  
  // Refs
  const previousTheme = useRef<ThemeMode>(defaultTheme);
  const previousResolvedTheme = useRef<ResolvedTheme>(
    defaultTheme === 'system' ? 'light' : defaultTheme
  );

  // ==========================================================================
  // Initialization
  // ==========================================================================
  
  useEffect(() => {
    // Get stored theme preference
    const initialTheme = getInitialTheme(storageKey, defaultTheme);
    const initialSystemTheme = getSystemTheme();
    const initialResolved = resolveTheme(initialTheme, initialSystemTheme);
    
    setThemeState(initialTheme);
    setSystemTheme(initialSystemTheme);
    setResolvedTheme(initialResolved);
    
    // Check for reduced motion preference
    const motionQuery = window.matchMedia(REDUCED_MOTION_QUERY);
    setReducedMotion(motionQuery.matches);
    
    const handleMotionChange = (e: MediaQueryListEvent) => {
      setReducedMotion(e.matches);
    };
    
    motionQuery.addEventListener('change', handleMotionChange);
    
    // Apply initial theme to document
    applyTheme(initialResolved);
    
    setIsInitialized(true);
    
    return () => {
      motionQuery.removeEventListener('change', handleMotionChange);
    };
  }, [storageKey, defaultTheme]);

  // ==========================================================================
  // System Theme Detection
  // ==========================================================================
  
  useEffect(() => {
    if (!enableSystem || typeof window === 'undefined') return;
    
    const mediaQuery = window.matchMedia(MEDIA_QUERY);
    
    const handleChange = (e: MediaQueryListEvent) => {
      const newSystemTheme: ResolvedTheme = e.matches ? 'dark' : 'light';
      setSystemTheme(newSystemTheme);
      
      // If using system theme, update resolved theme
      if (theme === 'system') {
        updateResolvedTheme('system', newSystemTheme);
      }
    };
    
    mediaQuery.addEventListener('change', handleChange);
    
    // Set initial system theme
    setSystemTheme(mediaQuery.matches ? 'dark' : 'light');
    
    return () => mediaQuery.removeEventListener('change', handleChange);
  }, [enableSystem, theme]);

  // ==========================================================================
  // Theme Application
  // ==========================================================================
  
  const applyTheme = useCallback((newResolvedTheme: ResolvedTheme) => {
    if (typeof document === 'undefined') return;
    
    const root = document.documentElement;
    
    // Apply class-based theme
    if (attribute === 'class') {
      root.classList.remove('light', 'dark');
      root.classList.add(newResolvedTheme);
    } else if (typeof attribute === 'string') {
      // Apply custom attribute
      root.setAttribute(attribute, newResolvedTheme);
    }
    
    // Set data-theme attribute for CSS selectors
    root.setAttribute('data-theme', newResolvedTheme);
    
    // Update color scheme for browser UI
    root.style.colorScheme = newResolvedTheme;
    
    // Update meta theme-color for mobile browsers
    const metaThemeColor = document.querySelector('meta[name="theme-color"]');
    if (metaThemeColor) {
      metaThemeColor.setAttribute(
        'content',
        newResolvedTheme === 'dark' ? '#0f172a' : '#ffffff'
      );
    }
  }, [attribute]);

  const updateResolvedTheme = useCallback((
    newTheme: ThemeMode,
    currentSystemTheme: ResolvedTheme
  ) => {
    const newResolved = resolveTheme(newTheme, currentSystemTheme);
    
    // Store previous values for event
    const prevTheme = previousTheme.current;
    const prevResolved = previousResolvedTheme.current;
    
    // Disable transitions if needed
    const enableTransitions = !disableTransitionOnChange && 
                              enableSmoothTransitions && 
                              !reducedMotion;
    
    let cleanupTransitions: (() => void) | undefined;
    
    if (!enableTransitions && isInitialized) {
      cleanupTransitions = disableTransitions(true, transitionDuration);
    }
    
    // Apply theme
    applyTheme(newResolved);
    setResolvedTheme(newResolved);
    
    // Update refs
    previousTheme.current = newTheme;
    previousResolvedTheme.current = newResolved;
    
    // Re-enable transitions
    if (cleanupTransitions) {
      setTimeout(() => {
        cleanupTransitions?.();
      }, transitionDuration);
    }
    
    // Dispatch custom events
    const eventDetail = {
      theme: newTheme,
      resolvedTheme: newResolved,
      previousTheme: prevTheme,
      previousResolvedTheme: prevResolved,
    };
    
    window.dispatchEvent(new CustomEvent('themechange', { detail: eventDetail }));
    window.dispatchEvent(new CustomEvent('sera:themechange', { detail: eventDetail }));
    
    // Call callback
    onThemeChange?.(newTheme, newResolved);
  }, [
    applyTheme,
    disableTransitionOnChange,
    enableSmoothTransitions,
    reducedMotion,
    transitionDuration,
    isInitialized,
    onThemeChange,
  ]);

  // ==========================================================================
  // Public API
  // ==========================================================================
  
  const setTheme = useCallback((newTheme: ThemeMode) => {
    // Persist to localStorage
    try {
      localStorage.setItem(storageKey, newTheme);
    } catch (e) {
      console.warn('Failed to save theme to localStorage:', e);
    }
    
    setThemeState(newTheme);
    
    // Update resolved theme based on system preference if needed
    const currentSystem = systemTheme ?? getSystemTheme();
    updateResolvedTheme(newTheme, currentSystem);
  }, [storageKey, systemTheme, updateResolvedTheme]);

  const toggleTheme = useCallback((cycleSystem = false) => {
    setThemeState(currentTheme => {
      let newTheme: ThemeMode;
      
      if (cycleSystem) {
        // Cycle: light -> dark -> system -> light
        if (currentTheme === 'light') {
          newTheme = 'dark';
        } else if (currentTheme === 'dark') {
          newTheme = 'system';
        } else {
          newTheme = 'light';
        }
      } else {
        // Simple toggle: light <-> dark
        newTheme = currentTheme === 'light' ? 'dark' : 'light';
      }
      
      // Persist to localStorage
      try {
        localStorage.setItem(storageKey, newTheme);
      } catch (e) {
        console.warn('Failed to save theme to localStorage:', e);
      }
      
      // Update resolved theme
      const currentSystem = systemTheme ?? getSystemTheme();
      updateResolvedTheme(newTheme, currentSystem);
      
      return newTheme;
    });
  }, [storageKey, systemTheme, updateResolvedTheme]);

  // ==========================================================================
  // Context Value
  // ==========================================================================
  
  const contextValue = useMemo<ThemeContextValue>(
    () => ({
      theme,
      resolvedTheme,
      isDark: resolvedTheme === 'dark',
      isSystem: theme === 'system',
      setTheme,
      toggleTheme,
      systemTheme,
      isInitialized,
    }),
    [theme, resolvedTheme, systemTheme, isInitialized, setTheme, toggleTheme]
  );

  return (
    <ThemeContext.Provider value={contextValue}>
      {children}
    </ThemeContext.Provider>
  );
}

// ============================================================================
// Hook
// ============================================================================

export function useTheme(): ThemeContextValue {
  const context = useContext(ThemeContext);
  if (context === undefined) {
    throw new Error('useTheme must be used within a ThemeProvider');
  }
  return context;
}

// ============================================================================
// Exports
// ============================================================================

export { ThemeContext };
export default ThemeProvider;

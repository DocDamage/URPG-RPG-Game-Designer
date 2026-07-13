'use client';

import { createContext, useContext, useEffect, useCallback, ReactNode } from 'react';
import { usePathname, useSearchParams } from 'next/navigation';

/**
 * Analytics Context Type
 */
interface AnalyticsContextType {
  trackEvent: (eventName: string, properties?: Record<string, any>) => void;
  trackPageView: (path: string) => void;
  identify: (userId: string, traits?: Record<string, any>) => void;
  reset: () => void;
}

const AnalyticsContext = createContext<AnalyticsContextType | undefined>(undefined);

/**
 * Hook to use analytics
 */
export function useAnalytics() {
  const context = useContext(AnalyticsContext);
  if (context === undefined) {
    // Return no-op functions if analytics is not configured
    return {
      trackEvent: () => {},
      trackPageView: () => {},
      identify: () => {},
      reset: () => {},
    };
  }
  return context;
}

interface AnalyticsProviderProps {
  children: ReactNode;
  apiKey?: string;
  enabled?: boolean;
}

/**
 * Analytics Provider Component
 * 
 * Handles page view tracking and event tracking for analytics.
 * Supports multiple analytics providers (Google Analytics, Segment, etc.)
 * 
 * Privacy-focused: Only tracks in production, respects DNT header
 */
export function AnalyticsProvider({ 
  children, 
  apiKey,
  enabled = process.env.NODE_ENV === 'production'
}: AnalyticsProviderProps) {
  const pathname = usePathname();
  const searchParams = useSearchParams();

  /**
   * Check if tracking is allowed
   */
  const isTrackingAllowed = useCallback(() => {
    // Check for Do Not Track
    if (typeof window !== 'undefined' && window.navigator.doNotTrack === '1') {
      return false;
    }
    
    // Check for GDPR consent
    const consent = localStorage.getItem('analytics-consent');
    if (consent === 'denied') {
      return false;
    }
    
    return enabled;
  }, [enabled]);

  /**
   * Track a page view
   */
  const trackPageView = useCallback((path: string) => {
    if (!isTrackingAllowed()) return;

    // Google Analytics
    if (typeof window !== 'undefined' && (window as any).gtag) {
      (window as any).gtag('config', apiKey, {
        page_path: path,
        transport_type: 'beacon',
      });
    }

    // Plausible Analytics (privacy-focused alternative)
    if (typeof window !== 'undefined' && (window as any).plausible) {
      (window as any).plausible('pageview', { u: window.location.href });
    }

    // Custom analytics endpoint
    if (process.env.NEXT_PUBLIC_ANALYTICS_ENDPOINT) {
      fetch(process.env.NEXT_PUBLIC_ANALYTICS_ENDPOINT, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          type: 'pageview',
          path,
          timestamp: new Date().toISOString(),
          referrer: document.referrer,
        }),
        keepalive: true,
      }).catch(() => {
        // Silently fail - don't break the app for analytics
      });
    }
  }, [apiKey, isTrackingAllowed]);

  /**
   * Track a custom event
   */
  const trackEvent = useCallback((eventName: string, properties?: Record<string, any>) => {
    if (!isTrackingAllowed()) return;

    // Sanitize properties to avoid sending PII
    const sanitizedProperties = sanitizeProperties(properties);

    // Google Analytics
    if (typeof window !== 'undefined' && (window as any).gtag) {
      (window as any).gtag('event', eventName, sanitizedProperties);
    }

    // Plausible Analytics
    if (typeof window !== 'undefined' && (window as any).plausible) {
      (window as any).plausible(eventName, { props: sanitizedProperties });
    }

    // Console log in development
    if (process.env.NODE_ENV === 'development') {
      console.log('[Analytics]', eventName, sanitizedProperties);
    }
  }, [isTrackingAllowed]);

  /**
   * Identify a user (use sparingly for privacy)
   */
  const identify = useCallback((userId: string, traits?: Record<string, any>) => {
    if (!isTrackingAllowed()) return;

    // Only identify if explicitly consented
    const consent = localStorage.getItem('analytics-consent');
    if (consent !== 'granted') return;

    // Hash the userId for privacy
    const hashedUserId = hashString(userId);

    if (typeof window !== 'undefined' && (window as any).gtag) {
      (window as any).gtag('config', apiKey, {
        user_id: hashedUserId,
      });
    }
  }, [apiKey, isTrackingAllowed]);

  /**
   * Reset analytics identity
   */
  const reset = useCallback(() => {
    if (typeof window !== 'undefined' && (window as any).gtag) {
      // Clear user data
    }
    localStorage.removeItem('analytics-consent');
  }, []);

  /**
   * Sanitize properties to remove potential PII
   */
  function sanitizeProperties(properties?: Record<string, any>): Record<string, any> {
    if (!properties) return {};

    const piiKeys = ['email', 'phone', 'name', 'ssn', 'dob', 'address', 'patient', 'individual'];
    const sanitized: Record<string, any> = {};

    for (const [key, value] of Object.entries(properties)) {
      // Skip known PII keys
      if (piiKeys.some(piiKey => key.toLowerCase().includes(piiKey))) {
        sanitized[key] = '[REDACTED]';
      } else {
        sanitized[key] = value;
      }
    }

    return sanitized;
  }

  /**
   * Hash a string for privacy
   */
  function hashString(str: string): string {
    let hash = 0;
    for (let i = 0; i < str.length; i++) {
      const char = str.charCodeAt(i);
      hash = ((hash << 5) - hash) + char;
      hash = hash & hash;
    }
    return hash.toString(16);
  }

  // Track page views on route changes
  useEffect(() => {
    if (pathname) {
      const url = pathname + (searchParams?.toString() ? `?${searchParams.toString()}` : '');
      trackPageView(url);
    }
  }, [pathname, searchParams, trackPageView]);

  const value = {
    trackEvent,
    trackPageView,
    identify,
    reset,
  };

  return (
    <AnalyticsContext.Provider value={value}>
      {children}
    </AnalyticsContext.Provider>
  );
}

/**
 * Analytics Consent Banner
 * Shows a banner for GDPR compliance
 */
export function AnalyticsConsentBanner() {
  const [showBanner, setShowBanner] = useState(false);

  useEffect(() => {
    const consent = localStorage.getItem('analytics-consent');
    if (consent === null) {
      setShowBanner(true);
    }
  }, []);

  const handleAccept = () => {
    localStorage.setItem('analytics-consent', 'granted');
    setShowBanner(false);
  };

  const handleDecline = () => {
    localStorage.setItem('analytics-consent', 'denied');
    setShowBanner(false);
  };

  if (!showBanner) return null;

  return (
    <div className="fixed bottom-0 left-0 right-0 bg-white border-t shadow-lg p-4 z-50">
      <div className="max-w-7xl mx-auto flex flex-col sm:flex-row items-center justify-between gap-4">
        <div className="text-sm text-gray-600">
          <p className="font-medium text-gray-900 mb-1">We value your privacy</p>
          <p>
            We use analytics to improve your experience. No personal health information is ever tracked.{' '}
            <a href="/privacy" className="text-blue-600 hover:underline">Learn more</a>
          </p>
        </div>
        <div className="flex gap-3 shrink-0">
          <button
            onClick={handleDecline}
            className="px-4 py-2 text-sm text-gray-600 hover:text-gray-900 transition-colors"
          >
            Decline
          </button>
          <button
            onClick={handleAccept}
            className="px-4 py-2 text-sm bg-blue-600 text-white rounded hover:bg-blue-700 transition-colors"
          >
            Accept
          </button>
        </div>
      </div>
    </div>
  );
}

// Add missing import
import { useState } from 'react';

/**
 * Branding Integration Examples
 * 
 * Example usage patterns for the S.E.R.A. branding system.
 * Copy and adapt these patterns for your application.
 */

import React from 'react';
import {
  BrandingProvider,
  BrandedHeader,
  BrandedFooter,
  BrandedButton,
  BrandedLogo,
  useTheme,
  useBranding,
  useOrganization,
  useColors,
  useLogo,
} from './index';

// ============================================================================
// Example 1: Basic App Layout with Branding
// ============================================================================

interface AppLayoutProps {
  children: React.ReactNode;
  organizationId: string;
}

export function BrandedAppLayout({ children, organizationId }: AppLayoutProps) {
  return (
    <BrandingProvider organizationId={organizationId}>
      <div className="min-h-screen bg-[var(--sera-bg-main)] flex flex-col">
        <BrandedHeader
          navigation={[
            { label: 'Dashboard', href: '/dashboard' },
            { label: 'Individuals', href: '/individuals' },
            { label: 'Medications', href: '/medications' },
            { label: 'Reports', href: '/reports' },
          ]}
          actions={
            <>
              <BrandedButton variant="ghost" size="sm">
                Help
              </BrandedButton>
              <BrandedButton variant="primary" size="sm">
                Profile
              </BrandedButton>
            </>
          }
        />
        
        <main className="flex-1 pt-16">
          {children}
        </main>
        
        <BrandedFooter
          links={[
            { label: 'Privacy Policy', href: '/privacy' },
            { label: 'Terms of Service', href: '/terms' },
            { label: 'Support', href: '/support' },
            { label: 'API Docs', href: '/api-docs' },
          ]}
        />
      </div>
    </BrandingProvider>
  );
}

// ============================================================================
// Example 2: Custom Component Using Branding Hooks
// ============================================================================

export function CustomBrandedCard({
  title,
  children,
}: {
  title: string;
  children: React.ReactNode;
}) {
  const { theme, isDark } = useTheme();
  const { colors, getLogoUrl } = useBranding();
  const { name: orgName } = useOrganization();

  return (
    <div
      className="rounded-lg p-6 shadow-md"
      style={{
        backgroundColor: colors.background.surface,
        border: `1px solid ${colors.border.default}`,
      }}
    >
      <div className="flex items-center gap-3 mb-4">
        {getLogoUrl() && (
          <img
            src={getLogoUrl()}
            alt={orgName}
            className="h-8 w-auto"
          />
        )}
        <h2
          className="text-xl font-semibold"
          style={{ color: colors.text.primary }}
        >
          {title}
        </h2>
      </div>
      
      <div style={{ color: colors.text.secondary }}>
        {children}
      </div>
      
      <div className="mt-4 text-xs" style={{ color: colors.text.muted }}>
        Current theme: {theme} (isDark: {isDark ? 'yes' : 'no'})
      </div>
    </div>
  );
}

// ============================================================================
// Example 3: Themed Status Badge
// ============================================================================

export function StatusBadge({
  status,
  children,
}: {
  status: 'success' | 'warning' | 'error' | 'info';
  children: React.ReactNode;
}) {
  const { colors } = useColors();

  const statusColors = {
    success: colors.semantic.success,
    warning: colors.semantic.warning,
    error: colors.semantic.error,
    info: colors.semantic.info,
  };

  const color = statusColors[status];

  return (
    <span
      className="inline-flex items-center px-2.5 py-0.5 rounded-full text-xs font-medium"
      style={{
        backgroundColor: `${color}20`, // 20% opacity
        color: color,
      }}
    >
      <span
        className="w-1.5 h-1.5 rounded-full mr-1.5"
        style={{ backgroundColor: color }}
      />
      {children}
    </span>
  );
}

// ============================================================================
// Example 4: Organization Logo with Fallback
// ============================================================================

export function OrganizationIdentity() {
  const { name, tagline } = useOrganization();
  const { url: logoUrl, dimensions } = useLogo();

  return (
    <div className="flex flex-col items-center gap-4 p-8">
      {logoUrl ? (
        <img
          src={logoUrl}
          alt={name}
          width={dimensions?.width}
          height={dimensions?.height}
          className="max-h-24 w-auto"
        />
      ) : (
        <div
          className="text-3xl font-bold"
          style={{ fontFamily: 'var(--sera-font-heading)' }}
        >
          {name}
        </div>
      )}
      
      {tagline && (
        <p className="text-lg text-[var(--sera-text-secondary)]">
          {tagline}
        </p>
      )}
    </div>
  );
}

// ============================================================================
// Example 5: Theme Toggle Button
// ============================================================================

export function ThemeToggle() {
  const { theme, toggleTheme, isDark } = useTheme();

  return (
    <button
      onClick={toggleTheme}
      className="flex items-center gap-2 px-4 py-2 rounded-md transition-colors"
      style={{
        backgroundColor: 'var(--sera-bg-surface)',
        color: 'var(--sera-text-primary)',
        border: '1px solid var(--sera-border-default)',
      }}
    >
      {isDark ? (
        <>
          <SunIcon />
          <span>Light Mode</span>
        </>
      ) : (
        <>
          <MoonIcon />
          <span>Dark Mode</span>
        </>
      )}
    </button>
  );
}

// ============================================================================
// Example 6: Branded Form Components
// ============================================================================

export function BrandedInput({
  label,
  error,
  ...props
}: React.InputHTMLAttributes<HTMLInputElement> & {
  label?: string;
  error?: string;
}) {
  const { colors } = useColors();

  return (
    <div className="flex flex-col gap-1">
      {label && (
        <label
          className="text-sm font-medium"
          style={{ color: colors.text.primary }}
        >
          {label}
        </label>
      )}
      <input
        className="px-3 py-2 rounded-md border transition-colors focus:outline-none focus:ring-2"
        style={{
          backgroundColor: colors.background.elevated,
          borderColor: error ? colors.semantic.error : colors.border.default,
          color: colors.text.primary,
        }}
        {...props}
      />
      {error && (
        <span className="text-sm" style={{ color: colors.semantic.error }}>
          {error}
        </span>
      )}
    </div>
  );
}

// ============================================================================
// Example 7: Loading State with Brand Colors
// ============================================================================

export function BrandedSkeleton() {
  const { colors } = useColors();

  return (
    <div
      className="animate-pulse rounded-md"
      style={{
        backgroundColor: colors.border.light,
      }}
    >
      <div className="h-4 w-3/4 mb-2" />
      <div className="h-4 w-1/2" />
    </div>
  );
}

// ============================================================================
// Example 8: Error Boundary with Branding
// ============================================================================

import { Component, ErrorInfo, ReactNode } from 'react';

interface BrandedErrorBoundaryProps {
  children: ReactNode;
  fallback?: ReactNode;
}

interface BrandedErrorBoundaryState {
  hasError: boolean;
  error?: Error;
}

export class BrandedErrorBoundary extends Component<
  BrandedErrorBoundaryProps,
  BrandedErrorBoundaryState
> {
  constructor(props: BrandedErrorBoundaryProps) {
    super(props);
    this.state = { hasError: false };
  }

  static getDerivedStateFromError(error: Error): BrandedErrorBoundaryState {
    return { hasError: true, error };
  }

  componentDidCatch(error: Error, errorInfo: ErrorInfo) {
    console.error('Error caught by boundary:', error, errorInfo);
  }

  render() {
    if (this.state.hasError) {
      if (this.props.fallback) {
        return this.props.fallback;
      }

      return (
        <BrandingErrorFallback error={this.state.error} />
      );
    }

    return this.props.children;
  }
}

function BrandingErrorFallback({ error }: { error?: Error }) {
  return (
    <div className="min-h-screen flex items-center justify-center p-4">
      <div className="max-w-md w-full text-center">
        <BrandedLogo height={48} className="mx-auto mb-6" />
        
        <h1
          className="text-2xl font-bold mb-2"
          style={{ color: 'var(--sera-text-primary)' }}
        >
          Something went wrong
        </h1>
        
        <p
          className="mb-6"
          style={{ color: 'var(--sera-text-secondary)' }}
        >
          We&apos;re having trouble loading the application. Please try refreshing the page.
        </p>
        
        {error && (
          <pre
            className="p-4 rounded-md text-left text-sm overflow-auto mb-6"
            style={{
              backgroundColor: 'var(--sera-bg-surface)',
              color: 'var(--sera-text-secondary)',
              border: '1px solid var(--sera-border-default)',
            }}
          >
            {error.message}
          </pre>
        )}
        
        <BrandedButton onClick={() => window.location.reload()}>
          Refresh Page
        </BrandedButton>
      </div>
    </div>
  );
}

// ============================================================================
// Icon Components
// ============================================================================

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

// ============================================================================
// Next.js App Router Example
// ============================================================================

/**
 * For Next.js App Router, use this pattern in your layout.tsx:
 * 
 * ```tsx
 * // app/layout.tsx
 * import { BrandingProvider } from '@/lib/branding';
 * import { BrandedHeader } from '@/lib/branding';
 * 
 * export default function RootLayout({
 *   children,
 * }: {
 *   children: React.ReactNode;
 * }) {
 *   return (
 *     <html suppressHydrationWarning>
 *       <body>
 *         <BrandingProvider organizationId={process.env.NEXT_PUBLIC_ORG_ID!}>
 *           <BrandedHeader />
 *           {children}
 *         </BrandingProvider>
 *       </body>
 *     </html>
 *   );
 * }
 * ```
 */

// ============================================================================
// Next.js Pages Router Example
// ============================================================================

/**
 * For Next.js Pages Router, use this pattern in your _app.tsx:
 * 
 * ```tsx
 * // pages/_app.tsx
 * import { BrandingProvider } from '@/lib/branding';
 * import type { AppProps } from 'next/app';
 * 
 * export default function App({ Component, pageProps }: AppProps) {
 *   return (
 *     <BrandingProvider organizationId={process.env.NEXT_PUBLIC_ORG_ID!}>
 *       <Component {...pageProps} />
 *     </BrandingProvider>
 *   );
 * }
 * ```
 */

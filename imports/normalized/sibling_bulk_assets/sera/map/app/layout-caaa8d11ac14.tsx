import type { Metadata, Viewport } from 'next'
import { Inter } from 'next/font/google'
import './globals.css'
import { ThemeProvider } from '@/lib/theme'
import { OfflineProvider } from '@/components/providers/OfflineProvider'
import { WebSocketProvider } from '@/components/providers/WebSocketProvider'
import { Navbar } from '@/components/navbar'
import { Footer } from '@/components/footer'
import { Toaster } from '@/components/ui/toaster'
import { ShortcutProvider } from '@/lib/shortcuts/useKeyboardShortcuts'
import { KeyboardShortcutsHandler } from '@/components/KeyboardShortcutsHandler'
import { ServiceWorkerRegistration } from '@/components/ServiceWorkerRegistration'
import { ErrorBoundary } from '@/components/ErrorBoundary'
import { AnalyticsProvider } from '@/components/providers/AnalyticsProvider'

const inter = Inter({ 
  subsets: ['latin'],
  display: 'swap', // Optimize font loading
  preload: true,
})

/**
 * Application metadata for SEO and PWA
 */
export const metadata: Metadata = {
  title: {
    default: 'S.E.R.A. - Support, Evaluation, Reporting & Administration',
    template: '%s | S.E.R.A.',
  },
  description: 'Comprehensive healthcare management platform for group homes. HIPAA-compliant electronic health records, medication administration tracking, and care coordination.',
  keywords: ['healthcare', 'EHR', 'group home', 'medication tracking', 'HIPAA compliant', 'care management'],
  authors: [{ name: 'OneWell Health Group' }],
  creator: 'OneWell Health Group',
  publisher: 'OneWell Health Group',
  metadataBase: new URL(process.env.NEXT_PUBLIC_APP_URL || 'https://sera.onewell.com'),
  alternates: {
    canonical: '/',
  },
  openGraph: {
    type: 'website',
    locale: 'en_US',
    url: '/',
    title: 'S.E.R.A. - Healthcare Management Platform',
    description: 'Comprehensive healthcare management platform for group homes',
    siteName: 'S.E.R.A.',
  },
  twitter: {
    card: 'summary_large_image',
    title: 'S.E.R.A. - Healthcare Management Platform',
    description: 'Comprehensive healthcare management platform for group homes',
  },
  robots: {
    index: process.env.NODE_ENV === 'production',
    follow: process.env.NODE_ENV === 'production',
    googleBot: {
      index: process.env.NODE_ENV === 'production',
      follow: process.env.NODE_ENV === 'production',
      'max-video-preview': -1,
      'max-image-preview': 'large',
      'max-snippet': -1,
    },
  },
  manifest: '/manifest.json',
  icons: {
    icon: [
      { url: '/icons/favicon-16x16.png', sizes: '16x16', type: 'image/png' },
      { url: '/icons/favicon-32x32.png', sizes: '32x32', type: 'image/png' },
    ],
    apple: [
      { url: '/icons/apple-touch-icon.png', sizes: '180x180', type: 'image/png' },
    ],
    other: [
      { rel: 'mask-icon', url: '/icons/safari-pinned-tab.svg', color: '#0f172a' },
    ],
  },
  appleWebApp: {
    title: 'S.E.R.A.',
    statusBarStyle: 'black-translucent',
    capable: true,
  },
  applicationName: 'S.E.R.A.',
  formatDetection: {
    telephone: false,
  },
  verification: {
    google: process.env.NEXT_PUBLIC_GOOGLE_SITE_VERIFICATION,
  },
  other: {
    'msapplication-TileColor': '#0f172a',
    'msapplication-config': '/icons/browserconfig.xml',
  },
}

/**
 * Viewport configuration for responsive design
 */
export const viewport: Viewport = {
  width: 'device-width',
  initialScale: 1,
  maximumScale: 5,
  userScalable: true,
  themeColor: [
    { media: '(prefers-color-scheme: light)', color: '#ffffff' },
    { media: '(prefers-color-scheme: dark)', color: '#0f172a' },
  ],
  colorScheme: 'light dark',
}

/**
 * Root layout component with all providers and global configuration
 */
export default function RootLayout({
  children,
}: {
  children: React.ReactNode
}) {
  return (
    <html 
      lang="en" 
      suppressHydrationWarning
      className="scroll-smooth"
    >
      <head>
        {/* Preconnect to external domains for performance */}
        <link rel="preconnect" href={process.env.NEXT_PUBLIC_API_URL || ''} />
        <link rel="dns-prefetch" href={process.env.NEXT_PUBLIC_API_URL || ''} />
        
        {/* Preload critical resources */}
        <link rel="preload" href="/manifest.json" as="fetch" crossOrigin="anonymous" />
        
        {/* Security headers (also set in next.config.js) */}
        <meta httpEquiv="X-UA-Compatible" content="IE=edge" />
        <meta name="referrer" content="strict-origin-when-cross-origin" />
        
        {/* PWA tags */}
        <meta name="mobile-web-app-capable" content="yes" />
        <meta name="apple-mobile-web-app-capable" content="yes" />
        <meta name="apple-mobile-web-app-status-bar-style" content="black-translucent" />
        <meta name="apple-mobile-web-app-title" content="S.E.R.A." />
        <meta name="application-name" content="S.E.R.A." />
        <meta name="msapplication-TileColor" content="#0f172a" />
        
        {/* Performance optimizations */}
        <meta name="format-detection" content="telephone=no" />
        
        {/* Feature Policy / Permissions Policy */}
        <meta 
          httpEquiv="Permissions-Policy" 
          content="camera=(), microphone=(), geolocation=(self), fullscreen=(self)" 
        />
      </head>
      <body className={`${inter.className} theme-transition antialiased`}>
        <ErrorBoundary>
          <ThemeProvider
            defaultTheme="system"
            enableSystemTheme
            enableSmoothTransitions
            storageKey="sera-theme-preference"
          >
            <ShortcutProvider>
              <OfflineProvider>
                <WebSocketProvider>
                  <AnalyticsProvider>
                    <KeyboardShortcutsHandler>
                      <div className="min-h-screen flex flex-col">
                        <Navbar />
                        <main className="flex-1">
                          {children}
                        </main>
                        <Footer />
                      </div>
                      <Toaster />
                    </KeyboardShortcutsHandler>
                  </AnalyticsProvider>
                </WebSocketProvider>
              </OfflineProvider>
            </ShortcutProvider>
          </ThemeProvider>
        </ErrorBoundary>
        
        {/* Service Worker Registration */}
        <ServiceWorkerRegistration />
        
        {/* No-JS fallback */}
        <noscript>
          <div className="fixed inset-0 bg-background flex items-center justify-center z-50">
            <div className="text-center p-8">
              <h1 className="text-2xl font-bold mb-4">JavaScript Required</h1>
              <p className="text-muted-foreground">
                S.E.R.A. requires JavaScript to function. Please enable JavaScript in your browser settings.
              </p>
            </div>
          </div>
        </noscript>
      </body>
    </html>
  )
}

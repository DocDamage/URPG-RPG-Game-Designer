import Link from 'next/link';
import { Metadata } from 'next';

export const metadata: Metadata = {
  title: 'Page Not Found',
  robots: {
    index: false,
    follow: false,
  },
};

/**
 * 404 Not Found Page
 * 
 * Custom error page shown when a route is not found.
 * Provides navigation options back to the application.
 */
export default function NotFound() {
  return (
    <div className="min-h-[calc(100vh-4rem)] flex items-center justify-center px-4 sm:px-6 lg:px-8">
      <div className="max-w-lg w-full text-center">
        {/* Error Icon */}
        <div className="mx-auto h-24 w-24 text-slate-300 mb-8">
          <svg
            fill="none"
            viewBox="0 0 24 24"
            stroke="currentColor"
            aria-hidden="true"
          >
            <path
              strokeLinecap="round"
              strokeLinejoin="round"
              strokeWidth={1.5}
              d="M9.172 16.172a4 4 0 015.656 0M9 10h.01M15 10h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z"
            />
          </svg>
        </div>

        {/* Error Code */}
        <p className="text-sm font-semibold text-blue-600 tracking-wide uppercase mb-2">
          404 Error
        </p>

        {/* Title */}
        <h1 className="text-4xl font-bold text-slate-900 sm:text-5xl mb-4">
          Page not found
        </h1>

        {/* Description */}
        <p className="text-lg text-slate-600 mb-8">
          Sorry, we couldn&apos;t find the page you&apos;re looking for. 
          It might have been moved, deleted, or you may have typed the URL incorrectly.
        </p>

        {/* Action Buttons */}
        <div className="flex flex-col sm:flex-row gap-4 justify-center">
          <Link
            href="/"
            className="inline-flex items-center justify-center px-6 py-3 border border-transparent text-base font-medium rounded-md text-white bg-blue-600 hover:bg-blue-700 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-blue-500 transition-colors"
          >
            <svg
              className="-ml-1 mr-2 h-5 w-5"
              fill="none"
              viewBox="0 0 24 24"
              stroke="currentColor"
            >
              <path
                strokeLinecap="round"
                strokeLinejoin="round"
                strokeWidth={2}
                d="M3 12l2-2m0 0l7-7 7 7M5 10v10a1 1 0 001 1h3m10-11l2 2m-2-2v10a1 1 0 01-1 1h-3m-6 0a1 1 0 001-1v-4a1 1 0 011-1h2a1 1 0 011 1v4a1 1 0 001 1m-6 0h6"
              />
            </svg>
            Back to Home
          </Link>

          <Link
            href="/dashboard"
            className="inline-flex items-center justify-center px-6 py-3 border border-slate-300 text-base font-medium rounded-md text-slate-700 bg-white hover:bg-slate-50 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-blue-500 transition-colors"
          >
            <svg
              className="-ml-1 mr-2 h-5 w-5"
              fill="none"
              viewBox="0 0 24 24"
              stroke="currentColor"
            >
              <path
                strokeLinecap="round"
                strokeLinejoin="round"
                strokeWidth={2}
                d="M4 6a2 2 0 012-2h2a2 2 0 012 2v2a2 2 0 01-2 2H6a2 2 0 01-2-2V6zM14 6a2 2 0 012-2h2a2 2 0 012 2v2a2 2 0 01-2 2h-2a2 2 0 01-2-2V6zM4 16a2 2 0 012-2h2a2 2 0 012 2v2a2 2 0 01-2 2H6a2 2 0 01-2-2v-2zM14 16a2 2 0 012-2h2a2 2 0 012 2v2a2 2 0 01-2 2h-2a2 2 0 01-2-2v-2z"
              />
            </svg>
            Dashboard
          </Link>
        </div>

        {/* Help Links */}
        <div className="mt-12 border-t border-slate-200 pt-8">
          <p className="text-sm text-slate-500 mb-4">
            Need help? Try these resources:
          </p>
          <div className="flex flex-wrap justify-center gap-4 text-sm">
            <Link 
              href="/help" 
              className="text-blue-600 hover:text-blue-500 transition-colors"
            >
              Help Center
            </Link>
            <span className="text-slate-300">|</span>
            <Link 
              href="/help/shortcuts" 
              className="text-blue-600 hover:text-blue-500 transition-colors"
            >
              Keyboard Shortcuts
            </Link>
            <span className="text-slate-300">|</span>
            <button
              onClick={() => window.history.back()}
              className="text-blue-600 hover:text-blue-500 transition-colors"
            >
              Go Back
            </button>
          </div>
        </div>

        {/* Support Contact */}
        <div className="mt-8 text-sm text-slate-400">
          <p>
            If you believe this is an error, please contact{' '}
            <a 
              href="mailto:support@onewell.com" 
              className="text-blue-600 hover:text-blue-500 transition-colors"
            >
              support@onewell.com
            </a>
          </p>
        </div>
      </div>
    </div>
  );
}

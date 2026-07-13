'use client';

import { useEffect } from 'react';
import Link from 'next/link';

/**
 * Error Page Component
 * 
 * Handles runtime errors in the application.
 * Provides user-friendly error messaging and recovery options.
 */
export default function Error({
  error,
  reset,
}: {
  error: Error & { digest?: string };
  reset: () => void;
}) {
  useEffect(() => {
    // Log error to monitoring service
    console.error('Application error:', error);
    
    // Send to error tracking if available
    if (typeof window !== 'undefined' && (window as any).Sentry) {
      (window as any).Sentry.captureException(error);
    }
  }, [error]);

  return (
    <div className="min-h-[calc(100vh-4rem)] flex items-center justify-center px-4 sm:px-6 lg:px-8">
      <div className="max-w-lg w-full text-center">
        {/* Error Icon */}
        <div className="mx-auto h-24 w-24 text-red-500 mb-8">
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
              d="M12 9v2m0 4h.01m-6.938 4h13.856c1.54 0 2.502-1.667 1.732-3L13.732 4c-.77-1.333-2.694-1.333-3.464 0L3.34 16c-.77 1.333.192 3 1.732 3z"
            />
          </svg>
        </div>

        {/* Error Code */}
        <p className="text-sm font-semibold text-red-600 tracking-wide uppercase mb-2">
          Application Error
        </p>

        {/* Title */}
        <h1 className="text-4xl font-bold text-slate-900 sm:text-5xl mb-4">
          Something went wrong
        </h1>

        {/* Description */}
        <p className="text-lg text-slate-600 mb-8">
          We&apos;re sorry, but something unexpected happened. Our team has been notified 
          and is working to resolve the issue.
        </p>

        {/* Error Details (Development Only) */}
        {process.env.NODE_ENV === 'development' && (
          <div className="mb-8 text-left">
            <div className="bg-red-50 border border-red-200 rounded-lg p-4 overflow-auto">
              <p className="text-sm font-mono text-red-800 break-all">
                {error.message}
              </p>
              {error.stack && (
                <pre className="mt-2 text-xs text-red-700 overflow-auto max-h-48">
                  {error.stack}
                </pre>
              )}
              {error.digest && (
                <p className="mt-2 text-xs text-red-600">
                  Error ID: {error.digest}
                </p>
              )}
            </div>
          </div>
        )}

        {/* Action Buttons */}
        <div className="flex flex-col sm:flex-row gap-4 justify-center">
          <button
            onClick={reset}
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
                d="M4 4v5h.582m15.356 2A8.001 8.001 0 004.582 9m0 0H9m11 11v-5h-.581m0 0a8.003 8.003 0 01-15.357-2m15.357 2H15"
              />
            </svg>
            Try Again
          </button>

          <Link
            href="/"
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
                d="M3 12l2-2m0 0l7-7 7 7M5 10v10a1 1 0 001 1h3m10-11l2 2m-2-2v10a1 1 0 01-1 1h-3m-6 0a1 1 0 001-1v-4a1 1 0 011-1h2a1 1 0 011 1v4a1 1 0 001 1m-6 0h6"
              />
            </svg>
            Back to Home
          </Link>
        </div>

        {/* Troubleshooting Tips */}
        <div className="mt-12 border-t border-slate-200 pt-8">
          <p className="text-sm font-medium text-slate-900 mb-4">
            Troubleshooting tips:
          </p>
          <ul className="text-sm text-slate-600 space-y-2 text-left max-w-md mx-auto">
            <li className="flex items-start">
              <svg className="h-5 w-5 text-green-500 mr-2 shrink-0" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M5 13l4 4L19 7" />
              </svg>
              Refresh the page and try again
            </li>
            <li className="flex items-start">
              <svg className="h-5 w-5 text-green-500 mr-2 shrink-0" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M5 13l4 4L19 7" />
              </svg>
              Clear your browser cache and cookies
            </li>
            <li className="flex items-start">
              <svg className="h-5 w-5 text-green-500 mr-2 shrink-0" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M5 13l4 4L19 7" />
              </svg>
              Check your internet connection
            </li>
            <li className="flex items-start">
              <svg className="h-5 w-5 text-green-500 mr-2 shrink-0" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M5 13l4 4L19 7" />
              </svg>
              Try using a different browser
            </li>
          </ul>
        </div>

        {/* Support Contact */}
        <div className="mt-8 text-sm text-slate-400">
          <p>
            If the problem persists, please contact{' '}
            <a 
              href="mailto:support@onewell.com" 
              className="text-blue-600 hover:text-blue-500 transition-colors"
            >
              support@onewell.com
            </a>
            {error.digest && (
              <span className="block mt-2 text-xs">
                Reference: {error.digest}
              </span>
            )}
          </p>
        </div>
      </div>
    </div>
  );
}

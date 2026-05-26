import { Metadata } from 'next';

export const metadata: Metadata = {
  title: 'Maintenance Mode',
  robots: {
    index: false,
    follow: false,
  },
};

/**
 * Maintenance Mode Page
 * 
 * Displayed when the application is undergoing maintenance.
 * Can be triggered by setting NEXT_PUBLIC_MAINTENANCE_MODE=true
 */
export default function MaintenancePage() {
  // Get maintenance info from environment
  const maintenanceEndTime = process.env.NEXT_PUBLIC_MAINTENANCE_END;
  const maintenanceMessage = process.env.NEXT_PUBLIC_MAINTENANCE_MESSAGE;

  return (
    <div className="min-h-[calc(100vh-4rem)] flex items-center justify-center px-4 sm:px-6 lg:px-8">
      <div className="max-w-lg w-full text-center">
        {/* Maintenance Icon */}
        <div className="mx-auto h-24 w-24 text-amber-500 mb-8">
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
              d="M10.325 4.317c.426-1.756 2.924-1.756 3.35 0a1.724 1.724 0 002.573 1.066c1.543-.94 3.31.826 2.37 2.37a1.724 1.724 0 001.065 2.572c1.756.426 1.756 2.924 0 3.35a1.724 1.724 0 00-1.066 2.573c.94 1.543-.826 3.31-2.37 2.37a1.724 1.724 0 00-2.572 1.065c-.426 1.756-2.924 1.756-3.35 0a1.724 1.724 0 00-2.573-1.066c-1.543.94-3.31-.826-2.37-2.37a1.724 1.724 0 00-1.065-2.572c-1.756-.426-1.756-2.924 0-3.35a1.724 1.724 0 001.066-2.573c-.94-1.543.826-3.31 2.37-2.37.996.608 2.296.07 2.572-1.065z"
            />
            <path
              strokeLinecap="round"
              strokeLinejoin="round"
              strokeWidth={1.5}
              d="M15 12a3 3 0 11-6 0 3 3 0 016 0z"
            />
          </svg>
        </div>

        {/* Status */}
        <p className="text-sm font-semibold text-amber-600 tracking-wide uppercase mb-2">
          System Maintenance
        </p>

        {/* Title */}
        <h1 className="text-4xl font-bold text-slate-900 sm:text-5xl mb-4">
          We&apos;ll be back soon
        </h1>

        {/* Message */}
        <p className="text-lg text-slate-600 mb-6">
          {maintenanceMessage || 
            "We're currently performing scheduled maintenance to improve your experience. " +
            "We apologize for any inconvenience caused."
          }
        </p>

        {/* Estimated Time */}
        {maintenanceEndTime && (
          <div className="mb-8 p-4 bg-amber-50 border border-amber-200 rounded-lg">
            <p className="text-sm text-amber-800">
              <span className="font-medium">Estimated completion:</span>{' '}
              {new Date(maintenanceEndTime).toLocaleString('en-US', {
                weekday: 'long',
                year: 'numeric',
                month: 'long',
                day: 'numeric',
                hour: 'numeric',
                minute: '2-digit',
                timeZoneName: 'short',
              })}
            </p>
          </div>
        )}

        {/* Status Check */}
        <div className="flex flex-col items-center gap-4">
          <button
            onClick={() => window.location.reload()}
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
            Check Status
          </button>

          <p className="text-sm text-slate-500">
            The page will automatically reload when service is restored
          </p>
        </div>

        {/* Progress Indicator */}
        <div className="mt-12">
          <div className="relative">
            <div className="overflow-hidden h-2 text-xs flex rounded bg-amber-100">
              <div 
                className="animate-pulse shadow-none flex flex-col text-center whitespace-nowrap text-white justify-center bg-amber-500"
                style={{ width: '60%' }}
              />
            </div>
          </div>
          <p className="mt-2 text-sm text-slate-500">
            Maintenance in progress...
          </p>
        </div>

        {/* Contact Info */}
        <div className="mt-12 border-t border-slate-200 pt-8">
          <p className="text-sm text-slate-500 mb-4">
            Need urgent assistance?
          </p>
          <div className="flex flex-col sm:flex-row justify-center gap-4 text-sm">
            <a 
              href="tel:+1-800-ONeWell" 
              className="inline-flex items-center justify-center text-blue-600 hover:text-blue-500 transition-colors"
            >
              <svg className="h-4 w-4 mr-2" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M3 5a2 2 0 012-2h3.28a1 1 0 01.948.684l1.498 4.493a1 1 0 01-.502 1.21l-2.257 1.13a11.042 11.042 0 005.516 5.516l1.13-2.257a1 1 0 011.21-.502l4.493 1.498a1 1 0 01.684.949V19a2 2 0 01-2 2h-1C9.716 21 3 14.284 3 6V5z" />
              </svg>
              Emergency Line
            </a>
            <a 
              href="mailto:support@onewell.com" 
              className="inline-flex items-center justify-center text-blue-600 hover:text-blue-500 transition-colors"
            >
              <svg className="h-4 w-4 mr-2" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M3 8l7.89 5.26a2 2 0 002.22 0L21 8M5 19h14a2 2 0 002-2V7a2 2 0 00-2-2H5a2 2 0 00-2 2v10a2 2 0 002 2z" />
              </svg>
              Email Support
            </a>
          </div>
        </div>
      </div>
    </div>
  );
}

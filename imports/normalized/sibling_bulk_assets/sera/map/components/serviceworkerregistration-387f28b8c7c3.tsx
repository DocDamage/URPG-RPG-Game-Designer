'use client';

import { useEffect, useState } from 'react';

/**
 * Service Worker Registration Component
 * 
 * Handles PWA service worker registration, updates, and offline functionality.
 * Provides feedback to users when updates are available.
 */
export function ServiceWorkerRegistration() {
  const [updateAvailable, setUpdateAvailable] = useState(false);
  const [offlineReady, setOfflineReady] = useState(false);
  const [registration, setRegistration] = useState<ServiceWorkerRegistration | null>(null);

  useEffect(() => {
    if (
      typeof window !== 'undefined' &&
      'serviceWorker' in navigator &&
      window.workbox !== undefined
    ) {
      const wb = window.workbox;

      // Add event listeners to handle PWA lifecycle
      
      // Fired when the service worker is installed
      wb.addEventListener('installed', (event: Event) => {
        if ((event as any).isUpdate) {
          console.log('Service worker updated');
          setUpdateAvailable(true);
        } else {
          console.log('Service worker installed for the first time');
          setOfflineReady(true);
        }
      });

      // Fired when the service worker is controlling the page
      wb.addEventListener('controlling', () => {
        console.log('Service worker is now controlling the page');
      });

      // Fired when a new service worker is waiting
      wb.addEventListener('waiting', () => {
        console.log('New service worker waiting');
        setUpdateAvailable(true);
      });

      // Fired when the service worker activates
      wb.addEventListener('activated', () => {
        console.log('Service worker activated');
      });

      // Register the service worker
      wb.register()
        .then((reg) => {
          console.log('Service Worker registered:', reg);
          setRegistration(reg);

          // Check for updates periodically
          setInterval(() => {
            reg.update();
          }, 60 * 60 * 1000); // Check every hour
        })
        .catch((error) => {
          console.error('Service Worker registration failed:', error);
        });
    }
  }, []);

  const handleUpdate = () => {
    if (registration && registration.waiting) {
      // Send skip waiting message to the waiting service worker
      registration.waiting.postMessage({ type: 'SKIP_WAITING' });
      
      // Reload the page after a short delay to allow the new service worker to take control
      setTimeout(() => {
        window.location.reload();
      }, 100);
    }
  };

  const handleDismiss = () => {
    setUpdateAvailable(false);
    setOfflineReady(false);
  };

  // Show notification when offline functionality is ready
  if (offlineReady) {
    return (
      <div className="fixed bottom-4 right-4 z-50 bg-green-600 text-white px-4 py-3 rounded-lg shadow-lg flex items-center gap-3 animate-in slide-in-from-bottom-4">
        <svg className="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
          <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M5 13l4 4L19 7" />
        </svg>
        <div>
          <p className="font-medium">App ready for offline use</p>
          <p className="text-sm opacity-90">You can use S.E.R.A. without an internet connection</p>
        </div>
        <button 
          onClick={handleDismiss}
          className="ml-2 p-1 hover:bg-green-700 rounded"
          aria-label="Dismiss"
        >
          <svg className="w-4 h-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M6 18L18 6M6 6l12 12" />
          </svg>
        </button>
      </div>
    );
  }

  // Show update notification
  if (updateAvailable) {
    return (
      <div className="fixed bottom-4 right-4 z-50 bg-blue-600 text-white px-4 py-3 rounded-lg shadow-lg flex items-center gap-3 animate-in slide-in-from-bottom-4">
        <svg className="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
          <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M4 4v5h.582m15.356 2A8.001 8.001 0 004.582 9m0 0H9m11 11v-5h-.581m0 0a8.003 8.003 0 01-15.357-2m15.357 2H15" />
        </svg>
        <div>
          <p className="font-medium">Update available</p>
          <p className="text-sm opacity-90">A new version of S.E.R.A. is ready</p>
        </div>
        <div className="flex gap-2 ml-2">
          <button 
            onClick={handleUpdate}
            className="px-3 py-1 bg-white text-blue-600 rounded text-sm font-medium hover:bg-blue-50 transition-colors"
          >
            Update
          </button>
          <button 
            onClick={handleDismiss}
            className="px-3 py-1 bg-blue-700 text-white rounded text-sm hover:bg-blue-800 transition-colors"
          >
            Later
          </button>
        </div>
      </div>
    );
  }

  return null;
}

// Type declarations for Workbox
declare global {
  interface Window {
    workbox?: {
      register: () => Promise<ServiceWorkerRegistration>;
      addEventListener: (event: string, callback: (event: Event) => void) => void;
    };
  }
}

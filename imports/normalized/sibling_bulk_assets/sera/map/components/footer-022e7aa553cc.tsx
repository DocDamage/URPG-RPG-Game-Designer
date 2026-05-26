"use client";
import { useOfflineSync } from "../lib/hooks/useOfflineSync";

export default function Footer() {
  const { lastSync, isOnline } = useOfflineSync();

  const formatLastSync = (timestamp: number | null) => {
    if (!timestamp) return "Never";
    const date = new Date(timestamp);
    const now = Date.now();
    const diff = now - timestamp;

    if (diff < 60000) return "Just now";
    if (diff < 3600000) return `${Math.floor(diff / 60000)}m ago`;
    return date.toLocaleTimeString();
  };

  return (
    <footer className="w-full border-t border-gray-200 dark:border-gray-700 bg-white dark:bg-gray-900 px-4 py-2">
      <div className="flex items-center justify-between text-xs text-gray-500">
        <p>© {new Date().getFullYear()} S.E.R.A. - Smart Electronic Records Assistant</p>
        <div className="flex items-center space-x-4">
          <span className={isOnline ? "text-green-600" : "text-yellow-600"}>
            {isOnline ? "● Online" : "● Offline"}
          </span>
          <span>Last sync: {formatLastSync(lastSync)}</span>
        </div>
      </div>
    </footer>
  );
}
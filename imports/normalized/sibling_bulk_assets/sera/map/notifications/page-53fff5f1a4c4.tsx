"use client";

import { NotificationPanel } from "../../components/NotificationPanel";

export default function NotificationsPage() {
  return (
    <main className="min-h-screen bg-gray-50 dark:bg-gray-950">
      <div className="max-w-5xl mx-auto px-4 py-8">
        <NotificationPanel />
      </div>
    </main>
  );
}

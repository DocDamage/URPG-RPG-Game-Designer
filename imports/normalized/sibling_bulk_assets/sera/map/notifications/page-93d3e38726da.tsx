"use client";

import { NotificationPreferences } from "../../../components/NotificationPreferences";

export default function NotificationSettingsPage() {
  return (
    <main className="min-h-screen bg-gray-50 dark:bg-gray-950">
      <div className="max-w-4xl mx-auto px-4 py-8">
        <NotificationPreferences />
      </div>
    </main>
  );
}

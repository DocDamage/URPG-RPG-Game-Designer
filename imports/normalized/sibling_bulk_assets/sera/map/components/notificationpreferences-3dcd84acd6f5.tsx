"use client";

import React, { useState } from "react";
import { Bell, Mail, Smartphone, Monitor, Clock, Save, Check, AlertCircle } from "lucide-react";
import { clsx } from "clsx";
import { useNotifications } from "../hooks/useNotifications";
import { NotificationType, NotificationPreferences as NotificationPreferencesType } from "../lib/api/notifications";

interface NotificationPreferencesProps {
  className?: string;
}

const typeLabels: Record<NotificationType, string> = {
  mar_reminder: "Medication Reminders",
  mar_overdue: "Missed Medications",
  incident_reported: "New Incidents",
  incident_escalated: "Escalated Incidents",
  shift_handoff: "Shift Handoffs",
  shift_uncovered: "Uncovered Shifts",
  inventory_low: "Low Inventory",
  training_due: "Training Due",
  family_message: "Family Messages",
  system_alert: "System Alerts",
  behavioral_alert: "Behavioral Alerts",
  emergency: "Emergency Alerts",
};

const typeDescriptions: Record<NotificationType, string> = {
  mar_reminder: "Get notified before medications are due",
  mar_overdue: "Urgent alerts for missed medications",
  incident_reported: "When a new incident is reported",
  incident_escalated: "When an incident requires supervisor attention",
  shift_handoff: "Reminders for shift change documentation",
  shift_uncovered: "Urgent alerts when shifts need coverage",
  inventory_low: "When supplies are running low",
  training_due: "Upcoming certification renewals",
  family_message: "New messages from family members",
  system_alert: "System maintenance and updates",
  behavioral_alert: "Changes in behavioral patterns",
  emergency: "Critical emergency notifications",
};

// Default channel preferences for each type
const defaultTypePreferences: Record<NotificationType, { email: boolean; sms: boolean; push: boolean; in_app: boolean }> = {
  mar_reminder: { email: true, sms: false, push: true, in_app: true },
  mar_overdue: { email: true, sms: true, push: true, in_app: true },
  incident_reported: { email: false, sms: false, push: true, in_app: true },
  incident_escalated: { email: true, sms: true, push: true, in_app: true },
  shift_handoff: { email: false, sms: false, push: true, in_app: true },
  shift_uncovered: { email: true, sms: true, push: true, in_app: true },
  inventory_low: { email: true, sms: false, push: false, in_app: true },
  training_due: { email: true, sms: false, push: false, in_app: true },
  family_message: { email: true, sms: false, push: true, in_app: true },
  system_alert: { email: true, sms: false, push: false, in_app: false },
  behavioral_alert: { email: false, sms: false, push: true, in_app: true },
  emergency: { email: true, sms: true, push: true, in_app: true },
};

export function NotificationPreferences({ className }: NotificationPreferencesProps) {
  const { preferences, isPreferencesLoading, updatePreferences } = useNotifications();
  const [localPreferences, setLocalPreferences] = useState<Partial<NotificationPreferencesType> | null>(null);
  const [hasChanges, setHasChanges] = useState(false);
  const [saveStatus, setSaveStatus] = useState<"idle" | "saving" | "saved" | "error">("idle");

  // Use local preferences if they exist, otherwise use server preferences
  const currentPreferences = localPreferences || preferences;

  // Initialize local preferences when server preferences load
  React.useEffect(() => {
    if (preferences && !localPreferences) {
      setLocalPreferences(preferences);
    }
  }, [preferences, localPreferences]);

  const handleMasterToggle = (channel: keyof NotificationPreferencesType) => {
    if (!currentPreferences) return;
    
    setLocalPreferences({
      ...currentPreferences,
      [channel]: !currentPreferences[channel],
    });
    setHasChanges(true);
    setSaveStatus("idle");
  };

  const handleTypeToggle = (
    type: NotificationType,
    channel: "email" | "sms" | "push" | "in_app"
  ) => {
    if (!currentPreferences) return;

    const typePrefs = currentPreferences.type_preferences || {};
    const currentTypePref = typePrefs[type] || defaultTypePreferences[type];

    setLocalPreferences({
      ...currentPreferences,
      type_preferences: {
        ...typePrefs,
        [type]: {
          ...currentTypePref,
          [channel]: !currentTypePref[channel],
        },
      },
    });
    setHasChanges(true);
    setSaveStatus("idle");
  };

  const handleQuietHoursChange = (field: "quiet_hours_start" | "quiet_hours_end", value: string) => {
    if (!currentPreferences) return;

    setLocalPreferences({
      ...currentPreferences,
      [field]: value,
    });
    setHasChanges(true);
    setSaveStatus("idle");
  };

  const handleSave = async () => {
    if (!localPreferences || !hasChanges) return;

    setSaveStatus("saving");
    try {
      await updatePreferences(localPreferences);
      setHasChanges(false);
      setSaveStatus("saved");
      setTimeout(() => setSaveStatus("idle"), 2000);
    } catch (error) {
      setSaveStatus("error");
    }
  };

  const handleReset = () => {
    setLocalPreferences(preferences);
    setHasChanges(false);
    setSaveStatus("idle");
  };

  if (isPreferencesLoading || !currentPreferences) {
    return (
      <div className={clsx("bg-white dark:bg-gray-900 rounded-lg shadow-sm border border-gray-200 dark:border-gray-700 p-8", className)}>
        <div className="animate-pulse space-y-4">
          <div className="h-8 bg-gray-200 dark:bg-gray-700 rounded w-1/3" />
          <div className="h-32 bg-gray-200 dark:bg-gray-700 rounded" />
          <div className="h-64 bg-gray-200 dark:bg-gray-700 rounded" />
        </div>
      </div>
    );
  }

  return (
    <div className={clsx("bg-white dark:bg-gray-900 rounded-lg shadow-sm border border-gray-200 dark:border-gray-700", className)}>
      {/* Header */}
      <div className="px-6 py-4 border-b border-gray-200 dark:border-gray-700">
        <h2 className="text-xl font-semibold text-gray-900 dark:text-white flex items-center gap-2">
          <Bell className="w-5 h-5" />
          Notification Preferences
        </h2>
        <p className="text-sm text-gray-500 dark:text-gray-400 mt-1">
          Customize how and when you receive notifications
        </p>
      </div>

      <div className="p-6 space-y-8">
        {/* Master Toggles */}
        <section>
          <h3 className="text-sm font-medium text-gray-900 dark:text-white uppercase tracking-wider mb-4">
            Notification Channels
          </h3>
          <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-4">
            <ChannelToggle
              icon={Monitor}
              label="In-App"
              description="Show in notification center"
              enabled={currentPreferences.in_app_enabled}
              onToggle={() => handleMasterToggle("in_app_enabled")}
            />
            <ChannelToggle
              icon={Mail}
              label="Email"
              description="Send to your email"
              enabled={currentPreferences.email_enabled}
              onToggle={() => handleMasterToggle("email_enabled")}
            />
            <ChannelToggle
              icon={Smartphone}
              label="SMS"
              description="Text messages to your phone"
              enabled={currentPreferences.sms_enabled}
              onToggle={() => handleMasterToggle("sms_enabled")}
            />
            <ChannelToggle
              icon={Bell}
              label="Push"
              description="Browser/mobile push notifications"
              enabled={currentPreferences.push_enabled}
              onToggle={() => handleMasterToggle("push_enabled")}
            />
          </div>
        </section>

        {/* Quiet Hours */}
        <section className="border-t border-gray-200 dark:border-gray-700 pt-6">
          <h3 className="text-sm font-medium text-gray-900 dark:text-white uppercase tracking-wider mb-4 flex items-center gap-2">
            <Clock className="w-4 h-4" />
            Quiet Hours
          </h3>
          <div className="bg-gray-50 dark:bg-gray-800 rounded-lg p-4">
            <p className="text-sm text-gray-600 dark:text-gray-400 mb-4">
              During quiet hours, only urgent notifications will be sent. Other notifications will be queued and delivered when quiet hours end.
            </p>
            <div className="flex items-center gap-4 flex-wrap">
              <div className="flex items-center gap-2">
                <label className="text-sm text-gray-700 dark:text-gray-300">From</label>
                <input
                  type="time"
                  value={currentPreferences.quiet_hours_start || ""}
                  onChange={(e) => handleQuietHoursChange("quiet_hours_start", e.target.value)}
                  className="px-3 py-1.5 text-sm border border-gray-300 dark:border-gray-600 rounded-md bg-white dark:bg-gray-700 text-gray-900 dark:text-white"
                />
              </div>
              <div className="flex items-center gap-2">
                <label className="text-sm text-gray-700 dark:text-gray-300">To</label>
                <input
                  type="time"
                  value={currentPreferences.quiet_hours_end || ""}
                  onChange={(e) => handleQuietHoursChange("quiet_hours_end", e.target.value)}
                  className="px-3 py-1.5 text-sm border border-gray-300 dark:border-gray-600 rounded-md bg-white dark:bg-gray-700 text-gray-900 dark:text-white"
                />
              </div>
              {(currentPreferences.quiet_hours_start || currentPreferences.quiet_hours_end) && (
                <button
                  onClick={() => {
                    handleQuietHoursChange("quiet_hours_start", "");
                    handleQuietHoursChange("quiet_hours_end", "");
                  }}
                  className="text-sm text-gray-500 hover:text-gray-700 dark:hover:text-gray-300"
                >
                  Clear
                </button>
              )}
            </div>
          </div>
        </section>

        {/* Type-Specific Preferences */}
        <section className="border-t border-gray-200 dark:border-gray-700 pt-6">
          <h3 className="text-sm font-medium text-gray-900 dark:text-white uppercase tracking-wider mb-4">
            Notification Types
          </h3>
          <div className="space-y-3">
            {(Object.keys(typeLabels) as NotificationType[]).map((type) => (
              <TypePreferenceRow
                key={type}
                type={type}
                label={typeLabels[type]}
                description={typeDescriptions[type]}
                preferences={currentPreferences.type_preferences?.[type] || defaultTypePreferences[type]}
                masterPreferences={{
                  email: currentPreferences.email_enabled,
                  sms: currentPreferences.sms_enabled,
                  push: currentPreferences.push_enabled,
                  in_app: currentPreferences.in_app_enabled,
                }}
                onToggle={(channel) => handleTypeToggle(type, channel)}
              />
            ))}
          </div>
        </section>
      </div>

      {/* Footer Actions */}
      <div className="px-6 py-4 border-t border-gray-200 dark:border-gray-700 bg-gray-50 dark:bg-gray-800/50 rounded-b-lg">
        <div className="flex items-center justify-between">
          <div className="flex items-center gap-2">
            {hasChanges && (
              <span className="text-sm text-orange-600 dark:text-orange-400 flex items-center gap-1">
                <AlertCircle className="w-4 h-4" />
                Unsaved changes
              </span>
            )}
            {saveStatus === "saved" && (
              <span className="text-sm text-green-600 dark:text-green-400 flex items-center gap-1">
                <Check className="w-4 h-4" />
                Saved successfully
              </span>
            )}
            {saveStatus === "error" && (
              <span className="text-sm text-red-600 dark:text-red-400 flex items-center gap-1">
                <AlertCircle className="w-4 h-4" />
                Failed to save
              </span>
            )}
          </div>
          <div className="flex items-center gap-3">
            {hasChanges && (
              <button
                onClick={handleReset}
                className="px-4 py-2 text-sm font-medium text-gray-700 dark:text-gray-300 hover:text-gray-900 dark:hover:text-white"
              >
                Reset
              </button>
            )}
            <button
              onClick={handleSave}
              disabled={!hasChanges || saveStatus === "saving"}
              className={clsx(
                "flex items-center gap-2 px-4 py-2 text-sm font-medium rounded-lg transition-colors",
                hasChanges
                  ? "bg-blue-600 text-white hover:bg-blue-700"
                  : "bg-gray-300 text-gray-500 cursor-not-allowed"
              )}
            >
              <Save className="w-4 h-4" />
              {saveStatus === "saving" ? "Saving..." : "Save Changes"}
            </button>
          </div>
        </div>
      </div>
    </div>
  );
}

// Channel Toggle Component
interface ChannelToggleProps {
  icon: React.ElementType;
  label: string;
  description: string;
  enabled: boolean;
  onToggle: () => void;
}

function ChannelToggle({ icon: Icon, label, description, enabled, onToggle }: ChannelToggleProps) {
  return (
    <button
      onClick={onToggle}
      className={clsx(
        "flex items-start gap-3 p-4 rounded-lg border-2 text-left transition-colors",
        enabled
          ? "border-blue-500 bg-blue-50 dark:bg-blue-900/20"
          : "border-gray-200 dark:border-gray-700 hover:border-gray-300 dark:hover:border-gray-600"
      )}
    >
      <div
        className={clsx(
          "p-2 rounded-lg shrink-0",
          enabled ? "bg-blue-500 text-white" : "bg-gray-100 dark:bg-gray-800 text-gray-500"
        )}
      >
        <Icon className="w-5 h-5" />
      </div>
      <div className="flex-1">
        <div className="flex items-center gap-2">
          <span className={clsx("font-medium", enabled ? "text-blue-900 dark:text-blue-100" : "text-gray-700 dark:text-gray-300")}>
            {label}
          </span>
          <div
            className={clsx(
              "w-8 h-4 rounded-full transition-colors relative",
              enabled ? "bg-blue-500" : "bg-gray-300 dark:bg-gray-600"
            )}
          >
            <div
              className={clsx(
                "absolute top-0.5 w-3 h-3 bg-white rounded-full transition-transform",
                enabled ? "left-4.5 translate-x-0" : "left-0.5"
              )}
              style={{ left: enabled ? "18px" : "2px" }}
            />
          </div>
        </div>
        <p className="text-xs text-gray-500 dark:text-gray-400 mt-1">{description}</p>
      </div>
    </button>
  );
}

// Type Preference Row Component
interface TypePreferenceRowProps {
  type: NotificationType;
  label: string;
  description: string;
  preferences: { email: boolean; sms: boolean; push: boolean; in_app: boolean };
  masterPreferences: { email: boolean; sms: boolean; push: boolean; in_app: boolean };
  onToggle: (channel: "email" | "sms" | "push" | "in_app") => void;
}

function TypePreferenceRow({
  label,
  description,
  preferences,
  masterPreferences,
  onToggle,
}: TypePreferenceRowProps) {
  const channels: { key: "email" | "sms" | "push" | "in_app"; label: string }[] = [
    { key: "in_app", label: "In-App" },
    { key: "email", label: "Email" },
    { key: "sms", label: "SMS" },
    { key: "push", label: "Push" },
  ];

  return (
    <div className="flex items-center justify-between p-4 bg-gray-50 dark:bg-gray-800 rounded-lg">
      <div className="flex-1 min-w-0 mr-4">
        <h4 className="font-medium text-gray-900 dark:text-white">{label}</h4>
        <p className="text-sm text-gray-500 dark:text-gray-400">{description}</p>
      </div>
      <div className="flex items-center gap-2">
        {channels.map(({ key, label }) => (
          <button
            key={key}
            onClick={() => onToggle(key)}
            disabled={!masterPreferences[key]}
            className={clsx(
              "px-3 py-1.5 text-xs font-medium rounded-md border transition-colors",
              !masterPreferences[key]
                ? "border-gray-200 dark:border-gray-700 text-gray-300 dark:text-gray-600 cursor-not-allowed"
                : preferences[key]
                ? "border-blue-500 bg-blue-50 dark:bg-blue-900/20 text-blue-700 dark:text-blue-300"
                : "border-gray-300 dark:border-gray-600 text-gray-500 dark:text-gray-400 hover:border-gray-400 dark:hover:border-gray-500"
            )}
            title={!masterPreferences[key] ? `${label} notifications are disabled globally` : label}
          >
            {label}
          </button>
        ))}
      </div>
    </div>
  );
}

export default NotificationPreferences;

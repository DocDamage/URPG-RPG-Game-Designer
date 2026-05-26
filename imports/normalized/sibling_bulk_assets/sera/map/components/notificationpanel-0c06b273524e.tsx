"use client";

import React, { useState } from "react";
import {
  Bell,
  Check,
  Trash2,
  AlertTriangle,
  Clock,
  Package,
  MessageSquare,
  Calendar,
  BookOpen,
  Info,
  Filter,
  CheckCheck,
  X,
  ChevronDown,
} from "lucide-react";
import { clsx } from "clsx";
import { useNotifications } from "../hooks/useNotifications";
import { Notification, NotificationType, NotificationPriority } from "../lib/api/notifications";

interface NotificationPanelProps {
  className?: string;
}

type FilterType = "all" | "unread" | "high_priority";

// Icon mapping for notification types
const typeIcons: Record<NotificationType, React.ElementType> = {
  mar_reminder: Clock,
  mar_overdue: AlertTriangle,
  incident_reported: AlertTriangle,
  incident_escalated: AlertTriangle,
  shift_handoff: Calendar,
  shift_uncovered: Calendar,
  inventory_low: Package,
  training_due: BookOpen,
  family_message: MessageSquare,
  system_alert: Info,
  behavioral_alert: AlertTriangle,
  emergency: AlertTriangle,
};

// Color mapping for priorities
const priorityColors: Record<NotificationPriority, { bg: string; text: string; border: string }> = {
  low: { bg: "bg-gray-100", text: "text-gray-700", border: "border-gray-300" },
  normal: { bg: "bg-blue-100", text: "text-blue-700", border: "border-blue-300" },
  high: { bg: "bg-orange-100", text: "text-orange-700", border: "border-orange-300" },
  urgent: { bg: "bg-red-100", text: "text-red-700", border: "border-red-300" },
};

// Type labels
const typeLabels: Record<NotificationType, string> = {
  mar_reminder: "Medication Reminder",
  mar_overdue: "Medication Overdue",
  incident_reported: "Incident Reported",
  incident_escalated: "Incident Escalated",
  shift_handoff: "Shift Handoff",
  shift_uncovered: "Shift Uncovered",
  inventory_low: "Low Inventory",
  training_due: "Training Due",
  family_message: "Family Message",
  system_alert: "System Alert",
  behavioral_alert: "Behavioral Alert",
  emergency: "Emergency",
};

// Priority labels
const priorityLabels: Record<NotificationPriority, string> = {
  low: "Low",
  normal: "Normal",
  high: "High",
  urgent: "Urgent",
};

export function NotificationPanel({ className }: NotificationPanelProps) {
  const [filter, setFilter] = useState<FilterType>("all");
  const [selectedTypes, setSelectedTypes] = useState<NotificationType[]>([]);
  const [showTypeFilter, setShowTypeFilter] = useState(false);
  
  const {
    notifications,
    unreadCount,
    priorityNotifications,
    isLoading,
    markAsRead,
    markAllAsRead,
    deleteNotification,
  } = useNotifications({ enableRealtime: true });

  // Filter notifications
  const filteredNotifications = notifications.filter((notification) => {
    // Filter by read status/priority
    if (filter === "unread" && notification.read) return false;
    if (filter === "high_priority" && notification.priority !== "high" && notification.priority !== "urgent") return false;
    
    // Filter by type
    if (selectedTypes.length > 0 && !selectedTypes.includes(notification.type)) return false;
    
    return true;
  });

  const toggleTypeFilter = (type: NotificationType) => {
    setSelectedTypes((prev) =>
      prev.includes(type) ? prev.filter((t) => t !== type) : [...prev, type]
    );
  };

  const formatTime = (dateString: string) => {
    const date = new Date(dateString);
    const now = new Date();
    const diffMs = now.getTime() - date.getTime();
    const diffMins = Math.floor(diffMs / 60000);
    const diffHours = Math.floor(diffMs / 3600000);
    const diffDays = Math.floor(diffMs / 86400000);

    if (diffMins < 1) return "Just now";
    if (diffMins < 60) return `${diffMins} minutes ago`;
    if (diffHours < 24) return `${diffHours} hours ago`;
    if (diffDays === 1) return "Yesterday";
    if (diffDays < 7) return `${diffDays} days ago`;
    return date.toLocaleDateString("en-US", {
      month: "short",
      day: "numeric",
      year: date.getFullYear() !== now.getFullYear() ? "numeric" : undefined,
    });
  };

  const formatFullDate = (dateString: string) => {
    const date = new Date(dateString);
    return date.toLocaleString("en-US", {
      month: "long",
      day: "numeric",
      year: "numeric",
      hour: "numeric",
      minute: "2-digit",
    });
  };

  return (
    <div className={clsx("bg-white dark:bg-gray-900 rounded-lg shadow-sm border border-gray-200 dark:border-gray-700", className)}>
      {/* Header */}
      <div className="px-6 py-4 border-b border-gray-200 dark:border-gray-700">
        <div className="flex items-center justify-between flex-wrap gap-4">
          <div>
            <h2 className="text-xl font-semibold text-gray-900 dark:text-white flex items-center gap-2">
              <Bell className="w-5 h-5" />
              Notifications
              {unreadCount > 0 && (
                <span className="px-2.5 py-0.5 text-sm bg-red-100 text-red-700 dark:bg-red-900/30 dark:text-red-300 rounded-full">
                  {unreadCount} unread
                </span>
              )}
            </h2>
            <p className="text-sm text-gray-500 dark:text-gray-400 mt-1">
              Stay updated with important alerts and messages
            </p>
          </div>

          <div className="flex items-center gap-2">
            {unreadCount > 0 && (
              <button
                onClick={() => markAllAsRead()}
                className="flex items-center gap-2 px-4 py-2 text-sm font-medium text-blue-600 dark:text-blue-400 hover:bg-blue-50 dark:hover:bg-blue-900/20 rounded-lg transition-colors"
              >
                <CheckCheck className="w-4 h-4" />
                Mark all read
              </button>
            )}
          </div>
        </div>

        {/* Filters */}
        <div className="flex flex-wrap items-center gap-3 mt-4">
          {/* Filter Tabs */}
          <div className="flex items-center bg-gray-100 dark:bg-gray-800 rounded-lg p-1">
            {(["all", "unread", "high_priority"] as FilterType[]).map((f) => (
              <button
                key={f}
                onClick={() => setFilter(f)}
                className={clsx(
                  "px-3 py-1.5 text-sm font-medium rounded-md transition-colors",
                  filter === f
                    ? "bg-white dark:bg-gray-700 text-gray-900 dark:text-white shadow-sm"
                    : "text-gray-600 dark:text-gray-400 hover:text-gray-900 dark:hover:text-white"
                )}
              >
                {f === "all" && "All"}
                {f === "unread" && `Unread (${unreadCount})`}
                {f === "high_priority" && `Priority (${priorityNotifications.length})`}
              </button>
            ))}
          </div>

          {/* Type Filter Dropdown */}
          <div className="relative">
            <button
              onClick={() => setShowTypeFilter(!showTypeFilter)}
              className={clsx(
                "flex items-center gap-2 px-3 py-1.5 text-sm font-medium rounded-lg border transition-colors",
                selectedTypes.length > 0
                  ? "border-blue-500 text-blue-600 dark:text-blue-400 bg-blue-50 dark:bg-blue-900/20"
                  : "border-gray-300 dark:border-gray-600 text-gray-700 dark:text-gray-300 hover:bg-gray-50 dark:hover:bg-gray-800"
              )}
            >
              <Filter className="w-4 h-4" />
              Filter by Type
              {selectedTypes.length > 0 && (
                <span className="px-1.5 py-0.5 text-xs bg-blue-500 text-white rounded-full">
                  {selectedTypes.length}
                </span>
              )}
              <ChevronDown className="w-4 h-4" />
            </button>

            {showTypeFilter && (
              <div className="absolute top-full left-0 mt-2 w-64 bg-white dark:bg-gray-900 rounded-lg shadow-lg border border-gray-200 dark:border-gray-700 z-10">
                <div className="p-2">
                  <div className="flex items-center justify-between px-2 py-1.5">
                    <span className="text-sm font-medium text-gray-700 dark:text-gray-300">
                      Notification Types
                    </span>
                    {selectedTypes.length > 0 && (
                      <button
                        onClick={() => setSelectedTypes([])}
                        className="text-xs text-blue-600 hover:underline"
                      >
                        Clear all
                      </button>
                    )}
                  </div>
                  <div className="mt-1 space-y-1">
                    {(Object.keys(typeLabels) as NotificationType[]).map((type) => (
                      <button
                        key={type}
                        onClick={() => toggleTypeFilter(type)}
                        className={clsx(
                          "w-full flex items-center gap-2 px-2 py-1.5 text-sm rounded-md transition-colors",
                          selectedTypes.includes(type)
                            ? "bg-blue-50 dark:bg-blue-900/20 text-blue-700 dark:text-blue-300"
                            : "hover:bg-gray-100 dark:hover:bg-gray-800 text-gray-700 dark:text-gray-300"
                        )}
                      >
                        <div
                          className={clsx(
                            "w-4 h-4 rounded border flex items-center justify-center",
                            selectedTypes.includes(type)
                              ? "bg-blue-500 border-blue-500"
                              : "border-gray-300 dark:border-gray-600"
                          )}
                        >
                          {selectedTypes.includes(type) && (
                            <Check className="w-3 h-3 text-white" />
                          )}
                        </div>
                        {typeLabels[type]}
                      </button>
                    ))}
                  </div>
                </div>
              </div>
            )}
          </div>
        </div>
      </div>

      {/* Notification List */}
      <div className="divide-y divide-gray-200 dark:divide-gray-700">
        {isLoading ? (
          <div className="p-12 text-center">
            <div className="animate-spin w-8 h-8 border-2 border-gray-300 border-t-blue-500 rounded-full mx-auto mb-4" />
            <p className="text-gray-500">Loading notifications...</p>
          </div>
        ) : filteredNotifications.length === 0 ? (
          <div className="p-12 text-center">
            <div className="w-16 h-16 bg-gray-100 dark:bg-gray-800 rounded-full flex items-center justify-center mx-auto mb-4">
              <Bell className="w-8 h-8 text-gray-400" />
            </div>
            <h3 className="text-lg font-medium text-gray-900 dark:text-white mb-1">
              No notifications
            </h3>
            <p className="text-gray-500">
              {filter === "all" && selectedTypes.length === 0
                ? "You're all caught up! We'll notify you when something important happens."
                : "No notifications match your current filters."}
            </p>
          </div>
        ) : (
          filteredNotifications.map((notification) => (
            <NotificationRow
              key={notification.id}
              notification={notification}
              onMarkAsRead={() => markAsRead(notification.id)}
              onDelete={() => deleteNotification(notification.id)}
              formatTime={formatTime}
              formatFullDate={formatFullDate}
            />
          ))
        )}
      </div>
    </div>
  );
}

// Notification Row Component
interface NotificationRowProps {
  notification: Notification;
  onMarkAsRead: () => void;
  onDelete: () => void;
  formatTime: (date: string) => string;
  formatFullDate: (date: string) => string;
}

function NotificationRow({
  notification,
  onMarkAsRead,
  onDelete,
  formatTime,
  formatFullDate,
}: NotificationRowProps) {
  const Icon = typeIcons[notification.type] || Info;
  const priorityStyle = priorityColors[notification.priority];
  const [isExpanded, setIsExpanded] = useState(false);

  return (
    <div
      className={clsx(
        "p-4 hover:bg-gray-50 dark:hover:bg-gray-800/50 transition-colors",
        !notification.read && "bg-blue-50/30 dark:bg-blue-900/5"
      )}
    >
      <div className="flex items-start gap-4">
        {/* Icon */}
        <div
          className={clsx(
            "p-2 rounded-lg shrink-0",
            priorityStyle.bg
          )}
        >
          <Icon className={clsx("w-5 h-5", priorityStyle.text)} />
        </div>

        {/* Content */}
        <div className="flex-1 min-w-0">
          <div className="flex items-start justify-between gap-4">
            <div className="flex-1">
              <div className="flex items-center gap-2 flex-wrap">
                <h3
                  className={clsx(
                    "font-medium",
                    !notification.read
                      ? "text-gray-900 dark:text-white"
                      : "text-gray-700 dark:text-gray-300"
                  )}
                >
                  {notification.title}
                </h3>
                {!notification.read && (
                  <span className="w-2 h-2 bg-blue-500 rounded-full" />
                )}
                <span
                  className={clsx(
                    "px-2 py-0.5 text-xs font-medium rounded-full border",
                    priorityStyle.bg,
                    priorityStyle.text,
                    priorityStyle.border
                  )}
                >
                  {priorityLabels[notification.priority]}
                </span>
              </div>

              <p className="text-sm text-gray-600 dark:text-gray-400 mt-1">
                {notification.message}
              </p>

              {/* Additional Data */}
              {notification.data && Object.keys(notification.data).length > 0 && isExpanded && (
                <div className="mt-3 p-3 bg-gray-100 dark:bg-gray-800 rounded-lg">
                  <pre className="text-xs text-gray-600 dark:text-gray-400 overflow-x-auto">
                    {JSON.stringify(notification.data, null, 2)}
                  </pre>
                </div>
              )}

              {/* Meta */}
              <div className="flex items-center gap-3 mt-2 text-sm text-gray-500">
                <span title={formatFullDate(notification.created_at)}>
                  {formatTime(notification.created_at)}
                </span>
                <span>•</span>
                <span>{typeLabels[notification.type]}</span>
                {notification.data && Object.keys(notification.data).length > 0 && (
                  <>
                    <span>•</span>
                    <button
                      onClick={() => setIsExpanded(!isExpanded)}
                      className="text-blue-600 hover:underline"
                    >
                      {isExpanded ? "Hide details" : "Show details"}
                    </button>
                  </>
                )}
              </div>
            </div>

            {/* Actions */}
            <div className="flex items-center gap-1">
              {!notification.read && (
                <button
                  onClick={onMarkAsRead}
                  className="p-2 text-gray-400 hover:text-blue-600 hover:bg-blue-50 dark:hover:bg-blue-900/20 rounded-lg transition-colors"
                  title="Mark as read"
                >
                  <Check className="w-4 h-4" />
                </button>
              )}
              <button
                onClick={onDelete}
                className="p-2 text-gray-400 hover:text-red-600 hover:bg-red-50 dark:hover:bg-red-900/20 rounded-lg transition-colors"
                title="Delete"
              >
                <Trash2 className="w-4 h-4" />
              </button>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}

export default NotificationPanel;

"use client";

import React, { useState, useRef, useEffect } from "react";
import { Bell, Check, Trash2, AlertTriangle, Clock, Package, MessageSquare, Calendar, BookOpen, Info, X } from "lucide-react";
import { clsx } from "clsx";
import { useNotifications } from "../hooks/useNotifications";
import { Notification, NotificationType, NotificationPriority } from "../lib/api/notifications";

interface NotificationBellProps {
  className?: string;
}

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
const priorityColors: Record<NotificationPriority, string> = {
  low: "bg-gray-400",
  normal: "bg-blue-500",
  high: "bg-orange-500",
  urgent: "bg-red-600",
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

export function NotificationBell({ className }: NotificationBellProps) {
  const [isOpen, setIsOpen] = useState(false);
  const dropdownRef = useRef<HTMLDivElement>(null);
  const {
    notifications,
    unreadCount,
    unreadNotifications,
    isLoading,
    markAsRead,
    markAllAsRead,
    deleteNotification,
  } = useNotifications({ enableRealtime: true });

  // Close dropdown when clicking outside
  useEffect(() => {
    function handleClickOutside(event: MouseEvent) {
      if (dropdownRef.current && !dropdownRef.current.contains(event.target as Node)) {
        setIsOpen(false);
      }
    }

    document.addEventListener("mousedown", handleClickOutside);
    return () => document.removeEventListener("mousedown", handleClickOutside);
  }, []);

  const handleMarkAsRead = (e: React.MouseEvent, id: string) => {
    e.stopPropagation();
    markAsRead(id);
  };

  const handleDelete = (e: React.MouseEvent, id: string) => {
    e.stopPropagation();
    deleteNotification(id);
  };

  const formatTime = (dateString: string) => {
    const date = new Date(dateString);
    const now = new Date();
    const diffMs = now.getTime() - date.getTime();
    const diffMins = Math.floor(diffMs / 60000);
    const diffHours = Math.floor(diffMs / 3600000);
    const diffDays = Math.floor(diffMs / 86400000);

    if (diffMins < 1) return "Just now";
    if (diffMins < 60) return `${diffMins}m ago`;
    if (diffHours < 24) return `${diffHours}h ago`;
    if (diffDays < 7) return `${diffDays}d ago`;
    return date.toLocaleDateString();
  };

  return (
    <div className={clsx("relative", className)} ref={dropdownRef}>
      {/* Bell Button */}
      <button
        onClick={() => setIsOpen(!isOpen)}
        className={clsx(
          "relative p-2 rounded-full transition-colors",
          "hover:bg-gray-100 dark:hover:bg-gray-800",
          isOpen && "bg-gray-100 dark:bg-gray-800"
        )}
        aria-label={`Notifications ${unreadCount > 0 ? `(${unreadCount} unread)` : ""}`}
      >
        <Bell className="w-5 h-5 text-gray-600 dark:text-gray-300" />
        
        {/* Unread Badge */}
        {unreadCount > 0 && (
          <span className="absolute -top-1 -right-1 min-w-[18px] h-[18px] px-1 flex items-center justify-center bg-red-500 text-white text-xs font-bold rounded-full">
            {unreadCount > 99 ? "99+" : unreadCount}
          </span>
        )}
      </button>

      {/* Dropdown */}
      {isOpen && (
        <div className="absolute right-0 mt-2 w-96 max-w-[calc(100vw-2rem)] bg-white dark:bg-gray-900 rounded-lg shadow-lg border border-gray-200 dark:border-gray-700 z-50">
          {/* Header */}
          <div className="flex items-center justify-between px-4 py-3 border-b border-gray-200 dark:border-gray-700">
            <h3 className="font-semibold text-gray-900 dark:text-white">Notifications</h3>
            <div className="flex items-center gap-2">
              {unreadCount > 0 && (
                <button
                  onClick={() => markAllAsRead()}
                  className="text-sm text-blue-600 dark:text-blue-400 hover:underline"
                >
                  Mark all read
                </button>
              )}
              <button
                onClick={() => setIsOpen(false)}
                className="p-1 rounded hover:bg-gray-100 dark:hover:bg-gray-800"
              >
                <X className="w-4 h-4 text-gray-500" />
              </button>
            </div>
          </div>

          {/* Notification List */}
          <div className="max-h-[400px] overflow-y-auto">
            {isLoading ? (
              <div className="p-4 text-center text-gray-500">
                <div className="animate-spin w-6 h-6 border-2 border-gray-300 border-t-blue-500 rounded-full mx-auto mb-2" />
                Loading...
              </div>
            ) : notifications.length === 0 ? (
              <div className="p-8 text-center text-gray-500">
                <Bell className="w-12 h-12 mx-auto mb-3 text-gray-300" />
                <p>No notifications yet</p>
              </div>
            ) : (
              <div className="divide-y divide-gray-100 dark:divide-gray-800">
                {notifications.map((notification) => (
                  <NotificationItem
                    key={notification.id}
                    notification={notification}
                    onMarkAsRead={(e) => handleMarkAsRead(e, notification.id)}
                    onDelete={(e) => handleDelete(e, notification.id)}
                    formatTime={formatTime}
                  />
                ))}
              </div>
            )}
          </div>

          {/* Footer */}
          <div className="px-4 py-2 border-t border-gray-200 dark:border-gray-700 text-center">
            <a
              href="/notifications"
              className="text-sm text-blue-600 dark:text-blue-400 hover:underline"
              onClick={() => setIsOpen(false)}
            >
              View all notifications
            </a>
          </div>
        </div>
      )}
    </div>
  );
}

// Individual Notification Item Component
interface NotificationItemProps {
  notification: Notification;
  onMarkAsRead: (e: React.MouseEvent) => void;
  onDelete: (e: React.MouseEvent) => void;
  formatTime: (date: string) => string;
}

function NotificationItem({ notification, onMarkAsRead, onDelete, formatTime }: NotificationItemProps) {
  const Icon = typeIcons[notification.type] || Info;
  const priorityColor = priorityColors[notification.priority];

  return (
    <div
      className={clsx(
        "relative p-4 hover:bg-gray-50 dark:hover:bg-gray-800/50 transition-colors group",
        !notification.read && "bg-blue-50/50 dark:bg-blue-900/10"
      )}
    >
      {/* Priority Indicator */}
      <div className={clsx("absolute left-0 top-0 bottom-0 w-1", priorityColor)} />

      <div className="flex items-start gap-3">
        {/* Icon */}
        <div className={clsx("p-2 rounded-full shrink-0", priorityColor.replace("bg-", "bg-opacity-10 bg-"))}>
          <Icon className={clsx("w-4 h-4", priorityColor.replace("bg-", "text-").replace("600", "600").replace("500", "500").replace("400", "400"))} />
        </div>

        {/* Content */}
        <div className="flex-1 min-w-0">
          <div className="flex items-start justify-between gap-2">
            <div>
              <p className={clsx("text-sm font-medium", !notification.read && "text-gray-900 dark:text-white")}>
                {notification.title}
              </p>
              <p className="text-sm text-gray-600 dark:text-gray-400 mt-0.5 line-clamp-2">
                {notification.message}
              </p>
            </div>
            
            {/* Actions */}
            <div className="flex items-center gap-1 opacity-0 group-hover:opacity-100 transition-opacity">
              {!notification.read && (
                <button
                  onClick={onMarkAsRead}
                  className="p-1.5 rounded hover:bg-gray-200 dark:hover:bg-gray-700 text-blue-600"
                  title="Mark as read"
                >
                  <Check className="w-4 h-4" />
                </button>
              )}
              <button
                onClick={onDelete}
                className="p-1.5 rounded hover:bg-gray-200 dark:hover:bg-gray-700 text-red-500"
                title="Delete"
              >
                <Trash2 className="w-4 h-4" />
              </button>
            </div>
          </div>

          {/* Meta */}
          <div className="flex items-center gap-2 mt-1.5">
            <span className="text-xs text-gray-500">
              {formatTime(notification.created_at)}
            </span>
            <span className="text-xs text-gray-400">•</span>
            <span className="text-xs text-gray-500">
              {typeLabels[notification.type]}
            </span>
            {!notification.read && (
              <>
                <span className="text-xs text-gray-400">•</span>
                <span className="text-xs text-blue-600 font-medium">Unread</span>
              </>
            )}
          </div>
        </div>
      </div>
    </div>
  );
}

export default NotificationBell;

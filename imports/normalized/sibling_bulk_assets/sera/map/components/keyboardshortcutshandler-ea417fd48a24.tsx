/**
 * Keyboard Shortcuts Handler
 * Handles global shortcut events and provides toast feedback
 */

'use client';

import { useEffect, useCallback } from 'react';
import { useRouter } from 'next/navigation';
import { useTheme } from '@/lib/theme';
import { useToast } from './ui/use-toast';
import { useShortcutContext, useGlobalShortcuts } from '@/lib/shortcuts';
import { ShortcutsHelpModal } from '@/lib/shortcuts/ShortcutsHelpModal';
import { 
  Moon, 
  Sun, 
  Bell, 
  Search, 
  Save,
  MessageSquare,
  AlertTriangle,
  Users,
  Navigation
} from 'lucide-react';

interface KeyboardShortcutsHandlerProps {
  children: React.ReactNode;
}

export function KeyboardShortcutsHandler({ children }: KeyboardShortcutsHandlerProps) {
  const router = useRouter();
  const { resolvedTheme, toggleTheme } = useTheme();
  const { toast } = useToast();
  const { isHelpOpen, hideHelp, showHelp, userRole } = useShortcutContext();
  
  // Register global shortcuts
  useGlobalShortcuts();

  // Handle shortcut events
  const handleSave = useCallback(() => {
    toast({
      title: 'Save Triggered',
      description: 'Attempting to save your changes...',
      icon: <Save className="w-4 h-4" />,
    });
    
    // Dispatch to any listening forms
    document.dispatchEvent(new CustomEvent('form-save-request'));
  }, [toast]);

  const handleSearch = useCallback(() => {
    toast({
      title: 'Search',
      description: 'Opening global search...',
      icon: <Search className="w-4 h-4" />,
    });
    
    // Focus on any search input
    const searchInput = document.querySelector('[data-search-input]') as HTMLInputElement;
    if (searchInput) {
      searchInput.focus();
    }
  }, [toast]);

  const handleToggleTheme = useCallback(() => {
    toggleTheme();
    toast({
      title: resolvedTheme === 'dark' ? 'Light Mode Enabled' : 'Dark Mode Enabled',
      description: 'Theme preference saved.',
      icon: resolvedTheme === 'dark' ? <Sun className="w-4 h-4" /> : <Moon className="w-4 h-4" />,
    });
  }, [toggleTheme, resolvedTheme, toast]);

  const handleToggleNotifications = useCallback(() => {
    toast({
      title: 'Notifications',
      description: 'Toggling notification panel...',
      icon: <Bell className="w-4 h-4" />,
    });
    
    // Dispatch event for notification panel
    document.dispatchEvent(new CustomEvent('toggle-notifications'));
  }, [toast]);

  const handleQuickNav = useCallback(() => {
    toast({
      title: 'Quick Navigation',
      description: 'Use Ctrl+letter to navigate: D=Dashboard, M=MAR, I=Incidents, L=Logs',
      icon: <Navigation className="w-4 h-4" />,
      duration: 5000,
    });
  }, [toast]);

  const handleCloseModal = useCallback(() => {
    // Find and close any open modals
    const closeButtons = document.querySelectorAll('[data-modal-close]');
    if (closeButtons.length > 0) {
      (closeButtons[closeButtons.length - 1] as HTMLButtonElement).click();
    }
    
    // Also dispatch close event
    document.dispatchEvent(new CustomEvent('close-modal'));
    document.dispatchEvent(new CustomEvent('close-dropdown'));
  }, []);

  const handleBroadcast = useCallback(() => {
    toast({
      title: 'Broadcast Message',
      description: 'Opening broadcast message composer...',
      icon: <MessageSquare className="w-4 h-4" />,
    });
    
    document.dispatchEvent(new CustomEvent('open-broadcast'));
  }, [toast]);

  const handleCriticalAlerts = useCallback(() => {
    toast({
      title: 'Critical Alerts',
      description: 'Viewing critical system alerts...',
      icon: <AlertTriangle className="w-4 h-4" />,
      variant: 'destructive',
    });
    
    document.dispatchEvent(new CustomEvent('view-critical-alerts'));
  }, [toast]);

  const handleStaffOverview = useCallback(() => {
    toast({
      title: 'Staff Overview',
      description: 'Opening staff monitoring dashboard...',
      icon: <Users className="w-4 h-4" />,
    });
    
    router.push('/supervisor/staff');
  }, [router, toast]);

  // Register event listeners for shortcut actions
  useEffect(() => {
    const handleShortcutEvent = (event: CustomEvent) => {
      const { shortcutId, description } = event.detail || {};
      
      if (shortcutId) {
        toast({
          title: 'Shortcut Activated',
          description: description || `Executed: ${shortcutId}`,
          duration: 1500,
        });
      }
    };

    document.addEventListener('shortcut-activated', handleShortcutEvent as EventListener);
    return () => {
      document.removeEventListener('shortcut-activated', handleShortcutEvent as EventListener);
    };
  }, [toast]);

  // Listen for custom shortcut events
  useEffect(() => {
    document.addEventListener('shortcut-save', handleSave);
    document.addEventListener('shortcut-search', handleSearch);
    document.addEventListener('shortcut-toggle-theme', handleToggleTheme);
    document.addEventListener('shortcut-toggle-notifications', handleToggleNotifications);
    document.addEventListener('shortcut-quick-nav', handleQuickNav);
    document.addEventListener('shortcut-close-modal', handleCloseModal);
    document.addEventListener('shortcut-show-help', showHelp);
    document.addEventListener('shortcut-broadcast', handleBroadcast);
    document.addEventListener('shortcut-critical-alerts', handleCriticalAlerts);
    document.addEventListener('shortcut-staff-overview', handleStaffOverview);

    return () => {
      document.removeEventListener('shortcut-save', handleSave);
      document.removeEventListener('shortcut-search', handleSearch);
      document.removeEventListener('shortcut-toggle-theme', handleToggleTheme);
      document.removeEventListener('shortcut-toggle-notifications', handleToggleNotifications);
      document.removeEventListener('shortcut-quick-nav', handleQuickNav);
      document.removeEventListener('shortcut-close-modal', handleCloseModal);
      document.removeEventListener('shortcut-show-help', showHelp);
      document.removeEventListener('shortcut-broadcast', handleBroadcast);
      document.removeEventListener('shortcut-critical-alerts', handleCriticalAlerts);
      document.removeEventListener('shortcut-staff-overview', handleStaffOverview);
    };
  }, [
    handleSave, 
    handleSearch, 
    handleToggleTheme, 
    handleToggleNotifications,
    handleQuickNav,
    handleCloseModal,
    showHelp,
    handleBroadcast,
    handleCriticalAlerts,
    handleStaffOverview
  ]);

  return (
    <>
      {children}
      <ShortcutsHelpModal 
        isOpen={isHelpOpen} 
        onClose={hideHelp}
        userRole={userRole}
      />
    </>
  );
}

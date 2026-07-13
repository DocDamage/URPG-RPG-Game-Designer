"use client";
import Link from "next/link";
import { usePathname } from "next/navigation";
import { ThemeToggle } from "@/lib/theme";
import { Button } from "./ui/button";
import { ChevronDown, Shield, Building2, FileText, Palette, Keyboard, HelpCircle } from "lucide-react";
import { useShortcutContext } from "@/lib/shortcuts";
import { clsx } from "clsx";
import { useOfflineSync } from "../lib/hooks/useOfflineSync";
import { Badge } from "./ui/badge";
import NotificationBell from "./NotificationBell";
import {
  DropdownMenu,
  DropdownMenuContent,
  DropdownMenuItem,
  DropdownMenuTrigger,
  DropdownMenuSeparator,
} from "@/components/ui/dropdown-menu";
import { useEffect, useState } from "react";

interface User {
  id: string;
  email: string;
  name: string;
  role: string;
}

const mainLinks = [
  { href: "/", label: "Home" },
  { href: "/dashboard", label: "Dashboard" },
  { href: "/records", label: "Records" },
  { href: "/prn", label: "PRN" },
  { href: "/mar", label: "MAR" },
  { href: "/briefing", label: "Briefing" },
  { href: "/scheduling", label: "Scheduling" },
  { href: "/training", label: "Training" },
  { href: "/gamification", label: "Achievements" },
  { href: "/documents", label: "Documents" },
  { href: "/medication-inventory", label: "Inventory" },
  { href: "/location", label: "Location" },
];

const supervisorLinks = [
  { href: "/supervisor", label: "Supervisor Dashboard" },
  { href: "/supervisor/analytics", label: "Analytics" },
  { href: "/supervisor/staff", label: "Staff" },
  { href: "/supervisor/scheduling", label: "Scheduling" },
  { href: "/supervisor/reports", label: "Reports" },
];

const complianceLinks = [
  { href: "/compliance/state-reports", label: "State Reports" },
  { href: "/compliance/hipaa", label: "HIPAA Compliance" },
  { href: "/compliance/policies", label: "Policies" },
];

const adminLinks = [
  { href: "/admin/organizations", label: "Organizations", icon: Building2 },
  { href: "/admin/branding", label: "Branding", icon: Palette },
];

export default function Navbar() {
  const pathname = usePathname();
  const { isOnline, isSyncing, pendingItems } = useOfflineSync();
  const [user, setUser] = useState<User | null>(null);
  const { showHelp } = useShortcutContext();

  useEffect(() => {
    // Load user from localStorage
    const storedUser = localStorage.getItem('sera_user');
    if (storedUser) {
      try {
        setUser(JSON.parse(storedUser));
      } catch {
        setUser(null);
      }
    }
  }, []);

  const isAdmin = user?.role === 'admin' || user?.role === 'super_admin';
  const isSupervisor = user?.role === 'supervisor' || isAdmin;
  const isCompliance = ['supervisor', 'admin', 'compliance_officer', 'super_admin'].includes(user?.role || '');

  const isActive = (href: string) => pathname === href || pathname.startsWith(`${href}/`);

  return (
    <header className="w-full border-b border-gray-200 dark:border-gray-700 bg-white dark:bg-gray-900 px-4 py-2 flex items-center justify-between theme-transition">
      <div className="flex items-center gap-4">
        <h1 className="font-bold text-lg text-gray-900 dark:text-gray-100">S.E.R.A.</h1>
        <nav className="hidden lg:flex gap-4">
          {/* Main Navigation */}
          {mainLinks.map((link) => (
            <Link
              key={link.href}
              href={link.href}
              className={clsx(
                "text-sm font-medium transition-colors",
                isActive(link.href)
                  ? "text-primary dark:text-primary-light"
                  : "text-gray-600 dark:text-gray-300 hover:text-primary dark:hover:text-primary-light"
              )}
            >
              {link.label}
            </Link>
          ))}

          {/* Supervisor Dropdown */}
          {isSupervisor && (
            <DropdownMenu>
              <DropdownMenuTrigger asChild>
                <button className={clsx(
                  "text-sm font-medium flex items-center gap-1 transition-colors",
                  isActive("/supervisor")
                    ? "text-primary dark:text-primary-light"
                    : "text-gray-600 dark:text-gray-300 hover:text-primary dark:hover:text-primary-light"
                )}>
                  Supervisor
                  <ChevronDown className="w-3 h-3" />
                </button>
              </DropdownMenuTrigger>
              <DropdownMenuContent align="start" className="w-48">
                {supervisorLinks.map((link) => (
                  <DropdownMenuItem key={link.href} asChild>
                    <Link href={link.href} className="cursor-pointer">
                      {link.label}
                    </Link>
                  </DropdownMenuItem>
                ))}
              </DropdownMenuContent>
            </DropdownMenu>
          )}

          {/* Compliance Dropdown */}
          {isCompliance && (
            <DropdownMenu>
              <DropdownMenuTrigger asChild>
                <button className={clsx(
                  "text-sm font-medium flex items-center gap-1 transition-colors",
                  isActive("/compliance")
                    ? "text-primary dark:text-primary-light"
                    : "text-gray-600 dark:text-gray-300 hover:text-primary dark:hover:text-primary-light"
                )}>
                  <Shield className="w-3 h-3" />
                  Compliance
                  <ChevronDown className="w-3 h-3" />
                </button>
              </DropdownMenuTrigger>
              <DropdownMenuContent align="start" className="w-48">
                {complianceLinks.map((link) => (
                  <DropdownMenuItem key={link.href} asChild>
                    <Link href={link.href} className="cursor-pointer">
                      {link.label}
                    </Link>
                  </DropdownMenuItem>
                ))}
              </DropdownMenuContent>
            </DropdownMenu>
          )}

          {/* Admin Dropdown - Admin Only */}
          {isAdmin && (
            <DropdownMenu>
              <DropdownMenuTrigger asChild>
                <button className={clsx(
                  "text-sm font-medium flex items-center gap-1 transition-colors",
                  isActive("/admin")
                    ? "text-primary dark:text-primary-light"
                    : "text-gray-600 dark:text-gray-300 hover:text-primary dark:hover:text-primary-light"
                )}>
                  Admin
                  <ChevronDown className="w-3 h-3" />
                </button>
              </DropdownMenuTrigger>
              <DropdownMenuContent align="start" className="w-48">
                {adminLinks.map((link) => (
                  <DropdownMenuItem key={link.href} asChild>
                    <Link href={link.href} className="cursor-pointer flex items-center gap-2">
                      <link.icon className="w-4 h-4" />
                      {link.label}
                    </Link>
                  </DropdownMenuItem>
                ))}
                <DropdownMenuSeparator />
                <DropdownMenuItem asChild>
                  <Link href="/analytics" className="cursor-pointer flex items-center gap-2">
                    <FileText className="w-4 h-4" />
                    System Analytics
                  </Link>
                </DropdownMenuItem>
              </DropdownMenuContent>
            </DropdownMenu>
          )}
        </nav>
      </div>
      <div className="flex items-center gap-3">
        {/* Offline/Sync Status */}
        {!isOnline && (
          <Badge variant="warning" size="sm">
            Offline {pendingItems > 0 && `(${pendingItems})`}
          </Badge>
        )}
        {isOnline && isSyncing && (
          <Badge variant="info" size="sm">
            Syncing...
          </Badge>
        )}

        <NotificationBell />
        
        {/* Keyboard Shortcuts Help Button */}
        <Button 
          variant="outline" 
          size="sm" 
          onClick={showHelp}
          className="gap-1.5 hidden sm:flex"
          title="Keyboard Shortcuts (? for help)"
          aria-label="Show keyboard shortcuts help"
        >
          <Keyboard className="w-4 h-4" />
          <kbd className="hidden md:inline-block px-1 py-0 bg-gray-100 dark:bg-gray-800 rounded text-[10px]">?</kbd>
        </Button>
        
        {/* Mobile Help Button */}
        <Button 
          variant="outline" 
          size="sm" 
          onClick={showHelp}
          className="sm:hidden p-2"
          title="Keyboard Shortcuts"
          aria-label="Show keyboard shortcuts help"
        >
          <HelpCircle className="w-4 h-4" />
        </Button>
        
        {/* Theme Toggle with Dropdown */}
        <ThemeToggle 
          variant="outline" 
          size="sm" 
          showDropdown 
          className="hidden sm:flex"
        />
        
        {/* Mobile: Simple toggle without dropdown */}
        <ThemeToggle 
          variant="outline" 
          size="sm" 
          showDropdown={false}
          className="sm:hidden"
        />
      </div>
    </header>
  );
}

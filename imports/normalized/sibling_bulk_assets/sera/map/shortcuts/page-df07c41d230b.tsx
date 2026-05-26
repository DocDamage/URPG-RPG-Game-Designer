"use client";

/**
 * Keyboard Shortcuts Help Page
 * 
 * Displays all available keyboard shortcuts including theme shortcuts.
 */

import { useEffect } from 'react';
import { useShortcutHelp, registerModifierShortcut } from '@/hooks/useKeyboardShortcuts';
import { useTheme } from '@/lib/theme';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Badge } from '@/components/ui/badge';
import { Keyboard, Sun, Moon, Monitor, Command } from 'lucide-react';

export default function ShortcutsPage() {
  const shortcuts = useShortcutHelp();
  const { toggleTheme } = useTheme();
  
  // Register theme toggle shortcut
  useEffect(() => {
    const unregister = registerModifierShortcut({
      key: 'l',
      ctrl: true,
      shift: true,
      action: () => toggleTheme(),
      description: 'Toggle Light/Dark Theme',
    });
    
    return unregister;
  }, [toggleTheme]);

  // Group shortcuts by category
  const navigationShortcuts = shortcuts.filter(s => 
    s.shortcut.startsWith('g ') || s.shortcut === '/' || s.shortcut === '?'
  );
  
  const actionShortcuts = shortcuts.filter(s => 
    s.shortcut.startsWith('n ')
  );
  
  const modifierShortcuts = shortcuts.filter(s => 
    s.shortcut.includes('Ctrl') || s.shortcut.includes('Shift') || s.shortcut.includes('Alt')
  );
  
  const otherShortcuts = shortcuts.filter(s => 
    !navigationShortcuts.includes(s) && 
    !actionShortcuts.includes(s) && 
    !modifierShortcuts.includes(s)
  );

  const ShortcutItem = ({ shortcut, description }: { shortcut: string; description: string }) => (
    <div className="flex items-center justify-between py-3 border-b border-gray-200 dark:border-gray-700 last:border-0">
      <span className="text-gray-700 dark:text-gray-300">{description}</span>
      <Badge variant="secondary" className="font-mono text-xs">
        {shortcut}
      </Badge>
    </div>
  );

  const SectionTitle = ({ icon: Icon, title }: { icon: any; title: string }) => (
    <div className="flex items-center gap-2 mb-4">
      <Icon className="w-5 h-5 text-primary dark:text-primary-light" />
      <h2 className="text-lg font-semibold text-gray-900 dark:text-gray-100">{title}</h2>
    </div>
  );

  return (
    <div className="container mx-auto px-4 py-8 max-w-4xl">
      <div className="mb-8">
        <h1 className="text-3xl font-bold text-gray-900 dark:text-gray-100 mb-2">
          Keyboard Shortcuts
        </h1>
        <p className="text-gray-600 dark:text-gray-400">
          Speed up your workflow with these keyboard shortcuts. Press <kbd className="px-2 py-1 bg-gray-100 dark:bg-gray-800 rounded text-sm font-mono">?</kbd> from anywhere to view this page.
        </p>
      </div>

      {/* Theme Toggle Highlight */}
      <Card className="mb-6 border-primary/20 dark:border-primary-light/20 bg-primary/5 dark:bg-primary-light/5">
        <CardContent className="p-6">
          <div className="flex items-start gap-4">
            <div className="p-3 bg-primary/10 dark:bg-primary-light/10 rounded-lg">
              <Command className="w-6 h-6 text-primary dark:text-primary-light" />
            </div>
            <div className="flex-1">
              <h3 className="text-lg font-semibold text-gray-900 dark:text-gray-100 mb-2">
                Quick Theme Toggle
              </h3>
              <p className="text-gray-600 dark:text-gray-400 mb-3">
                Quickly switch between light and dark modes without using the mouse.
              </p>
              <div className="flex items-center gap-2">
                <Badge variant="outline" className="font-mono">Ctrl</Badge>
                <span className="text-gray-400">+</span>
                <Badge variant="outline" className="font-mono">Shift</Badge>
                <span className="text-gray-400">+</span>
                <Badge variant="outline" className="font-mono">L</Badge>
              </div>
            </div>
            <div className="hidden sm:flex items-center gap-2">
              <Sun className="w-5 h-5 text-amber-500" />
              <span className="text-gray-400">↔</span>
              <Moon className="w-5 h-5 text-indigo-400" />
            </div>
          </div>
        </CardContent>
      </Card>

      <div className="grid md:grid-cols-2 gap-6">
        {/* Navigation Shortcuts */}
        <Card className="theme-transition">
          <CardHeader>
            <SectionTitle icon={Keyboard} title="Navigation" />
          </CardHeader>
          <CardContent className="pt-0">
            {navigationShortcuts.map((s, i) => (
              <ShortcutItem key={i} shortcut={s.shortcut} description={s.description} />
            ))}
          </CardContent>
        </Card>

        {/* Action Shortcuts */}
        <Card className="theme-transition">
          <CardHeader>
            <SectionTitle icon={Command} title="Quick Actions" />
          </CardHeader>
          <CardContent className="pt-0">
            {actionShortcuts.map((s, i) => (
              <ShortcutItem key={i} shortcut={s.shortcut} description={s.description} />
            ))}
          </CardContent>
        </Card>

        {/* Modifier Shortcuts */}
        <Card className="theme-transition">
          <CardHeader>
            <SectionTitle icon={Monitor} title="System & Theme" />
          </CardHeader>
          <CardContent className="pt-0">
            {modifierShortcuts.length > 0 ? (
              modifierShortcuts.map((s, i) => (
                <ShortcutItem key={i} shortcut={s.shortcut} description={s.description} />
              ))
            ) : (
              <p className="text-gray-500 dark:text-gray-400 py-3">No modifier shortcuts registered.</p>
            )}
          </CardContent>
        </Card>

        {/* Other Shortcuts */}
        <Card className="theme-transition">
          <CardHeader>
            <SectionTitle icon={Keyboard} title="General" />
          </CardHeader>
          <CardContent className="pt-0">
            {otherShortcuts.map((s, i) => (
              <ShortcutItem key={i} shortcut={s.shortcut} description={s.description} />
            ))}
          </CardContent>
        </Card>
      </div>

      {/* Tips */}
      <Card className="mt-6 theme-transition">
        <CardHeader>
          <CardTitle className="text-lg">Pro Tips</CardTitle>
        </CardHeader>
        <CardContent>
          <ul className="space-y-2 text-gray-600 dark:text-gray-400">
            <li className="flex items-start gap-2">
              <span className="text-primary dark:text-primary-light">•</span>
              <span>Shortcuts work from anywhere in the app (except when typing in input fields)</span>
            </li>
            <li className="flex items-start gap-2">
              <span className="text-primary dark:text-primary-light">•</span>
              <span>Multi-key shortcuts (like &quot;g d&quot;) can be pressed sequentially - no need to hold them together</span>
            </li>
            <li className="flex items-start gap-2">
              <span className="text-primary dark:text-primary-light">•</span>
              <span>Press <kbd className="px-2 py-0.5 bg-gray-100 dark:bg-gray-800 rounded text-sm font-mono">Esc</kbd> to close any open modal or dropdown</span>
            </li>
            <li className="flex items-start gap-2">
              <span className="text-primary dark:text-primary-light">•</span>
              <span>On Mac, use <kbd className="px-2 py-0.5 bg-gray-100 dark:bg-gray-800 rounded text-sm font-mono">⌘</kbd> instead of <kbd className="px-2 py-0.5 bg-gray-100 dark:bg-gray-800 rounded text-sm font-mono">Ctrl</kbd> for modifier shortcuts</span>
            </li>
          </ul>
        </CardContent>
      </Card>
    </div>
  );
}

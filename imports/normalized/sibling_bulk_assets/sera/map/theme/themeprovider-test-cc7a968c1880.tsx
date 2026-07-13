/**
 * Theme Provider Tests
 * 
 * Comprehensive tests for the S.E.R.A. Theme System.
 */

import React from 'react';
import { render, screen, fireEvent, waitFor } from '@testing-library/react';
import { ThemeProvider, useTheme } from '@/lib/theme';

// Mock localStorage
const localStorageMock = {
  getItem: jest.fn(),
  setItem: jest.fn(),
  removeItem: jest.fn(),
  clear: jest.fn(),
};
Object.defineProperty(window, 'localStorage', {
  value: localStorageMock,
});

// Mock matchMedia
Object.defineProperty(window, 'matchMedia', {
  writable: true,
  value: jest.fn().mockImplementation((query: string) => ({
    matches: false,
    media: query,
    onchange: null,
    addListener: jest.fn(),
    removeListener: jest.fn(),
    addEventListener: jest.fn(),
    removeEventListener: jest.fn(),
    dispatchEvent: jest.fn(),
  })),
});

// Test component that uses the theme
function TestComponent() {
  const { theme, resolvedTheme, isDark, setTheme, toggleTheme } = useTheme();
  
  return (
    <div>
      <div data-testid="theme">{theme}</div>
      <div data-testid="resolved-theme">{resolvedTheme}</div>
      <div data-testid="is-dark">{isDark ? 'true' : 'false'}</div>
      <button data-testid="set-light" onClick={() => setTheme('light')}>Light</button>
      <button data-testid="set-dark" onClick={() => setTheme('dark')}>Dark</button>
      <button data-testid="set-system" onClick={() => setTheme('system')}>System</button>
      <button data-testid="toggle" onClick={() => toggleTheme()}>Toggle</button>
    </div>
  );
}

describe('ThemeProvider', () => {
  beforeEach(() => {
    jest.clearAllMocks();
    document.documentElement.classList.remove('light', 'dark');
    document.documentElement.removeAttribute('data-theme');
  });

  it('should render with default light theme', () => {
    render(
      <ThemeProvider defaultTheme="light">
        <TestComponent />
      </ThemeProvider>
    );
    
    expect(screen.getByTestId('theme').textContent).toBe('light');
    expect(screen.getByTestId('resolved-theme').textContent).toBe('light');
    expect(screen.getByTestId('is-dark').textContent).toBe('false');
  });

  it('should render with default dark theme', () => {
    render(
      <ThemeProvider defaultTheme="dark">
        <TestComponent />
      </ThemeProvider>
    );
    
    expect(screen.getByTestId('theme').textContent).toBe('dark');
    expect(screen.getByTestId('resolved-theme').textContent).toBe('dark');
    expect(screen.getByTestId('is-dark').textContent).toBe('true');
  });

  it('should toggle between light and dark', () => {
    render(
      <ThemeProvider defaultTheme="light">
        <TestComponent />
      </ThemeProvider>
    );
    
    // Start with light
    expect(screen.getByTestId('theme').textContent).toBe('light');
    
    // Toggle to dark
    fireEvent.click(screen.getByTestId('toggle'));
    expect(screen.getByTestId('theme').textContent).toBe('dark');
    expect(screen.getByTestId('is-dark').textContent).toBe('true');
    
    // Toggle back to light
    fireEvent.click(screen.getByTestId('toggle'));
    expect(screen.getByTestId('theme').textContent).toBe('light');
    expect(screen.getByTestId('is-dark').textContent).toBe('false');
  });

  it('should set theme directly', () => {
    render(
      <ThemeProvider defaultTheme="light">
        <TestComponent />
      </ThemeProvider>
    );
    
    // Set to dark
    fireEvent.click(screen.getByTestId('set-dark'));
    expect(screen.getByTestId('theme').textContent).toBe('dark');
    
    // Set to light
    fireEvent.click(screen.getByTestId('set-light'));
    expect(screen.getByTestId('theme').textContent).toBe('light');
    
    // Set to system
    fireEvent.click(screen.getByTestId('set-system'));
    expect(screen.getByTestId('theme').textContent).toBe('system');
  });

  it('should persist theme to localStorage', () => {
    render(
      <ThemeProvider defaultTheme="light" storageKey="test-theme">
        <TestComponent />
      </ThemeProvider>
    );
    
    fireEvent.click(screen.getByTestId('set-dark'));
    
    expect(localStorageMock.setItem).toHaveBeenCalledWith('test-theme', 'dark');
  });

  it('should load theme from localStorage', () => {
    localStorageMock.getItem.mockReturnValue('dark');
    
    render(
      <ThemeProvider defaultTheme="light" storageKey="test-theme">
        <TestComponent />
      </ThemeProvider>
    );
    
    expect(localStorageMock.getItem).toHaveBeenCalledWith('test-theme');
    expect(screen.getByTestId('theme').textContent).toBe('dark');
  });

  it('should apply theme class to document element', async () => {
    render(
      <ThemeProvider defaultTheme="dark">
        <TestComponent />
      </ThemeProvider>
    );
    
    await waitFor(() => {
      expect(document.documentElement.classList.contains('dark')).toBe(true);
      expect(document.documentElement.getAttribute('data-theme')).toBe('dark');
    });
  });

  it('should dispatch custom events on theme change', () => {
    const eventListener = jest.fn();
    window.addEventListener('themechange', eventListener);
    
    render(
      <ThemeProvider defaultTheme="light">
        <TestComponent />
      </ThemeProvider>
    );
    
    fireEvent.click(screen.getByTestId('set-dark'));
    
    expect(eventListener).toHaveBeenCalled();
    
    window.removeEventListener('themechange', eventListener);
  });
});

describe('ThemeProvider Error Handling', () => {
  it('should handle localStorage errors gracefully', () => {
    const consoleSpy = jest.spyOn(console, 'warn').mockImplementation();
    localStorageMock.setItem.mockImplementation(() => {
      throw new Error('localStorage disabled');
    });
    
    render(
      <ThemeProvider defaultTheme="light">
        <TestComponent />
      </ThemeProvider>
    );
    
    // Should not throw
    fireEvent.click(screen.getByTestId('set-dark'));
    
    expect(consoleSpy).toHaveBeenCalled();
    consoleSpy.mockRestore();
  });
});

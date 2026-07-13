'use client';

import { useState, useEffect, useCallback, useRef } from 'react';
import {
  Palette,
  Upload,
  Type,
  Code,
  Mail,
  Eye,
  Sun,
  Moon,
  Monitor,
  Check,
  Undo,
  Save,
  Image as ImageIcon,
  AlertCircle,
  Loader2,
  Trash2
} from 'lucide-react';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Card, CardContent, CardHeader, CardTitle, CardDescription } from '@/components/ui/card';
import { Tabs, TabsContent, TabsList, TabsTrigger } from '@/components/ui/tabs';
import { Label } from '@/components/ui/label';
import { Textarea } from '@/components/ui/textarea';
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from '@/components/ui/select';
import { Alert, AlertDescription } from '@/components/ui/alert';
import { Switch } from '@/components/ui/switch';
import { Slider } from '@/components/ui/slider';
import { Badge } from '@/components/ui/badge';
import { Separator } from '@/components/ui/separator';
import { useToast } from '@/components/ui/use-toast';
import { brandingApi } from '@/lib/api/branding';
import type { OrganizationBranding, LogoVariant } from '@/lib/branding/types';

interface ColorConfig {
  primary: string;
  secondary: string;
  accent: string;
  background: string;
  surface: string;
  text: string;
  textMuted: string;
  success: string;
  warning: string;
  error: string;
  info: string;
}

export default function BrandingPage() {
  const { toast } = useToast();
  const [branding, setBranding] = useState<OrganizationBranding | null>(null);
  const [loading, setLoading] = useState(true);
  const [saving, setSaving] = useState(false);
  const [activeTheme, setActiveTheme] = useState<'light' | 'dark'>('light');
  const [previewMode, setPreviewMode] = useState(false);
  const fileInputRef = useRef<HTMLInputElement>(null);
  const [logoVariant, setLogoVariant] = useState<LogoVariant>('default');
  
  // Color state
  const [lightColors, setLightColors] = useState<ColorConfig>({
    primary: '#2563eb',
    secondary: '#64748b',
    accent: '#06b6d4',
    background: '#ffffff',
    surface: '#f8fafc',
    text: '#1e293b',
    textMuted: '#64748b',
    success: '#22c55e',
    warning: '#f59e0b',
    error: '#ef4444',
    info: '#3b82f6'
  });

  const [darkColors, setDarkColors] = useState<ColorConfig>({
    primary: '#3b82f6',
    secondary: '#94a3b8',
    accent: '#22d3ee',
    background: '#0f172a',
    surface: '#1e293b',
    text: '#f1f5f9',
    textMuted: '#94a3b8',
    success: '#4ade80',
    warning: '#fbbf24',
    error: '#f87171',
    info: '#60a5fa'
  });

  // Typography state
  const [typography, setTypography] = useState({
    headingFont: 'system',
    bodyFont: 'system',
    baseSize: 16,
    lineHeight: 1.5,
    letterSpacing: 0,
    scaleRatio: 1.25
  });

  // CSS state
  const [customCss, setCustomCss] = useState('');

  // Settings state
  const [settings, setSettings] = useState({
    borderRadius: 'medium',
    shadowIntensity: 'medium',
    spacing: 'comfortable',
    animations: 'full',
    enableBlur: true
  });

  const fetchBranding = useCallback(async () => {
    try {
      setLoading(true);
      // Using a default organization ID - in production, get from context
      const orgId = 'default';
      const data = await brandingApi.fetchOrganizationBranding(orgId);
      setBranding(data);
      
      // Apply loaded branding to state
      if (data.lightColors) {
        setLightColors({
          primary: data.lightColors.primary,
          secondary: data.lightColors.secondary,
          accent: data.lightColors.accent,
          background: data.lightColors.background.main,
          surface: data.lightColors.background.surface,
          text: data.lightColors.text.primary,
          textMuted: data.lightColors.text.secondary,
          success: data.lightColors.semantic.success,
          warning: data.lightColors.semantic.warning,
          error: data.lightColors.semantic.error,
          info: data.lightColors.semantic.info
        });
      }
      
      if (data.darkColors) {
        setDarkColors({
          primary: data.darkColors.primary,
          secondary: data.darkColors.secondary,
          accent: data.darkColors.accent,
          background: data.darkColors.background.main,
          surface: data.darkColors.background.surface,
          text: data.darkColors.text.primary,
          textMuted: data.darkColors.text.secondary,
          success: data.darkColors.semantic.success,
          warning: data.darkColors.semantic.warning,
          error: data.darkColors.semantic.error,
          info: data.darkColors.semantic.info
        });
      }
      
      if (data.typography) {
        setTypography({
          headingFont: data.typography.headings.family,
          bodyFont: data.typography.body.family,
          baseSize: data.typography.body.baseSize,
          lineHeight: data.typography.body.lineHeight,
          letterSpacing: data.typography.body.letterSpacing,
          scaleRatio: data.typography.scaleRatio
        });
      }
      
      if (data.customCss) {
        setCustomCss(data.customCss);
      }
      
      setSettings({
        borderRadius: String(data.borderRadius),
        shadowIntensity: data.shadowIntensity,
        spacing: data.spacing,
        animations: data.animations,
        enableBlur: data.enableBlur
      });
    } catch (err) {
      toast({
        title: 'Error',
        description: 'Failed to load branding settings',
        variant: 'destructive'
      });
    } finally {
      setLoading(false);
    }
  }, [toast]);

  useEffect(() => {
    fetchBranding();
  }, [fetchBranding]);

  const handleColorChange = (theme: 'light' | 'dark', key: keyof ColorConfig, value: string) => {
    if (theme === 'light') {
      setLightColors(prev => ({ ...prev, [key]: value }));
    } else {
      setDarkColors(prev => ({ ...prev, [key]: value }));
    }
  };

  const handleLogoUpload = async (e: React.ChangeEvent<HTMLInputElement>) => {
    const file = e.target.files?.[0];
    if (!file) return;

    try {
      setSaving(true);
      const orgId = 'default';
      await brandingApi.uploadLogo(orgId, file, logoVariant);
      toast({
        title: 'Success',
        description: 'Logo uploaded successfully'
      });
      fetchBranding();
    } catch (err) {
      toast({
        title: 'Error',
        description: 'Failed to upload logo',
        variant: 'destructive'
      });
    } finally {
      setSaving(false);
    }
  };

  const handleSave = async () => {
    try {
      setSaving(true);
      const orgId = 'default';
      
      await brandingApi.updateBranding(orgId, {
        lightColors: {
          primary: lightColors.primary,
          secondary: lightColors.secondary,
          accent: lightColors.accent,
          background: {
            main: lightColors.background,
            surface: lightColors.surface,
            elevated: '#ffffff'
          },
          text: {
            primary: lightColors.text,
            secondary: lightColors.textMuted,
            muted: lightColors.textMuted,
            inverse: '#ffffff'
          },
          semantic: {
            success: lightColors.success,
            warning: lightColors.warning,
            error: lightColors.error,
            info: lightColors.info
          },
          border: {
            light: '#e2e8f0',
            default: '#cbd5e1',
            strong: '#94a3b8'
          }
        },
        darkColors: {
          primary: darkColors.primary,
          secondary: darkColors.secondary,
          accent: darkColors.accent,
          background: {
            main: darkColors.background,
            surface: darkColors.surface,
            elevated: '#334155'
          },
          text: {
            primary: darkColors.text,
            secondary: darkColors.textMuted,
            muted: darkColors.textMuted,
            inverse: '#0f172a'
          },
          semantic: {
            success: darkColors.success,
            warning: darkColors.warning,
            error: darkColors.error,
            info: darkColors.info
          },
          border: {
            light: '#334155',
            default: '#475569',
            strong: '#64748b'
          }
        },
        typography: {
          headings: {
            family: typography.headingFont as any,
            weights: [400, 500, 600, 700],
            baseSize: typography.baseSize * 1.5,
            lineHeight: typography.lineHeight,
            letterSpacing: typography.letterSpacing
          },
          body: {
            family: typography.bodyFont as any,
            weights: [400, 500, 600],
            baseSize: typography.baseSize,
            lineHeight: typography.lineHeight,
            letterSpacing: typography.letterSpacing
          },
          mono: {
            family: 'monospace',
            weights: [400, 500],
            baseSize: typography.baseSize * 0.875,
            lineHeight: 1.5,
            letterSpacing: 0
          },
          scaleRatio: typography.scaleRatio
        },
        customCss,
        borderRadius: settings.borderRadius as any,
        shadowIntensity: settings.shadowIntensity as any,
        spacing: settings.spacing as any,
        animations: settings.animations as any,
        enableBlur: settings.enableBlur
      });

      toast({
        title: 'Success',
        description: 'Branding settings saved successfully'
      });
    } catch (err) {
      toast({
        title: 'Error',
        description: 'Failed to save branding settings',
        variant: 'destructive'
      });
    } finally {
      setSaving(false);
    }
  };

  const getCurrentColors = () => activeTheme === 'light' ? lightColors : darkColors;

  const renderColorPicker = (label: string, key: keyof ColorConfig) => (
    <div className="flex items-center justify-between p-3 bg-gray-50 dark:bg-gray-800 rounded-lg">
      <div className="flex items-center gap-3">
        <div
          className="w-8 h-8 rounded-lg border shadow-sm"
          style={{ backgroundColor: getCurrentColors()[key] }}
        />
        <Label className="font-medium">{label}</Label>
      </div>
      <div className="flex items-center gap-2">
        <Input
          type="color"
          value={getCurrentColors()[key]}
          onChange={(e) => handleColorChange(activeTheme, key, e.target.value)}
          className="w-12 h-8 p-1"
        />
        <Input
          type="text"
          value={getCurrentColors()[key]}
          onChange={(e) => handleColorChange(activeTheme, key, e.target.value)}
          className="w-24 text-sm font-mono"
        />
      </div>
    </div>
  );

  const renderPreview = () => {
    const colors = getCurrentColors();
    return (
      <div
        className="rounded-xl p-6 transition-all duration-300"
        style={{
          backgroundColor: colors.background,
          color: colors.text,
          fontFamily: typography.bodyFont === 'system' ? 'system-ui, sans-serif' : typography.bodyFont,
          fontSize: `${typography.baseSize}px`,
          lineHeight: typography.lineHeight
        }}
      >
        <div className="space-y-6">
          {/* Header Preview */}
          <div
            className="flex items-center justify-between p-4 rounded-lg"
            style={{ backgroundColor: colors.surface }}
          >
            <div className="flex items-center gap-3">
              <div
                className="w-10 h-10 rounded-lg flex items-center justify-center"
                style={{ backgroundColor: colors.primary }}
              >
                <span className="text-white font-bold">S</span>
              </div>
              <span className="font-semibold text-lg">S.E.R.A.</span>
            </div>
            <div className="flex items-center gap-2">
              <div
                className="px-4 py-2 rounded-md text-white text-sm"
                style={{ backgroundColor: colors.primary }}
              >
                Primary Button
              </div>
              <div
                className="px-4 py-2 rounded-md text-sm border"
                style={{ borderColor: colors.textMuted, color: colors.text }}
              >
                Secondary
              </div>
            </div>
          </div>

          {/* Content Preview */}
          <div className="space-y-4">
            <h2
              className="text-2xl font-bold"
              style={{ color: colors.text }}
            >
              Sample Heading
            </h2>
            <p style={{ color: colors.textMuted }}>
              This is sample body text demonstrating how your content will appear with the selected typography and color scheme.
            </p>

            {/* Form Elements */}
            <div className="space-y-3">
              <div
                className="p-3 rounded-lg border"
                style={{ borderColor: colors.textMuted + '40', backgroundColor: colors.surface }}
              >
                <Label style={{ color: colors.text }}>Sample Input</Label>
                <Input
                  placeholder="Type here..."
                  className="mt-1"
                  style={{ borderColor: colors.textMuted + '40' }}
                />
              </div>
            </div>

            {/* Status Badges */}
            <div className="flex flex-wrap gap-2">
              <Badge style={{ backgroundColor: colors.success, color: 'white' }}>Success</Badge>
              <Badge style={{ backgroundColor: colors.warning, color: 'white' }}>Warning</Badge>
              <Badge style={{ backgroundColor: colors.error, color: 'white' }}>Error</Badge>
              <Badge style={{ backgroundColor: colors.info, color: 'white' }}>Info</Badge>
            </div>

            {/* Cards */}
            <div className="grid grid-cols-2 gap-4">
              <div
                className="p-4 rounded-lg"
                style={{ backgroundColor: colors.surface, border: `1px solid ${colors.textMuted}20` }}
              >
                <h4 style={{ color: colors.text }} className="font-medium mb-2">Card Title</h4>
                <p style={{ color: colors.textMuted }} className="text-sm">Card content with secondary text color.</p>
              </div>
              <div
                className="p-4 rounded-lg"
                style={{
                  backgroundColor: colors.primary + '10',
                  border: `1px solid ${colors.primary}30`
                }}
              >
                <h4 style={{ color: colors.primary }} className="font-medium mb-2">Accent Card</h4>
                <p style={{ color: colors.textMuted }} className="text-sm">Card with primary color accent.</p>
              </div>
            </div>
          </div>
        </div>
      </div>
    );
  };

  if (loading) {
    return (
      <div className="container mx-auto p-6 flex items-center justify-center min-h-[60vh]">
        <Loader2 className="w-8 h-8 animate-spin" />
      </div>
    );
  }

  return (
    <div className="container mx-auto p-6 space-y-6">
      {/* Header */}
      <div className="flex flex-col md:flex-row md:items-center md:justify-between gap-4">
        <div>
          <h1 className="text-3xl font-bold flex items-center gap-2">
            <Palette className="w-8 h-8 text-blue-600" />
            Branding & White-Label
          </h1>
          <p className="text-gray-500 mt-1">Customize your organization&apos;s appearance</p>
        </div>
        <div className="flex items-center gap-2">
          <Button
            variant="outline"
            onClick={() => setPreviewMode(!previewMode)}
            className="gap-2"
          >
            <Eye className="w-4 h-4" />
            {previewMode ? 'Edit' : 'Preview'}
          </Button>
          <Button onClick={handleSave} disabled={saving} className="gap-2">
            {saving ? <Loader2 className="w-4 h-4 animate-spin" /> : <Save className="w-4 h-4" />}
            Save Changes
          </Button>
        </div>
      </div>

      {/* Live Preview Banner */}
      {previewMode && (
        <Alert className="bg-blue-50 border-blue-200">
          <Eye className="w-4 h-4 text-blue-600" />
          <AlertDescription className="text-blue-700">
            Live Preview Mode - Changes are shown in real-time but not saved until you click &quot;Save Changes&quot;
          </AlertDescription>
        </Alert>
      )}

      <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
        {/* Main Editor */}
        <div className="lg:col-span-2 space-y-6">
          <Tabs defaultValue="colors">
            <TabsList className="w-full">
              <TabsTrigger value="colors" className="gap-2">
                <Palette className="w-4 h-4" />
                Colors
              </TabsTrigger>
              <TabsTrigger value="logos" className="gap-2">
                <ImageIcon className="w-4 h-4" />
                Logos
              </TabsTrigger>
              <TabsTrigger value="typography" className="gap-2">
                <Type className="w-4 h-4" />
                Typography
              </TabsTrigger>
              <TabsTrigger value="css" className="gap-2">
                <Code className="w-4 h-4" />
                Custom CSS
              </TabsTrigger>
            </TabsList>

            {/* Colors Tab */}
            <TabsContent value="colors" className="space-y-6 mt-6">
              <Card>
                <CardHeader>
                  <div className="flex items-center justify-between">
                    <CardTitle>Color Scheme</CardTitle>
                    <div className="flex items-center gap-2">
                      <Button
                        variant={activeTheme === 'light' ? 'default' : 'outline'}
                        size="sm"
                        onClick={() => setActiveTheme('light')}
                        className="gap-2"
                      >
                        <Sun className="w-4 h-4" />
                        Light
                      </Button>
                      <Button
                        variant={activeTheme === 'dark' ? 'default' : 'outline'}
                        size="sm"
                        onClick={() => setActiveTheme('dark')}
                        className="gap-2"
                      >
                        <Moon className="w-4 h-4" />
                        Dark
                      </Button>
                    </div>
                  </div>
                  <CardDescription>
                    Customize colors for the {activeTheme} theme
                  </CardDescription>
                </CardHeader>
                <CardContent className="space-y-6">
                  <div className="space-y-3">
                    <h4 className="font-medium text-sm text-gray-500 uppercase">Brand Colors</h4>
                    {renderColorPicker('Primary', 'primary')}
                    {renderColorPicker('Secondary', 'secondary')}
                    {renderColorPicker('Accent', 'accent')}
                  </div>
                  <Separator />
                  <div className="space-y-3">
                    <h4 className="font-medium text-sm text-gray-500 uppercase">Semantic Colors</h4>
                    {renderColorPicker('Success', 'success')}
                    {renderColorPicker('Warning', 'warning')}
                    {renderColorPicker('Error', 'error')}
                    {renderColorPicker('Info', 'info')}
                  </div>
                  <Separator />
                  <div className="space-y-3">
                    <h4 className="font-medium text-sm text-gray-500 uppercase">UI Colors</h4>
                    {renderColorPicker('Background', 'background')}
                    {renderColorPicker('Surface', 'surface')}
                    {renderColorPicker('Text Primary', 'text')}
                    {renderColorPicker('Text Muted', 'textMuted')}
                  </div>
                </CardContent>
              </Card>
            </TabsContent>

            {/* Logos Tab */}
            <TabsContent value="logos" className="space-y-6 mt-6">
              <Card>
                <CardHeader>
                  <CardTitle>Logo Upload</CardTitle>
                  <CardDescription>
                    Upload logos for different themes and contexts
                  </CardDescription>
                </CardHeader>
                <CardContent className="space-y-6">
                  <div className="flex items-center gap-4">
                    <Select value={logoVariant} onValueChange={(v) => setLogoVariant(v as LogoVariant)}>
                      <SelectTrigger className="w-48">
                        <SelectValue />
                      </SelectTrigger>
                      <SelectContent>
                        <SelectItem value="default">Default</SelectItem>
                        <SelectItem value="light">Light Theme</SelectItem>
                        <SelectItem value="dark">Dark Theme</SelectItem>
                        <SelectItem value="monochrome">Monochrome</SelectItem>
                      </SelectContent>
                    </Select>
                    <input
                      ref={fileInputRef}
                      type="file"
                      accept="image/*"
                      onChange={handleLogoUpload}
                      className="hidden"
                    />
                    <Button
                      variant="outline"
                      onClick={() => fileInputRef.current?.click()}
                      disabled={saving}
                      className="gap-2"
                    >
                      <Upload className="w-4 h-4" />
                      Upload Logo
                    </Button>
                  </div>

                  {/* Logo Preview */}
                  <div className="grid grid-cols-2 md:grid-cols-4 gap-4">
                    {(['default', 'light', 'dark', 'monochrome'] as LogoVariant[]).map((variant) => (
                      <Card key={variant} className="overflow-hidden">
                        <CardContent className="p-4">
                          <div className="aspect-video bg-gray-100 dark:bg-gray-800 rounded-lg flex items-center justify-center mb-3">
                            {branding?.logos?.[variant] ? (
                              <img
                                src={branding.logos[variant]?.url}
                                alt={`${variant} logo`}
                                className="max-w-full max-h-full object-contain"
                              />
                            ) : (
                              <ImageIcon className="w-12 h-12 text-gray-400" />
                            )}
                          </div>
                          <div className="flex items-center justify-between">
                            <span className="text-sm font-medium capitalize">{variant}</span>
                            {branding?.logos?.[variant] && (
                              <Button
                                variant="ghost"
                                size="sm"
                                onClick={() => brandingApi.deleteLogo('default', variant)}
                              >
                                <Trash2 className="w-4 h-4 text-red-500" />
                              </Button>
                            )}
                          </div>
                        </CardContent>
                      </Card>
                    ))}
                  </div>
                </CardContent>
              </Card>
            </TabsContent>

            {/* Typography Tab */}
            <TabsContent value="typography" className="space-y-6 mt-6">
              <Card>
                <CardHeader>
                  <CardTitle>Typography Settings</CardTitle>
                </CardHeader>
                <CardContent className="space-y-6">
                  <div className="grid grid-cols-2 gap-4">
                    <div>
                      <Label>Heading Font</Label>
                      <Select
                        value={typography.headingFont}
                        onValueChange={(v) => setTypography({ ...typography, headingFont: v })}
                      >
                        <SelectTrigger>
                          <SelectValue />
                        </SelectTrigger>
                        <SelectContent>
                          <SelectItem value="system">System</SelectItem>
                          <SelectItem value="serif">Serif</SelectItem>
                          <SelectItem value="sans-serif">Sans Serif</SelectItem>
                          <SelectItem value="monospace">Monospace</SelectItem>
                        </SelectContent>
                      </Select>
                    </div>
                    <div>
                      <Label>Body Font</Label>
                      <Select
                        value={typography.bodyFont}
                        onValueChange={(v) => setTypography({ ...typography, bodyFont: v })}
                      >
                        <SelectTrigger>
                          <SelectValue />
                        </SelectTrigger>
                        <SelectContent>
                          <SelectItem value="system">System</SelectItem>
                          <SelectItem value="serif">Serif</SelectItem>
                          <SelectItem value="sans-serif">Sans Serif</SelectItem>
                          <SelectItem value="monospace">Monospace</SelectItem>
                        </SelectContent>
                      </Select>
                    </div>
                  </div>
                  <div>
                    <Label>Base Font Size: {typography.baseSize}px</Label>
                    <Slider
                      value={[typography.baseSize]}
                      onValueChange={([v]) => setTypography({ ...typography, baseSize: v })}
                      min={12}
                      max={20}
                      step={1}
                      className="mt-2"
                    />
                  </div>
                  <div>
                    <Label>Line Height: {typography.lineHeight}</Label>
                    <Slider
                      value={[typography.lineHeight]}
                      onValueChange={([v]) => setTypography({ ...typography, lineHeight: v })}
                      min={1}
                      max={2}
                      step={0.1}
                      className="mt-2"
                    />
                  </div>
                  <div>
                    <Label>Letter Spacing: {typography.letterSpacing}px</Label>
                    <Slider
                      value={[typography.letterSpacing]}
                      onValueChange={([v]) => setTypography({ ...typography, letterSpacing: v })}
                      min={-1}
                      max={2}
                      step={0.1}
                      className="mt-2"
                    />
                  </div>
                  <div>
                    <Label>Scale Ratio: {typography.scaleRatio}</Label>
                    <Slider
                      value={[typography.scaleRatio]}
                      onValueChange={([v]) => setTypography({ ...typography, scaleRatio: v })}
                      min={1.1}
                      max={1.5}
                      step={0.05}
                      className="mt-2"
                    />
                  </div>
                </CardContent>
              </Card>
            </TabsContent>

            {/* Custom CSS Tab */}
            <TabsContent value="css" className="space-y-6 mt-6">
              <Card>
                <CardHeader>
                  <CardTitle>Custom CSS</CardTitle>
                  <CardDescription>
                    Add custom CSS styles to override default theme
                  </CardDescription>
                </CardHeader>
                <CardContent>
                  <Textarea
                    value={customCss}
                    onChange={(e) => setCustomCss(e.target.value)}
                    placeholder="/* Add your custom CSS here */\n.custom-class {\n  color: blue;\n}"
                    className="font-mono text-sm min-h-[300px]"
                  />
                </CardContent>
              </Card>
            </TabsContent>
          </Tabs>
        </div>

        {/* Preview Panel */}
        <div className="space-y-6">
          <Card className="sticky top-6">
            <CardHeader>
              <div className="flex items-center justify-between">
                <CardTitle>Live Preview</CardTitle>
                <div className="flex items-center gap-1 bg-gray-100 dark:bg-gray-800 rounded-lg p-1">
                  <Button
                    variant="ghost"
                    size="sm"
                    onClick={() => setActiveTheme('light')}
                    className={activeTheme === 'light' ? 'bg-white shadow-sm' : ''}
                  >
                    <Sun className="w-4 h-4" />
                  </Button>
                  <Button
                    variant="ghost"
                    size="sm"
                    onClick={() => setActiveTheme('dark')}
                    className={activeTheme === 'dark' ? 'bg-gray-700 shadow-sm' : ''}
                  >
                    <Moon className="w-4 h-4" />
                  </Button>
                </div>
              </div>
            </CardHeader>
            <CardContent>
              {renderPreview()}
            </CardContent>
          </Card>

          {/* Settings Card */}
          <Card>
            <CardHeader>
              <CardTitle>Appearance Settings</CardTitle>
            </CardHeader>
            <CardContent className="space-y-4">
              <div>
                <Label>Border Radius</Label>
                <Select
                  value={settings.borderRadius}
                  onValueChange={(v) => setSettings({ ...settings, borderRadius: v })}
                >
                  <SelectTrigger>
                    <SelectValue />
                  </SelectTrigger>
                  <SelectContent>
                    <SelectItem value="none">None</SelectItem>
                    <SelectItem value="small">Small</SelectItem>
                    <SelectItem value="medium">Medium</SelectItem>
                    <SelectItem value="large">Large</SelectItem>
                    <SelectItem value="full">Full</SelectItem>
                  </SelectContent>
                </Select>
              </div>
              <div>
                <Label>Shadow Intensity</Label>
                <Select
                  value={settings.shadowIntensity}
                  onValueChange={(v) => setSettings({ ...settings, shadowIntensity: v })}
                >
                  <SelectTrigger>
                    <SelectValue />
                  </SelectTrigger>
                  <SelectContent>
                    <SelectItem value="none">None</SelectItem>
                    <SelectItem value="light">Light</SelectItem>
                    <SelectItem value="medium">Medium</SelectItem>
                    <SelectItem value="heavy">Heavy</SelectItem>
                  </SelectContent>
                </Select>
              </div>
              <div>
                <Label>Spacing</Label>
                <Select
                  value={settings.spacing}
                  onValueChange={(v) => setSettings({ ...settings, spacing: v })}
                >
                  <SelectTrigger>
                    <SelectValue />
                  </SelectTrigger>
                  <SelectContent>
                    <SelectItem value="compact">Compact</SelectItem>
                    <SelectItem value="comfortable">Comfortable</SelectItem>
                    <SelectItem value="spacious">Spacious</SelectItem>
                  </SelectContent>
                </Select>
              </div>
              <div>
                <Label>Animations</Label>
                <Select
                  value={settings.animations}
                  onValueChange={(v) => setSettings({ ...settings, animations: v })}
                >
                  <SelectTrigger>
                    <SelectValue />
                  </SelectTrigger>
                  <SelectContent>
                    <SelectItem value="none">None</SelectItem>
                    <SelectItem value="reduced">Reduced</SelectItem>
                    <SelectItem value="full">Full</SelectItem>
                  </SelectContent>
                </Select>
              </div>
              <div className="flex items-center justify-between">
                <Label>Enable Blur Effects</Label>
                <Switch
                  checked={settings.enableBlur}
                  onCheckedChange={(v) => setSettings({ ...settings, enableBlur: v })}
                />
              </div>
            </CardContent>
          </Card>
        </div>
      </div>
    </div>
  );
}

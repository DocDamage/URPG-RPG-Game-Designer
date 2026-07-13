'use client'

import { useState, useEffect } from 'react'
import { useRouter } from 'next/navigation'
import { Button } from '@/components/ui/button'
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from '@/components/ui/card'
import { Input } from '@/components/ui/input'
import { Label } from '@/components/ui/label'
import { RadioGroup, RadioGroupItem } from '@/components/ui/radio-group'
import { useToast } from '@/components/ui/use-toast'
import { Shield, Smartphone, Mail, Key, Copy, Check, Loader2 } from 'lucide-react'

interface MFASettings {
  method: 'totp' | 'sms' | 'email'
  secret?: string
  qrCodeUrl?: string
  backupCodes: string[]
}

export default function MFASetupPage() {
  const router = useRouter()
  const { toast } = useToast()
  const [step, setStep] = useState<'method' | 'setup' | 'verify'>('method')
  const [method, setMethod] = useState<'totp' | 'sms' | 'email'>('totp')
  const [phoneNumber, setPhoneNumber] = useState('')
  const [email, setEmail] = useState('')
  const [verificationCode, setVerificationCode] = useState('')
  const [mfaSettings, setMfaSettings] = useState<MFASettings | null>(null)
  const [loading, setLoading] = useState(false)
  const [copiedCodes, setCopiedCodes] = useState(false)

  const handleInitialize = async () => {
    setLoading(true)
    try {
      const token = localStorage.getItem('token')
      const response = await fetch('/api/mfa/initialize', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          'Authorization': `Bearer ${token}`,
        },
        body: JSON.stringify({
          method,
          phoneNumber: method === 'sms' ? phoneNumber : undefined,
          email: method === 'email' ? email : undefined,
        }),
      })

      if (!response.ok) {
        const error = await response.json()
        throw new Error(error.error || 'Failed to initialize MFA')
      }

      const data = await response.json()
      setMfaSettings(data)
      setStep('setup')
      toast({
        title: 'MFA Initialized',
        description: 'Please save your backup codes and complete setup.',
      })
    } catch (error: any) {
      toast({
        title: 'Error',
        description: error.message,
        variant: 'destructive',
      })
    } finally {
      setLoading(false)
    }
  }

  const handleVerify = async () => {
    setLoading(true)
    try {
      const token = localStorage.getItem('token')
      const response = await fetch('/api/mfa/verify-setup', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          'Authorization': `Bearer ${token}`,
        },
        body: JSON.stringify({ code: verificationCode }),
      })

      if (!response.ok) {
        const error = await response.json()
        throw new Error(error.error || 'Invalid verification code')
      }

      toast({
        title: 'MFA Enabled',
        description: 'Multi-factor authentication has been successfully enabled.',
      })
      router.push('/settings/security')
    } catch (error: any) {
      toast({
        title: 'Verification Failed',
        description: error.message,
        variant: 'destructive',
      })
    } finally {
      setLoading(false)
    }
  }

  const copyBackupCodes = () => {
    if (mfaSettings?.backupCodes) {
      navigator.clipboard.writeText(mfaSettings.backupCodes.join('\n'))
      setCopiedCodes(true)
      setTimeout(() => setCopiedCodes(false), 2000)
      toast({
        title: 'Copied',
        description: 'Backup codes copied to clipboard.',
      })
    }
  }

  return (
    <div className="container mx-auto px-4 py-8 max-w-2xl">
      <div className="mb-8">
        <h1 className="text-3xl font-bold flex items-center gap-3">
          <Shield className="w-8 h-8 text-blue-600" />
          Multi-Factor Authentication Setup
        </h1>
        <p className="text-gray-600 dark:text-gray-300 mt-2">
          Add an extra layer of security to your account
        </p>
      </div>

      {step === 'method' && (
        <Card>
          <CardHeader>
            <CardTitle>Choose Authentication Method</CardTitle>
            <CardDescription>
              Select how you want to receive verification codes
            </CardDescription>
          </CardHeader>
          <CardContent className="space-y-6">
            <RadioGroup value={method} onValueChange={(v) => setMethod(v as any)}>
              <div className="flex items-center space-x-2 space-y-2">
                <RadioGroupItem value="totp" id="totp" />
                <Label htmlFor="totp" className="flex items-center gap-2 cursor-pointer">
                  <Key className="w-4 h-4" />
                  Authenticator App (Recommended)
                </Label>
              </div>
              <div className="flex items-center space-x-2 space-y-2">
                <RadioGroupItem value="sms" id="sms" />
                <Label htmlFor="sms" className="flex items-center gap-2 cursor-pointer">
                  <Smartphone className="w-4 h-4" />
                  SMS Text Message
                </Label>
              </div>
              <div className="flex items-center space-x-2 space-y-2">
                <RadioGroupItem value="email" id="email" />
                <Label htmlFor="email" className="flex items-center gap-2 cursor-pointer">
                  <Mail className="w-4 h-4" />
                  Email
                </Label>
              </div>
            </RadioGroup>

            {method === 'sms' && (
              <div className="space-y-2">
                <Label htmlFor="phone">Phone Number</Label>
                <Input
                  id="phone"
                  type="tel"
                  placeholder="+1 (555) 123-4567"
                  value={phoneNumber}
                  onChange={(e) => setPhoneNumber(e.target.value)}
                />
              </div>
            )}

            {method === 'email' && (
              <div className="space-y-2">
                <Label htmlFor="email-input">Email Address</Label>
                <Input
                  id="email-input"
                  type="email"
                  placeholder="your@email.com"
                  value={email}
                  onChange={(e) => setEmail(e.target.value)}
                />
              </div>
            )}

            <Button 
              onClick={handleInitialize} 
              disabled={loading || (method === 'sms' && !phoneNumber) || (method === 'email' && !email)}
              className="w-full"
            >
              {loading ? (
                <>
                  <Loader2 className="w-4 h-4 mr-2 animate-spin" />
                  Setting up...
                </>
              ) : (
                'Continue'
              )}
            </Button>
          </CardContent>
        </Card>
      )}

      {step === 'setup' && mfaSettings && (
        <div className="space-y-6">
          {method === 'totp' && mfaSettings.qrCodeUrl && (
            <Card>
              <CardHeader>
                <CardTitle>Scan QR Code</CardTitle>
                <CardDescription>
                  Open your authenticator app and scan this QR code
                </CardDescription>
              </CardHeader>
              <CardContent className="flex flex-col items-center">
                <img 
                  src={mfaSettings.qrCodeUrl} 
                  alt="MFA QR Code" 
                  className="w-48 h-48 mb-4"
                />
                <p className="text-sm text-gray-500 text-center">
                  Can't scan? Enter this code manually:
                </p>
                <code className="mt-2 p-2 bg-gray-100 dark:bg-gray-800 rounded text-sm">
                  {mfaSettings.secret}
                </code>
              </CardContent>
            </Card>
          )}

          {(method === 'sms' || method === 'email') && (
            <Card>
              <CardHeader>
                <CardTitle>Verification Code Sent</CardTitle>
                <CardDescription>
                  A verification code has been sent to your {method === 'sms' ? 'phone' : 'email'}.
                  Please enter it in the next step.
                </CardDescription>
              </CardHeader>
            </Card>
          )}

          <Card className="border-yellow-200 bg-yellow-50 dark:bg-yellow-900/20">
            <CardHeader>
              <CardTitle className="text-yellow-800 dark:text-yellow-200">
                Save Your Backup Codes
              </CardTitle>
              <CardDescription className="text-yellow-700 dark:text-yellow-300">
                These codes let you access your account if you lose your authenticator device.
                Each code can only be used once.
              </CardDescription>
            </CardHeader>
            <CardContent>
              <div className="grid grid-cols-2 gap-2 mb-4">
                {mfaSettings.backupCodes.map((code, index) => (
                  <code 
                    key={index} 
                    className="p-2 bg-white dark:bg-gray-800 rounded text-center font-mono text-sm"
                  >
                    {code}
                  </code>
                ))}
              </div>
              <Button 
                variant="outline" 
                onClick={copyBackupCodes}
                className="w-full"
              >
                {copiedCodes ? (
                  <>
                    <Check className="w-4 h-4 mr-2" />
                    Copied!
                  </>
                ) : (
                  <>
                    <Copy className="w-4 h-4 mr-2" />
                    Copy Backup Codes
                  </>
                )}
              </Button>
            </CardContent>
          </Card>

          <Card>
            <CardHeader>
              <CardTitle>Verify Setup</CardTitle>
              <CardDescription>
                Enter the verification code to complete setup
              </CardDescription>
            </CardHeader>
            <CardContent className="space-y-4">
              <div className="space-y-2">
                <Label htmlFor="verify-code">Verification Code</Label>
                <Input
                  id="verify-code"
                  placeholder="Enter 6-digit code"
                  value={verificationCode}
                  onChange={(e) => setVerificationCode(e.target.value)}
                  maxLength={6}
                />
              </div>
              <div className="flex gap-2">
                <Button 
                  variant="outline" 
                  onClick={() => setStep('method')}
                  className="flex-1"
                >
                  Back
                </Button>
                <Button 
                  onClick={handleVerify}
                  disabled={loading || verificationCode.length < 6}
                  className="flex-1"
                >
                  {loading ? (
                    <>
                      <Loader2 className="w-4 h-4 mr-2 animate-spin" />
                      Verifying...
                    </>
                  ) : (
                    'Verify & Enable MFA'
                  )}
                </Button>
              </div>
            </CardContent>
          </Card>
        </div>
      )}
    </div>
  )
}

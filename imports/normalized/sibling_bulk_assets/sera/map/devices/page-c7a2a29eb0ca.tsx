'use client'

import { useState, useEffect } from 'react'
import { Button } from '@/components/ui/button'
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from '@/components/ui/card'
import { 
  Smartphone, 
  Laptop, 
  Tablet, 
  Trash2, 
  ShieldAlert, 
  CheckCircle2, 
  XCircle,
  Loader2,
  RefreshCw
} from 'lucide-react'
import { useToast } from '@/components/ui/use-toast'
import { Badge } from '@/components/ui/badge'
import {
  AlertDialog,
  AlertDialogAction,
  AlertDialogCancel,
  AlertDialogContent,
  AlertDialogDescription,
  AlertDialogFooter,
  AlertDialogHeader,
  AlertDialogTitle,
  AlertDialogTrigger,
} from '@/components/ui/alert-dialog'

interface Device {
  id: string
  deviceName: string
  deviceFingerprint: string
  registeredAt: string
  lastSeen: string
  status: 'active' | 'inactive' | 'blacklisted' | 'wiped'
  remoteWipeRequested: boolean
  ipAddress?: string
  userAgent?: string
}

export default function DevicesPage() {
  const [devices, setDevices] = useState<Device[]>([])
  const [loading, setLoading] = useState(true)
  const [actionLoading, setActionLoading] = useState<string | null>(null)
  const { toast } = useToast()

  const fetchDevices = async () => {
    try {
      const token = localStorage.getItem('token')
      const response = await fetch('/api/devices', {
        headers: {
          'Authorization': `Bearer ${token}`,
        },
      })

      if (!response.ok) {
        throw new Error('Failed to fetch devices')
      }

      const data = await response.json()
      setDevices(data)
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

  useEffect(() => {
    fetchDevices()
  }, [])

  const handleUnregister = async (deviceId: string) => {
    setActionLoading(deviceId)
    try {
      const token = localStorage.getItem('token')
      const response = await fetch(`/api/devices/${deviceId}`, {
        method: 'DELETE',
        headers: {
          'Authorization': `Bearer ${token}`,
        },
      })

      if (!response.ok) {
        throw new Error('Failed to unregister device')
      }

      toast({
        title: 'Device Unregistered',
        description: 'The device has been removed from your account.',
      })
      fetchDevices()
    } catch (error: any) {
      toast({
        title: 'Error',
        description: error.message,
        variant: 'destructive',
      })
    } finally {
      setActionLoading(null)
    }
  }

  const handleRemoteWipe = async (deviceId: string) => {
    setActionLoading(deviceId)
    try {
      const token = localStorage.getItem('token')
      const response = await fetch(`/api/devices/${deviceId}/wipe`, {
        method: 'POST',
        headers: {
          'Authorization': `Bearer ${token}`,
        },
      })

      if (!response.ok) {
        throw new Error('Failed to request remote wipe')
      }

      toast({
        title: 'Remote Wipe Requested',
        description: 'The device will be wiped when it comes online.',
      })
      fetchDevices()
    } catch (error: any) {
      toast({
        title: 'Error',
        description: error.message,
        variant: 'destructive',
      })
    } finally {
      setActionLoading(null)
    }
  }

  const getDeviceIcon = (userAgent?: string) => {
    if (!userAgent) return <Smartphone className="w-5 h-5" />
    if (userAgent.includes('Mobile')) return <Smartphone className="w-5 h-5" />
    if (userAgent.includes('Tablet') || userAgent.includes('iPad')) return <Tablet className="w-5 h-5" />
    return <Laptop className="w-5 h-5" />
  }

  const getStatusBadge = (device: Device) => {
    if (device.remoteWipeRequested) {
      return (
        <Badge variant="destructive" className="gap-1">
          <ShieldAlert className="w-3 h-3" />
          Wipe Pending
        </Badge>
      )
    }
    
    switch (device.status) {
      case 'active':
        return (
          <Badge variant="default" className="bg-green-600 gap-1">
            <CheckCircle2 className="w-3 h-3" />
            Active
          </Badge>
        )
      case 'inactive':
        return (
          <Badge variant="secondary" className="gap-1">
            <XCircle className="w-3 h-3" />
            Inactive
          </Badge>
        )
      case 'blacklisted':
        return (
          <Badge variant="destructive" className="gap-1">
            <ShieldAlert className="w-3 h-3" />
            Blocked
          </Badge>
        )
      case 'wiped':
        return (
          <Badge variant="destructive" className="gap-1">
            <Trash2 className="w-3 h-3" />
            Wiped
          </Badge>
        )
      default:
        return null
    }
  }

  const formatDate = (dateString: string) => {
    return new Date(dateString).toLocaleDateString('en-US', {
      year: 'numeric',
      month: 'short',
      day: 'numeric',
      hour: '2-digit',
      minute: '2-digit',
    })
  }

  if (loading) {
    return (
      <div className="container mx-auto px-4 py-8">
        <div className="flex items-center justify-center h-64">
          <Loader2 className="w-8 h-8 animate-spin" />
        </div>
      </div>
    )
  }

  return (
    <div className="container mx-auto px-4 py-8 max-w-4xl">
      <div className="mb-8">
        <h1 className="text-3xl font-bold flex items-center gap-3">
          <Smartphone className="w-8 h-8 text-blue-600" />
          Device Management
        </h1>
        <p className="text-gray-600 dark:text-gray-300 mt-2">
          Manage devices that have access to your account
        </p>
      </div>

      <div className="mb-4 flex justify-end">
        <Button variant="outline" onClick={fetchDevices} className="gap-2">
          <RefreshCw className="w-4 h-4" />
          Refresh
        </Button>
      </div>

      {devices.length === 0 ? (
        <Card>
          <CardContent className="p-8 text-center">
            <Smartphone className="w-12 h-12 mx-auto text-gray-400 mb-4" />
            <h3 className="text-lg font-medium text-gray-900 dark:text-gray-100">
              No Devices Found
            </h3>
            <p className="text-gray-500 mt-1">
              You haven't registered any devices yet.
            </p>
          </CardContent>
        </Card>
      ) : (
        <div className="space-y-4">
          {devices.map((device) => (
            <Card key={device.id}>
              <CardContent className="p-6">
                <div className="flex items-start justify-between">
                  <div className="flex items-start gap-4">
                    <div className="p-3 bg-blue-100 dark:bg-blue-900 rounded-lg">
                      {getDeviceIcon(device.userAgent)}
                    </div>
                    <div>
                      <div className="flex items-center gap-2 mb-1">
                        <h3 className="font-semibold">{device.deviceName}</h3>
                        {getStatusBadge(device)}
                      </div>
                      <p className="text-sm text-gray-500">
                        Registered: {formatDate(device.registeredAt)}
                      </p>
                      <p className="text-sm text-gray-500">
                        Last seen: {formatDate(device.lastSeen)}
                      </p>
                      {device.ipAddress && (
                        <p className="text-sm text-gray-500">
                          IP: {device.ipAddress}
                        </p>
                      )}
                    </div>
                  </div>

                  <div className="flex gap-2">
                    {!device.remoteWipeRequested && device.status === 'active' && (
                      <AlertDialog>
                        <AlertDialogTrigger asChild>
                          <Button 
                            variant="outline" 
                            size="sm"
                            disabled={actionLoading === device.id}
                            className="gap-1 text-orange-600 hover:text-orange-700"
                          >
                            {actionLoading === device.id ? (
                              <Loader2 className="w-4 h-4 animate-spin" />
                            ) : (
                              <ShieldAlert className="w-4 h-4" />
                            )}
                            Remote Wipe
                          </Button>
                        </AlertDialogTrigger>
                        <AlertDialogContent>
                          <AlertDialogHeader>
                            <AlertDialogTitle>Request Remote Wipe?</AlertDialogTitle>
                            <AlertDialogDescription>
                              This will erase all S.E.R.A. data from {device.deviceName}.
                              The wipe will occur when the device comes online.
                              This action cannot be undone.
                            </AlertDialogDescription>
                          </AlertDialogHeader>
                          <AlertDialogFooter>
                            <AlertDialogCancel>Cancel</AlertDialogCancel>
                            <AlertDialogAction 
                              onClick={() => handleRemoteWipe(device.id)}
                              className="bg-red-600 hover:bg-red-700"
                            >
                              Request Wipe
                            </AlertDialogAction>
                          </AlertDialogFooter>
                        </AlertDialogContent>
                      </AlertDialog>
                    )}

                    <AlertDialog>
                      <AlertDialogTrigger asChild>
                        <Button 
                          variant="outline" 
                          size="sm"
                          disabled={actionLoading === device.id}
                          className="gap-1"
                        >
                          {actionLoading === device.id ? (
                            <Loader2 className="w-4 h-4 animate-spin" />
                          ) : (
                            <Trash2 className="w-4 h-4" />
                          )}
                          Unregister
                        </Button>
                      </AlertDialogTrigger>
                      <AlertDialogContent>
                        <AlertDialogHeader>
                          <AlertDialogTitle>Unregister Device?</AlertDialogTitle>
                          <AlertDialogDescription>
                            This will remove {device.deviceName} from your account.
                            You'll need to sign in again on this device.
                          </AlertDialogDescription>
                        </AlertDialogHeader>
                        <AlertDialogFooter>
                          <AlertDialogCancel>Cancel</AlertDialogCancel>
                          <AlertDialogAction 
                            onClick={() => handleUnregister(device.id)}
                            className="bg-red-600 hover:bg-red-700"
                          >
                            Unregister
                          </AlertDialogAction>
                        </AlertDialogFooter>
                      </AlertDialogContent>
                    </AlertDialog>
                  </div>
                </div>
              </CardContent>
            </Card>
          ))}
        </div>
      )}

      <Card className="mt-8 bg-blue-50 dark:bg-blue-900/20 border-blue-200">
        <CardHeader>
          <CardTitle className="text-blue-800 dark:text-blue-200 flex items-center gap-2">
            <ShieldAlert className="w-5 h-5" />
            Security Tips
          </CardTitle>
        </CardHeader>
        <CardContent className="text-blue-700 dark:text-blue-300 space-y-2">
          <p>• Only register devices you own and control</p>
          <p>• Unregister devices you no longer use</p>
          <p>• Use Remote Wipe if a device is lost or stolen</p>
          <p>• Check this list regularly for unrecognized devices</p>
        </CardContent>
      </Card>
    </div>
  )
}

'use client'

import { useEffect, useState } from 'react'
import Link from 'next/link'
import { Button } from '@/components/ui/button'
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from '@/components/ui/card'
import { 
  Activity, 
  Users, 
  ClipboardCheck, 
  Calendar, 
  TrendingUp, 
  Shield, 
  Clock,
  FileText,
  AlertCircle,
  Zap
} from 'lucide-react'
import { useToast } from '@/components/ui/use-toast'

export default function Home() {
  const [isAuthenticated, setIsAuthenticated] = useState(false)
  const [user, setUser] = useState<any>(null)
  const { toast } = useToast()

  useEffect(() => {
    const token = localStorage.getItem('token')
    const userData = localStorage.getItem('user')
    if (token && userData) {
      setIsAuthenticated(true)
      setUser(JSON.parse(userData))
    }
  }, [])

  const features = [
    {
      icon: ClipboardCheck,
      title: 'MAR & PRN Management',
      description: 'Track medication administration with smart safety checks',
      href: '/mar',
    },
    {
      icon: FileText,
      title: 'Daily Logs',
      description: 'Document activities and care notes efficiently',
      href: '/logs',
    },
    {
      icon: AlertCircle,
      title: 'Incident Reports',
      description: 'Quick incident documentation with AI assistance',
      href: '/incidents',
    },
    {
      icon: Activity,
      title: 'Behavioral Tracking',
      description: 'Monitor patterns and interventions',
      href: '/behavioral',
    },
    {
      icon: Calendar,
      title: 'Scheduling',
      description: 'Staff shifts and appointments management',
      href: '/scheduling',
    },
    {
      icon: TrendingUp,
      title: 'Analytics',
      description: 'Insights and compliance reporting',
      href: '/analytics',
    },
  ]

  const stats = [
    { label: 'Active Individuals', value: '24', icon: Users },
    { label: 'Staff On Duty', value: '8', icon: Shield },
    { label: 'Pending MAR', value: '12', icon: Clock },
    { label: 'Alerts', value: '3', icon: AlertCircle },
  ]

  return (
    <div className="container mx-auto px-4 py-8">
      {/* Hero Section */}
      <section className="mb-12">
        <div className="text-center max-w-3xl mx-auto">
          <h1 className="text-4xl md:text-5xl font-bold mb-4 bg-gradient-to-r from-blue-600 to-cyan-500 bg-clip-text text-transparent">
            S.E.R.A.
          </h1>
          <p className="text-xl text-gray-600 dark:text-gray-300 mb-2">
            Support, Evaluation, Reporting & Administration
          </p>
          <p className="text-gray-500 dark:text-gray-400 mb-8">
            Comprehensive healthcare management for group homes
          </p>
          
          {!isAuthenticated ? (
            <div className="flex gap-4 justify-center">
              <Link href="/login">
                <Button size="lg" className="gap-2">
                  <Shield className="w-4 h-4" />
                  Sign In
                </Button>
              </Link>
              <Link href="/about">
                <Button size="lg" variant="outline">
                  Learn More
                </Button>
              </Link>
            </div>
          ) : (
            <div className="flex gap-4 justify-center">
              <Link href="/dashboard">
                <Button size="lg" className="gap-2">
                  <Zap className="w-4 h-4" />
                  Go to Dashboard
                </Button>
              </Link>
            </div>
          )}
        </div>
      </section>

      {/* Stats Section - Only show when authenticated */}
      {isAuthenticated && (
        <section className="mb-12">
          <div className="grid grid-cols-2 md:grid-cols-4 gap-4">
            {stats.map((stat, index) => (
              <Card key={index}>
                <CardContent className="p-6">
                  <div className="flex items-center gap-4">
                    <div className="p-3 bg-blue-100 dark:bg-blue-900 rounded-lg">
                      <stat.icon className="w-6 h-6 text-blue-600 dark:text-blue-300" />
                    </div>
                    <div>
                      <p className="text-sm text-gray-500 dark:text-gray-400">{stat.label}</p>
                      <p className="text-2xl font-bold">{stat.value}</p>
                    </div>
                  </div>
                </CardContent>
              </Card>
            ))}
          </div>
        </section>
      )}

      {/* Features Grid */}
      <section className="mb-12">
        <h2 className="text-2xl font-bold mb-6 text-center">Core Features</h2>
        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
          {features.map((feature, index) => (
            <Link key={index} href={feature.href}>
              <Card className="h-full hover:shadow-lg transition-shadow cursor-pointer">
                <CardHeader>
                  <div className="flex items-center gap-3">
                    <div className="p-2 bg-blue-100 dark:bg-blue-900 rounded-lg">
                      <feature.icon className="w-6 h-6 text-blue-600 dark:text-blue-300" />
                    </div>
                    <CardTitle className="text-lg">{feature.title}</CardTitle>
                  </div>
                </CardHeader>
                <CardContent>
                  <CardDescription className="text-base">
                    {feature.description}
                  </CardDescription>
                </CardContent>
              </Card>
            </Link>
          ))}
        </div>
      </section>

      {/* Quick Actions - Only show when authenticated */}
      {isAuthenticated && (
        <section className="mb-12">
          <h2 className="text-2xl font-bold mb-6">Quick Actions</h2>
          <div className="flex flex-wrap gap-4">
            <Link href="/mar/administer">
              <Button variant="outline" className="gap-2">
                <ClipboardCheck className="w-4 h-4" />
                Administer Medication
              </Button>
            </Link>
            <Link href="/logs/new">
              <Button variant="outline" className="gap-2">
                <FileText className="w-4 h-4" />
                New Log Entry
              </Button>
            </Link>
            <Link href="/incidents/new">
              <Button variant="outline" className="gap-2">
                <AlertCircle className="w-4 h-4" />
                Report Incident
              </Button>
            </Link>
            <Link href="/schedule/view">
              <Button variant="outline" className="gap-2">
                <Calendar className="w-4 h-4" />
                View Schedule
              </Button>
            </Link>
          </div>
        </section>
      )}

      {/* Security Notice */}
      <section className="mt-12">
        <Card className="bg-gradient-to-r from-blue-50 to-cyan-50 dark:from-blue-900/20 dark:to-cyan-900/20 border-blue-200">
          <CardContent className="p-6">
            <div className="flex items-start gap-4">
              <div className="p-3 bg-blue-100 dark:bg-blue-800 rounded-full">
                <Shield className="w-6 h-6 text-blue-600 dark:text-blue-300" />
              </div>
              <div>
                <h3 className="text-lg font-semibold mb-2">HIPAA-Compliant & Secure</h3>
                <p className="text-gray-600 dark:text-gray-300">
                  S.E.R.A. is built with enterprise-grade security including end-to-end encryption, 
                  multi-factor authentication, comprehensive audit trails, and strict access controls 
                  to protect sensitive healthcare data.
                </p>
              </div>
            </div>
          </CardContent>
        </Card>
      </section>
    </div>
  )
}

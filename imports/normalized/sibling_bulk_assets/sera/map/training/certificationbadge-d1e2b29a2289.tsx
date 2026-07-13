"use client";

import React from 'react';
import { format, parseISO, differenceInDays } from 'date-fns';
import { Award, CheckCircle2, AlertCircle, XCircle, Clock } from 'lucide-react';
import { Badge } from '@/components/ui/badge';
import { cn } from '@/lib/utils';

export interface Certification {
  id: string;
  certificationType: string;
  issuingOrganization?: string;
  certificationNumber?: string;
  issuedDate: string;
  expirationDate: string;
  documentUrl?: string;
  verifiedBy?: string;
  verifiedAt?: string;
  status: 'active' | 'expired' | 'revoked' | 'pending_verification';
}

interface CertificationBadgeProps {
  certification: Certification;
  showDetails?: boolean;
  className?: string;
}

export function CertificationBadge({
  certification,
  showDetails = false,
  className,
}: CertificationBadgeProps) {
  const daysUntilExpiration = differenceInDays(
    parseISO(certification.expirationDate),
    new Date()
  );

  const getStatusConfig = (status: Certification['status'], daysLeft: number) => {
    switch (status) {
      case 'active':
        if (daysLeft <= 7) {
          return {
            icon: <AlertCircle className="h-4 w-4" />,
            color: 'bg-red-100 text-red-800 border-red-200',
            label: 'Expiring Soon',
          };
        }
        if (daysLeft <= 30) {
          return {
            icon: <Clock className="h-4 w-4" />,
            color: 'bg-yellow-100 text-yellow-800 border-yellow-200',
            label: 'Expiring Soon',
          };
        }
        return {
          icon: <CheckCircle2 className="h-4 w-4" />,
          color: 'bg-green-100 text-green-800 border-green-200',
          label: 'Active',
        };
      case 'expired':
        return {
          icon: <XCircle className="h-4 w-4" />,
          color: 'bg-red-100 text-red-800 border-red-200',
          label: 'Expired',
        };
      case 'revoked':
        return {
          icon: <XCircle className="h-4 w-4" />,
          color: 'bg-gray-100 text-gray-800 border-gray-200',
          label: 'Revoked',
        };
      case 'pending_verification':
        return {
          icon: <Clock className="h-4 w-4" />,
          color: 'bg-blue-100 text-blue-800 border-blue-200',
          label: 'Pending Verification',
        };
      default:
        return {
          icon: <Award className="h-4 w-4" />,
          color: 'bg-gray-100 text-gray-800',
          label: status,
        };
    }
  };

  const config = getStatusConfig(certification.status, daysUntilExpiration);

  if (!showDetails) {
    return (
      <Badge
        variant="outline"
        className={cn("flex items-center gap-1", config.color, className)}
      >
        {config.icon}
        <span>{certification.certificationType}</span>
      </Badge>
    );
  }

  return (
    <div
      className={cn(
        "flex items-start gap-3 p-3 rounded-lg border",
        config.color,
        className
      )}
    >
      <div className="p-2 bg-white/50 rounded-full">
        <Award className="h-5 w-5" />
      </div>
      <div className="flex-1 min-w-0">
        <div className="flex items-center gap-2 flex-wrap">
          <p className="font-medium">{certification.certificationType}</p>
          <Badge variant="outline" className="text-xs">
            {config.label}
          </Badge>
        </div>
        
        {certification.issuingOrganization && (
          <p className="text-sm opacity-90">
            {certification.issuingOrganization}
          </p>
        )}
        
        <div className="mt-2 space-y-1 text-sm">
          <p>
            <span className="opacity-70">Issued:</span>{' '}
            {format(parseISO(certification.issuedDate), 'MMM d, yyyy')}
          </p>
          <p>
            <span className="opacity-70">Expires:</span>{' '}
            {format(parseISO(certification.expirationDate), 'MMM d, yyyy')}
            {certification.status === 'active' && daysUntilExpiration <= 30 && (
              <span className={cn(
                "ml-2 font-medium",
                daysUntilExpiration <= 7 ? "text-red-600" : "text-yellow-700"
              )}>
                ({daysUntilExpiration} days left)
              </span>
            )}
          </p>
          {certification.certificationNumber && (
            <p>
              <span className="opacity-70">Cert #:</span>{' '}
              {certification.certificationNumber}
            </p>
          )}
        </div>

        {certification.documentUrl && (
          <a
            href={certification.documentUrl}
            target="_blank"
            rel="noopener noreferrer"
            className="inline-block mt-2 text-sm underline hover:no-underline"
          >
            View Certificate
          </a>
        )}
      </div>
    </div>
  );
}

export default CertificationBadge;

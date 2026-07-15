import * as React from "react"
import { clsx } from "clsx"

const alertVariants = {
  default: "bg-white text-gray-950 dark:bg-gray-950 dark:text-gray-50",
  destructive: "border-red-500/50 text-red-500 dark:border-red-500 [&>svg]:text-red-500 dark:border-red-900/50 dark:text-red-900 dark:dark:border-red-900 dark:[&>svg]:text-red-900",
  info: "bg-blue-50 border-blue-200 text-blue-800 dark:bg-blue-950 dark:border-blue-900 dark:text-blue-300",
  success: "bg-green-50 border-green-200 text-green-800 dark:bg-green-950 dark:border-green-900 dark:text-green-300",
  warning: "bg-yellow-50 border-yellow-200 text-yellow-800 dark:bg-yellow-950 dark:border-yellow-900 dark:text-yellow-300"
}

interface AlertProps extends React.HTMLAttributes<HTMLDivElement> {
  variant?: keyof typeof alertVariants
}

const Alert = React.forwardRef<HTMLDivElement, AlertProps>(
  ({ className, variant = "default", ...props }, ref) => (
    <div
      ref={ref}
      role="alert"
      className={clsx(
        "relative w-full rounded-lg border p-4 [&>svg~*]:pl-7 [&>svg+div]:translate-y-[-3px] [&>svg]:absolute [&>svg]:left-4 [&>svg]:top-4 [&>svg]:text-gray-950 dark:[&>svg]:text-gray-50",
        alertVariants[variant],
        className
      )}
      {...props}
    />
  )
)
Alert.displayName = "Alert"

const AlertTitle = React.forwardRef<
  HTMLParagraphElement,
  React.HTMLAttributes<HTMLHeadingElement>
>(({ className, ...props }, ref) => (
  <h5
    ref={ref}
    className={clsx("mb-1 font-medium leading-none tracking-tight", className)}
    {...props}
  />
))
AlertTitle.displayName = "AlertTitle"

const AlertDescription = React.forwardRef<
  HTMLParagraphElement,
  React.HTMLAttributes<HTMLParagraphElement>
>(({ className, ...props }, ref) => (
  <div
    ref={ref}
    className={clsx("text-sm [&_p]:leading-relaxed", className)}
    {...props}
  />
))
AlertDescription.displayName = "AlertDescription"

// Legacy Alert component for backward compatibility
interface LegacyAlertProps {
    children: React.ReactNode;
    variant?: 'info' | 'success' | 'warning' | 'error';
    title?: string;
    onClose?: () => void;
    className?: string;
}

export function AlertLegacy({ children, variant = 'info', title, onClose, className = '' }: LegacyAlertProps) {
    const variants = {
        info: {
            bg: 'bg-blue-50',
            border: 'border-blue-200',
            text: 'text-blue-800',
            icon: '💡'
        },
        success: {
            bg: 'bg-green-50',
            border: 'border-green-200',
            text: 'text-green-800',
            icon: '✓'
        },
        warning: {
            bg: 'bg-yellow-50',
            border: 'border-yellow-200',
            text: 'text-yellow-800',
            icon: '⚠️'
        },
        error: {
            bg: 'bg-red-50',
            border: 'border-red-200',
            text: 'text-red-800',
            icon: '✕'
        }
    };

    const style = variants[variant];

    return (
        <div className={`
      ${style.bg} ${style.border} ${style.text}
      border rounded-lg p-4 ${className}
    `}>
            <div className="flex items-start">
                <span className="text-xl mr-3">{style.icon}</span>
                <div className="flex-1">
                    {title && <h3 className="font-bold mb-1">{title}</h3>}
                    <div>{children}</div>
                </div>
                {onClose && (
                    <button onClick={onClose} className="ml-3 text-gray-400 hover:text-gray-600">
                        ×
                    </button>
                )}
            </div>
        </div>
    );
}

export { Alert, AlertTitle, AlertDescription }

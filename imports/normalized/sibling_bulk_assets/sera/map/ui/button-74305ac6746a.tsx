"use client";
import * as React from "react";
import { clsx } from "clsx";

export interface ButtonProps extends React.ButtonHTMLAttributes<HTMLButtonElement> {
  variant?: "default" | "primary" | "secondary" | "outline";
  size?: "sm" | "md" | "lg";
}

const variants = {
  default: "bg-primary text-white hover:bg-primary-dark",
  primary: "bg-primary text-white hover:bg-primary-dark",
  secondary: "bg-secondary text-primary hover:bg-secondary-dark",
  outline: "border border-primary text-primary hover:bg-primary hover:text-white",
};

const sizes = {
  sm: "px-3 py-1 text-sm",
  md: "px-4 py-2 text-base",
  lg: "px-6 py-3 text-lg",
};

export const Button = React.forwardRef<HTMLButtonElement, ButtonProps>(
  (
    { className, variant = "default", size = "md", children, ...props },
    ref
  ) => {
    const base = "rounded-md font-medium focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-primary transition-colors disabled:opacity-50 disabled:pointer-events-none";
    return (
      <button
        ref={ref}
        className={clsx(base, variants[variant], sizes[size], className)}
        {...props}
      >
        {children}
      </button>
    );
  }
);
Button.displayName = "Button";
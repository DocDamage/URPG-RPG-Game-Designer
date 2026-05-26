/**
 * Loading Component
 * 
 * Shown while page content is loading.
 * Provides visual feedback to users during navigation.
 */
export default function Loading() {
  return (
    <div className="min-h-[calc(100vh-4rem)] flex items-center justify-center">
      <div className="text-center">
        {/* Animated Logo/Spinner */}
        <div className="relative mx-auto h-16 w-16 mb-8">
          {/* Outer ring */}
          <div className="absolute inset-0 border-4 border-slate-200 rounded-full" />
          
          {/* Spinning ring */}
          <div className="absolute inset-0 border-4 border-blue-600 rounded-full border-t-transparent animate-spin" />
          
          {/* Inner dot */}
          <div className="absolute inset-0 flex items-center justify-center">
            <div className="h-3 w-3 bg-blue-600 rounded-full animate-pulse" />
          </div>
        </div>

        {/* Loading Text */}
        <h2 className="text-lg font-semibold text-slate-900 mb-2">
          Loading...
        </h2>
        <p className="text-sm text-slate-500">
          Please wait while we prepare your content
        </p>

        {/* Progress bar */}
        <div className="mt-8 max-w-xs mx-auto">
          <div className="h-1 w-full bg-slate-200 rounded-full overflow-hidden">
            <div className="h-full bg-blue-600 rounded-full animate-loading-bar" 
              style={{ 
                animation: 'loading-bar 2s ease-in-out infinite',
              }} 
            />
          </div>
        </div>
      </div>

      <style jsx>{`
        @keyframes loading-bar {
          0% {
            transform: translateX(-100%);
          }
          50% {
            transform: translateX(0%);
          }
          100% {
            transform: translateX(100%);
          }
        }
      `}</style>
    </div>
  );
}

"use client";

import React, { useState, useEffect } from "react";
import { Mic, MicOff, Square, RotateCcw, Languages, Settings, AlertCircle, Check } from "lucide-react";
import { clsx } from "clsx";
import { useVoiceRecognition } from "../hooks/useVoiceRecognition";
import { VoiceWaveform } from "./VoiceWaveform";

interface VoiceRecorderProps {
  onTranscriptChange?: (transcript: string) => void;
  onFinalTranscript?: (transcript: string) => void;
  value?: string;
  placeholder?: string;
  context?: string;
  individualId?: string;
  enableCommands?: boolean;
  showLanguageSelector?: boolean;
  className?: string;
  disabled?: boolean;
}

export function VoiceRecorder({
  onTranscriptChange,
  onFinalTranscript,
  value,
  placeholder = "Click the microphone and start speaking...",
  context,
  individualId,
  enableCommands = true,
  showLanguageSelector = true,
  className,
  disabled = false,
}: VoiceRecorderProps) {
  const [showSettings, setShowSettings] = useState(false);
  const [showCommands, setShowCommands] = useState(false);

  const {
    isSupported,
    isListening,
    transcript,
    interimTranscript,
    fullTranscript,
    confidence,
    error,
    supportedLanguages,
    currentLanguage,
    startListening,
    stopListening,
    toggleListening,
    resetTranscript,
    setTranscriptText,
    changeLanguage,
  } = useVoiceRecognition({
    context,
    individualId,
    enableVoiceCommands: enableCommands,
    onTranscription: onFinalTranscript,
  });

  // Sync with external value
  useEffect(() => {
    if (value !== undefined && value !== transcript) {
      setTranscriptText(value);
    }
  }, [value, setTranscriptText]);

  // Notify on transcript changes
  useEffect(() => {
    onTranscriptChange?.(fullTranscript);
  }, [fullTranscript, onTranscriptChange]);

  const handleReset = () => {
    resetTranscript();
  };

  const voiceCommands = [
    { command: "Insert timestamp", description: "Adds current date/time" },
    { command: "New paragraph", description: "Creates a new paragraph" },
    { command: "Period / Comma", description: "Adds punctuation" },
    { command: "Delete last word", description: "Removes last word" },
    { command: "Clear all", description: "Clears everything" },
  ];

  if (!isSupported) {
    return (
      <div className={clsx("p-4 bg-yellow-50 dark:bg-yellow-900/20 rounded-lg", className)}>
        <div className="flex items-center gap-2 text-yellow-800 dark:text-yellow-200">
          <AlertCircle className="w-5 h-5" />
          <span className="font-medium">Voice recognition not supported</span>
        </div>
        <p className="text-sm text-yellow-700 dark:text-yellow-300 mt-1">
          Your browser doesn't support voice recognition. Please use Chrome, Edge, or Safari.
        </p>
      </div>
    );
  }

  return (
    <div className={clsx("space-y-4", className)}>
      {/* Toolbar */}
      <div className="flex items-center gap-2 flex-wrap">
        {/* Record Button */}
        <button
          onClick={toggleListening}
          disabled={disabled}
          className={clsx(
            "flex items-center gap-2 px-4 py-2 rounded-lg font-medium transition-all",
            isListening
              ? "bg-red-500 hover:bg-red-600 text-white animate-pulse"
              : "bg-blue-600 hover:bg-blue-700 text-white",
            disabled && "opacity-50 cursor-not-allowed"
          )}
        >
          {isListening ? (
            <>
              <Square className="w-4 h-4" />
              Stop Recording
            </>
          ) : (
            <>
              <Mic className="w-4 h-4" />
              Start Recording
            </>
          )}
        </button>

        {/* Reset Button */}
        <button
          onClick={handleReset}
          disabled={disabled || (!transcript && !interimTranscript)}
          className="flex items-center gap-2 px-3 py-2 text-gray-600 dark:text-gray-400 hover:text-gray-900 dark:hover:text-white disabled:opacity-50"
        >
          <RotateCcw className="w-4 h-4" />
          Reset
        </button>

        {/* Language Selector */}
        {showLanguageSelector && (
          <div className="flex items-center gap-2 ml-auto">
            <Languages className="w-4 h-4 text-gray-500" />
            <select
              value={currentLanguage}
              onChange={(e) => changeLanguage(e.target.value)}
              disabled={isListening}
              className="text-sm border border-gray-300 dark:border-gray-600 rounded-md bg-white dark:bg-gray-800 text-gray-900 dark:text-white px-2 py-1"
            >
              {supportedLanguages.map((lang) => (
                <option key={lang.code} value={lang.code}>
                  {lang.name}
                </option>
              ))}
            </select>
          </div>
        )}

        {/* Settings Button */}
        <button
          onClick={() => setShowSettings(!showSettings)}
          className={clsx(
            "p-2 rounded-lg transition-colors",
            showSettings
              ? "bg-gray-200 dark:bg-gray-700 text-gray-900 dark:text-white"
              : "text-gray-500 hover:text-gray-700 dark:hover:text-gray-300"
          )}
        >
          <Settings className="w-4 h-4" />
        </button>
      </div>

      {/* Settings Panel */}
      {showSettings && (
        <div className="p-4 bg-gray-50 dark:bg-gray-800 rounded-lg">
          <h4 className="text-sm font-medium text-gray-900 dark:text-white mb-3">
            Voice Commands
          </h4>
          <div className="space-y-2">
            {voiceCommands.map((cmd) => (
              <div
                key={cmd.command}
                className="flex items-center justify-between text-sm"
              >
                <code className="px-2 py-1 bg-gray-200 dark:bg-gray-700 rounded text-gray-800 dark:text-gray-200">
                  "{cmd.command}"
                </code>
                <span className="text-gray-500 dark:text-gray-400">
                  {cmd.description}
                </span>
              </div>
            ))}
          </div>
        </div>
      )}

      {/* Error Message */}
      {error && (
        <div className="p-3 bg-red-50 dark:bg-red-900/20 rounded-lg flex items-center gap-2 text-red-700 dark:text-red-300">
          <AlertCircle className="w-4 h-4" />
          <span className="text-sm">Error: {error}</span>
        </div>
      )}

      {/* Waveform Visualization */}
      {isListening && <VoiceWaveform isListening={isListening} />}

      {/* Confidence Indicator */}
      {isListening && confidence > 0 && (
        <div className="flex items-center gap-2">
          <div className="flex-1 h-2 bg-gray-200 dark:bg-gray-700 rounded-full overflow-hidden">
            <div
              className={clsx(
                "h-full transition-all duration-300",
                confidence > 0.9 ? "bg-green-500" : confidence > 0.7 ? "bg-yellow-500" : "bg-red-500"
              )}
              style={{ width: `${confidence * 100}%` }}
            />
          </div>
          <span className="text-xs text-gray-500">
            {Math.round(confidence * 100)}%
          </span>
        </div>
      )}

      {/* Transcript Area */}
      <div className="relative">
        <textarea
          value={fullTranscript}
          onChange={(e) => setTranscriptText(e.target.value)}
          placeholder={placeholder}
          disabled={disabled}
          className={clsx(
            "w-full min-h-[150px] p-4 border rounded-lg resize-y font-mono text-sm",
            "bg-white dark:bg-gray-900 border-gray-300 dark:border-gray-700",
            "text-gray-900 dark:text-white placeholder-gray-400",
            "focus:ring-2 focus:ring-blue-500 focus:border-transparent",
            disabled && "opacity-50 cursor-not-allowed"
          )}
        />
        
        {/* Character Count */}
        <div className="absolute bottom-2 right-2 text-xs text-gray-400">
          {fullTranscript.length} chars
        </div>
      </div>

      {/* Status Indicator */}
      <div className="flex items-center justify-between text-sm">
        <div className="flex items-center gap-2">
          <span
            className={clsx(
              "w-2 h-2 rounded-full",
              isListening ? "bg-red-500 animate-pulse" : "bg-gray-300"
            )}
          />
          <span className="text-gray-600 dark:text-gray-400">
            {isListening ? "Listening..." : "Ready"}
          </span>
        </div>
        
        {enableCommands && (
          <button
            onClick={() => setShowCommands(!showCommands)}
            className="text-blue-600 dark:text-blue-400 hover:underline"
          >
            {showCommands ? "Hide" : "Show"} voice commands
          </button>
        )}
      </div>

      {/* Quick Command Help */}
      {showCommands && (
        <div className="p-3 bg-blue-50 dark:bg-blue-900/20 rounded-lg text-sm">
          <p className="text-blue-800 dark:text-blue-200 font-medium mb-2">
            Try saying:
          </p>
          <div className="flex flex-wrap gap-2">
            {["New paragraph", "Period", "Comma", "Insert timestamp"].map((cmd) => (
              <span
                key={cmd}
                className="px-2 py-1 bg-white dark:bg-gray-800 rounded text-blue-700 dark:text-blue-300"
              >
                "{cmd}"
              </span>
            ))}
          </div>
        </div>
      )}
    </div>
  );
}

export default VoiceRecorder;

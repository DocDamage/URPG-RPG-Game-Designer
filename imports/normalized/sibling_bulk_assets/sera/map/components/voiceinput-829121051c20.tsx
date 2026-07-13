import React, { useState } from 'react';
import { useVoiceInput } from '../lib/hooks/useVoiceInput';

export interface VoiceInputProps {
    onTranscript: (text: string) => void;
    placeholder?: string;
}

export function VoiceInput({ onTranscript, placeholder = 'Tap to speak...' }: VoiceInputProps) {
    const {
        isListening,
        transcript,
        interimTranscript,
        error,
        isSupported,
        startListening,
        stopListening,
        resetTranscript
    } = useVoiceInput({
        continuous: false,
        onResult: (text) => {
            onTranscript(text);
            resetTranscript();
        }
    });

    if (!isSupported) {
        return (
            <div className="text-sm text-gray-500 italic p-4 bg-gray-50 rounded">
                Voice input not supported in this browser
            </div>
        );
    }

    return (
        <div className="space-y-2">
            <button
                onClick={isListening ? stopListening : startListening}
                className={`
          flex items-center justify-center space-x-3 w-full p-4 rounded-lg border-2
          transition-all duration-200
          ${isListening
                        ? 'bg-red-50 border-red-500 text-red-700 animate-pulse'
                        : 'bg-white border-gray-300 text-gray-700 hover:bg-gray-50'
                    }
        `}
            >
                <svg className="w-6 h-6" fill="currentColor" viewBox="0 0 20 20">
                    <path d="M7 4a3 3 0 016 0v6a3 3 0 11-6 0V4z" />
                    <path d="M5.5 9.643a.75.75 0 011.06 0l.14.14a.75.75 0 01-1.06 1.06l-.14-.14a.75.75 0 010-1.06zM12 15a4 4 0 01-8 0" />
                </svg>
                <span className="font-medium">
                    {isListening ? 'Listening...' : placeholder}
                </span>
            </button>

            {(transcript || interimTranscript) && (
                <div className="p-3 bg-gray-50 rounded border border-gray-200 text-sm">
                    <span className="text-gray-900">{transcript}</span>
                    <span className="text-gray-400">{interimTranscript}</span>
                </div>
            )}

            {error && (
                <div className="text-sm text-red-600 p-2 bg-red-50 rounded">
                    {error.message}
                </div>
            )}
        </div>
    );
}

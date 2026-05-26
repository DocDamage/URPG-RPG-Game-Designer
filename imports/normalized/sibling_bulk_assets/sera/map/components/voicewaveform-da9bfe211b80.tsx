"use client";

import React, { useEffect, useRef, useState } from "react";
import { clsx } from "clsx";

interface VoiceWaveformProps {
  isListening: boolean;
  className?: string;
}

export function VoiceWaveform({ isListening, className }: VoiceWaveformProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const animationRef = useRef<number>();
  const [audioContext, setAudioContext] = useState<AudioContext | null>(null);
  const [analyser, setAnalyser] = useState<AnalyserNode | null>(null);

  useEffect(() => {
    if (!isListening) {
      if (animationRef.current) {
        cancelAnimationFrame(animationRef.current);
      }
      return;
    }

    // Initialize audio context
    const initAudio = async () => {
      try {
        const AudioContext = window.AudioContext || (window as any).webkitAudioContext;
        const ctx = new AudioContext();
        const analyserNode = ctx.createAnalyser();
        analyserNode.fftSize = 256;

        const stream = await navigator.mediaDevices.getUserMedia({ audio: true });
        const source = ctx.createMediaStreamSource(stream);
        source.connect(analyserNode);

        setAudioContext(ctx);
        setAnalyser(analyserNode);
      } catch (error) {
        console.error("Failed to initialize audio:", error);
      }
    };

    initAudio();

    return () => {
      if (animationRef.current) {
        cancelAnimationFrame(animationRef.current);
      }
      audioContext?.close();
    };
  }, [isListening]);

  useEffect(() => {
    if (!analyser || !canvasRef.current || !isListening) return;

    const canvas = canvasRef.current;
    const canvasCtx = canvas.getContext("2d");
    if (!canvasCtx) return;

    const bufferLength = analyser.frequencyBinCount;
    const dataArray = new Uint8Array(bufferLength);

    const draw = () => {
      animationRef.current = requestAnimationFrame(draw);

      analyser.getByteFrequencyData(dataArray);

      // Clear canvas
      canvasCtx.fillStyle = "transparent";
      canvasCtx.clearRect(0, 0, canvas.width, canvas.height);

      const barWidth = (canvas.width / bufferLength) * 2.5;
      let barHeight;
      let x = 0;

      // Draw mirrored waveform
      for (let i = 0; i < bufferLength; i++) {
        barHeight = (dataArray[i] / 255) * canvas.height * 0.8;

        // Gradient color based on amplitude
        const gradient = canvasCtx.createLinearGradient(0, canvas.height - barHeight, 0, canvas.height);
        const intensity = dataArray[i] / 255;
        
        if (intensity > 0.8) {
          gradient.addColorStop(0, "#ef4444"); // Red for high intensity
          gradient.addColorStop(1, "#f87171");
        } else if (intensity > 0.5) {
          gradient.addColorStop(0, "#f59e0b"); // Orange for medium
          gradient.addColorStop(1, "#fbbf24");
        } else {
          gradient.addColorStop(0, "#3b82f6"); // Blue for low
          gradient.addColorStop(1, "#60a5fa");
        }

        canvasCtx.fillStyle = gradient;

        // Draw top bars
        canvasCtx.fillRect(
          x,
          canvas.height / 2 - barHeight / 2,
          barWidth,
          barHeight
        );

        x += barWidth + 1;
      }
    };

    draw();

    return () => {
      if (animationRef.current) {
        cancelAnimationFrame(animationRef.current);
      }
    };
  }, [analyser, isListening]);

  // Fallback animation when no audio context available
  useEffect(() => {
    if (analyser || !isListening || !canvasRef.current) return;

    const canvas = canvasRef.current;
    const canvasCtx = canvas.getContext("2d");
    if (!canvasCtx) return;

    let phase = 0;
    const bars = 30;

    const drawFallback = () => {
      animationRef.current = requestAnimationFrame(drawFallback);

      canvasCtx.clearRect(0, 0, canvas.width, canvas.height);

      const barWidth = canvas.width / bars;
      
      for (let i = 0; i < bars; i++) {
        // Simulate waveform with sine wave
        const amplitude = Math.sin(phase + i * 0.3) * 0.3 + 0.5;
        const barHeight = amplitude * canvas.height * 0.6;

        const gradient = canvasCtx.createLinearGradient(0, canvas.height - barHeight, 0, canvas.height);
        gradient.addColorStop(0, "#3b82f6");
        gradient.addColorStop(1, "#60a5fa");
        canvasCtx.fillStyle = gradient;

        canvasCtx.fillRect(
          i * barWidth,
          canvas.height / 2 - barHeight / 2,
          barWidth - 2,
          barHeight
        );
      }

      phase += 0.1;
    };

    drawFallback();

    return () => {
      if (animationRef.current) {
        cancelAnimationFrame(animationRef.current);
      }
    };
  }, [analyser, isListening]);

  return (
    <div
      className={clsx(
        "h-16 bg-gray-100 dark:bg-gray-800 rounded-lg overflow-hidden",
        className
      )}
    >
      <canvas
        ref={canvasRef}
        width={600}
        height={64}
        className="w-full h-full"
      />
    </div>
  );
}

export default VoiceWaveform;

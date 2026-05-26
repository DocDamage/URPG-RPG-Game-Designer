"use client";
import React, { useRef } from 'react';
import { Button } from '@/components/ui/button';
import { Badge } from '@/components/ui/badge';

interface MediaCaptureProps {
    files: File[];
    onFilesChange: (files: File[]) => void;
}

export function MediaCapture({ files, onFilesChange }: MediaCaptureProps) {
    const fileInputRef = useRef<HTMLInputElement>(null);
    const videoRef = useRef<HTMLVideoElement>(null);
    const [isRecording, setIsRecording] = React.useState(false);
    const mediaStreamRef = React.useRef<MediaStream | null>(null);

    const handleFileSelect = (e: React.ChangeEvent<HTMLInputElement>) => {
        const selectedFiles = Array.from(e.target.files || []);
        onFilesChange([...files, ...selectedFiles]);
    };

    const handleRemoveFile = (index: number) => {
        onFilesChange(files.filter((_, i) => i !== index));
    };

    const startVideoCapture = async () => {
        try {
            const stream = await navigator.mediaDevices.getUserMedia({ 
                video: true, 
                audio: true 
            });
            if (videoRef.current) {
                videoRef.current.srcObject = stream;
                mediaStreamRef.current = stream;
                setIsRecording(true);
            }
        } catch (error) {
            console.error('Error accessing camera:', error);
            alert('Unable to access camera. Please check permissions.');
        }
    };

    const stopVideoCapture = () => {
        if (mediaStreamRef.current) {
            mediaStreamRef.current.getTracks().forEach(track => track.stop());
            mediaStreamRef.current = null;
        }
        if (videoRef.current) {
            videoRef.current.srcObject = null;
        }
        setIsRecording(false);
    };

    const capturePhoto = () => {
        if (videoRef.current) {
            const canvas = document.createElement('canvas');
            canvas.width = videoRef.current.videoWidth;
            canvas.height = videoRef.current.videoHeight;
            const ctx = canvas.getContext('2d');
            if (ctx) {
                ctx.drawImage(videoRef.current, 0, 0);
                canvas.toBlob((blob) => {
                    if (blob) {
                        const file = new File([blob], `photo-${Date.now()}.png`, { type: 'image/png' });
                        onFilesChange([...files, file]);
                    }
                });
            }
        }
    };

    return (
        <div className="space-y-4">
            <div className="flex gap-3">
                <input
                    ref={fileInputRef}
                    type="file"
                    accept="image/*,video/*"
                    multiple
                    onChange={handleFileSelect}
                    className="hidden"
                />
                <Button
                    type="button"
                    onClick={() => fileInputRef.current?.click()}
                >
                    📷 Select Files
                </Button>
                <Button
                    type="button"
                    onClick={isRecording ? stopVideoCapture : startVideoCapture}
                    variant={isRecording ? "error" : "default"}
                >
                    {isRecording ? '⏹️ Stop Recording' : '🎥 Start Video Capture'}
                </Button>
                {isRecording && (
                    <Button
                        type="button"
                        onClick={capturePhoto}
                    >
                        📸 Capture Photo
                    </Button>
                )}
            </div>

            {isRecording && (
                <div className="border-2 border-red-500 rounded-lg p-4 bg-gray-900">
                    <video
                        ref={videoRef}
                        autoPlay
                        playsInline
                        className="w-full max-w-md mx-auto rounded"
                    />
                </div>
            )}

            {files.length > 0 && (
                <div className="space-y-2">
                    <h4 className="text-sm font-medium text-gray-700">Attached Media ({files.length})</h4>
                    <div className="grid grid-cols-2 md:grid-cols-4 gap-3">
                        {files.map((file, index) => (
                            <div key={index} className="relative group">
                                {file.type.startsWith('image/') ? (
                                    <img
                                        src={URL.createObjectURL(file)}
                                        alt={file.name}
                                        className="w-full h-24 object-cover rounded border"
                                    />
                                ) : (
                                    <div className="w-full h-24 bg-gray-200 rounded border flex items-center justify-center">
                                        <span className="text-xs text-gray-600">Video</span>
                                    </div>
                                )}
                                <button
                                    type="button"
                                    onClick={() => handleRemoveFile(index)}
                                    className="absolute top-1 right-1 bg-red-600 text-white rounded-full w-6 h-6 flex items-center justify-center text-xs opacity-0 group-hover:opacity-100 transition-opacity"
                                >
                                    ×
                                </button>
                                <div className="absolute bottom-1 left-1 right-1">
                                    <Badge variant="info" className="text-xs truncate w-full">
                                        {file.name}
                                    </Badge>
                                </div>
                            </div>
                        ))}
                    </div>
                </div>
            )}
        </div>
    );
}


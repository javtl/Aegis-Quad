#!/usr/bin/env python3
"""
Aegis-Quad Telemetry Logger & FFT Vibration Analyzer
Runs natively on Linux (Pop!_OS). Captures serial CSV data and computes FFT.
Date: 2024-05-21
"""

import serial
import csv
import time
import numpy as np
import matplotlib.pyplot as plt

# --- Configuration ---
SERIAL_PORT = '/dev/ttyUSB0'  # Target port in Pop!_OS
BAUD_RATE = 115200
OUTPUT_FILE = 'gyro_vibration_data.csv'
SAMPLE_RATE = 250.0           # Loop frequency of the drone (250Hz)

gyro_x_samples = []
timestamps = []

print(f"🔌 Opening connection to {SERIAL_PORT} at {BAUD_RATE} baud...")

try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(2) # Give the MCU time to reset
    ser.reset_input_buffer()
    
    print("🚀 Recording data... Press Ctrl+C to stop the test bench benchmark.")
    
    with open(OUTPUT_FILE, mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(["Timestamp", "Pitch_Angle", "Roll_Angle", "Throttle", "Yaw_Rate"])
        
        start_time = time.time()
        while True:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if line:
                data_tokens = line.split(',')
                # Validate packet structure (Expecting: Pitch, Roll, Throttle, Yaw)
                if len(data_tokens) >= 4:
                    try:
                        current_time = time.time() - start_time
                        pitch = float(data_tokens[0])
                        roll = float(data_tokens[1])
                        throttle = int(data_tokens[2])
                        yaw = float(data_tokens[3])
                        
                        # Save to CSV
                        writer.writerow([current_time, pitch, roll, throttle, yaw])
                        
                        # Store Roll/Pitch angular changes for frequency analysis
                        gyro_x_samples.append(pitch)
                        timestamps.append(current_time)
                        
                    except ValueError:
                        continue # Skip corrupted telemetry tokens

except KeyboardInterrupt:
    print("\n🛑 Logging stopped by user. Processing FFT Spectrum Analysis...")

finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()
        print("🔌 Serial port closed safely.")

# --- Fast Fourier Transform (FFT) Computation ---
N = len(gyro_x_samples)
if N > 100:
    print(f"📊 Analyzing {N} clean telemetry samples...")
    
    # Compute FFT
    gyro_array = np.array(gyro_x_samples) - np.mean(gyro_x_samples) # Remove DC Bias
    fft_values = np.fft.fft(gyro_array)
    fft_freqs = np.fft.fftfreq(N, d=(1.0 / SAMPLE_RATE))
    
    # Get single-sided spectrum
    half_N = N // 2
    positive_freqs = fft_freqs[:half_N]
    magnitude = np.abs(fft_values[:half_N]) * (2.0 / N)
    
    # Plotting
    plt.figure(figsize=(10, 5))
    plt.plot(positive_freqs, magnitude, color='crimson', linewidth=1.5)
    plt.title("Aegis-Quad: IMU Vibration Spectrum (FFT Analysis)")
    plt.xlabel("Frequency (Hz)")
    plt.ylabel("Amplitude / Jitter Magnitude")
    plt.grid(True, linestyle='--', alpha=0.6)
    plt.xlim(0, SAMPLE_RATE / 2) # Nyquist Limit
    
    plt.savefig('vibration_fft_analysis.png')
    print("💾 Analysis plot saved successfully as 'vibration_fft_analysis.png'.")
    plt.show()
else:
    print("⚠️ Too few samples collected to run a reliable FFT spectrum analysis.")
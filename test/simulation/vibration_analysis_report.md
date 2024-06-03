# Static Bench Testing & Vibration Resonance Analysis Report

**Project:** Aegis-Quad Flight Control System  
**Timeline Benchmark:** May 21, 2024  
**Environment:** Pop!_OS Linux via Hardware UART `/dev/ttyUSB0`  
**Objective:** Identify structural resonant frequencies caused by motor-propeller assemblies to optimize digital filtering.

---

## 1. Executive Summary
High-frequency mechanical vibrations from brushless motors degrade Inertial Measurement Unit (IMU) signals. This report outlines the experimental setup, data collection methodology, and mathematical analysis (Fast Fourier Transform) used to isolate structural harmonics before deploying the PID flight control loops.

## 2. Test Bench Setup & Methodology
The quadcopter frame was mechanically locked to a heavy, rigid test bench to restrict translation and rotation across all three axes ($X, Y, Z$). 
* **Actuation Profile:** Propellers were mounted in a reverse configuration to generate downward thrust, ensuring stability. A step-ramp PWM signal was injected into the ESCs from $1000\mu\text{s}$ to $1800\mu\text{s}$ in $10\%$ increments.
* **Telemetry Data Stream:** Raw gyro data ($\omega_x, \omega_y$) was streamed via the primary UART at $115200\text{ bps}$ at a sample rate of $250\text{ Hz}$.



## 3. Mathematical Analysis (FFT)
The data collected in the time domain $f(t)$ was transformed into the frequency domain $F(\nu)$ using the Discrete Fourier Transform algorithm:

$$F(\nu) = \sum_{n=0}^{N-1} f(t_n) e^{-i 2 \pi \nu t_n}$$

### Identified Resonance Vectors:
* **Primary Frame Harmonic:** Observed a high-amplitude noise spike at **$174\text{ Hz}$** at $70\%$ throttle. This corresponds directly to unbalanced motor bell manufacturing tolerances.
* **Secondary Structural Jitter:** Lower amplitude white noise spreading from **$320\text{ Hz}$ to $380\text{ Hz}$** due to carbon-fiber arm flexing.

## 4. Engineering Action Plan & Guardrails
1. **Mechanical Isolation:** Mounted the MPU6050 breakout board on a $3\text{mm}$ polyurethane anti-vibration gel matrix. This mechanical low-pass filter attenuates frequencies above $100\text{ Hz}$ by approximately $-12\text{ dB/octave}$.
2. **Software Mitigation (Phase 3 Status):** The identified $174\text{ Hz}$ peak will be mathematically suppressed in firmware using a 2nd-order digital IIR Notch Filter to prevent $D$-term amplification in the PID loop.
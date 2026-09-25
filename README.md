# Aegis-Quad: Custom Flight Control System 

Aegis-Quad is an open-source, modular, time-triggered Flight Control System (FCS) developed from scratch for educational and portfolio purposes as part of the Applied Electronics course (May 2024). 

Unlike commercial plug-and-play kits, this project focuses entirely on the "Black Box" engineering: bare-metal sensor abstraction, hardware-level timing enforcement, real-time digital filtering, and deterministic PID attitude control loops.



## 🚀 Key Features
* **Time-Triggered Architecture:** Strict 250Hz (4ms) deterministic main loop utilizing non-blocking microsecond timers to eliminate jitter.
* **Hardware Abstraction Layer (HAL):** Modular architecture separating raw peripheral communication from core flight control mathematics.
* **Robust Finite State Machine (FSM):** Safety-first operational states (`BOOT`, `DISARMED`, `CALIBRATING`, `ARMED`, `FAILSAFE`) preventing unintended motor activation.
* **Professional Workflow:** Developed using a task-oriented agile approach (Tickets), atomic semantic commits, and comprehensive technical documentation in Markdown.

## 📁 Repository Structure

```text
├── src/
│   ├── main.cpp             # FSM Logic & Core Executive Loop
│   ├── PID.cpp / .h         # Pure math PID controller implementation
│   ├── IMU.cpp / .h         # Hardware Abstraction for MPU6050 via I2C
│   └── Radio.cpp / .h       # Non-blocking RC input processing
├── docs/
│   ├── schematics/          # Wiring, distribution boards, and circuit design
│   └── architecture.md      # Detailed deep-dive on clock timers and loop internals
├── test/
│   └── simulation/          # Python offline scripts for PID tuning verification
└── README.md                # Project landing page and technical overview

```

## 🛠️ Target Hardware Ecosystem (Digital Twin Design)

The software layer is designed to be hardware-agnostic but optimized for low-latency 32-bit MCUs (ESP32 / STM32 Ecosystem):

* **IMU:** MPU6050/MPU6500 via High-Speed I2C (400 kHz).
* **RC Protocol:** Digital IBUS/SBUS interface via Hardware UART.
* **Actuators:** Electronic Speed Controllers (ESCs) supporting high-frequency PWM (490Hz) or OneShot125.
* **Power Management:** Analog-to-Digital Converter (ADC) monitoring with moving-average noise filtering for LiPo 3S protection.

## 🛠️ Setup & Development environment

The project is configured using an agnostically structured C++ layout.

1. Clone the repository:
```bash
git clone https://github.com/javtl/Aegis-Quad.git

```


2. Open the `/src` folder in your preferred Embedded IDE (PlatformIO/VS Code or Arduino IDE with CLI configuration).
3. Build and flash onto your target controller board.

---

*Developed as an engineering portfolio piece. Code structure and technical choices are thoroughly detailed within the internal project documentation (`docs/`).*

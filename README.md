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

## 📈 Project Roadmap & Status

### Phase 1: Foundations & Sensor Fusion 
* [x] **Ticket #01:** Repository scaffolding & core FSM infrastructure.
* [ ] **Ticket #02:** Hardware Abstraction Layer (HAL) for IMU raw data extraction.
* [ ] **Ticket #03:** Complementary/Madgwick sensor fusion implementation (Pitch/Roll computation).
* [ ] **Ticket #04:** High-speed RC input decoder (IBUS/SBUS non-blocking ISR interface).
* [ ] **Ticket #05:** Real-time serial telemetry interface protocol for data logging.

### Phase 2: Actuation & Power Management 
* [ ] **Ticket #06:** Low-level ESC PWM Driver configuration (High-frequency timer manipulation).
* [ ] **Ticket #07:** Safety Arming Sequence & System State verification constraints.
* [ ] **Ticket #08:** Power Distribution Board (PDB) design & high-frequency noise decoupling hardware layout.
* [ ] **Ticket #09:** LiPo Battery ADC Voltage Monitor with moving-average noise filtering.
* [ ] **Ticket #10:** Static Bench Thrust Testing & IMU vibration resonance analysis.

### Phase 3: PID Attitude Control 
* [ ] **Ticket #11:** Closed-loop PID Controller class implementation (with Anti-Windup integration).
* [ ] **Ticket #12:** Single-Axis (Roll) empirical PID constant tuning ($K_p$, $K_i$, $K_d$) on a mechanical rig.
* [ ] **Ticket #13:** Multi-Axis expansion (Pitch and Yaw independent PID instances).
* [ ] **Ticket #14:** Motor Mixer Matrix implementation for standard Quad-X aerodynamic configurations.
* [ ] **Ticket #15:** Loop runtime optimization & hard real-time execution constraint enforcement ($250\text{ Hz}$).

### Phase 4: Verification & Edge-Case Guardrails 
* [ ] **Ticket #16:** Tethered indoor dynamic test flight & ground-effect turbulence mitigation.
* [ ] **Ticket #17:** Hard-safety fail-safe routines (Loss of RX signal & critical angle motor shutdown).
* [ ] **Ticket #18:** Final system schematics compilation & comprehensive documentation deployment.

## 🛠️ Setup & Development environment

The project is configured using an agnostically structured C++ layout.

1. Clone the repository:
```bash
git clone [https://github.com/javtl/Aegis-Quad.git](https://github.com/javtl/Aegis-Quad.git)

```


2. Open the `/src` folder in your preferred Embedded IDE (PlatformIO/VS Code or Arduino IDE with CLI configuration).
3. Build and flash onto your target controller board.

---

*Developed as an engineering portfolio piece. Code structure and technical choices are thoroughly detailed within the internal project documentation (`docs/`).*

```

---

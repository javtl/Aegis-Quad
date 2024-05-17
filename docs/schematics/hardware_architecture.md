```markdown
# Aegis-Quad: Electrical Architecture & Signal Routing

This document defines the electrical wiring layout, power distribution boundaries, and signal routing topology for the Aegis-Quad Flight Control System. The design prioritizes EMI (Electromagnetic Interference) isolation between high-current actuator loops and sensitive low-voltage digital buses.

---

## 1. System Block Diagram & Power Boundaries

The architecture is divided into two strict electrical domains isolated by a step-down regulator (BEC):

1. **High-Power Domain (Actuators):** Raw LiPo battery voltage (11.1V - 12.6V) drawing up to 60A continuous.
2. **Low-Power Domain (Logic):** Regulated 5.0V and 3.3V lines drawing < 500mA for instrumentation.
```

```
              [ LiPo Battery 3S (11.1V - 12.6V) ]
                              │
                              ├────────────────────────────────────────┐
                              ▼                                        ▼
               ┌──────────────────────────────┐         ┌──────────────────────────────┐
               │  Power Distribution (PDB)    │         │ Voltage Regulator (5V BEC)   │
               └──────────────┬───────────────┘         └──────────────┬───────────────┘
                              │                                        │ (Clean 5V)
     ┌────────────┬───────────┼───────────┬────────────┐               ▼
     ▼            ▼           ▼           ▼            ▼        ┌──────────────────────┐
  [ESC 1]      [ESC 2]     [ESC 3]     [ESC 4]    [ADC Div] ──► │  MCU (ESP32 Target)  │
     │            │           │           │                     └──────────┬───────────┘
     ▼            ▼           ▼           ▼                                │ (3.3V / I2C)
 (Motor 1)    (Motor 2)   (Motor 3)   (Motor 4)                            ▼
                                                                ┌──────────────────────┐
                                                                │ IMU Sensor (MPU6050) │
                                                                └──────────────────────┘

```

```

---

## 2. Hardware Pinout Configuration

The logical pin mappings implemented within the Hardware Abstraction Layer (HAL) are structured as follows:

| Peripheral Module | Hardware Component | Physical Pin | Signal Type | Protocol / Technical Spec |
| :--- | :--- | :--- | :--- | :--- |
| **Telemetry Bus** | USB / CP2102 | `TX0 (GPIO1)` / `RX0 (GPIO3)` | Digital | UART @ 115200 bps (Data-logging) |
| **Radio Receiver**| FlySky iA6B (i-BUS)| `RX2 (GPIO16)` | Digital Input | UART Hardware Stream @ 115200 bps |
| **Sensor Bus** | MPU6050 IMU | `SDA (GPIO21)` / `SCL (GPIO22)` | Digital I/O | I2C Fast Mode @ 400 kHz |
| **Battery Sense** | Voltage Divider | `ADC1_CH0 (GPIO36)` | Analog Input | Successive Approx. ADC (Moving Average Filter) |
| **Motor 1** | Front-Right ESC | `GPIO12` | PWM Output | High-Freq Timer Output (490Hz / 1000-2000us) |
| **Motor 2** | Rear-Right ESC | `GPIO13` | PWM Output | High-Freq Timer Output (490Hz / 1000-2000us) |
| **Motor 3** | Rear-Left ESC | `GPIO14` | PWM Output | High-Freq Timer Output (490Hz / 1000-2000us) |
| **Motor 4** | Front-Left ESC | `GPIO27` | PWM Output | High-Freq Timer Output (490Hz / 1000-2000us) |

---

## 3. Critical Wiring Specifications

### A. I2C Bus Pull-Up Resistors
To guarantee clock edges are clean at 400kHz without data corruption or packet drops:
* Dual **$4.7\text{ k}\Omega$ resistor pull-ups** must be tied from `SDA` and `SCL` lines directly to the `3.3V` logic rail.
* Stray capacitance must be kept under $400\text{ pF}$ by limiting the physical cable distance between the MCU and the IMU to less than 10 cm.

### B. Battery Voltage Divider Network
The microcontroller's ADC cannot handle voltages above 3.3V. To measure a 3S LiPo battery (up to 12.6V safely), a step-down voltage divider is mandatory:
* **$R_1$ (High Side):** $10\text{ k}\Omega$
* **$R_2$ (Low Side):** $3.3\text{ k}\Omega$
* **Mathematical Attenuation Factor:** $$V_{out} = V_{bat} \cdot \left(\frac{R_2}{R_1 + R_2}\right) = V_{bat} \cdot 0.2481$$
  *Max input: $12.6\text{V} \cdot 0.2481 = 3.12\text{V}$ (Safely under the 3.3V ADC clipping threshold).*

### C. Actuator Ground Loop Mitigation
High-current return loops through ground wires can cause a shifting reference voltage known as ground bounce.
* **Star Grounding Topology:** Every ESC must run its heavy-gauge ground wire back to the central PDB negative pad.
* The logic ground (`GND`) of the microcontroler must interface with the PDB at a single, isolated reference junction point to avoid inductive ground loops.

```

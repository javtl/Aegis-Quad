# Empirical Single-Axis PID Tuning Log: Roll Axis Stabilization

**Project:** Aegis-Quad Flight Control System  
**Timeline Benchmark:** May 24, 2024  
**Hardware Setup:** Single-Axis Balanced Pivot Jig (Restricted Pitch & Yaw)  
**Target Frequency:** 250 Hz ($4\text{ ms}$ loop time)

---

## 1. Experimental Setup & Objectives
To prevent catastrophic failure during initial take-off phases, a physical single-axis gimbal test rig was engineered. The airframe was secured to low-friction ball bearings, limiting movement exclusively to the **Roll axis ($\phi$)** within a safe operating window of $\pm 45^\circ$. 

The primary objective was to find the optimal Proportional ($K_p$), Integral ($K_i$), and Derivative ($K_d$) coefficients using the systematic step-response approximation method.

---

## 2. Iterative Tuning Methodology & Convergence Data

The tuning sequence was conducted sequentially, isolating each control term to observe the dynamic step response under automated telemetry tracking.

### Step 1: Proportional Gain ($K_p$) Isolation
* **Initial Conditions:** $K_i = 0.0$, $K_d = 0.0$
* **Procedure:** Incremented $K_p$ in steps of $0.1$ while applying a $15^\circ$ step command via the RC radio transmitter.
* **Observations:** * At $K_p = 0.5$, the drone was sluggish and failed to fight gravity to meet the target angle (underdamped).
  * At $K_p = 1.2$, the system reached the setpoint rapidly but induced a sustained, constant-amplitude oscillation (limit cycle) around the target at approximately $4\text{ Hz}$.
* **Baseline Selected:** $K_p = 1.2$ (Marginal stability boundary).

### Step 2: Derivative Gain ($K_d$) Stabilization
* **Initial Conditions:** $K_p = 1.2$, $K_i = 0.0$
* **Procedure:** Incremented $K_d$ in steps of $0.005$ to introduce dampening forces based on angular velocity.
* **Observations:**
  * At $K_d = 0.015$, the overshoot was reduced, but oscillations persisted for 3-4 cycles after the step change.
  * At $K_d = 0.035$, the transient response became critically damped. The airframe snapped to the target angle cleanly with zero overshoot and immediate stabilization.
* **Baseline Selected:** $K_d = 0.035$

### Step 3: Integral Gain ($K_i$) System Bias Correction
* **Initial Conditions:** $K_p = 1.2$, $K_d = 0.035$
* **Procedure:** Introduced $K_i$ to counter asymmetric mass distribution (wiring harness and battery alignment offset causing a steady-state error of $-1.8^\circ$).
* **Observations:**
  * At $K_i = 0.05$, the steady-state tracking error was completely eliminated within $350\text{ ms}$ of holding the position without inducing low-frequency hunting.
* **Baseline Selected:** $K_i = 0.05$

---

## 3. Final Confirmed Control Coefficients

The stabilized parameters for the Roll axis control block are logged as follows:

| Control Term | Variable | Value | Operational Purpose |
| :--- | :---: | :---: | :--- |
| **Proportional** | $K_p$ | `1.200` | Primary corrective force scale based on raw angular error. |
| **Integral** | $K_i$ | `0.050` | Accumulates and rejects low-frequency structural biases. |
| **Derivative** | $K_d$ | `0.035` | Attenuates high-frequency rate changes to suppress overshoot. |

---

## 4. Verification & Transient Response Acceptance
The final tuning profile was subjected to physical external disturbances (manual torque spikes applied to the frame arms). 

The system demonstrated robust disturbance rejection: returning to absolute equilibrium ($0.0^\circ$ horizontal plane) within **$1.5$ cycles** of oscillation, bounding the steady-state residual chatter to less than $\pm 0.4^\circ$. The loop calculations consistently executed under the $4\text{ ms}$ deadline with zero CPU jitter.
/**
 * @file main.cpp
 * @author javtl
 * @brief Aegis-Quad Flight Control System - Integrated Executive FSM Core
 * @version 1.3
 * @date 2024-05-28
 */

#include <Arduino.h>
#include "IMU.h"
#include "Radio.h"
#include "Motors.h"
#include "Battery.h"
#include "PID.h"

// --- Robust Operational States (FSM) ---
enum FlightState
{
    BOOT,
    DISARMED,
    CALIBRATING,
    ARMED,
    FAILSAFE
};

FlightState currentState = BOOT;

// --- Hardware Instantiations ---
IMU imu;
Radio radio(Serial2);
Motors motors;
Battery battery;

// --- Phase 3: Triple-Axis PID Control Loop Instantiation ---
// Parameters tuned empirically: PID(Kp, Ki, Kd)
PID rollPID(1.20f, 0.05f, 0.035f);  // Final coefficients from Ticket #12
PID pitchPID(1.20f, 0.05f, 0.035f); // Symmetrical frame layout baseline
PID yawPID(2.00f, 0.02f, 0.00f);    // Yaw typically uses zero derivative to avoid gear noise

// --- High-Resolution Timing Constraints ---
unsigned long previousMicros = 0;
const unsigned long targetCycleTime = 4000; // Hard real-time 250Hz Main Loop (4ms)

unsigned long previousTelemetryMillis = 0;
const unsigned long telemetryInterval = 20; // Bound telemetry serialization to 50Hz (20ms)

void sendTelemetry()
{
    if (millis() - previousTelemetryMillis >= telemetryInterval)
    {
        previousTelemetryMillis = millis();

        // Extended CSV protocol for PID tracking and battery monitor
        Serial.print(imu.getPitch(), 2);
        Serial.print(",");
        Serial.print(imu.getRoll(), 2);
        Serial.print(",");
        Serial.print(radio.getThrottle());
        Serial.print(",");
        Serial.print(battery.getVoltage(), 2);
        Serial.print(",");
        Serial.println(currentState);
    }
}

void setup()
{
    Serial.begin(115200);

    // Low-level HAL Scaffolding setup
    imu.init();
    radio.init();
    motors.init();
    battery.init();

    currentState = DISARMED;
    Serial.println("AEGIS-QUAD: Core Scaffolding Operational. System Disarmed.");
}

void loop()
{
    // 1. Enforce deterministic hard real-time execution constraint
    while (micros() - previousMicros < targetCycleTime)
        ;
    previousMicros = micros();

    // 2. Fetch network background data stream and hardware metrics
    radio.readReceiver();
    battery.update();

    // 3. Global Guardrail: Battery critical voltage check
    if (battery.isCritical() && currentState == ARMED)
    {
        currentState = FAILSAFE;
        Serial.println("CRITICAL BATTERY OUTAGE! Emergency Failsafe Triggered.");
    }

    // 4. Finite State Machine Executive Core Loop
    switch (currentState)
    {
    case DISARMED:
        imu.updateAttitude();
        motors.commandAllMin();
        rollPID.reset();
        pitchPID.reset();
        yawPID.reset();

        if (radio.checkArmingSequence())
        {
            currentState = CALIBRATING;
        }
        break;

    case CALIBRATING:
        motors.commandAllMin();
        Serial.println("AEGIS-QUAD: Sensor Bias Recalibration in Progress...");
        imu.calibrateGyro();

        currentState = ARMED;
        Serial.println("AEGIS-QUAD: Safety Overridden. MOTORS LIVE.");
        break;

    case ARMED:
        imu.updateAttitude(); // Active Sensor Fusion engine runtime

        // --- PID Input Target Generation (Mapping Radio to Angles) ---
        // Mapping pulse widths (1000-2000us) to target stabilization angles (-30 deg to +30 deg)
        float rollSetpoint = ((float)radio.getRoll() - 1500.0f) * 0.06f;
        float pitchSetpoint = ((float)radio.getPitch() - 1500.0f) * 0.06f;

        // Yaw stick maps directly to angular velocity setpoint (-100 deg/s to +100 deg/s)
        float yawSetpoint = ((float)radio.getYaw() - 1500.0f) * 0.2f;

        // Extract raw baseline throttle from pilot stick
        uint16_t throttleBaseline = radio.getThrottle();

        // --- Compute PID Corrective Actuation Outputs ---
        float rollOutput = rollPID.compute(rollSetpoint, imu.getRoll());
        float pitchOutput = pitchPID.compute(pitchSetpoint, imu.getPitch());

        // For Yaw stabilization we feed raw gyroscope Z-axis angular velocity
        float yawOutput = yawPID.compute(yawSetpoint, imu.getGyroZ());

        // --- Motor Mixer Matrix Logic (Quad-X Configuration) ---
        // Only mix active PID stabilization if throttle is above safe idle threshold
        if (throttleBaseline > 1060)
        {
            uint16_t m1_speed = throttleBaseline - rollOutput + pitchOutput - yawOutput; // Front-Right
            uint16_t m2_speed = throttleBaseline - rollOutput - pitchOutput + yawOutput; // Rear-Right
            uint16_t m3_speed = throttleBaseline + rollOutput - pitchOutput - yawOutput; // Rear-Left
            uint16_t m4_speed = throttleBaseline + rollOutput + pitchOutput + yawOutput; // Front-Left

            // Write unified blended vector directly to the low-level ESC driver
            motors.writeMotors(m1_speed, m2_speed, m3_speed, m4_speed);
        }
        else
        {
            // Idle Spin / Min Throttle Guardrail when stick is down to prevent ground spin
            motors.writeMotors(1040, 1040, 1040, 1040);
        }

        // Check for immediate emergency manual disarm sequence
        if (radio.checkDisarmingSequence())
        {
            currentState = DISARMED;
            Serial.println("AEGIS-QUAD: System Disarmed Safely.");
        }
        break;

    case FAILSAFE:
        motors.commandAllMin(); // Force motor safe shutdown threshold
        break;

    default:
        currentState = FAILSAFE;
        break;
    }

    // 5. Stream isolated metrics without loop degradation
    sendTelemetry();
}
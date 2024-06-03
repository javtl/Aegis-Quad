/**
 * @file main.cpp
 * @author javtl
 * @brief Aegis-Quad Flight Control System - Integrated Executive FSM Core
 * @version 1.5
 * @date 2024-06-03
 */

#include <Arduino.h>
#include "IMU.h"
#include "Radio.h"
#include "Motors.h"
#include "Battery.h"
#include "PID.h"
#include "Blackbox.h"

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
Blackbox blackbox(10); // Instancia de la Blackbox usando el Pin 10 como Chip Select (CS)

// --- Phase 3: Triple-Axis PID Control Loop Instantiation ---
PID rollPID(1.20f, 0.05f, 0.035f);
PID pitchPID(1.20f, 0.05f, 0.035f);
PID yawPID(2.00f, 0.02f, 0.00f);

// --- High-Resolution Timing Constraints ---
unsigned long previousMicros = 0;
const unsigned long targetCycleTime = 4000; // Hard real-time 250Hz Main Loop (4ms)

unsigned long previousTelemetryMillis = 0;
const unsigned long telemetryInterval = 20; // Bound telemetry serialization to 50Hz (20ms)

// Phase 4: Multi-Rate Blackbox Constraints
unsigned long previousBlackboxMillis = 0;
const unsigned long blackboxInterval = 50; // Bound local SD logging to 20Hz (50ms) to prevent write stalls

// Phase 4: Static Telemetry Buffer Allocation to eliminate dynamic overhead
char telemetryBuffer[96];

void sendTelemetry()
{
    if (millis() - previousTelemetryMillis >= telemetryInterval)
    {
        previousTelemetryMillis = millis();

        // High-speed single-pass formatting directly into memory block
        int written = snprintf(telemetryBuffer, sizeof(telemetryBuffer),
                               "%lu,%.2f,%.2f,%u,%.2f,%d",
                               previousTelemetryMillis,
                               imu.getPitch(),
                               imu.getRoll(),
                               radio.getThrottle(),
                               battery.getVoltage(),
                               (int)currentState);

        // Single atomic hardware UART write operation
        if (written > 0 && written < (int)sizeof(telemetryBuffer))
        {
            Serial.println(telemetryBuffer);
        }
    }
}

void writeBlackbox()
{
    // Solo escribimos localmente en la SD si ha pasado el intervalo de 50ms y el dron está armado
    if (currentState == ARMED && (millis() - previousBlackboxMillis >= blackboxInterval))
    {
        previousBlackboxMillis = millis();

        // Ingestión atómica directa utilizando el buffer estático pre-formateado
        blackbox.writeRow(telemetryBuffer);
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

    // Inicialización del sistema de almacenamiento Blackbox
    blackbox.init();

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
        float rollSetpoint = ((float)radio.getRoll() - 1500.0f) * 0.06f;
        float pitchSetpoint = ((float)radio.getPitch() - 1500.0f) * 0.06f;
        float yawSetpoint = ((float)radio.getYaw() - 1500.0f) * 0.2f;

        // Extract raw baseline throttle from pilot stick
        uint16_t throttleBaseline = radio.getThrottle();

        // --- Compute PID Corrective Actuation Outputs ---
        float rollOutput = rollPID.compute(rollSetpoint, imu.getRoll());
        float pitchOutput = pitchPID.compute(pitchSetpoint, imu.getPitch());
        float yawOutput = yawPID.compute(yawSetpoint, imu.getGyroZ());

        // --- Motor Mixer Matrix Logic (Quad-X Configuration) ---
        if (throttleBaseline > 1060)
        {
            uint16_t m1_speed = throttleBaseline - rollOutput + pitchOutput - yawOutput; // Front-Right
            uint16_t m2_speed = throttleBaseline - rollOutput - pitchOutput + yawOutput; // Rear-Right
            uint16_t m3_speed = throttleBaseline + rollOutput - pitchOutput - yawOutput; // Rear-Left
            uint16_t m4_speed = throttleBaseline + rollOutput + pitchOutput + yawOutput; // Front-Left

            motors.writeMotors(m1_speed, m2_speed, m3_speed, m4_speed);
        }
        else
        {
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
        motors.commandAllMin();
        break;

    default:
        currentState = FAILSAFE;
        break;
    }

    // 5. Stream isolated metrics without loop degradation
    sendTelemetry();

    // 6. Asynchronously append current telemetry frame down to SD block
    writeBlackbox();
}
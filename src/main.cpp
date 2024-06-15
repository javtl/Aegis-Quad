/**
 * @file main.cpp
 * @author javtl
 * @brief Aegis-Quad Flight Control System - Integrated Executive FSM Core
 * @version 1.6
 * @date 2026-06-28
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
Blackbox blackbox(10); // Chip Select (CS) en Pin 10

// --- Phase 3: Triple-Axis PID Control Loop Instantiation ---
PID rollPID(1.20f, 0.05f, 0.035f);
PID pitchPID(1.20f, 0.05f, 0.035f);
PID yawPID(2.00f, 0.02f, 0.00f);

// --- High-Resolution Timing Constraints ---
unsigned long previousMicros = 0;
const unsigned long targetCycleTime = 4000; // Hard real-time 250Hz Main Loop (4ms)

unsigned long previousTelemetryMillis = 0;
const unsigned long telemetryInterval = 20; // Telemetría UART a 50Hz (20ms)

unsigned long previousBlackboxMillis = 0;
const unsigned long blackboxInterval = 50; // Escritura SD a 20Hz (50ms) para evitar write stalls

// --- Static Buffers Allocation (Elimina overhead dinámico en runtime) ---
char telemetryBuffer[96];
char blackboxBuffer[96];

void sendTelemetry()
{
    unsigned long currentMillis = millis();
    if (currentMillis - previousTelemetryMillis >= telemetryInterval)
    {
        previousTelemetryMillis = currentMillis;

        // Formateo directo en memoria estática
        int written = snprintf(telemetryBuffer, sizeof(telemetryBuffer), "%lu,%.2f,%.2f,%u,%.2f,%d",
                               previousTelemetryMillis, imu.getPitch(), imu.getRoll(),
                               radio.getThrottle(), battery.getVoltage(), (int)currentState);

        // Operación atómica de escritura UART
        if (written > 0 && written < (int)sizeof(telemetryBuffer))
        {
            Serial.println(telemetryBuffer);
        }
    }
}

void writeBlackbox()
{
    // Solo registramos si está ARMED y ha pasado el intervalo de 50ms
    unsigned long currentMillis = millis();
    if (currentState == ARMED && (currentMillis - previousBlackboxMillis >= blackboxInterval))
    {
        previousBlackboxMillis = currentMillis;

        // Genera su propio frame independiente para evitar colisiones con el ritmo de la telemetría
        int written = snprintf(blackboxBuffer, sizeof(blackboxBuffer), "%lu,%.2f,%.2f,%u,%.2f,%d",
                               previousBlackboxMillis, imu.getPitch(), imu.getRoll(),
                               radio.getThrottle(), battery.getVoltage(), (int)currentState);

        if (written > 0 && written < (int)sizeof(blackboxBuffer))
        {
            blackbox.writeRow(blackboxBuffer);
        }
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
    blackbox.init();

    currentState = DISARMED;
    Serial.println("AEGIS-QUAD: Core Scaffolding Operational. System Disarmed.");
}

void loop()
{
    // 1. Enforce deterministic hard real-time execution constraint (250Hz)
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
        // Permite desarmar manualmente en Failsafe para cortar alertas/bloqueos
        if (radio.checkDisarmingSequence())
        {
            currentState = DISARMED;
            Serial.println("AEGIS-QUAD: Failsafe Cleared. System Disarmed.");
        }
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
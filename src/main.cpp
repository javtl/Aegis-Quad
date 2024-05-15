/**
 * @file main.cpp
 * @author javtl
 * @brief Aegis-Quad Flight Control System - Integrated Executive FSM Core
 * @version 1.1
 * @date 2024-05-15
 */

#include <Arduino.h>
#include "IMU.h"
#include "Radio.h"
#include "Motors.h"

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
Radio radio(Serial2); // Explicit UART routing for digital RC receiver
Motors motors;

// --- High-Resolution Timing Constraints ---
unsigned long previousMicros = 0;
const unsigned long targetCycleTime = 4000; // Hard real-time 250Hz Main Loop (4ms)

unsigned long previousTelemetryMillis = 0;
const unsigned long telemetryInterval = 20; // Bound telemetry serialization to 50Hz (20ms)

/**
 * @brief High-speed non-blocking telemetry payload deployment
 */
void sendTelemetry()
{
    if (millis() - previousTelemetryMillis >= telemetryInterval)
    {
        previousTelemetryMillis = millis();

        // Compact CSV protocol format for real-time serial plotting
        Serial.print(imu.getPitch(), 2);
        Serial.print(",");
        Serial.print(imu.getRoll(), 2);
        Serial.print(",");
        Serial.print(radio.getThrottle());
        Serial.print(",");
        Serial.print(radio.getYaw());
        Serial.print(",");
        Serial.println(currentState); // Track numerical state value
    }
}

void setup()
{
    Serial.begin(115200); // Primary debug and data-logging pipeline

    // Low-level HAL Scaffolding setup
    imu.init();
    radio.init();
    motors.init();

    currentState = DISARMED;
    Serial.println("AEGIS-QUAD: Core Scaffolding Operational. System Disarmed.");
}

void loop()
{
    // 1. Enforce deterministic hard real-time execution constraint
    while (micros() - previousMicros < targetCycleTime)
        ;
    previousMicros = micros();

    // 2. Fetch network background data stream (non-blocking rx UART parsing)
    radio.readReceiver();

    // 3. Finite State Machine Executive Core Loop
    switch (currentState)
    {
    case DISARMED:
        imu.updateAttitude();   // Track current orientation in standby
        motors.commandAllMin(); // Enforce hardware cutoff safety signal

        // Verify if the safety sustained stick combination is fulfilled
        if (radio.checkArmingSequence())
        {
            currentState = CALIBRATING;
        }
        break;

    case CALIBRATING:
        motors.commandAllMin(); // Keep safe boundaries while configuring
        Serial.println("AEGIS-QUAD: Sensor Bias Recalibration in Progress...");
        imu.calibrateGyro(); // Wipe out drift offsets right before throttle window opens

        currentState = ARMED;
        Serial.println("AEGIS-QUAD: Safety Overridden. MOTORS LIVE.");
        break;

    case ARMED:
        imu.updateAttitude(); // Active Sensor Fusion engine runtime

        // [Fase 3: PID Computation, Motor Mixer Integration and writeMotors execution will be placed here]

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

    // 4. Stream isolated metrics without loop degradation
    sendTelemetry();
}
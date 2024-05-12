/**
 * @file main.cpp
 * @author Your Name
 * @brief Aegis-Quad Flight Control System - Unified Phase 1 Core Execution
 * @version 1.0
 * @date 2024-05-12
 */

#include <Arduino.h>
#include "IMU.h"
#include "Radio.h"

// --- System States ---
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
Radio radio(Serial2); // Se asume Serial2 para la Radio (IBUS)

// --- Timing Control (Hard Real-Time Architecture) ---
unsigned long previousMicros = 0;
const unsigned long targetCycleTime = 4000; // 250Hz Main Loop (4000us)

unsigned long previousTelemetryMillis = 0;
const unsigned long telemetryInterval = 20; // Telemetría a 50Hz (cada 20ms) para no saturar el bus

// --- Optimized Telemetry Function ---
void sendTelemetry()
{
    if (millis() - previousTelemetryMillis >= telemetryInterval)
    {
        previousTelemetryMillis = millis();

        // Formato CSV ligero optimizado para Serial Plotter
        Serial.print(imu.getPitch(), 2);
        Serial.print(",");
        Serial.print(imu.getRoll(), 2);
        Serial.print(",");
        Serial.print(radio.getThrottle());
        Serial.print(",");
        Serial.println(radio.getYaw());
    }
}

void setup()
{
    Serial.begin(115200); // Telemetría principal por el puerto UART0 (USB)

    // HAL Initializations
    imu.init();
    radio.init();

    // Calibración inicial estática del giroscopio
    currentState = CALIBRATING;
    Serial.println("AEGIS-QUAD: Calibrating sensors... Keep flat.");
    imu.calibrateGyro();

    currentState = DISARMED;
    Serial.println("AEGIS-QUAD: System Ready.");
}

void loop()
{
    // 1. Enforce strict 250Hz frequency loop
    while (micros() - previousMicros < targetCycleTime)
        ;
    previousMicros = micros();

    // 2. Read Background Radio stream (Non-blocking UART buffer check)
    radio.readReceiver();

    // 3. Finite State Machine Executive Core
    switch (currentState)
    {
    case DISARMED:
        imu.updateAttitude(); // Seguir calculando ángulos en reposo

        // Lógica de transición temporal (Si el switch auxiliar está activo -> ARM)
        if (radio.getSwitchA() > 1500 && radio.getThrottle() < 1050)
        {
            currentState = ARMED;
        }
        break;

    case ARMED:
        imu.updateAttitude(); // Fusión de sensores en tiempo real

        // Aquí irá el cómputo de la clase PID en la Fase 3

        if (radio.getSwitchA() < 1500)
        { // Desarmado de emergencia instantáneo
            currentState = DISARMED;
        }
        break;

    case FAILSAFE:
        // Apagar comandos de salida (Fase 4)
        break;

    default:
        currentState = FAILSAFE;
        break;
    }

    // 4. Stream non-blocking telemetry data
    sendTelemetry();
}
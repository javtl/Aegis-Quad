/**
 * @file Battery.h
 * @author javtl
 * @brief Battery Voltage Monitor via ADC with Moving Average Filter
 * @version 1.0
 * @date 2024-05-19
 */

#ifndef BATTERY_H
#define BATTERY_H

#include <Arduino.h>

class Battery
{
private:
    const int ADC_PIN = 36;                    // pin ADC1_CH0
    const float VOLTAGE_DIVIDER_FACTOR = 4.03; // Inversa del factor de atenuación (1 / 0.2481)
    const float ADC_REF_VOLTAGE = 3.3;         // Voltaje de referencia del ADC
    const int ADC_RESOLUTION = 4095;           // 12 bits (0-4095)

    // Configuración del Filtro de Media Móvil
    static const int FILTER_SAMPLES = 10;
    float sampleBuffer[FILTER_SAMPLES];
    int bufferIndex = 0;
    float currentFilteredVoltage = 0.0;

public:
    /**
     * @brief Initializes the ADC pin and primes the filter buffer with initial readings.
     */
    void init();

    /**
     * @brief Samples the raw ADC channel, injects it into the circular buffer, and computes the moving average.
     */
    void update();

    /**
     * @brief Returns the clean, filtered voltage of the battery pack.
     */
    float getVoltage() const { return currentFilteredVoltage; }

    /**
     * @brief Critical state safety guardrail check.
     * @return true if battery voltage drops under the critical 3S threshold (9.6V / 3.2V per cell).
     */
    bool isCritical() const { return (currentFilteredVoltage < 9.6 && currentFilteredVoltage > 1.0); }
};

#endif // BATTERY_H
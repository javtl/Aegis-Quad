/**
 * @file Battery.cpp
 * @author javtl
 * @brief Implementation of the circular buffer digital filter for battery cells
 * @version 1.0
 * @date 2024-05-19
 */

#include "Battery.h"

void Battery::init()
{
    pinMode(ADC_PIN, INPUT);

    // Muestreo inicial rápido para rellenar el búfer y evitar lecturas espurias en el arranque
    float initialRead = (analogRead(ADC_PIN) * ADC_REF_VOLTAGE / ADC_RESOLUTION) * VOLTAGE_DIVIDER_FACTOR;
    for (int i = 0; i < FILTER_SAMPLES; i++)
    {
        sampleBuffer[i] = initialRead;
    }
    currentFilteredVoltage = initialRead;
}

void Battery::update()
{
    // 1. Convertir lectura analógica pura a voltaje real de la batería
    int rawADC = analogRead(ADC_PIN);
    float rawVoltage = (rawADC * ADC_REF_VOLTAGE / (float)ADC_RESOLUTION) * VOLTAGE_DIVIDER_FACTOR;

    // 2. Inyectar la muestra en el búfer circular reemplazando la más antigua
    sampleBuffer[bufferIndex] = rawVoltage;
    bufferIndex = (bufferIndex + 1) % FILTER_SAMPLES;

    // 3. Calcular la media aritmética de las muestras almacenadas
    float sum = 0.0;
    for (int i = 0; i < FILTER_SAMPLES; i++)
    {
        sum += sampleBuffer[i];
    }
    currentFilteredVoltage = sum / (float)FILTER_SAMPLES;
}
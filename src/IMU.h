#ifndef IMU_H
#define IMU_H

#include <Arduino.h>
#include <Wire.h>

class IMU
{
private:
    const int MPU_ADDR = 0x68;
    int16_t rawAccelX, rawAccelY, rawAccelZ;
    int16_t rawGyroX, rawGyroY, rawGyroZ;

    // Offsets de calibración
    float gyroBiasX = 0, gyroBiasY = 0, gyroBiasZ = 0;

public:
    void init();
    void readRawData();
    void calibrateGyro();

    // Getters para el siguiente ticket
    float getAccelX() { return rawAccelX / 16384.0; }           // Conversión a Gs (escala +/-2g)
    float getGyroX() { return (rawGyroX - gyroBiasX) / 131.0; } // Conversión a º/s (escala +/-250º/s)
};

#endif
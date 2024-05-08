/**
 * @file IMU.cpp
 * @author Your Name
 * @brief Implementation of the IMU class using wire.h for fast-mode I2C
 * @version 1.0
 * @date 2024-05-08
 */

#include "IMU.h"
#include <math.h>

void IMU::init()
{
    Wire.begin();
    Wire.setClock(400000); // 400kHz Fast Mode I2C configuration

    // Wake up MPU6050 by writing 0x00 to the Power Management 1 register (0x6B)
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x6B);
    Wire.write(0x00);
    Wire.endTransmission(true);
}

void IMU::readRawData()
{
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B);            // Starting register address (Accel_X_Out_High)
    Wire.endTransmission(false); // Send restart command to keep bus active

    Wire.requestFrom(MPU_ADDR, 14, true); // Request 14 bytes in a single burst

    // Bit shift high and low bytes into 16-bit signed integers
    rawAccelX = (Wire.read() << 8) | Wire.read();
    rawAccelY = (Wire.read() << 8) | Wire.read();
    rawAccelZ = (Wire.read() << 8) | Wire.read();

    (void)((Wire.read() << 8) | Wire.read()); // Read and discard temperature bytes

    rawGyroX = (Wire.read() << 8) | Wire.read();
    rawGyroY = (Wire.read() << 8) | Wire.read();
    rawGyroZ = (Wire.read() << 8) | Wire.read();
}

void IMU::calibrateGyro()
{
    long sumX = 0, sumY = 0, sumZ = 0;
    const int samples = 2000;

    // Accumulate samples while the drone is perfectly static
    for (int i = 0; i < samples; i++)
    {
        readRawData();
        sumX += rawGyroX;
        sumY += rawGyroY;
        sumZ += rawGyroZ;
        delay(2); // Short delay allowed ONLY during initialization setup
    }

    // Calculate final mean error offset
    gyroBiasX = sumX / (float)samples;
    gyroBiasY = sumY / (float)samples;
    gyroBiasZ = sumZ / (float)samples;
}

void IMU::updateAttitude()
{
    // 1. Fetch latest raw data from registers
    readRawData();

    // 2. Compute static angles from Accelerometer vectors using trigonometry
    float accelRoll = atan2(rawAccelY, rawAccelZ) * 180.0 / M_PI;
    float accelPitch = atan2(-rawAccelX, sqrt((long)rawAccelY * rawAccelY + (long)rawAccelZ * rawAccelZ)) * 180.0 / M_PI;

    // 3. Sensor Fusion Loop (Complementary Filter formula)
    // High-pass on Gyro (dynamic/fast tracking) + Low-pass on Accel (long-term stabilization)
    roll = alpha * (roll + getGyroX() * dt) + (1.0 - alpha) * accelRoll;
    pitch = alpha * (pitch + getGyroY() * dt) + (1.0 - alpha) * accelPitch;
}
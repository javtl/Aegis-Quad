/**
 * @file IMU.h
 * @author Your Name
 * @brief Inertial Measurement Unit (MPU6050) Abstraction & Sensor Fusion
 * @version 1.0
 * @date 2024-05-08
 */

#ifndef IMU_H
#define IMU_H

#include <Arduino.h>
#include <Wire.h>

class IMU
{
private:
    // MPU6050 I2C Default Address
    const int MPU_ADDR = 0x68;

    // Raw register data buffers
    int16_t rawAccelX, rawAccelY, rawAccelZ;
    int16_t rawGyroX, rawGyroY, rawGyroZ;

    // Calibration bias coefficients
    float gyroBiasX = 0.0;
    float gyroBiasY = 0.0;
    float gyroBiasZ = 0.0;

    // Filtered attitude angles (degrees)
    float pitch = 0.0;
    float roll = 0.0;

    // Complementary Filter constants
    const float alpha = 0.98;
    const float dt = 0.004; // 4ms cycle time (250Hz loop)

public:
    /**
     * @brief Initializes the I2C bus at 400kHz and wakes up the MPU6050.
     */
    void init();

    /**
     * @brief Reads 14 bytes of raw data (Accel, Temp, Gyro) from the sensor.
     */
    void readRawData();

    /**
     * @brief Samples the gyroscope in a static position to calculate error offsets.
     */
    void calibrateGyro();

    /**
     * @brief Computes real Pitch and Roll angles via Complementary Filter sensor fusion.
     */
    void updateAttitude();

    // --- Getters for Hardware Abstraction Layer (HAL) ---
    float getAccelX() const { return rawAccelX / 16384.0; } // Scale factor for +/-2g
    float getAccelY() const { return rawAccelY / 16384.0; }
    float getAccelZ() const { return rawAccelZ / 16384.0; }

    float getGyroX() const { return (rawGyroX - gyroBiasX) / 131.0; } // Scale factor for +/-250 deg/s
    float getGyroY() const { return (rawGyroY - gyroBiasY) / 131.0; }
    float getGyroZ() const { return (rawGyroZ - gyroBiasZ) / 131.0; }

    float getPitch() const { return pitch; }
    float getRoll() const { return roll; }
};

#endif // IMU_H
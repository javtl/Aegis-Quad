#include "IMU.h"

void IMU::init()
{
    Wire.begin();
    Wire.setClock(400000); // 400kHz Fast Mode I2C

    // Despertar el MPU6050 (Registro 0x6B / Power Management)
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x6B);
    Wire.write(0x00);
    Wire.endTransmission(true);
}

void IMU::readRawData()
{
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B); // Registro de inicio (Accel_X_High)
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, 14, true); // Solicitar 14 bytes

    rawAccelX = Wire.read() << 8 | Wire.read();
    rawAccelY = Wire.read() << 8 | Wire.read();
    rawAccelZ = Wire.read() << 8 | Wire.read();
    (void)(Wire.read() << 8 | Wire.read()); // Ignorar registro de temperatura
    rawGyroX = Wire.read() << 8 | Wire.read();
    rawGyroY = Wire.read() << 8 | Wire.read();
    rawGyroZ = Wire.read() << 8 | Wire.read();
}

void IMU::calibrateGyro()
{
    long sumX = 0, sumY = 0, sumZ = 0;
    const int samples = 2000;

    for (int i = 0; i < samples; i++)
    {
        readRawData();
        sumX += rawGyroX;
        sumY += rawGyroY;
        sumZ += rawGyroZ;
        delay(2); // Retardo controlado solo durante la calibración inicial
    }
    gyroBiasX = sumX / (float)samples;
    gyroBiasY = sumY / (float)samples;
    gyroBiasZ = sumZ / (float)samples;
}
/**
 * @file Blackbox.h
 * @author javtl
 * @brief Blackbox Flight Logging System - SPI SD Card HAL
 * @version 1.0
 * @date 2024-06-01
 */

#ifndef BLACKBOX_H
#define BLACKBOX_H

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

class Blackbox
{
private:
    uint8_t _csPin;
    bool _isOperational;
    char _filename[13]; // Estructura de nombre de archivo 8.3 de FAT16/FAT32

    void generateUniqueFilename();

public:
    Blackbox(uint8_t csPin = 10);

    bool init();
    void writeRow(const char *dataBuffer);
    bool isOperational() const { return _isOperational; }
    const char *getFilename() const { return _filename; }
};

#endif // BLACKBOX_H
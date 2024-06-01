/**
 * @file Blackbox.cpp
 * @author javtl
 * @brief Blackbox Flight Logging System - SPI SD Card HAL Implementation
 * @version 1.0
 * @date 2024-06-01
 */

#include "Blackbox.h"

Blackbox::Blackbox(uint8_t csPin) : _csPin(csPin), _isOperational(false)
{
    memset(_filename, 0, sizeof(_filename));
}

bool Blackbox::init()
{
    // Inicializar la interfaz SD mediante el bus SPI físico
    if (!SD.begin(_csPin))
    {
        Serial.println("AEGIS-BLACKBOX: Hardware SPI Initialization FAILED. SD Card missing or faulty.");
        _isOperational = false;
        return false;
    }

    _isOperational = true;
    generateUniqueFilename();

    // Crear el archivo e inscribir la cabecera CSV de forma inmediata
    File dataFile = SD.open(_filename, FILE_WRITE);
    if (dataFile)
    {
        dataFile.println("Time_ms,Pitch,Roll,Throttle,Voltage,State");
        dataFile.close();
        Serial.print("AEGIS-BLACKBOX: Core Active. Logging to target: ");
        Serial.println(_filename);
    }
    else
    {
        Serial.println("AEGIS-BLACKBOX: Fatal error creating log structure descriptor.");
        _isOperational = false;
    }

    return _isOperational;
}

void Blackbox::generateUniqueFilename()
{
    // Escaneo iterativo en el sistema de archivos para no sobrescribir vuelos anteriores
    for (uint16_t index = 0; index < 1000; index++)
    {
        snprintf(_filename, sizeof(_filename), "LOG_%03u.CSV", index);
        if (!SD.exists(_filename))
        {
            // Hemos encontrado un slot de nombre disponible
            break;
        }
    }
}

void Blackbox::writeRow(const char *dataBuffer)
{
    if (!_isOperational)
        return;

    // Apertura y volcado atómico rápido del buffer de memoria estático
    File dataFile = SD.open(_filename, FILE_WRITE);
    if (dataFile)
    {
        dataFile.println(dataBuffer);
        dataFile.close(); // Fuerza el vaciado del búfer del sector a la memoria física flash
    }
}
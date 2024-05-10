/**
 * @file Radio.h
 * @author Your Name
 * @brief i-BUS Digital RC Receiver Protocol Decoder
 * @version 1.0
 * @date 2024-05-10
 */

#ifndef RADIO_H
#define RADIO_H

#include <Arduino.h>

class Radio
{
private:
    static const int IBUS_BUFF_SIZE = 32;
    uint8_t ibusBuffer[IBUS_BUFF_SIZE];
    uint16_t channels[6]; // Mapeamos solo los primeros 6 canales esenciales

    HardwareSerial &rxSerial; // Referencia al puerto UART físico usado

public:
    /**
     * @brief Constructor asignando el puerto serie de hardware (e.g., Serial2 o Serial1)
     */
    Radio(HardwareSerial &serialPort);

    /**
     * @brief Initializes the target serial interface at 115200 baud.
     */
    void init();

    /**
     * @brief Non-blocking parser that reads incoming serial buffers and validates checksums.
     */
    bool readReceiver();

    // --- HAL Getters (valores normalizados o en microsegundos de 1000 a 2000) ---
    uint16_t getRoll() const { return channels[0]; }     // Canal 1
    uint16_t getPitch() const { return channels[1]; }    // Canal 2
    uint16_t getThrottle() const { return channels[2]; } // Canal 3
    uint16_t getYaw() const { return channels[3]; }      // Canal 4
    uint16_t getSwitchA() const { return channels[4]; }  // Canal 5 (Aux 1)
    uint16_t getSwitchB() const { return channels[5]; }  // Canal 6 (Aux 2)
};

#endif // RADIO_H
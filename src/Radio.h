/**
 * @file Radio.h
 * @author javtl
 * @brief i-BUS Digital RC Receiver Protocol Decoder & Safety Arming Logic
 * @version 1.1
 * @date 2024-05-15
 */

#ifndef RADIO_H
#define RADIO_H

#include <Arduino.h>

class Radio
{
private:
    static const int IBUS_BUFF_SIZE = 32;
    uint8_t ibusBuffer[IBUS_BUFF_SIZE];
    uint16_t channels[6]; // Maps the first 6 core RC channels

    HardwareSerial &rxSerial; // Hardware UART link

    // Safety & Arming non-blocking timers
    unsigned long armingTimer = 0;
    bool isArmingSequenceMet = false;

public:
    /**
     * @brief Constructor binding the hardware serial port (e.g., Serial2).
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

    /**
     * @brief Verifies if the pilot holds the stick combination for Arming (1.5s hold).
     * @return true if the arming criteria is met successfully.
     */
    bool checkArmingSequence();

    /**
     * @brief Verifies if the pilot triggers the instant disarming combination.
     * @return true if the disarming action is commanded.
     */
    bool checkDisarmingSequence();

    // --- HAL Getters (Returns raw microsecond PWM widths: 1000us - 2000us) ---
    uint16_t getRoll() const { return channels[0]; }     // Channel 1
    uint16_t getPitch() const { return channels[1]; }    // Channel 2
    uint16_t getThrottle() const { return channels[2]; } // Channel 3 (Critical Safety)
    uint16_t getYaw() const { return channels[3]; }      // Channel 4
    uint16_t getSwitchA() const { return channels[4]; }  // Channel 5 (Aux 1 - 2 Position Switch)
    uint16_t getSwitchB() const { return channels[5]; }  // Channel 6 (Aux 2)
};

#endif // RADIO_H
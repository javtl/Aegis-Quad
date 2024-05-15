/**
 * @file Radio.cpp
 * @author javtl
 * @brief Non-blocking i-BUS processing and sustained timing validation
 * @version 1.1
 * @date 2024-05-15
 */

#include "Radio.h"

Radio::Radio(HardwareSerial &serialPort) : rxSerial(serialPort)
{
    for (int i = 0; i < 6; i++)
    {
        channels[i] = 1500; // Safe default centered positions (Neutral attitude)
    }
    channels[2] = 1000; // Strict safety constraint: Initialize Throttle to absolute minimum
}

void Radio::init()
{
    rxSerial.begin(115200); // i-BUS operates strictly at 115200 bps
}

bool Radio::readReceiver()
{
    while (rxSerial.available() >= IBUS_BUFF_SIZE)
    {
        if (rxSerial.peek() == 0x20)
        { // Check valid header packet length byte
            rxSerial.readBytes(ibusBuffer, IBUS_BUFF_SIZE);

            // Mathematical verification of 16-bit Checksum
            uint16_t calculatedChecksum = 0xFFFF;
            for (int i = 0; i < 30; i++)
            {
                calculatedChecksum -= ibusBuffer[i];
            }

            uint16_t receivedChecksum = ibusBuffer[30] | (ibusBuffer[31] << 8);

            if (calculatedChecksum == receivedChecksum)
            {
                // Byte parsing (Little Endian alignment: low byte first)
                for (int i = 0; i < 6; i++)
                {
                    channels[i] = ibusBuffer[2 + i * 2] | (ibusBuffer[3 + i * 2] << 8);
                }
                return true;
            }
        }
        else
        {
            rxSerial.read(); // Discard corrupt shifting bytes to resync packet frame boundary
        }
    }
    return false;
}

bool Radio::checkArmingSequence()
{
    // Condition: Throttle down (< 1050us) AND Yaw full right (> 1900us)
    if (getThrottle() < 1050 && getYaw() > 1900)
    {
        if (!isArmingSequenceMet)
        {
            isArmingSequenceMet = true;
            armingTimer = millis(); // Benchmark beginning of trigger
        }
        // Strict guardrail: Arm only if stick configuration is sustained for 1500ms
        if (millis() - armingTimer >= 1500)
        {
            return true;
        }
    }
    else
    {
        isArmingSequenceMet = false; // Instant reset if pilot fails to hold positions
    }
    return false;
}

bool Radio::checkDisarmingSequence()
{
    // Condition: Throttle down (< 1050us) AND Yaw full left (< 1100us)
    // Disarming is instantaneous to prioritize physical safety
    if (getThrottle() < 1050 && getYaw() < 1100)
    {
        return true;
    }
    return false;
}
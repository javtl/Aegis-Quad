/**
 * @file Radio.cpp
 * @author Your Name
 * @brief Non-blocking i-BUS frame processing implementation
 * @version 1.0
 * @date 2024-05-10
 */

#include "Radio.h"

Radio::Radio(HardwareSerial &serialPort) : rxSerial(serialPort)
{
    for (int i = 0; i < 6; i++)
    {
        channels[i] = 1500; // Inicialización segura a mitad de stick (neutral)
    }
    channels[2] = 1000; // Throttle inicializado al mínimo por seguridad
}

void Radio::init()
{
    rxSerial.begin(115200); // i-BUS opera estrictamente a 115200 baudios
}

bool Radio::readReceiver()
{
    // Leer mientras existan datos en el buffer de la UART
    while (rxSerial.available() >= IBUS_BUFF_SIZE)
    {
        if (rxSerial.peek() == 0x20)
        { // Validar byte de cabecera (Longitud 32)
            rxSerial.readBytes(ibusBuffer, IBUS_BUFF_SIZE);

            // Verificación del Checksum
            uint16_t calculatedChecksum = 0xFFFF;
            for (int i = 0; i < 30; i++)
            {
                calculatedChecksum -= ibusBuffer[i];
            }

            uint16_t receivedChecksum = ibusBuffer[30] | (ibusBuffer[31] << 8);

            if (calculatedChecksum == receivedChecksum)
            {
                // Decodificación de canales (Little Endian: byte bajo + byte alto)
                for (int i = 0; i < 6; i++)
                {
                    channels[i] = ibusBuffer[2 + i * 2] | (ibusBuffer[3 + i * 2] << 8);
                }
                return true; // Trama correcta y datos actualizados
            }
        }
        else
        {
            rxSerial.read(); // Descartar byte corrupto para buscar la siguiente cabecera
        }
    }
    return false; // No hay trama nueva lista
}
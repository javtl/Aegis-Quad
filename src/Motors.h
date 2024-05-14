/**
 * @file Motors.h
 * @author Your Name
 * @brief Low-level ESC PWM Driver using Hardware Timers
 * @version 1.0
 * @date 2024-05-14
 */

#ifndef MOTORS_H
#define MOTORS_H

#include <Arduino.h>

class Motors
{
private:
    // Pines asignados a los 4 motores (Geometría Quad-X)
    // Se eligen pines con soporte PWM por hardware/Timer dedicado
    const int MOTOR_1_PIN = 12; // Front-Right
    const int MOTOR_2_PIN = 13; // Rear-Right
    const int MOTOR_3_PIN = 14; // Rear-Left
    const int MOTOR_4_PIN = 27; // Front-Left

    // Constantes de seguridad para los ESCs
    const uint16_t MIN_PULSE = 1000; // us
    const uint16_t MAX_PULSE = 2000; // us

public:
    /**
     * @brief Configures the hardware timers and PWM channels for the 4 motor pins.
     */
    void init();

    /**
     * @brief Updates the speed of all 4 motors simultaneously.
     * @param m1 Speed for Motor 1 (1000 to 2000 us)
     * @param m2 Speed for Motor 2 (1000 to 2000 us)
     * @param m3 Speed for Motor 3 (1000 to 2000 us)
     * @param m4 Speed for Motor 4 (1000 to 2000 us)
     */
    void writeMotors(uint16_t m1, uint16_t m2, uint16_t m3, uint16_t m4);

    /**
     * @brief Forces all motors to minimum pulse (1000us) for safety/shutdown.
     */
    void commandAllMin();
};

#endif // MOTORS_H
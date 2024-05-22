/**
 * @file PID.cpp
 * @brief High-frequency implementation of proportional-integral-derivative math
 * @version 1.0
 * @date 2024-05-22
 */

#include "PID.h"
#include <Arduino.h> // For constrain utility function

PID::PID(float p, float i, float d) : kp(p), ki(i), kd(d)
{
    reset();
}

float PID::compute(float setpoint, float actualState)
{
    // 1. Calculate primary tracking error
    float error = setpoint - actualState;

    // 2. Proportional computation
    float pTerm = kp * error;

    // 3. Integral computation with strict Anti-Windup guardrail clamping
    integralTerm += ki * error * dt;
    integralTerm = constrain(integralTerm, -INTEGRAL_LIMIT, INTEGRAL_LIMIT);

    // 4. Derivative computation (Rate of change tracking)
    float dTerm = kd * ((error - previousError) / dt);
    previousError = error;

    // 5. Synthesize unified system output response
    return pTerm + integralTerm + dTerm;
}

void PID::reset()
{
    integralTerm = 0.0;
    previousError = 0.0;
}
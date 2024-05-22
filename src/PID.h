/**
 * @file PID.h
 * @author javtl
 * @brief Precision Closed-Loop PID Controller with Anti-Windup Guardrails
 * @version 1.0
 * @date 2024-05-22
 */

#ifndef PID_H
#define PID_H

class PID
{
private:
    // Tunable Gains
    float kp;
    float ki;
    float kd;

    // Control State Variables
    float integralTerm = 0.0;
    float previousError = 0.0;

    // Safety Bounds
    const float INTEGRAL_LIMIT = 400.0; // Maximum integration saturation constraint
    const float dt = 0.004;             // Hardcoded strict 250Hz period delta (4ms)

public:
    /**
     * @brief Constructor to setup the PID instance gains.
     */
    PID(float p, float i, float d);

    /**
     * @brief Computes the corrected actuator output based on target vs state error.
     * @param setpoint Desired value (e.g., pilot stick position in degrees)
     * @param actualState Real value measured by the sensor fusion engine
     * @return float Adjusted control output variable
     */
    float compute(float setpoint, float actualState);

    /**
     * @brief Resets the integral memory buffer to prevent unexpected ground acceleration spikes.
     */
    void reset();

    // Getters / Setters for runtime tuning exploration
    void setGains(float p, float i, float d)
    {
        kp = p;
        ki = i;
        kd = d;
    }
};

#endif // PID_H
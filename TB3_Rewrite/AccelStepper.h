#ifndef AccelStepper_h
#define AccelStepper_h

#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#include <wiring.h>
#endif

/**
 * \class AccelStepper
 * \brief Provides control for single 2 or 4 pin stepper motors with optional acceleration, deceleration, and absolute positioning.
 * Supports multiple steppers with different speeds and accelerations. 
 * Important: This is an open loop controller. If the motor stalls or is oversped, AccelStepper won't know the motor's real position.
 */

constexpr uint8_t INVALID_PIN = 0xFF;
     
class AccelStepper
{
public:
    typedef enum
    {
        DIRECTION_CCW = 0,  // Counter-Clockwise
        DIRECTION_CW  = 1   // Clockwise
    } Direction;

    AccelStepper(uint8_t pin1, uint8_t pin2, uint8_t enablePin = INVALID_PIN, bool enable = true);

    // Positioning Functions
    void moveTo(int32_t absolute);
    void move(int32_t relative);
    uint32_t run();
    int32_t distanceToGo();
    Direction direction(int32_t distanceTo);
    int32_t targetPosition();
    int32_t currentPosition();
    void setCurrentPosition(int32_t position);

    // Speed and Acceleration Functions
    void setMaxSpeed(uint16_t maxSpeed);
    uint16_t maxSpeed();
    void setAcceleration(uint16_t acceleration);
    uint16_t acceleration();
    void setSpeed(uint16_t speed);
    uint16_t speed();
    uint32_t runSpeed();

    // Utility Functions
    void stop();
    bool isRunning();
    void setEnablePin(uint8_t enablePin = 0xff);
    void setPinsInverted(bool directionInvert = false, bool stepInvert = false, bool enableInvert = false);

    virtual void disableOutputs();
    virtual void enableOutputs();
    virtual ~AccelStepper() {};

protected:
    uint32_t computeNewSpeed();
    void setOutputPins(uint8_t mask);
    void step();
    uint32_t roundedDivision(uint32_t numerator, uint32_t denominator);
    uint16_t sqrtApprox(int32_t number);

private: 
    uint8_t _pin[2];
    bool _pinInverted[2];
    bool _direction;
    uint32_t _stepInterval;
    int32_t _currentPos;
    int32_t _targetPos;
    uint16_t _speed;
    uint16_t _maxSpeed;
    uint16_t _acceleration;
    uint32_t _lastStepTime;
    bool _enableInverted;
    uint8_t _enablePin;
    int32_t _n;
    uint32_t _c0;
    uint32_t _cmin;
};

#endif
